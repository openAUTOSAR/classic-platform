
# ARCH defines
ARCH=arm_cm3
ARCH_FAM=arm
ARCH_MCU=arm_cm3

#
# CFG (y/n) macros
# 

CFG=ARM ARM_CM3
# Add our board  
CFG+=BRD_STM32_MCBSTM32 

# 
# ST have devided devices into ( See chapter 6 in Ref manual )
# LD - Low Density.    STM32F101xx,F102xx,F103xx). Flash 16->32Kbytes
# MD - Medium Density. Same as above.              Flash 64->128Kbytes
# HD - High Denstiry.  STM32F101xx,F103xx.         Flash 256->512Kbytes
# CL - Connectivity Line. STM32F105xx,F107xx     
#  
# [ STM32_MD | CFG_STM32_LD | CFG_STM32_HD | CFG_STM32_CL ]  
CFG+=STM32_CL

# What buildable modules does this board have, 
# default or private
MOD_AVAIL=KERNEL MCU ECUM PWM ADC SIMPLE_PRINTF ARM_ITM_TERM RAMLOG DEM IOHWAB

#
# Modules needed by us
#
MOD_USE=KERNEL MCU

#
# Extra defines 
#

# Use little heap
def-y += HEAPSIZE=4000
# Select the right device in ST header files.
# [ STM32F10X_LD | STM32F10X_MD |  STM32F10X_HD |STM32F10X_CL ]
def-y += STM32F10X_CL

