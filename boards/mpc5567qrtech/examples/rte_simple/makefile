

PROJECTNAME=rte_simple
ROOTDIR?=../../../..
include $(ROOTDIR)/scripts/project_defaults.mk
include $(ROOTDIR)/examples/rte_simple/rte_simple.mk

ifneq (${MAKELEVEL},0) 

	# object files
	PROJECT_C_FILES=$(notdir $(wildcard ../*.c))
	obj-y += $(PROJECT_C_FILES:%.c=%.o)
	
	VPATH += $(ROOTDIR)/examples
	VPATH += $(ROOTDIR)/examples/$(PROJECTNAME)

endif





