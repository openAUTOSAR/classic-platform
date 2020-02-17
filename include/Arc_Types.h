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

#ifndef ARC_TYPES_H_
#define ARC_TYPES_H_

#include <stdint.h>

typedef volatile int8_t vint8_t;
typedef volatile uint8_t vuint8_t;

typedef volatile int16_t vint16_t;
typedef volatile uint16_t vuint16_t;

typedef volatile int32_t vint32_t;
typedef volatile uint32_t vuint32_t;

typedef volatile int64_t vint64_t;
typedef volatile uint64_t vuint64_t;

/* Need to set the type hierarchy since it is overwritten by the typedefs above */
//lint -parent(uint64, uint32)
//lint -parent(uint64, vuint64_t)
//lint -parent(uint32, uint16)
//lint -parent(uint32, uint8)
//lint -parent(uint32, uint32_t)
//lint -parent(uint32, vuint32_t)
//lint -parent(uint16, uint8)
//lint -parent(uint16, vuint16_t,)
//lint -parent(uint16, uint16_t)
//lint -parent(uint8, vuint8_t)
//lint -parent(uint8, uint8_t)
//lint -parent(sint32, sint64)
//lint -parent(sint16, sint32)
//lint -parent(sint8, sint16)


#endif /* ARC_TYPES_H_ */

