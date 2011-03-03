
_BOARD_COMMON_MK:=y  # Include guard for backwards compatability

obj-$(CFG_PPC) += crt0.o
obj-$(CFG_HC1X) += crt0.o

vpath-$(CFG_ARM_CM3) += $(ROOTDIR)/$(ARCH_PATH-y)/kernel
vpath-$(CFG_ARM_CM3) += $(ROOTDIR)/$(ARCH_PATH-y)/drivers/STM32F10x_StdPeriph_Driver/src
vpath-$(CFG_ARM_CM3) += $(ROOTDIR)/$(ARCH_PATH-y)/drivers/STM32_ETH_Driver/src
inc-$(CFG_ARM_CM3) += $(ROOTDIR)/$(ARCH_PATH-y)/drivers/STM32F10x_StdPeriph_Driver/inc
inc-$(CFG_ARM_CM3) += $(ROOTDIR)/$(ARCH_PATH-y)/drivers/STM32_ETH_Driver/inc
obj-$(CFG_ARM_CM3) += core_cm3.o
obj-$(CFG_ARM_CM3) += startup_stm32f10x.o
#stm32 lib files needed by drivers
obj-$(CFG_ARM_CM3) += stm32f10x_rcc.o
obj-$(CFG_ARM_CM3)-$(USE_CAN) += stm32f10x_can.o
obj-$(CFG_ARM_CM3)-$(USE_PORT) += stm32f10x_gpio.o
obj-$(CFG_ARM_CM3)-$(USE_ADC) += stm32f10x_adc.o
obj-$(CFG_ARM_CM3)-$(USE_ADC) += stm32f10x_dma.o
obj-$(CFG_ARM_CM3)-$(USE_FLS) += stm32f10x_flash.o
obj-$(CFG_ARM_CM3)-$(USE_PWM) += stm32f10x_tim.o
obj-$(CFG_ARM_CM3)-$(USE_LWIP) += stm32_eth.o

obj-$(USE_TTY_TMS570_KEIL) += GLCD.o
obj-$(USE_TTY_TMS570_KEIL) += emif.o

# Cortex R4
obj-$(CFG_ARM_CR4) += startup_cr4.o

# OS object files. 
# (checking if already included for compatability)
ifeq ($(filter Os_Cfg.o,$(obj-y)),)
obj-$(USE_KERNEL) += Os_Cfg.o
endif

#Ecu
obj-$(USE_ECUM) += EcuM.o
obj-$(USE_ECUM) += EcuM_Main.o
obj-$(USE_ECUM) += EcuM_Cfg.o
obj-$(USE_ECUM) += EcuM_Callout_template.o
inc-$(USE_ECUM) += $(ROOTDIR)/system/EcuM
vpath-$(USE_ECUM) += $(ROOTDIR)/system/EcuM

# Gpt
obj-$(USE_GPT) += Gpt.o
obj-$(USE_GPT) += Gpt_Cfg.o

# Dma
obj-$(USE_DMA) += Dma.o
obj-$(USE_DMA) += Dma_Cfg.o
inc-$(USE_DMA) += $(ROOTDIR)/$(ARCH_PATH-y)/drivers

# Mcu
obj-$(USE_MCU) += Mcu.o
obj-$(USE_MCU) += Mcu_Cfg.o
#obj-$(CFG_MPC55XX)-$(USE_MCU) += Mcu_Exceptions.o

# Flash
obj-$(USE_FLS) += Fls.o
obj-$(USE_FLS) += Fls_Cfg.o
obj-$(CFG_MPC55XX)-$(USE_FLS) += Fls_H7F.o

# Bring in the freescale driver source  
inc-$(CFG_MPC55XX) +=  $(ROOTDIR)/$(ARCH_PATH-y)/delivery/mpc5500_h7f/include

# Can
obj-$(USE_CAN) += Can.o
obj-$(USE_CAN) += Can_Lcfg.o

# CanIf
obj-$(USE_CANIF) += CanIf.o
obj-$(USE_CANIF) += CanIf_Cfg.o
vpath-$(USE_CANIF) += $(ROOTDIR)/communication/CanIf
inc-$(USE_CANIF) += $(ROOTDIR)/communication/CanIf

# CanTp
obj-$(USE_CANTP) += CanTp.o
obj-$(USE_CANTP) += CanTp_Cfg.o

vpath-$(USE_CANTP) += $(ROOTDIR)/communication/CanTp
inc-$(USE_CANTP) += $(ROOTDIR)/communication/CanTp

obj-$(USE_DIO) += Dio.o
obj-$(USE_DIO) += Dio_Lcfg.o

obj-$(USE_PORT) += Port.o
obj-$(USE_PORT) += Port_Cfg.o

