#EcuMFixed
obj-$(USE_ECUM_FIXED) += EcuM.o
obj-$(USE_ECUM_FIXED) += EcuM_Fixed.o
obj-$(USE_ECUM_FIXED) += EcuM_Main.o
obj-$(USE_ECUM_FIXED) += EcuM_PBcfg.o
obj-$(USE_ECUM_FIXED) += EcuM_PBHeader.o
obj-$(USE_ECUM_FIXED)-$(CFG_SAFETY_PLATFORM) += EcuM_A0.o
obj-$(USE_ECUM_FIXED)-$(CFG_SAFETY_PLATFORM) += EcuM_QM.o
obj-$(USE_ECUM_FIXED) += EcuM_GeneratedCallouts.o

ifeq ($(filter EcuM_Callout_Stubs.o,$(obj-y)),)
obj-$(USE_ECUM_FIXED) += EcuM_Callout_Stubs.o
endif
ifeq ($(filter EcuM_SP_Callout_Stubs.o,$(obj-y)),)
obj-$(USE_ECUM_FIXED)-$(CFG_SAFETY_PLATFORM) += EcuM_SP_Callout_Stubs.o
endif
inc-$(USE_ECUM_FIXED) += $(ROOTDIR)/system/EcuM/inc
inc-$(USE_ECUM_FIXED) += $(ROOTDIR)/system/EcuM/src
vpath-$(USE_ECUM_FIXED) += $(ROOTDIR)/system/EcuM/src

#EcuMFlexible
obj-$(USE_ECUM_FLEXIBLE) += EcuM.o
obj-$(USE_ECUM_FLEXIBLE) += EcuM_Flexible.o
obj-$(USE_ECUM_FLEXIBLE) += EcuM_PBcfg.o
obj-$(USE_ECUM_FLEXIBLE) += EcuM_PBHeader.o
obj-$(USE_ECUM_FLEXIBLE) += EcuM_GeneratedCallouts.o
ifeq ($(filter EcuM_Callout_Stubs.o,$(obj-y)),)
obj-$(USE_ECUM_FLEXIBLE) += EcuM_Callout_Stubs.o
endif
ifeq ($(filter EcuM_SP_Callout_Stubs.o,$(obj-y)),)
obj-$(USE_ECUM_FLEXIBLE)-$(CFG_SAFETY_PLATFORM) += EcuM_SP_Callout_Stubs.o
endif
inc-$(USE_ECUM_FLEXIBLE) += $(ROOTDIR)/system/EcuM/inc
inc-$(USE_ECUM_FLEXIBLE) += $(ROOTDIR)/system/EcuM/src
vpath-$(USE_ECUM_FLEXIBLE) += $(ROOTDIR)/system/EcuM/src