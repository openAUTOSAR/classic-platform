
# Version of build system
REQUIRED_BUILD_SYSTEM_VERSION=1.0.0


# Get configuration makefiles
-include ../config/*.mk
-include ../config/$(BOARDDIR)/*.mk


# Settings
MOD_USE+=DET ECUM MCU KERNEL RAMLOG 

SELECT_CONSOLE = RAMLOG
SELECT_OS_CONSOLE = RAMLOG

def-y += CFG_RAMLOG_SIZE=1024
