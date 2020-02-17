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
//lint -w2 Static code analysis warning level
#include <string.h>
#include "Dcm.h"
#include "Dcm_Internal.h"
#include "MemMap.h"
#ifndef DCM_NOT_SERVICE_COMPONENT
#include "Rte_Dcm.h"
#endif

#if defined(USE_NVM)
#include "NvM.h"
#if (NVM_SET_RAM_BLOCK_STATUS_API == STD_ON)
#define ROE_USE_RAM_BLOCK_STATUS
#endif
#endif

#ifdef DCM_USE_SERVICE_RESPONSEONEVENT

#define DCM_MAX_NUMBER_OF_ROE_EVENTS 4       /* Max number of active ROE events */
#define DCM_ROE_INFINITE_WINDOW     0x02

#define ROE_INVALID_EVENT (uint16)0x0
#ifndef DCM_ROE_NVM_BLOCK_ID
#define DCM_ROE_NVM_BLOCK_ID 0
#endif
#ifndef DCM_ROE_NVM_BLOCK_SIZE
#define DCM_ROE_NVM_BLOCK_SIZE 0
#endif

#define INVALID_PDU 0xffff

/* This enum describes the internal statue of ROE. */
typedef enum {
    ROE_INIT,      /* Never been activated (hence no valid DCM_ROE_RxPduId)  */
    ROE_ACTIVE,    /* Active and may trigger responses */
    ROE_INACTIVE   /* Inactive but a valid DCM_ROE_RxPduId exists */
} DCM_ROE_InternalStateType;

typedef struct {
    const Dcm_DspDidType *didPtr;
    uint16 eventTypeRecord;
    uint16 bufferStart;
    uint16 bufferLength;   /* same as did length including did ids */
    uint8 serviceToRespondTo[DCM_ROE_MAX_SERVICE_TO_RESPOND_TO_LENGTH];
    uint8 serviceResponseLength;
} Dcm_ROE_EventType;

#if defined(USE_NVM)
typedef struct {
    uint16  eventTypeRecord;
    uint8   serviceToRespondTo[DCM_ROE_MAX_SERVICE_TO_RESPOND_TO_LENGTH];
    uint8   serviceResponseLength;
}Dcm_ROE_StoredEventType;

typedef struct {
    PduIdType               rxPdu;
    boolean                 active;
    Dcm_ROE_StoredEventType events[DCM_MAX_NUMBER_OF_ROE_EVENTS];
}Dcm_ROE_StorageType;

Dcm_ROE_StorageType Dcm_ROE_StorageBuffer = {.active = FALSE};

#if defined(ROE_USE_RAM_BLOCK_STATUS)
static boolean PendingNvmSetRamBlockStatus = FALSE;
#endif
#endif

static PduIdType DCM_ROE_RxPduId;
static DCM_ROE_InternalStateType Dcm_ROE_InternalState = ROE_INIT;
static uint8 Dcm_ROE_DataIdentifierBuffer[DCM_ROE_BUFFER_SIZE] = {0};
#if (DCM_ROE_INTERNAL_DIDS == STD_ON)
/* @req DCM523 */
static uint8 Dcm_ROE_DataIdentifierTempBuffer[DCM_ROE_BUFFER_SIZE] = {0};
#endif
static uint16 Dcm_ROE_FirstFreeBufferIndex = 0;
static Dcm_ROE_EventType Dcm_ROE_Events[DCM_MAX_NUMBER_OF_ROE_EVENTS];
static uint8 Dcm_ROE_NofEvents = 0;
static boolean protocolStartRequested = FALSE;

#if defined(ROE_USE_RAM_BLOCK_STATUS)
/**
 * Tries to set NvM block changed
 */
static Std_ReturnType setNvMBlockChanged(NvM_BlockIdType blockId) {
    NvM_RequestResultType requestResult = NVM_REQ_PENDING;
    Std_ReturnType ret = E_OK;
    if( 0 != blockId ) {
        ret = NvM_GetErrorStatus(blockId, &requestResult);
        if((E_OK == ret) && (requestResult != NVM_REQ_PENDING) ) {
            (void)NvM_SetRamBlockStatus( blockId, TRUE );
        } else {
            ret = E_NOT_OK;
        }
    }
    return ret;
}
#endif

