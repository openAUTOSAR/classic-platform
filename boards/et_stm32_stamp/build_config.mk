
# ARCH defines
ARCH=arm_cm3
ARCH_FAM=arm
ARCH_MCU=arm_cm3

# CFG (y/n) macros
CFG=ARM ARM_CM3 BRD_ET_STM32_STAMP


# What buildable modules does this board have, 
# default or private
MOD_AVAIL=KERNEL MCU T32_TERM

# Needed by us
MOD_USE=KERNEL MCU
