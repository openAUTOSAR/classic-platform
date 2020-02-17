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
/** @addtogroup LinTP LIN Interface
 *  @{ */

/** @file LinTP.h
 * API and type definitions for LIN Interface.
 */

#ifndef LINTP_H_
#define LINTP_H_

#define LINTP_MODULE_ID            62u
#define LINTP_VENDOR_ID            60u

#define LINTP_SW_MAJOR_VERSION              1u
#define LINTP_SW_MINOR_VERSION              0u
#define LINTP_SW_PATCH_VERSION              0u
#define LINTP_AR_RELEASE_MAJOR_VERSION      4u
#define LINTP_AR_RELEASE_MINOR_VERSION      3u
#define LINTP_AR_RELEASE_PATCH_VERSION      0u


#include "Std_Types.h"
#include "LinIf.h"
#include "LinTp_Cfg.h"
#include "LinTp_Types.h"
#include "ComM_Types.h"


/** @name Service id's */
//@{
/* @req SWS_LinIf_00352 */
#define LINTP_GETVERSIONINFO_SERVICE_ID     0x42u
/* @req SWS_LinIf_00355 */
#define LINTP_SHUTDOWN_SERVICE_ID           0x43u
/* @req SWS_LinIf_00500 */
#define LINTP_CANCELTRANSMIT_SERVICE_ID     0x4au
/* @req SWS_LinIf_00501 */
#define LINTP_CHANGEPARAMETER_SERVICE_ID    0x4bu
/* @req SWS_LinIf_00378 */
#define LINIF_CHECKWAKEUP_SERVICE_ID        0x60u
/* @req SWS_LinIf_00625 */
#define LINTP_CANCELRECEIVE_SERVICE_ID      0x4cu
/* @req SWS_LinIf_00715 */
#define LINIF_WAKEUPCONFIRMATION_SERVICE_ID 0x61u


//@}
/* @req SWS_LinIf_00376 */
/** @name Error Codes */
//@{
#define LINIF_E_UNINIT                      (uint8) 0x00u
#define LINIF_E_NONEXISTENT_CHANNEL         (uint8) 0x20u
#define LINIF_E_PARAMETER                   (uint8) 0x30u
#define LINIF_E_PARAM_POINTER               (uint8) 0x40u
#define LINIF_E_SCHEDULE_OVERFLOW           (uint8) 0x50u
#define LINIF_E_SCHEDULE_REQUEST_ERROR      (uint8) 0x51u
#define LINIF_E_TRCV_INV_MODE               (uint8) 0x53u
#define LINIF_E_TRCV_NOT_NORMAL             (uint8) 0x54u
#define LINIF_E_PARAM_WAKEUPSOURCE          (uint8) 0x55u
#define LINIF_E_RESPONSE                    (uint8) 0x60u
#define LINIF_E_NC_NO_RESPONSE              (uint8) 0x61u
// Following error codes are added by ArcCore
#define LINIF_E_UNEXPECTED_EXECUTION        0x70u
//@}

/* @req SWS_LinIf_00639 */
/* @req SWS_LinIf_00352 */
/** @brief Return the version information of LinTp module.
 *
 * The function LinTp_GetVersionInfo shall return the version
 * information of the LinTp module.
 * The version information includes:
 * - Module Id
 * - Vendor Id
 * - sw_major_version
 * - sw_minor_version
 * - sw_patch_version
 *
 * The function LinTp_GetVersionInfo raises a development error "LINIF_E_PARAM_POINTER."
 * if versioninfo has an invalid value
 *
 *  @param Std_VersionInfoType The type including Module and Vendor ID for the LinTP Module.
 *  @return void.
 */
#if (LINTP_VERSION_INFO_API == STD_ON)
void LinTp_GetVersionInfo( Std_VersionInfoType *versionInfo );
#endif
/** @} */

/* @req SWS_LinIf_00350 */
void LinTp_Init( const LinTp_ConfigType* ConfigPtr );

/* @req SWS_LinIf_00351 */
Std_ReturnType LinTp_Transmit(PduIdType LinTxPduId,const PduInfoType* PduInfoPtr);

/* @req SWS_LinIf_00355 */
void  LinTp_Shutdown(void);

/* @req SWS_LinIf_00501 */
Std_ReturnType  LinTp_ChangeParameter(PduIdType id,TPParameterType parameter,uint16 value);

/* @req SWS_LinIf_00625 */
Std_ReturnType  LinTp_CancelReceive(PduIdType RxPduId);

/* @req SWS_LinIf_00500 */
Std_ReturnType  LinTp_CancelTransmit(PduIdType TxPduId);

#endif
