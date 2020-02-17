#BswM

obj-$(USE_BSWM) += BswM.o
obj-$(USE_BSWM) += BswM_Cfg.o
pb-obj-$(USE_BSWM) += BswM_PBcfg.o
pb-pc-file-$(USE_BSWM) += BswM_Cfg.h BswM_Cfg.c

inc-$(USE_BSWM) += $(ROOTDIR)/system/BswM/inc
inc-$(USE_BSWM) += $(ROOTDIR)/system/BswM/src
vpath-$(USE_BSWM) += $(ROOTDIR)/system/BswM/src