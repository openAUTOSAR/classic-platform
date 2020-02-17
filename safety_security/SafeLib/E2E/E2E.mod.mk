#E2E
obj-$(USE_E2E) += E2E.o
obj-$(USE_E2E) += E2E_P01.o
obj-$(USE_E2E) += E2E_P02.o
obj-$(USE_E2E) += E2E_P04.o
obj-$(USE_E2E) += E2E_P05.o
obj-$(USE_E2E) += E2E_P06.o
obj-$(USE_E2E) += E2E_SM.o
inc-$(USE_E2E) += $(ROOTDIR)/safety_security/SafeLib/E2E/inc
inc-$(USE_E2E) += $(ROOTDIR)/safety_security/SafeLib/E2E/src
vpath-$(USE_E2E) += $(ROOTDIR)/safety_security/SafeLib/E2E/src