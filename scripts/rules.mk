
###############################################################################
# BUILD SETTINGS                                                              #
###############################################################################

# By default, create a srec file.

# Board settings
include $(ROOTDIR)/boards/build_config_bsw.mk
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

ifeq (${COMPILER},armcc)
ARMCC_COMPILE?=${DEFAULT_ARMCC_COMPILE}
endif



DATE=$(shell date +"%F %X")
cflags-y += -DBUILD_DATE="$(DATE)"

###############################################################################
# TOOLS CONFIGURATION                                                         #
###############################################################################

# set debug optimization level as default
SELECT_OPT?=OPT_DEBUG

# Create srecords by default
CFG_CREATE_SREC?=n

# Do a library build
CFG_BUILD_AS_LIB = $(BUILD_AS_LIB)

ifeq ($(findstring OPT_,$(SELECT_OPT)),OPT_)
  $(eval CFG_$(SELECT_OPT)=y)
else
  CFG_OPT_FLAGS=y
endif

$(info SELECT_OPT=$(SELECT_OPT) $(origin SELECT_OPT))
ECUM_INC=system/EcuM/inc
OS_TEST_FOLDER?=system/Os/utest
OS_INC=Os/rtos/inc
OSAL_ARCH=system/Os/osal
ARCH_KERNEL_PATH-y = $(OSAL_ARCH)/$(ARCH_FAM)/$(ARCH)
ARCH_DRIVER_PATH-y = mcal/arch/$(ARCH)

CFG_CC_$(COMPILER)=y

# Include compiler settings
include $(ROOTDIR)/scripts/cc_$(COMPILER).mk

# Include pclint or splint settings
ifneq ($(PCLINT),)
include $(ROOTDIR)/scripts/cc_pclint.mk
endif
ifneq ($(SPLINT),)
include $(ROOTDIR)/scripts/cc_splint.mk
endif

SRECORD_PATH?=/c/devtools/srecord
SREC_CAT=$(SRECORD_PATH)/srec_cat.exe
SREC_INFO=$(SRECORD_PATH)/srec_info.exe
SREC_CMP=$(SRECORD_PATH)/srec_cmp.exe


###############################################################################
# PROJECT MAKEFILE                                                            #
###############################################################################

$(info from rules.mk, $(abspath $(CURDIR)))
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

#####

inc-y += ../include
inc-y += $(ROOTDIR)/include
inc-y += $(ROOTDIR)/include/rte
inc-y += $(ROOTDIR)/include/$(ARCH_FAM)
inc-y += $(ROOTDIR)/include/$(ARCH_FAM)/$(ARCH)
inc-y += $(ROOTDIR)/$(ECUM_INC)

inc-y += $(inc-y-y)
  
# Override system includes...  
ifeq ($(COMPILER),armcc)
cflags-y += $(addprefix -J,$(inc-system-y) $(inc-system2-y) )
else    
inc-y :=  $(inc-system-y) $(inc-system2-y) $(inc-y) $(inc-last-y) 
endif

# Find default linker files
inc-y += $(ROOTDIR)/scripts/linker

VPATH := $(vpath-first-y) $(VPATH) $(vpath-last-y)

###############################################################################
# POST BUILD                                                                  #
###############################################################################

all-mod-post += $(build-srec-pb-y)
ifeq ($(CFG_POSTBUILD),y)
all-mod += PreCompiledDataHash.h
endif

# The memory bounderies for post build section shall be defined in build_config.mk
def-$(CFG_POSTBUILD) += POSTBUILD_ADDRESS=$(POSTBUILD_ADDRESS_START)
def-$(CFG_POSTBUILD) += POSTBUILD_ADDRESS_END=$(POSTBUILD_ADDRESS_STOP)

###############################################################################
# LINT                                                                        #
###############################################################################

LINT_EXCLUDE_PATHS := $(foreach path,$(LINT_EXCLUDE_PATHS), $(abspath $(path)) )
$(info $(LINT_EXCLUDE_PATHS))

