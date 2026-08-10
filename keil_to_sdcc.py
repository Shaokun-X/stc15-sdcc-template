#!/usr/bin/env python3
"""
keil2sdcc.py

Convert common Keil C51 source/header syntax to SDCC/mcs51 syntax.

Usage:
    python3 keil2sdcc.py INPUT_DIR OUTPUT_DIR

Examples:
    python3 keil2sdcc.py ./keil_project ./sdcc_project
    python3 keil2sdcc.py ./inc ./converted_inc --no-copy

What it converts:
    sfr NAME = 0x80;
        -> __sfr __at (0x80) NAME;

    sfr16 NAME = 0xCC;
        -> __sfr16 __at (0xCC) NAME;

    sbit NAME = 0x90;
        -> __sbit __at (0x90) NAME;

    sbit NAME = P1^3;
        -> __sbit __at (0x93) NAME;
        if P1's address is known

    unsigned char xdata foo;
        -> __xdata unsigned char foo;

    unsigned char code table[];
        -> __code unsigned char table[];

    bit flag;
        -> __bit flag;

    void foo(void) interrupt 1
        -> void foo(void) __interrupt (1)

    void foo(void) interrupt 1 using 2
        -> void foo(void) __interrupt (1) __using (2)

    _nop_();
        -> __asm nop __endasm;

Also handles:
    data   -> __data
    idata  -> __idata
    xdata  -> __xdata
    pdata  -> __pdata
    code   -> __code
    reentrant -> __reentrant

Important:
    This is a source-to-source migration helper, not a complete C parser.
    Always compile the result with SDCC and inspect warnings/errors.
"""

from __future__ import annotations

import argparse
import re
import shutil
import sys
from dataclasses import dataclass, field
from pathlib import Path


# ---------------------------------------------------------------------------
# Regexes
# ---------------------------------------------------------------------------

IDENT = r"[A-Za-z_]\w*"
NUMBER = r"(?:0[xX][0-9A-Fa-f]+|\d+)"

SFR_RE = re.compile(
    rf"""
    ^(?P<indent>\s*)
    sfr
    \s+
    (?P<name>{IDENT})
    \s*=\s*
    (?P<addr>{NUMBER})
    \s*;
    (?P<trailer>.*)
    $
    """,
    re.VERBOSE,
)

SFR16_RE = re.compile(
    rf"""
    ^(?P<indent>\s*)
    sfr16
    \s+
    (?P<name>{IDENT})
    \s*=\s*
    (?P<addr>{NUMBER})
    \s*;
    (?P<trailer>.*)
    $
    """,
    re.VERBOSE,
)

SFR32_RE = re.compile(
    rf"""
    ^(?P<indent>\s*)
    sfr32
    \s+
    (?P<name>{IDENT})
    \s*=\s*
    (?P<addr>{NUMBER})
    \s*;
    (?P<trailer>.*)
    $
    """,
    re.VERBOSE,
)

SBIT_ABSOLUTE_RE = re.compile(
    rf"""
    ^(?P<indent>\s*)
    sbit
    \s+
    (?P<name>{IDENT})
    \s*=\s*
    (?P<addr>{NUMBER})
    \s*;
    (?P<trailer>.*)
    $
    """,
    re.VERBOSE,
)

SBIT_REGISTER_RE = re.compile(
    rf"""
    ^(?P<indent>\s*)
    sbit
    \s+
    (?P<name>{IDENT})
    \s*=\s*
    (?P<reg>{IDENT}|{NUMBER})
    \s*\^\s*
    (?P<bit>[0-7])
    \s*;
    (?P<trailer>.*)
    $
    """,
    re.VERBOSE,
)


# Keil interrupt function:
#
#   void timer0(void) interrupt 1
#   void timer0(void) interrupt 1 using 2
#
INTERRUPT_USING_RE = re.compile(
    r"\binterrupt\s+(\d+)\s+using\s+([0-3])\b"
)

INTERRUPT_RE = re.compile(
    r"\binterrupt\s+(\d+)\b"
)

