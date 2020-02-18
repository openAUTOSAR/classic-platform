
_BOARD_COMMON_MK:=y  # Include guard for backwards compatability

ifndef NOKERNEL
obj-$(CFG_PPC) += crt0.o
obj-$(CFG_RH850) += crt0.o

# mpc5xxx.h headers 
inc-$(CFG_PPC) += $(ROOTDIR)/mcal/arch/$(ARCH)

# Coverage
obj-$(CFG_BULLSEYE)  += Bullseye_Coverage_Hooks.o

inc-last-$(CFG_BULLSEYE) +=  $(COV_BULLSEYE_PATH)

##PPC##
inc-$(CFG_PPC) += $(ROOTDIR)/$(mcal/arch/$(ARCH_MCU))

##TC26X##
vpath-$(CFG_TC26X) += $(ROOTDIR)/mcal/arch/tcxxx/src
vpath-$(CFG_TC26X) += $(ROOTDIR)/mcal/arch/tcxxx/src/integration

obj-$(CFG_TC26X) += IfxCpu_CStart$(SELECT_CORE).o
obj-$(CFG_TC26X) += IfxCpu_cfg.o
obj-$(CFG_TC26X) += IfxCpu.o
obj-$(CFG_TC26X) += IfxScuWdt.o
obj-$(CFG_TC26X) += CompilerGnuc.o
obj-$(CFG_TC26X) += Ifx_preInitHook.o

IFXTC26X_BASE_FRAMEWORK =  $(ROOTDIR)/mcal/arch/tcxxx/src/contrib/ap32201_TC2xx_Source/0_Src


inc-$(CFG_TC26X) += $(IFXTC26X_BASE_FRAMEWORK)/4_McHal/Tricore/_Reg
inc-$(CFG_TC26X) += $(IFXTC26X_BASE_FRAMEWORK)/4_McHal/Tricore/_Impl
inc-$(CFG_TC26X) += $(IFXTC26X_BASE_FRAMEWORK)/4_McHal/Tricore/Smu

inc-$(CFG_TC26X) += $(IFXTC26X_BASE_FRAMEWORK)/0_AppSw/Config/Common
inc-$(CFG_TC26X) += $(IFXTC26X_BASE_FRAMEWORK)/0_AppSw/Config/Tricore/Start

inc-$(CFG_TC26X) += $(IFXTC26X_BASE_FRAMEWORK)/4_McHal/Tricore/Scu/Std
inc-$(CFG_TC26X) += $(IFXTC26X_BASE_FRAMEWORK)/4_McHal/Tricore/Cpu/Std
inc-$(CFG_TC26X) += $(IFXTC26X_BASE_FRAMEWORK)/4_McHal/Tricore/Mtu

inc-$(CFG_TC26X) += $(IFXTC26X_BASE_FRAMEWORK)/4_McHal/Tricore
inc-$(CFG_TC26X) += $(IFXTC26X_BASE_FRAMEWORK)/1_SrvSw

vpath-$(CFG_TC26X) += $(IFXTC26X_BASE_FRAMEWORK)/4_McHal/Tricore/Cpu/CStart
vpath-$(CFG_TC26X) += $(IFXTC26X_BASE_FRAMEWORK)/4_McHal/Tricore/Scu/Std
vpath-$(CFG_TC26X) += $(IFXTC26X_BASE_FRAMEWORK)/1_SrvSw/Tricore/Compilers

inc-$(CFG_TC26X) += $(IFXTC26X_BASE_FRAMEWORK)/0_AppSw/Tricore/DemoApp/Start
vpath-$(CFG_TC26X) += $(IFXTC26X_BASE_FRAMEWORK)/0_AppSw/Tricore/DemoApp/Start

##TC29X##
vpath-$(CFG_TC29X) += $(ROOTDIR)/mcal/arch/tcxxx/src
vpath-$(CFG_TC29X) += $(ROOTDIR)/mcal/arch/tcxxx/src/integration
vpath-$(CFG_TC29X) += $(ROOTDIR)/system/Os/osal/tricore/aurix/kernel

obj-$(CFG_TC29X) += IfxCpu_CStart$(SELECT_CORE).o
obj-$(CFG_TC29X) += IfxCpu_cfg.o
obj-$(CFG_TC29X) += IfxCpu.o
obj-$(CFG_TC29X) += IfxScuWdt.o
ifeq ($(CFG_TC212),y)
# Don't add for this one
ifndef MCAL_PATH
	obj-y += IfxScuCcu.o
endif

else
obj-$(CFG_TC29X) += IfxScuCcu.o
endif

obj-$(CFG_TC29X) += CompilerGnuc.o
obj-$(CFG_TC29X) += Ifx_preInitHook.o


IFXTC29X_BASE_FRAMEWORK =  $(ROOTDIR)/mcal/arch/tcxxx/src/contrib/ap32201_TC2xx_Source/0_Src


inc-$(CFG_TC29X) += $(IFXTC29X_BASE_FRAMEWORK)/4_McHal/Tricore/_Reg
inc-$(CFG_TC29X) += $(IFXTC29X_BASE_FRAMEWORK)/4_McHal/Tricore/_Impl
inc-$(CFG_TC29X) += $(IFXTC29X_BASE_FRAMEWORK)/4_McHal/Tricore/Smu

