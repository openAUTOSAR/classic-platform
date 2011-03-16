


export CROSS_COMPILE=/opt/powerpc-eabispe/bin/powerpc-eabispe-

root_path=$(abspath $(CURDIR)/../../../..)
board_path=$(abspath $(CURDIR)/../..)

ifeq (${MAKELEVEL},0)
boarddir=$(subst $(root_path)/boards/,,$(board_path))

.PHONY: all clean
all:
	$(Q)$(MAKE) -C $(root_path) BOARDDIR=$(boarddir) BDIR=$(CURDIR) all
	
clean:	
	@echo Cleaning dir $(boarddir) 
	$(Q)rm -rf obj_$(boarddir)
	@echo done!

export example:=$(subst $(abspath $(CURDIR)/..)/,,$(CURDIR))
#$(error $(example))
	
else
VPATH += ..
VPATH += $(ROOTDIR)/examples
inc-y += ..

# Board object files
obj-y += system_hooks.o
obj-y += kernel_extra.o

include $(ROOTDIR)/boards/board_common.mk
	
ldcmdfile-y = linkscript_$(COMPILER).lcf
vpath %.ldf $(ROOTDIR)/$(ARCH_PATH-y)/scripts
	
# What I want to build
build-exe-y = $(example).elf


endif

	 

