#MemIf
obj-$(USE_MEMIF) += MemIf.o

inc-$(USE_MEMIF) += $(ROOTDIR)/memory/MemIf/inc
vpath-$(USE_MEMIF) += $(ROOTDIR)/memory/MemIf/src