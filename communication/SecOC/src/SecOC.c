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

#include "Std_Types.h"

/* @req SWS_SecOC_00153 SecOC module shall be implemented so that no other module depend on it */

#include "SecOC.h"
#include "SecOC_Cbk.h"
#include "PduR_SecOC.h"
#include <string.h>
#include "Cal.h"
#include "SecOC_Types.h"


//lint -save -e715 -e818



#if (SecOCDevErrorDetect == STD_ON)
#include "Det.h"

#define DET_REPORTERROR(_api, _error)                     \
    do {                                                  \
        (void)Det_ReportError(SECOC_MODULE_ID, 0, _api, _error); \
    } while(false)

#else
#define DET_REPORTERROR(_api, _error)
#endif

typedef struct {
    uint32 msgSize;
    uint8 Authenticator[32];
    uint8 DataToAuthenticator[32];
} VerifyType;

typedef struct {
    uint8 Authenticator[32];
    uint8 DataToAuthenticator[64];
} AuthType;

/**
 * Function prototypes
 */
static Std_ReturnType verifyIPDU(VerifyType data);
static Std_ReturnType authenticateIPDU(uint16 keyId, const uint8 *data, uint16 length, uint8 *result, uint32 *resultLength);
static void freeSecuredIPDUBuffers(uint8 pos);
static void authLoop(void);
static void verifyLoop(void);

/**
 * Global variables
 */

SecOC_KeyType keyList[16];

//local variables

const SecOC_ConfigType* secOc_Config;
SecOC_StateType state = SECOC_UNINIT;
SecuredIPDU_BufType securedPduBuffer[64];  /* should be 32  */
uint8 securedPos = 0;
AuthenticIPDU_BufType authenticPduBuffer[64]; /* should be 32 */
uint8 authenticPos = 0;

static void freeSecuredIPDUBuffers(uint8 pos) {
    securedPduBuffer[pos].freshnessValueID = 0;
    securedPduBuffer[pos].length = 0;
    securedPduBuffer[pos].pduId = 0;
}

static Std_ReturnType verifyIPDU(VerifyType data) {
    Cal_ConfigIdType cfgId = 0;
    Cal_MacVerifyCtxBufType ctxBuffer;
    Cal_VerifyResultType result = CAL_E_VER_OK;

    Std_ReturnType status;
    status = E_NOT_OK;
    Cal_SymKeyType key;
    key.length = 20;
    Cal_AlignType keyData[] = { 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b };
    memcpy(key.data, keyData, sizeof(keyData));


    uint32 MacLength = 32;
    uint32 msgSize = data.msgSize;

    if (Cal_MacVerifyStart(cfgId, ctxBuffer, &key) != CAL_E_OK) {
        status = E_NOT_OK;
    }

    else if (Cal_MacVerifyUpdate(cfgId, ctxBuffer, data.DataToAuthenticator, msgSize) != CAL_E_OK) {
        status = E_NOT_OK;
    }

    else if( (Cal_MacVerifyFinish(cfgId, ctxBuffer, data.Authenticator, MacLength, &result) == CAL_E_OK) && (result == CAL_E_VER_OK)) {

        status = E_OK;

    } else {

        status = E_NOT_OK;
    }
    return status;
}

