#ifndef __COMPARATOR_H
#define __COMPARATOR_H

#include "config.h"

/* CMPCR1 fields. */
#define CMP_ENABLE_MASK             0x80
#define CMP_INTERRUPT_FLAG_MASK     0x40
#define CMP_INTERRUPT_RISING        0x20
#define CMP_INTERRUPT_FALLING       0x10
#define CMP_INTERRUPT_BOTH          0x30
#define CMP_INTERRUPT_NONE          0x00
#define CMP_POSITIVE_P55            0x00
#define CMP_POSITIVE_ADC            0x08
#define CMP_NEGATIVE_BANDGAP        0x00
#define CMP_NEGATIVE_P54            0x04
#define CMP_OUTPUT_DISABLE          0x00
#define CMP_OUTPUT_ENABLE           0x02
#define CMP_RESULT_MASK             0x01

/* CMPCR2 fields. */
#define CMP_OUTPUT_NORMAL           0x00
#define CMP_OUTPUT_INVERTED         0x80
#define CMP_FILTER_ENABLE           0x00
#define CMP_FILTER_DISABLE          0x40
#define CMP_DEBOUNCE_MASK           0x3f

/*
 * Configure the comparator directly.
 *
 * POSITIVE_INPUT:
 *   CMP_POSITIVE_P55 selects the dedicated CMP+ input on P5.5.
 *   CMP_POSITIVE_ADC selects the ADC input named by ADC_CONTR.CHS[2:0].
 * NEGATIVE_INPUT:
 *   CMP_NEGATIVE_BANDGAP selects the internal band-gap reference.
 *   CMP_NEGATIVE_P54 selects the dedicated CMP- input on P5.4.
 * EDGE: CMP_INTERRUPT_NONE, CMP_INTERRUPT_RISING,
 *   CMP_INTERRUPT_FALLING, or CMP_INTERRUPT_BOTH.
 * PIN_OUTPUT: CMP_OUTPUT_DISABLE or CMP_OUTPUT_ENABLE for output on P1.2.
 * POLARITY: CMP_OUTPUT_NORMAL or CMP_OUTPUT_INVERTED.
 * FILTER: CMP_FILTER_ENABLE enables the approximately 0.1 us hardware
 *   filter; CMP_FILTER_DISABLE bypasses it. The register control is
 *   active-low, which is reflected in these names.
 * DEBOUNCE_CLOCKS: 0 outputs immediately; 1 through 63 require the result
 *   to remain stable for that many system clocks. Only the low six bits are
 *   used.
 *
 * This resets a pending interrupt flag and enables the comparator. Enable EA
 * separately when EDGE requests interrupts. All arguments must be
 * side-effect-free.
 */
#define CMP_INIT(POSITIVE_INPUT, NEGATIVE_INPUT, EDGE, PIN_OUTPUT, POLARITY, \
		 FILTER, DEBOUNCE_CLOCKS) do { \
	CMPCR2 = (POLARITY) | (FILTER) | \
		((DEBOUNCE_CLOCKS) & CMP_DEBOUNCE_MASK); \
	CMPCR1 = CMP_ENABLE_MASK | (EDGE) | (POSITIVE_INPUT) | \
		(NEGATIVE_INPUT) | (PIN_OUTPUT); \
} while (0)

/* Enable or disable the comparator without changing its configuration. */
#define CMP_ENABLE() do { \
	CMPCR1 |= CMP_ENABLE_MASK; \
} while (0)
#define CMP_DISABLE() do { \
	CMPCR1 &= ~CMP_ENABLE_MASK; \
} while (0)

/* EDGE: any CMP_INTERRUPT_* value. Clears any previously selected edges. */
#define CMP_SET_INTERRUPT_EDGE(EDGE) do { \
	CMPCR1 = (CMPCR1 & ~CMP_INTERRUPT_BOTH) | (EDGE); \
} while (0)

/* CMPIF is set by hardware and must be cleared by software. */
#define CMP_INTERRUPT_PENDING() \
	((CMPCR1 & CMP_INTERRUPT_FLAG_MASK) != 0)
#define CMP_CLEAR_INTERRUPT_FLAG() do { \
	CMPCR1 &= ~CMP_INTERRUPT_FLAG_MASK; \
} while (0)

/* Nonzero when the comparator's positive input is above its negative input. */
#define CMP_RESULT() ((CMPCR1 & CMP_RESULT_MASK) != 0)

/* Select ADC channel 0 through 7 when CMP_POSITIVE_ADC is configured. */
#define CMP_SELECT_ADC_CHANNEL(CHANNEL) do { \
	ADC_CONTR = (ADC_CONTR & ~0x07) | ((CHANNEL) & 0x07); \
} while (0)

#endif
