#
# Default build settings. Included in project makefile
#
# This makefile also helps to build the examples. It tries to figure out if 
# it is an in-tree-build or if it is a seperate project (it's then invoked 
# from the top makefile)  
#

ifndef PROJECTNAME
$(error PROJECTNAME is not set. . This makefile is invoked the wrong way))
endif

ifndef ROOTDIR
$(error ROOTDIR is not set. This makefile is invoked the wrong way)
endif

ifneq (${MAKELEVEL},0)

#####################################################################
# Default settings
#####################################################################

	VPATH += ..
	inc-y += $(ROOTDIR)/system/kernel/$(objdir)
	inc-y += $(ROOTDIR)/system/kernel/include
	
	# The more precise configuration, the higher preceedance.
	VPATH := ../config/$(board_name) ../config $(VPATH)
	inc-y := ../config/$(board_name) ../config $(inc-y)

	ldcmdfile-y = linkscript_$(COMPILER).lcf
	vpath %.ldf $(ROOTDIR)/$(ARCH_PATH-y)/scripts
	vpath %.ldf $(board_path)
	

	# What I want to build
	build-exe-y = $(PROJECTNAME).elf
	os-orti-y = $(PROJECTNAME).orti


else

#####################################################################
# In-tree-build (for examples)
#####################################################################

    ifneq ($(BOARDDIR),)
    $(warning BOARDDIR defined in an in-tree-build)
    endif

	BUILD_IN_TREE=y


	# Try to get name of board
	ifeq ($(boarddir),)
      boardpath=$(realpath $(CURDIR)/../..)
      boarddir=$(subst $(realpath $(ROOTDIR)/boards)/,,$(boardpath))
    endif

	# Redirect to normal build
.PHONY: all clean
all:
	@echo "==========[ Building \"$(example)\" ]=========="
	$(Q)$(MAKE) -C $(ROOTDIR) BOARDDIR=$(boarddir) BDIR=$(CURDIR) all
	
clean:	
	@echo Removing dir $(boarddir) 
	rm -rfv obj_$(boarddir)
	rm -rfv $(ROOTDIR)/binaries/$(boarddir)/$(PROJECTNAME)*
	@echo done!

endif

