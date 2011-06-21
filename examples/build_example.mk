#
# This makefile helps to build the examples. It tries to figure out if 
# it is an in-tree-build or if it is a seperate project (it's then invoked 
# from the top makefile)  
#

ifndef EXAMPLENAME
$(error EXAMPLENAME is not set. . This makefile is invoked the wrong way))
endif

ifndef ROOTDIR
$(error ROOTDIR is not set. This makefile is invoked the wrong way)
endif

ifeq (${MAKELEVEL},0)
BUILD_IN_TREE=y

    ifneq ($(BOARDDIR),)
    $(warning BOARDDIR defined in an in-tree-build)
    endif

    boardpath=$(realpath $(CURDIR)/../..)
    boarddir=$(subst $(realpath $(ROOTDIR)/boards)/,,$(boardpath))
endif

ifeq (${BUILD_IN_TREE},y) 

export example:=$(subst $(abspath $(CURDIR)/..)/,,$(CURDIR))

.PHONY: all clean
all:
	@echo "==========[ Building \"$(example)\" ]=========="
	$(Q)$(MAKE) -C $(ROOTDIR) BOARDDIR=$(boarddir) BDIR=$(CURDIR) all
	
clean:	
	@echo Cleaning dir $(boarddir) 
	$(Q)rm -rf obj_$(boarddir)
	@echo done!

else 
export example:=$(EXAMPLENAME)
endif

ifneq (${MAKELEVEL},0)

VPATH += ..
VPATH += ../config
VPATH += $(ROOTDIR)/examples
VPATH += $(ROOTDIR)/examples/$(example)
inc-y += ..
inc-y += ../config

ldcmdfile-y = linkscript_$(COMPILER).lcf
vpath %.ldf $(ROOTDIR)/$(ARCH_PATH-y)/scripts
	
# What I want to build
build-exe-y = $(EXAMPLENAME).elf

endif