inc-$(CFG_TC29X) += $(IFXTC29X_BASE_FRAMEWORK)/0_AppSw/Config/Common
inc-$(CFG_TC29X) += $(IFXTC29X_BASE_FRAMEWORK)/0_AppSw/Config/Tricore/Start

inc-$(CFG_TC29X) += $(IFXTC29X_BASE_FRAMEWORK)/4_McHal/Tricore/Scu/Std
inc-$(CFG_TC29X) += $(IFXTC29X_BASE_FRAMEWORK)/4_McHal/Tricore/Cpu/Std
inc-$(CFG_TC29X) += $(IFXTC29X_BASE_FRAMEWORK)/4_McHal/Tricore/Mtu

inc-$(CFG_TC29X) += $(IFXTC29X_BASE_FRAMEWORK)/4_McHal/Tricore
inc-$(CFG_TC29X) += $(IFXTC29X_BASE_FRAMEWORK)/1_SrvSw

vpath-$(CFG_TC29X) += $(IFXTC29X_BASE_FRAMEWORK)/4_McHal/Tricore/Cpu/CStart
vpath-$(CFG_TC29X) += $(IFXTC29X_BASE_FRAMEWORK)/4_McHal/Tricore/Scu/Std
vpath-$(CFG_TC29X) += $(IFXTC29X_BASE_FRAMEWORK)/1_SrvSw/Tricore/Compilers

inc-$(CFG_TC29X) += $(IFXTC29X_BASE_FRAMEWORK)/0_AppSw/Tricore/DemoApp/Start
vpath-$(CFG_TC29X) += $(IFXTC29X_BASE_FRAMEWORK)/0_AppSw/Tricore/DemoApp/Start


##TC39X##
obj-$(CFG_TC39X) += Ifx_Ssw_Tc$(SELECT_CORE).o
obj-$(CFG_TC39X) += IfxCpu_cfg_Tc3xx.o
obj-$(CFG_TC39X) += IfxCpu_Tc3xx.o
obj-$(CFG_TC39X) += Ifx_Cfg_SswCallouts.o
obj-$(CFG_TC39X) += Ifx_Cfg_Ssw.o
obj-$(CFG_TC39X) += Ifx_Ssw_Infra.o
obj-$(CFG_TC39X) += IfxScuWdt.o
obj-$(CFG_TC39X) += IfxScuCcu.o
obj-$(CFG_TC39X) += CompilerGnuc.o
obj-$(CFG_TC39X) += IfxPmsEvr.o

IFXTC399_BASE_FRAMEWORK =  $(ROOTDIR)/system/Os/osal/tricore/aurix/TC39x_Source/IFX_base_famework/BaseFramework_TC39xA/0_Src
inc-$(CFG_TC39X) +=  $(IFXTC399_BASE_FRAMEWORK)/AppSw/Config/Common
inc-$(CFG_TC39X) +=  $(IFXTC399_BASE_FRAMEWORK)/SrvSw/CpuGeneric
inc-$(CFG_TC39X) +=  $(IFXTC399_BASE_FRAMEWORK)/BaseSw
inc-$(CFG_TC39X) +=  $(IFXTC399_BASE_FRAMEWORK)/BaseSw/Sfr_39A
inc-$(CFG_TC39X) +=  $(IFXTC399_BASE_FRAMEWORK)/BaseSw/Sfr_39A/_Reg
inc-$(CFG_TC39X) +=  $(IFXTC399_BASE_FRAMEWORK)/BaseSw/StartUp_39A/Tricore
inc-$(CFG_TC39X) +=  $(IFXTC399_BASE_FRAMEWORK)/BaseSw/iLLD_39A/Tricore
inc-$(CFG_TC39X) +=  $(IFXTC399_BASE_FRAMEWORK)/BaseSw/iLLD_39A/Tricore/_Impl
inc-$(CFG_TC39X) +=  $(IFXTC399_BASE_FRAMEWORK)/BaseSw/iLLD_39A/Tricore/Scu/Std
inc-$(CFG_TC39X) +=  $(IFXTC399_BASE_FRAMEWORK)/BaseSw/iLLD_39A/Tricore/Pms/Std
inc-$(CFG_TC39X) +=  $(IFXTC399_BASE_FRAMEWORK)/BaseSw/iLLD_39A/Tricore/Cpu/Std
vpath-$(CFG_TC39X) += $(IFXTC399_BASE_FRAMEWORK)/AppSw/Tricore/Cfg_Ssw
vpath-$(CFG_TC39X) += $(IFXTC399_BASE_FRAMEWORK)/BaseSw/StartUp_39A/Tricore
vpath-$(CFG_TC39X) += $(IFXTC399_BASE_FRAMEWORK)/BaseSw/Compilers/Tricore
vpath-$(CFG_TC39X) += $(IFXTC399_BASE_FRAMEWORK)/BaseSw/iLLD_39A/Tricore/_Impl
vpath-$(CFG_TC39X) += $(IFXTC399_BASE_FRAMEWORK)/BaseSw/iLLD_39A/Tricore/Scu/Std
vpath-$(CFG_TC39X) += $(IFXTC399_BASE_FRAMEWORK)/BaseSw/iLLD_39A/Tricore/Pms/Std
vpath-$(CFG_TC39X) += $(IFXTC399_BASE_FRAMEWORK)/BaseSw/iLLD_39A/Tricore/Cpu/Std
## Excluding header from Lint ##
ifeq ($(ARCH_FAM),tricore)
LINT_EXCLUDE_PATHS +=$(IFXTC399_BASE_FRAMEWORK)/AppSw/Tricore/Cfg_Ssw
LINT_EXCLUDE_PATHS +=$(IFXTC399_BASE_FRAMEWORK)/BaseSw/StartUp_39A/Tricore
LINT_EXCLUDE_PATHS +=$(IFXTC399_BASE_FRAMEWORK)/BaseSw/Compilers/Tricore
LINT_EXCLUDE_PATHS +=$(IFXTC399_BASE_FRAMEWORK)/BaseSw/iLLD_39A/Tricore/_Impl
LINT_EXCLUDE_PATHS +=$(IFXTC399_BASE_FRAMEWORK)/BaseSw/iLLD_39A/Tricore/Scu/Std
LINT_EXCLUDE_PATHS +=$(IFXTC399_BASE_FRAMEWORK)/BaseSw/iLLD_39A/Tricore/Pms/Std
LINT_EXCLUDE_PATHS +=$(IFXTC399_BASE_FRAMEWORK)/BaseSw/iLLD_39A/Tricore/Cpu/Std
endif

