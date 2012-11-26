
# prefered version
CC_VERSION=4.4.5
# ARMv7, Thumb-2, little endian, soft-float. 
cflags-y 	+= -mthumb -mcpu=cortex-r4 -mbig-endian
cflags-y 	+= -ggdb

cflags-y += -ffunction-sections

lib-y   	+= -lgcc -lc
ASFLAGS 	+= -mthumb -mcpu=cortex-r4 -mbig-endian

LDFLAGS     += 