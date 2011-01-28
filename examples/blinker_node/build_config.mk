-include ../config/*.mk
-include ../config/$(BOARDDIR)/*.mk

MOD_USE+=ECUM KERNEL MCU PORT DIO DET

SELECT_CONSOLE = RAMLOG
SELECT_OS_CONSOLE = RAMLOG
