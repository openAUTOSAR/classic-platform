# EthIF
obj-$(USE_ETHIF) += EthIf_Lcfg.o
obj-$(USE_ETHIF) += EthIf_PBcfg.o
obj-$(USE_ETHIF) += EthIf.o

vpath-$(USE_ETHIF) += $(ROOTDIR)/communication/EthIf/src
inc-$(USE_ETHIF) += $(ROOTDIR)/communication/EthIf/inc
inc-$(USE_ETHIF) += $(ROOTDIR)/mcal/Eth/inc