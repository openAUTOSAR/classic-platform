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
	
	PROJECT_DIR:=$(abspath $(CURDIR)/..)

	ifneq ($(findstring RTE,$(MOD_USE)),)
	VPATH := $(PROJECT_DIR)/config/$(board_name)/Rte/Config $(PROJECT_DIR)/config/Rte/Config $(VPATH)
	VPATH := $(PROJECT_DIR)/config/$(board_name)/Rte/Contract $(PROJECT_DIR)/config/Rte/Contract $(VPATH)
	VPATH := $(PROJECT_DIR)/config/$(board_name)/Rte/MemMap $(PROJECT_DIR)/config/Rte/MemMap $(VPATH)
	inc-y := $(PROJECT_DIR)/config/$(board_name)/Rte/Config $(PROJECT_DIR)/config/Rte/Config $(inc-y)
	inc-y := $(PROJECT_DIR)/config/$(board_name)/Rte/Contract $(PROJECT_DIR)/config/Rte/Contract $(inc-y)
	inc-y := $(PROJECT_DIR)/config/$(board_name)/Rte/MemMap $(PROJECT_DIR)/config/Rte/MemMap $(inc-y)
	endif
	
	VPATH += ..
	
	# The more precise configuration, the higher preceedance.
	VPATH := ../config/$(board_name) ../config $(VPATH)
	inc-y := ../config/$(board_name) ../config $(inc-y)

    ldcmdfile-y = linkscript_$(COMPILER).lcf
    vpath %.ldf $(ROOTDIR)/$(ARCH_KERNEL_PATH-y)/scripts
    vpath %.ldf $(board_path)

	# What I want to build
	ifeq (${CFG_BUILD_AS_LIB},y)
	build-lib-y = $(PROJECTNAME).a		
	else
		ifneq ($(findstring HOST_TEST,$(def-y)),)
		build-exe-y = $(PROJECTNAME).out
		else
		build-exe-y = $(PROJECTNAME).elf
		build-srec-$(CFG_CREATE_SREC) = $(PROJECTNAME).s3
		build-srec-fill-$(CFG_CREATE_SREC) = $(PROJECTNAME)_fill.s3
		ifeq (${CFG_POSTBUILD},y)
			build-srec-pb-$(CFG_CREATE_SREC) = $(PROJECTNAME)_pb.s3
		endif
		os-orti-$(USE_KERNEL) = $(PROJECTNAME).orti
		endif
	
	endif
else

#####################################################################
# In-tree-build (for examples)
#####################################################################

#    ifneq ($(BOARDDIR),)
#    $(warning BOARDDIR defined in an in-tree-build)
#    endif

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

