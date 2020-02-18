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

/** @file E2E.h
 *
 *  The E2E library provides
 *      *The definition of profiles 1,2,4,5 and 6 including check and protect functions
 *      *A state machine describing the logical algorithm of E2E monitoring independent of the used profile
 */

#ifndef E2E_H_
#define E2E_H_


/* @req SWS_E2E_00037 The E2E library shall not have any configuration options. */

/* @req SWS_E2E_00050 The implementation of the E2E Library shall comply with the requirements
 * for the development of safety-related software for the automotive domain. */

/* @req SWS_E2E_00311 The configuration of the E2E Library and of the code invoking it
 * (e.g. E2E wrapper or E2E callouts) shall be implemented and configured
 * (including configuration options used from other subsystems, e.g. COM signal to I-PDU mapping)
 * according to the requirements for the development of safety-related software for the automotive domain. */

/* @req SWS_E2E_00115 E2E library files (i.e. E2E_*.*) shall not include any RTE files */

/* @req SWS_E2E_00048 E2E library shall be built of the following files:
 *      E2E.h (common header), E2E.c (implementation of common parts),
 *      E2E_PXX.c and E2E_PXX.h (where XX: e.g. 01, 02, … representing the profile)
 *      and E2E_SM.c and E2E_SM.h
 */
/* @req SWS_E2E_00110 The E2E library shall not call any functions from external
 *      modules apart from explicitly listed expected interfaces of E2E Library:
 *      Crc.h:  Crc_CalculateCRC8, Crc_CalculateCRC16, Crc_CalculateCRC32P4, Crc_CalculateCRC8H2F*/
#include "Crc.h"

/* @req SWS_E2E_00017
 * All types included shall be only Std_Types and Std_VersionInfoType
 */
#include "Std_Types.h"

/* According to AUTOSAR_SWS_E2ELibrary.pdf, Figure5-1: File dependencies:
 * E2E_MemMap.h is included from all E2E files
 */
#include "E2E_MemMap.h"


/* @req SWS_E2E_00038
 * The standardized common published parameters
 * shall be published within the header file of this module.
 */
#define E2E_VENDOR_ID               60
#define E2E_MODULE_ID               207

#define E2E_AR_RELEASE_MAJOR_VERSION        4u
#define E2E_AR_RELEASE_MINOR_VERSION        3u
#define E2E_AR_RELEASE_REVISION_VERSION     0u

#define E2E_SW_MAJOR_VERSION        2u
#define E2E_SW_MINOR_VERSION        0u
#define E2E_SW_PATCH_VERSION        0u

/* @req SWS_E2E_00327
 * The implementer of the E2E Library shall avoid the integration of incompatible files. Minimum implementation is the version check of the header files.
 */
#if (E2E_AR_RELEASE_MAJOR_VERSION != CRC_AR_RELEASE_MAJOR_VERSION) || (E2E_AR_RELEASE_MINOR_VERSION != CRC_AR_RELEASE_MINOR_VERSION)
#error "SW Version mismatch between E2E.h and Crc.h"
#endif

/* @req SWS_E2E_00011
 * The E2E Library shall report errors detected by library-internal
 * mechanisms to callers of E2E functions through return value.
 */
/* @req SWS_E2E_00047 The following error flags for errors shall be used by all E2E Library functions: */
#define E2E_E_INPUTERR_NULL     0x13 //ASR4.3.0 At least one pointer parameter is a NULL pointer
#define E2E_E_INPUTERR_WRONG    0x17 //ASR4.3.0 At least one input parameter is erroneous, e.g. out of range
#define E2E_E_INTERR            0x19 //ASR4.3.0 An internal library error has occurred (e.g. error detected by program flow monitoring, violated invariant or postcondition)
#define E2E_E_OK                0x00 //ASR4.3.0 Function completed successfully
#define E2E_E_WRONGSTATE        0x1A //ASR4.3.0 Function executed in wrong state


/* @req SWS_E2E_00032 API: E2E_GetVersionInfo */
/* @req SWS_E2E_00033 The function E2E_GetVersionInfo shall return the version information of the E2E module */
/** @brief Return the version information of E2E module.
 *
 * The function E2E_GetVersionInfo shall return the version
 * information of the E2E module.
 * The version information includes:
 * - Module Id
 * - Vendor Id
 * - sw_major_version
 * - sw_minor_version
 * - sw_patch_version
 *
 *  @param Std_VersionInfoType The type including Module and Vendor ID for the E2E Module.
 *  @return void.
 */
#if (SAFELIB_VERSIONINFO_API == STD_ON)
void E2E_GetVersionInfo(Std_VersionInfoType* versioninfo);
#endif

#endif /* E2E_H_ */
