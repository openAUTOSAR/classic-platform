
###############################################################################
# BUILD SETTINGS                                                              #
###############################################################################

# Board settings
include $(board_path)/build_config.mk

# Project settings
-include ../build_config.mk

# Perform build system version check
include $(ROOTDIR)/scripts/version_check.mk


###############################################################################
# MODULE CONFIGURATION                                                        #
###############################################################################

# Some useful vars
CFG_ARCH_$(ARCH):=y
RELDIR := $(subst $(TOPDIR)/,,$(CURDIR))
target := $(subst /,_,$(SUBDIR))
goal=$@

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
$(foreach mod,$(sort $(MOD_USE)),$(eval $(call MOD_USE_template,${mod})))
$(foreach mod,$(CFG),$(eval $(call CFG_template,${mod})))
#def-y += $(ARCH) $(ARCH_FAM) $(ARCH_MCU) 

# Select console / debug
$(foreach mod,$(SELECT_OS_CONSOLE),$(eval $(call MOD_USE_template,${mod})))
$(foreach mod,$(SELECT_CONSOLE),$(eval $(call MOD_USE_template,${mod})))

def-y += SELECT_OS_CONSOLE=$(if $(SELECT_OS_CONSOLE),$(SELECT_OS_CONSOLE),TTY_NONE)
def-y += SELECT_CONSOLE=$(if $(SELECT_CONSOLE),$(SELECT_CONSOLE),TTY_NONE)
def-$(USE_DEBUG_PRINTF) += USE_DEBUG_PRINTF 

not_avail = $(filter-out $(MOD_AVAIL),$(sort $(MOD_USE)))
ifneq ($(not_avail),)
$(error Trying to build a module that is not available: $(not_avail))
endif

COMPILER?=gcc
ifeq (${COMPILER},gcc)
# Assume that the board have set DEFAULT_CROSS_COMPILE
CROSS_COMPILE?=${DEFAULT_CROSS_COMPILE}
endif

ifeq (${COMPILER},cw)
CW_COMPILE?=${DEFAULT_CW_COMPILE}
endif
ifeq (${COMPILER},iar)
IAR_COMPILE?=${DEFAULT_IAR_COMPILE}
endif
ifeq (${COMPILER},diab)
DIAB_COMPILE?=${DEFAULT_DIAB_COMPILE}
endif

ifeq (${COMPILER},ghs)
GHS_COMPILE?=${DEFAULT_GHS_COMPILE}
endif

# Check cross compiler setting against default from board config
ifeq (${COMPILER},cw)
ifneq (${CW_COMPILE},${DEFAULT_CW_COMPILE})
${warning Not using default cross compiler for architecture.}
${warning CW_COMPILE:            ${CW_COMPILE} [${origin CW_COMPILE}]}
${warning DEFAULT_CW_COMPILE:    ${DEFAULT_CW_COMPILE} [${origin DEFAULT_CW_COMPILE}]}
endif
else ifeq (${COMPILER},iar)
ifneq (${IAR_COMPILE},${DEFAULT_IAR_COMPILE})
${warning Not using default cross compiler for architecture.}
${warning IAR_COMPILE:            ${IAR_COMPILE} [${origin IAR_COMPILE}]}
${warning DEFAULT_IAR_COMPILE:    ${DEFAULT_IAR_COMPILE} [${origin DEFAULT_IAR_COMPILE}]}
endif
else ifeq (${COMPILER},diab)
ifneq (${DIAB_COMPILE},${DEFAULT_DIAB_COMPILE})
${warning Not using default cross compiler for architecture.}
${warning DIAB_COMPILE:            ${DIAB_COMPILE} [${origin DIAB_COMPILE}]}
${warning DEFAULT_DIAB_COMPILE:    ${DEFAULT_DIAB_COMPILE} [${origin DEFAULT_DIAB_COMPILE}]}
endif
else
ifneq (${DEFAULT_CROSS_COMPILE},)
ifneq (${CROSS_COMPILE},${DEFAULT_CROSS_COMPILE})
#${warning Not using default cross compiler for architecture.}
#${warning CROSS_COMPILE:         ${CROSS_COMPILE} [${origin CROSS_COMPILE}]}
#${warning DEFAULT_CROSS_COMPILE: ${DEFAULT_CROSS_COMPILE} [${origin DEFAULT_CROSS_COMPILE}]}
endif
endif
endif



###############################################################################
# TOOLS CONFIGURATION                                                         #
###############################################################################

# set debug optimization level as default
SELECT_OPT?=OPT_DEBUG


ifeq ($(findstring OPT_,$(SELECT_OPT)),OPT_)
  $(eval CFG_$(SELECT_OPT)=y)
else
  CFG_OPT_FLAGS=y
endif

$(info SELECT_OPT=$(SELECT_OPT) $(origin SELECT_OPT))

ARCH_PATH-y = arch/$(ARCH_FAM)/$(ARCH)

