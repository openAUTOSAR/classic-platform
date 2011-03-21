

ifndef ROOTDIR
$(error ROOTDIR is not set. This makefile is invoked the wrong way)
endif

ifndef BOARDDIR
# Assume in-tree-build 
boardpath=$(realpath $(CURDIR)/../..)
boarddir=$(subst $(realpath $(ROOTDIR)/boards)/,,$(boardpath))
ugh=1
else
  # BOARDIR is defined
  ifndef BDIR
    # Assume that we want to build current directory
    BDIR=$(CURDIR)
    ugh=1
  else
    # BOARDIR and BDIR are defined 
    # out-of-tree build
  endif
endif

ifeq ($(ugh),1) 
export example:=$(subst $(abspath $(CURDIR)/..)/,,$(CURDIR))

.PHONY: all clean
all:
	@echo "==========[ Building \"$(example)\" ]=========="
	$(Q)$(MAKE) -C $(ROOTDIR) BOARDDIR=$(boarddir) BDIR=$(CURDIR) all
	
clean:	
	@echo Cleaning dir $(boarddir) 
	$(Q)rm -rf obj_$(boarddir)
	@echo done!

endif

ifneq (${MAKELEVEL},0)

VPATH += ..
VPATH += $(ROOTDIR)/examples
inc-y += ..

ldcmdfile-y = linkscript_$(COMPILER).lcf
vpath %.ldf $(ROOTDIR)/$(ARCH_PATH-y)/scripts
	
# What I want to build
build-exe-y = $(example).elf

endif

