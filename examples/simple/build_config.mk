
-include ../config/*.mk
-include ../config/$(BOARDDIR)/*.mk

MOD_USE+= COMMON DET ECUM MCU NEWLIB KERNEL RAMLOG 

#def-y += NDEBUG
#def-y += HEAPSIZE=1400