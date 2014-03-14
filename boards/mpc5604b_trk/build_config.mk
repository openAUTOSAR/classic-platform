
# ARCH defines
ARCH=mpc55xx
ARCH_FAM=ppc
ARCH_MCU=mpc5604b

# CFG (y/n) macros
CFG=PPC E200Z0 MPC55XX MPC560X MPC560XB MPC5604B BRD_MPC5604B_TRK VLE
CFG+=MCU_ARC_LP

# What buildable modules does this board have, 
# default or private

# Memory + Peripherals
MOD_AVAIL+=ADC DIO DMA CAN GPT LIN MCU PORT PWM WDG NVM MEMIF FEE FLS SPI EEP EA
# System + Communication + Diagnostic
MOD_AVAIL+=LINIF CANIF CANTP COM DCM DEM DET ECUM IOHWAB KERNEL PDUR WDGM WDGIF RTE J1939TP SCHM
# Network management
MOD_AVAIL+=COMM NM CANNM CANSM LINSM
# Additional
MOD_AVAIL+= RAMLOG 
# CRC
MOD_AVAIL+=CRC32 CRC16
# Required modules
MOD_USE += MCU KERNEL ECUM

# Default cross compiler
COMPILER?=cw
DEFAULT_CROSS_COMPILE = /opt/powerpc-eabispe/bin/powerpc-eabispe-
DEFAULT_CW_COMPILE= /c/devtools/Freescale/cw_mpc5xxx_2.10

# Defines
def-y += SRAM_SIZE=0xc000


