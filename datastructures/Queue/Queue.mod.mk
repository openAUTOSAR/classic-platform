ifneq ($(filter y,$(USE_RTE) $(USE_QUEUE)),)
obj-$(USE_QUEUE) += Queue.o


inc-y += $(ROOTDIR)/datastructures/Queue/inc
vpath-y += $(ROOTDIR)/datastructures/Queue/src
endif