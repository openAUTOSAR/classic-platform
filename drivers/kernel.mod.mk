#kernel
# CPU specific
ifeq ($(CFG_PPC),y)
obj-$(USE_KERNEL) += mpc5xxx_handlers.o
obj-$(USE_KERNEL) += mpc5xxx_handlers_asm.o
ifeq ($(filter os_mpu_mpc5516.o os_mpu_mpc5643l.o os_mpu_spc56xl70.o os_mpu_mpc5744p.o os_mpu_mpc560x.o,$(obj-y)),)
obj-$(USE_KERNEL)-$(CFG_MPC5516) += os_mpu_mpc5516.o
obj-$(USE_KERNEL)-$(CFG_MPC5643L) += os_mpu_mpc5643l.o
obj-$(USE_KERNEL)-$(CFG_SPC56XL70) += os_mpu_spc56xl70.o
obj-$(USE_KERNEL)-$(CFG_MPC5744P) += os_mpu_mpc5744p.o
obj-$(USE_KERNEL)-$(CFG_MPC560X) += os_mpu_mpc560x.o
endif
ifeq ($(filter mpc5xxx_callout_stubs.o,$(obj-y)),)
obj-$(USE_KERNEL) += mpc5xxx_callout_stubs.o
endif
endif



ifeq ($(CFG_ARM_CM3)$(CFG_ARM_CM4),y)
obj-$(USE_KERNEL) += arm_cortex_mx_handlers.o
ifeq ($(filter arm_cortex_mx_callout_stubs.o,$(obj-y)),)
obj-$(USE_KERNEL) += arm_cortex_mx_callout_stubs.o
endif
endif

obj-$(USE_KERNEL)-$(CFG_TMS570) += os_mpu_tms570.o

ifeq ($(filter armv7ar_callout_stubs.o,$(obj-y)),)
obj-$(USE_KERNEL)-$(CFG_TMS570) += armv7ar_callout_stubs.o
obj-$(USE_KERNEL)-$(CFG_TRAVEO) += armv7ar_callout_stubs.o
endif

vpath-$(CFG_TRAVEO) += $(ROOTDIR)/$(ARCH_KERNEL_PATH-y)/integration

obj-$(USE_KERNEL)-$(CFG_AURIX) += tcxxx_trap_asm.o
obj-$(USE_KERNEL)-$(CFG_AURIX) += tcxxx_trap.o
obj-$(USE_KERNEL)-$(CFG_TC2XX) += os_mpu_tc297.o
obj-$(USE_KERNEL)-$(CFG_TC2XX) += tc2xx_trap_handlers.o
obj-$(USE_KERNEL)-$(CFG_TC2XX) += tc2xx_callout_stubs.o

