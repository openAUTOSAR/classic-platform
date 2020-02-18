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

/** @reqSettings DEFAULT_SPECIFICATION_REVISION=4.3.0 */

/** @file Crc.h
 *
 *  The Crc Library provides functions for 8 bit, 16 bit and 32 bit CRC (cyclic redundancy check) calculations.
 *  The Crc library can be scaled in terms of
 *      * Table based calculation (fast, but higher code size)
 *      * Runtime calculation (slow, but smaller code size)
 *      * Different standard CRC generator polynomials
 */

#ifndef CRC_H_
#define CRC_H_

/* @req SWS_CRC_00048
 * Additional module-specific published parameters are listed below
 *
 * @req SWS_CRC_00050
 * The standardized common published parameters
 * shall be published within the header file of this module.
 *
 * @req SWS_BSW_00059 The Published information of the BSW Module shall be provided within 
 * all header files by defining preprocessor directives.
 */
#define CRC_VENDOR_ID               60u
#define CRC_MODULE_ID               201u

#define CRC_AR_RELEASE_MAJOR_VERSION        4u
#define CRC_AR_RELEASE_MINOR_VERSION        3u
#define CRC_AR_RELEASE_REVISION_VERSION     0u

#define CRC_SW_MAJOR_VERSION    2u
#define CRC_SW_MINOR_VERSION    0u
#define CRC_SW_PATCH_VERSION    0u

/* SWS_CRC_00023
 * Users of the Crc module (e.g. NVRAM Manager) shall only include Crc.h
 * This requirement is only applicable to users. That's why it is not verified in the unit tests
 *
 * @req SWS_CRC_00024
 * The Crc module shall provide the following files:
 * C file Crc_xxx.c containing parts of CRC code
 * An API interface Crc.h providing the function prototypes to access the library CRC functions
 * A header file Crc_Cfg.h providing specific parameters for the CRC.
*/

/* @req SWS_CRC_00022
 * The Crc module shall comply with the following include file structure:
 * Crc.h shall include Crc_Cfg.h, Std_Types.h and Crc_MemMap.h
 * Crc_xxx.c shall include Crc.h
 */
#include "Crc_Cfg.h"

/* @req SWS_CRC_00018
 * All types included shall be only Std_Types
 */
#include "Std_Types.h"

/* These two applies to the caller and it set by the interface
 *
 * @req SWS_CRC_00014
 * The CRC function (with parameter Crc_IsFirstCall = TRUE) shall do the specified operations.
 *
 * @req SWS_CRC_00041
 * The CRC function (with parameter Crc_IsFirstCall = FALSE) shall do the specified operations.
 *
 * For the details of the specified operations refer to AUTOSAR_SWS_CRCLibrary.pdf
 */


#ifdef Crc_8_Mode
/** @brief This service makes a CRC8 calculation on Crc_Length data bytes,
 *         using the polynomial 0x1D.
 *
 *  @param Crc_DataPtr Pointer to start address of data block to be calculated.
 *  @param Crc_Length Length of data block to be calculated in bytes.
 *  @param Crc_StartValue8 Start value when the algorithm starts.
 *  @param Crc_IsFirstCall TRUE: First call in a sequence or individual CRC calculation;
 *                         FALSE: Subsequent call in a call sequence.
 *  @return 8 bit result of CRC calculation.
 */
/* @req SWS_CRC_00031 API for 8-bit SAE J1850 CRC Calculation */
uint8  Crc_CalculateCRC8(const uint8* Crc_DataPtr, uint32 Crc_Length, uint8 Crc_StartValue8, boolean Crc_IsFirstCall);
#endif


#ifdef Crc_8_8H2FMode
/** @brief This service makes a CRC8 calculation on Crc_Length data bytes,
 *         using the polynomial 0x2F.
 *
 *  @param Crc_DataPtr Pointer to start address of data block to be calculated.
 *  @param Crc_Length Length of data block to be calculated in bytes.
 *  @param Crc_StartValue8H2F Start value when the algorithm starts.
 *  @param Crc_IsFirstCall TRUE: First call in a sequence or individual CRC calculation;
 *                         FALSE: Subsequent call in a call sequence.
 *  @return 8 bit result of CRC calculation.
 */
