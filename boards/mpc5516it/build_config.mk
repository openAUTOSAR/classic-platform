
# ARCH defines
ARCH=mpc5xxx
ARCH_FAM=ppc

# CFG (y/n) macros
CFG=PPC BOOKE E200Z1 MPC55XX MPC5516 BRD_MPC5516IT TIMER_TB
CFG+=MCU_ARC_CONFIG MCU_MMU MCU_ARC_LP
CFG+=CREATE_SREC
CFG+=VLE

# MCAL
MOD_AVAIL+=ADC DIO DMA CAN GPT LIN MCU PORT PWM WDG SPI
# Extra
MOD_AVAIL+=FLS_SST25XX
# Required modules
MOD_USE += MCU KERNEL

COMPILER?=gcc
COMPILER_FLAVOR=s32_newlib

# Default cross compiler
DEFAULT_CROSS_COMPILE = /c/devtools/Freescale/S32DS_Power_v2017.R1/Cross_Tools/powerpc-eabivle-4_9/bin/powerpc-eabivle-
DEFAULT_DIAB_COMPILE = /c/devtools/WindRiver/diab/5.9.3.0/WIN32
DEFAULT_GHS_COMPILE = /c/devtools/ghs/comp_201314p
DEFAULT_CW_COMPILE= /c/devtools/Freescale/cw_mpc5xxx_2.10

vle=$(if $(filter $(CFG),VLE),y)
novle=$(if $(vle),n,y)

diab-$(vle)=-tPPCE200Z1VFN:simple
diab-$(novle)=-tPPCE200Z1NFS:simple
DIAB_TARGET?=$(diab-y)

# VLE
GHS_TARGET?=ppc5516

# Defines
def-y += SRAM_SIZE=0x14000
def-y += L_BOOT_RESERVED_SPACE=0x10000
