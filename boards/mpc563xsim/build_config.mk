
# ARCH defines
ARCH=mpc55xx
ARCH_FAM=ppc
ARCH_MCU=mpc5633

# CFG (y/n) macros
CFG=PPC BOOKE E200Z3 MPC55XX MPC5633 BRD_MPC5633SIM SPE

# What buildable modules does this board have, 
# default or private
MOD_AVAIL=KERNEL RAMLOG MCU WDG WDGM PORT DIO WDG WDGM PWM CAN CANIF COM ADC DMA

# Needed by us
MOD_USE=KERNEL MCU


