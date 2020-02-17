# Ocu
obj-$(USE_OCU) += Ocu_mpc5xxx.o
obj-$(USE_OCU) += Ocu_PBcfg.o
obj-$(USE_OCU) += Ocu_Irq.o


vpath-$(USE_OCU)+= $(ROOTDIR)/mcal/Ocu/src
inc-$(USE_OCU) += $(ROOTDIR)/mcal/Ocu/inc