obj-$(USE_ADC) += Adc.o
obj-$(USE_ADC) += Adc_Cfg.o

# Include the kernel
ifneq ($(USE_KERNEL),)
include $(ROOTDIR)/system/kernel/makefile
endif

# Spi
obj-$(USE_SPI) += Spi.o
obj-$(USE_SPI) += Spi_Lcfg.o

# NvM
obj-$(USE_NVM) += NvM.o
obj-$(USE_NVM) += NvM_Cfg.o
inc-$(USE_NVM) += $(ROOTDIR)/memory/NvM
vpath-$(USE_NVM) += $(ROOTDIR)/memory/NvM

# Fee
obj-$(USE_FEE) += Fee.o
obj-$(USE_FEE) += Fee_Cfg.o
inc-$(USE_FEE) += $(ROOTDIR)/memory/Fee
vpath-$(USE_FEE) += $(ROOTDIR)/memory/Fee

#Eep
obj-$(USE_EEP) += Eep.o
obj-$(USE_EEP) += Eep_Lcfg.o
obj-$(USE_EEP) += Eeprom_Lcfg.o

#Fls ext
obj-$(USE_FLS_SST25XX) += Fls_SST25xx.o
obj-$(USE_FLS_SST25XX) += Fls_SST25xx_Cfg.o
vpath-y += $(ROOTDIR)/peripherals

#Wdg
obj-$(USE_WDG) += Wdg.o
obj-$(USE_WDG) += Wdg_Lcfg.o

#WdgIf
obj-$(USE_WDG) += WdgIf.o
obj-$(USE_WDG) += WdgIf_Cfg.o
inc-y += $(ROOTDIR)/system/WdgIf
vpath-y += $(ROOTDIR)/system/WdgIf

#WdgM
obj-$(USE_WDGM) += WdgM.o
obj-$(USE_WDGM) += WdgM_PBcfg.o
inc-y += $(ROOTDIR)/system/WdgM
vpath-y += $(ROOTDIR)/system/WdgM

#Pwm
obj-$(USE_PWM) += Pwm.o
obj-$(USE_PWM) += Pwm_Cfg.o

# Misc
obj-$(USE_DET) += Det.o

# Lin
obj-$(USE_LIN) += Lin_PBcfg.o
obj-$(USE_LIN) += Lin_Lcfg.o
obj-$(USE_LIN) += LinIf_Lcfg.o
obj-$(USE_LIN) += LinIf_PBcfg.o
obj-$(USE_LIN) += LinSM_Lcfg.o
obj-$(USE_LIN) += LinSM_PBcfg.o
obj-$(USE_LIN) += LinSM_Cfg.o
obj-$(USE_LIN) += Lin.o
obj-$(USE_LIN) += LinIf.o
obj-$(USE_LIN) += LinSM.o
vpath-y += $(ROOTDIR)/drivers/Lin/
vpath-y += $(ROOTDIR)/communication/Lin
inc-y += $(ROOTDIR)/communication/ComM

# ComM
obj-$(USE_COMM) += ComM.o
obj-$(USE_COMM) += ComM_Cfg.o
inc-$(USE_COMM) += $(ROOTDIR)/communication/ComM
vpath-$(USE_COMM) += $(ROOTDIR)/communication/ComM
vpath-y += $(ROOTDIR)/communication/ComM
inc-$(USE_COMM) += $(ROOTDIR)/communication/ComM

# Nm
obj-$(USE_NM) += Nm.o
obj-$(USE_NM) += Nm_Cfg.o
inc-$(USE_NM) += $(ROOTDIR)/communication/Nm
vpath-$(USE_NM) += $(ROOTDIR)/communication/Nm

# CanNm
obj-$(USE_CANNM) += CanNm.o
obj-$(USE_CANNM) += CanNm_LCfg.o
inc-$(USE_CANNM) += $(ROOTDIR)/communication/CanNm
vpath-$(USE_CANNM) += $(ROOTDIR)/communication/CanNm

# CanSm
obj-$(USE_CANSM) += CanSM.o
obj-$(USE_CANSM) += CanSM_LCfg.o
inc-$(USE_CANSM) += $(ROOTDIR)/communication/CanSM
vpath-$(USE_CANSM) += $(ROOTDIR)/communication/CanSM

# Com
obj-$(USE_COM) += Com_PbCfg.o
obj-$(USE_COM) += Com_Com.o
obj-$(USE_COM) += Com_Sched.o
obj-$(USE_COM) += Com.o
obj-$(USE_COM) += Com_misc.o
inc-$(USE_PDUR) += $(ROOTDIR)/communication/Com
inc-$(USE_COM) += $(ROOTDIR)/communication/Com
vpath-$(USE_COM) += $(ROOTDIR)/communication/Com