static Std_ReturnType authenticateIPDU(uint16 keyId, const uint8 *data, uint16 length, uint8 *result, uint32 *resultLength) {
    Cal_ReturnType retVal;
    Cal_ConfigIdType cfgId = 0;
    Cal_MacVerifyCtxBufType ctxBuffer;

    Std_ReturnType status;
    status = E_OK;
    Cal_SymKeyType key;
    key.length = keyList[keyId].length;
    memcpy(key.data, keyList[keyId].data, key.length);

    retVal = Cal_MacGenerateStart(cfgId, ctxBuffer, &key);
    if (retVal != CAL_E_OK) {
        status = E_NOT_OK;
    } else {
        retVal = Cal_MacGenerateUpdate(cfgId, ctxBuffer, data, length);
        if (retVal != CAL_E_OK ) {
            status = E_NOT_OK;
        } else {
            retVal = Cal_MacGenerateFinish(cfgId, ctxBuffer, result, resultLength, TRUE);
            if (retVal != CAL_E_OK) {
                status = E_NOT_OK;
            }
        }
    }
    return status;
}
/* @req SWS_SecOC_00031 Creation of secured I-PDU should follow the six steps described in spec */
/* @req SWS_SecOC_00033 */
/* @req SWS_SecOC_00034 */
/* @req SWS_SecOC_00035 */
/* @req SWS_SecOC_00062 SecOC shall provide complete Secured I-PDU for further transmission by triggering PduR_SecOCTransmit*/
/* @req SWS_SecOC_00110 The buffer that contains the Authentic I-PDU shall not overwrite Secured I-PDU*/
/* @req SWS_SecOC_00057 SecOC shall provide sufficient buffer capacities to store the Authentic I-PDU*/
/* @req SWS_SecOC_00108 SecOC shall return E_NOT_OK if the transmission of Authentic I-PDU is not able to serve*/
static void authLoop(void) {
    AuthType dataToAuthenticator;
    PduInfoType pduInfo;


    //empty all the buffers
    memset(dataToAuthenticator.DataToAuthenticator, 0, sizeof(dataToAuthenticator.DataToAuthenticator));
    memset(dataToAuthenticator.Authenticator, 0, sizeof(dataToAuthenticator.Authenticator));

    pduInfo.SduDataPtr = dataToAuthenticator.DataToAuthenticator;
    pduInfo.SduLength = 0;

    // Loop through the PDUs in the buffer
    for(uint8 i = 0; i < authenticPos; i++) {
        // SecOCDataId | AuthenticIPDU | FreshnessVerifyValue
        uint32 resLength = 32;
        memcpy(&dataToAuthenticator.DataToAuthenticator[0],
               &secOc_Config->SecOCTxPduProcessing[authenticPduBuffer[i].pduId].SecOCDataId,
               sizeof(secOc_Config->SecOCTxPduProcessing[authenticPduBuffer[i].pduId].SecOCDataId)); // SecOCDataId

        memcpy(&dataToAuthenticator.DataToAuthenticator[sizeof(secOc_Config->SecOCTxPduProcessing[authenticPduBuffer[i].pduId].SecOCDataId)],
               authenticPduBuffer[i].pduInfo.SduDataPtr,
               authenticPduBuffer[i].pduInfo.SduLength);
        Std_ReturnType res;

        res = authenticateIPDU(secOc_Config->SecOCTxPduProcessing[authenticPduBuffer[i].pduId].SecOCKeyId,
                               dataToAuthenticator.DataToAuthenticator,
                               authenticPduBuffer[i].pduInfo.SduLength + sizeof(secOc_Config->SecOCTxPduProcessing[authenticPduBuffer[i].pduId].SecOCDataId),
                               dataToAuthenticator.Authenticator,
                               &resLength);
        if (res == E_OK) {
            // Add Auth and Freshness to PDU
            memcpy(&dataToAuthenticator.DataToAuthenticator[0], authenticPduBuffer[i].pduInfo.SduDataPtr, authenticPduBuffer[i].pduInfo.SduLength);
            //dataToAuthenticator.DataToAuthenticator[authenticPduBuffer[i].pduInfo.SduLength] = 1;
            memcpy(&dataToAuthenticator.DataToAuthenticator[authenticPduBuffer[i].pduInfo.SduLength], dataToAuthenticator.Authenticator, sizeof(dataToAuthenticator.Authenticator));


            pduInfo.SduLength = authenticPduBuffer[i].pduInfo.SduLength + 32;
            pduInfo.SduDataPtr = dataToAuthenticator.DataToAuthenticator;
            if(PduR_SecOCTransmit(secOc_Config->SecOCTxPduProcessing[authenticPduBuffer[i].pduId].SecOCPduRPduId, &pduInfo) == E_NOT_OK)
            {
                // IMPROVEMENT: Set det error?
            }
        }
    }
    authenticPos = 0;
}

