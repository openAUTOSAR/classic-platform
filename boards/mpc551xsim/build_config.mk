
# ARCH defines
ARCH=mpc55xx
ARCH_FAM=ppc
ARCH_MCU=mpc5516

# CFG (y/n) macros
CFG=PPC BOOKE E200Z1 MPC55XX MPC5516 BRD_MPC551XSIM SIMULATOR

# What buildable modules does this board have, 
# default or private

# Memory + Peripherals
MOD_AVAIL+=ADC DIO DMA CAN GPT LIN MCU PORT PWM WDG    
# System + Communication + Diagnostic
MOD_AVAIL+=CANIF CANTP COM DCM DEM DET ECUM IOHWAB KERNEL PDUR WDGM RTE KERNEL_EXTRA SCHM
# Network management
MOD_AVAIL+=COMM NM CANNM CANSM
# Additional
MOD_AVAIL+=RAMLOG KERNEL_EXTRA CRC32

# Required modules
MOD_USE += MCU KERNEL ECUM

# Default cross compiler
DEFAULT_CROSS_COMPILE = /opt/powerpc-eabispe/bin/powerpc-eabispe-

# Defines
def-y += SRAM_SIZE=0x14000