/* @req SWS_CRC_00043 API for 8-bit 0x2F polynomial CRC Calculation */
uint8  Crc_CalculateCRC8H2F(const uint8* Crc_DataPtr, uint32 Crc_Length, uint8 Crc_StartValue8H2F, boolean Crc_IsFirstCall);
#endif


#ifdef Crc_16_Mode
/** @brief This service makes a CRC16 calculation on Crc_Length data bytes,
 *         using the polynomial 0x1021.
 *  @param Crc_DataPtr Pointer to start address of data block to be calculated.
 *  @param Crc_Length Length of data block to be calculated in bytes.
 *  @param Crc_StartValue16 Start value when the algorithm starts.
 *  @param Crc_IsFirstCall TRUE: First call in a sequence or individual CRC calculation;
 *                         FALSE: Subsequent call in a call sequence.
 *  @return 16 bit result of CRC calculation.
 */
/* @req SWS_CRC_00019 API for 16-bit CCITT-FALSE CRC16 */
uint16 Crc_CalculateCRC16(const uint8* Crc_DataPtr, uint32 Crc_Length, uint16 Crc_StartValue16, boolean Crc_IsFirstCall);
#endif


#ifdef Crc_32_Mode
/** @brief This service makes a CRC32 calculation on Crc_Length data bytes,
 *         using the polynomial 0x04C11DB7.
 *
 *  @param Crc_DataPtr Pointer to start address of data block to be calculated.
 *  @param Crc_Length Length of data block to be calculated in bytes.
 *  @param Crc_StartValue32 Start value when the algorithm starts.
 *  @param Crc_IsFirstCall TRUE: First call in a sequence or individual CRC calculation;
 *                         FALSE: Subsequent call in a call sequence.
 *  @return 32 bit result of CRC calculation.
 */
/* @req SWS_CRC_00020 API for 32-bit Ethernet CRC Calculation */
uint32 Crc_CalculateCRC32(const uint8* Crc_DataPtr, uint32 Crc_Length, uint32 Crc_StartValue32, boolean Crc_IsFirstCall);
#endif


#ifdef Crc_32P4_Mode
/** @brief This service makes a CRC32 calculation on Crc_Length data bytes,
 *         using the polynomial 0xF4ACFB13. This CRC routine is used by E2E Profile 4.
 *
 *  @param Crc_DataPtr Pointer to start address of data block to be calculated.
 *  @param Crc_Length Length of data block to be calculated in bytes.
 *  @param Crc_StartValue32P4 Start value when the algorithm starts.
 *  @param Crc_IsFirstCall TRUE: First call in a sequence or individual CRC calculation;
 *                         FALSE: Subsequent call in a call sequence.
 *  @return 32 bit result of CRC calculation.
 */
/* @req SWS_CRC_00058 API for 32-bit 0xF4ACFB13 polynomial CRC calculation */
uint32 Crc_CalculateCRC32P4(const uint8* Crc_DataPtr, uint32 Crc_Length, uint32 Crc_StartValue32P4, boolean Crc_IsFirstCall);
#endif


/* @req SWS_CRC_00011 The function Crc_GetVersionInfo shall return the version information of the Crc module */
/* @req SWS_CRC_00021 API for Crc_GetVersionInfo */
/** @brief Return the version information of Crc module.
 *
 * The function Crc_GetVersionInfo shall return the version
 * information of the Crc module.
 * The version information includes:
 * - Module Id
 * - Vendor Id
 * - sw_major_version
 * - sw_minor_version
 * - sw_patch_version
 *
 *  @param Std_VersionInfoType The type including Module and Vendor ID for the Crc Module.
 *  @return void.
 */

#if (SAFELIB_VERSIONINFO_API == STD_ON)
void Crc_GetVersionInfo(Std_VersionInfoType* versioninfo);
#endif

#endif /* CRC_H_ */
