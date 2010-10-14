
# included from rules.mk

# =======================================================================
# APPLICATION

obj-y += test_02_ip.o
obj-y += test_hooks.o
obj-y += test_framework.o

obj-y += Os_Cfg.o

# Not supported yet...
#obj-$(CFG_ARM_CM3) += test_sup_irq.o

# Grab the board the board files.
include $(ROOTDIR)/boards/board_common.mk

# What modules do we use
MOD_USE+=KERNEL MCU

# TODO: Fix this....

vpath-y += $(ROOTDIR)/$(ARCH_PATH-y)/kernel
vpath-y += $(ROOTDIR)/$(ARCH_PATH-y)/drivers
vpath-y += $(ROOTDIR)/boards/$(BOARDDIR)
vpath-y += $(ROOTDIR)/boards/$(BOARDDIR)/config
VPATH += $(vpath-y)
VPATH += $(ROOTDIR)/$(SUBDIR)/config
VPATH += $(realpath ../..)
VPATH += ..

# libs needed by us 
#libitem-y += $(ROOTDIR)/libs/libkernel_$(ARCH_MCU).a

#linkfile
ldcmdfile-y = linkscript_gcc.ldp
vpath %.ldf $(ROOTDIR)/$(ARCH_PATH-y)/scripts

inc-y += ..
inc-y += ../..
inc-y += $(ROOTDIR)/system/kernel/$(objdir)
inc-y += $(ROOTDIR)/system/kernel/include
inc-y += $(ROOTDIR)/$(ARCH_PATH-y)
inc-y += $(ROOTDIR)/drivers/include
inc-y += $(ROOTDIR)/boards/$(BOARDDIR)/config
inc-y += $(ROOTDIR)/$(ARCH_PATH-y)/drivers
inc-y += ../config

# What I want to build
build-exe-y = $(target).elf









