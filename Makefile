# SPDX-License-Identifier: BSD-2-Clause
# 
# Copyright (c) 2022 Vincent DEFERT. All rights reserved.
# 
# Redistribution and use in source and binary forms, with or without 
# modification, are permitted provided that the following conditions 
# are met:
# 
# 1. Redistributions of source code must retain the above copyright 
# notice, this list of conditions and the following disclaimer.
# 
# 2. Redistributions in binary form must reproduce the above copyright 
# notice, this list of conditions and the following disclaimer in the 
# documentation and/or other materials provided with the distribution.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE 
# COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN 
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
# POSSIBILITY OF SUCH DAMAGE.

# Prerequisites --------------------------------------------------------
#
# Besides make, his project requires: 
#
# - sdcc
# - stcgal-patched
# - minicom
# - doxygen

# Usage ----------------------------------------------------------------
#
# Build executable in release mode:
#   make
#
# Build executable in debug mode:
#   make BUILD_MODE=debug
#
# Build documentation:
#   make doc
#
# Upload executable to MCU:
#   make upload
#
# Open serial console in new window:
#   make console
#
# Clean project (remove all build files):
#   make clean

# Target MCU settings --------------------------------------------------

# Note: using a system clock around 24MHz works with all MCU
# having an internal RC oscillator.
# MCU_FREQ_KHZ := 24000
# STACK_SIZE := 112
# # Tested on STC8A8K64D4-45I-LQFP48
# XRAM_SIZE := 8192
# FLASH_SIZE := 65024
# MEMORY_MODEL := --model-medium
# HAS_DUAL_DPTR := y

# # Define UNISTC_DIR, HAL_DIR, DRIVER_DIR, and MAKE_DIR -----------------
# UNISTC_ROOT_DIR := $(dir $(abspath $(dir $(lastword $(MAKEFILE_LIST)))))
# UNISTC_DIR := $(UNISTC_ROOT_DIR)include
# HAL_DIR := $(UNISTC_ROOT_DIR)hal
# DRIVER_DIR := $(UNISTC_ROOT_DIR)drivers
# MAKE_DIR := $(UNISTC_ROOT_DIR)makefiles

SED_VERSION := $(shell sed --version 2> /dev/null | grep -F 'GNU sed' | head -1)

ifeq ($(SED_VERSION),)
	# BSD sed (e.g. MacOS, FreeBSD)
	SED_CMD := sed -i ''
else
	# GNU sed (Linux, Windows)
	SED_CMD := sed -i
endif

# # Project settings -----------------------------------------------------
# PROJECT_NAME := test

# SRCS := \
# 	$(HAL_DIR)/delay.c \
# 	$(HAL_DIR)/gpio-hal.c \
# 	$(HAL_DIR)/adc-comp-hal.c \
# 	$(HAL_DIR)/serial-console.c \
# 	$(HAL_DIR)/timer-hal.c \
# 	$(HAL_DIR)/fifo-buffer.c \
# 	$(HAL_DIR)/uart-hal.c \
# 	main.c

# CONSOLE_BAUDRATE := 57600
# CONSOLE_PORT := /dev/ttyUSB0

# ISP_PORT := /dev/ttyUSB0

# # Boilerplate rules ----------------------------------------------------
# ifeq ($(PROJECT_NAME),)
# $(error The PROJECT_NAME variable must be defined)
# endif

# ifeq ($(SRCS),)
# $(error The SRCS variable must be defined)
# endif

# ifeq ($(MCU_FREQ_KHZ),)
# $(error The MCU_FREQ_KHZ variable must be defined)
# endif

# ifeq ($(HAS_DUAL_DPTR),)
# $(error The HAS_DUAL_DPTR variable must be defined)
# endif

# ifeq ($(MEMORY_MODEL),)
# $(error The MEMORY_MODEL variable must be defined)
# endif

# ifeq ($(FLASH_SIZE),)
# $(error The FLASH_SIZE variable must be defined)
# endif

# ifeq ($(IRAM_SIZE),)
# IRAM_SIZE := 256
# endif

# ifeq ($(XRAM_SIZE),)
# $(error The XRAM_SIZE variable must be defined)
# endif

# ifeq ($(STACK_SIZE),)
# $(error The STACK_SIZE variable must be defined)
# endif

# ifeq ($(DRIVER_DIR),)
# $(error The DRIVER_DIR variable must be defined)
# endif

# ifeq ($(HAL_DIR),)
# $(error The HAL_DIR variable must be defined)
# endif

# ifeq ($(UNISTC_DIR),)
# $(error The UNISTC_DIR variable must be defined)
# endif

# ifeq ($(ISP_PORT),)
# $(error The ISP_PORT variable must be defined)
# endif

# ifeq ($(CONSOLE_PORT),)
# 	CONSOLE_PORT := $(ISP_PORT)
# endif

