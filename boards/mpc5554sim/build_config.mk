
# ARCH defines
ARCH=mpc55xx
ARCH_FAM=ppc
ARCH_MCU=mpc5554

# CFG (y/n) macros
CFG=PPC BOOKE SPE E200Z6 MPC55XX MPC5554 BRD_MPC5554SIM SIMULATOR

# What buildable modules does this board have, 
# default or private

# Memory + Peripherals
MOD_AVAIL+=ADC DIO DMA CAN GPT LIN MCU PORT PWM WDG    
# System + Communication + Diagnostic
MOD_AVAIL+=CANIF CANTP COM DCM DEM DET ECUM IOHWAB KERNEL PDUR WDGM RTE
# Additional
MOD_AVAIL+=RAMLOG 