#if defined(USE_NVM)
/**
 * Stores event and service to respond to in buffer written to NvM
 */
static Std_ReturnType storeROEDataIdentifierEvent(uint16 eventTypeRecord, const uint8* serviceToRespondTo, uint8 serviceToRespondToLength)
{
    Std_ReturnType ret = E_NOT_OK;
    uint8 storeIndex = 0;
    boolean done = FALSE;
    /* Check if already stored */
    for( uint8 i = 0; (i < DCM_MAX_NUMBER_OF_ROE_EVENTS) && !done; i++ ) {
        if( (ROE_INVALID_EVENT == Dcm_ROE_StorageBuffer.events[i].eventTypeRecord) || (eventTypeRecord == Dcm_ROE_StorageBuffer.events[i].eventTypeRecord) ) {
            storeIndex = i;
            ret = E_OK;
            if(eventTypeRecord == Dcm_ROE_StorageBuffer.events[i].eventTypeRecord) {
                done = TRUE;
            }
        }
    }
    if((E_OK == ret) && (ROE_INVALID_EVENT == Dcm_ROE_StorageBuffer.events[storeIndex].eventTypeRecord)) {
        Dcm_ROE_StorageBuffer.events[storeIndex].eventTypeRecord = eventTypeRecord;
        memcpy(Dcm_ROE_StorageBuffer.events[storeIndex].serviceToRespondTo, serviceToRespondTo, serviceToRespondToLength);
        Dcm_ROE_StorageBuffer.events[storeIndex].serviceResponseLength = serviceToRespondToLength;
    }
    return ret;
}

/**
 * Removes all stored event from buffer written to NvM
 */
static void removeROEDataIdentifierEvents(void)
{
#if defined(ROE_USE_RAM_BLOCK_STATUS)
    boolean eventRemoved = FALSE;
#endif
    for( uint8 i = 0; i < DCM_MAX_NUMBER_OF_ROE_EVENTS; i++ ) {
        if( ROE_INVALID_EVENT != Dcm_ROE_StorageBuffer.events[i].eventTypeRecord ) {
            Dcm_ROE_StorageBuffer.events[i].eventTypeRecord = ROE_INVALID_EVENT;
            memset(Dcm_ROE_StorageBuffer.events[i].serviceToRespondTo, 0, DCM_ROE_MAX_SERVICE_TO_RESPOND_TO_LENGTH);
            Dcm_ROE_StorageBuffer.events[i].serviceResponseLength = 0;
#if defined(ROE_USE_RAM_BLOCK_STATUS)
            eventRemoved = TRUE;
#endif
        }
    }
#if defined(ROE_USE_RAM_BLOCK_STATUS)
    if( eventRemoved && (E_OK != setNvMBlockChanged(DCM_ROE_NVM_BLOCK_ID)) ) {
        PendingNvmSetRamBlockStatus = TRUE;
    }
#endif
}

/**
 * Adds stored events on startup
 */
static void addStoredROEEvents(void)
{
    for( uint8 i = 0; i < DCM_MAX_NUMBER_OF_ROE_EVENTS; i++ ) {
        if( ROE_INVALID_EVENT != Dcm_ROE_StorageBuffer.events[i].eventTypeRecord ) {
            if( E_OK != Dcm_Arc_AddDataIdentifierEvent(Dcm_ROE_StorageBuffer.events[i].eventTypeRecord,
                    Dcm_ROE_StorageBuffer.events[i].serviceToRespondTo, Dcm_ROE_StorageBuffer.events[i].serviceResponseLength)) {
                Dcm_ROE_StorageBuffer.events[i].eventTypeRecord = ROE_INVALID_EVENT;
            }
        }
    }
}

/**
 * Validates the size of the NvM block
 */
#if !defined(HOST_TEST)
static void validateROEBlockSize(uint32 actual, uint32 expected) {
    if(actual != expected) {
        /* Ending up here means that the configured block size
         * for ROE events differs from the actual size of the
         * buffer (or DCM_ROE_NVM_BLOCK_SIZE is not defined). */
        while(1) {};
    }
}
#endif

