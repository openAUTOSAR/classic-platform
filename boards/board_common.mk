
_BOARD_COMMON_MK:=y  # Include guard for backwards compatability


obj-$(CFG_PPC) += crt0.o
obj-$(CFG_HC1X)-$(COMPILER) += crt0_$(COMPILER).o

ifneq ($(filter-out arm generic,$(ARCH_FAM)),)
obj-y+=init.o
endif

#Raspberry Pi, ARM processor, V6 architecture, 
obj-$(CFG_ARM_V6) += crt0.o
obj-$(CFG_ARM_V6) += led.o
obj-$(CFG_ARM_V6) += bcm2835.o
obj-$(CFG_ARM_V6) += Uart.o
obj-$(CFG_ARM_V6) += SpeedSensor.o
obj-$(CFG_ARM_V6) += Mcp3008.o
obj-$(CFG_ARM_V6) += UltrasonicSensor.o

vpath-$(CFG_ARM_V6) += $(ROOTDIR)/$(ARCH_PATH-y)/drivers/pi_usb/src
vpath-$(CFG_ARM_V6) += $(ROOTDIR)/$(ARCH_PATH-y)/drivers/pi_eth/src
vpath-$(CFG_ARM_V6) += $(ROOTDIR)/$(ARCH_PATH-y)/drivers/sensors
inc-$(CFG_ARM_V6) += $(ROOTDIR)/$(ARCH_PATH-y)/drivers/pi_usb/inc
inc-$(CFG_ARM_V6) += $(ROOTDIR)/$(ARCH_PATH-y)/drivers/pi_eth/inc
inc-$(CFG_ARM_V6) += $(ROOTDIR)/$(ARCH_PATH-y)/drivers/sensors

#for pi
#eth
obj-$(CFG_ARM_V6)-$(USE_ETH)  += pi_eth.o                    
obj-$(CFG_ARM_V6)-$(USE_ETH) += smsc9512.o
obj-$(CFG_ARM_V6)-$(USE_ETH) += Eth_test.o
#usb
obj-$(CFG_ARM_V6)-$(USE_USB) += usb_core.o
obj-$(CFG_ARM_V6)-$(USE_USB) += usb_hub.o
obj-$(CFG_ARM_V6)-$(USE_USB) += usb_dwc_hcd.o
obj-$(CFG_ARM_V6)-$(USE_USB) += usb_memory.o
obj-$(CFG_ARM_V6)-$(USE_USB) += usb_semaphore.o
obj-$(CFG_ARM_V6)-$(USE_USB) += usb_mailbox.o
obj-$(CFG_ARM_V6)-$(USE_USB) += usb_debug.o
obj-$(CFG_ARM_V6)-$(USE_USB) += semaphore.o
#i2c
obj-$(CFG_ARM_V6)-$(USE_I2C) += I2c.o
#opb715
#obj-$(CFG_ARM_V6)-$(USE_OPB715) += Opb715.o
#end pi




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




#Ecu (deprecated, use EcuMFixed or EcuMFlexible instead)
obj-$(USE_ECUM) += EcuM.o
obj-$(USE_ECUM) += EcuM_Fixed.o
obj-$(USE_ECUM) += EcuM_Main.o
obj-$(USE_ECUM) += EcuM_PBcfg.o
obj-$(USE_ECUM) += EcuM_PBHeader.o
ifeq ($(filter EcuM_Callout_Stubs.o,$(obj-y)),)
obj-$(USE_ECUM) += EcuM_Callout_Stubs.o
endif
obj-$(USE_ECUM)-$(CFG_ECUM_USE_SERVICE_COMPONENT) += EcuM_ServiceComponent.o
inc-$(USE_ECUM) += $(ROOTDIR)/system/EcuM
vpath-$(USE_ECUM) += $(ROOTDIR)/system/EcuM

