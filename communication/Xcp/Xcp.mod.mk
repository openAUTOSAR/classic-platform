# Xcp
obj-$(USE_XCP) += Xcp.o
obj-$(USE_XCP) += Xcp_FlashProgramming.o
obj-$(USE_XCP) += Xcp_OnlineCalibration.o
obj-$(USE_XCP) += Xcp_MemoryHelper.o
obj-$(USE_XCP) += Xcp_Cfg.o

obj-$(USE_XCP)-$(USE_CANIF) += XcpOnCan.o
obj-$(USE_XCP)-$(USE_SOAD)  += XcpOnEth.o

ifeq ($(filter Xcp_Callout_Stubs.o,$(obj-y)),)
obj-$(USE_XCP) += Xcp_Callout_Stubs.o
endif

inc-$(USE_XCP) += $(ROOTDIR)/communication/Xcp/inc
inc-$(USE_XCP) += $(ROOTDIR)/communication/Xcp/src
vpath-$(USE_XCP) += $(ROOTDIR)/communication/Xcp/src
vpath-$(USE_XCP) += $(ROOTDIR)/communication/Xcp/integration
