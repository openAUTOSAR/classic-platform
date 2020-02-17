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

/** @file Platform_Types.h
 * General platform type definitions.
 */

#ifndef PLATFORM_TYPES_H
#define PLATFORM_TYPES_H

#include <stdbool.h>
#include <stdint.h>


#define CPU_TYPE            CPU_TYPE_32 
#define CPU_BIT_ORDER       MSB_FIRST

#define HIGH_BYTE_FIRST     0U
#define LOW_BYTE_FIRST      1U

#if defined(__GNUC__)

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__ /*lint !e553 handled by #error */
#define CPU_BYTE_ORDER LOW_BYTE_FIRST                                     /* Autosar Little Endian */
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__ /*lint !e553 handled by #error */
#define CPU_BYTE_ORDER HIGH_BYTE_FIRST                                    /* Autosar Big Endian */
#else
#error No endian defined by compiler.
#endif

#elif defined(__ghs__)

#if defined(__LITTLE_ENDIAN__)
#define CPU_BYTE_ORDER LOW_BYTE_FIRST                                     /* Autosar Little Endian */
#elif defined(__BIG_ENDIAN__)
#define CPU_BYTE_ORDER HIGH_BYTE_FIRST                                    /* Autosar Big Endian */
#else
#error No endian defined by compiler.
#endif

#elif defined(__CWCC__)

#if __option(little_endian)
#define CPU_BYTE_ORDER LOW_BYTE_FIRST                                     /* Autosar Little Endian */
#else
#define CPU_BYTE_ORDER HIGH_BYTE_FIRST                                    /* Autosar Big Endian */
#endif

/* Compiler does not have a pre-defined macro,  manually set __LITTLE_ENDIAN__ or  __BIG_ENDIAN__*/
#elif defined(__DCC__)

#if defined(__LITTLE_ENDIAN__)
#define CPU_BYTE_ORDER LOW_BYTE_FIRST                                     /* Autosar Little Endian */
#elif defined(__BIG_ENDIAN__)
#define CPU_BYTE_ORDER HIGH_BYTE_FIRST                                    /* Autosar Big Endian */
#else
#error No endian defined by compiler.
#endif

#elif defined(__ARMCC_VERSION)

#if defined(__BIG_ENDIAN)
#define CPU_BYTE_ORDER HIGH_BYTE_FIRST                                    /* Autosar Big Endian */
#else
#define CPU_BYTE_ORDER LOW_BYTE_FIRST                                     /* Autosar Little Endian */
#endif

#elif defined(_WIN32)
#define CPU_BYTE_ORDER LOW_BYTE_FIRST                                     /* Autosar Little Endian */

#elif defined(__IAR_SYSTEMS_ICC__)

#if defined(__LITTLE_ENDIAN__)
#define CPU_BYTE_ORDER LOW_BYTE_FIRST                                     /* Autosar Little Endian */
#elif defined(__BIG_ENDIAN__)
#define CPU_BYTE_ORDER HIGH_BYTE_FIRST                                    /* Autosar Big Endian */
#else
#error No endian defined by compiler.
#endif

#else
#error Compiler not defined
#endif


#ifndef FALSE
#define FALSE		0 /*@req SWS_Platform_00026*/
#endif
#ifndef TRUE
#define TRUE		1 /*@req SWS_Platform_00026*/
#endif

/*lint -save -e138   Prevent lint error message if type definitions below cause any loops.
 * Relationships may be explicitly created by lint elsewhere, to allow
 * conversions between different types derived from e.g. uintxx_t and uintxx
 * and others as defined below.
 */
typedef uint8_t      		boolean; /*@req SWS_Platform_00027*/
typedef int8_t         		sint8;
typedef uint8_t       		uint8;
typedef uint8_t				utf8;
typedef char				char_t;
typedef int16_t        		sint16;
typedef uint16_t      		uint16;
typedef uint16_t			ucs2;
typedef int32_t         	sint32;
typedef uint32_t       		uint32;
typedef int64_t  			sint64;
typedef uint64_t  			uint64;
typedef uint_least8_t       uint8_least;
typedef uint_least16_t      uint16_least;
typedef uint_least32_t      uint32_least;
typedef int_least8_t        sint8_least;
typedef int_least16_t       sint16_least;
typedef int_least32_t       sint32_least;
typedef float               float32; 
typedef double              float64;  
/*lint -restore */


#endif
