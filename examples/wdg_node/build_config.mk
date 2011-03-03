-include ../config/*.mk
-include ../config/$(BOARDDIR)/*.mk

MOD_USE += WDG WDGM PORT DIO MCU KERNEL ECUM DET GPT

SELECT_CONSOLE = RAMLOG

SELECT_OPT = OPT_DEBUG 

