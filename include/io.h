/*-------------------------------- Arctic Core ------------------------------
 * Copyright (C) 2013, ArcCore AB, Sweden, www.arccore.com.
 * Contact: <contact@arccore.com>
 * 
 * You may ONLY use this file:
 * 1)if you have a valid commercial ArcCore license and then in accordance with  
 * the terms contained in the written license agreement between you and ArcCore, 
 * or alternatively
 * 2)if you follow the terms found in GNU General Public License version 2 as 
 * published by the Free Software Foundation and appearing in the file 
 * LICENSE.GPL included in the packaging of this file or here 
 * <http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt>
 *-------------------------------- Arctic Core -----------------------------*/

#ifndef IO_H_
#define IO_H_

#include "Arc_Types.h"

#define WRITE8(address, value)      (*(uint8_t*)(address) = (value))
#define READ8(address)              ((uint8)(*(uint8_t*)(address)))

#define WRITE16(address, value)     (*(vuint16_t*)(address) = (value))
#define READ16(address)             ((uint16)(*(vuint16_t*)(address)))

#define WRITE32(address, value)     (*(vuint32_t*)(address) = (value))
#define READ32(address)             ((uint32)(*(vuint32_t*)(address)))

#define WRITE64(address, value)     (*(vuint64_t*)(address) = (value))
#define READ64(address)             ((uint64)(*(vuint64_t*)(address)))

/* Not aligned reads */
#define READ32_NA(address )			(uint32)( (((uint32)((address)[0]))<<24u) +	\
									(((uint32)((address)[1]))<<16u) +	\
									(((uint32)((address)[2]))<<8u) +	\
									((uint32)((address)[3])) )

#define READ16_NA(address )			(uint16)( (((uint16)((address)[0]))<<8u) +	\
									  (((uint16)((address)[1]))) )

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
static inline void READWRITE32(uint32_t address, uint32_t mask, uint32_t val ) {
    WRITE32(address,(READ32(address)&~(mask))|val);
}

#define READWRITE8(address,mask,val)   WRITE8(address,(READ8(address)&~(mask))|val)




/* NA - Not Aligned */
#define WRITE32_NA(address, value ) \
    	(address)[0] = ((value>>24u)&0xffu); \
    	(address)[1] = ((value>>16u)&0xffu); \
    	(address)[2] = ((value>>8u)&0xffu);  \
    	(address)[3] = ((value&0xffu)); \


#define WRITE16_NA(address, value ) \
    	(address)[0] = ((value>>8u)&0xffu); \
    	(address)[1] = ((value&0xffu)); \


#endif /* IO_H_ */
