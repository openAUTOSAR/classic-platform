-include ../config/*.mk
-include ../config/$(BOARDDIR)/*.mk

MOD_USE+=ADC DET ECUM DMA KERNEL MCU 

SELECT_CONSOLE = RAMLOG
SELECT_OS_CONSOLE = RAMLOG