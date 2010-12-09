
# ARCH defines
ARCH=mpc55xx
ARCH_FAM=ppc
ARCH_MCU=mpc5633

# CFG (y/n) macros
CFG=PPC BOOKE E200Z3 MPC55XX MPC5633 BRD_MPC5633SIM SPE SIMULATOR

# What buildable modules does this board have, 
# default or private

# Memory + Peripherals
MOD_AVAIL+=ADC DIO DMA CAN GPT LIN MCU PORT PWM WDG    
# System + Communication + Diagnostic
MOD_AVAIL+=CANIF CANTP COM DCM DEM DET ECUM IOHWAB KERNEL PDUR WDGM RTE
# Additional
MOD_AVAIL+=RAMLOG 

