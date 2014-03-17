
# ARCH defines
ARCH=mpc55xx
ARCH_FAM=ppc
ARCH_MCU=mpc5516

# CFG (y/n) macros
CFG=PPC BOOKE E200Z1 MPC55XX MPC5516 BRD_MPC5516IT
CFG+=MCU_ARC_CONFIG
CFG+=MCU_ARC_LP

# What buildable modules does this board have, 
# default or private

# Memory + Peripherals
MOD_AVAIL+=ADC DIO DMA CAN GPT LIN MCU PORT PWM WDG NVM MEMIF FEE FLS SPI EEP 
# System + Communication + Diagnostic
MOD_AVAIL+=CANIF CANTP LINIF COM DCM DEM DET ECUM IOHWAB KERNEL PDUR WDGM WDGIF RTE J1939TP SCHM
# Network management
MOD_AVAIL+=COMM NM CANNM CANSM EA LINSM
# Additional
MOD_AVAIL+= RAMLOG FLS_SST25XX
# CRC
MOD_AVAIL+=CRC32 CRC16
# Required modules
MOD_USE += MCU KERNEL ECUM

# Default cross compiler
DEFAULT_CROSS_COMPILE = /opt/powerpc-eabispe/bin/powerpc-eabispe-
DEFAULT_DIAB_COMPILE = /c/devtools/WindRiver/diab/5.9.0.0/WIN32

vle=$(if $(filter $(CFG),VLE),y)
novle=$(if $(vle),n,y)

diab-$(vle)=-tPPCE200Z1VFN:simple
diab-$(novle)=-tPPCE200Z1NFS:simple

DIAB_TARGET?=$(diab-y)

# Defines
def-y += SRAM_SIZE=0x14000
