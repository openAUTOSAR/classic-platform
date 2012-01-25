
# Diab versions and "features"
# 5.6.0.0->5.9.0.0  
#  - Can't handle initializtion with a depth more that 2,e.g. .NBYTESu.B.SMLOE = 1;
#    (in code this works fine)
#
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

cflags-y += -XO

#TARGET = -tPPCE200Z1VFN:simple
TARGET = -tPPCVLEFS:simple
cflags-y += $(TARGET) 

# Treat warning as error
cflags-y += -Werror

#cflags-y 		+= -c 
#cflags-y 		+= -fno-common
#cflags-y 		+= -std=gnu99
cflags-y 		+= -Xc-new		# const const problems
#cflags-y 		+= -Xdialect-c99
cflags-y 		+= -Xsuppress-warnings
cflags-y 		+= -Xkeywords=0x4		# Enable inline keywork

# Generate dependencies
#cflags-y 		+= -MMD

# Warnings
cflags-y          += -Wall

# Conformance
#cflags-y          += -fno-strict-aliasing
#cflags-y          += -fno-builtin

# Get machine cflags
#cflags-y		+= $(cflags-$(ARCH))

CFLAGS = $(cflags-y) $(cflags-yy)

CCOUT 		= -o $@ 

# ---------------------------------------------------------------------------
# Preprocessor

CPP = $(CC) -P
CPPOUT = -o 
CPP_ASM_FLAGS = $(TARGET)
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

LDFLAGS += $(TARGET)
LDFLAGS += -m6

lib-y += -lm
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
ASFLAGS += $(TARGET) 
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
