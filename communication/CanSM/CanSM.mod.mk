# CanSm
obj-$(USE_CANSM) += CanSM.o
obj-$(USE_CANSM) += CanSM_Internal.o
obj-$(USE_CANSM) += CanSM_LCfg.o
ifeq ($(filter CanSM_Extension.o,$(obj-y)),)
obj-$(USE_CANSM_EXTENSION) += CanSM_Extension.o
endif

inc-$(USE_CANSM) += $(ROOTDIR)/communication/CanSM/inc
inc-$(USE_CANSM) += $(ROOTDIR)/communication/CanSM/src
vpath-$(USE_CANSM) += $(ROOTDIR)/communication/CanSM/src
