
# ARCH defines
ARCH=armv7_m
ARCH_FAM=arm
ARCH_MCU=stm32

#
# CFG (y/n) macros
# 

CFG=ARM ARMV7_M ARM_CM3 THUMB
CFG+=STM32 STM32F1X

# Add our board  
CFG+=BRD_STM32_STM3210C 
CFG+=TIMER_DWT
CFG+=CREATE_SREC

# 
# ST have devided devices into ( See chapter 6 in Ref manual )
# LD - Low Density.    STM32F101xx,F102xx,F103xx). Flash 16->32Kbytes
# MD - Medium Density. Same as above.              Flash 64->128Kbytes
# HD - High Denstiry.  STM32F101xx,F103xx.         Flash 256->512Kbytes
# CL - Connectivity Line. STM32F105xx,F107xx     
#  
# [ STM32_MD | CFG_STM32_LD | CFG_STM32_HD | CFG_STM32_CL ]  
CFG+=STM32_CL

# What buildable modules does this board have, 
# default or private

# MCAL
MOD_AVAIL+=ADC CAN DIO MCU FLS PORT PWM GPT EA
MOD_AVAIL+=TCF LWIP TCPIP SLEEP SOAD DOIP ETHSM
# Required modules
MOD_USE += MCU KERNEL


def-y += NON_ASR_ETH_DRIVER
def-y += USE_NO_ETHIF


#
# Extra defines 
#

# Select the right device in ST header files.
# [ STM32F10X_LD | STM32F10X_MD |  STM32F10X_HD |STM32F10X_CL ]
def-y += STM32F10X_CL


# Default cross compiler
DEFAULT_CROSS_COMPILE = /opt/arm-none-eabi/bin/arm-none-eabi-

def-y += L_BOOT_RESERVED_SPACE=0x0
