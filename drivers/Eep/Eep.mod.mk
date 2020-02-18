#Eep
obj-$(USE_EEP) += Eep.o
obj-$(USE_EEP) += Eep_Lcfg.o
inc-$(USE_EEP)-$(if $(CFG_JACINTO)$(CFG_RH850)$(CFG_MPC55XX)$(CFG_MPC56XX)$(CFG_TMS570),y) += $(ROOTDIR)/drivers/Eep
vpath-$(USE_EEP)-$(if $(CFG_JACINTO)$(CFG_RH850)$(CFG_MPC55XX)$(CFG_MPC56XX)$(CFG_TMS570),y) += $(ROOTDIR)/drivers/Eep

