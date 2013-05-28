
# Errors in manual:
# -s2 is NOT marked as deprecated in manual. Compiler says use -On, that is not in the manual.

# Preprocessor:
#  __ICCHCS12__ - Identifies the IAR compiler platform (preferreed)
#  __IAR_SYSTEMS_ICC__ -  Identifies the IAR C/C++ Compiler for HCS12
#  __IAR_SYSTEMS_ASM__  - asm 


#IAR_COMPILE=/c/devtools/IAR_5.6/hcs12
IAR_BIN=$(IAR_COMPILE)/bin
IAR_LIB=$(IAR_COMPILE)/lib/dlib


# ---------------------------------------------------------------------------
# Compiler
# CCFLAGS - compile flags

CC	= 	$(IAR_BIN)/icchcs12.exe 

cflags-y        += --dlib_config $(IAR_LIB)/dlhcs12bff.h 
cflags-y 		+= -e
cflags-y 		+= --no_wrap_diagnostics
cflags-y 		+= --error_limit=10
cflags-y 		+= --silent
cflags-y 		+= --debug
cflags-y 		+= --code_model=banked 
cflags-y 		+= -lA $@.lst #Get a list file for each file
cflags-y 		+= --diag_suppress=Pa050,Pe550,Pe188

# z is the size options
cflags-$(CFG_OPT_RELEASE) += -Ohz
cflags-$(CFG_OPT_DEBUG)   += -On
cflags-$(CFG_OPT_FLAGS)   += $(SELECT_OPT)

# Generate dependencies, 
#cflags-y 		+= --dependencies $*.d 

# Warnings
#cflags-y          += -W=most

# Conformance

CFLAGS = $(cflags-y) $(cflags-yy)

CCOUT 		= -o $@ 

SELECT_CLIB?=CLIB_IAR

# ---------------------------------------------------------------------------
# Preprocessor

CPP	= 	$(CC) --preprocess=n
#CPPOUT = -precompile

#CPP_ASM_FLAGS += -ppopt noline -ppopt nopragma -dialect c

comma = ,
empty = 
space = $(empty) $(empty)


cc_inc_path += $(IAR_COMPILE)/inc/dlib
cc_inc_path += $(IAR_COMPILE)/inc/
inc-y += $(cc_inc_path)
libpath-y += $(IAR_COMPILE)/lib/dlib

lib-y += dlhcs12bff.r12

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


LD = $(IAR_BIN)/xlink.exe

ldflags-y += -s _start
ldflags-y += -Felf -xms
ldflags-y += -f linkscript_iar.lcf
TE = elf
ldflags-y += -l$(subst .$(TE),.map, $@)

LDFLAGS += $(ldflags-y) 
#LDOUT 		= -o $@

#LD_FILE = -lcf

libitem-y += $(libitem-yy)

# ---------------------------------------------------------------------------
# Assembler
#
# ASFLAGS 		- assembler flags
# ASOUT 		- how to generate output file

AS	= 	$(IAR_BIN)/ahcs12.exe

asflags-y += -r
ASFLAGS += $(asflags-y)


# Memory footprint
#define do-memory-footprint 
#	@gawk --non-decimal-data -f $(ROOTDIR)/scripts/memory_footprint_$(COMPILER).awk $(subst .elf,.map,$@) 
#endef

# Dependency generation
#define do-compile-post 
#	@sed -e "/.*PowerPC_EABI_Support/d;s/ \\\/ qqaass/;s/\\\/\//g;s/qqaass/\\\/" $(subst .o,.d,$@) > $(subst .o,.d,$@)d
#	@mv $(subst .o,.d,$@)d $(subst .o,.d,$@)
#endef

#define do-memory-footprint2-$(CFG_MEMORY_FOOTPRINT2)
#	@gawk -f $(ROOTDIR)/scripts/memory_footprint2_$(COMPILER).awk  $(subst .$(TE),.map, $@)
#endef






