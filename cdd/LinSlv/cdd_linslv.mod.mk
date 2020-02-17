#CDD_LinSlv
obj-$(USE_CDD_LINSLV) += CDD_LinSlv.o
obj-$(USE_CDD_LINSLV) += CDD_LinSlv_Internal.o
obj-$(USE_CDD_LINSLV) += CDD_LinSlv_Cfg.o
inc-$(USE_CDD_LINSLV) += $(ROOTDIR)/cdd/LinSlv/src/
vpath-$(USE_CDD_LINSLV) += $(ROOTDIR)/cdd/LinSlv/src
vpath-$(USE_CDD_LINSLV) += $(ROOTDIR)/cdd/LinSlv/src/$(ARCH_FAM)