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

/** @addtogroup General General
 *  @{ */

/** @file Platform_Types.h
 * General platform type definitions.
 */

#include <stdbool.h>

#ifndef PLATFORM_TYPES_H
#define PLATFORM_TYPES_H

#define CPU_TYPE            CPU_TYPE_32 
#define CPU_BIT_ORDER       MSB_FIRST 
#define CPU_BYTE_ORDER      HIGH_BYTE_FIRST

#ifndef FALSE
#define FALSE		(boolean)false
#endif
#ifndef TRUE
#define TRUE		(boolean)true
#endif

//typedef unsigned long       boolean;
typedef _Bool      boolean;
typedef signed char         sint8;        
typedef unsigned char       uint8;
typedef char				char_t;
typedef signed short        sint16;       
typedef unsigned short      uint16;       
typedef signed long         sint32;       
typedef unsigned long       uint32;
typedef unsigned long long  uint64;
typedef unsigned long       uint8_least;  
typedef unsigned long       uint16_least; 
typedef unsigned long       uint32_least; 
typedef signed long         sint8_least;  
typedef signed long         sint16_least; 
typedef signed long         sint32_least; 
typedef float               float32; 
typedef double              float64;  

#endif
/** @} */
