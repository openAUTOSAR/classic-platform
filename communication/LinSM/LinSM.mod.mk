#LinSM
obj-$(USE_LINSM) += LinSM_Lcfg.o
obj-$(USE_LINSM) += LinSM_Cfg.o
obj-$(USE_LINSM) += LinSM.o

inc-$(USE_LINSM) += $(ROOTDIR)/communication/LinSM/inc
vpath-$(USE_LINSM) += $(ROOTDIR)/communication/LinSM/src