/**
 * Stores active flag and pduId to NvM
 */
static void storeROEActive(boolean active, PduIdType rxPduId)
{
    if( active != Dcm_ROE_StorageBuffer.active ) {
        Dcm_ROE_StorageBuffer.active = active;
        Dcm_ROE_StorageBuffer.rxPdu = rxPduId;
#if defined(ROE_USE_RAM_BLOCK_STATUS)
        if( E_OK != setNvMBlockChanged(DCM_ROE_NVM_BLOCK_ID) ) {
            PendingNvmSetRamBlockStatus = TRUE;
        }
#endif
    }
}
#endif

/* Function: ReadDidData
 * Description: Reads DID data including all referred data
 */
static boolean ReadDidData(const Dcm_DspDidType *didPtr, uint8* buffer, uint16* length) {

    uint16 didIndex = 0;
    uint16 didStartIndex = 0;
    uint16 pendingDid = 0;
    uint16 pendingDidLen = 0;
    uint16 pendingSignalIndex = 0;
    uint16 didDataStartPos = 0;

    ReadDidPendingStateType pendingState = DCM_READ_DID_IDLE;

    PduInfoType pduTxData = { .SduDataPtr = buffer,
                              .SduLength = DCM_ROE_BUFFER_SIZE };


    Dcm_NegativeResponseCodeType responseCode = readDidData(didPtr,
                                                            &pduTxData,
                                                            length,
                                                            &pendingState,
                                                            &pendingDid,
                                                            &pendingSignalIndex,
                                                            &pendingDidLen,
                                                            &didIndex,
                                                            didStartIndex,
                                                            &didDataStartPos);

     if( DCM_E_POSITIVERESPONSE == responseCode ) {
         return TRUE;
     }
     else {
         return FALSE;
     }

}

/* Function: NotifyExternalEvents
 * Description: Calls the activate function for external events
 */
static Std_ReturnType NotifyExternalEvents(Dcm_RoeStateType state) {

    for (uint8 indx=0;indx<Dcm_ROE_NofEvents;indx++) {
        if ((Dcm_ROE_Events[indx].didPtr->DspDidRoeActivateFnc != NULL) &&
            (E_NOT_OK == Dcm_ROE_Events[indx].didPtr->DspDidRoeActivateFnc(Dcm_ROE_Events[indx].didPtr->DspDidRoeEventId, state))) {
                return E_NOT_OK;
        }
    }

    return E_OK;
}

/**
 * Adds preconfigured events to ROE list
 */
static void addPreconfiguredEvents(void)
{
    const Dcm_ArcROEDidPreconfigType *ROEPreConfig = NULL;
    /* Add preconfigured events */
    if( E_OK == Dcm_Arc_GetROEPreConfig(&ROEPreConfig) ) {
        if( (NULL != ROEPreConfig) && (NULL != ROEPreConfig->ROEDids)) {
            for( uint8 i = 0; i < ROEPreConfig->NofROEDids; i++ ) {
                if( E_OK != Dcm_Arc_AddDataIdentifierEvent(ROEPreConfig->ROEDids[i].DID,
                        ROEPreConfig->ROEDids[i].ServiceToRespondTo, ROEPreConfig->ROEDids[i].ServiceToRespondToLength)) {
                    /* Could not add event. */
                    DCM_DET_REPORTERROR(DCM_GLOBAL_ID, DCM_E_INTEGRATION_ERROR);
                }
            }
        } else {
            DCM_DET_REPORTERROR(DCM_GLOBAL_ID, DCM_E_INTEGRATION_ERROR);
        }
    }
}

/* Function: DCM_ROE_Init
 * Description: Initializes the module
 */
