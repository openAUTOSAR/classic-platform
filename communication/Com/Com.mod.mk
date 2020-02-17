# Com
obj-$(USE_COM) += Com_Cfg.o
pb-obj-$(USE_COM) += Com_PbCfg.o
pb-pc-file-$(USE_COM) += Com_Cfg.h Com_Cfg.c

obj-$(USE_COM) += Com_Com.o
obj-$(USE_COM) += Com_Sched.o
obj-$(USE_COM) += Com.o
obj-$(USE_COM) += Com_misc.o

inc-$(USE_COM) += $(ROOTDIR)/communication/Com/inc
inc-$(USE_COM) += $(ROOTDIR)/communication/Com/src
vpath-$(USE_COM) += $(ROOTDIR)/communication/Com/src