else
obj-$(CFG_STARTUP) += crt0.o
endif
obj-$(CFG_HC1X)-$(COMPILER) += crt0_$(COMPILER).o


ifneq ($(filter-out arm generic gnulinux,$(ARCH_FAM)),)
obj-y+=init.o
endif


obj-$(CFG_ZYNQ)+=init.o
obj-$(CFG_TMS570) += init.o
obj-$(CFG_TRAVEO) += init.o

##ZYNQ##
inc-$(CFG_ZYNQ) += $(ROOTDIR)/mcal/arch/zynq/src
vpath-$(CFG_ZYNQ) += $(ROOTDIR)/mcal/arch/zynq/src
vpath-$(CFG_ZYNQ) += $(ROOTDIR)/mcal/arch/zynq/src/integration

##STM32##
vpath-$(CFG_STM32F1X) += $(ROOTDIR)/mcal/arch/$(ARCH_MCU)/src/contrib/STM32F10x_StdPeriph_Driver/src
vpath-$(CFG_STM32F1X) += $(ROOTDIR)/mcal/arch/$(ARCH_MCU)/src/contrib/STM32_ETH_Driver/src
vpath-$(CFG_STM32F1X) += $(ROOTDIR)/mcal/arch/$(ARCH_MCU)/src
inc-$(CFG_STM32F1X) += $(ROOTDIR)/mcal/arch/$(ARCH_MCU)/src/contrib/STM32F10x_StdPeriph_Driver/inc
inc-$(CFG_STM32F1X) += $(ROOTDIR)/mcal/arch/$(ARCH_MCU)/src/contrib/STM32_ETH_Driver/inc

obj-$(CFG_STM32F1X) += startup_cmx.o
obj-$(CFG_STM32F3X) += startup_cmx.o
obj-$(CFG_S32K144) += startup_cmx.o
obj-$(CFG_S32K148) += startup_cmx.o

obj-$(CFG_JACINTO) += startup_jacinto.o

vpath-$(if $(CFG_STM32F1X)$(CFG_STM32F3X)$(CFG_S32K144)$(CFG_JACINTO),y) += $(ROOTDIR)/system/Os/osal/arm/armv7_m

#stm32 lib files needed by drivers
obj-$(CFG_STM32F1X) += stm32f10x_rcc.o
obj-$(CFG_STM32F1X)-$(USE_CAN) += stm32f10x_can.o
obj-$(CFG_STM32F1X)-$(USE_PORT) += stm32f10x_gpio.o
obj-$(CFG_STM32F1X)-$(USE_ADC) += stm32f10x_adc.o
obj-$(CFG_STM32F1X)-$(USE_ADC) += stm32f10x_dma.o
obj-$(CFG_STM32F1X)-$(USE_FLS) += stm32f10x_flash.o
obj-$(CFG_STM32F1X)-$(USE_PWM) += stm32f10x_tim.o
obj-$(CFG_STM32F1X)-$(USE_LWIP) += stm32_eth.o

obj-$(USE_TTY_TMS570_KEIL) += GLCD.o
obj-$(USE_TTY_TMS570_KEIL) += emif.o


##JACINTO##

inc-$(CFG_JACINTO) += $(ROOTDIR)/mcal/arch/jacinto/src
vpath-$(CFG_JACINTO) += $(ROOTDIR)/mcal/arch/jacinto/src




##TMS570##
# Cortex R4

inc-$(CFG_TMS570) += $(ROOTDIR)/mcal/arch/tms570/src
vpath-$(CFG_TMS570) += $(ROOTDIR)/mcal/arch/tms570/src/integration
vpath-$(CFG_TMS570) += $(ROOTDIR)/mcal/arch/tms570/src

