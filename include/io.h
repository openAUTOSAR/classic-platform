/* -------------------------------- Arctic Core ------------------------------
 * Copyright (C) 2009-2011 ArcCore AB <contact@arccore.com>
 * Licensed under ArcCore Embedded Software License Agreement.
 * -------------------------------- Arctic Core ------------------------------*/

#ifndef IO_H_
#define IO_H_

#define WRITE8(address, value)      (*(uint8_t*)(address) = (value))
#define READ8(address)              ((uint8_t)(*(uint8_t*)(address)))

#define WRITE16(address, value)     (*(vuint16_t*)(address) = (value))
#define READ16(address)             ((uint16_t)(*(vuint16_t*)(address)))

#define WRITE32(address, value)     (*(vuint32_t*)(address) = (value))
#define READ32(address)             ((uint32_t)(*(vuint32_t*)(address)))

#define WRITE64(address, value)     (*(vuint64_t*)(address) = (value))
#define READ64(address)             ((uint64_t)(*(vuint64_t*)(address)))

/* Not aligned reads */
#define READ32_NA(address )			( (((uint32_t)(((uint8_t *)address)[0]))<<24) +	\
									  (((uint32_t)(((uint8_t *)address)[1]))<<16) +	\
									  (((uint32_t)(((uint8_t *)address)[2]))<<8) +	\
									  ((uint32_t)(((uint8_t *)address)[3])) )

#define READ16_NA(address )			( (((uint32_t)(((uint8_t *)address)[0]))<<8) +	\
									  (((uint32_t)(((uint8_t *)address)[1]))) )

#define WRITE32_NA(address, value ) \
	do { \
    	((uint8_t *)address)[0] = ((value&0xff)<<24); \
    	((uint8_t *)address)[1] = ((value&0xff)<<16); \
    	((uint8_t *)address)[2] = ((value&0xff)<<8);  \
    	((uint8_t *)address)[3] = ((value&0xff)); \
	} while(0)

#define WRITE16_NA(address, value ) \
	do { \
    	((uint8_t *)address)[0] = ((value&0xff)<<8); \
    	((uint8_t *)address)[1] = ((value&0xff)); \
	} while(0)



#endif /* IO_H_ */
