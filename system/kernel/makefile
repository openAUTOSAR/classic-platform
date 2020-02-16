

#VPATH += $(ROOTDIR)/arch/arm/arm_cm3/kernel
vpath-y += $(ARCH_PATH-y)/kernel
#obj-y += asm_sample.o
#CFLAGS_asm_sample.o += -O3
obj-y += arch_krn.o
obj-$(CFG_HC1X) += arch_irq.o
#obj-y += stm32f10x_it.o

# object files 
obj-y += event.o
obj-y += os_init.o
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
obj-$(CFG_MULTICORE) += spinlock.o
obj-$(CFG_MULTICORE) += multicore.o
obj-$(USE_KERNEL_EXTRA) += sleep.o
obj-$(USE_KERNEL_EXTRA) += semaphore.o


# These are VERY cpu dependent.. remove
#obj-y += int_ctrl.o
obj-y += irq.o

# We are compiling the kernel
dep-$(CFG_PPC) += arch_offset.h
dep-y += asm_offset.h
#dep-y += kernel_offset.h
dep-$(CFG_ARM_CM3) += arch_offset.h


ifeq ($(COMPILER),gcc)
# ARM assembler generates "define STACK_APA $12". The extra '$' we want to go.
# Assembler offsets
%_offset.h: %_offset.c
	@echo "  >> generating $@ from $<"
	$(Q)$(CC) -S $(CFLAGS) -o $(@:.h=.s) $(addprefix -I,$(inc-y)) $(addprefix -D,$(def-y)) $<
	$(Q)gawk '/.*define/ { print $$1 " " $$2 " " gensub("#","","g",$$3) }' $(@:.h=.s)  > $@

# Assembler offsets
asm_offset.h: asm_offset.c
	@echo "  >> asm offset gen $<"
	$(Q)$(CC) -S $(CFLAGS) -o $(<:.c=.s) $(addprefix -I,$(inc-y)) $(addprefix -D,$(def-y))  $<
	@$(SED) -e "/#define/!d" -e "s/#//2g" $(<:.c=.s) > $@	
	@rm 	$(<:.c=.s)
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
	$(Q)$(CC) -S $(filter-out -c -dwarf2,$(CFLAGS)) -o $(<:.c=.s) $(addprefix -I,$(inc-y)) $(addprefix -D,$(def-y))  $<
	@$(SED) -e "/#define/!d" -e "s/#//2g" $(<:.c=.s) > $@	
	@rm 	$(<:.c=.s)
endif



# sed -n '/apa.*/ {h;n;s/q://;x;s/^/#define /;s/$//;x;p}' bepa.c
# gawk '/apa/ { var=$0;getline tjo;print "#define " var tjo}' bepa.c
# $ gawk '/^arc_dummy/ { va=gensub(/arc_dummy_(.*):.*/,"\\1",$0);getline;printf "
#define %-30s %s\n",va,$2 }' asm_offset.h


ifeq ($(COMPILER),iar)
%_offset.h: %_offset.c
	@echo "  >> generating $@ from $<"
	$(CC) -lB $@_ $(CFLAGS) $(addprefix -I,$(inc-y)) $< -o $@__ 
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
 