/* @req SWS_SecOC_00040 */
/* @req SWS_SecOC_00046 */
/* @req SWS_SecOC_00047 SecOCUseFreshnessTimestamp set to FALSE, SecOC shall verify the Authenticator by passing DataToAuthenticator*/
/* @req SWS_SecOC_00111 Secured I-PDU shall not overwrite Authentic I-PDU buffer */
/* @req SWS_SecOC_00057 SecOC should provide sufficient buffer capacities to store Secured I-PDU*/
/* @req SWS_SecOC_00109 SecOC shall return E_NOT_OK if the transmission of  Secured I-PDU is not able to serve*/
static void verifyLoop(void) {

    boolean status;
    status = TRUE;
    VerifyType dataToAuthenticator;
    uint8 attempts = 0;
    //FreshnessVerifyValueId = 0;
    //SecOC_VerificationResultType verificationResult;
    //extern const SecOCRxPduProcessingType SecOCRxPduProcessing[ ];

    //reset buffer
    memset(dataToAuthenticator.DataToAuthenticator, 0, sizeof(dataToAuthenticator.DataToAuthenticator));
    memset(dataToAuthenticator.Authenticator, 0, sizeof(dataToAuthenticator.Authenticator));

    //loop through incoming Secured I-PDU from lower layer

    for(uint8 i = 0; i < securedPos ; i++) {

        attempts = 0;

        //repeat
        while (attempts <= (secOc_Config->SecOCRxPduProcessing[securedPduBuffer[i].pduId].SecOCFreshnessCounterSyncAttempts) ) {

            //The SecOC module copies the Secured I-PDU into its own memory.

            //dataToAuthenticator.DataToAuthenticator[0] = securedPduBuffer[i].pduId;

            memcpy(&dataToAuthenticator.DataToAuthenticator[0],
                    &secOc_Config->SecOCRxPduProcessing[securedPduBuffer[i].pduId].SecOCDataId,
                    sizeof(secOc_Config->SecOCRxPduProcessing[securedPduBuffer[i].pduId].SecOCDataId)); // SecOCDataId
            dataToAuthenticator.msgSize = securedPduBuffer[i].authenticIPDU.pduInfo.SduLength +
                    sizeof(secOc_Config->SecOCRxPduProcessing[securedPduBuffer[i].pduId].SecOCDataId);

            //Copy message to DataToAuthenticator
            memcpy(&dataToAuthenticator.DataToAuthenticator[sizeof(secOc_Config->SecOCRxPduProcessing[securedPduBuffer[i].pduId].SecOCDataId)],
                   securedPduBuffer[i].authenticIPDU.pduInfo.SduDataPtr,
                   securedPduBuffer[i].authenticIPDU.pduInfo.SduLength);

            //copy MAC value to Authenticator
            memcpy(dataToAuthenticator.Authenticator, securedPduBuffer[i].auth, 32);

            //verify the contents of the Secured I-PDU
            Std_ReturnType verifyResult = verifyIPDU(dataToAuthenticator);

            //If ver success, call PduR_SecOCRxIndication
            if(verifyResult == E_OK){
                /* @req SWS_SecOC_00050 Pass the Authentic I-PDU to the upper layer using the lower layer interfaces of the PduR*/

                //SecOCRxPduProcessing[0].SecOCFreshnessValueId = FreshnessVerifyValueId;

                /* @req SWS_SecOC_00080 Call PduR_SecOCRxIndication referencing the Authentic I-PDU in the Secured I-PDU*/

                PduR_SecOCRxIndication(secOc_Config->SecOCRxPduProcessing[securedPduBuffer[i].pduId].SecOCPduRPduId,
                                       &securedPduBuffer[i].authenticIPDU.pduInfo);

                /* @req SWS_SecOC_00087 free all the buffers related to a Secured I-PDU if authenticated I-PDU has passed via PduR_SecOCRxIndication */
                freeSecuredIPDUBuffers(i);

                //verificationResult = SECOC_VERIFICATIONSUCCESS;
                securedPos = 0;
                status = FALSE;
                break;

            } else if((verifyResult == E_NOT_OK) && (attempts >= secOc_Config->SecOCRxPduProcessing[securedPduBuffer[i].pduId].SecOCFreshnessCounterSyncAttempts)){ //If ver fail, drop the message

                /* @req SWS_SecOC_00087 free all the buffers related to a Secured I-PDU if verification failed*/
               freeSecuredIPDUBuffers(i);

               //verificationResult = SECOC_VERIFICATIONFAILURE;
               /*lint -e{9011} break is required to terminate loop */
               break;

            } else if((verifyResult == E_NOT_OK) && (attempts < secOc_Config->SecOCRxPduProcessing[securedPduBuffer[i].pduId].SecOCFreshnessCounterSyncAttempts)){
                attempts++;

                //verificationResult = SECOC_VERIFICATIONFAILURE; //ska ej va här

                //re-attempt Authentication
                //break;

            } else {}
        }
    }
    if (status == TRUE) {
        securedPos = 0;
    }
}

