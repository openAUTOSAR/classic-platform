
# Diab versions and "features"
# - Can't handle initializtion with a depth more that 2,e.g. .NBYTESu.B.SMLOE = 1;
# - 5.6.0.0
#  - Do NOT use. dialect-c99 makes const (.text) end up in .data section
#    Seems to be no workarounds 
#  - 5.7.0.0  
#   

DIAB_COMPILE ?= /c/devtools/WindRiver/diab/5.9.0.0/WIN32
DIAB_BIN = $(DIAB_COMPILE)/bin

# ---------------------------------------------------------------------------
# Compiler
# CCFLAGS - compile flags

CC	= 	$(DIAB_BIN)/dcc	
ifeq ($(OPT_DEBUG),y)
cflags-y += -O0
else
#cflags-y += -Os
endif

ifeq ($(DIAB_TARGET),)
$(error DIAB_TARGET is not defined. Check your build_config.mk for it)
endif

cflags-y += -c
cflags-y += -g3

cflags-y += -Xdialect-c99
cflags-y += -Xc-new
cflags-y += -Xlibc-new

cflags-y += -Xsmall-data=0
cflags-y += -Xsmall-const=0
cflags-y += -Xno-common
cflags-y += -Xnested-interrupts
#cflags-y += -Xstop-on-warning
cflags-y += -Xsection-split
cflags-y += -Xforce-prototypes
cflags-y += -Xforce-declarations
#cflags-y += -XO
cflags-y += -Xkeywords=0x4		# Enable inline keywork
#cflags-y += -Xstmw-slow
cflags-y += -ei4618
cflags-y += -Xmake-dependency=6
cflags-y += $(DIAB_TARGET)

CFLAGS = $(cflags-y) $(cflags-yy)

CCOUT 		= -o $@ 

# ---------------------------------------------------------------------------
# Preprocessor

CPP = $(CC) -P
CPPOUT = -o 
CPP_ASM_FLAGS = $(DIAB_TARGET)
CPP_ASM_FLAGS += -Xpreprocess-assembly
#CPP_ASM_FLAGS += -Xkeep-assembly-file

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


LD = $(DIAB_BIN)/dld.exe

LDFLAGS += $(DIAB_TARGET)
LDFLAGS += -m6

lib-y += -lc
lib-y += -limpl
lib-y += -li

LDOUT 		= -o $@
TE = elf
LDMAPFILE = > $(subst .elf,.map, $@)

libitem-y += $(libitem-yy)

LDFLAGS += $(ldflags-y)

# ---------------------------------------------------------------------------
# Assembler
# ---------------------------------------------------------------------------
AS	= 	$(DIAB_BIN)/das
ASFLAGS += -Xsemi-is-newline
ASFLAGS += $(DIAB_TARGET) 
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

define do-memory-footprint2-$(CFG_MEMORY_FOOTPRINT2)
	@gawk -f $(ROOTDIR)/scripts/memory_footprint2_$(COMPILER).awk  $(subst .$(TE),.map, $@)
endef


