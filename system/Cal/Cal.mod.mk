#CAL
obj-$(USE_CAL) += Cal_Mac.o
obj-$(USE_CAL) += Cal_Cfg.o

inc-$(USE_CAL) += $(ROOTDIR)/system/Cal/inc
vpath-$(USE_CAL) += $(ROOTDIR)/system/Cal/src