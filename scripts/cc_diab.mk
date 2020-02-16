

# Diab versions and "features"
# - Can't handle initializtion with a depth more that 2,e.g. .NBYTESu.B.SMLOE = 1;
# - 5.6.0.0
#  - Do NOT use. dialect-c99 makes const (.text) end up in .data section
#    Seems to be no workarounds 
#  - 5.7.0.0
#    - "-g3" option -Xkill-opt=0x080000   
#   

# Diab SPE
#  The SPE consist of up to 4 APUs.
#    1. SPE APU - Vector stuff. 64-bit GPRS
#    2. Vector Single Prec. FP APU. 64-bit GPRS 
#    3. Scalar Single Prec. FP APU. 32-bit GPRS
#    4. Scalar Double Prec. FP APU. 64-bit GPRS
#     For more information see E500CORERM.pdf chapter 
#     3.8.1 (SPE and Embedded Floating-Point APUs)
#    
#  Most cores with SPE have 1-3 and very few have 4 (e500v2)   
#
# What does Diab generate for? 
#
 
 
DIAB_VERSION=5.9.3.0
DIAB_COMPILE ?= /c/devtools/WindRiver/diab/5.9.3.0/WIN32
DIAB_BIN = $(DIAB_COMPILE)/bin

# ---------------------------------------------------------------------------
# Compiler
# CCFLAGS - compile flags

CC	= 	$(DIAB_BIN)/dcc	

cflags-$(CFG_OPT_RELEASE) += -g3
cflags-$(CFG_OPT_DEBUG)   += -g2 -Xoptimized-debug-off
cflags-$(CFG_OPT_FLAGS)   += $(SELECT_OPT)


ifeq ($(DIAB_TARGET),)
$(error DIAB_TARGET is not defined. Check your build_config.mk for it)
endif

cflags-y += -c

cflags-y += -Xdialect-c99
cflags-y += -Xc-new
cflags-y += -Xlibc-new
cflags-y += -Xkill-opt=0x080000		# Generates wrong code on 5.7 and 5.8

cflags-y += -Xsmall-data=0
cflags-y += -Xsmall-const=0
cflags-y += -Xno-common
cflags-y += -Xnested-interrupts
#cflags-y += -Xstop-on-warning
cflags-y += -Xsection-split
cflags-y += -Xforce-prototypes
cflags-y += -Xforce-declarations
#cflags-y += -XO

# Enable inline and __ev64_opaque__
cflags-y += -Xkeywords=0x900004		
#cflags-y += -Xstmw-slow
cflags-y += -ei4618
cflags-y += -Xmake-dependency=6
cflags-y += $(DIAB_TARGET)

cflags-y += -ei5388,5387

#CFLAGS_diab_Adc_Cfg.o += -ei4068  # 4068 should be fixed in generator.
#CFLAGS_diab_Dio.o += -ei4546
#CFLAGS_diab_IoHwAb_Analog.o += -ei4111 -ei4549  
#CFLAGS_diab_IoHwAb_Digital.o += -ei4111
#CFLAGS_diab_init.o += -ei4236
#CFLAGS_diab_task.o += -ei4546 -ei4550
#CFLAGS_diab_counter.o += -ei1573
#CFLAGS_diab_application.o += -ei4186
#CFLAGS_diab_IoHwAb_Pwm.o += -ei4186 -ei4111
#CFLAGS_diab_Spi_Lcfg.o += -ei4068
#CFLAGS_diab_NvM.o += -ei4111 -ei4177
#CFLAGS_diab_counter.o += -Xlocal-data-area=0
#CFLAGS_diab_WdgM.o += -ei4186
#CFLAGS_diab_EcuM_Main.o += -ei4550 -ei4188
#CFLAGS_diab_EcuM.o += -ei4188
#CFLAGS_diab_Mcu.o += -ei4177
#CFLAGS_diab_Mcu_Cfg.o += -ei4188
#CFLAGS_diab_Can.o += -ei4550
#CFLAGS_diab_CanIf.o += -ei4550 -ei4188 -ei4111
#CFLAGS_diab_Nm.o += -ei4188
#CFLAGS_diab_arch.o += -ei1639
#CFLAGS_diab_Port.o += -ei4550


CFLAGS = $(cflags-y) $(cflags-yy) $(CFLAGS_diab_$@)

CCOUT 		= -o $@ 

SELECT_CLIB=CLIB_DIAB
CFG_ARC_CLIB?=y

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
LDFLAGS += -Xremove-unused-sections

ifeq ($(DIAB_VERSION),5.9.3.0)
  # Diab is having problems finding the right libs,
  # so do it manually
  lib-y +=-li
  lib-y +=-lchar
  lib-y +=-limpl
  lib-y +=-limpfp
  lib-y +=-lg
  lib-y +=-lc
else
  lib-y +=-lc
  lib-y +=-limpl
  lib-y +=-lm
endif 

LDOUT 		= -o $@
TE = elf
LDMAPFILE = > $(subst .elf,.map, $@)

libitem-y += $(libitem-yy)

LDFLAGS += $(ldflags-y)

# ---------------------------------------------------------------------------
# Assembler
# ---------------------------------------------------------------------------
AS	= 	$(DIAB_BIN)/das
asflags-$(CFG_OPT_DEBUG) += -g 
asflags-y += -Xsemi-is-newline
asflags-y += $(DIAB_TARGET) 
ASOUT = -o $@

ASFLAGS += $(asflags-y)

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

# Memory footprint
define do-memory-footprint 
	@gawk --non-decimal-data -f $(ROOTDIR)/scripts/memory_footprint_$(COMPILER).awk $(subst .elf,.map,$@) 
endef


define do-memory-footprint2-$(CFG_MEMORY_FOOTPRINT2)
	@gawk -f $(ROOTDIR)/scripts/memory_footprint2_$(COMPILER).awk  $(subst .$(TE),.map, $@)
endef