void DCM_ROE_Init(void) {
    Dcm_ROE_InternalState = ROE_INIT;
    Dcm_ROE_FirstFreeBufferIndex = 0;
    Dcm_ROE_NofEvents = 0;
    protocolStartRequested = FALSE;
    memset(Dcm_ROE_DataIdentifierBuffer, 0, sizeof(Dcm_ROE_DataIdentifierBuffer));
    memset(Dcm_ROE_Events, 0, sizeof(Dcm_ROE_Events));
#if (DCM_ROE_INTERNAL_DIDS == STD_ON)
    memset(Dcm_ROE_DataIdentifierTempBuffer, 0, sizeof(Dcm_ROE_DataIdentifierTempBuffer));
#endif
    /* Add preconfigured events */
    addPreconfiguredEvents();

#if defined(USE_NVM)

#if !defined(HOST_TEST)
    validateROEBlockSize((uint32)DCM_ROE_NVM_BLOCK_SIZE, (uint32)sizeof(Dcm_ROE_StorageBuffer));
#endif
    /* Add events stored in NvRam */
    addStoredROEEvents();
#endif
}

/**
 * Function called first main function after init to allow ROE to request
 * protocol start request
 */
void DcmRoeCheckProtocolStartRequest(void)
{
#if defined(USE_NVM)
    /* Check if ROE was stored as active. If it was, request start of protocol. */
    if( Dcm_ROE_StorageBuffer.active && (INVALID_PDU != Dcm_ROE_StorageBuffer.rxPdu)) {
        uint16 srcAddr;
        Dcm_ProtocolType protocolId;
        Dcm_ProtocolAddrTypeType reqType;
        if( E_OK == Arc_DslGetRxConnectionParams(Dcm_ROE_StorageBuffer.rxPdu, &srcAddr, &reqType, &protocolId) ) {
            if( E_OK == DcmRequestStartProtocol(DCM_REQ_ROE, DCM_DEFAULT_SESSION, protocolId, (uint8)srcAddr, FALSE) ) {
                protocolStartRequested = TRUE;
            } else {
                DCM_DET_REPORTERROR(DCM_GLOBAL_ID, DCM_E_UNEXPECTED_RESPONSE);
            }
        }
    }
#endif
}

/**
 * Notification that a requested protocol has been started (or not)
 */
void DcmRoeProtocolStartNotification(boolean started)
{
    if( protocolStartRequested ) {
        if( started ) {
            uint8 dummySduData[8];
            PduInfoType pduRxData;
            pduRxData.SduDataPtr = dummySduData;
            (void)DCM_ROE_Start(0, 0, Dcm_ROE_StorageBuffer.rxPdu, &pduRxData);
            protocolStartRequested = FALSE;
        }
    } else {
        /* Have not requested a start.. */
        DCM_DET_REPORTERROR(DCM_GLOBAL_ID, DCM_E_UNEXPECTED_EXECUTION);
    }
}

/* Function: DCM_ROE_IsActive
 * Description: Returns true if the ROE function is active otherwise false
 */
boolean DCM_ROE_IsActive(void) {
    return (Dcm_ROE_InternalState == ROE_ACTIVE);
}

/* Function: DCM_ROE_Start
 * Description: Handles the ROE start subfunction and compiles a response
 */
Dcm_NegativeResponseCodeType DCM_ROE_Start(uint8 storageState, uint8 eventWindowTime, PduIdType rxPduId, PduInfoType *pduTxData) {

    /* @req Dcm127 */
    DCM_ROE_RxPduId = rxPduId;
    Dcm_ROE_InternalState = ROE_INACTIVE; /* Got proper RxPduId */

    /* @req DCM525 The Dcm_RestartROE() calls the activate functions for external events */
    if (E_OK == Dcm_RestartROE()) {
        pduTxData->SduDataPtr[1] = 0x05 | storageState;
        pduTxData->SduDataPtr[2] = 0;               /* numberOfIdentifiedEvents (N/A) */
        pduTxData->SduDataPtr[3] = eventWindowTime; /* Mirror event time, shall not be used */
        pduTxData->SduLength = 4;
        if(storageState != 0) {
            storeROEActive(TRUE, DCM_ROE_RxPduId);
        }
        return DCM_E_POSITIVERESPONSE;
    }
    else
    {
        /* @req DCM679 */
        return DCM_E_REQUESTOUTOFRANGE;
    }


}


