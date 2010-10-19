

#VPATH += $(ROOTDIR)/arch/arm/arm_cm3/kernel
vpath-y += $(ARCH_PATH-y)/kernel
obj-y += asm_sample.o
#CFLAGS_asm_sample.o += -O3
obj-y += arch_krn.o
obj-$(CFG_HCS12D) += arch_irq.o
#obj-y += stm32f10x_it.o

# object files 
obj-y += event.o
obj-y += init.o
#obj-y += trusted.o
obj-y += arch.o
obj-y += task.o
#obj-y += task_i.o
obj-y += resource.o
#obj-y += swap.o
obj-y += alarm.o
obj-y += sched_table.o
obj-y += counter.o
#obj-y += com_internal.o
#obj-y += create.o
obj-y += sys_tick.o
#obj-y += stack.o
obj-y += isr.o
obj-y += os_arctest.o
obj-y += application.o


# These are VERY cpu dependent.. remove
#obj-y += int_ctrl.o
obj-y += irq.o

# We are compiling the kernel
dep-y += asm_offset.h
dep-y += kernel_offset.h
dep-$(CFG_ARM_CM3) += arch_offset.h


# ARM assembler generates "define STACK_APA $12". The extra '$' we want to go.
# Assembler offsets
%_offset.h: %_offset.c
	@echo "  >> generating $@ from $<"
	$(Q)$(CC) -S $(CFLAGS) -o $(@:.h=.s) $(addprefix -I ,$(inc-y)) $(addprefix -D,$(def-y)) $<
	$(Q)gawk '/.*define/ { print $$1 " " $$2 " " gensub("#","","g",$$3) }' $(@:.h=.s)  > $@

# Assembler offsets
asm_offset.h: asm_offset.c
	@echo "  >> asm offset gen $<"
	$(Q)$(CC) -S $(CFLAGS) -o $(<:.c=.s) $(addprefix -I ,$(inc-y)) $(addprefix -D,$(def-y)) $<	
	@$(SED) -n "/#define/p" $(<:.c=.s) > $@
	@rm 	$(<:.c=.s)
	
#VPATH += ..
VPATH += $(ROOTDIR)/system/kernel
VPATH += $(ROOTDIR)/arch/$(ARCH_FAM)/$(ARCH)/kernel
VPATH += $(ROOTDIR)/arch/$(ARCH_FAM)/$(ARCH)/drivers

inc-y += .
inc-y += $(ROOTDIR)/drivers/include
inc-y += $(ROOTDIR)/arch/$(ARCH_FAM)/$(ARCH)/kernel
inc-y += $(ROOTDIR)/arch/$(ARCH_FAM)/$(ARCH)/drivers
inc-y += $(ROOTDIR)/arch/$(ARCH_FAM)/$(ARCH)/config
inc-y += $(ROOTDIR)/arch/$(ARCH)
inc-y += $(ROOTDIR)/include/$(ARCH_FAM)
 


