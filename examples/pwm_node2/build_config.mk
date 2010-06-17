
-include ../config/*.mk
-include ../config/$(BOARDDIR)/*.mk

MOD_USE+=KERNEL MCU ECUM NEWLIB COMMON DET PORT COM CAN CANIF PWM RAMLOG
