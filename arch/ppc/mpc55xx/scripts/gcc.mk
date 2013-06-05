
# prefered version
CC_VERSION=4.1.2

# No smalldata
ppc_common-y	+= -msdata=none

gcc_version := $(word 3,$(shell ${CROSS_COMPILE}gcc --version))
gcc_split = $(subst ., ,$(gcc_version))

# If version 4.3 or above then use -te500v1 (CodeSourcery)
#ifeq ($(word 1,$(gcc_split)),4)
#ifneq ($(filter $(word 2,$(gcc_split)),3 4 5 6 7 8 9),)
#ppc_common-$(CFG_SPE)		+= -te500v1 -mhard-float
#endif
#endif

ifeq ($(CFG_SPE),y)
ppc_common-y += -mfloat-gprs=single -mspe=yes -mhard-float 
else
ppc_common-y += -msoft-float
endif

ppc_common-y    += -mcpu=8540
ppc_common-y 	+= -mmultiple
ppc_common-y 	+= -mstrict-align
ppc_common-y 	+= -gdwarf-2
ppc_common-y   += -D_PPC

cflags-$(CFG_MPC55XX) 	+= -B$(prefix)/libexec/gcc:/opt
cflags-$(CFG_MPC55XX) += $(ppc_common-y)

cflags-y += -ffunction-sections

lib-y   	+= -lgcc -lc 
#LDFLAGS += -te500v1

asflags-$(CFG_BOOKE) += -me500
asflags-$(CFG_SPE) += -mspe -me500
asflags-y += -mregnames
 
ASFLAGS += $(asflags-y)


