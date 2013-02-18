
# ARCH defines
ARCH=mpc55xx
ARCH_FAM=ppc
ARCH_MCU=mpc5554

# CFG (y/n) macros
CFG=PPC BOOKE SPE E200Z6 MPC55XX MPC555X MPC5554 BRD_MPC5554SIM SIMULATOR

# What buildable modules does this board have, 
# default or private

# Memory + Peripherals
MOD_AVAIL+=ADC DIO DMA CAN GPT LIN MCU PORT PWM WDG    
# System + Communication + Diagnostic
MOD_AVAIL+=CANIF CANTP COM DCM DEM DET ECUM IOHWAB KERNEL PDUR WDGM RTE SCHM
# Network management
MOD_AVAIL+=COMM NM CANNM CANSM
# Additional
MOD_AVAIL+=RAMLOG 
# CRC
MOD_AVAIL+=CRC32 CRC16
# Required modules
MOD_USE += MCU KERNEL ECUM

# Default cross compiler
DEFAULT_CROSS_COMPILE = /opt/powerpc-eabispe/bin/powerpc-eabispe-

# Defines
def-y += SRAM_SIZE=0x14000
