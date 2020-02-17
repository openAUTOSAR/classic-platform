#FrNm
obj-$(USE_FRNM) += FrNm.o
obj-$(USE_FRNM) += FrNm_Internal.o
obj-$(USE_FRNM) += FrNm_PBcfg.o
obj-$(USE_FRNM) += FrNm_Lcfg.o

vpath-$(USE_FRNM) += $(ROOTDIR)/communication/FrNm/src
inc-$(USE_FRNM) += $(ROOTDIR)/communication/FrNm/inc
inc-$(USE_FRNM) += $(ROOTDIR)/communication/FrNm/src