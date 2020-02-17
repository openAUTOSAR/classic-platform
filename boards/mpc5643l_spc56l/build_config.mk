
# ARCH defines
ARCH=mpc5xxx
ARCH_FAM=ppc

# CFG (y/n) macros
CFG=PPC E200Z4D MPC55XX MPC5643L BRD_MPC5643L_SPC56L
CFG+=SPE_FPU_SCALAR_SINGLE
CFG+=MCU_ARC_CONFIG FCCU
CFG+=CREATE_SREC

ifneq ($(COMPILER),gcc)
CFG+=VLE
endif


# What buildable modules does this board have, 
# default or private

# MCAL
MOD_AVAIL+=ADC DIO DMA CAN GPT LIN MCU PORT PWM WDG SPI FLS

# Required modules
ifndef NOKERNEL
MOD_USE += MCU KERNEL
endif
# Defines
def-y += SRAM_SIZE=0x20000
ifndef NOKERNEL
def-y += L_BOOT_RESERVED_SPACE=0x20000
endif
# Default cross compiler
COMPILER?=cw
DEFAULT_CROSS_COMPILE = /opt/powerpc-eabispe/bin/powerpc-eabispe-
DEFAULT_CW_COMPILE= /c/devtools/Freescale/cw_mpc5xxx_2.10
DEFAULT_DIAB_COMPILE = /c/devtools/WindRiver/diab/5.9.3.0/WIN32

vle=$(if $(filter $(CFG),VLE),y)
novle=$(if $(vle),n,y)
SPE_FPU_SCALAR_SINGLE=$(if $(filter $(CFG),SPE_FPU_SCALAR_SINGLE),y)
nospe=$(if $(SPE_FPU_SCALAR_SINGLE),n,y)

diab-$(vle)$(nospe)+=-tPPCE200Z3VFN:simple
diab-$(novle)$(nospe)+=-tPPCE200Z3NFS:simple
diab-$(vle)$(SPE_FPU_SCALAR_SINGLE)+=-tPPCE200Z3VFF:simple
diab-y+=$(diab-yy)

DIAB_TARGET?=$(diab-y)

# VLE
GHS_TARGET?=ppc563xm


