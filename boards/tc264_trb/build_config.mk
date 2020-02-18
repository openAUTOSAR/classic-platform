
# ARCH defines
ARCH=aurix
ARCH_FAM=tricore

#
# CFG (y/n) macros
# 
CFG=TRICORE
CFG+=AURIX
CFG+=BRD_TC264_TRB
CFG+=TC26X
CFG+=TC2XX
CFG+=OS_SYSTICK2
CFG+=TIMER
CFG+=TIMER_AURIX

SELECT_CORE?=0

# What buildable modules does this board have, 
# default or private

# MCAL (supplied by Infineon)
MOD_AVAIL+=CAN DIO MCU FLS PORT DMA GPT GTM
# Required modules
MOD_USE += KERNEL

#
# Extra defines 
#

def-y += __TC29XX__

COMPILER=gcc

DEFAULT_CROSS_COMPILE=/c/devtools/HIGHTEC/toolchains/tricore/v4.9.2.0/bin/tricore-

PATH:=$(PATH):/c/devtools/HIGHTEC/licensemanager