/* Function: DCM_ROE_Stop
 * Description: Handles the ROE stop subfunction and compiles a response
 */
Dcm_NegativeResponseCodeType DCM_ROE_Stop(uint8 storageState, uint8 eventWindowTime, PduInfoType *pduTxData) {

    /* @req DCM526 The Dcm_StopROE() calls the activate functions for external events */
    if (E_OK == Dcm_StopROE()) {
        if(0 != storageState) {
            storeROEActive(FALSE, INVALID_PDU);
        }
        pduTxData->SduDataPtr[1] = 0x00 | storageState;
        pduTxData->SduDataPtr[2] = 0;               /* numberOfIdentifiedEvents (N/A) */
        pduTxData->SduDataPtr[3] = eventWindowTime; /* Mirror event time, shall not be used */
        pduTxData->SduLength = 4;
        return DCM_E_POSITIVERESPONSE;
    }
    else {
        /* @req DCM679 */
        return DCM_E_REQUESTOUTOFRANGE;
    }
}



/* Function: DCM_ROE_ClearEventList
 * Description: Handles the ROE clear event list  and compiles a response
 */
Dcm_NegativeResponseCodeType DCM_ROE_ClearEventList(uint8 storageState, uint8 eventWindowTime, PduInfoType *pduTxData) {

    if ((Dcm_ROE_InternalState == ROE_ACTIVE) && (E_NOT_OK == Dcm_StopROE())) {
        /* @req DCM679 */
        return DCM_E_REQUESTOUTOFRANGE;
    }
    else {
        Dcm_ROE_NofEvents = 0;
#if defined(USE_NVM)
        removeROEDataIdentifierEvents();
#endif
        pduTxData->SduDataPtr[1] = 0x06 | storageState;
        pduTxData->SduDataPtr[2] = 0;               /* numberOfIdentifiedEvents (N/A) */
        pduTxData->SduDataPtr[3] = eventWindowTime; /* Mirror event time, shall not be used */
        pduTxData->SduLength = 4;
        return DCM_E_POSITIVERESPONSE;
    }
}


/* Function: DCM_ROE_GetEventList
 * Description: Handles the ROE get event list  and compiles a response
 */
Dcm_NegativeResponseCodeType DCM_ROE_GetEventList(uint8 storageState, PduInfoType *pduTxData) {

    uint8 responseByteIndx = 1;

    pduTxData->SduDataPtr[responseByteIndx++] = 0x04 | storageState; /* eventType */
    pduTxData->SduDataPtr[responseByteIndx++] = (Dcm_ROE_InternalState == ROE_ACTIVE) ? Dcm_ROE_NofEvents : 0;

    if( Dcm_ROE_InternalState == ROE_ACTIVE ) {
        for (uint8 i = 0; i < Dcm_ROE_NofEvents; i++) {

            /* Check if the data fits in the response */
            if (pduTxData->SduLength <= (responseByteIndx + 4 + Dcm_ROE_Events[i].serviceResponseLength)) {
                return DCM_E_RESPONSETOOLONG;
            }

            pduTxData->SduDataPtr[responseByteIndx++] = 0x03; /* eventTypeOfActiveEvent */
            pduTxData->SduDataPtr[responseByteIndx++] = DCM_ROE_INFINITE_WINDOW;              /* eventWindowTime */
            pduTxData->SduDataPtr[responseByteIndx++] = (uint8)(Dcm_ROE_Events[i].eventTypeRecord >> 8);   /* eventTypeRecord */
            pduTxData->SduDataPtr[responseByteIndx++] = (uint8)(Dcm_ROE_Events[i].eventTypeRecord & 0xFF); /* eventTypeRecord */

            for (uint8 j=0;j<Dcm_ROE_Events[i].serviceResponseLength;j++) {
                pduTxData->SduDataPtr[responseByteIndx++] = Dcm_ROE_Events[i].serviceToRespondTo[j];
            }
        }
    }
    pduTxData->SduLength = responseByteIndx;
    return DCM_E_POSITIVERESPONSE;
}
/* Function: checkROEPreCondition
 * Description: Checks preconditions for DCM_ROE_AddDataIdentifierEvent
 */
