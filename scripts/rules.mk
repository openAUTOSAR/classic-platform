
CFG_ARCH_$(ARCH):=y

RELDIR := $(subst $(TOPDIR)/,,$(CURDIR))

# Create the target name... 
target := $(subst /,_,$(SUBDIR))

#goal=$(subst /cygdrive/c/,c:/,$(abspath $@))
#goal=$(abspath $@)
goal=$@

#===== MODULE CONFIGURATION =====
include $(ROOTDIR)/boards/$(BOARDDIR)/build_config.mk
-include ../build_config.mk

define MOD_AVAIL_template
	MOD_$(1)=y
endef

define MOD_USE_template
	USE_$(1)=y
	def-y += USE_$(1)
endef

define CFG_template
	CFG_$(1)=y
	def-y += CFG_$(1)
endef

$(foreach mod,$(MOD_AVAIL),$(eval $(call MOD_AVAIL_template,${mod})))
$(foreach mod,$(MOD_USE),$(eval $(call MOD_USE_template,${mod})))
$(foreach mod,$(CFG),$(eval $(call CFG_template,${mod})))
def-y += $(ARCH) $(ARCH_FAM) $(ARCH_MCU) 

# Select console / debug
$(foreach mod,$(SELECT_OS_CONSOLE),$(eval $(call MOD_USE_template,${mod})))
$(foreach mod,$(SELECT_CONSOLE),$(eval $(call MOD_USE_template,${mod})))

def-y += SELECT_OS_CONSOLE=$(if $(SELECT_OS_CONSOLE),$(SELECT_OS_CONSOLE),TTY_NONE)
def-y += SELECT_CONSOLE=$(if $(SELECT_CONSOLE),$(SELECT_CONSOLE),TTY_NONE)
def-$(USE_DEBUG_PRINTF) += USE_DEBUG_PRINTF 

not_avail = $(filter-out $(MOD_AVAIL),$(MOD_USE))
ifneq ($(not_avail),)
$(error Trying to build a module that is not available: $(not_avail))
endif

#===== COMPILER CONFIG =====

# set debug optimization level as default
ifeq ($(SELECT_OPT),)
SELECT_OPT=OPT_DEBUG
endif

$(eval CFG_$(SELECT_OPT)=y)

ARCH_PATH-y = arch/$(ARCH_FAM)/$(ARCH)

# Include compiler generic and arch specific
COMPILER?=gcc
ifneq ($(ARCH),)
include $(ROOTDIR)/$(ARCH_PATH-y)/scripts/gcc.mk
endif
include $(ROOTDIR)/scripts/cc_$(COMPILER).mk

# Get object files
include ../makefile

inc-y += $(ROOTDIR)/include
#inc-$(CFG_PPC) += $(ROOTDIR)/include/ppc
#inc-$(CFG_ARM) += $(ROOTDIR)/include/arm
inc-y += $(ROOTDIR)/include/$(ARCH_FAM)

.PHONY config:

config:
	@echo "board   modules:" $(MOD_AVAIL)
	@echo "example modules:" $(MOD_USE)
	@echo $(MOD) ${def-y}


$(ROOTDIR)/binaries:
	@mkdir -p $@

# build- targets are "end" target that the included makefile want's to build
all: $(build-exe-y) $(build-hex-y) $(build-lib-y) $(ROOTDIR)/binaries
	@cp -v $(build-lib-y) $(build-exe-y) $(build-hex-y) $(ROOTDIR)/binaries

#.PHONY post_process:
#post_process:: $(ROOTDIR)/binaries
	 

# Determine what kind of filetype to build from  
VPATH += $(ROOTDIR)/$(SUBDIR)/src
VPATH += $(ROOTDIR)/$(SUBDIR)

inc-y += ../include

.SUFFIXES:

# Simple depencendy stuff
-include $(subst .o,.d,$(obj-y))
# Some dependency for xxx_offset.c/h also
-include $(subst .h,.d,$(dep-y))

# Compile
%.o: %.c
	@echo "  >> CC $(notdir $<)"
	$(Q)$(CC) -c $(CFLAGS) -o $(goal) $(addprefix -I ,$(inc-y)) $(addprefix -D,$(def-y)) $(abspath $<)

# Assembler

%.o: %.s
	@echo "  >> AS $(notdir $<)  $(ASFLAGS)"
	$(Q)$(AS) $(ASFLAGS) -o $(goal) $<
	
# PP Assembler	
#.SECONDARY %.s:

%.s: %.sx
	@echo "  >> CPP $(notdir $<)"
	$(Q)$(CPP) -x assembler-with-cpp -E -o $@ $(addprefix -I ,$(inc-y)) $(addprefix -D,$(def-y)) $<


#	@cat $@ 
	
.PHONY $(ROOTDIR)/libs:
$(ROOTDIR)/libs:
	$(Q)mkdir -p $@

dep-y += $(ROOTDIR)/libs
	
# lib	
$(build-lib-y): $(dep-y) $(obj-y)
	@echo "  >> AR $@"   
	$(Q)$(AR) -r -o $@ $(obj-y) 2> /dev/null

$(build-hex-y): $(build-exe-y)
	@echo "  >> OBJCOPY $@"   
	$(Q)$(CROSS_COMPILE)objcopy -O ihex $< $@

# Could use readelf -S instead of parsing the *.map file.
$(build-exe-y): $(dep-y) $(obj-y) $(sim-y) $(libitem-y) $(ldcmdfile-y)
	@echo "  >> LD $@"
	$(Q)$(LD) $(LDFLAGS) -T $(ldcmdfile-y) -o $@ $(libpath-y) --start-group $(obj-y) $(lib-y) $(libitem-y) --end-group $(LDMAPFILE)
ifdef CFG_MC912DG128A
	@$(CROSS_COMPILE)objdump -h $@ | gawk -f $(ROOTDIR)/scripts/hc1x_memory.awk
else
	@echo "Image size: (decimal)"
	@gawk --non-decimal-data 	'/^\.text/ { print "  text:"  $$3+0 " bytes"; rom+=$$3 };\
	 							/^\.data/ { print "  data:"  $$3+0 " bytes"; rom+=$$3; ram+=$$3}; \
	 							/^\.bss/ { print "  bss :"  $$3+0 " bytes"; ram+=$$3}; \
	 							END { print "  ROM: ~" rom " bytes"; print "  RAM: ~" ram " bytes"}' $(subst .elf,.map,$@)
endif
	@echo "  >>>>>>>  DONE  <<<<<<<<<"
	

$(size-exe-y): $(build-exe-y)
	$(Q)$(OBJDUMP) -h $<
	@echo TODO: Parse the file....

.PHONY clean:
	@-rm -f *.o *.d *.h *.elf *.a
