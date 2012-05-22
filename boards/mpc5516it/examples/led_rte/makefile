

PROJECTNAME=led_rte_mpc5516it
ROOTDIR?=../../../..
include $(ROOTDIR)/scripts/project_defaults.mk

ifneq (${MAKELEVEL},0) 

    # Project files (adds all .c files in project root)
    PROJECT_C_FILES=$(notdir $(wildcard ../*.c))
    obj-y += $(PROJECT_C_FILES:%.c=%.o)


endif
