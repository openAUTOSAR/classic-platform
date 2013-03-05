/* -------------------------------- Arctic Core ------------------------------
 * Arctic Core - the open source AUTOSAR platform http://arccore.com
 *
 * Copyright (C) 2009  ArcCore AB <contact@arccore.com>
 *
 * This source code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by the
 * Free Software Foundation; See <http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
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

#define READ16_NA(address )			( (((uint16_t)(((uint8_t *)address)[0]))<<8) +	\
									  (((uint16_t)(((uint8_t *)address)[1]))) )

#define SET32(  _addr, _val)		(*(vuint32_t*)(_addr) |= (_val))
#define CLEAR32(_addr, _val)		(*(vuint32_t*)(_addr) &= ~(_val))


/* READWRITE macros
 * address - The address to read/write from/to
 * mask    - The value read is inverted and AND:ed with mask
 * val     - The Value to write.
 *
 * READWRITE32(0x120,0x0,0x9) - Read from address 0x120 (contains 0x780), ANDs with
 *                              0xffff_ffff and ORs in 0x9 -> Write 0x789 to 0x120
 *
 */
#define READWRITE32(address,mask,val)  WRITE32(address,(READ32(address)&~(mask))|val)
#define READWRITE8(address,mask,val)   WRITE8(address,(READ8(address)&~(mask))|val)



/* NA - Not Aligned */
#define WRITE32_NA(address, value ) \
	do { \
    	((uint8_t *)address)[0] = ((value>>24)&0xff); \
    	((uint8_t *)address)[1] = ((value>>16)&0xff); \
    	((uint8_t *)address)[2] = ((value>>8)&0xff);  \
    	((uint8_t *)address)[3] = ((value&0xff)); \
	} while(0)

#define WRITE16_NA(address, value ) \
	do { \
    	((uint8_t *)address)[0] = ((value>>8)&0xff); \
    	((uint8_t *)address)[1] = ((value&0xff)); \
	} while(0)



#endif /* IO_H_ */
