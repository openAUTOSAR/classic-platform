
# Version of build system
REQUIRED_BUILD_SYSTEM_VERSION=1.0.0

# Get configuration makefiles
-include ../config/EcuM_Lin.mk
-include ../config/$(BOARDDIR)/*.mk


# Project settings

SELECT_CONSOLE = RAMLOG

SELECT_OPT = OPT_DEBUG 

