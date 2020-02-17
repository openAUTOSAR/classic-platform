# LinIf
obj-$(USE_LINIF) += LinIf_Lcfg.o
obj-$(USE_LINIF) += LinIf_PBcfg.o
obj-$(USE_LINIF) += LinIf.o

inc-$(USE_LINIF) += $(ROOTDIR)/communication/LinIf/inc
vpath-$(USE_LINIF) += $(ROOTDIR)/communication/LinIf/src