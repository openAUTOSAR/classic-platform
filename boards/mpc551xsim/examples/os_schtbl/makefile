

PROJECTNAME=schedule_table
ROOTDIR?=../../../..
include $(ROOTDIR)/scripts/project_defaults.mk

ifneq (${MAKELEVEL},0) 

	# object files
	obj-y += os_schtbl.o
	obj-y += system_hooks.o
	
	VPATH += $(ROOTDIR)/examples
	VPATH += $(ROOTDIR)/examples/$(PROJECTNAME)

endif





