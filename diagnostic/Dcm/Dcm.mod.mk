#Dcm
obj-$(USE_DCM) += Dcm.o
obj-$(USE_DCM) += Dcm_Dsp.o
obj-$(USE_DCM) += Dcm_Dsd.o
obj-$(USE_DCM) += Dcm_Dsl.o
obj-$(USE_DCM) += Dcm_ROE.o
obj-$(USE_DCM) += Dcm_Internal.o
obj-$(USE_DCM) += Dcm_LCfg.o
ifeq ($(filter Dcm_Callout_Stubs.o,$(obj-y)),)
obj-$(USE_DCM) += Dcm_Callout_Stubs.o
endif

inc-$(USE_DCM) += $(ROOTDIR)/diagnostic/Dcm/inc
inc-$(USE_DCM) += $(ROOTDIR)/diagnostic/Dcm/src
vpath-$(USE_DCM) += $(ROOTDIR)/diagnostic/Dcm/src