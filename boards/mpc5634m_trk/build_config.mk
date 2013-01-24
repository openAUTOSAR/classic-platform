
# ARCH defines
ARCH=mpc55xx
ARCH_FAM=ppc
ARCH_MCU=mpc5634m

# CFG (y/n) macros
CFG=PPC E200Z0 MPC55XX MPC563XM MPC5634M BRD_MPC5634M_TRK SPE

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
MOD_USE += MCU KERNEL ECUM DET

# Default cross compiler
COMPILER?=gcc
DEFAULT_CROSS_COMPILE = /opt/powerpc-eabispe/bin/powerpc-eabispe-
DEFAULT_CW_COMPILE= /opt/cw
DEFAULT_DIAB_COMPILE= /opt/diab

# SPE Y - Float and Double using single HW 
# SPE N - Single float 
# 
# VLE SPE
#  N   N   ->  -tPPCE200Z3NFS:simple  (2,156,2,1,simple) 
#  N   Y   ->  -tPPCE200Z3NFG:simple 
#  Y   N   ->  -tPPCE200Z3VFS:simple  (4,39,2,1,simple ) 
#  Y   Y   ->  -tPPCE200Z3VFG:simple  (4,39,2,3,simple )
vle=$(if $(filter $(CFG),VLE),y)
novle=$(if $(vle),n,y)
spe=$(if $(filter $(CFG),SPE),y)
nospe=$(if $(spe),n,y)

diab-$(novle)$(nospe)=-tPPCE200Z3NFS:simple
diab-$(novle)$(spe)=-tPPCE200Z3NFG:simple
diab-$(vle)$(nospe)=-tPPCE200Z3VFS:simple
diab-$(vle)$(spe)=-tPPCE200Z3VFG:simple

DIAB_TARGET?=$(diab-yy)

# Defines
def-y += SRAM_SIZE=0x17800