static Dcm_NegativeResponseCodeType checkROEPreCondition(uint8 eventWindowTime,
                                                    uint16 eventTypeRecord,
                                                    const uint8* serviceToRespondTo,
                                                    uint8 serviceToRespondToLength,
                                                    PduInfoType *pduTxData)
{
    /* Only supporting serviceToRespondTo = ReadDataByIdentifier,
     *                 eventWindowTime = infinite
     *                 the response service must at least contain one byte
     */
    if ((serviceToRespondToLength < 1) || (serviceToRespondTo[0] != SID_READ_DATA_BY_IDENTIFIER) ||
        (eventWindowTime != DCM_ROE_INFINITE_WINDOW)) {
        return DCM_E_REQUESTOUTOFRANGE;
    }

    /* Check that the event buffer is not full and that the service length is within range */
    if ((Dcm_ROE_NofEvents >= DCM_MAX_NUMBER_OF_ROE_EVENTS) ||
        (serviceToRespondToLength > DCM_ROE_MAX_SERVICE_TO_RESPOND_TO_LENGTH)) {
        return DCM_E_REQUESTOUTOFRANGE;
    }

    /* Add the event to the event list */
    if (!lookupNonDynamicDid(eventTypeRecord, &Dcm_ROE_Events[Dcm_ROE_NofEvents].didPtr)) {
        return DCM_E_REQUESTOUTOFRANGE;
    }

    /* Check that the eventTypeRecord isn't already in the list */
    for (uint8 indx=0; indx<Dcm_ROE_NofEvents;indx++) {
        if (Dcm_ROE_Events[indx].eventTypeRecord == eventTypeRecord) {
            return DCM_E_REQUESTOUTOFRANGE;
        }
    }

    /* Check that a positive response will fit in the tx buffer */
    if( (NULL != pduTxData) && ((serviceToRespondToLength + 5u) > pduTxData->SduLength) ) {
        return DCM_E_RESPONSETOOLONG;
    }

    return DCM_E_POSITIVERESPONSE;
}

/* Function: DCM_ROE_AddDataIdentifierEvent
 * Description: Handles the ROE on change of data identifier  and compiles a response
 */
Dcm_NegativeResponseCodeType DCM_ROE_AddDataIdentifierEvent(uint8 eventWindowTime,
                                                            uint8 storageState,
                                                            uint16 eventTypeRecord,
                                                            const uint8* serviceToRespondTo,
                                                            uint8 serviceToRespondToLength,
                                                            PduInfoType *pduTxData) {

    /* @req Dcm522 */

    uint8 responseByteIndx = 1;
    uint16 length = 0;
    uint16 nofDids = 0;

    /* Check preconditions */
    Dcm_NegativeResponseCodeType NRC = checkROEPreCondition(eventWindowTime,eventTypeRecord,serviceToRespondTo,serviceToRespondToLength,pduTxData);
    if( NRC != DCM_E_POSITIVERESPONSE ) {
        return NRC;
    }

    /* Allocate space in the buffer */
    Dcm_ROE_Events[Dcm_ROE_NofEvents].bufferStart = Dcm_ROE_FirstFreeBufferIndex;

    getDidLength(Dcm_ROE_Events[Dcm_ROE_NofEvents].didPtr, &length, &nofDids);
    length += (nofDids * 2); /* Add the lenght for the dids */

    Dcm_ROE_Events[Dcm_ROE_NofEvents].bufferLength = length;
    Dcm_ROE_FirstFreeBufferIndex += length;

    Dcm_ROE_Events[Dcm_ROE_NofEvents].eventTypeRecord = eventTypeRecord;
    memcpy(Dcm_ROE_Events[Dcm_ROE_NofEvents].serviceToRespondTo, serviceToRespondTo, serviceToRespondToLength);
    Dcm_ROE_Events[Dcm_ROE_NofEvents].serviceResponseLength = serviceToRespondToLength;
    Dcm_ROE_NofEvents++;

#if defined(USE_NVM)
    /* @req DCM709 */
    if( 0 != storageState ) {
        if(E_OK != storeROEDataIdentifierEvent(eventTypeRecord, serviceToRespondTo, serviceToRespondToLength) ) {
            DCM_DET_REPORTERROR(DCM_GLOBAL_ID, DCM_E_UNEXPECTED_EXECUTION);
        } else {
#if defined(ROE_USE_RAM_BLOCK_STATUS)
            if( E_OK != setNvMBlockChanged(DCM_ROE_NVM_BLOCK_ID) ) {
                PendingNvmSetRamBlockStatus = TRUE;
            }
#endif
        }
    }
#endif

    /* Put together a response */
    if (pduTxData != NULL ) {
        pduTxData->SduDataPtr[responseByteIndx++] = 0x03 | storageState;            /* eventType */
        pduTxData->SduDataPtr[responseByteIndx++] = 0;                              /* numberOfIdentifiedEvents */
        pduTxData->SduDataPtr[responseByteIndx++] = eventWindowTime;                /* eventTimeWindow */
        pduTxData->SduDataPtr[responseByteIndx++] = (eventTypeRecord >> 8)&0xFF;    /* eventTypeRecord High byte */
        pduTxData->SduDataPtr[responseByteIndx++] = eventTypeRecord&0xFF;           /* eventTypeRecord Low byte  */

        for (uint8 i=0;i<serviceToRespondToLength;i++) {
            pduTxData->SduDataPtr[responseByteIndx++] = serviceToRespondTo[i];      /* service to respond to  */
        }

        pduTxData->SduLength = responseByteIndx;
    }

    return DCM_E_POSITIVERESPONSE;
}



