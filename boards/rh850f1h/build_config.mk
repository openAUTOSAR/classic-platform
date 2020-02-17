
# ARCH defines
ARCH=rh850_x
ARCH_FAM=renesas

# CFG (y/n) macros
CFG=RH850 RH850F1H HW_FLOAT OS_SYSTICK2 TIMER TIMER_OSTM
CFG+=CREATE_SREC

# What buildable modules does this board have, 
# default or private

# MCAL
MOD_AVAIL+= CAN MCU PORT DIO GPT FR ETH WDG ADC
# Required modules
MOD_USE += MCU KERNEL

# Defines
#def-y += SRAM_SIZE=0x00020000

# Default cross compiler
COMPILER?=ghs
#DEFAULT_CROSS_COMPILE = /c/devtools/ghs/comp_201355
#DEFAULT_GHS_COMPILE = /c/devtools/ghs/comp_201355
DEFAULT_CROSS_COMPILE = /opt/v850-elf/bin/v850-elf-
DEFAULT_GHS_COMPILE = /c/devtools/ghs/comp_201355

GHS_TARGET?=rh850