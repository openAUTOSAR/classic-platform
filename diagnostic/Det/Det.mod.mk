#DET
obj-$(USE_DET) += Det.o
obj-$(USE_DET) += Det_Cfg.o
inc-$(USE_DET) += $(ROOTDIR)/diagnostic/Det/inc
inc-$(USE_DET) += $(ROOTDIR)/diagnostic/Det/src
vpath-$(USE_DET) += $(ROOTDIR)/diagnostic/Det/src