#EcuMFixed
obj-$(USE_ECUM_FIXED) += EcuM.o
obj-$(USE_ECUM_FIXED) += EcuM_Fixed.o
obj-$(USE_ECUM_FIXED) += EcuM_Main.o
obj-$(USE_ECUM_FIXED) += EcuM_PBcfg.o
obj-$(USE_ECUM_FIXED) += EcuM_PBHeader.o
ifeq ($(filter EcuM_Callout_Stubs.o,$(obj-y)),)
obj-$(USE_ECUM_FIXED) += EcuM_Callout_Stubs.o
endif
obj-$(USE_ECUM_FIXED)-$(CFG_ECUM_USE_SERVICE_COMPONENT) += EcuM_ServiceComponent.o
inc-$(USE_ECUM_FIXED) += $(ROOTDIR)/system/EcuM
vpath-$(USE_ECUM_FIXED) += $(ROOTDIR)/system/EcuM

#EcuMFlexible
obj-$(USE_ECUM_FLEXIBLE) += EcuM.o
obj-$(USE_ECUM_FLEXIBLE) += EcuM_Flexible.o
obj-$(USE_ECUM_FLEXIBLE) += EcuM_PBcfg.o
obj-$(USE_ECUM_FLEXIBLE) += EcuM_PBHeader.o
obj-$(USE_ECUM_FLEXIBLE)-$(CFG_ECUM_USE_SERVICE_COMPONENT) += EcuM_ServiceComponent.o
inc-$(USE_ECUM_FLEXIBLE) += $(ROOTDIR)/system/EcuM
vpath-$(USE_ECUM_FLEXIBLE) += $(ROOTDIR)/system/EcuM

#E2E
obj-$(USE_E2E) += E2E.o
obj-$(USE_E2E) += E2E_P01.o
obj-$(USE_E2E) += E2E_P02.o
inc-$(USE_E2E) += $(ROOTDIR)/system/E2E
vpath-$(USE_E2E) += $(ROOTDIR)/system/E2E

#Crc
obj-$(USE_CRC) += Crc.o
obj-$(USE_CRC) += Crc_8.o
obj-$(USE_CRC) += Crc_16.o
obj-$(USE_CRC) += Crc_32.o
inc-$(USE_CRC) += $(ROOTDIR)/system/Crc
vpath-$(USE_CRC) += $(ROOTDIR)/system/Crc

#BswM
obj-$(USE_BSWM) += BswM.o
inc-$(USE_BSWM) += $(ROOTDIR)/system/BswM
vpath-$(USE_BSWM) += $(ROOTDIR)/system/BswM

#Ea
obj-$(USE_EA) += Ea.o
obj-$(USE_EA) += Ea_Lcfg.o
vpath-$(USE_EA) += $(ROOTDIR)/memory/Ea

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
ifeq ($(CFG_MCU_ARC_CONFIG),y)
ifeq ($(filter Mcu_Arc_mpc55xx.o Mcu_Arc_mpc56xx.o,$(obj-y)),)
obj-$(USE_MCU)-$(if $(CFG_MPC5516)$(CFG_MPC5668)$(CFG_MPC5567),y) += Mcu_Arc_mpc55xx.o
obj-$(USE_MCU)-$(if $(CFG_MPC5516)$(CFG_MPC5668)$(CFG_MPC5567),n,y) += Mcu_Arc_mpc56xx.o
endif
endif

# CPU specific
ifeq ($(CFG_PPC),y)
obj-$(USE_KERNEL) += mpc5xxx_handlers.o
obj-$(USE_KERNEL) += mpc5xxx_handlers_asm.o
ifeq ($(filter mpc5xxx_callout_stubs,$(obj-y)),)
obj-$(USE_KERNEL) += mpc5xxx_callout_stubs.o
endif
endif

vpath-$(CFG_PPC) += $(ROOTDIR)/$(ARCH_PATH-y)/integration
obj-$(CFG_MCU_ARC_CONFIG) += Mcu_Arc_Cfg.o
obj-$(CFG_PPC) += Cpu.o

obj-$(CFG_PPC) += mm.o
vpath-$(CFG_PPC) += $(ROOTDIR)/$(ARCH_PATH-y)/mm
inc-$(CFG_PPC) += $(ROOTDIR)/$(ARCH_PATH-y)/mm

# Flash
obj-$(USE_FLS) += Fls.o
obj-$(USE_FLS) += Fls_Cfg.o
ifdef CFG_MPC5744P
obj-$(USE_FLS) += flash_c55.o
else
obj-$(CFG_MPC55XX)-$(USE_FLS) += flash_h7f_c90.o
obj-$(CFG_MPC55XX)-$(USE_FLS) += flash_ll_h7f_c90.o
endif

