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


#ifndef E2E_H_
#define E2E_H_

/* @req 4.0.3/E2E0115
 * @req 4.0.3/E2E0110
 * @req 4.0.3/E2E0017 No need to include Rte.h *
 * @req 4.0.3/E2E0037 No configuration of E2E
 * @req 4.0.3/E2E0048 File structure
 * @req 4.0.3/E2E0050 comply with development of safety-related
 */
#include "Crc.h"
#include "Std_Types.h"
#include "Platform_Types.h"
#include "MemMap.h"
#include "Modules.h"


/* @req 4.0.3/E2E0047 */
/* @req 4.0.3/E2E0295 */
/* @req 4.0.3/E2E0011 */
/* @req 4.0.3/E2E0047 */
/* Return values used by the E2E module */
#define E2E_E_INPUTERR_NULL     0x13
#define E2E_E_INPUTERR_WRONG    0x17
#define E2E_E_INTERR            0x19
#define E2E_E_OK                0x00
#define E2E_E_INVALID           0xFF


/* @req E2E0099 */
uint8 E2E_UpdateCounter(uint8 Counter);


/*  General CRC functions */


/* @req 4.0.3/E2E0091 */
/* @req 4.0.3/E2E0092 */


/* The spec is a bit unclear whether the final XOR used in CRC module shall
 * be removed or not. It is concluded that it is different between CRC8 and
 * CRC8H2F.
 *
 * CRC8:
 * The description of E2E0092 clearly states that "The function uses SAE J1850
 * polynomial, but with 0x00 as start value and XOR value." It is reasonable in
 * order to make it compatibility between AR3 and AR4. This means that if making
 * the CRC in multiple calls, there is no need to XOR the after each call.
 *
 * CRC8H2F:
 * For the CRC8H2F this is not the case, it was introduced in AR4 and it is
 * necessary to XOR the result after each call. Clearly described in E2E0096.
 */
uint8 E2E_CRC8u8(uint8 E2E_Data, uint8 E2E_StartValue);
uint8 E2E_CRC8u16(uint16 E2E_Data, uint8 E2E_StartValue);
uint8 E2E_CRC8u32(uint32 E2E_Data, uint8 E2E_StartValue);


/* @req 4.0.3/E2E0094 */
/* @req 4.0.3/E2E0095 */
uint8 E2E_CRC8u8Array(const uint8* E2E_DataPtr, uint32 E2E_ArrayLength, uint8 E2E_StartValue);
uint8 E2E_CRC8u16Array(const uint16* E2E_DataPtr, uint32 E2E_ArrayLength, uint8 E2E_StartValue);
uint8 E2E_CRC8u32Array(const uint32* E2E_DataPtr, uint32 E2E_ArrayLength, uint8 E2E_StartValue);


/* @req 4.0.3/E2E0096 */
/* @req 4.0.3/E2E0276 */
uint8 E2E_CRC8H2Fu8(uint8 E2E_Data, uint8 E2E_StartValue);
uint8 E2E_CRC8H2Fu16(uint16 E2E_Data, uint8 E2E_StartValue);
uint8 E2E_CRC8H2Fu32(uint32 E2E_Data, uint8 E2E_StartValue);


/* @req 4.0.3/E2E0097 */
/* @req 4.0.3/E2E0098 */
uint8 E2E_CRC8H2Fu8Array(const uint8* E2E_DataPtr, uint32 E2E_ArrayLength, uint8 E2E_StartValue);
uint8 E2E_CRC8H2Fu16Array(const uint16* E2E_DataPtr, uint32 E2E_ArrayLength, uint8 E2E_StartValue);
uint8 E2E_CRC8H2Fu32Array(const uint32* E2E_DataPtr, uint32 E2E_ArrayLength, uint8 E2E_StartValue);



/* @req 4.0.3/E2E0032 */
/* @req 4.0.3/E2E0038 */
#define E2E_VENDOR_ID               60
#define E2E_MODULE_ID               MODULE_ID_E2E
#define E2E_AR_MAJOR_VERSION        4
#define E2E_AR_MINOR_VERSION        0
#define E2E_AR_PATCH_VERSION        3

#define E2E_SW_MAJOR_VERSION        1
#define E2E_SW_MINOR_VERSION        0
#define E2E_SW_PATCH_VERSION        0


/* @req 4.0.3/E2E0032 */
/* @req 4.0.3/E2E0033 */
#define E2E_GetVersionInfo(_vi) STD_GET_VERSION_INFO(_vi,E2E)


#endif
