

OS_INC=Os/rtos/inc
OS_SRC=Os/rtos/src
OS_INTEGRATION=Os/rtos/integration

#vpath-y += $(ARCH_KERNEL_PATH-y)/kernel
#obj-y += asm_sample.o
#CFLAGS_asm_sample.o += -O3
obj-y += arch_krn.o
obj-$(CFG_HC1X) += arch_irq.o

# object files 
obj-y += os_event.o
obj-y += os_init.o
obj-y += arch.o
obj-y += os_task.o
obj-y += os_resource.o
obj-y += os_alarm.o
obj-y += os_sched_table.o
obj-y += os_counter.o
obj-y += $(if $(CFG_TRAVEO_S6J328C),sys_tick_traveo.o,sys_tick.o)
obj-y += os_isr.o
obj-y += os_application.o
obj-y += os_panic.o
obj-y += os_trap.o
obj-y += os_peripheral.o

obj-$(CFG_MULTICORE) += os_spinlock.o
obj-$(CFG_MULTICORE) += os_multicore.o
obj-$(CFG_KERNEL_EXTRA) += os_sleep.o
obj-$(CFG_KERNEL_EXTRA) += os_sem.o
obj-$(CFG_KERNEL_EXTRA) += os_mbox.o



# These are VERY cpu dependent.. remove
#obj-y += int_ctrl.o

obj-y += $(if $(CFG_TRAVEO_S6J328C),irq_traveo.o,irq.o)

# We are compiling the kernel
dep-$(CFG_PPC) += arch_offset.h
dep-y += asm_offset.h
#dep-y += kernel_offset.h
dep-$(CFG_ARM_CM4) += arch_offset.h
dep-$(CFG_ARM_CM3) += arch_offset.h
dep-$(CFG_ARM_CR4) += arch_offset.h
dep-$(CFG_ARM_CR5) += arch_offset.h 
dep-$(CFG_TC2XX) += arch_offset.h

ifeq ($(COMPILER),gcc)
# ARM assembler generates "define STACK_APA $12". The extra '$' we want to go.
# Assembler offsets
%_offset.h: %_offset.c
	@echo "  >> generating $@ from $<"
	$(Q)$(CC_PRE)$(CC) -S $(CFLAGS) -o $(@:.h=.s) $(addprefix -I,$(inc-y)) $(addprefix -D,$(def-y)) $<
	$(Q)gawk '/.*define/ { print $$1 " " $$2 " " gensub("#","","g",$$3) }' $(@:.h=.s)  > $@

# Assembler offsets
asm_offset.h: asm_offset.c
	@echo "  >> asm offset gen $<"
	$(Q)$(CC_PRE)$(CC) -S $(CFLAGS) -o $(@:.h=.s) $(addprefix -I,$(inc-y)) $(addprefix -D,$(def-y))  $<
	@$(SED) -e "/#define/!d" -e "s/#//2g" $(@:.h=.s) > $@
	@rm 	$(@:.h=.s)
endif

ifeq ($(COMPILER),ghs)
# ARM assembler generates "define STACK_APA $12". The extra '$' we want to go.
# Assembler offsets
%_offset.h: %_offset.c
	@echo "  >> generating $@ from $<"
	$(Q)$(CC) -S $(filter-out -c -dwarf2,$(CFLAGS)) -o $(@:.h=.s) $(addprefix -I,$(inc-y)) $(addprefix -D,$(def-y)) $<
	$(Q)gawk '/.*define/ { print $$1 " " $$2 " " gensub("#","","g",$$3) }' $(@:.h=.s)  > $@

# Assembler offsets
asm_offset.h: asm_offset.c
	@echo "  >> asm offset gen $<"
	$(Q)$(CC) -S $(filter-out -c -dwarf2,$(CFLAGS)) -o $(@:.h=.s) $(addprefix -I,$(inc-y)) $(addprefix -D,$(def-y))  $<
	@$(SED) -e "/#define/!d" -e "s/#//2g" $(@:.h=.s) > $@
	@rm 	$(@:.h=.s)
endif



# sed -n '/apa.*/ {h;n;s/q://;x;s/^/#define /;s/$//;x;p}' bepa.c
# gawk '/apa/ { var=$0;getline tjo;print "#define " var tjo}' bepa.c
# $ gawk '/^arc_dummy/ { va=gensub(/arc_dummy_(.*):.*/,"\\1",$0);getline;printf "
#define %-30s %s\n",va,$2 }' asm_offset.h