# Include compiler settings
include $(ROOTDIR)/scripts/cc_$(COMPILER).mk

# Include pclint or splint settings
ifneq ($(PCLINT),)
include $(ROOTDIR)/scripts/cc_pclint.mk
endif
ifneq ($(SPLINT),)
include $(ROOTDIR)/scripts/cc_splint.mk
endif



###############################################################################
# PROJECT MAKEFILE                                                            #
###############################################################################

# Include project makefile
include ../makefile





# All module object files (guarded for backwards compatability)
ifndef _BOARD_COMMON_MK
include $(ROOTDIR)/boards/board_common.mk
endif

# Misc tools
ifneq ($(CFG_BOOT),)
include $(board_path)/boot_info.mk
include $(ROOTDIR)/scripts/bootloader_image.mk
endif  

##### For backwards compatability with older project makefiles:

# Remove dependency on libkernel
deprecated-libs += $(ROOTDIR)/libs/libkernel_$(ARCH_MCU).a
deprecated-libs-included = $(filter $(deprecated-libs),$(libitem-y))
ifneq ($(deprecated-libs-included),)
$(info >>>> Ignoring deprecated lib dependencies: $(deprecated-libs-included)')
libitem-y := $(filter-out $(deprecated-libs),$(libitem-y))
endif

# Automatic preprocessing of std linkscripts
old-ldcmdfile = $(ROOTDIR)/$(ARCH_PATH-y)/scripts/linkscript_$(COMPILER).ldf
new-ldcmdfile = linkscript_gcc.ldp
old-ldcmdfile-used = $(filter $(old-ldcmdfile),$(ldcmdfile-y))
ifneq ($(old-ldcmdfile-used),)
$(info >>>> Changing linkscript to preprocessed version: $(old-ldcmdfile) -> $(new-ldcmdfile)')
ldcmdfile-y := $(subst $(old-ldcmdfile),$(new-ldcmdfile),$(ldcmdfile-y))
vpath %.ldf $(ROOTDIR)/$(ARCH_PATH-y)/scripts
endif

#####

inc-y += ../include
inc-y += $(ROOTDIR)/include
inc-y += $(ROOTDIR)/include/$(ARCH_FAM)
inc-y += $(ROOTDIR)/include/$(ARCH_FAM)/$(ARCH)
  
inc-y :=  $(inc-system-y) $(inc-y) 

# Find default linker files
inc-y += $(ROOTDIR)/scripts/linker

###############################################################################
# POST BUILD                                                                  #
###############################################################################

def-$(CFG_POSTBUILD) += CFG_POSTBUILD


###############################################################################
# LINT                                                                        #
###############################################################################

LINT_EXCLUDE_PATHS := $(abspath $(LINT_EXCLUDE_PATHS))
$(info $(LINT_EXCLUDE_PATHS))

LINT_BAD_EXCLUDE_PATHS := $(filter %/,$(LINT_EXCLUDE_PATHS))
ifneq ($(LINT_BAD_EXCLUDE_PATHS),)
$(warning LINT_EXCLUDE_PATHS entries must not end in '/'. Ignoring $(LINT_BAD_EXCLUDE_PATHS))
endif

LINT_NICE_EXCLUDE_PATHS := $(filter-out %/,$(LINT_EXCLUDE_PATHS))
LINT_NICE_EXCLUDE_PATHS := $(foreach path,$(LINT_NICE_EXCLUDE_PATHS),$(path)/)

ifneq ($(PCLINT),)
define run_pclint
$(if 
$(filter $(dir $(abspath $<)),$(LINT_NICE_EXCLUDE_PATHS)),
$(info $(abspath $<):0:0: Info: Not running lint check on $(abspath $<)),
$(Q)$(PCLINT) $(lint_extra) $(addprefix $(lintinc_ext),$(inc-y)) $(addprefix $(lintdef_ext),$(def-y) $(DEF_$@)) $(abspath $<))
endef
endif

ifneq ($(SPLINT),)
define run_splint
$(if 
$(filter $(dir $(abspath $<)),$(LINT_NICE_EXCLUDE_PATHS)),
$(info $(abspath $<):0:0: Info: Not running lint check on $(abspath $<)),
$(Q)$(SPLINT) $(splint_extra) $(addprefix $(lintinc_ext),$(inc-y)) $(addprefix $(lintdef_ext),$(def-y) $(DEF_$@)) $(abspath $<))
endef
endif



###############################################################################
# TOP-LEVEL TARGETS                                                           #
###############################################################################

.PHONY clean: 
clean: FORCE
	@echo
	@echo "  >> Rules Clean $(CURDIR)"
	$(Q)-rm -v *
	$(Q)-rm -v $(ROOTDIR)/binaries/$(board_name)/*
	
.PHONY : config 
config: FORCE
	@echo ">>>> Available modules:" $(sort $(MOD_AVAIL))
	@echo ">>>> Used modules:     " $(sort $(MOD_USE)) 
	@echo $(MOD) ${def-y}

.PHONY : module_config
module_config: FORCE
	@echo ">>>> Available modules:" $(sort $(MOD_AVAIL))
	@echo ">>>> Used modules:     " $(sort $(MOD_USE)) 


FORCE:

$(ROOTDIR)/binaries/$(board_name):
	@mkdir -p $@

all-mod += $(build-hex-y) $(build-exe-y) $(build-srec-y) 
all-mod += $(build-lib-y) $(build-bin-y)

.PHONY all:
all: | module_config $(ROOTDIR)/binaries/$(board_name) $(all-mod) $(all-mod-post)
all: module_config $(all-mod) $(all-mod-post) $(ROOTDIR)/binaries/$(board_name) 
	@cp -v $(all-mod) $(all-mod-post) $(os-orti-y) $(ROOTDIR)/binaries/$(board_name)

.SUFFIXES:


###############################################################################
# TARGETS                                                                     #
###############################################################################
	
# Simple depencendy stuff
-include $(subst .o,.d,$(obj-y))
# Some dependency for xxx_offset.c/h also
-include $(subst .h,.d,$(dep-y))

# Compile
%.o: %.c
	@echo
	@echo "  >> CC $(notdir $<)"
	$(Q)$(CC) $(CFLAGS) $(CFLAGS_$@) $(cflags-post-y) -o $(goal) $(addprefix -I,$(inc-y)) $(addprefix -D,$(def-y) $(DEF_$@)) $(abspath $<)
	$(do-compile-post)
# run lint if enabled
	$(run_pclint)
	$(run_splint)

# Assembler
%.o: %.s
	@echo
	@echo "  >> AS $(notdir $<)"
	$(Q)$(AS) $(ASFLAGS) -o $(goal) $<
	
# PP Assembler	

.SECONDARY %.s: # Don't remove *.s files (needed for debugging)

%.s: %.sx
	@echo
	@echo "  >> CPP $(notdir $<)"
	$(Q)$(CPP) $(CPP_ASM_FLAGS) $(CPPOUT) $@ $(addprefix -I,$(inc-y)) $(addprefix -D,$(def-y)) $<
	$(do-sx-to-s-post)

#	cat $@

# Board linker files are in the board directory 
inc-y += $(board_path)


# *.ldf (file on disc) -> *.lcf (preprocessed *.ldf file)
# Preprocess linker files..
%.lcf %.ldp: %.ldf
	@echo
	@echo "  >> CPP $(notdir $<)"
	$(Q)$(CPP) $(CPP_ASM_FLAGS) $(CPPOUT) $@ $(addprefix -I,$(inc-y)) $(addprefix -D,$(def-y)) $<

#$(error $(os-orti-y))

# ORTI file
$(os-orti-y): Os.orti
	@cp $< $@

.PHONY $(ROOTDIR)/libs:
$(ROOTDIR)/libs:
	$(Q)mkdir -p $@

dep-y += $(ROOTDIR)/libs
	
# lib output
$(build-lib-y): $(dep-y) $(obj-y)
	@echo
	@echo "  >> AR $@"   
	$(Q)$(AR) -r -o $@ $(obj-y) 2> /dev/null

# hex output
$(build-hex-y): $(build-exe-y)
	@echo
	@echo "  >> OBJCOPY $@"   
	$(Q)$(CROSS_COMPILE)objcopy -O ihex $< $@
	
# bin output
$(build-bin-y): $(build-exe-y)
	@echo
	@echo "  >> OBJCOPY $@"   
	$(Q)$(CROSS_COMPILE)objcopy -O binary $< $@	

# Linker
$(build-exe-y): $(dep-y) $(obj-y) $(os-orti-y) $(sim-y) $(libitem-y) $(ldcmdfile-y)
	@echo
	@echo "  >> LD $@"
ifeq ($(COMPILER),iar)
	$(Q)$(LD) $(obj-y) $(LDFLAGS) -o $@ -I$(libpath-y) $(lib-y) $(libitem-y)	
else ifeq ($(CROSS_COMPILE)$(COMPILER),gcc)
	$(Q)$(CC) $(LDFLAGS) -Xlinker -Map=$(basename $(@F)).map -o $@ $(libpath-y) $(obj-y) $(lib-y) $(libitem-y) 	
else
	$(Q)$(LD) $(LDFLAGS) $(LD_FILE) $(LD_FILE2)$(ldcmdfile-y) -o $@ $(libpath-y) $(LD_START_GRP) $(obj-y) $(lib-y) $(libitem-y) $(LD_END_GRP) $(LDMAPFILE)
	$(do-memory-footprint)
	$(do-memory-footprint2-y)
endif #($(CROSS_COMPILE),)
	@echo
	@echo "  >>>>>>>  DONE  <<<<<<<<<"
	@echo
	
