# build with:
#   $ make BOARDDIR=<board> BDIR=<dir>[,<dir>] CROSS_COMPILE=<gcc> all|clean|clean_all
#
# TARGETS
#   all:   		Target when building
#   clean:  	Remove generatated files for a board 
#   clean_all:  Remove all generated files
#   help:       Print some help
#
# VARIABLES:
#   BOARDDIR=<board dir> 
#       Select what board to build for 
#   BDIR=<dir>[,<dir>] 
#       Select what directories to build. The kernel is always built.
#   CROSS_COMPILE
#       Specify the compiler to use.  
#   Q=[(@)/empty] 
#  		If Q=@ cmd's will not be echoed.
#
# EXAMPLES
#   Clean all
#     $ make clean_all
#
#   Clean for a specific board
#     $ make BDIR=mpc551xsim clean
#
#   Build the simple example (assuming CROSS_COMPILE set)
#     $ make BOARDDIR= mpc551xsim BDIR=examples/simple all
#

# make BOARDDIR=bar
# make BOARDDIR=foo/bar
# -> board_name=BOARDDIR
# -> board_path=foo


# Some useful variables..
comma:= ,
empty:=
space:= $(empty) $(empty)
split = $(subst $(comma), ,$(1))

# C:/apa -> /c/apa
# ../tjo -> ../tjo
to_msyspath = $(shell echo "$(1)" | sed -e 's,\\,\/,g;s,\([a-zA-Z]\):,/\1,')

# Convert Path if on windows.
ifeq ($(OS),Windows_NT)
  	BDIR:=$(call to_msyspath,$(BDIR))
endif

USE_T32_SIM?=n
export USE_T32_SIM

# Tools
# Ugly thing to make things work under cmd.exe 
PATH := /usr/bin/:$(PATH) 
FIND := $(shell which find)

export UNAME:=$(shell uname)

ifneq ($(findstring Darwin,$(UNAME)),)
  SHELL:=/bin/bash
  export SED=/opt/local/bin/gsed
else
	export SED=sed
endif

ifeq ($(VERBOSE),y)
export Q?=
else
export Q?=@
endif

export TOPDIR = $(CURDIR)
export PATH

#KVM-build variables
ifeq ($(KVM), true)
KVMDIR = ../../../j2me_cldc/build/autosar
endif

BOARDDIR = Raspberry_Pi

# Select default console
# RAMLOG | TTY_T32 | TTY_WINIDEA
export SELECT_OS_CONSOLE
export SELECT_CONSOLE
export SELECT_OPT

ifneq ($(filter clean_all,$(MAKECMDGOALS)),clean_all)
  ifeq ($(BOARDDIR),)
    $(error BOARDDIR is empty)
  endif
  ifeq ($(BDIR),)
    $(error BDIR is empty)
  endif
endif

dir_cmd_goals  := $(call split,$(BDIR))
cmd_cmd_goals := $(filter all clean config,$(MAKECMDGOALS))

# Check for CROSS_COMPILE
ifneq ($(cmd_cmd_goals),)

ifeq ($(findstring /,$(BOARDDIR)),)

# Check that the board actually exist
ifdef BOARDDIR
  all_boards := $(subst /,,$(subst boards/,,$(shell $(FIND) boards/ -maxdepth 1 -type d)))
  all_boards_print := $(subst $(space),$(comma)$(space),$(strip $(all_boards)))
  ifeq ($(filter $(BOARDDIR),$(all_boards)),)
  	$(error no such board: $(BOARDDIR), valid boards are: $(all_boards_print))
  endif
endif

  export board_name:=$(BOARDDIR)
  export board_path=$(CURDIR)/boards/$(board_name)

else
  # it's a path, split into board_name and board_path (contains board_name)
  tmp  :=$(subst /,$(space),$(BOARDDIR))
  board_name:=$(strip $(word $(words $(tmp)),$(tmp)))
  board_path:=$(BOARDDIR)
  export board_name
  export board_path
endif

# Check BDIR
endif


export objdir = obj_$(board_name)
export CFG_MCU 
export CFG_CPU
export MCU
export def-y+=$(CFG_ARCH_$(ARCH)) $(CFG_MCU) $(CFG_CPU)


# We descend into the object directories and build the. That way it's easier to build
# multi-arch support and we don't have to use objdir everywhere.
# ROOTDIR - The top-most directory
# SUBDIR - The current subdirectory it's building.


libs:
	mkdir -p $@

.PHONY all:

all: libs kvm $(dir_cmd_goals)

