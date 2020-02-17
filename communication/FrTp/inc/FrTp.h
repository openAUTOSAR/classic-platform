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
#ifndef FRTP_H_
#define FRTP_H_
/* @req FRTP1004 */
#include "FrTp_Types.h"
#include "FrTp_Cbk.h" /* @req FRTP1135 */

#define FRTP_VENDOR_ID                     60u
#define FRTP_MODULE_ID                     36u

#define FRTP_AR_RELEASE_MAJOR_VERSION      4u
#define FRTP_AR_RELEASE_MINOR_VERSION      0u
#define FRTP_AR_RELEASE_REVISION_VERSION   3u

#define FRTP_AR_MAJOR_VERSION              FRTP_AR_RELEASE_MAJOR_VERSION
#define FRTP_AR_MINOR_VERSION              FRTP_AR_RELEASE_MINOR_VERSION
#define FRTP_AR_PATCH_VERSION              FRTP_AR_RELEASE_REVISION_VERSION

#define FRTP_SW_MAJOR_VERSION              1u
#define FRTP_SW_MINOR_VERSION              0u
#define FRTP_SW_PATCH_VERSION              0u

#include "FrTp_Cfg.h"
#include "FrIf.h"


/* @req FRTP1157 */

/*
 * Errors described by FrTp 7.7.3 Error classification.
 *****************************************************/
/** @req FRTP1132 */ /** @req  FRTP1111 */
#define FRTP_E_UNINIT               0x01u
#define FRTP_E_NULL_PTR             0x02u
#define FRTP_E_INVALID_PDU_SDU_ID   0x03u
#define FRTP_E_INVALID_PARAMETER    0x04u
#define FRTP_E_SEG_ERROR            0x05u
#define FRTP_E_UMSG_LENGTH_ERROR    0x06u
#define FRTP_E_NO_CHANNEL           0x07u

/*
 * Service IDs for FrTP function definitions.
 */
#define FRTP_SERVICE_ID_INIT                       0x00u
#define FRTP_SERVICE_ID_GET_VERSION_INFO           0x27u
#define FRTP_SERVICE_ID_SHUTDOWN                   0x01u
#define FRTP_SERVICE_ID_TRANSMIT                   0x02u
#define FRTP_SERVICE_ID_CANCEL_TRANSMIT_REQUEST    0x03u
#define FRTP_SERVICE_ID_CHANGE_PARAMETER           0x04u
#define FRTP_SERVICE_ID_CANCEL_RECEIVE             0x08u
#define FRTP_SERVICE_ID_TRIGGER_TRANSMIT           0x41u
#define FRTP_SERVICE_ID_RX_INDICATION              0x42u
#define FRTP_SERVICE_ID_TX_CONFIRMATION            0x40u
#define FRTP_SERVICE_ID_MAIN_FUNCTION              0x10u


#define FRTP_INVALID_PDU_ID 0xFFFFu
#define FRTP_NO_RETRY 0u
#define FRTP_ACK_WITH_RETRY 1u

/* 8.3.1 Standard functions */
/* This service returns the version information of this module. */
/* @req FRTP215 */
#if ( FRTP_VERSION_INFO_API == STD_ON ) /* @req FRTP498 */
void FrTp_GetVersionInfo(Std_VersionInfoType* versioninfo);
#endif /* FRTP_VERSION_INFO_API */

/* This service initializes all global variables of a FlexRay Transport Layer instance
and set it in the idle state */
/* @req FRTP147 */
void FrTp_Init(const FrTp_ConfigType* ConfigPtr);

/*This service closes all pending transport protocol connections by simply stopping
operation, frees all resources and stops the FrTp Module */
/* @req FRTP148 */
void FrTp_Shutdown(void);

/*This service is utilized to request the transfer of data*/
/* @req FRTP149 */
Std_ReturnType FrTp_Transmit(PduIdType FrTpTxSduId, const PduInfoType* FrTpTxSduInfoPtr);

/*This service primitive is used to cancel the transfer of pending Fr N-SDUs.*/
/* @req FRTP150 */
Std_ReturnType FrTp_CancelTransmit(PduIdType FrTpTxSduId);

/*Request to change transport protocol parameter BandwithControl.*/
/* !req FRTP151 */
Std_ReturnType FrTp_ChangeParameter( PduIdType id, TPParameterType parameter, uint16 value);

/*this API with the corresponding RxSduId the currently ongoing data
reception is terminated immediately */
/* @req FRTP1172 */
Std_ReturnType FrTp_CancelReceive(PduIdType FrTpRxSduId);

/*Schedules the FlexRay TP. (Entry point for scheduling)*/
/* @req FRTP162 */
void FrTp_MainFunction(void);


#endif /* FRTP_H_ */
