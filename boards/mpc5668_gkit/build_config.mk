
# ARCH defines
ARCH=mpc55xx
ARCH_FAM=ppc
ARCH_MCU=mpc5568

# CFG (y/n) macros
CFG=PPC BOOKE E200Z1 MPC55XX MPC5668 MPC5668G BRD_MPC5668_GKIT SPE
CFG+=MCU_ARC_LP
CFG+=MCU_ARC_CONFIG

# What buildable modules does this board have, 
# default or private

# Memory + Peripherals
MOD_AVAIL+=ADC DIO DMA CAN GPT LIN MCU PORT PWM WDG NVM MEMIF FEE FLS SPI EEP 
# System + Communication + Diagnostic
MOD_AVAIL+=CANIF CANTP COM DCM DEM DET ECUM IOHWAB KERNEL PDUR WDGM RTE SCHM WDGIF WDGM
# Network management
MOD_AVAIL+=COMM NM CANNM CANSM EA  
# Additional
MOD_AVAIL+= RAMLOG TTY_T32 BOOT
# CRC
MOD_AVAIL+=CRC32 CRC16
# Required modules
MOD_USE += MCU KERNEL ECUM DET

# Default cross compiler
DEFAULT_CROSS_COMPILE = /opt/powerpc-eabispe/bin/powerpc-eabispe-

# Defines (can be 0x94000 if MMU is setup for more that 256K)
def-y += SRAM_SIZE=0x40000
