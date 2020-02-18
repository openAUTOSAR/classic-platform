
# ARCH defines
ARCH=armv7_ar
ARCH_FAM=arm

#
# CFG (y/n) macros
# 

CFG=ARM ARMV7_AR ARM_GIC ZYNQ
# Add our board  
CFG+=BRD_ZYNQ_ZC702 OS_SYSTICK2

CFG+=TIMER_GLOBAL
CFG+=HW_FLOAT
CFG+=THUMB
CFG+=TIMER
CFG+=MMU_SETUP


# What buildable modules does this board have, 
# default or private

# MCAL
MOD_AVAIL+=ADC CAN DIO MCU FLS FEE PORT PWM GPT ICU WDG SPI ETH LIN
MOD_AVAIL+=TCF LWIP TCPIP SLEEP SOAD DOIP ETHSM
# Required modules
MOD_USE += MCU KERNEL

#
# Extra defines 
#

#def-y += NON_ASR_ETH_DRIVER
#def-y += USE_NO_ETHIF

# Default cross compiler
DEFAULT_CROSS_COMPILE = /opt/arm-none-eabi/bin/arm-none-eabi-
DEFAULT_ARMCC_COMPILE=/c/devtools/DS-5

