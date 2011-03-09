
# Version of build system
REQUIRED_BUILD_SYSTEM_VERSION=1.0.0


# Get configuration makefiles
-include ../config/*.mk
-include ../config/$(BOARDDIR)/*.mk


# Settings
MOD_USE+=ADC DET ECUM DMA KERNEL MCU 

SELECT_CONSOLE = RAMLOG
SELECT_OS_CONSOLE = RAMLOG