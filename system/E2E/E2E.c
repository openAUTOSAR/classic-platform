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

/* @req 4.0.3/E2E0115 */
/* @req 4.0.3/E2E0112 */
/* @req 4.0.3/E2E0216 */

#define E2E_AR_MAJOR_VERSION_INT        4
#define E2E_AR_MINOR_VERSION_INT        0
#define E2E_AR_PATCH_VERSION_INT        3

#define E2E_SW_MAJOR_VERSION_INT        1
#define E2E_SW_MINOR_VERSION_INT        0

#include "E2E.h"

/* @req 4.0.3/E2E0287 */
#if E2E_SW_MAJOR_VERSION != E2E_SW_MAJOR_VERSION_INT || E2E_SW_MINOR_VERSION != E2E_SW_MINOR_VERSION_INT
#error "SW Version mismatch between E2E.c and E2E.h"
#endif

#if E2E_AR_MAJOR_VERSION != E2E_AR_MAJOR_VERSION_INT || E2E_AR_MINOR_VERSION != E2E_AR_MINOR_VERSION_INT || \
    E2E_AR_PATCH_VERSION != E2E_AR_PATCH_VERSION_INT
#error "AR Version mismatch between E2E.c and E2E.h"
#endif



/* @req 4.0.3/E2E0099 */
/* @req 4.0.3/E2E0075 */
uint8 E2E_UpdateCounter(uint8 Counter) {
    return (Counter+1) % 15; /* Use the Profile 1 value */
}


/* @req 4.0.3/E2E0091 */
/* @req 4.0.3/E2E0092 */
uint8 E2E_CRC8u8(uint8 E2E_Data, uint8 E2E_StartValue) {
    return Crc_CalculateCRC8(&E2E_Data, sizeof(E2E_Data), E2E_StartValue, FALSE) ^ CRC8_XOR;
}


/* @req 4.0.3/E2E0091 */
/* @req 4.0.3/E2E0092 */
uint8 E2E_CRC8u16(uint16 E2E_Data, uint8 E2E_StartValue) {
    /* Support for both little and big endian */
    uint8 data[2] = { (uint8)(E2E_Data & 0xFF), (uint8)(E2E_Data >> 8)};
    return Crc_CalculateCRC8(data, sizeof(E2E_Data), E2E_StartValue, FALSE) ^ CRC8_XOR;
}

/* @req 4.0.3/E2E0091 */
/* @req 4.0.3/E2E0092 */
uint8 E2E_CRC8u32(uint32 E2E_Data, uint8 E2E_StartValue) {
    /* Support for both little and big endian */
    uint8 data[4] = {(uint8)(E2E_Data & 0xFF), (uint8)((E2E_Data >> 8) & 0xFF),
                     (uint8)((E2E_Data >> 16) & 0xFF), (uint8)(E2E_Data >> 24)};
    return Crc_CalculateCRC8(data, sizeof(E2E_Data), E2E_StartValue, FALSE) ^ CRC8_XOR;
}

/* @req 4.0.3/E2E0094 */
/* @req 4.0.3/E2E0095 */
uint8 E2E_CRC8u8Array(const uint8* E2E_DataPtr, uint32 E2E_ArrayLength, uint8 E2E_StartValue) {
    return Crc_CalculateCRC8(E2E_DataPtr, E2E_ArrayLength, E2E_StartValue, FALSE) ^ CRC8_XOR;
}

/* @req 4.0.3/E2E0094 */
/* @req 4.0.3/E2E0095 */
uint8 E2E_CRC8u16Array(const uint16* E2E_DataPtr, uint32 E2E_ArrayLength, uint8 E2E_StartValue) {

    /* Supports for both little and big endian by making in endianess idependant */

    uint8 crc = E2E_StartValue;

    for (uint32 i=0;i<E2E_ArrayLength;i++) {
        crc = E2E_CRC8u16(*E2E_DataPtr, crc);
        E2E_DataPtr++;
    }

    return crc;

}

