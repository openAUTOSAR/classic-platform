
# ARCH defines
ARCH=mpc5xxx
ARCH_FAM=ppc

# CFG (y/n) macros
CFG=PPC E200Z7 MPC55XX MPC57XX MPC5777C CAN_OSCILLATOR_CLOCK BRD_MPC5777C_MPC57xx_MB
CFG+=MCU_ARC_CONFIG
CFG+=MCU_CACHE_I
CFG+=EFPU
CFG+=VLE
CFG+=TIMER
CFG+=TIMER_STM

# What buildable modules does this board have, 
# default or private

# Memory + Peripherals
MOD_AVAIL+=OS ADC DIO CAN GPT MCU PWM PORT WDG FLS SPI ETH DMA
# Additional
MOD_AVAIL+= LWIP TCPIP UDPNM 
# Required modules
MOD_USE += MCU KERNEL


# Default cross compiler
COMPILER?=ghs
# Default cross compiler
COMPILER_FLAVOR=s32_newlib
DEFAULT_CROSS_COMPILE = /c/devtools/Freescale/S32DS_Power_v2017.R1/Cross_Tools/powerpc-eabivle-4_9/bin/powerpc-eabivle-
DEFAULT_GHS_COMPILE = /c/devtools/ghs/comp_201314p
DEFAULT_CW_COMPILE= /c/devtools/Freescale/cw_mpc5xxx_2.10
DEFAULT_DIAB_COMPILE = /c/devtools/WindRiver/diab/5.9.3.0/WIN32


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
GHS_TARGET?=ppc5777mz720

# Defines
def-y += SRAM_SIZE=0x80000
def-y += L_BOOT_RESERVED_SPACE=0x0000
def-y += __BIG_ENDIAN__