# Bring in the freescale driver source  
inc-$(CFG_MPC55XX) +=  $(ROOTDIR)/$(ARCH_PATH-y)/delivery/mpc5500_h7f/include

# Can
obj-$(USE_CAN) += Can.o
obj-$(USE_CAN)-$(CFG_PPC) += Can_PBcfg.o
obj-$(USE_CAN)-$(CFG_ARM_CM3) += Can_PBcfg.o
obj-$(USE_CAN)-$(CFG_ARM_V6) += Can_PBcfg.o
obj-$(USE_CAN)-$(CFG_ARM_CR4) += Can_Lcfg.o
obj-$(USE_CAN)-$(CFG_HC1X) += Can_Lcfg.o

#for Pi_CAN // temp
#obj-$(USE_SPI)-$(CFG_ARM_V6) += mcp2515.o 


# CanIf
obj-$(USE_CANIF) += CanIf.o
obj-$(USE_CANIF) += CanIf_Cfg.o
obj-$(USE_CANIF) += CanIf_PBCfg.o
vpath-$(USE_CANIF) += $(ROOTDIR)/communication/CanIf
inc-$(USE_CANIF) += $(ROOTDIR)/communication/CanIf

# CanTp
obj-$(USE_CANTP) += CanTp.o
obj-$(USE_CANTP) += CanTp_PBCfg.o

vpath-$(USE_CANTP) += $(ROOTDIR)/communication/CanTp
inc-$(USE_CANTP) += $(ROOTDIR)/communication/CanTp

obj-$(USE_DIO) += Dio.o
obj-$(USE_DIO) += Dio_Lcfg.o

obj-$(USE_PORT) += Port.o
obj-$(USE_PORT) += Port_Cfg.o

obj-$(USE_ADC)-$(CFG_MPC560X) += Adc_560x.o
obj-$(USE_ADC)-$(CFG_MPC5668) += Adc_560x.o
obj-$(USE_ADC)-$(CFG_MPC5744P) += Adc_560x.o
obj-$(USE_ADC)-$(CFG_HC1X) += Adc.o
obj-$(USE_ADC)-$(CFG_ARM_CM3) += Adc.o
obj-$(USE_ADC)-$(CFG_MPC5516) += Adc_eQADC.o
obj-$(USE_ADC)-$(CFG_MPC5567) += Adc_eQADC.o
obj-$(USE_ADC)-$(CFG_MPC563XM) += Adc_eQADC.o
obj-$(USE_ADC) += Adc_Cfg.o
obj-$(USE_ADC) += Adc_Internal.o
vpath-y += $(ROOTDIR)/drivers
inc-y += $(ROOTDIR)/drivers

# SchM, always find the include files.
inc-y += $(ROOTDIR)/system/SchM
vpath-$(USE_SCHM) += $(ROOTDIR)/system/SchM
ifeq ($(filter SchM.o,$(obj-y)),)
obj-$(USE_SCHM) += SchM.o
endif

# J1939Tp
obj-$(USE_J1939TP) += J1939Tp.o
#obj-$(USE_J1939TP) += J1939Tp_LCfg.o
obj-$(USE_J1939TP) += J1939Tp_PBcfg.o
inc-$(USE_J1939TP) += $(ROOTDIR)/communication/J1939Tp
vpath-$(USE_J1939TP) += $(ROOTDIR)/communication/J1939Tp

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
obj-$(USE_NVM)-$(CFG_NVM_USE_SERVICE_COMPONENT) += NvM_ServiceComponent.o
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
#obj-$(USE_EEP) += Eeprom_Lcfg.o

#Fls ext
obj-$(USE_FLS_SST25XX) += Fls_SST25xx.o
obj-$(USE_FLS_SST25XX) += Fls_SST25xx_Cfg.o

#Wdg
obj-$(USE_WDG) += Wdg.o
obj-$(USE_WDG) += Wdg_Lcfg.o

