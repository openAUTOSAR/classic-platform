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

/** @req XCP505 */ /*Provide Xcp.h - can not be tested with conventional module tests*/

#ifndef XCP_H_
#define XCP_H_

#include "Modules.h"

/*********************************************
 *            MODULE PARAMETERS              *
 *********************************************/

#define XCP_VENDOR_ID                   60u
#define XCP_MODULE_ID                   MODULE_ID_XCP
#define XCP_AR_RELEASE_MAJOR_VERSION    4u
#define XCP_AR_RELEASE_MINOR_VERSION    1u
#define XCP_AR_RELEASE_REVISION_VERSION 3u

#define XCP_SW_MAJOR_VERSION   1u
#define XCP_SW_MINOR_VERSION   0u
#define XCP_SW_PATCH_VERSION   0u
#define XCP_PROTOCOL_MAJOR_VERSION 1u
#define XCP_PROTOCOL_MINOR_VERSION 0u
#define XCP_TRANSPORT_MAJOR_VERSION 1u
#define XCP_TRANSPORT_MINOR_VERSION 0u

#include "Xcp_Cfg.h"

/*********************************************
 *               MAIN FUNCTIONS              *
 *********************************************/


#define XCP_E_INIT_FAILED     0x04u
#define XCP_E_INV_POINTER     0x01u
#define XCP_E_NOT_INITIALIZED 0x02u
#define XCP_E_INVALID_PDUID   0x03u
#define XCP_E_NULL_POINTER    0x12u
#define XCP_E_INVALID_EVENT   0x13u

#if (XCP_VERSION_INFO_API == STD_ON)
void Xcp_GetVersionInfo(Std_VersionInfoType* versionInfo);
#endif /*XCP_VERION_INFO_API == STD_ON*/

void Xcp_Init(const Xcp_ConfigType* Xcp_ConfigPtr);
#if 0
void Xcp_Disconnect();
#endif
void Xcp_MainFunction(void);
void Xcp_MainFunction_Channel(uint32 channel);

/*********************************************
 *          PROTOCOL SETTINGS                *
 *********************************************/

/*Only element size 1 is supported*/
#define XCP_ELEMENT_SIZE 1
#define XCP_GRANULARITY_ODT_ENTRY_SIZE_DAQ  1
#define XCP_GRANULARITY_ODT_ENTRY_SIZE_STIM 1

#if(XCP_IDENTIFICATION == XCP_IDENTIFICATION_ABSOLUTE)
#   define XCP_MAX_ODT_SIZE (XCP_MAX_DTO - 1) /**< defines the maximum number of bytes that can fit in a dto packages data area*/
#elif(XCP_IDENTIFICATION == XCP_IDENTIFICATION_RELATIVE_BYTE)
#   define XCP_MAX_ODT_SIZE (XCP_MAX_DTO - 2) /**< defines the maximum number of bytes that can fit in a dto packages data area*/
#elif(XCP_IDENTIFICATION == XCP_IDENTIFICATION_RELATIVE_WORD)
#   define XCP_MAX_ODT_SIZE (XCP_MAX_DTO - 3) /**< defines the maximum number of bytes that can fit in a dto packages data area*/
#elif(XCP_IDENTIFICATION == XCP_IDENTIFICATION_RELATIVE_WORD_ALIGNED)
#   define XCP_MAX_ODT_SIZE (XCP_MAX_DTO - 4) /**< defines the maximum number of bytes that can fit in a dto packages data area*/
#endif

#ifndef    XCP_MAX_ODT_ENTRY_SIZE_DAQ
#   define XCP_MAX_ODT_ENTRY_SIZE_DAQ     XCP_MAX_ODT_SIZE
#endif

#ifndef    XCP_MAX_ODT_ENTRY_SIZE_STIM
#   define XCP_MAX_ODT_ENTRY_SIZE_STIM    XCP_MAX_ODT_SIZE
#endif

#if(XCP_GRANULARITY_ODT_ENTRY_SIZE_STIM > XCP_GRANULARITY_ODT_ENTRY_SIZE_DAQ)
#   define XCP_GRANULARITY_ODT_ENTRY_SIZE_MIN XCP_GRANULARITY_ODT_ENTRY_SIZE_DAQ
#   define XCP_GRANULARITY_ODT_ENTRY_SIZE_MAX XCP_GRANULARITY_ODT_ENTRY_SIZE_STIM
#else
#   define XCP_GRANULARITY_ODT_ENTRY_SIZE_MIN XCP_GRANULARITY_ODT_ENTRY_SIZE_STIM
#   define XCP_GRANULARITY_ODT_ENTRY_SIZE_MAX XCP_GRANULARITY_ODT_ENTRY_SIZE_DAQ
#endif

#ifndef XCP_MAX_ODT_ENTRIES
#   define XCP_MAX_ODT_ENTRIES (XCP_MAX_ODT_SIZE / XCP_GRANULARITY_ODT_ENTRY_SIZE_MIN)
#endif

#endif /* XCP_H_ */
