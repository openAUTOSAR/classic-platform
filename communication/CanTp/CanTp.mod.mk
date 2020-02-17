#CanTp
obj-$(USE_CANTP) += CanTp.o
pb-obj-$(USE_CANTP) += CanTp_PBCfg.o
pb-pc-file-$(USE_CANTP) += CanTp_Cfg.h

vpath-$(USE_CANTP) += $(ROOTDIR)/communication/CanTp/src
inc-$(USE_CANTP) += $(ROOTDIR)/communication/CanTp/inc