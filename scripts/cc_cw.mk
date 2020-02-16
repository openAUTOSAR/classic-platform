

HOST := $(shell uname)
export prefix

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

cflags-y        += -c
cflags-y 		+= -cpp_exceptions=off
cflags-y 		+= -readonlystrings
cflags-y 		+= -RTTI=off
cflags-y 		+= -dialect=c99
cflags-y 		+= -gccext=on
cflags-y 		+= -gdwarf-2
cflags-y 		+= -gccinc
cflags-y 		+= -cwd explicit
cflags-y 		+= -msgstyle gcc
cflags-y 		+= -maxerrors 10
cflags-y 		+= -maxwarnings 10
cflags-$(CFG_OPT_RELEASE) += -opt level=2
cflags-$(CFG_OPT_DEBUG)   += -opt off 
cflags-$(CFG_OPT_FLAGS)   += $(SELECT_OPT)

# Generate dependencies, 
# Should be -MMD here but it only gives the *.c files (for some reason
# the compiler thinks all include files are system file, option?)
cflags-y 		+= -gccdepends -MD

# Warnings
cflags-y          += -W=most
#cflags-y          += -requireprotos

# Conformance
cflags-y          += -abi=eabi
cflags-$(CFG_VLE) += -ppc_asm_to_vle  # Convert ppc to vle ppc
cflags-$(CFG_VLE) += -vle  # Convert ppc to vle ppc
cflags-y          += -abi=eabi
cflags-y          += -proc=5565
cflags-$(CFG_EFPU)+= -fp=efpu
#cflags-y          += -use_isel=on
cflags-y          += -sdata=0 -sdata2=0

# Get machine cflags
#cflags-y		+= $(cflags-$(CFG_ARCH))

CFLAGS_cw_IoHwAb.o += -W=nounused
CFLAGS_cw_Dio.o +=  -W=nounused
CFLAGS_cw_IoHwAb_Analog.o +=  -W=nounused -W=nounusedexpr
CFLAGS_cw_IoHwAb_Digital.o +=  -W=nounused
CFLAGS_cw_EcuM_Main.o +=  -W=nounused -W=off
CFLAGS_cw_EcuM.o +=  -W=nounused -W=off
CFLAGS_cw_Mcu.o += -W=nounused 
CFLAGS_cw_Can.o +=  -W=nounused
CFLAGS_cw_CanIf.o +=  -W=off
CFLAGS_cw_init.o +=  -W=off
CFLAGS_cw_Nm.o +=  -W=nounused -W=off
CFLAGS_cw_Mcu_Cfg.o += -W=off
CFLAGS_cw_ComM.o +=  -W=nounused
CFLAGS_cw_CanNm.o +=  -W=nounused
CFLAGS_cw_Pwm.o +=  -W=nounused
CFLAGS_cw_Adc_eQADC.o += -W=nopossible -W=nounwanted
CFLAGS_cw_Com_misc.o +=  -W=nounused
CFLAGS_cw_EcuM_Callout_Stubs.o +=  -W=nounused
CFLAGS_cw_SchM.o +=  -W=nounused




CFLAGS = $(cflags-y) $(cflags-yy) $(CFLAGS_cw_$@)

CCOUT 		= -o $@ 

SELECT_CLIB?=CLIB_CW

# ---------------------------------------------------------------------------
# Preprocessor

CPP	= 	$(CC) -E -P
CPPOUT = -precompile

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


# MSL library magic:

# MSL_C.PPCEABI.bare.SZ.VS.a
# - Softfloat, sdata=8
# - 


# libnames .bare - No operating system
#          V     - VLE
#          S     - software float
#          E     - e500, e200
#          UC    - Noooooo, unsigned char??
#          SZ    - Size optimized
# nothing really matches.......
lib-$(CFG_VLE)-$(CFG_EFPU) += -lRuntime.PPCEABI.V.a
lib-$(CFG_VLE)-$(CFG_EFPU) += -lMSL_C.PPCEABI.bare.SZ.V.a
ifneq ($(CFG_VLE),y)
lib-$(CFG_EFPU) += -lRuntime.PPCEABI.E.a
lib-$(CFG_EFPU) += -lMSL_C.PPCEABI.bare.SZ.E.a
ifneq ($(CFG_EFPU),y)
lib-y += -lRuntime.PPCEABI.S.a 
lib-y += -lMSL_C.PPCEABI.bare.SZ.S.a
endif
endif
ifneq ($(CFG_EFPU),y)
lib-$(CFG_VLE) += -lRuntime.PPCEABI.VS.a
lib-$(CFG_VLE) += -lMSL_C.PPCEABI.bare.SZ.VS.a
endif
lib-y += $(lib-y-y)
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

# allow config to override base address
CFG_FLASH_ADR ?= 0x0
CFG_FLASH_LEN ?= 0x0
def-y += CFG_FLASH_ADR=$(CFG_FLASH_ADR)
def-y += CFG_FLASH_LEN=$(CFG_FLASH_LEN)

# To make "rom" images (make LOAD() work)
ldflags-y += -romaddr $(CFG_FLASH_ADR) 
ldflags-y += -rambuffer $(CFG_FLASH_ADR)
#ldflags-y += -nodefaults
ldflags-y += -gdwarf-2
ldflags-y += -m _start
ldflags-y += -nostdlib
ldflags-y += -code_merging all
TE = elf
ldflags-y += -map $(subst .$(TE),.map, $@)
ldflags-y += -srec $(subst .$(TE),.s19, $@)

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
asflags-y += -maxerrors 10
asflags-y += -maxwarnings 10
asflags-$(CFG_VLE) += -vle
ASFLAGS += $(asflags-y)
ASOUT = -o $@

#$(error x$(CFG_VLE)x)

# Memory footprint
define do-memory-footprint 
	@gawk --non-decimal-data -f $(ROOTDIR)/scripts/memory_footprint_$(COMPILER).awk $(subst .elf,.map,$@) 
endef

# Dependency generation
define do-compile-post 
	@sed -e "/.*PowerPC_EABI_Support/d;s/ \\\/ qqaass/;s/\\\/\//g;s/qqaass/\\\/" $(subst .o,.d,$@) > $(subst .o,.d,$@)d
	@mv $(subst .o,.d,$@)d $(subst .o,.d,$@)
endef

define do-memory-footprint2-$(CFG_MEMORY_FOOTPRINT2)
	@gawk -f $(ROOTDIR)/scripts/memory_footprint2_$(COMPILER).awk  $(subst .$(TE),.map, $@)
endef






