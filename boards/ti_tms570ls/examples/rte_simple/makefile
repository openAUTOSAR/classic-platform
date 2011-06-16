

EXAMPLENAME=rte_simple
ROOTDIR?=../../../..
include $(ROOTDIR)/examples/build_example.mk

ifneq (${MAKELEVEL},0) 

# object files
PROJECT_C_FILES=$(notdir $(wildcard ../*.c))
obj-y += $(PROJECT_C_FILES:%.c=%.o)
obj-y += Calculator.o
obj-y += Tester.o
obj-y += rte_simple.o
obj-y += system_hooks.o

endif





