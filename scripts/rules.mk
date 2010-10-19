
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


##### For backwards compatability with older project makefiles
# Remove dependency on libkernel
deprecated-libs += $(ROOTDIR)/libs/libkernel_$(ARCH_MCU).a
deprecated-libs-included = $(filter $(deprecated-libs),$(libitem-y))
ifneq ($(deprecated-libs-included),)
$(info >>>> Ignoring deprecated lib dependencies: $(deprecated-libs-included)')
libitem-y := $(filter-out $(deprecated-libs),$(libitem-y))
endif

# Automatic preprocessing of std linkscripts
old-ldcmdfile = $(ROOTDIR)/$(ARCH_PATH-y)/scripts/linkscript_gcc.ldf
new-ldcmdfile = linkscript_gcc.ldp
old-ldcmdfile-used = $(filter $(old-ldcmdfile),$(ldcmdfile-y))
ifneq ($(old-ldcmdfile-used),)
$(info >>>> Changing linkscript to preprocessed version: $(old-ldcmdfile) -> $(new-ldcmdfile)')
ldcmdfile-y := $(subst $(old-ldcmdfile),$(new-ldcmdfile),$(ldcmdfile-y))
vpath %.ldf $(ROOTDIR)/$(ARCH_PATH-y)/scripts
endif

#####

inc-y += $(ROOTDIR)/include
inc-y += $(ROOTDIR)/include/$(ARCH_FAM)


.PHONY clean: 
clean: FORCE
	@-rm -f *.o *.d *.h *.elf *.a *.ldp

.PHONY config: 
config: FORCE
	@echo "board   modules:" $(MOD_AVAIL)
	@echo "example modules:" $(MOD_USE)
	@echo $(MOD) ${def-y}

FORCE:

$(ROOTDIR)/binaries:
	@mkdir -p $@

# build- targets are "end" target that the included makefile want's to build
.PHONY all:
all: $(build-exe-y) $(build-hex-y) $(build-lib-y) $(build-bin-y) $(ROOTDIR)/binaries
	@cp -v $(build-lib-y) $(build-exe-y) $(build-hex-y) $(build-bin-y) $(ROOTDIR)/binaries


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


# Board linker files are in the board directory 
inc-y += $(ROOTDIR)/boards/$(BOARDDIR)

# Preprocess linker files..
%.ldp: %.ldf
	@echo "  >> CPP $<"
	$(Q)$(CPP) -E -P -x assembler-with-cpp -o $@ $(addprefix -I ,$(inc-y)) $<

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
	
$(build-bin-y): $(build-exe-y)
	@echo "  >> OBJCOPY $@"   
	$(Q)$(CROSS_COMPILE)objcopy -O binary $< $@	

# Linker
# Could use readelf -S instead of parsing the *.map file.
$(build-exe-y): $(dep-y) $(obj-y) $(sim-y) $(libitem-y) $(ldcmdfile-y)
	@echo "  >> LD $@"
ifeq ($(CROSS_COMPILE),)
	$(Q)$(CC) $(LDFLAGS) -o $@ $(libpath-y) $(obj-y) $(lib-y) $(libitem-y)	
else	
	$(Q)$(LD) $(LDFLAGS) -T $(ldcmdfile-y) -o $@ $(libpath-y) --start-group $(obj-y) $(lib-y) $(libitem-y) --end-group $(LDMAPFILE)
ifdef CFG_MC912DG128A
	@$(CROSS_COMPILE)objdump -h $@ | gawk -f $(ROOTDIR)/scripts/hc1x_memory.awk
else
	@echo "Image size: (decimal)"
	@gawk --non-decimal-data 	'/^\.text/ { print "  text:"  $$3+0 " bytes"; rom+=$$3 };\
	 							/^\.data/ { print "  data:"  $$3+0 " bytes"; rom+=$$3; ram+=$$3}; \
	 							/^\.bss/ { print "  bss :"  $$3+0 " bytes"; ram+=$$3}; \
	 							END { print "  ROM: ~" rom " bytes"; print "  RAM: ~" ram " bytes"}' $(subst .elf,.map,$@)
ifeq ($(BUILD_LOAD_MODULE),y)
	@$(CROSS_COMPILE)objcopy -O srec $@ $@.raw.s19
	srec_cat $@.raw.s19 --crop 0x8008000 0x803fffc --fill 0x00 0x8008000 0x803fffc --l-e-crc32 0x803fffc -o $@.lm.s19
endif
endif
endif
	@echo
	@echo "  >>>>>>>  DONE  <<<<<<<<<"
	@echo
	
	
$(size-exe-y): $(build-exe-y)
	$(Q)$(OBJDUMP) -h $<
	@echo TODO: Parse the file....

