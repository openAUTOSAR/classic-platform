
# ARCH defines
ARCH=armv7_m
ARCH_FAM=arm
ARCH_MCU=s32k144

#
# CFG (y/n) macros
# 
#CFG=ARM ARMV7E_M ARM_CM4 THUMB 
CFG=ARM ARMV7_M ARM_CM3 THUMB 
CFG+=S32K144
CFG+=S32K144EVB_0100
# Add our board  
CFG+=BRD_S32K144EVB_0100

# What buildable modules does this board have, 
# default or private

# MCAL (supplied by NXP)
MOD_AVAIL+=ADC CAN DIO MCU FLS FEE PORT PWM GPT WDG SPI IRQ DMA  ETH FR  I2C ICU IOM LIN PWM STM UART TTY_UART TCPIP LWIP
# Required modules
MOD_USE += KERNEL

#
# Extra defines 
#

COMPILER?=gcc
#COMPILER?=ghs
#COMPILER?=iar

# Default cross compiler
DEFAULT_CROSS_COMPILE = /opt/arm-none-eabi/bin/arm-none-eabi-
DEFAULT_GHS_COMPILE=/c/devtools/ghs/arm/comp_201354
DEFAULT_IAR_COMPILE = /c/devtools/IAR/IAR_Systems/Embedded_Workbench_8_0
