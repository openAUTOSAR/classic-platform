#CanIf
obj-$(USE_CANIF) += CanIf.o
obj-$(USE_CANIF) += CanIf_Cfg.o
pb-obj-$(USE_CANIF) += CanIf_PBCfg.o
pb-pc-file-$(USE_CANIF) += CanIf_Cfg.h CanIf_Cfg.c

vpath-$(USE_CANIF) += $(ROOTDIR)/communication/CanIf/src
inc-$(USE_CANIF) += $(ROOTDIR)/communication/CanIf/inc
