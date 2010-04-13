# build with:
# make subdir1 			- build the subdir1 
# make subdir1/subdir2	- build subdir2
# 
# clean:
#   	Removed all generated files 
#
# BOARDDIR=<board dir> 
#       Select what board to build for 

# BOARDDIR=<board dir> 
#       Select what board to build for
# 
# Q=[(@)/empty] 
#  		If Q=@ cmd's will not be echoed.
#  
# Build
#   >make BOARDDIR=mpc551xsim BDIR=system/kernel,examples/simple all 
# Clean 
#   >make BOARDDIR=mpc551xsim BDIR=system/kernel,examples/simple clean
#

export UNAME:=$(shell uname)

ifneq ($(findstring Darwin,$(UNAME)),)
  SHELL:=/bin/bash
  export SED=/opt/local/bin/gsed
else
	export SED=sed
endif


Q?=@
export Q
export TOPDIR = $(CURDIR)
export PATH

# Select default console
# RAMLOG | TTY_T32 | TTY_WINIDEA 
export SELECT_OS_CONSOLE
export SELECT_CONSOLE
export USE_DEBUG_PRINTF
export SELECT_OPT?=OPT_RELEASE
export CFG_$(SELECT_OPT)=y

ifneq ($(filter clean_all,$(MAKECMDGOALS)),clean_all)
  ifeq (${BOARDDIR},)
    $(error BOARDDIR is empty) 
  endif
endif

USE_T32_SIM?=n
export USE_T32_SIM

# override BDIR := system/kernel ${BDIR} 

# Tools
# Ugly thing to make things work under cmd.exe 
PATH := /usr/bin/:$(PATH) 
find := $(shell which find)

export objdir = obj_$(BOARDDIR)

.PHONY: clean
.PHONY: release

.PHONY: help
help:
	@echo "Make kernel and a simple example"
	@echo "  > make BOARDDIR=mpc551xsim CROSS_COMPILE=/opt/powerpc-eabi/bin/powerpc-eabi- BDIR=examples/simple all"
	@echo ""
	@echo "Save the config (CROSS_COMPILE and BDIR)"
	@echo "  > make BOARDDIR=mpc551xsim CROSS_COMPILE=/opt/powerpc-eabi/bin/powerpc-eabi- BDIR=examples/simple save"
	@echo ""
	@echo "Clean"
	@echo "  > make clean"
	@echo ""
	@echo "Present config:"
	@echo "  ARCH=$(ARCH)"
	@echo "  ARCH_FAM=$(ARCH_FAM)"
	@echo "  BOARDDIR =$(BOARDDIR)"
	@echo "  CROSS_COMPILE =$(CROSS_COMPILE)"
	@echo ""
	
export CFG_MCU 
export CFG_CPU
export MCU
export def-y+=$(CFG_ARCH_$(ARCH)) $(CFG_MCU) $(CFG_CPU)

# We descend into the object directories and build the. That way it's easier to build
# multi-arch support and we don't have to use objdir everywhere.
# ROOTDIR - The top-most directory
# SUBDIR - The current subdirectory it's building.

comma:= ,
split = $(subst $(comma), ,$(1))
dir_cmd_goals  := $(call split,$(BDIR))
cmd_cmd_goals := $(filter clean all install,$(MAKECMDGOALS))

libs:
	mkdir -p $@

all: libs $(dir_cmd_goals)

show_build:
	@echo Building for $(dir_cmd_goals)
	@echo BOARDDIR: $(BOARDDIR)
	@echo ARCH_FAM/ARCH: $(ARCH_FAM)/$(ARCH)
	

$(dir_cmd_goals) :: show_build FORCE 	
	@echo ==========[ $@  ]===========
	+@[ -d $@/$(objdir) ] || mkdir -p $@/$(objdir)
	@chmod 777 $@/$(objdir)
	$(Q)$(MAKE) -r  -C $@/$(objdir) -f $(CURDIR)/scripts/rules.mk  ROOTDIR=$(CURDIR) SUBDIR=$@ $(cmd_cmd_goals)
.PHONY: test	

FORCE:

clean_all:
	@find . -type d -name obj_* | xargs rm -rf
	@find . -type f -name *.a | xargs rm -rf
	echo Done!
	
clean: $(dir_cmd_goals)
	@echo "Clean:"
	@echo "  Removing objectfiles and libs for ARCH=$(ARCH)"
	@find . -type d -name $(objdir) | xargs rm -rf
	@find . -type f -name *.a| xargs rm -rf
	@rm   -rf libs/*
	@echo Done!

		
	