obj-$(CFG_ARMV7_AR) += startup_armv7-ar.o
vpath-$(CFG_ARMV7_AR) += $(ROOTDIR)/system/Os/osal/arm/$(ARCH)

ifeq ($(COMPILER),gcc)
obj-$(CFG_ARMV7_AR) += crtin.o
endif

# OS object files. 
# (checking if already included for compatability)
ifeq ($(filter Os_Cfg.o,$(obj-y)),)
obj-$(USE_KERNEL) += Os_Cfg.o
endif

ifneq ($(CFG_SYSTEM_ASSERT),y)
obj-y += arc_assert.o
endif

ifeq ($(CFG_MCAL_EXTERNAL),)
ifeq ($(ARCH_MCU),)
inc-y += $(ROOTDIR)/$(ARCH_DRIVER_PATH-y)/drivers
vpath-y += $(ROOTDIR)/$(ARCH_DRIVER_PATH-y)/drivers
else
inc-y += $(ROOTDIR)/$(ARCH_DRIVER_PATH-y)/drivers_$(ARCH_MCU)
vpath-y += $(ROOTDIR)/$(ARCH_DRIVER_PATH-y)/drivers_$(ARCH_MCU)
endif
endif

#EcuM
ECUM-MOD-MK?=$(ROOTDIR)/system/EcuM/EcuM.mod.mk
include $(ECUM-MOD-MK)  

#Rtm
RTM-MOD-MK?=$(ROOTDIR)/safety_security/Sm/Rtm/Rtm.mod.mk
-include $(RTM-MOD-MK)

#Smal
SMAL-MOD-MK?=$(ROOTDIR)/safety_security/Sm/Smal/Smal.mod.mk
-include $(SMAL-MOD-MK)

#SecOC
SECOC-MOD-MK?=$(ROOTDIR)/communication/SecOC/SecOC.mod.mk
include $(SECOC-MOD-MK) 

#E2E
E2E-MOD-MK?=$(ROOTDIR)/safety_security/SafeLib/E2E/E2E.mod.mk
include $(E2E-MOD-MK)  

#HTMSS
HTMSS-MOD-MK?=$(ROOTDIR)/safety_security/Sm/Htmss/Htmss.mod.mk
-include $(HTMSS-MOD-MK)  

#CPL
CPL-MOD-MK?=$(ROOTDIR)/system/Cpl/Cpl.mod.mk
include $(CPL-MOD-MK) 

#CAL
CAL-MOD-MK?=$(ROOTDIR)/system/Cal/Cal.mod.mk
include $(CAL-MOD-MK) 

#Crc
CRC-MOD-MK?=$(ROOTDIR)/safety_security/SafeLib/Crc/Crc.mod.mk
include $(CRC-MOD-MK)

#BswM
BSWM-MOD-MK?=$(ROOTDIR)/system/BswM/BswM.mod.mk
include $(BSWM-MOD-MK)  

#Ea
EA-MOD-MK?=$(ROOTDIR)/memory/Ea/Ea.mod.mk
include $(EA-MOD-MK)  

# Gpt
GPT-MOD-MK?=$(ROOTDIR)/mcal/Gpt/gpt.mod.mk
include $(GPT-MOD-MK)  

# Dma
DMA-MOD-MK?=$(ROOTDIR)/drivers/dma.mod.mk
include $(DMA-MOD-MK)

#CDD_LinSlv
CDD_LINSLV-MOD-MK?=$(ROOTDIR)/cdd/LinSlv/cdd_linslv.mod.mk
include $(CDD_LINSLV-MOD-MK)  

#CDD_PduR
ifeq ($(USE_CDDPDUR),y)
include $(ROOTDIR)/cdd/PduR/mod.mk
endif

#DLT UartCom
ifeq ($(USE_DLTUARTCOM),y)
include $(ROOTDIR)/cdd/DltUartCom/mod.mk
endif

# CDD_EthTrcv
ifeq ($(USE_CDDETHTRCV),y)
include $(ROOTDIR)/cdd/EthTrcv/mod.mk
endif

# Mcu
MCU-MOD-MK?=$(ROOTDIR)/mcal/Mcu/mcu.mod.mk
include $(MCU-MOD-MK)  

# Port
PORT-MOD-MK?=$(ROOTDIR)/mcal/Port/port.mod.mk
include $(PORT-MOD-MK) 

# CPU specific
# kernel
KERNEL-MOD-MK?=$(ROOTDIR)/drivers/kernel.mod.mk
include $(KERNEL-MOD-MK) 

vpath-$(CFG_PPC) += $(ROOTDIR)/$(ARCH_KERNEL_PATH-y)/integration
vpath-$(CFG_ARM_CM3) += $(ROOTDIR)/$(ARCH_KERNEL_PATH-y)/integration
vpath-$(CFG_ARM_CM4) += $(ROOTDIR)/$(ARCH_KERNEL_PATH-y)/integration
vpath-$(CFG_TC2XX) += $(ROOTDIR)/$(ARCH_KERNEL_PATH-y)/integration
obj-$(CFG_MCU_ARC_LP) += Mcu_Arc_Cfg.o
obj-$(CFG_PPC) += Cpu.o

