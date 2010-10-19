
-include ../config/*.mk
-include ../config/$(BOARDDIR)/*.mk

MOD_USE+=DET ECUM MCU KERNEL RAMLOG 

SELECT_CONSOLE = RAMLOG
SELECT_OS_CONSOLE = RAMLOG

def-y += CFG_RAMLOG_SIZE=1024
