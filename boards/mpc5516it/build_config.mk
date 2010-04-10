
# ARCH defines
ARCH=mpc55xx
ARCH_FAM=ppc
ARCH_MCU=mpc5516

# CFG (y/n) macros
CFG=PPC BOOKE E200Z1 MPC55XX MPC5516 BRD_MPC5516IT

# What buildable modules does this board have, 
# default or private
MOD_AVAIL+=KERNEL RAMLOG MCU GPT LIN CAN CANIF PORT DIO WDG WDGM PWM COM ADC DMA DEM PDUR IOHWAB

# Needed by us
MOD_USE=KERNEL MCU