USING_RE = re.compile(
    r"\busing\s+([0-3])\b"
)

REENTRANT_RE = re.compile(r"\breentrant\b")


# Keil absolute placement:
#
#   unsigned char x _at_ 0x30;
#
AT_RE = re.compile(
    rf"\b_at_\s*({NUMBER})"
)


# ---------------------------------------------------------------------------
# Results/reporting
# ---------------------------------------------------------------------------

@dataclass
class ConversionStats:
    files_processed: int = 0
    files_copied: int = 0

    sfr: int = 0
    sfr16: int = 0
    sfr32: int = 0
    sbit: int = 0

    memory_keywords: int = 0
    bit_keywords: int = 0

    interrupts: int = 0
    using: int = 0
    reentrant: int = 0
    at_attributes: int = 0

    intrinsics: int = 0

    warnings: list[str] = field(default_factory=list)


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

def parse_int(value: str) -> int:
    return int(value, 0)


def hex8(value: int) -> str:
    return f"0x{value:02X}"


def hex16(value: int) -> str:
    return f"0x{value:04X}"


def split_comment(line: str) -> tuple[str, str]:
    """
    Split // comment from code.

    This is intentionally simple and avoids rewriting comments.
    """
    pos = line.find("//")

    if pos < 0:
        return line, ""

    return line[:pos], line[pos:]


def replace_outside_strings(
    line: str,
    replacement_function,
) -> str:
    """
    Apply replacement_function only to code, not inside quoted
    string/character literals.

    This is not a complete C lexer, but is sufficient for most
    embedded C source.
    """

    result = []
    current = []
    quote = None
    escaped = False

    def flush_code():
        if current:
            result.append(replacement_function("".join(current)))
            current.clear()

    for ch in line:
        if quote is None:
            if ch in ('"', "'"):
                flush_code()
                quote = ch
                current.append(ch)
            else:
                current.append(ch)

        else:
            current.append(ch)

            if escaped:
                escaped = False
            elif ch == "\\":
                escaped = True
            elif ch == quote:
                result.append("".join(current))
                current.clear()
                quote = None

    if current:
        if quote is None:
            result.append(replacement_function("".join(current)))
        else:
            result.append("".join(current))

    return "".join(result)


# ---------------------------------------------------------------------------
# First pass: collect SFR addresses
# ---------------------------------------------------------------------------

def collect_sfrs(text: str) -> dict[str, int]:
    """
    Collect Keil sfr declarations so that:

        sbit LED = P1^3;

    can become:

        __sbit __at (0x93) LED;
    """

    sfrs: dict[str, int] = {}

    for line in text.splitlines():
        m = SFR_RE.match(line)

        if m:
            sfrs[m.group("name")] = parse_int(m.group("addr"))

    return sfrs


def collect_global_sfrs(input_dir: Path) -> dict[str, int]:
    """
    Scan the whole project before conversion.

    This means an sbit in one header can refer to an SFR defined
    in another header.
    """

    sfrs: dict[str, int] = {}

    for path in input_dir.rglob("*"):
        if not path.is_file():
            continue

        if path.suffix.lower() not in {".c", ".h"}:
            continue

        try:
            text = path.read_text(
                encoding="utf-8",
                errors="replace",
            )
        except OSError:
            continue

        sfrs.update(collect_sfrs(text))

    return sfrs


# ---------------------------------------------------------------------------
# Specific declaration conversions
# ---------------------------------------------------------------------------

