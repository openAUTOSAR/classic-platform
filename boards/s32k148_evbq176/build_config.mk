
# ARCH defines
ARCH=armv7_m
ARCH_FAM=arm
ARCH_MCU=s32k148

#
# CFG (y/n) macros
# 
#CFG=ARM ARMV7E_M ARM_CM4 THUMB 
CFG=ARM ARMV7_M ARM_CM3 THUMB 
CFG+=S32K148

# Add our board  
CFG+=BRD_S32K148_EVBQ176

# What buildable modules does this board have, 
# default or private

# MCAL (supplied by NXP)


MOD_AVAIL+=MCU CAN DIO PORT LIN SPI GPT ETH MCL ADC LWIP TCPIP

# Required modules
MOD_USE += KERNEL

#
# Extra defines 
#

# Default cross compiler
COMPILER?=gcc
# Default cross compiler
#COMPILER_FLAVOR=s32_newlib
#DEFAULT_CROSS_COMPILE = /c/NXP/S32DS_ARM_v2018.R1/Cross_Tools/gcc-arm-none-eabi-4_9/bin/arm-none-eabi-
DEFAULT_CROSS_COMPILE = /opt/arm-none-eabi/bin/arm-none-eabi-