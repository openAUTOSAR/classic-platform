# ========================================================================= #
# 						IAR compiler - Generic settings						#
# ========================================================================= #
# ---------------------------------------------------------------------------
# Setting CC <compiler>
# ---------------------------------------------------------------------------
IAR_VERSION=8_0
IAR_COMPILE=/c/devtools/IAR/IAR_Systems/Embedded_Workbench_$(IAR_VERSION)/arm
IAR_BIN=$(IAR_COMPILE)/bin
IAR_LIB=$(IAR_COMPILE)/lib/dlib

# ---------------------------------------------------------------------------
# Arch specific settings
# Setting CC <compiler>
# Setting architecture specific cflags <compiler flags>
# Setting architecture specific asflags <assembler flags>

ifneq ($(ARCH),)
 include $(ROOTDIR)/$(ARCH_KERNEL_PATH-y)/scripts/iar.mk
endif

cflags-y += -e
cflags-y += -c
#cflags-y += --no_clustering
#cflags-y += --no_mem_idioms
#cflags-y += --no_explicit_zero_opt
cflags-y += --thumb
cflags-y += -DDEBUG_HARDFAULT
cflags-y += --debug
cflags-y += --diag_suppress=Pa050,Pe550,Pe301,Pe047
#cflags-y += --aeabi 
#cflags-y += --guard_calls

# z is the size options
cflags-$(CFG_OPT_RELEASE) += -Ohs
cflags-$(CFG_OPT_DEBUG)   += -On
opt-cflags-$(CFG_OPT_SIZE) += -Ohz
cflags-$(CFG_OPT_FLAGS)   += $(SELECT_OPT)

CFLAGS = $(cflags-y) $(cflags-yy)

CCOUT 		= -o $@ 

SELECT_CLIB?=CLIB_IAR

# ---------------------------------------------------------------------------
# Preprocessor

CPP	= 	$(CC) --preprocess=n

comma = ,
empty = 
space = $(empty) $(empty)

cc_inc_path += $(IAR_COMPILE)/inc/c
inc-y += $(cc_inc_path)
#libpath-y += $(IAR_COMPILE)/lib/rt7M_tl.a
# ---------------------------------------------------------------------------
# Linker
#
# LDFLAGS 		- linker flags
# LDOUT   		- How to Generate linker output file
# LDMAPFILE     - How to generate mapfile 
# ldcmdfile-y	- link cmd file
# libpath-y		- lib paths
# libitem-y		- the libs with path
# lib-y			- the libs, without path


#ldflags-y += --skip_dynamic_initialization
#ldflags-y += --enable_stack_usage
ldflags-y += --entry Reset_Handler
#ldflags-y += --no_library_search
#ldflags-y += --no_entry
ldflags-y += --diag_suppress Lp049

TE = elf
LDMAPFILE = --map $(subst .$(TE),.map, $@)

LDFLAGS += $(ldflags-y)
libitem-y += $(libitem-yy)
############################################
#ldflags-y += -s _start
#ldflags-y += -Felf -xms 

# ---------------------------------------------------------------------------
# Assembler
#
# ASFLAGS 		- assembler flags
# ASOUT 		- how to generate output file

asflags-y += -r
ASFLAGS += $(asflags-y)


# Memory footprint
define do-memory-footprint 
	@gawk --non-decimal-data -f $(ROOTDIR)/scripts/memory_footprint_$(COMPILER).awk $(subst .elf,.map,$@) 
endef

# Dependency generation
#define do-compile-post 
#	@sed -e "/.*PowerPC_EABI_Support/d;s/ \\\/ qqaass/;s/\\\/\//g;s/qqaass/\\\/" $(subst .o,.d,$@) > $(subst .o,.d,$@)d
#	@mv $(subst .o,.d,$@)d $(subst .o,.d,$@)
#endef

define do-memory-footprint2-$(CFG_MEMORY_FOOTPRINT2)
	@gawk -f $(ROOTDIR)/scripts/memory_footprint2_$(COMPILER).awk  $(subst .$(TE),.map, $@)
endef






