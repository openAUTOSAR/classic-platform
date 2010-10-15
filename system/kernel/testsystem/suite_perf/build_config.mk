
# Figure out the most of the modules to use.
OPTIMAL_USE = SIMPLE_PRINTF RAMLOG
MOD_USE+=KERNEL MCU  DET ECUM $(filter $(OPTIMAL_USE),$(MOD_AVAIL))
$(warning $(MOD_USE)) 
#MOD_USE+=KERNEL MCU SIMPLE_PRINTF RAMLOG