/* @req SWS_SecOC_00054 SecOC_Init shall initialize all internal global variables and the buffers of the SecOC I-PDUs */
/* @req SWS_SecOC_00106 SecOC_Init */
void SecOC_Init( const SecOC_ConfigType* config ) {

    secOc_Config = config;
    authenticPos = 0;
    securedPos = 0;

    // Set state to Initialized
    state = SECOC_INIT;

}

/* @req SWS_SecOC_00156 SecOC_DeInit shall store all Freshness Values and all key information to NVRAM */
/* @req SWS_SecOC_00157 SecOC_DeInit shall clear all internal global variables and the buffers */
/* @req SWS_SecOC_00161 SecOC_DeInit */
void SecOC_DeInit( void ) {
    //clear the buffers
    memset(securedPduBuffer[0].authenticIPDU.pduInfo.SduDataPtr, 0, securedPduBuffer[0].authenticIPDU.pduInfo.SduLength);
    authenticPduBuffer[0].pduId = 0;
    //authenticPduBuffer[0].pduInfo = 0;

    //store all Freshness Values and all key to NVRAM


    //NvM_ReadBlock( NvM_BlockIdType blockId, void *dstPtr ) ;

    //SecOC_Config = NULL;

    //NvM_ReadBlock( NvM_BlockIdType blockId, void *dstPtr );

    //NvM_WriteBlock( NvM_BlockIdType blockId, void *srcPtr );
    //NvM_GetErrorStatus( NvM_BlockIdType blockId, NvM_RequestResultType *requestResultPtr);


    //Set state to Uninitialized
    state = SECOC_UNINIT;
}

/* @req SWS_SecOC_00107 SecOC_GetVersionInfo */
void SecOC_GetVersionInfo( Std_VersionInfoType* versioninfo ) {

    //check the version
    versioninfo->sw_major_version = SECOC_SW_MAJOR_VERSION;
    versioninfo->sw_minor_version = SECOC_SW_MINOR_VERSION;
    versioninfo->sw_patch_version = SECOC_SW_PATCH_VERSION;
    versioninfo->moduleID         = SECOC_MODULE_ID;

}

/* @req SWS_SecOC_00112 SecOC_Transmit */
Std_ReturnType SecOC_Transmit( PduIdType id, const PduInfoType* info ) {

    if (state == SECOC_UNINIT) {
        DET_REPORTERROR(SERVICE_ID_SECOC_TRANSMIT, SECOC_E_UNINIT);
        /*lint -e{904} Return statement is necessary in case of reporting a DET error */
        return E_NOT_OK;
    }

    if (id >= SECOC_NBR_OF_TXPDUS) {
        DET_REPORTERROR(SERVICE_ID_SECOC_TRANSMIT, SECOC_E_INVALID_PDU_SDU_ID);
        /*lint -e{904} ARGUMENT CHECK */
        return E_NOT_OK;
    }

    if( (info == NULL) || (authenticPos > 31)) {
        /*lint -e{904} Return statement is necessary to avoid multiple if loops and hence increase readability in case of argument check */
        return E_NOT_OK;
    }
    authenticPduBuffer[authenticPos].pduInfo = *info;
    authenticPduBuffer[authenticPos].pduId = id;

    authenticPos++;

    return E_OK;
}

