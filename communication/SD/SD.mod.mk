# SD
obj-$(USE_SD) += SD_Cfg.o
obj-$(USE_SD) += SD_LCfg.o
obj-$(USE_SD) += SD_PBCfg.o
obj-$(USE_SD) += SD_ClientService.o
obj-$(USE_SD) += SD_ServerService.o
obj-$(USE_SD) += SD_Send_Receive.o
obj-$(USE_SD) += SD_Entries.o
obj-$(USE_SD) += SD_Messages.o
obj-$(USE_SD) += SD.o

inc-$(USE_SD) += $(ROOTDIR)/communication/SD/inc
vpath-$(USE_SD) += $(ROOTDIR)/communication/SD/src
