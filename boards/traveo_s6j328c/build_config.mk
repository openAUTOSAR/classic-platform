
# ARCH defines
ARCH=arm_cr4
ARCH_FAM=arm
#
# CFG (y/n) macros
# 

CFG=ARM ARM_CR4 ARM_CR5 ARMV7_AR
# Add our board  

CFG+=TRAVEO TRAVEO_S6J328C
CFG+=OS_SYSTICK2
CFG+=HW_FLOAT
CFG+=THUMB
CFG+=LITTLE_ENDIAN
CFG+=MCAL_CYPRESS
CFG+=MCAL_EXTERNAL

# What buildable modules does this board have, 
# default or private

# MCAL
MOD_AVAIL+=ADC CAN DIO FLS GPT MCU PORT PWM SPI WDG 

# Required modules
MOD_USE += MCU KERNEL

GHS_TARGET=cortexr4f

# Default cross compiler
DEFAULT_CROSS_COMPILE = /opt/arm-none-eabi/bin/arm-none-eabi-
GHS_COMPILE=/c/devtools/ghs/arm/comp_201354

