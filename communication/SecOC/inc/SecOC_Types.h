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

/* @req SWS_SecOC_00002 Type definitions of the SecOC module shall be defined in SecOC_Types.h */
/* @req SWS_SecOC_00103 */

#ifndef SECOC_TYPES_H_
#define SECOC_TYPES_H_

#include "ComStack_Types.h"

typedef struct {
    uint16 SecOCRxSecuredLayerPduId;
    void *SecOCRxSecuredLayerPduRef;
} SecOCRxSecuredPduLayerType;

typedef struct {
    uint16 SecOCRxAuthenticLayerPduId;
    void *SecOCRxAuthenticLayerPduRef;
} SecOCRxAuthenticPduLayerType;

typedef struct {
    uint16 SecOCTxSecuredLayerPduId;
    void *SecOCTxSecuredLayerPduRef;
} SecOCTxSecuredPduLayerType;

typedef struct {
    uint16 SecOCTxAuthenticLayerPduId;
    void *SecOCTxAuthenticLayerPduRef;
} SecOCTxAuthenticPduLayerType;


typedef struct {
    uint32  SecOCFreshnessTimestampTimePeriodFactor;
    uint16  SecOCAuthInfoRxLength;
    uint16  SecOCPduRPduId;
    uint16  SecOCDataId;
    uint16  SecOCFreshnessCounterSyncAttempts;
    uint16  SecOCFreshnessValueId;
    uint16  SecOCKeyId;
    uint8   SecOCFreshnessValueLength;
    uint8   SecOCFreshnessValueRxLength;
    float   SecOCRxAcceptanceWindow;
    /*
    uint16  SecOCSecondaryFreshnessValueId;
    boolean SecoCUseFreshnessTimestamp;
    SecOC_VerificationStatusPropType SecOCVerificationStatusPropagationMode;
    void *SecOCRxAuthServiceConfigRef;
    void *SecOCSameBufferPduRef;

    SecOCRxSecuredPduLayerType SecOCRxSecuredPduLayer;
    SecOCRxAuthenticPduLayerType SecOCRxAuthenticPduLayer;
    */
} SecOCRxPduProcessingType;

typedef struct {
    uint32  SecOCFreshnessTimestampTimePeriodFactor;
    uint16  SecOCPduRPduId;
    uint16  SecOCAuthInfoTxLength;
    uint16  SecOCAuthenticationRetries;
    uint16  SecOCDataId;
    uint16  SecOCFreshnessValueId;
    uint16  SecOCKeyId;
    uint8   SecOCFreshnessValueLength;
    uint8   SecOCFreshnessValueTxLength;
    boolean SecOCUseFreshnessTimestamp;
    /*
    void *SecOCTxAuthServiceConfigRef;
    void *SecOCSameBufferPduRef;

    SecOCTxSecuredPduLayerType SecOCTxSecuredPduLayer;
    SecOCTxAuthenticPduLayerType SecOCTxAuthenticPduLayer;
    */
} SecOCTxPduProcessingType;


/* @req SWS_SecOC_00104 */
typedef struct {
    uint8 cfgId;
    const SecOCTxPduProcessingType* SecOCTxPduProcessing;
    const SecOCRxPduProcessingType* SecOCRxPduProcessing;
} SecOC_ConfigType;

/* @req SWS_SecOC_00162 */
typedef enum {
    SECOC_UNINIT = 0,
    SECOC_INIT = 1
} SecOC_StateType;

/* @req SWS_SecOC_00149 SecOC_VerificationResultType */
typedef enum {
    SECOC_VERIFICATIONSUCCESS = 0,
    SECOC_VERIFICATIONFAILURE = 1,
    SECOC_FRESHNESSFAILURE = 2
} SecOC_VerificationResultType;

/* @req SWS_SecOC_00160 VerificationStatusType */
typedef struct {
    uint16 freshnessValueID;
    SecOC_VerificationResultType verificationStatus;
} SecOC_VerificationStatusType;




/* @req SWS_SecOC_00146 Separate buffers for the Authentic I-PD*U and the Secured I-PDU */

typedef struct {
    PduIdType pduId;
    PduInfoType pduInfo;
} AuthenticIPDU_BufType;

typedef struct {
    uint16 length;
    uint16 secOCDataId;
    uint16 freshnessValueID;
    PduIdType pduId;
    AuthenticIPDU_BufType authenticIPDU;
    uint8 auth[32];
} SecuredIPDU_BufType;



#endif /* SECOC_TYPES_H_ */
