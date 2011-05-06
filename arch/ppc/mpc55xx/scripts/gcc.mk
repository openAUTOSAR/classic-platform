
# prefered version
CC_VERSION=4.1.2

# No smalldata
ppc_common-y	+= -msdata=none

gcc_version := $(word 3,$(shell ${CROSS_COMPILE}gcc --version))
gcc_split = $(subst ., ,$(gcc_version))

# If version 4.3 or above then use -te500v1 (CodeSourcery)
#ifeq ($(word 1,$(gcc_split)),4)
#ifneq ($(filter $(word 2,$(gcc_split)),3 4 5 6 7 8 9),)
#ppc_common-$(CFG_SPE)		+= -te500v1 -mhard-float # -mfloat-gprs=single -mspe=yes -mhard-float
#endif
#endif
ifeq ($(ppc_common-y),)
ppc_common-$(CFG_SPE)		+= -mfloat-gprs=single -mspe=yes -mhard-float -mcpu=8540 -mno-eabi
endif
#
ppc_common-y 	+= -mmultiple
ppc_common-$(CFG_MPC5516)	+= -msoft-float -mcpu=8540 -mno-eabi
ppc_common-y 	+= -mstrict-align
ppc_common-y 	+= -gdwarf-2
ppc_common-y   += -D_PPC

cflags-$(CFG_MPC55XX) 	+= -B$(prefix)/libexec/gcc:/opt
#cflags-$(CFG_MPC55XX) 	+= -mcpu=8540 
#cflags-$(CFG_SPE) 	+= -mabi=spe
cflags-$(CFG_MPC55XX)  += -mno-eabi

cflags-$(CFG_MPC55XX) += $(ppc_common-y)

lib-y   	+= -lgcc -lc 
#LDFLAGS += -te500v1

asflags-$(CFG_BOOKE) += -me500
asflags-$(CFG_SPE) += -mspe
asflags-y += -mregnames
 
ASFLAGS += $(asflags-y)


