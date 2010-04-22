obj-$(CFG_PPC) += crt0.o
obj-$(CFG_HCS12D) += crt0.o
vpath-$(CFG_ARM_CM3) += $(ARCH_PATH-y)kernel
obj-$(CFG_ARM_CM3) += system_stm32f10x.o
obj-$(CFG_ARM_CM3) += core_cm3.o

obj-$(CFG_STM32_MD) += startup_stm32f10x_md.o
obj-$(CFG_STM32_LD) += startup_stm32f10x_ld.o
obj-$(CFG_STM32_HD) += startup_stm32f10x_hd.o
obj-$(CFG_STM32_CL) += startup_stm32f10x_cl.o

#Ecu
#obj-y += EcuM_$(BOARDDIR).o
obj-y += EcuM.o
obj-y += EcuM_Cfg.o
obj-y += EcuM_Callout_template.o
inc-y += $(ROOTDIR)/system/EcuM
vpath-y += $(ROOTDIR)/system/EcuM

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
obj-$(CFG_MPC55XX)-$(USE_MCU) += Mcu_Exceptions.o

# Flash
obj-$(USE_FLS) += Fls.o
obj-$(USE_FLS) += Fls_Cfg.o
obj-$(USE_FLS) += Fls_H7F.o

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
include $(ROOTDIR)/system/kernel/makefile

# Spi
obj-$(USE_SPI) += Spi.o
obj-$(USE_SPI) += Spi_Lcfg.o

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

#WdgM
obj-$(USE_WDGM) += WdgM.o
obj-$(USE_WDGM) += WdgM_Cfg.o

#Pwm
obj-$(USE_PWM) += Pwm.o
obj-$(USE_PWM) += Pwm_Cfg.o

# Misc
obj-y += Det.o

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
inc-$(USE_COMM) += $(ROOTDIR)/communication/ComM
vpath-$(USE_COMM) += $(ROOTDIR)/communication/ComM
vpath-y += $(ROOTDIR)/communication/ComM
inc-$(USE_COMM) += $(ROOTDIR)/communication/ComM


# Com
obj-$(USE_COM) += Com_PbCfg.o
obj-$(USE_COM) += Com_Com.o
obj-$(USE_COM) += Com_Sched.o
obj-$(USE_COM) += Com.o
obj-$(USE_COM) += Com_RunTest.o
obj-$(USE_COM) += Com_misc.o
#obj-$(USE_COM) += Com_TestData.o
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



#tests
#obj-y += RunTests.o
#obj-$(USE_CAN) += can_test.o
#obj-$(USE_DIO) += dio_test.o
#obj-$(USE_PORT) += port_test.o
#obj-$(USE_CANIF) += canif_test.o
#obj-$(USE_FLS) += fls_test.o
#obj-y += mahi_test.o
#obj-$(USE_GPT) += gpt_test.o
#obj-$(USE_SPI) += spi_test.o
#obj-$(USE_EEP) += eep_test.o
#obj-y += det_test.o
#obj-$(USE_MCU) += mcu_test.o
#obj-$(USE_FLS_SST25XX) += xfls_test.o
#obj-y += lin_test.o
#obj-$(USE_PDUR) += pdur_test.o
#obj-$(USE_COM) += com_test.o

#inc-$(USE_TESTS) += $(ROOTDIR)/embunit/embUnit
#inc-$(USE_TESTS) += $(ROOTDIR)/embunit/textui
#inc-$(USE_TESTS) += $(ROOTDIR)/embunit

#libitem-$(USE_TESTS) += $(ROOTDIR)/embunit/embUnit/obj_$(ARCH)/libembunit.a
#libitem-$(USE_TESTS) += $(ROOTDIR)/embunit/textui/obj_$(ARCH)/libtextui.a



# Common
obj-y += xtoa.o
obj-y += arc.o
#obj-y += malloc.o
obj-$(USE_RAMLOG) += ramlog.o

# If we have configured console output we include printf. 
# Overridden to use lib implementation with CFG_USE_NEWLIB_PRINTF
ifndef (CFG_USE_NEWLIB_PRINTF)
ifneq (,$(SELECT_CONSOLE) $(SELECT_OS_CONSOLE))
obj-y += printf.o
endif
endif

VPATH += $(ROOTDIR)/common

obj-y += newlib_port.o
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