obj-$(CFG_MPC5516) += mm.o
vpath-$(CFG_PPC) += $(ROOTDIR)/$(ARCH_KERNEL_PATH-y)/mm
inc-$(CFG_PPC) += $(ROOTDIR)/$(ARCH_KERNEL_PATH-y)/mm

# Flash
FLS-MOD-MK?=$(ROOTDIR)/drivers/Fls/fls.mod.mk
include $(FLS-MOD-MK)

#TI F021 flash programming API
ifdef CFG_ARM_CR4
inc-$(CFG_TMS570) += $(ROOTDIR)/mcal/Fls/src/contrib
endif
ifdef CFG_BRD_TMDX570_LC43HDK
inc-$(CFG_TMS570) += $(ROOTDIR)/mcal/Fls/src/contrib
endif

obj-$(CFG_ARM_CR5) += HL_sys_core.o
obj-$(CFG_ARM_CR5) += HL_sys_mpu.o
vpath-$(CFG_ARM_CR5) += $(ROOTDIR)/mcal/Fls/src/contrib/HL_sys

# Bring in the freescale driver source  
inc-$(CFG_MPC55XX) +=  $(ROOTDIR)/$(ARCH_DRIVER_PATH-y)/delivery/mpc5500_h7f/include

# Eth, Ethernet
ETH-MOD-MK?=$(ROOTDIR)/mcal/Eth/eth.mod.mk
include $(ETH-MOD-MK)

# Eth SM
ETHSM-MOD-MK?=$(ROOTDIR)/communication/EthSM/EthSM.mod.mk
include $(ETHSM-MOD-MK)

# EthIF
ETHIF-MOD-MK?=$(ROOTDIR)/communication/EthIf/EthIf.mod.mk
include $(ETHIF-MOD-MK)

# EthTSyn
ETHTSYN-MOD-MK?=$(ROOTDIR)/communication/EthTSyn/EthTSyn.mod.mk
include $(ETHTSYN-MOD-MK)
# Icu
ICU-MOD-MK?=$(ROOTDIR)/drivers/icu.mod.mk
include $(ICU-MOD-MK)

# Fr, Flexray
FR-MOD-MK?=$(ROOTDIR)/drivers/Fr/fr.mod.mk
include $(FR-MOD-MK)

# FrIf
FRIF-MOD-MK?=$(ROOTDIR)/communication/FrIf/FrIf.mod.mk
include $(FRIF-MOD-MK)

# FrSM
FRSM-MOD-MK?=$(ROOTDIR)/communication/FrSM/FrSM.mod.mk
include $(FRSM-MOD-MK)

# Can
CAN-MOD-MK?=$(ROOTDIR)/mcal/Can/Can.mod.mk
include $(CAN-MOD-MK) 

CANTRCV-MOD-MK?=$(ROOTDIR)/drivers/CanTrcv/cantrcv.mod.mk
include $(CANTRCV-MOD-MK) 

# CanIf
CANIF-MOD-MK?=$(ROOTDIR)/communication/CanIf/CanIf.mod.mk
include $(CANIF-MOD-MK)

# CanTp
CANTP-MOD-MK?=$(ROOTDIR)/communication/CanTp/CanTp.mod.mk
include $(CANTP-MOD-MK)

#Dio
DIO-MOD-MK?=$(ROOTDIR)/mcal/Dio/dio.mod.mk
include $(DIO-MOD-MK)

#Adc
ADC-MOD-MK?=$(ROOTDIR)/mcal/Adc/Adc.mod.mk
include $(ADC-MOD-MK)

#FrTp
FRTP-MOD-MK?=$(ROOTDIR)/communication/FrTp/FrTp.mod.mk
include $(FRTP-MOD-MK)

# SchM, always find the incluSchMde files.
SCHM-MOD-MK?=$(ROOTDIR)/system/SchM/SchM.mod.mk
include $(SCHM-MOD-MK)

# StbM
STBM-MOD-MK?=$(ROOTDIR)/system/StbM/StbM.mod.mk
include $(STBM-MOD-MK)

# J1939Tp
J1939TP-MOD-MK?=$(ROOTDIR)/communication/J1939Tp/J1939Tp.mod.mk
include $(J1939TP-MOD-MK)

# Include the kernel
ifneq ($(USE_KERNEL),)
include $(ROOTDIR)/system/Os/rtos/makefile
endif

# Spi
SPI-MOD-MK?=$(ROOTDIR)/mcal/Spi/spi.mod.mk
include $(SPI-MOD-MK)

# NvM
NVM-MOD-MK?=$(ROOTDIR)/memory/NvM/NvM.mod.mk
include $(NVM-MOD-MK)

# MemIf
MEMIF-MOD-MK?=$(ROOTDIR)/memory/MemIf/MemIf.mod.mk
include $(MEMIF-MOD-MK)

# Fee
FEE-MOD-MK?=$(ROOTDIR)/memory/Fee/Fee.mod.mk
include $(FEE-MOD-MK)

#Eep
EEP-MOD-MK?=$(ROOTDIR)/drivers/Eep/Eep.mod.mk
include $(EEP-MOD-MK)

#Fls ext
obj-$(USE_FLS_SST25XX) += Fls_SST25xx.o
obj-$(USE_FLS_SST25XX) += Fls_SST25xx_Cfg.o

