
# Usage:
#   SHELL
#    ...
#    $ make -f build_single.mk BDIR=/c/projects/workspace/Ticket1129-nvm-arctest/NvMTests all
#
#   ECLIPSE:
#    C/C++ build->Build directory :  ${project_loc}
#    ARC_DIR=<your arc directory>
#    BOARDDIR=linux
#    Make target: -f build_single.mk BDIR=/c/projects/workspace/Ticket1129-nvm-arctest/NvmTests all

ARC_DIR?=core

ifeq (${MAKELEVEL},0)
  # Build from within eclipse     
  ifeq ($(OS),Windows_NT)  
    PROJECT_DIR?=/$(subst :/,/,$(subst \,/,$(PWD)))
  else
    PROJECT_DIR?=$(PWD)
  endif
  ARC_PATH?=$(PROJECT_DIR)/../$(ARC_DIR)
  BDIR=$(CURDIR)
  export BDIR  
else
  # "Normal build
  ARC_PATH?=../$(ARC_DIR)
  PROJECT_DIR?=$(abspath $(CURDIR))
endif

export PROJECT_DIR
export Q

$(info ARC_DIR:     $(ARC_DIR))
$(info ARC_PATH:    $(ARC_PATH)) 
#$(info TEST_DIR:    $(TEST_DIR))
$(info PROJECT_DIR: $(PROJECT_DIR)) 
$(info CURDIR:      $(CURDIR))
$(info PWD:         $(PWD))

ifeq ($(BOARDDIR), linux)
export SELECT_CLIB=CLIB_NATIVE
else
endif

.PHONY: clean

clean all:
	$(Q)$(MAKE) -r  -C $(ARC_PATH) $(MAKECMDGOALS)
	
	
	