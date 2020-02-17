#WdgIf
obj-$(USE_WDGIF) += WdgIf.o
obj-$(USE_WDGIF) += WdgIf_Cfg.o

inc-y += $(ROOTDIR)/safety_security/WdgIf/inc
vpath-y += $(ROOTDIR)/safety_security/WdgIf/src