#Wdg
WDG-MOD-MK?=$(ROOTDIR)/mcal/Wdg/Wdg.mod.mk
include $(WDG-MOD-MK)

#WdgIf
WDGIF-MOD-MK?=$(ROOTDIR)/safety_security/WdgIf/WdgIf.mod.mk
include $(WDGIF-MOD-MK)

#WdgM
WDGM-MOD-MK?=$(ROOTDIR)/safety_security/WdgM/WdgM.mod.mk
include $(WDGM-MOD-MK)

#Pwm
PWM-MOD-MK?=$(ROOTDIR)/mcal/Pwm/Pwm.mod.mk
include $(PWM-MOD-MK)

# Ocu
OCU-MOD-MK?=$(ROOTDIR)/drivers/ocu.mod.mk
include $(OCU-MOD-MK)

#DET
DET-MOD-MK?=$(ROOTDIR)/diagnostic/Det/Det.mod.mk
include $(DET-MOD-MK)

# Dlt
DLT-MOD-MK?=$(ROOTDIR)/diagnostic/Dlt/Dlt.mod.mk
include $(DLT-MOD-MK)

# Lin
LIN-MOD-MK?=$(ROOTDIR)/drivers/lin.mod.mk
include $(LIN-MOD-MK)

# LinIf
LINIF-MOD-MK?=$(ROOTDIR)/communication/LinIf/LinIf.mod.mk
include $(LINIF-MOD-MK)

# LinTp
LINTP-MOD-MK?=$(ROOTDIR)/communication/LinIf/LinTp.mod.mk
include $(LINTP-MOD-MK)


# LinSm
LINSM-MOD-MK?=$(ROOTDIR)/communication/LinSM/LinSM.mod.mk
include $(LINSM-MOD-MK)

# ComM
COMM-MOD-MK?=$(ROOTDIR)/communication/ComM/ComM.mod.mk
include $(COMM-MOD-MK)

# Nm
NM-MOD-MK?=$(ROOTDIR)/communication/Nm/Nm.mod.mk
include $(NM-MOD-MK)

# CanNm
CANNM-MOD-MK?=$(ROOTDIR)/communication/CanNm/CanNm.mod.mk
include $(CANNM-MOD-MK)

# CanSm
CANSM-MOD-MK?=$(ROOTDIR)/communication/CanSM/CanSM.mod.mk
include $(CANSM-MOD-MK)

#FrNm
FRNM-MOD-MK?=$(ROOTDIR)/communication/FrNm/FrNm.mod.mk
include $(FRNM-MOD-MK)

#FrNm
OSEKNM-MOD-MK?=$(ROOTDIR)/communication/OsekNm/OsekNm.mod.mk
include $(OSEKNM-MOD-MK)

# Com
COM-MOD-MK?=$(ROOTDIR)/communication/Com/Com.mod.mk
include $(COM-MOD-MK)

# PduR
PDUR-MOD-MK?=$(ROOTDIR)/communication/PduR/PduR.mod.mk
include $(PDUR-MOD-MK)

# IpduM
IPDUM-MOD-MK?=$(ROOTDIR)/communication/IpduM/IpduM.mod.mk
include $(IPDUM-MOD-MK)

# DoIP
DOIP-MOD-MK?=$(ROOTDIR)/communication/DoIP/DoIP.mod.mk
include $(DOIP-MOD-MK)

# IO Hardware Abstraction
IOHWAB-MOD-MK?=$(ROOTDIR)/Peripherals/IoHwAb/IoHwAb.mod.mk
include $(IOHWAB-MOD-MK)

# SAFE IO Hardware Abstraction
SAFE-IOHWAB-MOD-MK?=$(ROOTDIR)/Peripherals/SafeIoHwAb/SafeIoHwAb.mod.mk
-include $(SAFE-IOHWAB-MOD-MK)

#Dem
DEM-MOD-MK?=$(ROOTDIR)/diagnostic/Dem/Dem.mod.mk
include $(DEM-MOD-MK)

#Dcm
DCM-MOD-MK?=$(ROOTDIR)/diagnostic/Dcm/Dcm.mod.mk
include $(DCM-MOD-MK)

#Fim
FIM-MOD-MK?=$(ROOTDIR)/diagnostic/FiM/FiM.mod.mk
include $(FIM-MOD-MK)

#RamTst
RAMTST-MOD-MK?=$(ROOTDIR)/memory/RamTst/RamTst.mod.mk
include $(RAMTST-MOD-MK)

#LinuxOs
LINUXOS-MOD-MK?=$(ROOTDIR)/system/LinuxOs/LinuxOs.mod.mk
include $(LINUXOS-MOD-MK)

# Common stuff, if speciied
VPATH += $(ROOTDIR)/common

# SD
SD-MOD-MK?=$(ROOTDIR)/communication/SD/SD.mod.mk
include $(SD-MOD-MK)

# SoAd
SOAD-MOD-MK?=$(ROOTDIR)/communication/SoAd/SoAd.mod.mk
include $(SOAD-MOD-MK)


# UdpNm
UDPNM-MOD-MK?=$(ROOTDIR)/communication/UdpNm/UdpNm.mod.mk
include $(UDPNM-MOD-MK)


