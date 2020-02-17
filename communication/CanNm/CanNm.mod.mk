# CanNm
obj-$(USE_CANNM) += CanNm.o
obj-$(USE_CANNM) += CanNm_Cfg.o
pb-obj-$(USE_CANNM) += CanNm_PBCfg.o
pb-pc-file-$(USE_CANNM) += CanNm_Cfg.c CanNm_Cfg.h

inc-$(USE_CANNM) += $(ROOTDIR)/communication/CanNm/inc
inc-$(USE_CANNM) += $(ROOTDIR)/communication/CanNm/src
vpath-$(USE_CANNM) += $(ROOTDIR)/communication/CanNm/src
