
# ARCH defines
ARCH=armv7_m
ARCH_FAM=arm
ARCH_MCU=jacinto

#
# CFG (y/n) macros
# 

CFG=ARM ARMV7_M ARM_CM3 THUMB
CFG+=JACINTO JAC5E
# Add our board  
CFG+=BRD_JACINTO
CFG+=MCU_MMU
CFG+=TIMER_SCTM

def-y += HEAPSIZE=3000

# What buildable modules does this board have, 
# default or private

# MCAL
MOD_AVAIL+= CAN DIO PWM MCU PORT FLS NVM MEMIF FEE FLS GPT ADC SPI
# System + Communication + Diagnostic
MOD_AVAIL+=XCP CANIF CANTP J1939TP COM DCM DEM DET ECUM_FLEXIBLE ECUM_FIXED IOHWAB KERNEL PDUR IPDUM WDGM RTE BSWM WDGIF
MOD_AVAIL+=COMM NM CANNM CANSM
# Additional
MOD_AVAIL+=RAMLOG TCF LWIP TCPIP SLEEP RTE RAMTST
# CRC
MOD_AVAIL+=CRC CPL CAL
# Required modules
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