#WdgIf
obj-$(USE_WDGIF) += WdgIf.o
obj-$(USE_WDGIF) += WdgIf_Cfg.o
obj-$(USE_WDGM)-$(CFG_WDGM_USE_SERVICE_COMPONENT) += WdgM_ServiceComponent.o
inc-y += $(ROOTDIR)/system/WdgIf
vpath-y += $(ROOTDIR)/system/WdgIf

#WdgM
obj-$(USE_WDGM) += WdgM.o
obj-$(USE_WDGM) += WdgM_PBcfg.o
inc-y += $(ROOTDIR)/system/WdgM
vpath-y += $(ROOTDIR)/system/WdgM

#Pwm
obj-$(USE_PWM) += Pwm_Cfg.o
obj-$(USE_PWM)-$(CFG_MPC5604P) += PwmFlex.o
obj-$(USE_PWM)-$(CFG_MPC5744P) += PwmFlex.o
obj-$(USE_PWM)-$(CFG_MPC560XB) += Pwm.o
obj-$(USE_PWM)-$(CFG_MPC5606S) += Pwm.o
obj-$(USE_PWM)-$(CFG_MPC5668) += Pwm.o
obj-$(USE_PWM)-$(CFG_HC1X) += Pwm.o
obj-$(USE_PWM)-$(CFG_ARM_CM3) += Pwm.o
obj-$(USE_PWM)-$(CFG_MPC5516) += Pwm.o
obj-$(USE_PWM)-$(CFG_MPC5567) += Pwm.o
obj-$(USE_PWM)-$(CFG_MPC563XM) += Pwm.o

#for Pi_pwm
obj-$(USE_PWM)-$(CFG_ARM_V6) += Pwm.o

# Misc
obj-$(USE_DET) += Det.o
obj-$(USE_DET) += Det_Cfg.o

# Lin
obj-$(USE_LIN) += Lin_PBcfg.o
obj-$(USE_LIN) += Lin_Lcfg.o
obj-$(USE_LIN)-$(CFG_MPC560X) += LinFlex.o
obj-$(USE_LIN)-$(CFG_MPC5744P) += LinFlex.o
obj-$(USE_LIN)-$(CFG_MPC5516) += Lin.o
obj-$(USE_LIN)-$(CFG_MPC5567) += Lin.o
obj-$(USE_LIN)-$(CFG_MPC563XM) += Lin.o

# LinIf
obj-$(USE_LINIF) += LinIf_Lcfg.o
obj-$(USE_LINIF) += LinIf_PBcfg.o
obj-$(USE_LINIF) += LinIf.o

# LinSm
obj-$(USE_LINSM) += LinSM_Lcfg.o
obj-$(USE_LINSM) += LinSM_Cfg.o
obj-$(USE_LINSM) += LinSM.o

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
obj-$(USE_NM_EXTENSION) += Nm_Extension.o
inc-$(USE_NM) += $(ROOTDIR)/communication/Nm
vpath-$(USE_NM) += $(ROOTDIR)/communication/Nm

# CanNm
obj-$(USE_CANNM) += CanNm.o
obj-$(USE_CANNM) += CanNm_Cfg.o
obj-$(USE_CANNM) += CanNm_PBCfg.o
inc-$(USE_CANNM) += $(ROOTDIR)/communication/CanNm
vpath-$(USE_CANNM) += $(ROOTDIR)/communication/CanNm

# CanSm
obj-$(USE_CANSM) += CanSM.o
obj-$(USE_CANSM) += CanSM_LCfg.o
obj-$(USE_CANSM_EXTENSION) += CanSM_Extension.o
inc-$(USE_CANSM) += $(ROOTDIR)/communication/CanSM
vpath-$(USE_CANSM) += $(ROOTDIR)/communication/CanSM

# Com
obj-$(USE_COM) += Com_Cfg.o
obj-$(USE_COM) += Com_PbCfg.o
obj-$(USE_COM) += Com_Com.o
obj-$(USE_COM) += Com_Sched.o
obj-$(USE_COM) += Com.o
obj-$(USE_COM) += Com_misc.o
inc-$(USE_PDUR) += $(ROOTDIR)/communication/Com
inc-$(USE_COM) += $(ROOTDIR)/communication/Com
vpath-$(USE_COM) += $(ROOTDIR)/communication/Com

