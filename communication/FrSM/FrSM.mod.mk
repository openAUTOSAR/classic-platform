#FrSM
obj-$(USE_FRSM) += FrSM.o
obj-$(USE_FRSM) += FrSM_Internal.o
obj-$(USE_FRSM) += FrSM_Lcfg.o
obj-$(USE_FRSM) += FrSM_PBcfg.o

vpath-$(USE_FRSM) += $(ROOTDIR)/communication/FrSM/src
inc-$(USE_FRSM) += $(ROOTDIR)/communication/FrSM/inc