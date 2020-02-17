#WdgM
obj-$(USE_WDGM) += WdgM.o
obj-$(USE_WDGM) += WdgM_PBcfg.o

inc-y += $(ROOTDIR)/safety_security/WdgM/inc
inc-y += $(ROOTDIR)/safety_security/WdgM/src
vpath-y += $(ROOTDIR)/safety_security/WdgM/src