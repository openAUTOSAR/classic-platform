ifeq ($(BOARDDIR),rh850f1h)
obj-y += CDD_EthTrcv_NonGeneric.o
else ifeq ($(BOARDDIR),mpc5748g_mpc574xg_mb)
obj-y += CDD_EthTrcv_NonGeneric.o
else ifeq ($(BOARDDIR),mpc5746c_mpc574xg_mb)
obj-y += CDD_EthTrcv_NonGeneric.o
else ifeq ($(BOARDDIR),mpc5748g_mpc574xg_mb)
obj-y += CDD_EthTrcv_NonGeneric.o
else ifeq ($(BOARDDIR),mpc5777c_mpc57xx_mb)
obj-y += CDD_EthTrcv_NonGeneric.o
else ifeq ($(BOARDDIR),jacinto6)
obj-y += CDD_EthTrcv_NonGeneric.o
else ifeq ($(BOARDDIR),s32k148_evbq176)
obj-y += CDD_EthTrcv_NonGeneric.o
else
obj-y += CDD_EthTrcv_Generic.o
endif
obj-$(USE_CDDETHTRCV) += CddEthTrcv_Cfg.o
inc-y +=   $(ROOTDIR)/communication/EthTrcv/inc
inc-y +=   $(ROOTDIR)/cdd/EthTrcv/inc
vpath-y += $(ROOTDIR)/cdd/EthTrcv/src