/* @req SWS_SecOC_00113 SecOC_CancelTransmit */
Std_ReturnType SecOC_CancelTransmit( PduIdType id )
{
    Std_ReturnType status;
    //check if id exist or if function request failed
    if((PduR_SecOCCancelTransmit(id) == E_OK)){
        status = E_OK;
    } else {
        status = E_NOT_OK;
    }
    return status;
}


/* @req SWS_SecOC_00116 SecOC_AssociateKey */
Std_ReturnType SecOC_AssociateKey(uint8 keyID, const SecOC_KeyType* keyPtr) {

    Std_ReturnType status;
    if (keyID < (sizeof(keyList) / sizeof(keyList[0]))) {
        keyList[keyID] = *keyPtr;
        status = E_OK;
    } else {
        status = E_NOT_OK;
    }
    return status;
}

#if 0
//defined in specification but not supported in this module
Std_ReturnType SecOC_FreshnessValueRead( uint16 freshnessValueID, uint64* counterValue ) {
    //overwrite the counter value freshnessValueID with Secondary FreshnessValueID
    //freshnessValueID = secondaryFreshnessID
    //*counterValue = 1;
    return E_OK;
}
#endif

#if 0
//defined in specification but not supported in this module
Std_ReturnType SecOC_FreshnessValueWrite( uint16 freshnessValueID, uint64 counterValue )
{
    //overwrite the counter value freshnessValueID with Secondary FreshnessValueID
    //freshnessValueID = secondaryFreshnessID
    return E_OK;
}
#endif

#if 0
//defined in specification but not supported in this module
Std_ReturnType SecOC_VerifyStatusOverride( uint16 freshnessValueID, uint8 overrideStatus, uint8 numberOfMessagesToOverride ) {
    return 0;
}
#endif

/* @req SWS_SecOC_00042 */
/* @req SWS_SecOC_00124 SecOc_RxIndication */
/* @req SWS_SecOC_00078 Copy the complete Secured I-PDU into its own buffer after it returns from SecOc_RxIndication */
void SecOC_RxIndication( PduIdType RxPduId, const PduInfoType* PduInfoPtr )
{

    if( (PduInfoPtr == NULL) || (securedPos > 31)) {
        /*lint -e{904} Return statement is necessary to avoid multiple if loops and hence increase readability in case of argument check */
        return;
    }

    /* Put secured PDU is buffer */
    securedPduBuffer[securedPos].authenticIPDU.pduInfo.SduLength = PduInfoPtr->SduLength - 32;
    securedPduBuffer[securedPos].authenticIPDU.pduInfo.SduDataPtr = PduInfoPtr->SduDataPtr;

    securedPduBuffer[securedPos].pduId = RxPduId;
    memcpy(securedPduBuffer[securedPos].auth, &PduInfoPtr->SduDataPtr[PduInfoPtr->SduLength-32], 32);

    securedPos++;

}

#if 0
//defined in specification but not supported in this module
void SecOC_TpRxIndication( PduIdType id, Std_ReturnType result ) {
    // Copy the PDU to the buffer
}
#endif

/* @req SWS_SecOC_00126 SecOc_TxConfirmation */
void SecOC_TxConfirmation( PduIdType TxPduId )
{
    PduR_SecOCTxConfirmation(TxPduId);
}

#if 0
//defined in specification but not supported in this module
void SecOC_TpTxConfirmation( PduIdType id, Std_ReturnType result ) {

}
#endif

