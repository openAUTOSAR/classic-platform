
# ARCH defines
ARCH=hcs12d
ARCH_FAM=hc1x
ARCH_MCU=MC912DG128A

# CFG (y/n) macros (These become CFG_XXX=y and def-y += CFG_XXX ) 
CFG=HC1X HCS12D MC912DG128A BRD_HCS12_ELMICRO_CARD12 SIMULATOR

# What buildable modules does this board have, 
# default or private (These become MOD_XXX=y )
MOD_AVAIL=KERNEL MCU GPT

# Needed by us (These become USE_XXX=y and def-y += USE_XXX )
MOD_USE=KERNEL MCU

