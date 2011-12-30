

# Arch specific settings
ifneq ($(ARCH),)
 include $(ROOTDIR)/$(ARCH_PATH-y)/scripts/gcc.mk
endif


HOST := $(shell uname)
export prefix

# If we are using codesourcery and cygwin..
ifneq ($(findstring CYGWIN,$(UNAME)),)
cygpath:= $(shell cygpath -m $(shell which cygpath))
export CYGPATH=$(cygpath)
endif

# ---------------------------------------------------------------------------
# Compiler
# CCFLAGS - compile flags

CC	= 	$(CROSS_COMPILE)gcc
cflags-$(CFG_OPT_RELEASE) += -O3
cflags-$(CFG_OPT_DEBUG) += -g -O0

# Remove sections if needed.. may be problems with other compilers here.
#cflags-y += -ffunction-sections

ifneq ($(filter -O2 -O3 -O1,$(cflags-y)),) 
	cflags-y += -fno-schedule-insns -fno-schedule-insns2
endif

#cflags-y 		+= -c 
#cflags-y 		+= -fno-common
cflags-y 		+= -std=gnu99

# Generate dependencies
cflags-y 		+= -MMD

# Warnings
cflags-y          += -Wall
#cflags-y          += -Winline	# warn if inline failed
#cflags-y          += -pedantic

# Conformance
cflags-y          += -fno-strict-aliasing	# Use restict keyword instead.
cflags-y          += -fno-builtin

# Get machine cflags
#cflags-y		+= $(cflags-$(ARCH))

CFLAGS = $(cflags-y) $(cflags-yy)

CCOUT 		= -o $@ 

# ---------------------------------------------------------------------------
# Preprocessor

CPP	= 	$(CC) -E
CPPOUT = -o
CPP_ASM_FLAGS = -x assembler-with-cpp 

comma = ,
empty = 
space = $(empty) $(empty)

# Note!
# Libs related to GCC(libgcc.a, libgcov.a) is located under 
# lib/gcc/<machine>/<version>/<multilib>
# Libs related to the library (libc.a,libm.a,etc) are under:
# <machine>/lib/<multilib>
# 
# Can't remember why haven't I just used gcc to link instead of ld? (it should 
# figure out the things below by itself)

# It seems some versions of make want "\=" and some "="
# "=" - msys cpmake on windows 7 
gcc_lib_path := "$(subst /libgcc.a,,$(shell $(CC) $(CFLAGS) --print-libgcc-file-name))" 
gcc_lib_path := $(subst \libgcc.a,,$(gcc_lib_path)) 
lib_lib_path := "$(subst /libc.a,,$(shell $(CC) $(CFLAGS) --print-file-name=libc.a))"
lib_lib_path := $(subst \libc.a,,$(lib_lib_path))
text_chunk := $(subst \,/,$(shell touch gcc_path_probe.c; $(CC) -v -c gcc_path_probe.c &> gcc_path_probe.tmp;gawk -f $(TOPDIR)/scripts/gcc_getinclude.awk gcc_path_probe.tmp))
cc_inc_path := $(realpath $(text_chunk))
libpath-y += -L$(lib_lib_path)
libpath-y += -L$(gcc_lib_path)

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


LD = $(CROSS_COMPILE)ld


LD_FILE = -T

LDOUT 		= -o $@
TE = elf

# Don't use a map file if we are compiling for native target.
ifneq ($(CROSS_COMPILE),) 
LDFLAGS += -Map $(subst .$(TE),.map, $@) 
endif



libitem-y += $(libitem-yy)

LD_START_GRP = --start-group
LD_END_GRP = --end-group

#LDFLAGS += --gc-section

# ---------------------------------------------------------------------------
# Assembler
#
# ASFLAGS 		- assembler flags
# ASOUT 		- how to generate output file

AS	= 	$(CROSS_COMPILE)as

ASFLAGS += --gdwarf2
ASOUT = -o $@

# ---------------------------------------------------------------------------
# Dumper

#DDUMP          = $(Q)$(COMPILER_ROOT)/$(cross_machine-y)-objcopy
#DDUMP_FLAGS    = -O srec
OBJCOPY 		= $(CROSS_COMPILE)objcopy

# ---------------------------------------------------------------------------
# Archiver
#
# AROUT 		- archiver flags

AR	= 	$(CROSS_COMPILE)ar
AROUT 	= $@


ifdef CFG_HC1X
define do-memory-footprint
	@$(CROSS_COMPILE)objdump -h $@ | gawk -f $(ROOTDIR)/scripts/hc1x_memory.awk
endef	
else
define do-memory-footprint
	@gawk --non-decimal-data -f $(ROOTDIR)/scripts/memory_footprint_gcc.awk $(subst .elf,.map,$@)
endef
endif	



