
# ARCH defines
ARCH=armv7_m
ARCH_FAM=arm
ARCH_MCU=jacinto

#
# CFG (y/n) macros
# 

CFG=ARM ARMV7_M ARM_CM3 THUMB
CFG+=JACINTO JAC5
# Add our board  
CFG+=MCU_MMU
CFG+=BRD_JACINTO

def-y += HEAPSIZE=3000

# What buildable modules does this board have, 
# default or private

# MCAL
MOD_AVAIL+= CAN DIO MCU PORT FLS SPI PWM GPT
# System + Communication + Diagnostic
MOD_AVAIL+=COMM NM CANNM CANSM
# Additional
MOD_AVAIL+=TCF LWIP TCPIP SLEEP

ifndef NOKERNEL
MOD_USE += MCU KERNEL
endif

# Defines
def-y += SRAM_SIZE=0x20000
ifndef NOKERNEL
def-y += L_BOOT_RESERVED_SPACE=0x20000
endif

# Default cross compiler
DEFAULT_CROSS_COMPILE = /opt/arm-none-eabi/bin/arm-none-eabi-

