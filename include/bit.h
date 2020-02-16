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


/*
 * Bit manipulation functions, NOT tested..
 */

#ifndef BIT_H_
#define BIT_H_

#include <stdint.h>
/**
 * @param aPtr 	Ptr to an array of unsigned chars.
 * @param num	The bit number to get.
 * @return
 */
static inline int Bit_Get(uint8_t *aPtr, int num ) {
	return (aPtr[num / 8] >> (num % 8)) & 1;
}

/**
 *
 * @param aPtr
 * @param num
 * @return
 */
static inline void Bit_Set(uint8_t *aPtr, int num ) {
	aPtr[num / 8] |=  (1<<(num % 8));
}

/**
 *
 * @param aPtr
 * @param num
 * @return
 */
static inline void Bit_Clear(uint8_t *aPtr, int num ) {
	aPtr[num / 8] &=  ~(1<<(num % 8));
}


#endif /* BIT_H_ */
