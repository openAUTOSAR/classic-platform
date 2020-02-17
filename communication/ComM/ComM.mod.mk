# ComM
obj-$(USE_COMM) += ComM.o
obj-$(USE_COMM) += ComM_Cfg.o
obj-$(USE_COMM) += ComM_ASIL.o
pb-obj-$(USE_COMM) += ComM_PBcfg.o
pb-pc-file-$(USE_COMM) += ComM_Cfg.h ComM_Cfg.c


inc-$(USE_COMM) += $(ROOTDIR)/communication/ComM/inc
inc-$(USE_COMM) += $(ROOTDIR)/communication/ComM/src
vpath-$(USE_COMM) += $(ROOTDIR)/communication/ComM/src