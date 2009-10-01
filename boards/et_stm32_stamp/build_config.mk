
# ARCH defines
ARCH=arm_cm3
ARCH_FAM=arm
ARCH_MCU=arm_cm3

# CFG (y/n) macros
CFG=ARM ARM_CM3 BRD_ET_STM32_STAMP

# What buildable modules does this board have, 
# default or private
MOD_AVAIL=KERNEL MCU SIMPLE_PRINTF ARM_ITM_TERM RAMLOG
#T32_TERM SIMPLE_PRINTF RAMLOG
#

# Needed by us
MOD_USE=KERNEL MCU

# Use little head
def-y += HEAPSIZE=400
