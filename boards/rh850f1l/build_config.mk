
# ARCH defines
ARCH=rh850_x
ARCH_FAM=renesas

# CFG (y/n) macros
CFG=RH850 RH850F1L OS_SYSTICK2 TIMER TIMER_OSTM
CFG+=CREATE_SREC

# Set RF7010132 (64pin) as default device
RH850_PARTNUM?=RF7010132

# Supported RH850F1L_PARTNUM below
ifeq ($(RH850_PARTNUM),RF7010132)
def-y += L_FLASH_CODE_0_LENGTH=0x80000 
endif

ifeq ($(RH850_PARTNUM),RF7010243)
def-y += L_FLASH_CODE_0_LENGTH=0xc0000
endif

# What buildable modules does this board have, 
# default or private

# MCAL
MOD_AVAIL+= CAN MCU PORT DIO GPT FR ETH WDG ADC SPI EEP PWM
# Required modules
MOD_USE += MCU KERNEL

# Defines
#def-y += SRAM_SIZE=0x00020000

# Default cross compiler
COMPILER?=ghs
#DEFAULT_CROSS_COMPILE = /c/devtools/ghs/comp_201355
#DEFAULT_GHS_COMPILE = /c/devtools/ghs/comp_201355
DEFAULT_CROSS_COMPILE = /opt/v850-elf/bin/v850-elf-
#DEFAULT_GHS_COMPILE = /c/devtools/ghs/comp_201355
DEFAULT_GHS_COMPILE = /c/devtools/ghs/v800/comp_201355

GHS_TARGET?=rh850