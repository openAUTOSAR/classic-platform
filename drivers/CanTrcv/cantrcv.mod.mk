#CanTrcv
vpath-$(USE_CANTRCV) += $(ROOTDIR)/drivers/CanTrcv
inc-$(USE_CANTRCV) += $(ROOTDIR)/drivers/Can
inc-$(USE_CANTRCV) += $(ROOTDIR)/drivers/CanTrcv
obj-$(USE_CANTRCV) += CanTrcv.o
obj-$(USE_CANTRCV) += CanTrcv_Cfg.o
pb-obj-$(USE_CANTRCV) += CanTrcv_PBcfg.o
pb-pc-file-$(USE_CANTRCV) += CanTrcv_Cfg.h CanTrcv_Cfg.c
vpath-$(USE_CANTRCV)-$(CFG_TJA1145) += $(ROOTDIR)/arch/transceivers/tja1145
inc-$(USE_CANTRCV)-$(CFG_TJA1145) += $(ROOTDIR)/arch/transceivers/tja1145
obj-$(USE_CANTRCV)-$(CFG_TJA1145) += CanTrcv_TJA1145.o