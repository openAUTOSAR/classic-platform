HOST := $(shell uname)
export prefix

# ---------------------------------------------------------------------------
# Compiler
# CCFLAGS - compile flags
DIAB_BIN = $(DIAB_COMPILE)/5.9.0.0/WIN32/bin
DIAB_LIB = $(DIAB_COMPILE)/5.9.0.0/WIN32/lib

TARGET = -tPPCVLEEN:cross	# powerpc VLE no floating point support

CC	= 	$(DIAB_BIN)/dcc
cflags-y += $(TARGET)
cflags-y += -Xkeywords=4 # enable inline keywork
cflags-y += -Xgcc-options-on # try to use gcc options
cflags-y += -Xdialect-c99
cflags-y += -Xdebug-dwarf2
cflags-y += -Xconventions-eabi
cflags-y += -g # Generate symbolic debugger information
cflags-y += -Xlicense-proxy-use=0 # dont use a license proxy
cflags-$(CFG_OPT_RELEASE) += -O3
cflags-$(CFG_OPT_DEBUG) += -g -O0

cflags-y 		+= -std=gnu99

# Warnings
cflags-y          += -Wall
# Conformance
cflags-y          += -fno-strict-aliasing	# Use restict keyword instead.
cflags-y          += -fno-builtin

CFLAGS = $(cflags-y) $(cflags-yy)
CCOUT 		= -o $@ 

# ---------------------------------------------------------------------------
# Preprocessor
# ---------------------------------------------------------------------------
CPP	= 	$(CC) -P
CPP_ASM_FLAGS += -Xpreprocess-assembly 
CPP_ASM_FLAGS += -Xkeep-assembly-file
comma = ,
empty = 
space = $(empty) $(empty)

lib-y += -lc
lib-y += -limpl

cc_inc_path += $(DIAB_COMPILE)/5.9.0.0/include/diab
cc_inc_path += $(DIAB_COMPILE)/5.9.0.0/include/

inc-y += $(cc_inc_path)
libpath-y += $(diab_lib_path)

# ---------------------------------------------------------------------------
# Linker
# ---------------------------------------------------------------------------
LD = $(DIAB_BIN)/dld.exe

LD_FILE =#no flag needed, just the link file name
LDOUT 		= -o $@
TE = elf

libitem-y += $(libitem-yy)

LDFLAGS += $(TARGET)
LDFLAGS += -m6

LDMAPFILE = > $@.map
# ---------------------------------------------------------------------------
# Assembler
# ---------------------------------------------------------------------------
AS = $(DIAB_BIN)/das
ASFLAGS += -Xsemi-is-newline
ASFLAGS += $(TARGET)
ASOUT = -o $@




