-include ../config/*.mk
-include ../config/$(BOARDDIR)/*.mk

MOD_USE+=KERNEL MCU PORT PWM

SELECT_CONSOLE = RAMLOG
SELECT_OS_CONSOLE = RAMLOG
