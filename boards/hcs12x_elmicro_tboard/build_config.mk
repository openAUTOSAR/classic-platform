
# ARCH defines
ARCH=hcs12d
ARCH_FAM=hc1x
ARCH_MCU=MC912DG128A

# CFG (y/n) macros (These become CFG_XXX=y and def-y += CFG_XXX ) 
CFG=HC1X HCS12XD MC912XDP512 BRD_HCS12X_ELMICRO_TBOARD SIMULATOR

# What buildable modules does this board have, 
# default or private (These become MOD_XXX=y )
MOD_AVAIL+=KERNEL MCU GPT DIO PORT COM CAN CANIF PWM ADC DEM DCM PDUR CANTP RTE IOHWAB DET ECUM RAMLOG SCHM
# Network management
MOD_AVAIL+=COMM NM CANNM CANSM NVM MEMIF EA FEE
# CRC
MOD_AVAIL+=CRC32 CRC16
# Needed by us (These become USE_XXX=y and def-y += USE_XXX )
# Required modules
MOD_USE += MCU KERNEL ECUM


# Default cross compiler
DEFAULT_CROSS_COMPILE = /opt/m6811-elf/bin/m6811-elf-
