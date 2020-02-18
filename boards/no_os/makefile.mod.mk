
# Note! This file should not be used directly, but included from
#       subdirectories

inc-y += $(ROOTDIR)/boards/no_os/config
inc-y += $(ROOTDIR)/boards/no_os/
vpath-y += $(ROOTDIR)/boards/no_os/src

obj-y += misc_stubs.o

ifeq ($(boarddir),linux)

else
# mpc5xxx_callout_stubs.o
obj-$(CFG_PPC) += mpc5xxx_callout_stubs.o
vpath-$(CFG_PPC) += $(ROOTDIR)/mcal/arch/mpc5xxx/src/integration

obj-$(CFG_TC29X) += tcxxx_trap_asm.o

endif
