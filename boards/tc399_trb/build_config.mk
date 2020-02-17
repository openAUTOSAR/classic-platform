
# ARCH defines
ARCH=aurix
ARCH_FAM=tricore

#
# CFG (y/n) macros
# 
CFG=TRICORE
CFG+=AURIX
CFG+=BRD_TC399_TB
CFG+=TC39X
CFG+=TC3XX
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

def-y += __TC39XX__

COMPILER=gcc

# For now assume located here:C:\devtools\HIGHTEC\toolchains\tricore\v4.6.6.0-infineon-1.1
DEFAULT_CROSS_COMPILE=/c/devtools/HIGHTEC_4661/toolchains/tricore/v4.6.6.1/bin/tricore-