def convert_special_register_line(
    line: str,
    sfrs: dict[str, int],
    stats: ConversionStats,
    location: str,
) -> str | None:

    # ---------------------------------------------------------
    # sfr
    # ---------------------------------------------------------

    m = SFR_RE.match(line)

    if m:
        stats.sfr += 1

        return (
            f'{m.group("indent")}'
            f'__sfr __at ({hex8(parse_int(m.group("addr")))}) '
            f'{m.group("name")};'
            f'{m.group("trailer")}'
        )

    # ---------------------------------------------------------
    # sfr16
    # ---------------------------------------------------------

    m = SFR16_RE.match(line)

    if m:
        stats.sfr16 += 1

        return (
            f'{m.group("indent")}'
            f'__sfr16 __at ({hex8(parse_int(m.group("addr")))}) '
            f'{m.group("name")};'
            f'{m.group("trailer")}'
        )

    # ---------------------------------------------------------
    # sfr32
    # ---------------------------------------------------------

    m = SFR32_RE.match(line)

    if m:
        stats.sfr32 += 1

        return (
            f'{m.group("indent")}'
            f'__sfr32 __at ({hex8(parse_int(m.group("addr")))}) '
            f'{m.group("name")};'
            f'{m.group("trailer")}'
        )

    # ---------------------------------------------------------
    # absolute sbit
    # ---------------------------------------------------------

    m = SBIT_ABSOLUTE_RE.match(line)

    if m:
        stats.sbit += 1

        return (
            f'{m.group("indent")}'
            f'__sbit __at ({hex8(parse_int(m.group("addr")))}) '
            f'{m.group("name")};'
            f'{m.group("trailer")}'
        )

    # ---------------------------------------------------------
    # sbit = SFR^n
    # ---------------------------------------------------------

    m = SBIT_REGISTER_RE.match(line)

    if m:
        name = m.group("name")
        reg = m.group("reg")
        bit_number = int(m.group("bit"))

        # Numeric base:
        #     sbit LED = 0x90^2;
        if re.fullmatch(NUMBER, reg):
            base = parse_int(reg)

        # Symbolic base:
        #     sbit LED = P1^2;
        elif reg in sfrs:
            base = sfrs[reg]

        else:
            stats.warnings.append(
                f"{location}: cannot resolve SFR '{reg}' "
                f"for sbit '{name}'"
            )

            return (
                f'{m.group("indent")}'
                f'/* KEIL2SDCC TODO: unresolved SFR {reg} */ '
                f'sbit {name} = {reg}^{bit_number};'
                f'{m.group("trailer")}'
            )

        bit_address = base + bit_number

        stats.sbit += 1

        return (
            f'{m.group("indent")}'
            f'__sbit __at ({hex8(bit_address)}) {name};'
            f'{m.group("trailer")}'
        )

    return None


# ---------------------------------------------------------------------------
# Generic keyword conversion
# ---------------------------------------------------------------------------

MEMORY_KEYWORDS = {
    "xdata": "__xdata",
    "idata": "__idata",
    "pdata": "__pdata",
    "data": "__data",
    "code": "__code",
}


def convert_memory_keywords(
    code: str,
    stats: ConversionStats,
) -> str:
    """
    Convert Keil memory-space qualifiers.

    Example:
        unsigned char xdata buffer[10];

    becomes:
        unsigned char __xdata buffer[10];

    SDCC accepts memory-space qualifiers as compiler keywords.
    """

    for keil, sdcc in MEMORY_KEYWORDS.items():
        pattern = re.compile(
            rf"(?<![_A-Za-z0-9]){keil}(?![_A-Za-z0-9])"
        )

        code, count = pattern.subn(sdcc, code)
        stats.memory_keywords += count

    return code


def convert_bit_keyword(
    code: str,
    stats: ConversionStats,
) -> str:
    """
    Keil:
        bit flag;

    SDCC:
        __bit flag;

    Do not touch sbit here; SFR bit declarations are handled earlier.
    """

    pattern = re.compile(
        r"(?<![_A-Za-z0-9])bit(?![_A-Za-z0-9])"
    )

    code, count = pattern.subn("__bit", code)
    stats.bit_keywords += count

    return code


# ---------------------------------------------------------------------------
# Function attributes
# ---------------------------------------------------------------------------

