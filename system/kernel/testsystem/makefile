
# included from rules.mk

# =======================================================================
# APPLICATION

obj-y += test_master.o
obj-y += test_01_task.o
obj-y += test_02_resource.o
obj-y += test_03_alarm.o
obj-y += test_04_stable.o
obj-y += test_05_const.o
obj-y += test_06_event.o

#obj-y += test_sup_01.o
#obj-y += test_sup_02.o
#obj-y += test_sup_03.o
#obj-y += test_sup_04.o	
obj-y += test_framework.o

obj-y += Os_Cfg.o

# Not supported yet...
#obj-$(CFG_ARM_CM3) += test_sup_irq.o

# Grab the board the board files.
include $(ROOTDIR)/boards/board_common.mk

# What modules do we use
MOD_USE+=KERNEL MCU

MOD_USE+=T32_TERM
#MOD_USE+=PROTECTIONHOOK STARTUPHOOK SHUTDOWNHOOK ERRORHOOK			1
#MOD_USE+=PRETASKHOOK POSTTASKHOOK
CFG+=CONSOLE_T32
CFG+=CONSOLE_WINIDEA


# TODO: Fix this....

vpath-y += $(ROOTDIR)/$(ARCH_PATH-y)/kernel
vpath-y += $(ROOTDIR)/$(ARCH_PATH-y)/drivers
vpath-y += $(ROOTDIR)/boards/$(BOARDDIR)
vpath-y += $(ROOTDIR)/boards/$(BOARDDIR)/config
VPATH += $(vpath-y)
VPATH += $(ROOTDIR)/$(SUBDIR)/config


# libs needed by us 
#libitem-y +=

#linkfile
ldcmdfile-y = $(ROOTDIR)/$(ARCH_PATH-y)/scripts/linkscript_gcc.ldf
#ldcmdfile-y = $(ROOTDIR)/$(ARCH_PATH-y)/scripts/linkscript_gcc.ldf

inc-y += ..
inc-y += $(ROOTDIR)/system/kernel/$(objdir)
inc-y += $(ROOTDIR)/system/kernel/include
inc-y += $(ROOTDIR)/$(ARCH_PATH-y)
inc-y += $(ROOTDIR)/drivers/include
inc-y += $(ROOTDIR)/boards/$(BOARDDIR)/config
inc-y += $(ROOTDIR)/$(ARCH_PATH-y)/drivers
inc-y += ../config

# What I want to build
build-exe-y = $(target).elf









