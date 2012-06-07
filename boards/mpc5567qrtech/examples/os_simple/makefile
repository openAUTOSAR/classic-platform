

PROJECTNAME=os_simple
ROOTDIR?=../../../..
include $(ROOTDIR)/scripts/project_defaults.mk

ifneq (${MAKELEVEL},0) 

	# object files
	obj-y += os_simple.o
	obj-y += system_hooks.o
	
	VPATH += $(ROOTDIR)/examples
	VPATH += $(ROOTDIR)/examples/$(PROJECTNAME)
	
ifneq ($(CFG_BOOT),)

# "downloader" need an srecord file 
build-srec-image-y = $(PROJECTNAME)_srec_image.srec

build-srec-y = $(PROJECTNAME).srec
build-bin-image-y = $(PROJECTNAME)_bin_image.bin
# To add a header with mkimage we need a binary file
build-bin-y = $(PROJECTNAME).bin

all-mod += $(build-srec-image-y)
all-mod += $(build-srec-y)
all-mod += $(build-bin-image-y)
##all-mod += $(build-bin-y)

mkimage = /c/Users/mahi/git/bootloader/bin/mkimage

SRECORD_PATH=/c/devtools/srecord
SREC_CAT=$(Q)$(SRECORD_PATH)/srec_cat.exe
SREC_INFO=$(Q)$(SRECORD_PATH)/srec_info.exe

# Changes:
# * Here, 2 places
# * linkfile  (linkscript_cw.ldf)
# * cc_cw.mk  (romaddr, etc) 

LOAD_ADDR=0x1c000

# To srecord again to load in tool
$(build-srec-image-y): $(build-bin-image-y)
	@echo ">> Binary to SREC to load with tool"
	$(SREC_CAT) $< -binary -offset ${LOAD_ADDR} -o $@
	$(SREC_INFO) $@

$(build-srec-y) : $(build-exe-y)
	@echo
	@echo "  >> OBJCOPY $@"   
	$(Q)$(CROSS_COMPILE)objcopy -O srec $< $@
	$(SREC_INFO) $@

# Binary image down to 0
$(build-bin-y) : $(build-srec-y)
	@echo ">> srec to binary with offset 0"
	$(SREC_INFO) $< 
	$(SREC_CAT) $< -offset - -minimum-addr $< -o $@ -binary

# Add image header 		
# Should extract the start address with "objdump -f"
$(build-bin-image-y): $(build-bin-y)
	@echo ">> Adding header" 
	$(mkimage) -o $@ -l 0x1c100 -s 0x1c6d0 -m "APP" -v $<

endif


endif





