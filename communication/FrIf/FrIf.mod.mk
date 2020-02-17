#FrIf
obj-$(USE_FRIF) += FrIf.o
obj-$(USE_FRIF) += FrIf_Cfg.o
obj-$(USE_FRIF) += FrIf_Lcfg.o
obj-$(USE_FRIF) += FrIf_PBcfg.o
obj-$(USE_FRIF) += FrIf_Internal.o

vpath-$(USE_FRIF) += $(ROOTDIR)/communication/FrIf/src
inc-$(USE_FRIF) += $(ROOTDIR)/communication/FrIf/inc

