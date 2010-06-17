
-include ../config/*.mk
-include ../config/$(BOARDDIR)/*.mk

MOD_USE+=KERNEL MCU ECUM NEWLIB COMMON DET RAMLOG RAMLOG

#def-y += HEAPSIZE=1400