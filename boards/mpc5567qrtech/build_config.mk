
# ARCH defines
ARCH=mpc5xxx
ARCH_FAM=ppc

# CFG (y/n) macros
CFG=PPC BOOKE E200Z6 MPC55XX MPC5567 BRD_MPC5567QRTECH SPE_FPU_SCALAR_SINGLE TIMER_TB
CFG+=MCU_ARC_CONFIG MCU_CACHE  # MCU_MMU MCU_LP
CFG+=CREATE_SREC
CFG+=VLE

#CFG+=BOOT

# What buildable modules does this board have, 
# default or private

# MCAL
MOD_AVAIL+=ADC DIO DMA CAN MCU PORT PWM WDG SPI ETH FLS
# Additional
MOD_AVAIL+=LWIP TCPIP SOAD DOIP UDPNM ETHSM
# Required modules
MOD_USE += MCU KERNEL

COMPILER?=gcc
COMPILER_FLAVOR=s32_newlib

# Default cross compiler
DEFAULT_CROSS_COMPILE = /c/devtools/Freescale/S32DS_Power_v2017.R1/Cross_Tools/powerpc-eabivle-4_9/bin/powerpc-eabivle-
DEFAULT_GHS_COMPILE = /c/devtools/ghs/comp_201314p
DEFAULT_CW_COMPILE= /c/devtools/Freescale/cw_mpc5xxx_2.10

# Defines
def-y += SRAM_SIZE=0x14000
def-y += L_BOOT_RESERVED_SPACE=0x10000

vle=$(if $(filter $(CFG),VLE),y)
novle=$(if $(vle),n,y)

# Software floating point, PowerPC No Small-Data ELF EABI Object Format
diab-$(vle)=-tPPCE200Z6VFS:simple
diab-$(novle)=-tPPCE200Z6NFS:simple
DIAB_TARGET?=$(diab-y)

GHS_TARGET=ppc5567