# PduR
obj-$(USE_PDUR) += PduR_Com.o
obj-$(USE_PDUR) += PduR_If.o
obj-$(USE_PDUR) += PduR_LinIf.o
obj-$(USE_PDUR) += PduR_PbCfg.o
obj-$(USE_PDUR) += PduR_CanIf.o
obj-$(USE_PDUR) += PduR_CanTp.o
obj-$(USE_PDUR) += PduR_Dcm.o
obj-$(USE_PDUR) += PduR.o
inc-$(USE_PDUR) += $(ROOTDIR)/communication/PduR
inc-$(USE_COM) += $(ROOTDIR)/communication/PduR
vpath-$(USE_PDUR) += $(ROOTDIR)/communication/PduR

# IO Hardware Abstraction
obj-$(USE_IOHWAB) += IoHwAb.o

#Dem
obj-$(USE_DEM) += Dem.o
obj-$(USE_DEM) += Dem_LCfg.o
inc-$(USE_DEM) += $(ROOTDIR)/diagnostic/Dem
vpath-$(USE_DEM) += $(ROOTDIR)/diagnostic/Dem

#Dcm
obj-$(USE_DCM) += Dcm.o
obj-$(USE_DCM) += Dcm_Dsp.o
obj-$(USE_DCM) += Dcm_Dsd.o
obj-$(USE_DCM) += Dcm_Dsl.o
obj-$(USE_DCM) += Dcm_LCfg.o
inc-$(USE_DCM) += $(ROOTDIR)/diagnostic/Dcm
vpath-$(USE_DCM) += $(ROOTDIR)/diagnostic/Dcm

obj-$(USE_RAMLOG) += ramlog.o

# Common stuff, if speciied
VPATH += $(ROOTDIR)/common


#TCF
obj-$(USE_TCF) += tcf.o
obj-$(USE_TCF) += Tcf_Cfg.o
obj-$(USE_TCF) += sys_monitor.o
obj-$(USE_TCF) += streams.o
inc-$(USE_TCF) += $(ROOTDIR)/common/tcf
vpath-$(USE_TCF) += $(ROOTDIR)/common/tcf

#SLEEP
obj-$(USE_SLEEP) += sleep.o

obj-y += xtoa.o

SELECT_CLIB?=CLIB_NEWLIB

ifeq ($(SELECT_CLIB),CLIB_CW)
  # This is not good, but don't know what to do right now....
  obj-y += msl_port.o
else
  # Newlib
  obj-y += newlib_port.o
  # If we have configured console output we include printf. 
  # Overridden to use lib implementation with CFG_NEWLIB_PRINTF
  ifneq ($(CFG_NEWLIB_PRINTF),y)
    ifneq (,$(SELECT_CONSOLE) $(SELECT_OS_CONSOLE))
      obj-y += printf.o
    endif # SELECT_CONSOLE
  endif # CFG_NEWLIB_PRINTF
endif # SELECT_CLIB 


obj-y += $(obj-y-y)

vpath-y += $(ROOTDIR)/$(ARCH_PATH-y)/kernel
vpath-y += $(ROOTDIR)/$(ARCH_PATH-y)/drivers
vpath-y += $(ROOTDIR)/boards/$(BOARDDIR)
vpath-y += $(ROOTDIR)/arch/$(ARCH_FAM)
vpath-y += $(ROOTDIR)/boards/$(BOARDDIR)/config
vpath-y += $(ROOTDIR)/diagnostic/Dem
vpath-y += $(ROOTDIR)/diagnostic/Dcm
vpath-y += $(ROOTDIR)/diagnostic/Det

# include files need by us
inc-y += $(ROOTDIR)/include
inc-y += $(ROOTDIR)/kernel/test
inc-y += $(ROOTDIR)/kernel/include
inc-y += $(ROOTDIR)/$(ARCH_PATH-y)/kernel
inc-y += $(ROOTDIR)/$(ARCH_PATH-y)/drivers
inc-y += $(ROOTDIR)/boards/$(BOARDDIR)/config
inc-y += $(ROOTDIR)/drivers/Dem
inc-y += $(ROOTDIR)/drivers/Dcm
inc-y += $(ROOTDIR)/drivers/test


#
# And last the generic board
#
inc-y += $(ROOTDIR)/boards/generic
vpath-y += $(ROOTDIR)/boards/generic


VPATH += $(vpath-y)
#$(error $(VPATH))

# libs needed by us
#build-lib-y += $(ROOTDIR)/libs/libboard_$(BOARDDIR).a

