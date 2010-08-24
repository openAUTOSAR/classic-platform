-include ../config/*.mk
-include ../config/$(BOARDDIR)/*.mk

-include ../config/*.mk
-include ../config/$(BOARDDIR)/*.mk

MOD_USE+=COMMON DET ECUM MCU NEWLIB KERNEL RAMLOG 

SELECT_CONSOLE = RAMLOG
SELECT_OS_CONSOLE = RAMLOG

def-y += CFG_RAMLOG_SIZE=1024
#def-y += HEAPSIZE=400