/* Function: DCM_ROE_PollDataIdentifiers
 * Description:This function is used for internally managed data identifiers, it polls each identifier and
 * sends a response if it has changed
 */
#if DCM_ROE_INTERNAL_DIDS == STD_ON
void DCM_ROE_PollDataIdentifiers(void) {

    /* @req DCM524 */

    /* Used to save the current event being checked, used to not cause that a single event is being sent all the
     * time if changed all the time
     */
    static uint16 indx = 0;

    if (Dcm_ROE_InternalState == ROE_ACTIVE) {

        if (indx >= Dcm_ROE_NofEvents) {
            indx = 0;
        }

        for (;indx<Dcm_ROE_NofEvents;indx++) {

            uint16 startIndex = 0;

            if ((Dcm_ROE_Events[indx].didPtr->DspDidRoeActivateFnc != NULL) ||
                !ReadDidData(Dcm_ROE_Events[indx].didPtr, Dcm_ROE_DataIdentifierTempBuffer, &startIndex)) {
                continue; /* A failing read will not cause a diagnostic response, the old data will be kept  */
            }

            if  ( memcmp(Dcm_ROE_DataIdentifierTempBuffer,
                         &Dcm_ROE_DataIdentifierBuffer[Dcm_ROE_Events[indx].bufferStart],
                         startIndex) != 0) {
                /* The data has changed trigger an internal diagnostic request */
                /* @req DCM128 */
                (void)DslInternal_ResponseOnOneEvent(DCM_ROE_RxPduId,
                                                    Dcm_ROE_Events[indx].serviceToRespondTo,
                                                    Dcm_ROE_Events[indx].serviceResponseLength);
                break;
            }
        }
    }
}
#endif


/* Function: Dcm_TriggerOnEvent
 * Description: Triggers an ROE response for the specified id. It is only possible
 * to trigger responses for external events. Called from the application using a service
 * interface.
 */
