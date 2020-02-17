#TCPIP

ifeq ($(CFG_GNULINUX),y)
	obj-$(USE_TCPIP)-$(CFG_GNULINUX) += TcpIp_Linos.o
	inc-$(USE_TCPIP)-$(CFG_GNULINUX) += $(ROOTDIR)/mcal/Eth/inc
else
	obj-$(USE_TCPIP) += TcpIp.o
endif
obj-$(USE_TCPIP) += TcpIp_Cfg.o


inc-$(USE_TCPIP) += $(ROOTDIR)/communication/TcpIp/inc
vpath-$(USE_TCPIP) += $(ROOTDIR)/communication/TcpIp/src