def convert_function_attributes(
    code: str,
    stats: ConversionStats,
) -> str:

    # interrupt N using M
    def interrupt_using(match):
        stats.interrupts += 1
        stats.using += 1

        return (
            f"__interrupt ({match.group(1)}) "
            f"__using ({match.group(2)})"
        )

    code = INTERRUPT_USING_RE.sub(
        interrupt_using,
        code,
    )

    # interrupt N
    def interrupt_only(match):
        stats.interrupts += 1
        return f"__interrupt ({match.group(1)})"

    code = INTERRUPT_RE.sub(
        interrupt_only,
        code,
    )

    # using N without interrupt
    def using_only(match):
        stats.using += 1
        return f"__using ({match.group(1)})"

    code = USING_RE.sub(
        using_only,
        code,
    )

    def reentrant(match):
        stats.reentrant += 1
        return "__reentrant"

    code = REENTRANT_RE.sub(
        reentrant,
        code,
    )

    return code


# ---------------------------------------------------------------------------
# _at_
# ---------------------------------------------------------------------------

def convert_at_attribute(
    code: str,
    stats: ConversionStats,
) -> str:
    """
    Keil:
        unsigned char x _at_ 0x30;

    SDCC placement syntax uses __at(address).

    This conversion preserves its location in the declaration:
        unsigned char x __at (0x30);

    Some unusual declarations may need manual adjustment.
    """

    def repl(match):
        stats.at_attributes += 1

        value = parse_int(match.group(1))

        if value <= 0xFF:
            formatted = hex8(value)
        else:
            formatted = hex16(value)

        return f"__at ({formatted})"

    return AT_RE.sub(repl, code)


# ---------------------------------------------------------------------------
# Keil intrinsic conversion
# ---------------------------------------------------------------------------

INTRINSIC_REPLACEMENTS = [
    (
        re.compile(r"\b_nop_\s*\(\s*\)\s*;"),
        "__asm\n    nop\n__endasm;",
        "_nop_",
    ),
]


def convert_intrinsics(
    code: str,
    stats: ConversionStats,
) -> str:

    for regex, replacement, _name in INTRINSIC_REPLACEMENTS:
        code, count = regex.subn(replacement, code)
        stats.intrinsics += count

    return code


# ---------------------------------------------------------------------------
# Header includes
# ---------------------------------------------------------------------------

INTRINS_INCLUDE_RE = re.compile(
    r'^\s*#\s*include\s*[<"]intrins\.h[>"]\s*$'
)


def convert_include(
    line: str,
    stats: ConversionStats,
) -> str | None:

    if INTRINS_INCLUDE_RE.match(line):
        return (
            "/* Keil <intrins.h> removed by keil2sdcc; "
            "supported intrinsics are converted inline. */"
        )

    return None


# ---------------------------------------------------------------------------
# Unsupported / dangerous Keil features
# ---------------------------------------------------------------------------

WARNING_PATTERNS = [
    (
        re.compile(r"\bbdata\b"),
        "Keil 'bdata' has no simple mechanical SDCC equivalent",
    ),
    (
        re.compile(r"\bcompact\b"),
        "Keil function/memory-model attribute 'compact' needs manual review",
    ),
    (
        re.compile(r"\blarge\b"),
        "Keil function attribute 'large' needs manual review",
    ),
    (
        re.compile(r"\bsmall\b"),
        "Keil function attribute 'small' needs manual review",
    ),
    (
        re.compile(r"\balien\b"),
        "Keil 'alien' calling convention needs manual review",
    ),
    (
        re.compile(r"\b_task_\b"),
        "Keil '_task_' requires manual/RTOS-specific conversion",
    ),
    (
        re.compile(r"\b_crol_\s*\("),
        "Keil _crol_ intrinsic needs manual conversion",
    ),
    (
        re.compile(r"\b_cror_\s*\("),
        "Keil _cror_ intrinsic needs manual conversion",
    ),
    (
        re.compile(r"\b_irol_\s*\("),
        "Keil _irol_ intrinsic needs manual conversion",
    ),
    (
        re.compile(r"\b_iror_\s*\("),
        "Keil _iror_ intrinsic needs manual conversion",
    ),
    (
        re.compile(r"\b_lrol_\s*\("),
        "Keil _lrol_ intrinsic needs manual conversion",
    ),
    (
        re.compile(r"\b_lror_\s*\("),
        "Keil _lror_ intrinsic needs manual conversion",
    ),
    (
        re.compile(r"\b_testbit_\s*\("),
        "Keil _testbit_ intrinsic needs manual conversion",
    ),
]


