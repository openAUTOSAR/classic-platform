
PROJECTNAME=kernel_extra
ROOTDIR?=../../../..
include $(ROOTDIR)/scripts/project_defaults.mk

ifneq (${MAKELEVEL},0) 

	# object files
	obj-y += system_hooks.o
	obj-y += os_kernel_extra.o
	
	VPATH += $(ROOTDIR)/examples
	VPATH += $(ROOTDIR)/examples/$(PROJECTNAME)

endif	 
	 

