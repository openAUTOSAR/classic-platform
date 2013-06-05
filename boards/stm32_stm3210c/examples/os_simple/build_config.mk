
# Version of build system
REQUIRED_BUILD_SYSTEM_VERSION=1.0.0

# Get configuration makefiles
-include ../config/*.mk
-include ../config/$(BOARDDIR)/*.mk

MOD_USE+= DET 

# Project settings

SELECT_CONSOLE = RAMLOG

SELECT_OPT = OPT_DEBUG 

