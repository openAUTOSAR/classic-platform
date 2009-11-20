
# Figure out the most of the modules to use.
OPTIMAL_USE = T32_TERM SIMPLE_PRINTF RAMLOG
MOD_USE+=KERNEL MCU $(filter $(OPTIMAL_USE),$(MOD_AVAIL))
$(warning $(MOD_USE)) 
#MOD_USE+=KERNEL MCU T32_TERM SIMPLE_PRINTF RAMLOG

