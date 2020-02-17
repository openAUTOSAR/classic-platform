#ICU

obj-$(USE_ICU) += Icu_Lcfg.o
obj-$(USE_ICU) += Icu_PBcfg.o

obj-$(USE_ICU)-$(CFG_MPC5645S) += Icu_mpc5xxx.o
obj-$(USE_ICU)-$(CFG_MPC5746C) += Icu_mpc5xxx.o
obj-$(USE_ICU)-$(CFG_MPC5646B) += Icu_mpc5xxx.o
obj-$(USE_ICU)-$(CFG_MPC560X) += Icu_mpc5xxx.o
obj-$(USE_ICU)-$(CFG_ZYNQ) += Icu_zynq.o
obj-$(USE_ICU)-$(CFG_MPC5748G) += Icu_mpc5xxx.o
vpath-$(USE_ICU)+= $(ROOTDIR)/mcal/Icu/src
inc-$(USE_ICU) += $(ROOTDIR)/mcal/Icu/inc