# PduR
obj-$(USE_PDUR) += PduR_CanNm.o
obj-$(USE_PDUR) += PduR_Com.o
obj-$(USE_PDUR) += PduR_Logic.o
obj-$(USE_PDUR) += PduR_LinIf.o
obj-$(USE_PDUR) += PduR_PbCfg.o
obj-$(USE_PDUR) += PduR_Cfg.o
obj-$(USE_PDUR) += PduR_CanIf.o
obj-$(USE_PDUR) += PduR_CanTp.o
obj-$(USE_PDUR) += PduR_J1939Tp.o
obj-$(USE_PDUR) += PduR_IpduM.o
obj-$(USE_PDUR) += PduR_Dcm.o
obj-$(USE_PDUR) += PduR_SoAd.o
obj-$(USE_PDUR) += PduR_Routing.o
obj-$(USE_PDUR) += PduR.o
inc-$(USE_PDUR) += $(ROOTDIR)/communication/PduR
inc-$(USE_COM) += $(ROOTDIR)/communication/PduR
vpath-$(USE_PDUR) += $(ROOTDIR)/communication/PduR

# IpduM
obj-$(USE_IPDUM) += IpduM.o
obj-$(USE_IPDUM) += IpduM_PbCfg.o
inc-$(USE_IPDUM) += $(ROOTDIR)/communication/IpduM
vpath-$(USE_IPDUM) += $(ROOTDIR)/communication/IpduM

# IO Hardware Abstraction
vpath-$(USE_IOHWAB) += $(ROOTDIR)/system/IoHwAb
obj-$(USE_IOHWAB) += IoHwAb_Digital.o
obj-$(USE_IOHWAB) += IoHwAb_Analog.o
obj-$(USE_IOHWAB) += IoHwAb_Pwm.o
obj-$(USE_IOHWAB)-$(CFG_IOHWAB_USE_SERVICE_COMPONENT) += IoHwAb_ServiceComponent.o

#Dem
obj-$(USE_DEM) += Dem.o
obj-$(USE_DEM_EXTENSION) += Dem_Extension.o
obj-$(USE_DEM) += Dem_LCfg.o
inc-$(USE_DEM) += $(ROOTDIR)/diagnostic/Dem
vpath-$(USE_DEM) += $(ROOTDIR)/diagnostic/Dem

#Dcm
obj-$(USE_DCM) += Dcm.o
obj-$(USE_DCM) += Dcm_Dsp.o
obj-$(USE_DCM) += Dcm_Dsd.o
obj-$(USE_DCM) += Dcm_Dsl.o
obj-$(USE_DCM) += Dcm_LCfg.o
ifeq ($(filter Dcm_Callout_Stubs.o,$(obj-y)),)
obj-$(USE_DCM) += Dcm_Callout_Stubs.o
endif

inc-$(USE_DCM) += $(ROOTDIR)/diagnostic/Dcm
vpath-$(USE_DCM) += $(ROOTDIR)/diagnostic/Dcm

obj-$(USE_RAMLOG) += ramlog.o

# Common stuff, if speciied
VPATH += $(ROOTDIR)/common

# SoAd
obj-$(USE_SOAD) += SoAd_Cfg.o
obj-$(USE_SOAD) += SoAd.o
obj-$(USE_SOAD) += SoAd_DoIp.o
inc-$(USE_SOAD) += $(ROOTDIR)/communication/SoAd
inc-$(USE_SOAD) += $(ROOTDIR)/communication/SoAd
vpath-$(USE_SOAD) += $(ROOTDIR)/communication/SoAd


# UdpNm
obj-$(USE_UDPNM) += UdpNm_LCfg.o
obj-$(USE_UDPNM) += UdpNm.o
inc-$(USE_DCM) += $(ROOTDIR)/communication/UdpNm
vpath-$(USE_UDPNM) += $(ROOTDIR)/communication/UdpNm

#LWIP
LWIP_VERSION?=1.5.0-beta