# ifeq ($(CONSOLE_BAUDRATE),)
# 	CONSOLE_BAUDRATE := 115200
# endif

ifeq ($(BUILD_ROOT),)
	BUILD_ROOT := build
endif

# Optional variables you can define in addition to the required listed 
# above:
#
# - PROJECT_FLAGS if you need specific build flags for your application.
# - EEPROM_SRC if you want to upload data to the MCU's EEPROM.

# Toolchain settings ---------------------------------------------------
PROJECT_NAME := stc15-template
VENDER_DIR := lib
SRC_DIR := src

HAS_DUAL_DPTR := y
TARGET_ARCH := -mmcs51
MEMORY_MODEL := --model-medium
IRAM_SIZE := 256
XRAM_SIZE := 256
STACK_SIZE := 112
FLASH_SIZE := 8192
ISP_FREQUENCY := 32000

AS := sdas8051
CC := sdcc
ASFLAGS := -plosgffw
CPPFLAGS := -I. -I./$(VENDER_DIR) -I./$(SRC_DIR)
CFLAGS := $(TARGET_ARCH) $(MEMORY_MODEL) $(CPPFLAGS) --opt-code-size
LDFLAGS := $(TARGET_ARCH) $(MEMORY_MODEL) \
	--iram-size $(IRAM_SIZE) \
	--xram-loc 0 \
	--xram-size $(XRAM_SIZE) \
	--stack-size $(STACK_SIZE) \
	--code-size $(FLASH_SIZE)

# Because we can't use debug mode, the extra directory level is useless.
# OBJDIR := $(BUILD_ROOT)/$(BUILD_DIR)
OBJDIR := $(BUILD_ROOT)

IHX_FILE := $(OBJDIR)/$(PROJECT_NAME).ihx
HEX_FILE := $(OBJDIR)/$(PROJECT_NAME).hex
DEP_FILE := $(OBJDIR)/dependencies.mk

SRCS := \
	$(SRC_DIR)/main.c

VENDER_SRCS := $(filter $(VENDER_DIR)%, $(SRCS))
VENDER_OBJS := $(subst $(VENDER_DIR),$(OBJDIR),$(subst .c,.rel,$(VENDER_SRCS)))

LOCAL_SRCS := $(filter-out $(VENDER_DIR)%, $(SRCS))
LOCAL_OBJS := $(addprefix $(OBJDIR)/,$(subst .c,.rel,$(LOCAL_SRCS)))

OBJDIR_TREE := $(sort $(dir $(DRIVER_OBJS) $(HAL_OBJS) $(LOCAL_OBJS)))

# ifeq ($(HAS_DUAL_DPTR),y)
# 	DUAL_DPTR_SUPPORT := $(OBJDIR)/crtxinit.rel
# else
# 	DUAL_DPTR_SUPPORT := 
# endif


# Rules ----------------------------------------------------------------

# .PHONY: all clean doc upload console
.PHONY: all clean download

all: $(DEP_FILE) $(HEX_FILE)


clean:
	@rm -rf $(BUILD_ROOT)

# doxygen.conf is per-project.
# doc:
# 	doxygen doxygen.conf

download:
	uv run stcgal -t $(ISP_FREQUENCY) $(HEX_FILE);

# console:
# 	# TODO: Replace with your favorite terminal emulator
# 	mate-terminal -t "$(PROJECT_NAME) console" -e "minicom -b $(CONSOLE_BAUDRATE) -D $(CONSOLE_PORT)"

# SDCC's dependencies generation is buggy and doesn't match the GCC
# documentation (which the SDCC manual says should apply), so we 
# need to compensate for this.
$(DEP_FILE):
	@mkdir -p $(OBJDIR_TREE)
	@rm -f $(DEP_FILE)
	@for srcFile in $(LOCAL_SRCS) $(VENDER_SRCS); do $(CC) $(CPPFLAGS) -MM $${srcFile} >> $(DEP_FILE); echo '' >> $(DEP_FILE); done
	@$(SED_CMD) "s/^\(.*\.rel:.*\)/$(BUILD_ROOT)\/\1/g" $(DEP_FILE)


# -------------------------------------------

-include $(DEP_FILE)

# -------------------------------------------

$(HEX_FILE): $(IHX_FILE)
	packihx $< > $@

$(IHX_FILE): $(VENDER_OBJS) $(LOCAL_OBJS) $(DUAL_DPTR_SUPPORT)
	$(CC) $(LDFLAGS) -o $@ $^

$(OBJDIR)/%.rel: $(VENDER_DIR)/%.asm
	$(AS) $(ASFLAGS) $@ $<

$(OBJDIR)/%.rel: %.asm
	$(AS) $(ASFLAGS) $@ $<

$(OBJDIR)/%.rel: $(VENDER_DIR)/%.c
	$(CC) $(CFLAGS) -o $@ -c $<

$(OBJDIR)/%.rel: %.c
	$(CC) $(CFLAGS) -o $@ -c $<


