
# ARCH defines
ARCH=hcs12d
ARCH_FAM=hc1x
ARCH_MCU=MC912DG128A

# CFG (y/n) macros (These become CFG_XXX=y and def-y += CFG_XXX ) 
CFG=HC1X HCS12D MC912DG128A BRD_HCS12_ELMICRO_CARD12 SIMULATOR

# What buildable modules does this board have, 
# default or private (These become MOD_XXX=y )
MOD_AVAIL+=MCU GPT PORT DIO CAN ADC PWM  
# System + Communication + Diagnostic
MOD_AVAIL+=CANIF CANTP COM DCM DEM DET ECUM IOHWAB KERNEL PDUR WDGM RTE EA SCHM EEP NVM MEMIF
# Network management
MOD_AVAIL+=COMM NM CANNM CANSM
# Additional
MOD_AVAIL+=RAMLOG
# CRC
MOD_AVAIL+=CRC32 CRC16

# Required modules
MOD_USE += MCU KERNEL ECUM


# Needed by us (These become USE_XXX=y and def-y += USE_XXX )
MOD_USE=KERNEL MCU

# Default cross compiler
DEFAULT_CROSS_COMPILE = /opt/m6811-elf/bin/m6811-elf-

