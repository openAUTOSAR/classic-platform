
# ARCH defines
ARCH=mpc5xxx
ARCH_FAM=ppc

# CFG (y/n) macros
CFG=PPC E200Z4D MPC55XX SPC56XL70 BRD_SPC56XL70_SPC56XL
CFG+=SPE_FPU_SCALAR_SINGLE 
CFG+=MCU_ARC_CONFIG FCCU
CFG+=CREATE_SREC
CFG+=VLE

# What buildable modules does this board have, 
# default or private

# Memory + Peripherals
MOD_AVAIL+=ADC DIO DMA CAN GPT LIN MCU PORT PWM WDG FLS SPI
# Required modules
MOD_USE += MCU KERNEL

# Defines
def-y += SRAM_SIZE=0x30000
def-y += L_BOOT_RESERVED_SPACE=0x10000

# Default cross compiler
COMPILER?=cw

# Default cross compiler
COMPILER_FLAVOR=s32_newlib
DEFAULT_CROSS_COMPILE = /c/devtools/Freescale/S32DS_Power_v2017.R1/Cross_Tools/powerpc-eabivle-4_9/bin/powerpc-eabivle-
DEFAULT_CW_COMPILE= /c/devtools/Freescale/cw_mpc5xxx_2.10
DEFAULT_DIAB_COMPILE = /c/devtools/WindRiver/diab/5.9.3.0/WIN32


vle=$(if $(filter $(CFG),VLE),y)
novle=$(if $(vle),n,y)
SPE_FPU_SCALAR_SINGLE=$(if $(filter $(CFG),SPE_FPU_SCALAR_SINGLE),y)
nospe=$(if $(SPE_FPU_SCALAR_SINGLE),n,y)


diab-$(vle)$(nospe)+=-tPPC5643LFS:simple		# SW floating point, VLE
diab-$(novle)$(nospe)+=-tPPCE200Z4DNMS:simple	# SW floating point, Book-E
diab-$(vle)$(SPE_FPU_SCALAR_SINGLE)+=-tPPC5643LFF:simple			# Single HW, Double SW Floating Point
diab-$(novle)$(SPE_FPU_SCALAR_SINGLE)+=-tPPCE200Z4DNFF:simple		# SW floating point, Book-E
diab-x+=$(diab-yy)

DIAB_TARGET?=$(strip $(diab-x))

# VLE
GHS_TARGET?=ppc563xm


