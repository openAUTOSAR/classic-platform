
-include ../config/*.mk
-include ../config/$(BOARDDIR)/*.mk

MOD_USE+= DET ECUM MCU KERNEL RAMLOG 

#def-y += NDEBUG
#def-y += HEAPSIZE=1400