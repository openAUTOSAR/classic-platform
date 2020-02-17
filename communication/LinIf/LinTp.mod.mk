# LinIf
obj-$(USE_LINTP) += LinTp_Lcfg.o
obj-$(USE_LINTP) += LinTp_PBcfg.o
obj-$(USE_LINTP) += LinTp.o

inc-$(USE_LINTP) += $(ROOTDIR)/communication/LinIf/inc
vpath-$(USE_LINTP) += $(ROOTDIR)/communication/LinIf/src