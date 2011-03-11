
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
