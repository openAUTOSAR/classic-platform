# UdpNm
obj-$(USE_UDPNM) += UdpNm_PBCfg.o
obj-$(USE_UDPNM) += UdpNm.o

inc-$(USE_UDPNM) += $(ROOTDIR)/communication/UdpNm/inc
vpath-$(USE_UDPNM) += $(ROOTDIR)/communication/UdpNm/src