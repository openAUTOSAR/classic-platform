
# Version of build system
REQUIRED_BUILD_SYSTEM_VERSION=1.0.0


# Get configuration makefiles
-include ../config/*.mk
-include ../config/$(BOARDDIR)/*.mk


# Settings
MOD_USE+=KERNEL MCU ECUM DET PORT COM CAN CANIF PWM RAMLOG
