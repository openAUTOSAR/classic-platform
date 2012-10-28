
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

cflags-y        += --dlib_config $(IAR_LIB)/dlhcs12bdf.h 
cflags-y 		+= -e
cflags-y 		+= --no_wrap_diagnostics
cflags-y 		+= --error_limit=10
cflags-y 		+= --silent

# z is the size options
cflags-$(CFG_OPT_RELEASE)        += -s9
cflags-$(CFG_OPT_DEBUG)        += -On  #s2

# Generate dependencies, 
#cflags-y 		+= --dependencies $*.d 

# Warnings
#cflags-y          += -W=most

# Conformance

CFLAGS = $(cflags-y) $(cflags-yy)

CCOUT 		= -o $@ 

# ---------------------------------------------------------------------------
# Preprocessor

CPP	= 	$(CC) --preprocess=n
#CPPOUT = -precompile

#CPP_ASM_FLAGS += -ppopt noline -ppopt nopragma -dialect c

comma = ,
empty = 
space = $(empty) $(empty)


#cw_lib_path += -L$(CW_COMPILE)/PowerPC_EABI_Support/Runtime/Lib
#cw_lib_path += -L$(CW_COMPILE)/PowerPC_EABI_Support/MSL/MSL_C/PPC_EABI/Lib
cc_inc_path += $(IAR_COMPILE)/inc/dlib
#cc_inc_path += $(CW_COMPILE)/PowerPC_EABI_Support/MSL/MSL_C/PPC_EABI/Include
inc-y += $(cc_inc_path)
#libpath-y += $(cw_lib_path)

# nothing really matches.......
#lib-y += -lRuntime.PPCEABI.S.a 
#lib-y += -lMSL_C.PPCEABI.bare.SZ.S.a

#C_TO_ASM = -P

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

#LDSCRIPT = -lcf

# To make "rom" images (make LOAD() work)
#ldflags-y += -romaddr 0x0 
#ldflags-y += -rambuffer 0x0
#ldflags-y += -nodefaults
#ldflags-y += -gdwarf-2
#ldflags-y += -m _start
TE = elf
#ldflags-y += -map $(subst .$(TE),.map, $@)

#LDFLAGS += $(ldflags-y) 
#LDOUT 		= -o $@

#LD_FILE = -lcf

libitem-y += $(libitem-yy)

# ---------------------------------------------------------------------------
# Assembler
#
# ASFLAGS 		- assembler flags
# ASOUT 		- how to generate output file

AS	= 	$(IAR_BIN)/ahcs12.exe

#asflags-y += --error_limit=10
#asflags-y += -proc e500 -gdwarf-2
#asflags-$(CFG_VLE) += -vle
ASFLAGS += $(asflags-y)
#ASOUT = -o $@

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






