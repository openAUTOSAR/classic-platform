# SoAd
obj-$(USE_SOAD) += SoAd_Cfg.o
obj-$(USE_SOAD) += SoAd.o


inc-$(USE_SOAD) += $(ROOTDIR)/communication/SoAd/inc
vpath-$(USE_SOAD) += $(ROOTDIR)/communication/SoAd/src