# LinuxOs

obj-$(USE_LINUXOS) += Os_Cfg.o sys_tick.o os_linos.o linos_logger.o linos_time.o os_main.o os_stubs.o
obj-$(USE_LINUXOS) += pmc_sync_status_common.o pmc_sync_status_reader.o
obj-$(USE_LINUXOS) += os_task_linos.o arch.o LinuxOs_StbM.o
obj-$(USE_LINUXOS) += os_counter_linos.o os_alarm_linos.o os_init_linos.o os_event_linos.o
inc-$(USE_LINUXOS) += $(ROOTDIR)/$(OSAL_ARCH)/gnulinux/gnulinux/kernel
vpath-$(USE_LINUXOS) += $(ROOTDIR)/$(OSAL_ARCH)/gnulinux/gnulinux/kernel
vpath-$(USE_LINUXOS) += $(ROOTDIR)/mcal/Mcu/src
inc-$(USE_LINUXOS) += $(ROOTDIR)/system/LinuxOs/inc
inc-$(USE_LINUXOS) += $(ROOTDIR)/system/LinuxOs/src
inc-$(USE_LINUXOS) += $(ROOTDIR)/system/LinuxOs/pmc_sync_status_reader
inc-$(USE_LINUXOS) += $(ROOTDIR)/system/StbM
inc-$(USE_LINUXOS) += $(ROOTDIR)/system/Os/rtos/inc
inc-$(USE_LINUXOS) += $(ROOTDIR)/system/Os/rtos/integration
inc-$(USE_LINUXOS) += $(ROOTDIR)/system/Os/rtos/src
vpath-$(USE_LINUXOS) += $(ROOTDIR)/system/LinuxOs/src
vpath-$(USE_LINUXOS) += $(ROOTDIR)/system/LinuxOs/pmc_sync_status_reader
CFLAGS += -D__PROGNAME__=\"$(PROJECTNAME)\"


# Antons ADPM project doesn't use the MCU module, but needs Mcu.o
ifndef USE_MCU
obj-$(USE_LINUXOS) += Mcu_gnulinux.o
endif
#

# RedHat 6 and Centos 6 and Ubuntu 12 
# needs the RealTime library to be defined. (GCC 4.8)
ifneq ($(OS),Windows_NT)
    ifdef USE_LINUXOS
    LDFLAGS += -lrt
    LDFLAGS += -lm
    endif
endif

# Ubuntu 12 needs this too.
#LDFLAGS += -Wl,--no-as-needed

ifneq ($(OS),Windows_NT)
    # Database module
    obj-$(USE_LINUXOS_DB) += database.o
    #Sqlite3 devel library needed for this.
    ifdef USE_LINUXOS_DB
    LDFLAGS += -lsqlite3
    endif
    #
endif
