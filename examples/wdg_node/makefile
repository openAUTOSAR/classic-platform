# Project files (adds all .c files in project root)
PROJECT_C_FILES=$(notdir $(wildcard ../*.c))
obj-y += $(PROJECT_C_FILES:%.c=%.o)

VPATH += ..

inc-y += $(ROOTDIR)/system/kernel/$(objdir)
inc-y += $(ROOTDIR)/system/kernel/include

# The more precise configuration, the higher preceedance.
VPATH := ../config/$(BOARDDIR) ../config $(VPATH)
inc-y := ../config/$(BOARDDIR) ../config $(inc-y)



# linkfile
ldcmdfile-y = linkscript_gcc.ldp
vpath %.ldf $(ROOTDIR)/$(ARCH_PATH-y)/scripts

# What I want to build
build-exe-y = wdgm_node_project.elf


#################### Arctic Core make system ######################
# Following is an explanation of the most usable features 
# of the Arctic Core make system.
#
# obj-y : list of object files to build.
# VPATH : list of directories in which to
#         look for source files.
# inc-y : list of directories in which to
#         look for header files.
# libitem-y : list of libraries to include. 
# build-exe-y : the name of build the output file.
# ldcmdfile-y: path to linkscript, used when building "build-exe-y"
#
# For more information see:
# http://arccore.com/wiki/Makesystem
###################################################################
