

cflags-$(OS_32BIT)+= -m32
LDFLAGS-$(OS_32BIT)+= -m32

# Nvidia recommended compiler flags for Linux userspace
ifneq ($(CROSS_COMPILE),)
# DEBUG
cflags-y += -march=armv8-a -Wcast-align
# RELEASE
# -O2 -fomit-frame-pointer -finline-functions -finline-limit=300 fgcse-after-reload
endif


# Code coverage 
#cflags-y += -fprofile-arcs -ftest-coverage
#LDFLAGS += -lgcov --coverage
#
ifneq ($(OS),Windows_NT)
	LDFLAGS +=      -pthread
endif
inc-y += $(ROOTDIR)/system/Os/include
# SimonG or $(ROOTDIR)/boards/linux/Os.h
