
# OS object files.
obj-y += Os_Cfg.o


VPATH += ..
VPATH += ../config

inc-y += ../config
inc-y += $(ROOTDIR)/system/kernel/$(objdir)
inc-y += $(ROOTDIR)/system/kernel/include

# The more precise configuration, the higher preceedance.
VPATH := ../config/$(BOARDDIR) ../config $(VPATH)
inc-y := ../config/$(BOARDDIR) ../config $(inc-y)

# Board object files
include $(ROOTDIR)/boards/board_common.mk

# Our object files
obj-y += tiny.o

# libs needed by us 
#libitem-y +=

#linkfile
ldcmdfile-y = linkscript_gcc.ldp
vpath %.ldf $(ROOTDIR)/$(ARCH_PATH-y)/scripts

# What I want to build
#build-lib-y = tiny.a
#libitem-y += tiny.a
build-exe-y = tiny.elf
#build-hex-y = tiny.hex








