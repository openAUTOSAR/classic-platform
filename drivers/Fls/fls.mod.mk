#Flash
obj-$(USE_FLS)-$(CFG_PPC) += Fls_mpc5xxx.o
obj-$(USE_FLS)-$(CFG_TMS570) += Fls_tms570.o
obj-$(USE_FLS)-$(CFG_STM32) += Fls_stm32.o
obj-$(USE_FLS)-$(CFG_JACINTO) += Fls_jacinto.o
obj-$(USE_FLS)-$(CFG_ZYNQ) += Fls_zynq.o
obj-$(USE_FLS)-$(CFG_GNULINUX) += Fls_gnulinux.o


obj-$(USE_FLS) += Fls_Cfg.o
ifeq ($(CFG_MPC5744P)$(CFG_MPC5777M)$(CFG_MPC5777C)$(CFG_MPC5748G)$(CFG_MPC5746C),y)
obj-$(USE_FLS) += flash_c55.o
else
obj-$(CFG_MPC56XX)-$(USE_FLS) += flash_h7f_c90.o
obj-$(CFG_MPC56XX)-$(USE_FLS) += flash_ll_h7f_c90.o
obj-$(CFG_MPC55XX)-$(USE_FLS) += flash_h7f_c90.o
obj-$(CFG_MPC55XX)-$(USE_FLS) += flash_ll_h7f_c90.o
endif
vpath-$(CFG_ZYNQ)-$(USE_FLS) += $(ROOTDIR)/mcal/Fls/src/contrib/qspips_v3_1/src
inc-$(CFG_ZYNQ)-$(USE_FLS) += $(ROOTDIR)/mcal/Fls/src/contrib/qspips_v3_1/src
inc-$(CFG_ZYNQ)-$(USE_FLS) += $(ROOTDIR)/mcal/Fls/src/contrib/qspips_v3_1
vpath-$(CFG_ZYNQ)-$(USE_FLS) += $(ROOTDIR)/mcal/Fls/src/contrib/qspips_v3_1/QspiIf
inc-$(CFG_ZYNQ)-$(USE_FLS) += $(ROOTDIR)/mcal/Fls/src/contrib/qspips_v3_1/QspiIf
inc-$(CFG_ZYNQ)-$(USE_FLS) += $(ROOTDIR)/mcal/Fls/src/contrib/drivers/qspips_v3_1
#files needed for zynq QSPI
obj-$(CFG_ZYNQ)-$(USE_FLS) += QspiIf.o
obj-$(CFG_ZYNQ)-$(USE_FLS) += xqspips_g.o
obj-$(CFG_ZYNQ)-$(USE_FLS) += xqspips_hw.o
obj-$(CFG_ZYNQ)-$(USE_FLS) += xqspips_options.o
obj-$(CFG_ZYNQ)-$(USE_FLS) += xqspips_selftest.o
obj-$(CFG_ZYNQ)-$(USE_FLS) += xqspips_sinit.o
obj-$(CFG_ZYNQ)-$(USE_FLS) += xqspips.o

inc-$(USE_FLS) += $(ROOTDIR)/drivers/Fls
vpath-$(USE_FLS) += $(ROOTDIR)/drivers/Fls

vpath-$(USE_FLS) += $(ROOTDIR)/mcal/Fls/src
inc-$(USE_FLS) += $(ROOTDIR)/mcal/Fls/inc
inc-$(USE_FLS) += $(ROOTDIR)/mcal/Fls/src

