# Supported version MC9S12DG128?

# prefered version
CC_VERSION=3.3.6
 
cflags-y 	+= -m68hcs12
# No local crap
cflags-y 	+= -mlong-calls
# huh, get lots lots of warnings without this...
#cflags-y 	+= -mshort

lib-y   	+= -lgcc -lc
ASFLAGS 	+= 


