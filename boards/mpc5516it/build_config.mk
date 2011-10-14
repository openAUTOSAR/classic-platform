
# ARCH defines
ARCH=mpc55xx
ARCH_FAM=ppc
ARCH_MCU=mpc5516

# CFG (y/n) macros
CFG=PPC BOOKE E200Z1 MPC55XX MPC5516 BRD_MPC5516IT

# What buildable modules does this board have, 
# default or private

# Memory + Peripherals
MOD_AVAIL+=ADC DIO DMA CAN GPT LIN MCU PORT PWM WDG NVM MEMIF FEE FLS SPI EEP 
# System + Communication + Diagnostic
MOD_AVAIL+=CANIF CANTP LINIF COM DCM DEM DET ECUM IOHWAB KERNEL PDUR WDGM RTE
# Network management
MOD_AVAIL+=COMM NM CANNM CANSM EA LINSM
# Additional
MOD_AVAIL+= RAMLOG 

# Required modules
MOD_USE += MCU KERNEL ECUM DET

# Default cross compiler
DEFAULT_CROSS_COMPILE = /opt/powerpc-eabispe/bin/powerpc-eabispe-

