


# Gpt
obj-y += Gpt.o
obj-y += Gpt_Cfg.o

# Mcu
obj-y += Mcu.o
obj-y += Mcu_Cfg.o
obj-y += Mcu_Exceptions.o

# Can
obj-y += Can.o
obj-y += Can_Lcfg.o
obj-y += CanIf.o
obj-y += CanIf_Cfg.o

obj-y += Dio.o
obj-y += Dio_Lcfg.o

obj-y += Port.o
obj-y += Port_Cfg.o

# Spi
obj-y += Spi.o
obj-y += Spi_Lcfg.o

#Eep
obj-y += Eep.o
obj-y += Eep_Lcfg.o
obj-y += Eeprom_Lcfg.o

#Fls ext
obj-y += Fls_SST25xx.o
obj-y += Fls_SST25xx_Cfg.o

# Misc
obj-y += Det.o

# Lin
obj-y += Lin_PBcfg.o
obj-y += Lin_Lcfg.o
obj-y += LinIf_Lcfg.o
obj-y += LinIf_PBcfg.o
obj-y += LinSM_Lcfg.o
obj-y += LinSM_PBcfg.o
obj-y += LinSM_Cfg.o
obj-y += Lin.o
obj-y += LinIf.o
obj-y += LinSM.o

# ComM
obj-y += ComM.o

# Com
obj-y += Com.o


# Common
obj-y += xtoa.o
obj-y += ramlog.o
VPATH += $(ROOTDIR)/common


VPATH += $(ROOTDIR)/arch/$(ARCH)/kernel
VPATH += $(ROOTDIR)/arch/$(ARCH)/drivers
#VPATH += $(ROOTDIR)/arch/$(ARCH)/config
VPATH += $(ROOTDIR)/boards/$(BOARDDIR)/config
VPATH += $(ROOTDIR)/drivers
VPATH += $(ROOTDIR)/drivers/config
VPATH += $(ROOTDIR)/drivers/Lin
VPATH += $(ROOTDIR)/drivers/ComM
VPATH += $(ROOTDIR)/drivers/Com


inc-y += $(ROOTDIR)/drivers/PduR
inc-y += $(ROOTDIR)/drivers/Com
inc-y += $(ROOTDIR)/drivers/ComM
inc-y += $(ROOTDIR)/drivers/config
inc-y += $(ROOTDIR)/boards/$(BOARDDIR)/config
inc-y += $(ROOTDIR)/drivers/include
inc-y += $(ROOTDIR)/include
#inc-y += $(ROOTDIR)/arch/$(ARCH)/config
inc-y += $(ROOTDIR)/arch/$(ARCH)/drivers
inc-y += $(ROOTDIR)/arch/$(ARCH)/kernel

inc-y += $(ROOTDIR)/embunit/embUnit
inc-y += $(ROOTDIR)/embunit/textui
inc-y += $(ROOTDIR)/embunit


#libs needed by us
libitem-y += $(ROOTDIR)/embunit/embUnit/obj_$(ARCH)/libembunit.a
libitem-y += $(ROOTDIR)/embunit/textui/obj_$(ARCH)/libtextui.a

#linkfile
ldcmdfile-$(CFG_MPC55XX) = $(ROOTDIR)/arch/$(ARCH)/scripts/linkscript_$(COMPILER).ldf

build-exe-y = ../../../$(target)_$(ARCH).$(TE)


