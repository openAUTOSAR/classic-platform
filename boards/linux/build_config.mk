
# ARCH defines
ARCH=linux
ARCH_FAM=generic
#ARCH=mpc55xx
#ARCH_FAM=ppc
#ARCH_MCU=mpc5516

# CFG (y/n) macros
CFG= BRD_LINUX

# What buildable modules does this board have, 
# default or private
MOD_AVAIL+=COM PDUR IPDUM WDGM WDGIF DET DCM DEM CANTP J1939TP CANIF RTE FEE NVM ECUM_FIXED ECUM_FLEXIBLE MCU EA KERNEL RAMLOG IOHWAB RTE E2E BSWM
# Network management
MOD_AVAIL+=COMM NM CANNM CANSM NMHB UDPNM
# Additional
MOD_AVAIL+=CRC
MOD_AVAIL+=LIN
MOD_AVAIL+=LINSM
MOD_AVAIL+=LINIF

# Required modules
#MOD_USE += 

# Needed by us
MOD_USE=
SELECT_CLIB=CLIB_NATIVE