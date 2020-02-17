#Ethernet
obj-$(USE_ETHTSYN) += EthTSyn_Cfg.o
obj-$(USE_ETHTSYN) += EthTSyn_Internal.o
obj-$(USE_ETHTSYN) += EthTSyn.o

inc-$(USE_ETHTSYN) += $(ROOTDIR)/include/rte
vpath-$(USE_ETHTSYN) += $(ROOTDIR)/communication/EthTSyn/src
inc-$(USE_ETHTSYN) += $(ROOTDIR)/communication/EthTSyn/inc
inc-$(USE_ETHTSYN) += $(ROOTDIR)/system/StbM/inc