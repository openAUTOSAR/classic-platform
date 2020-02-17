
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

export CROSS_COMPILE:=
export TEST_DIR:=$(abspath $(CURDIR))
export BOARDDIR:=linux
export BDIR=$(TEST_DIR)
export SELECT_CLIB=CLIB_NATIVE
export PROJECT_DIR

.PHONY: clean

clean all:
	$(Q)$(MAKE) -r  -C $(ARC_PATH) $(MAKECMDGOALS)

