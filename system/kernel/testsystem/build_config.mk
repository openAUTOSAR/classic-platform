
# Figure out the most of the modules to use.

OPTIMAL_USE = RAMLOG
MOD_USE+=KERNEL MCU DET ECUM $(filter $(OPTIMAL_USE),$(MOD_AVAIL))
$(warning $(MOD_USE)) 
#MOD_USE+=KERNEL MCU T32_TERM SIMPLE_PRINTF RAMLOG