# Xcp
XCP-MOD-MK?=$(ROOTDIR)/communication/Xcp/Xcp.mod.mk
include $(XCP-MOD-MK)

#LdCom
LDCOM-MOD-MK?=$(ROOTDIR)/communication/LdCom/LdCom.mod.mk
include $(LDCOM-MOD-MK)

#TCPIP
TCPIP-MOD-MK?=$(ROOTDIR)/communication/TcpIp/tcpip.mod.mk
-include $(TCPIP-MOD-MK)

#LWIP
LWIP-MOD-MK?=$(ROOTDIR)/communication/lwip.mod.mk
include $(LWIP-MOD-MK)

obj-$(USE_RAMLOG) += ramlog.o

#TCF
TCF-MOD-MK?=$(ROOTDIR)/common/tcf/tcf.mod.mk
include $(TCF-MOD-MK)

#Queue
QUEUE-MOD-MK?=$(ROOTDIR)/datastructures/Queue/Queue.mod.mk
include $(QUEUE-MOD-MK)

#Safety Queue
SAFE-QUEUE-MOD-MK?=$(ROOTDIR)/datastructures/Safety_Queue/SafeQ.mod.mk
include $(SAFE-QUEUE-MOD-MK)

ifeq ($(CFG_MCAL_EXTERNAL),y)
ifeq ($(ARCH_MCU),)
inc-y += $(ROOTDIR)/$(ARCH_DRIVER_PATH-y)/drivers
vpath-y += $(ROOTDIR)/$(ARCH_DRIVER_PATH-y)/drivers
else
inc-y += $(ROOTDIR)/$(ARCH_DRIVER_PATH-y)/drivers_$(ARCH_MCU)
vpath-y += $(ROOTDIR)/$(ARCH_DRIVER_PATH-y)/drivers_$(ARCH_MCU)
endif
endif


#SLEEP
obj-$(USE_SLEEP) += sleep.o

# Circular Buffer (always)
obj-y += cirq_buffer.o

obj-y += version.o

# IPC for JAC6
IPC-MK?=$(ROOTDIR)/mcal/arch/jacinto/ipc.mk
include $(IPC-MK)

#I2C for mpc
ifeq ($(USE_I2C_MPC),y) 
I2C_MPC-MOD-MK?=$(ROOTDIR)/mcal/arch/mpc5xxx/src/contrib/i2c/i2c_MPC.mod.mk
include $(I2C_MPC-MOD-MK)
endif  

ifeq ($(CFG_TIMER),y)
obj-$(CFG_TIMER_TB)-$(CFG_PPC)+=timer_tb.o
obj-$(CFG_TIMER_RTC)-$(CFG_PPC)+=timer_rtc.o
obj-$(CFG_TIMER_STM)-$(CFG_PPC)+=timer_stm.o
obj-$(CFG_TIMER_DWT)-$(CFG_ARM)+=timer_dwt.o
obj-$(CFG_TIMER_GLOBAL)-$(CFG_ZYNQ)+=timer_global.o
obj-$(CFG_TIMER_RTI)-$(CFG_TMS570)+=timer_rti.o
obj-$(CFG_TIMER_OSTM)+=timer_ostm.o
obj-$(CFG_TIMER_AURIX) += timer_aurix.o
obj-$(CFG_TIMER_TIMERS)-$(CFG_JACINTO) += timer.o
obj-$(CFG_TIMER_SCTM)-$(CFG_JACINTO) += timer_sctm.o

inc-y += $(ROOTDIR)/mcal/arch/$(ARCH)/src
vpath-y += $(ROOTDIR)/mcal/arch/$(ARCH)/src

endif

# Shell
obj-$(CFG_SHELL)+=shell.o

# Logger over shell (old)
obj-$(CFG_LOGGER) += logger.o
# New logger.
obj-$(CFG_LOG) += log.o

SELECT_CLIB?=CLIB_IAR

# Performance stuff
obj-$(CFG_OS_PERF)+=perf.o
def-$(CFG_OS_PERF)+=CFG_OS_ISR_HOOKS

# T1 integration stuff
ifeq ($(CFG_T1_ENABLE),y)
def-y += T1_ENABLE=1 #needed for T1 code
def-y += CFG_OS_ISR_HOOKS
ifeq ($(CFG_T1_DISABLE_FLEX),y)
def-y += T1_DISABLE_T1_FLEX=1
endif

T1_PATH?=$(PROJECT_DIR)
VPATH += $(T1_PATH)/T1/src
inc-y += $(T1_PATH)/T1/interface
inc-y += $(T1_PATH)/T1/src
libitem-y += $(T1_PATH)/T1/lib/libt1base.a
libitem-y += $(T1_PATH)/T1/lib/libt1com8.a
libitem-y += $(T1_PATH)/T1/lib/libt1cont.a
libitem-y += $(T1_PATH)/T1/lib/libt1delay.a
libitem-y += $(T1_PATH)/T1/lib/libt1flex.a
libitem-y += $(T1_PATH)/T1/lib/libt1mod.a
libitem-y += $(T1_PATH)/T1/lib/libt1scope.a
obj-y +=Arc_T1_Int.o
obj-y +=T1_AppInterface.o
obj-y +=T1_config.o
endif #CFG_T1_ENABLE

