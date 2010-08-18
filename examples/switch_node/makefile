
# Our object files
obj-y += switch_node_helpers.o
obj-y += switch.o
obj-y += Rte.o
obj-y += Tasks.o
obj-y += Hooks.o

VPATH += ..

inc-y += $(ROOTDIR)/system/kernel/$(objdir)
inc-y += $(ROOTDIR)/system/kernel/include

# OS object files.
obj-y += Os_Cfg.o
# Board object files
include $(ROOTDIR)/boards/board_common.mk

ABSDIR := $(subst $(TOPDIR)/,,$(CURDIR))

# The more precise configuration, the higher preceedance.
VPATH := ../config/$(BOARDDIR) ../config $(VPATH)
inc-y := ../config/$(BOARDDIR) ../config $(inc-y)

inc-y += $(ROOTDIR)/system/kernel/$(objdir)

#TODO: Remove?, Build other way?
VPATH += ../Rte
inc-y += ../Rte
VPATH += $(ROOTDIR)/components/switch
inc-y += $(ROOTDIR)/components/switch


# libs needed by us 
#libitem-y +=

#linkfile
ldcmdfile-y = linkscript_gcc.ldp
vpath %.ldf $(ROOTDIR)/$(ARCH_PATH-y)/scripts

# What I want to build
build-exe-y = switch_node.elf





