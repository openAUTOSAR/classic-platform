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


#ifndef IOHWAB_INTERNAL_H_
#define IOHWAB_INTERNAL_H_

#include "Cpu.h"
#if defined(USE_DEM)
#include "Dem.h"
#include "Rte_Dem_Type.h"
#endif

#if defined(USE_DET)
#include "Det.h"
#endif

/**
 * Takes a ptr to an array of big endian data and returns a uint32
 * @param src
 * @return converted data
 */
static inline uint32 GetU32FromPtr(uint8* src) {
    uint32 value = *(src) << 24u;
    value = (*(src + 1) << 16u) | value;
    value = (*(src + 2) << 8u) | value;
    value = (*(src + 3)) | value;
    return value;
}

/**
 * Takes a value and copies that value to an array of big endian data
 * @param value
 * @param dst
 */
static inline void SetU32ToPtr(uint32 value, uint8* dst) {
    *(dst) = (value & 0xFF000000) >> 24u ;
    *(dst + 1) = (value & 0x00FF0000) >> 16u ;
    *(dst + 2) = (value & 0x0000FF00) >> 8u ;
    *(dst + 3) = (value & 0x000000FF);
}


/**
 * Takes a ptr to an array of big endian data and returns a sint32
 * @param src
 * @return converted data
 */
static inline sint32 GetS32FromPtr(uint8* src) {
    sint32 value = *(src) << 24u;
    value = (*(src + 1) << 16u) | value;
    value = (*(src + 2) << 8u) | value;
    value = (*(src + 3)) | value;
    return value;
}

/**
 * Takes a value and copies that value to an array of big endian data
 * @param value
 * @param dst
 */
static inline void SetS32ToPtr(sint32 value, uint8* dst) {
    *(dst) = (value & 0xFF000000) >> 24 ;
    *(dst + 1) = (value & 0x00FF0000) >> 16 ;
    *(dst + 2) = (value & 0x0000FF00) >> 8 ;
    *(dst + 3) = (value & 0x000000FF);
}




#endif /* IOHWAB_INTERNAL_H_ */