inc-$(USE_LWIP) += $(ROOTDIR)/communication/lwip-$(LWIP_VERSION)/contrib/port/ArcticCore
inc-$(USE_LWIP) += $(ROOTDIR)/communication/lwip-$(LWIP_VERSION)/src/include
inc-$(USE_LWIP) += $(ROOTDIR)/communication/lwip-$(LWIP_VERSION)/src/include/ipv4
inc-$(USE_LWIP) += $(ROOTDIR)/communication/lwip-$(LWIP_VERSION)/src/include/ipv6

ifeq ($(CFG_ARM_CM3),y)
inc-$(USE_LWIP) += $(ROOTDIR)/communication/lwip-$(LWIP_VERSION)/contrib/port/ArcticCore/arm_cm3
vpath-$(USE_LWIP) += $(ROOTDIR)/communication/lwip-$(LWIP_VERSION)/contrib/port/ArcticCore/arm_cm3
endif

ifeq ($(CFG_ARM_V6),y)
inc-$(USE_LWIP) += $(ROOTDIR)/communication/lwip-$(LWIP_VERSION)/contrib/port/ArcticCore/arm_v6
vpath-$(USE_LWIP) += $(ROOTDIR)/communication/lwip-$(LWIP_VERSION)/contrib/port/ArcticCore/arm_v6
endif

ifeq ($(CFG_MPC5567),y)
inc-$(USE_LWIP) += $(ROOTDIR)/communication/lwip-$(LWIP_VERSION)/contrib/port/ArcticCore/mpc5xxx
vpath-$(USE_LWIP) += $(ROOTDIR)/communication/lwip-$(LWIP_VERSION)/contrib/port/ArcticCore/mpc5xxx
obj-$(USE_LWIP) += fec_5xxx.o
endif

obj-$(USE_LWIP) += sys_arch.o
obj-$(USE_LWIP) += lwip_handler.o
obj-$(USE_LWIP) += ethernetif.o
vpath-$(USE_LWIP) += $(ROOTDIR)/communication/lwip-$(LWIP_VERSION)/contrib/port/ArcticCore
#obj-$(USE_LWIP) += httpd.o
obj-$(USE_LWIP) += mbox.o

vpath-$(USE_LWIP) += $(ROOTDIR)/communication/lwip-$(LWIP_VERSION)/src/core/ipv4
vpath-$(USE_LWIP) += $(ROOTDIR)/communication/lwip-$(LWIP_VERSION)/src/core/ipv6

#ipv6
obj-$(USE_LWIP) += dhcp6.o
obj-$(USE_LWIP) += icmp6.o
obj-$(USE_LWIP) += ethip6.o
obj-$(USE_LWIP) += inet6.o
obj-$(USE_LWIP) += ip6_addr.o
obj-$(USE_LWIP) += ip6_frag.o
obj-$(USE_LWIP) += ip6.o
obj-$(USE_LWIP) += mld6.o
obj-$(USE_LWIP) += nd6.o
#ipv4
obj-$(USE_LWIP) += autoip.o
obj-$(USE_LWIP) += icmp.o
obj-$(USE_LWIP) += igmp.o
obj-$(USE_LWIP) += ip_frag.o
obj-$(USE_LWIP) += ip4_addr.o
obj-$(USE_LWIP) += ip4.o

vpath-$(USE_LWIP) += $(ROOTDIR)/communication/lwip-$(LWIP_VERSION)/src/core
obj-$(USE_LWIP) += inet_chksum.o
obj-$(USE_LWIP) += init_lwip.o
obj-$(USE_LWIP) += dns.o
obj-$(USE_LWIP) += dhcp.o
obj-$(USE_LWIP) += mem.o
obj-$(USE_LWIP) += memp.o
obj-$(USE_LWIP) += netif.o
obj-$(USE_LWIP) += pbuf.o
obj-$(USE_LWIP) += raw.o
obj-$(USE_LWIP) += stats.o
obj-$(USE_LWIP) += sys.o
obj-$(USE_LWIP) += tcp.o
obj-$(USE_LWIP) += tcp_in.o
obj-$(USE_LWIP) += tcp_out.o
obj-$(USE_LWIP) += udp.o
obj-$(USE_LWIP) += timers.o
obj-$(USE_LWIP) += netbios.o
obj-$(USE_LWIP) += def.o

