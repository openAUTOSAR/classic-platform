
# ARCH defines
ARCH=aurix
ARCH_FAM=tricore

#
# CFG (y/n) macros
# 
CFG+=TRICORE
CFG+=AURIX
CFG+=BRD_TC297_TB
CFG+=TC29X
CFG+=TC2XX
CFG+=OS_SYSTICK2
CFG+=TIMER
CFG+=TIMER_AURIX

SELECT_CORE?=0

# What buildable modules does this board have, 
# default or private

# MCAL (supplied by Infineon)
MOD_AVAIL+=ADC CAN DIO MCU FLS FEE PORT PWM GPT WDG SPI IRQ DMA DSADC ETH FR HSSL I2C ICU IOM LIN MSC PWM SENT SMU STM UART TTY_UART TCPIP LWIP
# Required modules
MOD_USE += KERNEL

#
# Extra defines 
#

def-y += __TC29XX__

COMPILER=gcc

# For now assume located here:C:\HIGHTEC\toolchains\tricore\v4.6.5.0-infineon-1.1
#DEFAULT_CROSS_COMPILE=/c/devtools/HIGHTEC/toolchains/tricore/v4.6.5.0/bin/tricore-
#DEFAULT_CROSS_COMPILE=/c/HIGHTEC/toolchains/tricore/v4.6.5.0-infineon-1.1/bin/tricore-
#DEFAULT_CROSS_COMPILE=/c/devtools/HIGHTEC/toolchains/tricore/v4.6.5.0-infineon-1.1/bin/tricore-
DEFAULT_CROSS_COMPILE=/c/devtools/HIGHTEC/toolchains/tricore/v4.6.5.0/bin/tricore-
#$(error $(PATH))
#PATH:=$(PATH):/c/devtools/HIGHTEC/licensemanager

# Default cross compiler
#DEFAULT_CROSS_COMPILE = /opt/arm-none-eabi/bin/arm-none-eabi-
