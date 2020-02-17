
# ARCH defines
ARCH=arm_cr4
ARCH_FAM=arm

#
# CFG (y/n) macros
# 

CFG=ARM ARM_CR5 ARMV7_AR # ARM_CR4 # ARMV7_AR
# Add our board  

CFG+=TMS570 BRD_TMDX570_LC43HDK TMS570LC43X
CFG+=OS_SYSTICK2
CFG+=HW_FLOAT
CFG+=THUMB
CFG+=BIG_ENDIAN
CFG+=TIMER_RTI
CFG += MCU_CACHE 

# 
# ST have devided devices into ( See chapter 6 in Ref manual )
# LD - Low Density.    STM32F101xx,F102xx,F103xx). Flash 16->32Kbytes
# MD - Medium Density. Same as above.              Flash 64->128Kbytes
# HD - High Denstiry.  STM32F101xx,F103xx.         Flash 256->512Kbytes
# CL - Connectivity Line. STM32F105xx,F107xx     
#  
# [ STM32_MD | CFG_STM32_LD | CFG_STM32_HD | CFG_STM32_CL ]  
#CFG+=STM32_CL

# What buildable modules does this board have, 
# default or private

# MCAL
MOD_AVAIL+=ADC CAN DIO MCU FLS FEE PORT PWM GPT WDG SPI
MOD_AVAIL+=TCF LWIP TCPIP SLEEP
# Required modules
MOD_USE += MCU KERNEL

#
# Extra defines 
#

# Select the right device in ST header files.
# [ STM32F10X_LD | STM32F10X_MD |  STM32F10X_HD |STM32F10X_CL ]
#def-y += STM32F10X_CL


# Default cross compiler
DEFAULT_CROSS_COMPILE = /opt/arm-none-eabi/bin/arm-none-eabi-
