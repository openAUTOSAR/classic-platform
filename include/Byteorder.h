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
