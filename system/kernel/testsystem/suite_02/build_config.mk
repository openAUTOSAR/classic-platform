
# Figure out the most of the modules to use.
OPTIMAL_USE = SIMPLE_PRINTF RAMLOG
MOD_USE+=KERNEL MCU COMMON NEWLIB DET ECUM $(filter $(OPTIMAL_USE),$(MOD_AVAIL))

