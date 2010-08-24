-include ../config/*.mk
-include ../config/$(BOARDDIR)/*.mk

MOD_USE+=ECUM KERNEL MCU WDG WDGM PORT DIO 

SELECT_CONSOLE = RAMLOG
SELECT_OS_CONSOLE = RAMLOG
