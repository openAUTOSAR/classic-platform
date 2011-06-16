

EXAMPLENAME=rte_simple
ROOTDIR?=../../../..
include $(ROOTDIR)/examples/build_example.mk
include $(ROOTDIR)/examples/rte_simple/rte_simple.mk

ifneq (${MAKELEVEL},0) 

# object files
PROJECT_C_FILES=$(notdir $(wildcard ../*.c))
obj-y += $(PROJECT_C_FILES:%.c=%.o)

endif





