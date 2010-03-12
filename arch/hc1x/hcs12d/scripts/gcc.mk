# Supported version MC9S12DG128?

# prefered version
CC_VERSION=3.3.6


cflags-y += -m68hcs12 -mshort -mlong-calls -gdwarf-2 -O0 -Wall -Wno-char-subscripts -msoft-reg-count=0 -mrelax
cflags-y 	+= -B/opt/m6811-elf/m6811-elf/

LDFLAGS += -mm68hc12elfb --relax

lib-y   	+= -lgcc -lc
ASFLAGS 	+= -mshort -m68hcs12