ifeq ($(CFG_ARC_CLIB),y)
  # Just use native clib
    # Override native C-library with ArcCore tweaks.
  inc-system-y += $(ROOTDIR)/clib
  vpath-y      += $(ROOTDIR)/clib

  obj-$(USE_TTY_T32)     += serial_dbg_t32.o  
  obj-$(USE_TTY_UDE)-$(CFG_PPC)     += serial_dbg_ude.o
  obj-$(USE_TTY_UDE)-$(CFG_ARM)     += serial_dbg_ude_arm.o  
  obj-$(USE_TTY_WINIDEA) += serial_dbg_winidea.o
  obj-$(USE_TTY_CODE_COMPOSER) += serial_dbg_code_composer.o
  obj-$(CFG_FS_RAM) += fs_ram.o
  obj-$(USE_TTY_SCI)     += serial_sci.o

  obj-y += clib_port.o
  obj-y += printf.o
  obj-y += xtoa.o
  obj-y-cw += strtok_r.o
  obj-y-diab += strtok_r.o    
else
  ifeq ($(SELECT_CLIB),CLIB_IAR)
    # This is not good, but don't know what to do right now....
    obj-y += iar_port.o
    obj-y += xtoa.o
  else ifeq ($(SELECT_CLIB),CLIB_CW)
    # This is not good, but don't know what to do right now....
    obj-y += xtoa.o
    obj-y += msl_port.o
    obj-$(USE_TTY_UDE)     += serial_dbg_ude.o
  endif    
endif # SELECT_CLIB 

# Path to serial dbg drivers..
obj-$(CFG_BITDEF) += bitdef.o
inc-y += $(ROOTDIR)/drivers
vpath-y += $(ROOTDIR)/drivers


# If postbuild mode, just build post-build files.
ifneq ($(CFG_POSTBUILD_BUILD),y)
obj-y += $(obj-y-y)
obj-y += $(obj-y-y-y)
obj-y += $(obj-y-y-y-y)
obj-y += $(obj-y-$(COMPILER))
else
obj-y =
endif

obj-y += $(pb-obj-y) $(pb-obj-y-y)

#$(error x$(obj-y)x)
vpath-y += $(vpath-y-y)


vpath-y += $(board_path)
vpath-y += $(ROOTDIR)/$(OSAL_ARCH)/$(ARCH_FAM)
vpath-y += $(ROOTDIR)/arch/$(ARCH_FAM)
vpath-y += $(board_path)/config
vpath-y += $(ROOTDIR)/diagnostic/Dem
vpath-y += $(ROOTDIR)/diagnostic/Dcm
vpath-y += $(ROOTDIR)/diagnostic/Det

# include files need by us
inc-y += $(ROOTDIR)/include
inc-y += $(ROOTDIR)/include/base

BASE-MOD-MK?=$(ROOTDIR)/base/base.mod.mk
include $(BASE-MOD-MK)

inc-y += $(ROOTDIR)/$(ARCH_KERNEL_PATH-y)/kernel
inc-y += $(board_path)/config

#
# And last the generic board
#

inc-y += $(ROOTDIR)/boards/generic
vpath-y += $(ROOTDIR)/boards/generic

vpath-y += $(ROOTDIR)/boards/

inc-y += $(ROOTDIR)/mcal/Mcu/inc

inc-y += $(ROOTDIR)/integration


VPATH += $(vpath-y)

#
# Remove unwanted warnings in BSW for different compilers and specific files
#
#Diab
CFLAGS_diab_os_alarm.o += -ei4546
CFLAGS_diab_Dio.o += -ei4546
CFLAGS_diab_Can.o += -ei2273,4111
CFLAGS_diab_Dem.o += -ei4186
#Codewarrior
CFLAGS_cw_IoHwAb.o += -W=nounused
CFLAGS_cw_Dio.o +=  -W=nounused
CFLAGS_cw_IoHwAb_Analog.o +=  -W=nounused -W=nounusedexpr
CFLAGS_cw_IoHwAb_Digital.o +=  -W=nounused
CFLAGS_cw_EcuM_Main.o +=  -W=nounused -W=off
CFLAGS_cw_EcuM.o +=  -W=nounused -W=off
CFLAGS_cw_Mcu.o += -W=nounused 
CFLAGS_cw_Can.o +=  -W=nounused
CFLAGS_cw_CanIf.o +=  -W=off
CFLAGS_cw_init.o +=  -W=off
CFLAGS_cw_Nm.o +=  -W=nounused -W=off
CFLAGS_cw_Mcu_Cfg.o += -W=off
CFLAGS_cw_ComM.o +=  -W=nounused
CFLAGS_cw_CanNm.o +=  -W=nounused
CFLAGS_cw_Pwm.o +=  -W=nounused
CFLAGS_cw_Adc_eQADC.o += -W=nopossible -W=nounwanted
CFLAGS_cw_Com_misc.o +=  -W=nounused
CFLAGS_cw_EcuM_Callout_Stubs.o +=  -W=nounused
CFLAGS_cw_SchM.o +=  -W=nounused

