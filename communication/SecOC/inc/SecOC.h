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

/** @reqSettings DEFAULT_SPECIFICATION_REVISION=4.1.2 */
/** @tagSettings DEFAULT_ARCHITECTURE=GENERIC */

/* @req SWS_SecOC_00001 General SecOC module definitions shall be defined in SecOC.h */
/* @req SWS_SecOC_00137*/

#ifndef SECOC_H_
#define SECOC_H_

#include "SecOC_Cfg.h"

#define SECOC_MODULE_ID		150u
#define SECOC_VENDOR_ID		60u

#define SECOC_SW_MAJOR_VERSION    1u
#define SECOC_SW_MINOR_VERSION    0u
#define SECOC_SW_PATCH_VERSION    0u
#define SECOC_AR_MAJOR_VERSION    4u
#define SECOC_AR_MINOR_VERSION    2u
#define SECOC_AR_PATCH_VERSION    1u

/* @req SWS_SecOC_00101 */
#if (SecOCDevErrorDetect == STD_ON)
#define SECOC_E_PARAM_POINTER               0x01u
#define SECOC_E_INVALID_REQUEST             0x02u
#define SECOC_E_INVALID_PDU_SDU_ID          0x03u
#define SECOC_E_CRYPTO_FAILURE              0x04u
#define SECOC_E_RESTORE_FAILURE             0x05u
#define SECOC_E_FRESHNESS_VALUE_AT_LIMIT    0x06u
#define SECOC_E_UNINIT                      0x20u
#endif


/*
 * Service IDs for SecOC function definitions.
 */
#define SERVICE_ID_SECOC_INIT                       0x01u
#define SERVICE_ID_SECOC_DEINIT                     0x05u
#define SERVICE_ID_SECOC_GET_VERSION_INFO           0x02u
#define SERVICE_ID_SECOC_TRANSMIT                   0x03u
#define SERVICE_ID_SECOC_CANCEL_TRANSMIT            0x04u
#define SERVICE_ID_SECOC_ASSOCIATE_KEY              0x07u
#define SERVICE_ID_SECOC_FRESHNESS_VALUE_READ       0x08u
#define SERVICE_ID_SECOC_FRESHNESS_VALUE_WRITE      0x09u
#define SERVICE_ID_SECOC_VERIFY_STATUS_OVERRIDE     0x0bu
#define SERVICE_ID_SECOC_RX_INDICATION              0x42u
#define SERVICE_ID_SECOC_TP_RX_INDICATION           0x45u
#define SERVICE_ID_SECOC_TX_CONFIRMATION            0x40u
#define SERVICE_ID_SECOC_TP_TX_CONFIRMATION         0x48u
#define SERVICE_ID_SECOC_TRIGGER_TRANSMIT           0x41u
#define SERVICE_ID_SECOC_COPY_RX_DATA               0x44u
#define SERVICE_ID_SECOC_COPY_TX_DATA               0x43u
#define SERVICE_ID_SECOC_START_OF_RECEPTION         0x46u
#define SERVICE_ID_SECOC_MAIN_FUNCTION              0x06u


/**
 * Initialize the SecOC module
 * @param config Pointer to selected configuration structure
 */
/* @req SWS_SecOC_00106 SecOC_Init */
void SecOC_Init( const SecOC_ConfigType* config);

/**
 * Stop the SecOC service, empties the vbuffers and set state to SecOC_UNINIT
 */
/* @req SWS_SecOC_00161 SecOC_DeInit */
void SecOC_DeInit( void );

/**
 * Return the version information for SecOC
 * @param versioninfo pointer to where the version information is to be stored
 */
/* @req SWS_SecOC_00107 SecOC_GetVersionInfo */
void SecOC_GetVersionInfo( Std_VersionInfoType* versioninfo );

/* @req SWS_SecOC_00122*/
//Std_ReturnType SecOC_VerifyStatusOverride(uint16 freshnessValueID, uint8 overrideStatus, uint8 numberOfMessagesToOverride);

/* @req SWS_SecOC_00116*/
Std_ReturnType SecOC_AssociateKey(uint8 keyID, const SecOC_KeyType* keyPtr);

/* @req SWS_SecOC_00117*/
//Std_ReturnType SecOC_FreshnessValueRead(uint16 freshnessValueID, uint64* counterValue);

/* @req SWS_SecOC_00118*/
//Std_ReturnType SecOC_FreshnessValueWrite(uint16 freshnessValueID, uint64 counterValue);


Std_ReturnType SecOC_Transmit( PduIdType id, const PduInfoType* info );
Std_ReturnType SecOC_CancelTransmit( PduIdType id );

void SecOC_RxIndication( PduIdType RxPduId, const PduInfoType* PduInfoPtr );
void SecOC_TxConfirmation( PduIdType TxPduId ); //PduR

//void SecOC_TpRxIndication( PduIdType id, Std_ReturnType result );
//void SecOC_TpTxConfirmation( PduIdType id, Std_ReturnType result );

//Std_ReturnType SecOC_TriggerTransmit( PduIdType TxPduId, PduInfoType* PduInfoPtr );

//BufReq_ReturnType SecOC_CopyRxData( PduIdType id, const PduInfoType* info, PduLengthType* bufferSizePtr );
//BufReq_ReturnType SecOC_CopyTxData( PduIdType id, const PduInfoType* info, RetryInfoType* retry, PduLengthType* availableDataPtr );
//BufReq_ReturnType SecOC_StartOfReception( PduIdType id, const PduInfoType* info, PduLengthType TpSduLength, PduLengthType* bufferSizePtr );

/**
 * Main function of SecOC, performs authentication and verification of packets.
 */
/* @req SWS_SecOC_00131 SecOC_MainFunction */
void SecOC_MainFunction( void );


#endif /* SECOC_H_ */
