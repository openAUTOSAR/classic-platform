
# ARCH defines
ARCH=arm_cr4
ARCH_FAM=arm

#
# CFG (y/n) macros
# 

CFG+=ARM ARM_CR4 ARMV7_AR
# Add our board  

CFG+=TMS570 BRD_TMDX570_LS12HDK TMS570LS12X
CFG+=OS_SYSTICK2
CFG+=HW_FLOAT
CFG+=THUMB
CFG+=BIG_ENDIAN
CFG+=TIMER_RTI

# What buildable modules does this board have, 
# default or private

# MCAL
MOD_AVAIL+=ADC CAN DIO MCU FLS FEE PORT PWM GPT WDG SPI LIN CAN_DCAN
MOD_AVAIL+=TCF LWIP TCPIP SLEEP
# Required modules
MOD_USE += MCU KERNEL

# Default cross compiler
DEFAULT_CROSS_COMPILE = /opt/arm-none-eabi/bin/arm-none-eabi-