def detect_unsupported(
    code: str,
    stats: ConversionStats,
    location: str,
) -> None:

    for regex, message in WARNING_PATTERNS:
        if regex.search(code):
            stats.warnings.append(
                f"{location}: {message}"
            )


# ---------------------------------------------------------------------------
# Line conversion
# ---------------------------------------------------------------------------

def convert_line(
    line: str,
    sfrs: dict[str, int],
    stats: ConversionStats,
    location: str,
    in_block_comment: bool,
) -> tuple[str, bool]:

    stripped = line.strip()

    # ---------------------------------------------------------
    # Multiline comments
    # ---------------------------------------------------------

    if in_block_comment:
        if "*/" in line:
            in_block_comment = False

        return line, in_block_comment

    if stripped.startswith("/*"):
        if "*/" not in stripped:
            in_block_comment = True

        return line, in_block_comment

    # ---------------------------------------------------------
    # Includes
    # ---------------------------------------------------------

    converted_include = convert_include(line, stats)

    if converted_include is not None:
        return converted_include, in_block_comment

    # ---------------------------------------------------------
    # sfr / sbit are declaration-specific, so handle before
    # generic keyword substitution.
    # ---------------------------------------------------------

    special = convert_special_register_line(
        line,
        sfrs,
        stats,
        location,
    )

    if special is not None:
        return special, in_block_comment

    # ---------------------------------------------------------
    # Preserve // comments while converting source section
    # ---------------------------------------------------------

    code, comment = split_comment(line)

    detect_unsupported(
        code,
        stats,
        location,
    )

    def convert_code_part(part: str) -> str:
        part = convert_function_attributes(part, stats)
        part = convert_at_attribute(part, stats)
        part = convert_intrinsics(part, stats)
        part = convert_memory_keywords(part, stats)
        part = convert_bit_keyword(part, stats)

        return part

    code = replace_outside_strings(
        code,
        convert_code_part,
    )

    return code + comment, in_block_comment


# ---------------------------------------------------------------------------
# Whole-file conversion
# ---------------------------------------------------------------------------

def convert_text(
    text: str,
    sfrs: dict[str, int],
    stats: ConversionStats,
    filename: Path,
) -> str:

    converted_lines = []
    in_block_comment = False

    # splitlines() preserves a clean conversion model. We'll normalize to LF.
    for line_number, line in enumerate(
        text.splitlines(),
        start=1,
    ):
        location = f"{filename}:{line_number}"

        converted, in_block_comment = convert_line(
            line,
            sfrs,
            stats,
            location,
            in_block_comment,
        )

        converted_lines.append(converted)

    output = "\n".join(converted_lines)

    if text.endswith("\n"):
        output += "\n"

    return output


def convert_file(
    input_file: Path,
    output_file: Path,
    sfrs: dict[str, int],
    stats: ConversionStats,
) -> None:

    text = input_file.read_text(
        encoding="utf-8",
        errors="replace",
    )

    converted = convert_text(
        text,
        sfrs,
        stats,
        input_file,
    )

    output_file.parent.mkdir(
        parents=True,
        exist_ok=True,
    )

    output_file.write_text(
        converted,
        encoding="utf-8",
        newline="\n",
    )

    stats.files_processed += 1


# ---------------------------------------------------------------------------
# Folder conversion
# ---------------------------------------------------------------------------

SOURCE_SUFFIXES = {".c", ".h"}