/* @req 4.0.3/E2E0094 */
/* @req 4.0.3/E2E0095 */
uint8 E2E_CRC8u32Array(const uint32* E2E_DataPtr, uint32 E2E_ArrayLength, uint8 E2E_StartValue) {

    /* Supports for both little and big endian by making in endianess idependant */

    uint8 crc = E2E_StartValue;

    for (uint32 i=0;i<E2E_ArrayLength;i++) {
        crc = E2E_CRC8u32(*E2E_DataPtr, crc);
        E2E_DataPtr++;
    }

    return crc;
}

/* @req 4.0.3/E2E0096 */
/* @req 4.0.3/E2E0276 */
uint8 E2E_CRC8H2Fu8(uint8 E2E_Data, uint8 E2E_StartValue) {
    return Crc_CalculateCRC8H2F(&E2E_Data, sizeof(E2E_Data), E2E_StartValue, FALSE);
}

/* @req 4.0.3/E2E0096 */
/* @req 4.0.3/E2E0276 */
uint8 E2E_CRC8H2Fu16(uint16 E2E_Data, uint8 E2E_StartValue) {
    /* Support for both little and big endian */
    uint8 data[2] = { (uint8)(E2E_Data & 0xFF), (uint8)(E2E_Data >> 8)};
    return Crc_CalculateCRC8H2F(data, sizeof(E2E_Data), E2E_StartValue, FALSE);
}

/* @req 4.0.3/E2E0096 */
/* @req 4.0.3/E2E0276 */
uint8 E2E_CRC8H2Fu32(uint32 E2E_Data, uint8 E2E_StartValue) {
    /* Support for both little and big endian */
    uint8 data[4] = {(uint8)(E2E_Data & 0xFF), (uint8)((E2E_Data >> 8) & 0xFF),
                     (uint8)((E2E_Data >> 16) & 0xFF), (uint8)(E2E_Data >> 24)};
    return Crc_CalculateCRC8H2F(data, sizeof(E2E_Data), E2E_StartValue, FALSE);
}


/* @req 4.0.3/E2E0097 */
/* @req 4.0.3/E2E0098 */
uint8 E2E_CRC8H2Fu8Array(const uint8* E2E_DataPtr, uint32 E2E_ArrayLength, uint8 E2E_StartValue) {
    return Crc_CalculateCRC8H2F(E2E_DataPtr, E2E_ArrayLength, E2E_StartValue, FALSE);
}


/* @req 4.0.3/E2E0097 */
/* @req 4.0.3/E2E0098 */
uint8 E2E_CRC8H2Fu16Array(const uint16* E2E_DataPtr, uint32 E2E_ArrayLength, uint8 E2E_StartValue) {

    /* Supports for both little and big endian by making in endianess idependant */

    uint8 crc = E2E_StartValue ^ CRC8_H2F_XOR; /* Needed cancel the XOR in loop */

    for (uint32 i=0;i<E2E_ArrayLength;i++) {
        crc = E2E_CRC8H2Fu16(*E2E_DataPtr, crc ^ CRC8_H2F_XOR);
        E2E_DataPtr++;
    }

    return crc;

}


/* @req 4.0.3/E2E0097 */
/* @req 4.0.3/E2E0098 */
uint8 E2E_CRC8H2Fu32Array(const uint32* E2E_DataPtr, uint32 E2E_ArrayLength, uint8 E2E_StartValue) {

    /* Supports for both little and big endian by making in endianess idependant */

    uint8 crc = E2E_StartValue ^ CRC8_H2F_XOR;

    for (uint32 i=0;i<E2E_ArrayLength;i++) {
        crc = E2E_CRC8H2Fu32(*E2E_DataPtr, crc ^ CRC8_H2F_XOR);
        E2E_DataPtr++;
    }

    return crc;

}