ifeq ($(COMPILER),iar)
%_offset.h: %_offset.c
	@echo "  >> generating $@ from $<"
	$(CC) -lB $@_ $(CFLAGS) $(addprefix -I,$(inc-y)) $(addprefix -D,$(def-y)) $< -o $@__ 
	$(Q)gawk '/^arc_dummy/ { va=gensub(/arc_dummy_(.*):.*/,"\\1",$$0);getline;printf "#define %-30s %s\n",va,$$2 }' $@_ > $@
	$(Q)rm $@_ $@__ #$(subst .h,.r12,$@) 
	
#	$ sed -n "/arc_dummy/ {n;s/q://p}" bepa.c
#	exit 0  
endif

#--preprocess=c

ifeq ($(COMPILER),diab)
# assembler generates this:
# arc_dummy_EXC_FRM_SRR1:
#	.long		16 
# 
# we want the size (16) and the name after arc_dummy
# assembler offsets
%_offset.h: %_offset.c
	@echo "  >> generating $@ from $<"
	$(Q)$(CC) -S -Xpreprocess-assembly $(CFLAGS) -o $(@:.h=.s) $(addprefix -I,$(inc-y)) $(addprefix -D,$(def-y)) $<
	$(Q)gawk 'BEGIN { RS = "arc_dummy_"; FS = ":"} match($$2, /long\t+([0-9]+)/, ary) \
			  {gsub("\\t", " ",$$1); print ("#define " $$1 " " ary[1]) }' $(@:.h=.s)  > $@
endif

ifeq ($(COMPILER),cw)
# The "\r" in the awk is a little strange...sed and awk if that does not work 
#  sed -e '/.apa/!d' -e '/OBJ/!d' -e 's/\[//' 123.tmp | gawk '{ print "#define " $9 " " $4 }'

%_offset.h: %_offset.c
	@echo "  >> generating $@ from $<"
#	$(Q)$(CC) -dis $(CFLAGS) $(addprefix -I,$(inc-y)) $(addprefix -D,$(def-y)) $<
	$(Q)$(CC) -dis $(CFLAGS) $(addprefix -I,$(inc-y)) $(addprefix -D,$(def-y)) $< > 123.tmp
	$(Q)gawk '$$9 ~/.apa/ && $$0 ~/OBJ/ { gsub(/\r/,""); printf("#define %s %d\n",$$10,$$5-100) }' 123.tmp > $@
endif


ifeq ($(COMPILER),armcc)

# Assembler offsets
asm_offset.h: asm_offset.c
	@echo "  >> asm offset gen $<"
	$(Q)$(CC) -S --preprocess_assembly $(CFLAGS) -o $(@:.h=.s) $(addprefix -I,$(inc-y)) $(addprefix -D,$(def-y))  $<
	$(Q)gawk 'BEGIN { RS="[\r]*\n" } /^arc_dummy_(.*)/ { var=gensub(/arc_dummy_/,"","g",$$1); getline; offset=$$2; printf "#define %s %s\n", var, offset }' $(@:.h=.s) > $@
	@rm 	$(@:.h=.s)
endif



VPATH += $(ROOTDIR)/system/$(OS_SRC)
inc-y += $(ROOTDIR)/system/$(OS_INC)
inc-y += $(ROOTDIR)/system/$(OS_SRC)
inc-y += $(ROOTDIR)/system/$(OS_INTEGRATION)
VPATH += $(ROOTDIR)/system/Os/rtos/integration
VPATH += $(ROOTDIR)/system/Os
inc-y += $(ROOTDIR)/system/Os/include

VPATH += $(ROOTDIR)/$(OSAL_ARCH)/$(ARCH_FAM)/$(ARCH)/kernel
VPATH += $(ROOTDIR)/mcal/arch/$(ARCH)/src/integration
VPATH += $(ROOTDIR)/mcal/arch/$(ARCH)/src

inc-y += .
inc-y += $(ROOTDIR)/$(OSAL_ARCH)/$(ARCH_FAM)/$(ARCH)/kernel
inc-y += $(ROOTDIR)/arch/$(ARCH_FAM)/$(ARCH)/drivers
inc-y += $(ROOTDIR)/$(OSAL_ARCH)/$(ARCH)
inc-y += $(ROOTDIR)/include/$(ARCH_FAM)
inc-y += $(ROOTDIR)/mcal/arch/$(ARCH)
inc-y += $(ROOTDIR)/mcal/arch/$(ARCH)/src
 