vpath-$(USE_LWIP) += $(ROOTDIR)/communication/lwip-$(LWIP_VERSION)/src/api
obj-$(USE_LWIP) += api_msg.o
obj-$(USE_LWIP) += api_lib.o
obj-$(USE_LWIP) += err.o
obj-$(USE_LWIP) += netbuf.o
obj-$(USE_LWIP) += netdb.o
obj-$(USE_LWIP) += netifapi.o
obj-$(USE_LWIP) += sockets.o
obj-$(USE_LWIP) += tcpip.o
vpath-$(USE_LWIP) += $(ROOTDIR)/communication/lwip-$(LWIP_VERSION)/src/netif
obj-$(USE_LWIP) += etharp.o
obj-$(USE_LWIP) += sleep.o

#TCF
obj-$(USE_TCF) += tcf.o
obj-$(USE_TCF) += Tcf_Cfg.o
obj-$(USE_TCF) += sys_monitor.o
obj-$(USE_TCF) += streams.o
inc-$(USE_TCF) += $(ROOTDIR)/common/tcf
vpath-$(USE_TCF) += $(ROOTDIR)/common/tcf

#SLEEP
obj-$(USE_SLEEP) += sleep.o

# Circular Buffer (always)
obj-y += cirq_buffer.o

ifeq ($(CFG_TIMER),y)
obj-$(CFG_TIMER_TB)-$(CFG_PPC)+=timer_tb.o
obj-$(CFG_TIMER_RTC)-$(CFG_PPC)+=timer_rtc.o
obj-$(CFG_TIMER_DWT)-$(CFG_ARM)+=timer_dwt.o
endif
obj-$(CFG_SHELL)+=shell.o
SELECT_CLIB?=CLIB_IAR

# Performance stuff
obj-$(CFG_OS_PERF)+=perf.o
def-$(CFG_OS_PERF)+=CFG_OS_ISR_HOOKS


ifeq ($(CFG_ARC_CLIB),y)
  # Just use native clib
    # Override native C-library with ArcCore tweaks.
  inc-system-y += $(ROOTDIR)/clib
  vpath-y      += $(ROOTDIR)/clib

  obj-$(USE_TTY_T32)     += serial_dbg_t32.o  
  obj-$(USE_TTY_UDE)     += serial_dbg_ude.o
  obj-$(USE_TTY_WINIDEA) += serial_dbg_winidea.o

  obj-y += clib_port.o
  obj-y += clib.o
  obj-y += printf.o
  obj-y += xtoa.o
  obj-y-cw += strtok_r.o
  obj-y-diab += strtok_r.o  
else
  ifeq ($(SELECT_CLIB),CLIB_IAR)
    # This is not good, but don't know what to do right now....
    obj-y += iar_port.o
    obj-y += xtoa.o
    obj-y += printf.o
  else ifeq ($(SELECT_CLIB),CLIB_CW)
    # This is not good, but don't know what to do right now....
    obj-y += xtoa.o
    obj-y += msl_port.o
  endif    
endif # SELECT_CLIB 

obj-y += $(obj-y-y)
obj-y += $(obj-y-$(COMPILER))


vpath-y += $(ROOTDIR)/$(ARCH_PATH-y)/kernel
vpath-y += $(ROOTDIR)/$(ARCH_PATH-y)/drivers
vpath-y += $(board_path)
vpath-y += $(ROOTDIR)/arch/$(ARCH_FAM)
vpath-y += $(board_path)/config
vpath-y += $(ROOTDIR)/diagnostic/Dem
vpath-y += $(ROOTDIR)/diagnostic/Dcm
vpath-y += $(ROOTDIR)/diagnostic/Det

# include files need by us
inc-y += $(ROOTDIR)/include
inc-y += $(ROOTDIR)/system/kernel/test
inc-y += $(ROOTDIR)/system/kernel/include
inc-y += $(ROOTDIR)/$(ARCH_PATH-y)/kernel
inc-y += $(ROOTDIR)/$(ARCH_PATH-y)/drivers
inc-y += $(board_path)/config


#
# And last the generic board
#
inc-y += $(ROOTDIR)/boards/generic
vpath-y += $(ROOTDIR)/boards/generic


VPATH += $(vpath-y)
#$(error $(VPATH))