def convert_folder(
    input_dir: Path,
    output_dir: Path,
    copy_other_files: bool,
) -> ConversionStats:

    if not input_dir.exists():
        raise SystemExit(
            f"Input directory does not exist: {input_dir}"
        )

    if not input_dir.is_dir():
        raise SystemExit(
            f"Input path is not a directory: {input_dir}"
        )

    input_dir = input_dir.resolve()
    output_dir = output_dir.resolve()

    if input_dir == output_dir:
        raise SystemExit(
            "Input and output directories must be different."
        )

    if input_dir in output_dir.parents:
        print(
            "Warning: output directory is inside input directory.\n"
            "The output directory will be skipped during traversal.",
            file=sys.stderr,
        )

    output_dir.mkdir(
        parents=True,
        exist_ok=True,
    )

    stats = ConversionStats()

    print("Scanning SFR declarations...")

    sfrs = collect_global_sfrs(input_dir)

    print(f"Found {len(sfrs)} SFR declaration(s).")
    print()

    paths = list(input_dir.rglob("*"))

    for input_path in paths:
        if not input_path.is_file():
            continue

        # Don't recursively process our generated output.
        try:
            input_path.relative_to(output_dir)
            continue
        except ValueError:
            pass

        relative = input_path.relative_to(input_dir)
        output_path = output_dir / relative

        if input_path.suffix.lower() in SOURCE_SUFFIXES:
            print(f"CONVERT  {relative}")

            convert_file(
                input_path,
                output_path,
                sfrs,
                stats,
            )

        elif copy_other_files:
            print(f"COPY     {relative}")

            output_path.parent.mkdir(
                parents=True,
                exist_ok=True,
            )

            shutil.copy2(
                input_path,
                output_path,
            )

            stats.files_copied += 1

    return stats


# ---------------------------------------------------------------------------
# Report
# ---------------------------------------------------------------------------

def write_report(
    output_dir: Path,
    stats: ConversionStats,
) -> Path:

    report_path = output_dir / "keil2sdcc-report.txt"

    lines = [
        "Keil C51 -> SDCC conversion report",
        "=" * 40,
        "",
        f"Source files converted : {stats.files_processed}",
        f"Other files copied     : {stats.files_copied}",
        "",
        f"sfr                    : {stats.sfr}",
        f"sfr16                  : {stats.sfr16}",
        f"sfr32                  : {stats.sfr32}",
        f"sbit                   : {stats.sbit}",
        f"memory qualifiers      : {stats.memory_keywords}",
        f"bit declarations       : {stats.bit_keywords}",
        f"interrupt attributes   : {stats.interrupts}",
        f"using attributes       : {stats.using}",
        f"reentrant attributes   : {stats.reentrant}",
        f"_at_ attributes        : {stats.at_attributes}",
        f"intrinsics converted   : {stats.intrinsics}",
        "",
    ]

    if stats.warnings:
        lines.extend([
            "MANUAL REVIEW REQUIRED",
            "-" * 40,
            "",
        ])

        for warning in stats.warnings:
            lines.append(warning)

    else:
        lines.append("No unsupported constructs detected.")

    report_path.write_text(
        "\n".join(lines) + "\n",
        encoding="utf-8",
    )

    return report_path


# ---------------------------------------------------------------------------
# CLI
# ---------------------------------------------------------------------------

def main() -> None:
    parser = argparse.ArgumentParser(
        description=(
            "Convert common Keil C51 .c/.h syntax to "
            "SDCC/mcs51 syntax."
        )
    )

    parser.add_argument(
        "input_dir",
        type=Path,
        help="Keil source directory",
    )

    parser.add_argument(
        "output_dir",
        type=Path,
        help="Destination directory",
    )

    parser.add_argument(
        "--no-copy",
        action="store_true",
        help=(
            "Do not copy non-.c/.h files to the "
            "destination directory"
        ),
    )

    args = parser.parse_args()

    stats = convert_folder(
        args.input_dir,
        args.output_dir,
        copy_other_files=not args.no_copy,
    )

    report = write_report(
        args.output_dir.resolve(),
        stats,
    )

    print()
    print("=" * 60)
    print("Conversion complete")
    print("=" * 60)
    print(f"C/H files converted : {stats.files_processed}")
    print(f"Other files copied  : {stats.files_copied}")
    print(f"Warnings            : {len(stats.warnings)}")
    print(f"Report              : {report}")

    if stats.warnings:
        print()
        print(
            "Some constructs require manual review. "
            "See the report."
        )


if __name__ == "__main__":
    main()