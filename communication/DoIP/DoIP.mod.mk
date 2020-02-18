# DoIP
obj-$(USE_DOIP) += DoIP_Cfg.o
obj-$(USE_DOIP) += DoIP.o
ifeq ($(filter DoIP_Callout_Stubs.o,$(obj-y)),)
obj-$(USE_DOIP) += DoIP_Callout_Stubs.o
endif


inc-$(USE_DOIP) += $(ROOTDIR)/communication/DoIP/inc
vpath-$(USE_DOIP) += $(ROOTDIR)/communication/DoIP/src