.PHONY: clean
.PHONY: release

.PHONY: help
help:
	@echo "Build a simple example"
	@echo "  > make BOARDDIR=mpc551xsim CROSS_COMPILE=/opt/powerpc-eabi/bin/powerpc-eabi- BDIR=examples/simple all"
	@echo ""
	@echo "Clean"
	@echo "  > make clean"
	@echo ""
	@echo "Present config:"
	@echo "  BDIR          = ${BDIR}"
	@echo "  BOARDDIR      = $(BOARDDIR)"
	@echo "  CROSS_COMPILE = $(CROSS_COMPILE)"
	@echo "  CURDIR        = $(CURDIR)"	
	@echo ""


test:
	@echo $(all_boards)

show_build:
	@echo ""
	@echo "==========[ BUILD INFO ]==========="
	@echo "  BDIR:           $(BDIR) [$(origin BDIR)]"
	@echo "  BOARDDIR:       $(BOARDDIR) [$(origin BOARDDIR)]"
	@echo "  COMPILER:       $(COMPILER) [$(origin COMPILER)]"	
ifeq ($(COMPILER),cw)	
	@echo "  CW_COMPILE:     $(CW_COMPILE) [$(origin CW_COMPILE)]"
else ifeq ($(COMPILER),iar)	
	@echo "  IAR_COMPILE:     $(IAR_COMPILE) [$(origin IAR_COMPILE)]"
else ifeq ($(COMPILER),ghs)	
	@echo "  GHS_COMPILE:     $(GHS_COMPILE) [$(origin GHS_COMPILE)]"	
else 
ifndef ($(CLANG_COMPILE),)
	@echo "  CLANG_COMPILE:  $(CLANG_COMPILE) [$(origin CLANG_COMPILE)]"
	@echo "  CFG_CLANG_SAFECODE:  $(CFG_CLANG_SAFECODE) [$(origin CFG_CLANG_SAFECODE)]"
endif
	@echo "  CROSS_COMPILE:  $(CROSS_COMPILE) [$(origin CROSS_COMPILE)]"
endif
	@echo "  SELECT_OPT:     $(SELECT_OPT) [$(origin SELECT_OPT)]"
	@echo "  CURDIR:         $(CURDIR)"
	@echo "  SELECT_CONSOLE: $(SELECT_CONSOLE) [$(origin SELECT_CONSOLE)]"
	
	
	
	
$(dir_cmd_goals) :: show_build FORCE
	@echo ""
	@echo ==========[ ${abspath $@}  ]===========
	@if [ ! -d $@ ]; then echo "No such directory: \"$@\" quitting"; exit 1; fi
	+@[ -d $@/$(objdir) ] || mkdir -p $@/$(objdir)
	@chmod 777 $@/$(objdir)
	$(Q)$(MAKE) -r  -C $@/$(objdir) -f $(CURDIR)/scripts/rules.mk  ROOTDIR=$(CURDIR) SUBDIR=$@ $(cmd_cmd_goals)
.PHONY: test	

FORCE:

.PHONY: boards
boards:
	@find . -type d -name *

clean_all:
	$(Q)find . -type d -name obj_* | xargs rm -rf
	$(Q)find . -type f -name *.a | xargs rm -rf
	@echo
	@echo "  >>>>>>>>>  DONE  <<<<<<<<<"
	@echo
	
config: $(dir_cmd_goals)	
	
.PHONY clean:	
clean: clean_kvm $(dir_cmd_goals)
	@echo
	@echo "  >> Cleaning MAIN $(CURDIR)"
#	$(Q)find . -type d -name $(objdir) | xargs rm -rf
#	$(Q)find . -type f -name *.a | xargs rm -rf
#	$(Q)rm   -rf libs/*
	@echo
	@echo "  >>>>>>>>>  DONE  <<<<<<<<<"
	@echo

kvm: 
ifeq ($(KVM), true)
	@echo "Starting recursive build of KVM..............................................."
	cd $(KVMDIR); $(MAKE) USE_KNI=true ARM_EABI=true || exit 1; cd $(BUILDDIR);
	@echo "KVM was successfully built..............................................."
endif	

clean_kvm: 
ifeq ($(KVM), true)
	@echo
	@echo ">>>>>>>>> Cleaning KVM  <<<<<<<<<"
	@echo
	cd $(KVMDIR); $(MAKE) clean || exit 1; cd $(BUILDDIR);
	@echo
	@echo ">>>>>>>>>  KVM clean done  <<<<<<<<<"
	@echo
endif
	



