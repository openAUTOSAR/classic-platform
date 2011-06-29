

HOST := $(shell uname)
export prefix

# If we are using codesourcery and cygwin..
ifneq ($(findstring CYGWIN,$(UNAME)),)
cygpath:= $(shell cygpath -m $(shell which cygpath))
export CYGPATH=$(cygpath)
endif

# CW and paths...
# Bin:   PowerPC_EABI_Tools/Command_Line_Tools/mwXXXXXX
# libs:  PowerPC_EABI_Support/Runtime/Lib/Runtime.XXXX
#        PowerPC_EABI_Support/MSL/MSL_C/PPC_EABI/Lib

#CW_COMPILE=/c/devtools/cw_55xx
CW_BIN = $(CW_COMPILE)/PowerPC_EABI_Tools/Command_Line_Tools
CW_LIB = $(CW_COMPILE)/PowerPC_EABI_Support/Runtime/Lib


# ---------------------------------------------------------------------------
# Compiler
# CCFLAGS - compile flags

CC	= 	$(CW_BIN)/mwcceppc.exe
#cflags-$(CFG_OPT_RELEASE) += -O3
#cflags-$(CFG_OPT_DEBUG) += make  -O0

#cflags-y 		+= -c 
cflags-y 		+= -dialect=c99
cflags-y 		+= -gccext=on
cflags-y 		+= -gdwarf-2
cflags-y 		+= -gccinc
cflags-y 		+= -cwd explicit

# Generate dependencies
cflags-y 		+= -gccdepends -MMD

# Warnings
cflags-y          += -W=most

# Conformance
cflags-y          += -abi=eabi
cflags-$(CFG_VLE) += -ppc_asm_to_vle  # Convert ppc to vle ppc
cflags-$(CFG_VLE) += -vle  # Convert ppc to vle ppc
cflags-y          += -abi=eabi
cflags-y          += -proc=5565
cflags-y          += -fp=soft
cflags-y          += -use_isel=on
#cflags-y          += -sdata=0xFFFF -sdata2=16
cflags-y          += -sdata=0xFFFF -sdata2=0

#cflags-y          += -fno-strict-aliasing
#cflags-y          += -fno-builtin

# Get machine cflags
#cflags-y		+= $(cflags-$(ARCH))

CFLAGS = $(cflags-y) $(cflags-yy)

CCOUT 		= -o $@ 

# ---------------------------------------------------------------------------
# Preprocessor

CPP	= 	$(CC) -E

CPP_ASM_FLAGS += -ppopt noline -ppopt nopragma -dialect c

comma = ,
empty = 
space = $(empty) $(empty)


cw_lib_path += -L$(CW_COMPILE)/PowerPC_EABI_Support/Runtime/Lib
cw_lib_path += -L$(CW_COMPILE)/PowerPC_EABI_Support/MSL/MSL_C/PPC_EABI/Lib
cc_inc_path += $(CW_COMPILE)/PowerPC_EABI_Support/MSL/MSL_C/MSL_Common/Include
cc_inc_path += $(CW_COMPILE)/PowerPC_EABI_Support/MSL/MSL_C/PPC_EABI/Include
inc-y += $(cc_inc_path)
libpath-y += $(cw_lib_path)


# libnames .bare - No operating system
#          .E    - e500, e200
#          UC    - Noooooo
#          S     - software float
# nothing really matches.......
lib-$(CFG_VLE) += -lRuntime.PPCEABI.VS.a   # is this VLE?
lib-$(CFG_VLE) += -lMSL_C.PPCEABI.bare.SZ.VS.a
ifneq ($(CFG_VLE),y)
lib-y += -lRuntime.PPCEABI.S.a 
lib-y += -lMSL_C.PPCEABI.bare.SZ.S.a
endif

C_TO_ASM = -P

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


LD = $(CW_BIN)/mwldeppc.exe

LDSCRIPT = -lcf

# To make "rom" images (make LOAD() work)
ldflags-y += -romaddr 0x0 
ldflags-y += -rambuffer 0x0
#ldflags-y += -nodefaults
ldflags-y += -gdwarf-2
ldflags-y += -m _start
TE = elf
ldflags-y += -map $(subst .$(TE),.map, $@)

LDFLAGS += $(ldflags-y) 
LDOUT 		= -o $@

LD_FILE = -lcf

libitem-y += $(libitem-yy)

# ---------------------------------------------------------------------------
# Assembler
#
# ASFLAGS 		- assembler flags
# ASOUT 		- how to generate output file

AS	= 	$(CW_BIN)/mwasmeppc.exe

asflags-y += -gnu_mode
asflags-y += -proc e500 -gdwarf-2
asflags-$(CFG_VLE) += -vle
ASFLAGS += $(asflags-y)
ASOUT = -o $@

# ---------------------------------------------------------------------------
# Dumper

#DDUMP          = $(Q)$(COMPILER_ROOT)/$(cross_machine-y)-objcopy
#DDUMP_FLAGS    = -O srec
#OBJCOPY 		= $(CROSS_COMPILE)objcopy

# ---------------------------------------------------------------------------
# Archiver
#
# AROUT 		- archiver flags

#AR	= 	$(CROSS_COMPILE)ar
#AROUT 	= $@




