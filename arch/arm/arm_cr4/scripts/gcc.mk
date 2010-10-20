
# prefered version
CC_VERSION=4.5.1
# ARMv7, Thumb-2, little endian, soft-float. 
cflags-y 	+= -mthumb -mcpu=cortex-r4f -mbig-endian #-mfix-cortex-m3-ldrd
cflags-y 	+= -ggdb -mbig-endian

lib-y   	+= -lgcc -lc
ASFLAGS 	+=  -mcpu=cortex-r4f -mbig-endian -mthumb

LDFLAGS     += 