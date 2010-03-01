# Supported version MC9S12DG128?

# prefered version
CC_VERSION=3.3.6


cflags-y += -m68hcs12 -mshort -mlong-calls -gdwarf-2 -O0 -Wall -Wmissing-prototypes -Wno-char-subscripts -msoft-reg-count=0 -mrelax
cflags-y 	+= -B/opt/m6812-elf-2/m6812-elf/

LDFLAGS += -mm68hc12elfb 

lib-y   	+= -lgcc -lc
ASFLAGS 	+= -mshort

