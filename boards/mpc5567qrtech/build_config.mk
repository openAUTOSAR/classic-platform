
# ARCH defines
ARCH=mpc55xx
ARCH_FAM=ppc
ARCH_MCU=mpc5567

# CFG (y/n) macros
CFG=PPC BOOKE E200Z6 MPC55XX MPC5567 BRD_MPC5567QRTECH SPE

# What buildable modules does this board have, 
# default or private
MOD_AVAIL=KERNEL MCU WDG WDGM PORT DIO WDG WDGM PWM CAN CANIF COM ADC DMA DEM DCM CANTP PDUR IOHWAB

# Needed by us
MOD_USE=KERNEL MCU