/* !req SWS_SecOC_00068 SecOC_TriggerTransmit shall copy the Secured I-PDU to the lower layer destination module */
/* !req SWS_SecOC_00127 SecOc_TriggerTransmit */
/* !req SWS_SecOC_00150 When no secured I-PDU is avalible, return E_NOT_OK */
Std_ReturnType SecOC_TriggerTransmit( PduIdType TxPduId, PduInfoType* PduInfoPtr )
{
    /* NOT IMPLEMENTED */
    return E_NOT_OK;
}

#if 0
//defined in specification but not supported in this module
BufReq_ReturnType SecOC_CopyRxData( PduIdType id, const PduInfoType* info, PduLengthType* bufferSizePtr )
{



    /**
    if(info->SduLength == 0){
        //SduDataPtr may be a NULL_PTR
        info = NULL_PTR;
    }

    securedPduBuffer[0].length = info->SduLength;
    securedPduBuffer[0].payload = info->SduDataPtr;

    //remaining available buffer after copying
    //bufferSizePtr = sizeof(securedPduBuffer)
    **/
    return BUFREQ_OK;
}
#endif

#if 0
//defined in specification but not supported in this module
BufReq_ReturnType SecOC_CopyTxData( PduIdType id, const PduInfoType* info, RetryInfoType* retry, PduLengthType* availableDataPtr )
{
/**
    if(availableDataPtr == NULL){
        return BUFREQ_E_BUSY;
    }

    if(info->SduLength == 0){
        info = NULL_PTR;
    }

    if(retry == NULL_PTR){
        //remove transmit data from buffer

    }

    authenticPduBuffer[0].pduId = id;
    authenticPduBuffer[0].pduInfo.SduDataPtr = info->SduDataPtr;
    authenticPduBuffer[0].pduInfo.SduLength = info->SduLength;

    //remaining bytes in Authentic buffer
    //availableDataPtr =
**/
    return BUFREQ_OK;
}
#endif

#if 0
//defined in specification but not supported in this module
BufReq_ReturnType SecOC_StartOfReception( PduIdType id, const PduInfoType* info, PduLengthType TpSduLength, PduLengthType* bufferSizePtr ) {
    return BUFREQ_OK;
}
#endif


/* @req SWS_SecOC_00131 SecOC_MainFunction */
/* @req SWS_SecOC_00132 If SecOC isnt initialized with SecOC_Init, then SecOC_MainFunction return */
/* @req SWS_SecOC_00133 The cycle time of SecOC_MainFunction is configured by the parameter SecOCMainFunctionPeriod */
/* @req SWS_SecOC_00134 If scheduled, SecOC shall check for new Authentic I-PDUs or Secured I-PDUs */
/* @req SWS_SecOC_00135 For each newly authenticated Authentic I-PDU, SecOC shall trigger the transmission of the Secured I-PDUby calling the PduR */
/* @req SWS_SecOC_00136 For each newly verified Secured I-PDU, SecOC shall pass the it along with PduR_SecOCRxIndication */
/* @req SWS_SecOC_00031 */
/* @req SWS_SecOC_00060 Calculate the Authenticator in the main function according to SWS_SecOC_00131*/
/* @req SWS_SecOC_00061 SecOC shall create the Secured I-PDU in the main function*/
/* @req SWS_SecOC_00079 The verification shall be processed in the scheduled main function*/
void SecOC_MainFunction( void ) {
    if (state == SECOC_UNINIT) {

#if (SecOCDevErrorDetect == STD_ON)
        Std_ReturnType Det_ReportError( uint16 ModuleId, uint8 InstanceId, uint8 ApiId, uint8 ErrorId);
        DET_REPORTERROR(SERVICE_ID_SECOC_MAIN_FUNCTION, SECOC_E_UNINIT);
#endif
        /*lint -e{904} Return statement is necessary in case of reporting a DET error */
        return;
    }

    //Check if there are new Secured I-PDU

    verifyLoop();

    //Check if there are new Authentic I-PDU

    authLoop();

}

//lint -restore
