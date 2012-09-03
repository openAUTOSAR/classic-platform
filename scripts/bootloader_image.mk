

MKIMAGE?=/c/Users/mahi/git/bootloader/bin/mkimage
SRECORD_PATH?=/c/devtools/srecord
SREC_CAT=$(Q)$(SRECORD_PATH)/srec_cat.exe
SREC_INFO=$(Q)$(SRECORD_PATH)/srec_info.exe

# "downloader" need an srecord file 
build-srec-image-y = $(PROJECTNAME)_srec_image.srec
build-srec-y = $(PROJECTNAME).srec
build-bin-image-y = $(PROJECTNAME)_bin_image.bin

# To add a header with mkimage we need a binary file
build-bin-y = $(PROJECTNAME).bin

all-mod += $(build-srec-image-y)
all-mod += $(build-srec-y)
all-mod += $(build-bin-image-y)

# To srecord again to load in tool
$(build-srec-image-y): $(build-bin-image-y)
	@echo ">> Binary to SREC to load with tool"
	$(SREC_CAT) $< -binary -offset ${BOOT_IMAGE_ADDR} -o $@
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
	$(MKIMAGE) -o $@ -l $(BOOT_BLOB_LOAD_ADDR) -s $(BOOT_BLOB_START_ADDR) -m "APP" -v $<