LINT_BAD_EXCLUDE_PATHS := $(filter %/,$(LINT_EXCLUDE_PATHS))
ifneq ($(LINT_BAD_EXCLUDE_PATHS),)
$(warning LINT_EXCLUDE_PATHS entries must not end in '/'. Ignoring $(LINT_BAD_EXCLUDE_PATHS))
endif

LINT_NICE_EXCLUDE_PATHS := $(filter-out %/,$(LINT_EXCLUDE_PATHS))
LINT_NICE_EXCLUDE_PATHS := $(foreach path,$(LINT_NICE_EXCLUDE_PATHS),$(path)/)

LINT_NICE_EXCLUDE_PATHS := $(foreach path,$(LINT_NICE_EXCLUDE_PATHS),$(subst /C/, , $(path)))

ifneq ($(PCLINT),)
define run_pclint

$(if 
$(filter "match", $(foreach ex_path,$(LINT_NICE_EXCLUDE_PATHS), $(if $(findstring $(ex_path), $(dir $(abspath $<))),"match", ))),
$(info $(abspath $<):0:0: Info: Not running lint check on $(abspath $<)),
$(Q)$(PCLINT) $(addprefix $(lintinc_ext),$(inc-y)) $(lint_extra) $(addprefix $(lintdef_ext),$(def-y) $(DEF_$@)) $(abspath $<))
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

.PHONY : distro_dir
distro_dir : FORCE
	@echo ">>>> Creating ditribution directory: " $(DISTRONAME)
	@mkdir -p  $(BDIR)/$(DISTRONAME)
	@chmod 777 $(BDIR)/$(DISTRONAME)

