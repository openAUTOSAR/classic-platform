
# ARCH defines
ARCH=mpc5xxx
ARCH_FAM=ppc

# CFG (y/n) macros
CFG+=PPC E200Z4 MPC55XX MPC57XX MPC5744P CAN_OSCILLATOR_CLOCK BRD_MPC5744P_MMB
CFG+=EFPU
CFG+=MCU_ARC_CONFIG
CFG+=CREATE_SREC
CFG+=VLE
CFG+=TIMER
CFG+=TIMER_STM
CFG+=MCU_CACHE_I
CFG+=OS_SYSTICK2
CFG+=FCCU
CFG+=MCU_ARC_CLEAR_FCCU

# What buildable modules does this board have, 
# default or private

# Memory + Peripherals
MOD_AVAIL+=ADC DIO DMA CAN GPT LIN MCU PORT PWM WDG FLS SPI FR

# Required modules
MOD_USE += MCU KERNEL

# Default cross compiler
COMPILER?=gcc
COMPILER_FLAVOR=s32_newlib

# Default cross compiler
COMPILER_FLAVOR=s32_newlib
DEFAULT_CROSS_COMPILE = /c/devtools/Freescale/S32DS_Power_v2017.R1/Cross_Tools/powerpc-eabivle-4_9/bin/powerpc-eabivle-
DEFAULT_CW_COMPILE= /c/devtools/Freescale/cw_mpc5xxx_2.10
DEFAULT_DIAB_COMPILE = /c/devtools/WindRiver/diab/5.9.3.0/WIN32
#DEFAULT_DIAB_COMPILE = /c/devtools/WindRiver/diab/5.9.4.8/WIN32
DEFAULT_GHS_COMPILE = /c/devtools/ghs/comp_201314p

vle=$(if $(filter $(CFG),VLE),y)
novle=$(if $(vle),n,y)
efpu=$(if $(filter $(CFG),EFPU),y)
nofpu=$(if $(efpu),n,y)

diab-$(vle)$(nofpu)+=-tPPCE200Z4VFN:simple
diab-$(novle)$(nofpu)+=-tPPCE200Z4NFS:simple
#diab-$(vle)$(efpu)+=-tPPCE200Z4251N3VFF:simple
diab-$(vle)$(efpu)+=-tPPCE200Z4VFF:simple
diab-y+=$(diab-yy)

DIAB_TARGET?=$(diab-y)

# VLE
GHS_TARGET?=ppc5744p

# Defines
def-y += SRAM_SIZE=0x60000
def-y += __BIG_ENDIAN__


