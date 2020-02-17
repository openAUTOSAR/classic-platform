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








#ifndef BYTEORDER_H_
#define BYTEORDER_H_

// NOTE!
// Implements only big endian stuff

//#define BE_TO_CPU_32(x) ((uint8_t*)(x))[0] + (((uint8_t*)(x))[1]<<8) + (((uint8_t*)(x))[2]<<16) + (((uint8_t*)(x))[3]<<24)
//#define BIG_ENDIAN

// a,b,c,d -> d,c,b,a
#define bswap32(x)		(((uint32)(x) >> 24) | (((uint32)(x) >> 8) & 0xff00) | (((uint32)(x) & 0xff00 ) << 8) | (((uint32)(x) & 0xff) << 24) )
#define bswap16(x)		(((uint16)(x) >> 8) | (((uint16)(x) & 0xff) << 8))

#if 0 //defined(BIG_ENDIAN)
#define cpu_to_le32(_x)	bswap32(_x)
#define cpu_to_be32(_x)
#define le32_to_cpu(_x)
#define be32_to_cpu(_x)
#endif


#endif /*BYTEORDER_H_*/