Std_ReturnType Dcm_TriggerOnEvent(uint8 RoeEventId) {

    /** @req DCM521 Returning E_NOT_OK when an event is not triggered (other cases than invalid id) */
    uint8 indx = 0;

    if (Dcm_ROE_InternalState != ROE_ACTIVE) {
        return E_NOT_OK;
    }

    for (indx = 0; indx < Dcm_ROE_NofEvents; indx++) {
        if ((Dcm_ROE_Events[indx].didPtr->DspDidRoeActivateFnc != NULL) && (Dcm_ROE_Events[indx].didPtr->DspDidRoeEventId == RoeEventId)) {
            /* If the request buffer is full, this function will return E_NOT_OK */
            /* @req DCM582 *//* @req DCM128 *//* @req DCM129 */
            return DslInternal_ResponseOnOneEvent(DCM_ROE_RxPduId,
                                                     Dcm_ROE_Events[indx].serviceToRespondTo,
                                                     Dcm_ROE_Events[indx].serviceResponseLength);
        }
    }

    if (indx == Dcm_ROE_NofEvents) {
        /* Did not find and ROE event with matching id */
        return E_NOT_OK;
    }

    return E_OK;

}


/* Function: Dcm_StopROE
 * Description: Stops the ROE. Called from the application using a service interface.
 */
Std_ReturnType Dcm_StopROE(void) {

    /** @req DCM730 */

    Std_ReturnType status = NotifyExternalEvents(DCM_ROE_UNACTIVE);

    if (status == E_OK) {
        /* Not allowed to leave ROE_INIT since no RxPduId is set */
        Dcm_ROE_InternalState = (Dcm_ROE_InternalState == ROE_INIT) ? ROE_INIT : ROE_INACTIVE;
    }

    return status;
}


/* Function: Dcm_RestartROE
 * Description: Restarts the ROE. It must have been initially started by a diagnostic request
 * so that the RxPduId is saved and may be used when sending a response. Called from the
 * application using a service interface.
 */
Std_ReturnType Dcm_RestartROE(void) {

    /** @req DCM731 *//** @req DCM714 */

    Std_ReturnType status;

    /* Cannot start if not been start by a diagnostic request since no valid RxPduId */
    if (Dcm_ROE_InternalState == ROE_INIT) {
        return E_NOT_OK;
    }

    status = NotifyExternalEvents(DCM_ROE_ACTIVE);

    if (status == E_OK) {

        /* Read data that will be used for comparison in the poll routine */
        for (uint8 indx=0;indx<Dcm_ROE_NofEvents;indx++) {

            uint16 startIndex = 0;

            if ((Dcm_ROE_Events[indx].didPtr->DspDidRoeActivateFnc == NULL)) {
                (void)ReadDidData(Dcm_ROE_Events[indx].didPtr, &Dcm_ROE_DataIdentifierBuffer[Dcm_ROE_Events[indx].bufferStart], &startIndex);
            }
        }

        /* @req Dcm714 */
        Dcm_ROE_InternalState =  ROE_ACTIVE;
    }

    return status;
}



/* Function: Dcm_Arc_AddDataIdentifierEvent
 * Description: This function is used to add data identifer events internally e.g at start up.
 */
Std_ReturnType Dcm_Arc_AddDataIdentifierEvent(uint16 eventTypeRecord,
                                             const uint8* serviceToRespondTo,
                                             uint8 serviceToRespondToLength) {

    if (DCM_ROE_AddDataIdentifierEvent(DCM_ROE_INFINITE_WINDOW,
                                       0,
                                       eventTypeRecord,
                                       serviceToRespondTo,
                                       serviceToRespondToLength,
                                       NULL) != DCM_E_POSITIVERESPONSE) {
        return E_NOT_OK;
    }
    else {
        return E_OK;
    }
}

/**
 * Main service function for ROE
 */
void Dcm_ROE_MainFunction(void)
{
#if defined(ROE_USE_RAM_BLOCK_STATUS)
    if(PendingNvmSetRamBlockStatus) {
        if( E_OK == setNvMBlockChanged(DCM_ROE_NVM_BLOCK_ID) ) {
            PendingNvmSetRamBlockStatus = FALSE;
        }
    }
#endif
}

#if defined(USE_NVM)
/**
 * Init block callback for use by NvM
 */
void Dcm_Arc_InitROEBlock(void)
{
    Dcm_ROE_StorageBuffer.active = FALSE;
    Dcm_ROE_StorageBuffer.rxPdu = INVALID_PDU;
    removeROEDataIdentifierEvents();
}
#endif

#endif


/* End of file */
