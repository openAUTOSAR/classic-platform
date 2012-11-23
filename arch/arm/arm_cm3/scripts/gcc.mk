
# prefered version
CC_VERSION=4.1.2
# ARMv7, Thumb-2, little endian, soft-float. 
cflags-y 	+= -mthumb -mcpu=cortex-m3 -mfix-cortex-m3-ldrd
cflags-y 	+= -ggdb

cflags-y += -ffunction-sections

lib-y   	+= -lgcc -lc
ASFLAGS 	+= -mcpu=cortex-m3 -mthumb


