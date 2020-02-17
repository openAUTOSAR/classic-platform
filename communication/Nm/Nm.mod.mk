# Nm
obj-$(USE_NM) += Nm.o
obj-$(USE_NM) += Nm_Cfg.o
ifeq ($(filter Nm_Extension.o,$(obj-y)),)
obj-$(USE_NM_EXTENSION) += Nm_Extension.o
endif

inc-$(USE_NM) += $(ROOTDIR)/communication/Nm/inc
vpath-$(USE_NM) += $(ROOTDIR)/communication/Nm/src