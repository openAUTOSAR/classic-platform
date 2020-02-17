#Flexray driver
obj-$(USE_FR) += Fr.o
obj-$(USE_FR) += Fr_PBcfg.o
obj-$(USE_FR)-$(RH850F1H) += Fr_Internal.o
obj-$(USE_FR)-$(CFG_PPC) += Fr_mpc5xxx.o

inc-$(USE_FR) += $(ROOTDIR)/drivers/Fr
vpath-$(USE_FR) += $(ROOTDIR)/drivers/Fr