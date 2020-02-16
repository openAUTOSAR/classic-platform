 
GHS_VERSION=201254
GHS_COMPILE?=/d/devtools/ghs/comp_201254
GHS_BIN = $(GHS_COMPILE)
#GHS_TARGET?=ppc560xc
GHS_TARGET?=ppc5514

# ---------------------------------------------------------------------------
# Compiler
# CCFLAGS - compile flags

CC	= 	$(GHS_BIN)/ccppc

cflags-$(CFG_OPT_RELEASE) += -Ospeed -OI -OB
cflags-$(CFG_OPT_DEBUG)   += -g -dwarf2 -Onone
cflags-$(CFG_OPT_SIZE)    += -g -dwarf2 -Osize
cflags-$(CFG_OPT_FLAGS)   += $(SELECT_OPT)


ifeq ($(GHS_TARGET),)
$(error GHS_TARGET is not defined. Check your build_config.mk for it)
endif

cflags-y += -c
cflags-y += -c99
cflags-y += --gnu_asm
cflags-$(CFG_VLE) += -vle
cflags-y += -cpu=$(GHS_TARGET)

# Generate dependencies
cflags-y 		+= -MMD

# Diagnostics
cflags-y += -errmax=10
cflags-y += --diag_suppress 618

CFLAGS = $(cflags-y) $(cflags-yy) $(CFLAGS_GHS_$@)

CCOUT 		= -o $@ 

SELECT_CLIB=CLIB_DIAB
CFG_ARC_CLIB?=y

# ---------------------------------------------------------------------------
# Preprocessor

CPP = $(CC) -E
CPPOUT = -o 
CPP_ASM_FLAGS = -cpu=$(GHS_TARGET)

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


LD = $(GHS_BIN)/ccppc.exe

LDOUT 		= -o $@
TE = elf
LDMAPFILE =-map=$(strip $(subst .elf,.map, $@))
#LD_FILE2=-lnkcmd=
LD_FILE= -T

libitem-y += $(libitem-yy)

LDFLAGS += --nocpp
# Use our own startup, not GHS
LDFLAGS += -nostartfile
#LDFLAGS += -v
LDFLAGS += -cpu=$(GHS_TARGET)
LDFLAGS += $(ldflags-y)

# ---------------------------------------------------------------------------
# Assembler
# ---------------------------------------------------------------------------
AS	= 	$(GHS_BIN)/asppc
#ASFLAGS-$(CFG_OPT_DEBUG) += -g 
#ASFLAGS += -Xsemi-is-newline
ASFLAGS += -cpu=$(GHS_TARGET) 

ASOUT = -o $@


# ---------------------------------------------------------------------------

OBJCOPY 		= $(tprefix-y)objcopy

# ---------------------------------------------------------------------------
NM 		= $(tprefix-y)nm


# ---------------------------------------------------------------------------
# Archiver

AR	= 	$(tprefix-y)ar
AROUT 	= $@

# ---------------------------------------------------------------------------
# PCLint

PCLINT_COMPILER_MAKEFILE      = $(PCLINT_FILES_DIR)/co-gcc.mak GCC_BIN=$(CC)
PCLINT_COMPILER_SETTINGS_FILE = $(PCLINT_FILES_DIR)/co-gcc.lnt

# GHS does not have multiline assembler sepateted by ";"... do we do it here manually
define do-sx-to-s-post
	gawk '{gsub(/;/,"\n")};1' $@ > $@.tmp; cp $@.tmp $@ 
endef

# Memory footprint
define do-memory-footprint 
	@gawk --non-decimal-data -f $(ROOTDIR)/scripts/memory_footprint_$(COMPILER).awk $(subst .elf,.map,$@) 
endef

define do-memory-footprint2-$(CFG_MEMORY_FOOTPRINT2)
	@gawk -f $(ROOTDIR)/scripts/memory_footprint2_$(COMPILER).awk  $(subst .$(TE),.map, $@)
endef


