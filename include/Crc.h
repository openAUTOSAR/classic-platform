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


#ifndef CRC_H_
#define CRC_H_

/* @req 4.0.3/CRC018 */
#include "Std_Types.h"
#include "Modules.h"

/* This requirement is also applicable to users of the interface otherwise
 * set by the interface
 * @req 4.0.3/CRC023
 */

/* @req 3.1.5|4.0.3/CRC022 */
/* @req 4.0.3/CRC023 */
#include "Crc_Cfg.h"
#include "Modules.h"

#define CRC_VENDOR_ID			    VENDOR_ID_ARCCORE
/* @req 4.0.3/CRC050 */
#define CRC_MODULE_ID               MODULE_ID_CRC
#define CRC_AR_MAJOR_VERSION        4
#define CRC_AR_MINOR_VERSION        0
#define CRC_AR_PATCH_VERSION        3

#define CRC_SW_MAJOR_VERSION        1
#define CRC_SW_MINOR_VERSION        0
#define CRC_SW_PATCH_VERSION        0


/* May be needed by CRC lib users since in order to create custom CRCs e.g.
 * used in the E2E lib
 */

/* For CRC 8*/
#define CRC8_START_VALUE        0xFFU
#define CRC8_XOR                0xFFU

/* For CRC8 H2F */
#define CRC8_H2F_START_VALUE    0xFFU
#define CRC8_H2F_XOR            0xFFU



/* These two applies to the caller and it set by the interface  */
/* @req 4.0.3/CRC014 */
/* @req 4.0.3/CRC041 */


/* @req 4.0.3/CRC031 */
uint8  Crc_CalculateCRC8(const uint8* Crc_DataPtr, uint32 Crc_Length, uint8 Crc_StartValue8, boolean Crc_IsFirstCall);

/* @req 4.0.3/CRC042 */
uint8  Crc_CalculateCRC8H2F(const uint8* Crc_DataPtr, uint32 Crc_Length, uint8 Crc_StartValue8, boolean Crc_IsFirstCall);

/* @req 4.0.3/CRC019 */
uint16 Crc_CalculateCRC16(const uint8* Crc_DataPtr, uint32 Crc_Length, uint16 Crc_StartValue16, boolean Crc_IsFirstCall);

/* @req 4.0.3/CRC020 */
uint32 Crc_CalculateCRC32( const uint8* Crc_DataPtr, uint32 Crc_Length, uint32 Crc_StartValue32, boolean Crc_IsFirstCall);


/* @req 4.0.3/CRC011 */
/* @req 4.0.3/CRC021 */
/* @req 4.0.3/CRC017 */
#if ( CRC_VERSION_INFO_API == STD_ON )
#define Crc_GetVersionInfo(_vi) STD_GET_VERSION_INFO(_vi,CRC)
#endif



#endif /* CRC_H_ */
