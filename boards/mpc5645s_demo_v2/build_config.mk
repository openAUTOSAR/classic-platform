
# ARCH defines
ARCH=mpc5xxx
ARCH_FAM=ppc

# CFG (y/n) macros
CFG=PPC E200Z4D MPC55XX MPC5645S BRD_MPC5645S_DEMO_V2 TIMER TIMER_TB
CFG+=VLE
CFG+=SPE_FPU_SCALAR_SINGLE
CFG+=MCU_ARC_CONFIG
CFG+=CREATE_SREC
CFG+=MCU_ARC_LP
CFG+=OS_SYSTICK2


# MCAL
MOD_AVAIL+=ADC DIO DMA CAN GPT LIN MCU PORT PWM WDG FLS SPI ICU OCU
# Complex device drivers
MOD_AVAIL+=CDD_LINSLV

# Required modules
MOD_USE += MCU KERNEL

# Defines
#def-y += SRAM_SIZE=0x30000

# Default cross compiler
COMPILER?=gcc
COMPILER_FLAVOR=s32_newlib
# Default cross compiler
COMPILER_FLAVOR=s32_newlib
DEFAULT_CROSS_COMPILE = /c/devtools/Freescale/S32DS_Power_v2017.R1/Cross_Tools/powerpc-eabivle-4_9/bin/powerpc-eabivle-
DEFAULT_CW_COMPILE= /c/devtools/Freescale/cw_mpc5xxx_2.10
DEFAULT_DIAB_COMPILE = /c/devtools/WindRiver/diab/5.9.3.0/WIN32
DEFAULT_GHS_COMPILE = /c/devtools/ghs/comp_201314p

vle=$(if $(filter $(CFG),VLE),y)
novle=$(if $(vle),n,y)
SPE_FPU_SCALAR_SINGLE=$(if $(filter $(CFG),SPE_FPU_SCALAR_SINGLE),y)
nospe=$(if $(SPE_FPU_SCALAR_SINGLE),n,y)

diab-$(vle)$(nospe)+=-tPPCE200Z4DVFS:simple
diab-$(novle)$(nospe)+=-tPPCE200Z4DNMS:simple
diab-$(vle)$(SPE_FPU_SCALAR_SINGLE)+=-tPPCE200Z4DVFF:simple		
diab-y+=$(diab-yy)

DIAB_TARGET?=$(diab-y)

GHS_TARGET?=ppc564xs


