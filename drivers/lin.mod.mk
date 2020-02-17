# Lin
obj-$(USE_LIN) += Lin_PBcfg.o
obj-$(USE_LIN) += Lin_Lcfg.o
obj-$(USE_LIN)-$(CFG_MPC560X) += LinFlex_mpc5xxx.o
obj-$(USE_LIN)-$(CFG_MPC5744P) += LinFlex_mpc5xxx.o
obj-$(USE_LIN)-$(CFG_MPC5746C) += LinFlex_mpc5xxx.o
obj-$(USE_LIN)-$(CFG_MPC5748G) += LinFlex_mpc5xxx.o
obj-$(USE_LIN)-$(CFG_MPC5777M) += LinFlex_mpc5xxx.o
obj-$(USE_LIN)-$(CFG_MPC5643L) += LinFlex_mpc5xxx.o
obj-$(USE_LIN)-$(CFG_MPC5646B) += LinFlex_mpc5xxx.o
obj-$(USE_LIN)-$(CFG_SPC56XL70) += LinFlex_mpc5xxx.o
obj-$(USE_LIN)-$(CFG_MPC5645S) += LinFlex_mpc5xxx.o
obj-$(USE_LIN)-$(CFG_MPC560X) += LinFlex_Common.o
obj-$(USE_LIN)-$(CFG_MPC5744P) += LinFlex_Common.o
obj-$(USE_LIN)-$(CFG_MPC5746C) += LinFlex_Common.o
obj-$(USE_LIN)-$(CFG_MPC5748G) += LinFlex_Common.o
obj-$(USE_LIN)-$(CFG_MPC5777M) += LinFlex_Common.o
obj-$(USE_LIN)-$(CFG_MPC5643L) += LinFlex_Common.o
obj-$(USE_LIN)-$(CFG_SPC56XL70) += LinFlex_Common.o
obj-$(USE_LIN)-$(CFG_MPC5645S) += LinFlex_Common.o
obj-$(USE_LIN)-$(CFG_MPC5516) += Lin_mpc5xxx.o
obj-$(USE_LIN)-$(CFG_MPC5567) += Lin_mpc5xxx.o
obj-$(USE_LIN)-$(CFG_MPC563XM) += Lin_mpc5xxx.o
obj-$(USE_LIN)-$(CFG_MPC5644A) += Lin_mpc5xxx.o
obj-$(USE_LIN)-$(CFG_MPC5668) += Lin_mpc5xxx.o
obj-$(USE_LIN)-$(CFG_ZYNQ) += Lin_zynq.o
obj-$(USE_LIN)-$(CFG_JAC6) += Lin_jacinto.o
obj-$(USE_LIN)-$(CFG_TMS570) += Lin.o
obj-$(USE_LIN)-$(CFG_TMS570) += Lin_Hw_tms570.o
obj-$(USE_LIN)-$(CFG_TMS570) += Lin_Irq.o

vpath-$(USE_LIN)+= $(ROOTDIR)/mcal/Lin/src
inc-$(USE_LIN) += $(ROOTDIR)/mcal/Lin/inc
inc-$(USE_LIN) += $(ROOTDIR)/mcal/Lin/src


#Jacinto6 reqiures timer for hwbug
#obj-$(USE_LIN)-$(CFG_JAC6) += Timer.o