.PHONY : copydist
copydist : FORCE
	@echo ">>>> Copying files"
	@cp -R $(ROOTDIR)/.settings $(BDIR)/$(DISTRONAME)	
	@cp -R $(ROOTDIR)/include $(BDIR)/$(DISTRONAME)
	@cp $(ROOTDIR)/base/*.* $(BDIR)/$(DISTRONAME)/include/
	@cp $(ROOTDIR)/base/compiler/*.* $(BDIR)/$(DISTRONAME)/include/
	@cp -R $(ROOTDIR)/scripts $(BDIR)/$(DISTRONAME)
	@cp -R $(ROOTDIR)/arxml $(BDIR)/$(DISTRONAME)
	@mkdir -p $(BDIR)/$(DISTRONAME)/boards
	@cp -R $(ROOTDIR)/boards/$(BOARDDIR) $(BDIR)/$(DISTRONAME)/boards/
	@cp -R $(ROOTDIR)/boards/generic $(BDIR)/$(DISTRONAME)/boards/
	@cp -R $(ROOTDIR)/boards/board_common.mk $(BDIR)/$(DISTRONAME)/boards/
	@cp -R $(ROOTDIR)/boards/build_config_bsw.mk $(BDIR)/$(DISTRONAME)/boards/
	@mkdir -p $(BDIR)/$(DISTRONAME)/$(OSAL_ARCH)
	@mkdir -p $(BDIR)/$(DISTRONAME)/$(OSAL_ARCH)/$(ARCH_FAM) 
	@mkdir -p $(BDIR)/$(DISTRONAME)/$(OSAL_ARCH)/$(ARCH_FAM)/$(ARCH)
	@mkdir -p $(BDIR)/$(DISTRONAME)/$(OSAL_ARCH)/$(ARCH_FAM)/$(ARCH)/kernel
	@cp -R $(ROOTDIR)/$(OSAL_ARCH)/$(ARCH_FAM)/$(ARCH)/scripts $(BDIR)/$(DISTRONAME)/$(OSAL_ARCH)/$(ARCH_FAM)/$(ARCH)/
	@cp -R $(ROOTDIR)/$(OSAL_ARCH)/$(ARCH_FAM)/$(ARCH)/kernel/*.h $(BDIR)/$(DISTRONAME)/$(OSAL_ARCH)/$(ARCH_FAM)/$(ARCH)/kernel
	@cp $(ROOTDIR)/system/Os/rtos/inc/Os.h $(BDIR)/$(DISTRONAME)/include
	@cp -R $(ROOTDIR)/integration $(BDIR)/$(DISTRONAME)/
	@cp $(ROOTDIR)/scripts/project_tmpl.txt $(BDIR)/$(DISTRONAME)/.project
	@cp $(ROOTDIR)/makefile $(BDIR)/$(DISTRONAME)/
		
.PHONY : copyapp
copyapp : $(COPY_APP_INCLUDES)
	@mkdir -p $(BDIR)/$(DISTRONAME)/libs
	@cp $(ROOTDIR)/binaries/$(board_name)/$(PROJECTNAME).a $(BDIR)/$(DISTRONAME)/libs
	
.PHONY: distro
distro: distro_dir copydist copyapp
	@echo ">>>> DONE"

.PHONY: clean_distro
clean_distro:
	rm -R $(BDIR)/$(DISTRONAME)

# Clean rules are split, clean_obj only removes the local obj folder
.PHONY clean clean_obj: 
clean_obj: FORCE
	@echo
	@echo "  >> Rules Clean $(CURDIR)"
	$(Q)-rm -v *

# Clean both the local obj folder, and the central binaries folder 
clean: clean_obj
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

.PHONY : show_build
show_build: FORCE
	@echo ""
	@echo "==========[ BUILD INFO ]==========="
	@echo "  Core version:   $(core_version)"
	@echo "  BDIR:           $(BDIR) [$(origin BDIR)]"
	@echo "  BOARDDIR:       $(BOARDDIR) [$(origin BOARDDIR)]"
	@echo "  COMPILER:       $(COMPILER) [$(origin COMPILER)]"	
ifeq ($(COMPILER),diab)
	@echo "  DIAB_COMPILE:   $(DIAB_COMPILE) [$(origin DIAB_COMPILE)]"
	@echo "  Compiler info:  $(shell ${DIAB_COMPILE}/bin/dcc -V)"
else ifeq ($(COMPILER),cw)	
	@echo "  CW_COMPILE:     $(CW_COMPILE) [$(origin CW_COMPILE)]"
	@echo "  Compiler info:  $(shell ${CW_COMPILE}/PowerPC_EABI_Tools/Command_Line_Tools/mwcceppc.exe -version)"
else ifeq ($(COMPILER),iar)	
	@echo "  IAR_COMPILE:     $(IAR_COMPILE) [$(origin IAR_COMPILE)]"
else ifeq ($(COMPILER),ghs)	
	@echo "  GHS_COMPILE:     $(GHS_COMPILE) [$(origin GHS_COMPILE)]"
ifneq ($(wildcard ${GHS_COMPILE}/ccrh850.exe),) 
	@echo "  Compiler info:  $(shell ${GHS_COMPILE}/ccrh850.exe -V dummy)"
endif
ifneq ($(wildcard ${GHS_COMPILE}/ccppc.exe),) 
	@echo "  Compiler info:  $(shell ${GHS_COMPILE}/ccppc.exe -V dummy)"
endif		
else ifeq ($(COMPILER),iar)	
	@echo "  IAR_COMPILE:     $(IAR_COMPILE) [$(origin IAR_COMPILE)]"
else ifeq ($(COMPILER),armcc)	
	@echo "  ARMCC_COMPILE:     $(ARMCC_COMPILE) [$(origin ARMCC_COMPILE)]"	
	@echo "  Compiler info:  $(shell $(ARMCC_COMPILE)/bin/armcc --version_number)"
else 
ifneq ($(CLANG_COMPILE),)
	@echo "  CLANG_COMPILE:  $(CLANG_COMPILE) [$(origin CLANG_COMPILE)]"
	@echo "  CFG_CLANG_SAFECODE:  $(CFG_CLANG_SAFECODE) [$(origin CFG_CLANG_SAFECODE)]"
endif
	@echo "  CROSS_COMPILE:  $(CROSS_COMPILE) [$(origin CROSS_COMPILE)]"
	@echo "  Compiler info:  $(shell ${CROSS_COMPILE}gcc --version)"
endif
	@echo "  SELECT_OPT:     $(SELECT_OPT) [$(origin SELECT_OPT)]"
	@echo "  CURDIR:         $(CURDIR)"
	@echo "  SELECT_CONSOLE: $(SELECT_CONSOLE) [$(origin SELECT_CONSOLE)]"



FORCE:

$(ROOTDIR)/binaries/$(board_name):
	@mkdir -p $@

all-mod += $(build-hex-y) $(build-exe-y) $(build-srec-y) 
all-mod += $(build-lib-y) $(build-bin-y)
ifneq ($(wildcard $(SREC_CAT)),)
all-mod-post += $(build-srec-fill-y)
endif


	

.PHONY all:
all: |  show_build module_config $(ROOTDIR)/binaries/$(board_name) $(all-mod) $(all-mod-post)
all: show_build module_config $(all-mod) $(all-mod-post) $(ROOTDIR)/binaries/$(board_name) 
	@cp -v $(all-mod) $(all-mod-post) $(os-orti-y) $(ROOTDIR)/binaries/$(board_name)

.SUFFIXES:
.PRECIOUS: %.s

###############################################################################
# TARGETS                                                                     #
###############################################################################
	
obj-y := $(filter-out $(rm-obj-y),$(obj-y)) 	
	
# Simple depencendy stuff
-include $(subst .o,.d,$(obj-y))
# Some dependency for xxx_offset.c/h also
-include $(subst .h,.d,$(dep-y))

# Compile
%.o: %.c
	@echo "  >> CC $(notdir $<)"
	$(Q)$(CC_PRE)$(CC) $(filter-out $(rm-cflags),$(CFLAGS)) $(CFLAGS_$@) $(cflags-post-y) -o $(goal) $(addprefix -I,$(inc-y)) $(addprefix -D,$(def-y) $(DEF_$@)) $(abspath $<)
	$(do-compile-post)
		
# run lint if enabled
	$(run_pclint)
	$(run_splint)

%.o: %.cpp
	@echo "  >> CXX $(notdir $<)"
	$(Q)$(CXX) $(filter-out $(rm-cxxflags),$(CXXFLAGS)) $(CFLAGS_$@) $(cflags-post-y) -o $(goal) $(addprefix -I,$(inc-y)) $(addprefix -D,$(def-y) $(DEF_$@)) $(abspath $<)
	$(do-compile-post)
	

# Assembler
%.o: %.s
	@echo "  >> AS $(notdir $<)"
	$(Q)$(AS) $(filter-out $(rm-asflags),$(ASFLAGS)) -o $(goal) $<
	
# PP Assembler	

%.s: %.sx
	@echo "  >> CPP $(notdir $<)"
	$(Q)$(CPP) $(CPP_ASM_FLAGS) $(CPPOUT) $@ $(addprefix -I,$(inc-y)) $(addprefix -D,$(def-y)) $<
	$(do-sx-to-s-post)

#	cat $@

# Board linker files are in the board directory 
inc-y += $(board_path)


# *.ldf (file on disc) -> *.lcf (preprocessed *.ldf file)
# Preprocess linker files..
%.lcf: %.ldf
	@echo "  >> CPP $(notdir $<)"
ifeq ($(COMPILER),iar)	
	@cp $< $@
else
	$(Q)$(CPP) $(CPP_ASM_FLAGS) $(CPPOUT) $@ $(addprefix -I,$(inc-y)) $(addprefix -D,$(def-y)) $<
endif

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
	$(Q)$(AR) $(ARFLAGS) -r -o $@ $(obj-y) 2> /dev/null

# hex output
$(build-hex-y): $(build-exe-y)
	@echo
	@echo "  >> OBJCOPY $@"   
	$(Q)$(CROSS_COMPILE)objcopy -O ihex $< $@

# srecord output
$(build-srec-y): $(build-exe-y)
	$(do-objcopy)

CROP_SREC?=0
	
# srecord filled output
$(build-srec-fill-y): $(build-srec-y)
	$(Q)$(SREC_CAT) $< -fill 0xff -over $< -o $@
ifneq (${CROP_SREC},0)	 
	$(Q)$(SREC_CAT) $@ -crop 0x0 ${CROP_SREC} -o $@ 
endif
	$(Q)$(SREC_INFO) $@

# Create Post build image
$(build-srec-pb-y): $(build-srec-fill-y)
	$(Q)$(SREC_CAT) $< -crop $(POSTBUILD_ADDRESS_START) $(POSTBUILD_ADDRESS_STOP) -o $@
	$(Q)$(SREC_INFO) $@
	
# bin output
#$(build-bin-y): $(build-exe-y)
#	@echo
#	@echo "  >> OBJCOPY $@"   
#	$(Q)$(CROSS_COMPILE)objcopy -O binary $< $@	

ifeq ($(BOARDDIR),gnulinux)
$(build-exe-y): $(dep-y) $(obj-y) $(os-orti-y) $(sim-y) $(libitem-y)
	@echo
	@echo "  >> LD Using gcc to link gnulinux board. $@"
	$(Q)$(CC) -Xlinker -Map=$(basename $(@F)).map -o $@ $(libpath-y) $(obj-y) $(lib-y) $(libitem-y) $(filter-out $(rm-ldflags),$(LDFLAGS))
	$(do-ld-post)
	@echo
	@echo "  >>>>>>>  DONE  <<<<<<<<<"
	@echo
else
$(build-exe-y): $(dep-y) $(obj-y) $(os-orti-y) $(sim-y) $(libitem-y) $(ldcmdfile-y)
	@echo
	@echo "  >> LD $@"
	$(do-ld-pre)
ifeq ($(COMPILER),iar)
	$(Q)$(LD) $(filter-out $(rm-ldflags),$(LDFLAGS)) --config $(ldcmdfile-y) --map $(basename $(@F)).map $(sort $(obj-y)) $(libpath-y) $(lib-y) $(libitem-y) -o $@
else ifeq ($(CROSS_COMPILE)$(COMPILER),gcc)
	$(Q)$(CC_PRE)$(CC) -Xlinker -Map=$(basename $(@F)).map -o $@ $(libpath-y) $(obj-y) $(lib-y) $(libitem-y)	$(filter-out $(rm-ldflags),$(LDFLAGS))
else
	$(Q)$(LD) $(filter-out $(rm-ldflags),$(LDFLAGS)) $(LD_FILE) $(LD_FILE2)$(ldcmdfile-y) -o $@ $(libpath-y) $(LD_START_GRP) $(obj-y) $(lib-y) $(libitem-y) $(LD_END_GRP) $(LDMAPFILE)
	$(do-memory-footprint)
	$(do-memory-footprint2-y)
endif #($(CROSS_COMPILE),)
	$(do-ld-post)
	@echo
	@echo "  >>>>>>>  DONE  <<<<<<<<<"
	@echo
endif #($(BOARDDIR),gnulinux)	

# Create the Post-Build checksum file
#
# Note: Since we cant use VPATH for these file, the path to the config folder is needed. 
#       The variable pb-pc-path must be specified in the application build-config.mk
PreCompiledDataHash.h: $(addprefix $(pb-pc-path)/,$(pb-pc-file-y))
	$(Q)cat $(^) | $(SED) $$'s/$$/\r/' | md5sum | $(SED) 's/\([a-f0-9]\{16\}\)\([a-f0-9]\{16\}\).*/#define PRE_COMPILED_DATA_HASH_LOW 0x\1=#define PRE_COMPILED_DATA_HASH_HIGH 0x\2/' | tr "=" "\n" > $@
