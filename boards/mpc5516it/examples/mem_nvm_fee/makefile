


#include $(PROJECT_DIR)/tests_common.mk
#
#$(if $(filter mpc5516it,$(BOARDDIR)),,$(error expect board mpc5516it, found $(BOARDDIR)))
#
## What I want to build
#build-exe-y = nvm_05_mcal_fls.elf

#ldcmdfile-y = $(ROOTDIR)/$(ARCH_PATH-y)/scripts/linkscript_gcc.ldf

PROJECTNAME=mem_nvm_fee_mpc5516it
ROOTDIR?=../../../..
include $(ROOTDIR)/scripts/project_defaults.mk

ifneq (${MAKELEVEL},0) 

    # Project files (adds all .c files in project root)
    PROJECT_C_FILES=$(notdir $(wildcard ../*.c))
    obj-y += $(PROJECT_C_FILES:%.c=%.o)


endif

