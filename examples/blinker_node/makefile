
# Our object files
obj-y += blinker_main.o
obj-y += Tasks.o
obj-y += Hooks.o
obj-y += Rte.o

VPATH += ../

ABSDIR := $(subst $(TOPDIR)/,,$(CURDIR))

# The more precise configuration, the higher preceedance.
VPATH := ../config/$(BOARDDIR) ../config $(VPATH)
inc-y := ../config/$(BOARDDIR) ../config $(inc-y)

inc-y += $(ROOTDIR)/system/kernel/$(objdir)
inc-y += $(ROOTDIR)/system/kernel/include

#TODO: Remove?, Build other way?
VPATH += ../Rte
inc-y += ../Rte
VPATH += $(ROOTDIR)/components/blinker
inc-y += $(ROOTDIR)/components/blinker

# libs needed by us 
#libitem-y +=

#linkfile
ldcmdfile-y = linkscript_gcc.ldp
vpath %.ldf $(ROOTDIR)/$(ARCH_PATH-y)/scripts

# What I want to build
build-exe-y = blinker_node.elf





