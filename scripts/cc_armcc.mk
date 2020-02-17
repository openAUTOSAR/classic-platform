 
ARMCC_BIN = $(ARMCC_COMPILE)/bin
ARMCC_TARGET?=--cpu=Cortex-A9

# ---------------------------------------------------------------------------
# Compiler
# CCFLAGS - compile flags

CC	= 	$(ARMCC_BIN)/armcc

cflags-$(CFG_OPT_RELEASE) += 
cflags-$(CFG_OPT_DEBUG)   += -g -O0
cflags-$(CFG_OPT_SIZE)    += -g -O3
cflags-$(CFG_OPT_FLAGS)   += $(SELECT_OPT)

cflags-y += -c
cflags-y += --c99

# Move to target part
 
cflags-y += $(ARMCC_TARGET)
cflags-y += --fpu=vfpv3
cflags-y += --thumb
cflags-y += --diag_style=gnu
cflags-y += --dwarf2
cflags-y += --min_array_alignment=4
cflags-y += --no_unaligned_access
cflags-y += --library_interface=none

# aeabi_clib99

# Generate dependencies
cflags-y 		+= --md 	

CFLAGS = $(cflags-y) $(cflags-yy) $(CFLAGS_ARMCC_$@)

CCOUT 		= -o $@ 

CFG_ARC_CLIB?=y

inc-system2-y += $(ARMCC_COMPILE)/include


# ---------------------------------------------------------------------------
# Preprocessor

CPP = $(CC) -E
CPPOUT = -o 
CPP_ASM_FLAGS = $(ARMCC_TARGET)

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


# Note!
#  gnu ld scripts are not supported by armcc for bare metal links 

LD = $(ARMCC_BIN)/armlink.exe

LDOUT 		= -o $@
TE = elf
LDMAPFILE =--map --info=sizes,totals --show_cmdline --symbols --list=$(strip $(subst .elf,.map, $@))
#  --verbose --xref --xrefdbg
LD_FILE2= --scatter=


ldflags-y += --entry=0
ldflags-y += --no_legacyalign
ldflags-y += --thumb2_library
ldflags-y += --datacompressor=off
libitem-y += $(libitem-yy)

LDFLAGS += $(ARMCC_TARGET)
LDFLAGS += $(ldflags-y)



# ---------------------------------------------------------------------------
# Assembler
# ---------------------------------------------------------------------------
AS	= 	$(ARMCC_BIN)/armasm
ASFLAGS += $(ARMCC_TARGET) 
ASFLAGS += -g
ASFLAGS += --diag_style=gnu
ASFLAGS += --thumb
ASFLAGS += --dwarf2
ASFLAGS += --apcs=//interwork

ASOUT = -o $@


