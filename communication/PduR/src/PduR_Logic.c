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

//lint -emacro(904,PDUR_VALIDATE_INITIALIZED,PDUR_VALIDATE_PDUPTR,PDUR_VALIDATE_PDUID) //904 PC-Lint exception to MISRA 14.7 (validate macros).
//lint -emacro(506,PDUR_VALIDATE_PDUPTR,PDUR_VALIDATE_PDUID) //PC-Lint exception Misra 13.7, 14.1 Allow configuration variables in boolean expression.
//lint -emacro(774,PDUR_VALIDATE_PDUPTR,PDUR_VALIDATE_PDUID) //PC-Lint exception Misra 13.7, 14.1 Allow configuration variables in boolean expression.


/* Only static routing, hard to tag at a specific line.
 * @req PDUR162  */


#include "PduR.h"

#include <string.h>
#include "debug.h"
#include "SchM_PduR.h"
#if (PDUR_DEV_ERROR_DETECT == STD_ON)
#include "Det.h"
#endif
#if defined(USE_DEM)
#include "Dem.h"
#endif
#if PDUR_LDCOM_SUPPORT == STD_ON
#include "LdCom.h"
#endif

#if PDUR_ZERO_COST_OPERATION == STD_OFF

#define PduRTpBuffer(_id) (&PduR_RamBufCfg.TpBuffers[_id])
#define PduRTpRouteBuffer(_id) (PduR_RamBufCfg.TpRouteBuffers[_id])

#define HAS_BUFFER_STATUS(_pduId, _status)  ((_pduId < PduR_RamBufCfg.NTpRouteBuffers) && (PduRTpRouteBuffer(_pduId) != NULL) && (PduRTpRouteBuffer(_pduId)->status == _status))
#define REPORT_BUFFER_ERROR(_serviceId) PDUR_DET_REPORTERROR(PDUR_MODULE_ID, PDUR_INSTANCE_ID, _serviceId, PDUR_E_BUFFER_ERROR);

#define PDU_MAX_ROUTING_PATH   8192 /* Out of 16 bits of PduId we use 13 for representing a PDUR_PDU_ID_XXX. Only if a path contains
a gateway to multiple Tp destinations then the upper 3 bits are used for representing the GW destination index */
#define PDUR_PDU_ID_MASK   8191 // Mask for extraction of lower 13 bits

// Static function prototypes
static boolean PduR_ARC_HasTpModuleDest(const PduRDestPdu_type * const *destPdus);
static const PduRDestPdu_type * PduR_ARC_FindUPDest(const PduRDestPdu_type * const *destPdus);
static BufReq_ReturnType PduR_ARC_AllocateBuffer(PduIdType PduId, PduLengthType TpSduLength);
static BufReq_ReturnType PduR_ARC_CheckBufferStatus(PduIdType PduId, uint16 length);
static BufReq_ReturnType PduR_ARC_ReleaseRxBuffer(PduIdType PduId);
static BufReq_ReturnType PduR_ARC_ReleaseTxBuffer(PduIdType PduId);
static void PduR_ARC_RxIndicationDirect(const PduRDestPdu_type * destination, const PduInfoType *PduInfo);
static void PduR_ARC_RxIndicationTT(const PduRDestPdu_type * destination, const PduInfoType *PduInfo);
static inline void calculateMinBufferSize(PduLengthType *minBufSize, const PduLengthType *avblBufSize);

#if (PDUR_MAX_NOF_ROUTING_PATH_GROUPS > 0)
static boolean PdurRoutingGroupEnabled[PDUR_MAX_NOF_ROUTING_PATH_GROUPS];
#endif

static BufReq_ReturnType PduR_ARC_AllocateBuffer(PduIdType PduId, PduLengthType TpSduLength) {
    BufReq_ReturnType retVal;

    SchM_Enter_PduR_EA_0();//Disable interrupts

    retVal = BUFREQ_BUSY;
    for (uint8 i = 0; i < PduR_RamBufCfg.NTpBuffers; i++) {
        if (PduRTpBuffer(i)->status == PDUR_BUFFER_FREE) {
            if (PduRTpBuffer(i)->bufferSize < TpSduLength) {
                retVal = BUFREQ_OVFL;
            } else {
                PduRTpRouteBuffer(PduId) = PduRTpBuffer(i);
                PduRTpBuffer(i)->pduInfoPtr->SduLength = TpSduLength;
                PduRTpRouteBuffer(PduId)->status = PDUR_BUFFER_RX_READY;
                memset(PduRTpRouteBuffer(PduId)->txByteCount,0, sizeof(PduLengthType)*PDUR_MAX_GW_DESTINATIONS);
                PduRTpRouteBuffer(PduId)->rxByteCount = 0;
                retVal = BUFREQ_OK;
                break;
            }
        }
    }

    SchM_Exit_PduR_EA_0();//Enable interrupts
    return retVal;
}

static BufReq_ReturnType PduR_ARC_CheckBufferStatus(PduIdType PduId, uint16 length) {
    BufReq_ReturnType retVal = BUFREQ_BUSY;

    if (PduRTpRouteBuffer(PduId) == NULL) {
        retVal = BUFREQ_NOT_OK;
    }
    else if ((length > (PduRTpRouteBuffer(PduId)->pduInfoPtr->SduLength - PduRTpRouteBuffer(PduId)->rxByteCount))){
        /* @req PDUR687 */
        retVal = BUFREQ_NOT_OK;
    }
    else if (PduRTpRouteBuffer(PduId)->status == PDUR_BUFFER_RX_READY) {

        PduRTpRouteBuffer(PduId)->status = PDUR_BUFFER_RX_BUSY;
        PduRTpRouteBuffer(PduId)->nAcc = 0;
        retVal = BUFREQ_OK;
    }
    else if ((PduRTpRouteBuffer(PduId)->status == PDUR_BUFFER_TX_BUSY)
            || (PduRTpRouteBuffer(PduId)->status == PDUR_BUFFER_RX_BUSY)) {

        retVal = BUFREQ_OK;
    }else{
        /*Do Nothing */
    }

    return retVal;
}

static BufReq_ReturnType PduR_ARC_ReleaseRxBuffer(PduIdType PduId) {
    BufReq_ReturnType retVal;
    if (PduId >= PduR_RamBufCfg.NTpRouteBuffers) {
        retVal = BUFREQ_NOT_OK;
    } else if (PduRTpRouteBuffer(PduId) == NULL) {
        retVal = BUFREQ_OK;
    } else if (PduRTpRouteBuffer(PduId)->status == PDUR_BUFFER_ALLOCATED_FROM_UP_MODULE) {
        PduRTpRouteBuffer(PduId)->status = PDUR_BUFFER_NOT_ALLOCATED_FROM_UP_MODULE;
        retVal = BUFREQ_BUSY;
    } else if (PduRTpRouteBuffer(PduId)->status == PDUR_BUFFER_RX_BUSY) {
        PduRTpRouteBuffer(PduId)->status = PDUR_BUFFER_TX_READY;
        retVal = BUFREQ_BUSY;
    } else {
        retVal = BUFREQ_OK;
    }
    return retVal;
}

static BufReq_ReturnType PduR_ARC_ReleaseTxBuffer(PduIdType PduId) {
    BufReq_ReturnType retVal = BUFREQ_NOT_OK;
    if ((PduRTpRouteBuffer(PduId) != NULL) &&
        (PduRTpRouteBuffer(PduId)->status == PDUR_BUFFER_TX_BUSY)) {
        PduRTpRouteBuffer(PduId)->status = PDUR_BUFFER_FREE;
        PduRTpRouteBuffer(PduId)->nAcc = 0;
        PduRTpRouteBuffer(PduId) = NULL;
        retVal = BUFREQ_OK;
    }
    return retVal;
}

Std_ReturnType PduR_ARC_Transmit(PduIdType PduId, const PduInfoType* PduInfo, uint8 serviceId) {
    PDUR_VALIDATE_INITIALIZED(serviceId,E_NOT_OK);
    PDUR_VALIDATE_PDUPTR(serviceId, PduInfo, E_NOT_OK);
    PDUR_VALIDATE_PDUID(serviceId, PduId, E_NOT_OK);

    /* @req PDUR629 */
    /* @req PDUR218 */
    /* @req PDUR745 */
    /* @req PDUR625 */
    /* @req PDUR626 */
    /* @req PDUR675 */
    /* @req PDUR718 Same as 675? */
    /* @req PDUR772 Same as 675/718? */
    /* @req PDUR634 */
    /* @req PDUR432 */

    Std_ReturnType ret;
    ret = E_OK;
    PduRRouteStatusType status;
    /* If at least one is rejected we should return E_NOT_OK,
     * if all are disabled we should return E_NOT_OK */
    /* @req PDUR714 */
    /* @req PDUR717 */
    PduRRouteStatusType totalStatus = PDUR_E_DISABLED;
    const PduRRoutingPath_type *route = PduRConfig->RoutingPaths[PduId];
    for (uint8 i = 0; route->PduRDestPdus[i] != NULL; i++) {
        const PduRDestPdu_type * destination = route->PduRDestPdus[i];
        status = PduR_ARC_RouteTransmit(destination, PduInfo);
        if( (PDUR_E_REJECTED == totalStatus) || (PDUR_E_REJECTED == status)) {
            totalStatus = PDUR_E_REJECTED;
        } else if(PDUR_E_OK == status) {
            totalStatus = PDUR_E_OK;
        } else {
            /* Rejected */
        }

    }
    if( (PDUR_E_REJECTED == totalStatus) || (PDUR_E_DISABLED == totalStatus) ) {
        ret = E_NOT_OK;
    } else {
        ret = E_OK;
    }
    return ret;
}

static void PduR_ARC_RxIndicationTT(const PduRDestPdu_type * destination, const PduInfoType *PduInfo) {

    /* @req PDUR160 */
    /* @req PDUR0661 */
    if(TRUE == PduRRoutingPathEnabled(destination) ) {
        memcpy(PduR_RamBufCfg.TxBuffers[destination->TxBufferId].DataPtr, PduInfo->SduDataPtr, PduInfo->SduLength);
    }

    if (PDUR_E_OK != PduR_ARC_RouteTransmit(destination, PduInfo)) {
        PDUR_DET_REPORTERROR(PDUR_MODULE_ID, 0, PDUR_SERVICEID_CANIFRXINDICATION, PDUR_E_PDU_INSTANCES_LOST);
    }
    /*
    // This is a gateway request which uses trigger transmit data provision. PDUR255
    if (destination->TxBufferRef->TxConfP) { // Transfer confirmation pending.
        // Enqueue the new I-PDU. This will flush the buffer if it is full according to the buffer specification.
        PduR_BufferQueue(destination->TxBufferRef, PduInfo->SduDataPtr);
    }

    if (destination->TxBufferRef->TxConfP) { // No transfer confirmation pending (anymore).
        uint8 val[PduInfo->SduLength];
        PduInfoType NewPduInfo = {
            .SduDataPtr = val,
            .SduLength = PduInfo->SduLength
        };
        PduR_BufferDeQueue(destination->TxBufferRef, val);
        PduR_BufferQueue(destination->TxBufferRef, PduInfo->SduDataPtr);
        retVal = PduR_ARC_RouteTransmit(destination, &NewPduInfo);
        if (retVal == E_OK) {
            setTxConfP(destination->TxBufferRef);
        }
    }
    */
}

static void PduR_ARC_RxIndicationDirect(const PduRDestPdu_type * destination, const PduInfoType *PduInfo) {

    /* @req PDUR160 */
    /* @req PDUR0745 */


    PduRRouteStatusType retVal = PduR_ARC_RouteTransmit(destination, PduInfo);
    if (retVal != PDUR_E_OK) {
        PDUR_DET_REPORTERROR(PDUR_MODULE_ID, 0, PDUR_SERVICEID_CANIFRXINDICATION, PDUR_E_PDU_INSTANCES_LOST);
    }
}

static void PduR_ARC_RxIndicationWithUpBuffer(PduIdType PduId, const PduInfoType* PduInfo, uint8 serviceId) {
    const PduRRoutingPath_type *route = PduRConfig->RoutingPaths[PduId];
    const PduRDestPdu_type * destination;

    if (PduRTpRouteBuffer(PduId) == NULL) {
        REPORT_BUFFER_ERROR(serviceId);
        /*lint -e{904} Return statement is necessary in case of reporting a DET error */
        return ;
    }
    // Save current buffer
    const PduInfoType *const upBuffer = PduRTpRouteBuffer(PduId)->pduInfoPtr;
    BufReq_ReturnType retVal = PduR_ARC_ReleaseRxBuffer(PduId);
    if (retVal != BUFREQ_BUSY) {
        REPORT_BUFFER_ERROR(serviceId);
        /*lint -e{904} Return statement is necessary in case of reporting a DET error */
        return;
    }

    // Get a new PduR buffer
    retVal = PduR_ARC_AllocateBuffer(PduId, upBuffer->SduLength);
    if (retVal != BUFREQ_OK) {
        REPORT_BUFFER_ERROR(serviceId);
        /*lint -e{904} Return statement is necessary in case of reporting a DET error */
        return;
    }

    // Copy the data from old buffer to new.
    PduRTpRouteBuffer(PduId)->status = PDUR_BUFFER_RX_BUSY;
    memcpy(PduRTpRouteBuffer(PduId)->pduInfoPtr->SduDataPtr, upBuffer->SduDataPtr, PduRTpRouteBuffer(PduId)->pduInfoPtr->SduLength);

    // notify upper module
    destination = PduR_ARC_FindUPDest(route->PduRDestPdus);
    PduR_ARC_RouteRxIndication(destination, PduInfo);

    // Then first transmit to all other destinations.
    for (uint8 i = 0; route->PduRDestPdus[i] != NULL; i++) {
        const PduRDestPdu_type * d = route->PduRDestPdus[i];
        PduRRouteStatusType status = PduR_ARC_RouteTransmit(d, PduRTpRouteBuffer(PduId)->pduInfoPtr);
        if(status!=PDUR_E_OK){
            // IMPROVEMENT: Add DET error
        }
    }

}

void PduR_ARC_TpRxIndication(PduIdType PduId, NotifResultType Result, uint8 serviceId) {
    PDUR_VALIDATE_INITIALIZED(serviceId);
    PDUR_VALIDATE_PDUID(serviceId, PduId);

#if 0
    PduInfoType PduInfo = {
        .SduDataPtr = &Result,
        .SduLength = 0 // To fix PC-Lint 785
    };
#endif

    if(Result != NTFRSLT_OK){
        // There was an error in the lower layer while receiving the PDU.
        // Release any buffers and notify upper layers
        (void)PduR_ARC_ReleaseRxBuffer(PduId);
        for (uint8 i = 0; PduRConfig->RoutingPaths[PduId]->PduRDestPdus[i] != NULL; i++) {
            const PduRDestPdu_type * destination = PduRConfig->RoutingPaths[PduId]->PduRDestPdus[i];
            if (PduR_IsUpModule(destination->DestModule)) {
                PduR_ARC_RouteTpRxIndication(destination, Result );
            }else if (PduRTpRouteBuffer(PduId) == NULL) {
                REPORT_BUFFER_ERROR(serviceId);
            }
            else {
                /*Gateway on Tp */
                /* @req PDUR689 */
                PduRTpRouteBuffer(PduId)->status = PDUR_BUFFER_FREE;
                PduRTpRouteBuffer(PduId)->nAcc = 0;
                PduRTpRouteBuffer(PduId) = NULL;
            }
        }

    } else {
        for (uint8 i = 0; PduRConfig->RoutingPaths[PduId]->PduRDestPdus[i] != NULL; i++) {
            const PduRDestPdu_type * destination = PduRConfig->RoutingPaths[PduId]->PduRDestPdus[i];
            if(PduR_IsLoModule(destination->DestModule)){
                    /* @req PDUR551 */
                if(PduRConfig->RoutingPaths[PduId]->PduRDirectGateway !=FALSE){

                    if (PDUR_E_OK == PduR_ARC_RouteTransmit(destination, PduRTpRouteBuffer(PduId)->pduInfoPtr)) {
                        PduRTpRouteBuffer(PduId)->status = PDUR_BUFFER_TX_BUSY;
                    }
                }
            }else{
                PduR_ARC_RouteTpRxIndication(destination, Result );
            }
        }
    }
}

void PduR_ARC_RxIndication(PduIdType PduId, const PduInfoType* PduInfo, uint8 serviceId) {
    PDUR_VALIDATE_INITIALIZED(serviceId);
    PDUR_VALIDATE_PDUPTR(serviceId, PduInfo);
    PDUR_VALIDATE_PDUID(serviceId, PduId);

    /* @req PDUR164 */
    /* @req PDUR161 */
    const PduRRoutingPath_type *route = PduRConfig->RoutingPaths[PduId];

    /* @req PDUR160 */
    /* @req PDUR766 */
    /* @req PDUR621 */
    /* @req PDUR744 */
    /* @req PDUR673 */
    /* @req PDUR673 */
    /* @req PDUR436 */
    /* @req PDUR437 */
    /* @req PDUR303 */
    /* @req PDUR306 */
    /* @req PDUR783 */
    /* @req PDUR643 ?*/
    /* @req PDUR683 ?*/

    if (HAS_BUFFER_STATUS(PduId, PDUR_BUFFER_ALLOCATED_FROM_UP_MODULE)) {
        PduR_ARC_RxIndicationWithUpBuffer(PduId, PduInfo, serviceId);

    } else {

        for (uint8 i = 0; route->PduRDestPdus[i] != NULL; i++) {
            const PduRDestPdu_type * destination = route->PduRDestPdus[i];

            if (PduR_IsUpModule(destination->DestModule)) {
                PduR_ARC_RouteRxIndication(destination, PduInfo);

            } else if (PduR_IsLoModule(destination->DestModule)) {
                if (destination->DataProvision == PDUR_TRIGGER_TRANSMIT) {
                    PduR_ARC_RxIndicationTT(destination, PduInfo);

                } else if (destination->DataProvision == PDUR_DIRECT) {
                    PduR_ARC_RxIndicationDirect(destination, PduInfo);

                } else {
                    // Do nothing
                }

            } else {
                // Do nothing
            }
        }
    }

    (void)PduR_ARC_ReleaseRxBuffer(PduId);
}

void PduR_ARC_TpTxConfirmation(PduIdType PduId, uint8 result, uint8 serviceId) {

    PduIdType sourcePduId;

    PDUR_VALIDATE_INITIALIZED(serviceId);

    if (PduId >= PDU_MAX_ROUTING_PATH) {
        sourcePduId = (PduId & (uint16)PDUR_PDU_ID_MASK);
    }else{
        sourcePduId = PduId;
    }

    PDUR_VALIDATE_PDUID(serviceId, sourcePduId);

    const PduRRoutingPath_type *route = PduRConfig->RoutingPaths[sourcePduId];

    /* @req PDUR301 */

    if (PduR_IsUpModule(route->SrcModule)) {
        PduR_ARC_RouteTpTxConfirmation(route,result);

    } else if (PduR_IsLoModule(route->SrcModule) && HAS_BUFFER_STATUS(sourcePduId, PDUR_BUFFER_TX_BUSY)) {
        PduRTpRouteBuffer(sourcePduId)->nAcc++;

        uint8 nDests = 0;
        for (uint8 i = 0; route->PduRDestPdus[i] != NULL; i++) {
            if (!PduR_IsUpModule(route->PduRDestPdus[i]->DestModule)) {
                nDests++;
            }
        }

        if (PduRTpRouteBuffer(sourcePduId)->nAcc >= nDests) {
            // Release any buffer held by this route
            /* @req PDUR637 */
            BufReq_ReturnType status = PduR_ARC_ReleaseTxBuffer(sourcePduId);
            if (status != BUFREQ_OK) {
                REPORT_BUFFER_ERROR(serviceId);
            }
        }
    } else {
        // Do nothing
    }
}

void PduR_ARC_TxConfirmation(PduIdType PduId, uint8 result, uint8 serviceId) {
    PDUR_VALIDATE_INITIALIZED(serviceId);
    PDUR_VALIDATE_PDUID(serviceId, PduId);

    const PduRRoutingPath_type *route = PduRConfig->RoutingPaths[PduId];

    /* @req PDUR627 */

    if (PduR_IsUpModule(route->SrcModule)) {
        PduR_ARC_RouteTxConfirmation(route, result);

    } else if (PduR_IsLoModule(route->SrcModule) && HAS_BUFFER_STATUS(PduId, PDUR_BUFFER_TX_BUSY)) {

        // Release any buffer held by this route
        /* @req PDUR637 */
        BufReq_ReturnType status = PduR_ARC_ReleaseTxBuffer(PduId);
        if (status != BUFREQ_OK) {
            REPORT_BUFFER_ERROR(serviceId);
        }
    } else {
        // Do nothing
    }
}

Std_ReturnType PduR_ARC_TriggerTransmit(PduIdType PduId, PduInfoType* PduInfo, uint8 serviceId) {
    /*lint -esym(613,PduInfo)*/ /* PC-Lint 613 misunderstanding: PduInfo is not null since it is validated in PDUR_VALIDATE_PDUPTR */
    PDUR_VALIDATE_INITIALIZED(serviceId, E_NOT_OK);
    PDUR_VALIDATE_PDUPTR(serviceId, PduInfo, E_NOT_OK);
    PDUR_VALIDATE_PDUID(serviceId, PduId, E_NOT_OK);

    Std_ReturnType retVal = E_NOT_OK;
    SchM_Enter_PduR_EA_0();
    /* @req PDUR0766 */
    /* @req PDUR773 */
    const PduRRoutingPath_type *route = PduRConfig->RoutingPaths[PduId];
    const PduRDestPdu_type * destination = route->PduRDestPdus[0];

    if (PduR_IsUpModule(route->SrcModule)) {
        retVal = PduR_ARC_RouteTriggerTransmit(route, PduInfo);

    } else if (PduR_IsLoModule(route->SrcModule)) {
        if (destination->DataProvision == PDUR_TRIGGER_TRANSMIT) {
            if( TRUE == PduRRoutingPathEnabled(destination) ) {
                /* @req PDUR662 */
                memcpy((void *)PduInfo->SduDataPtr, (void *)PduR_RamBufCfg.TxBuffers[destination->TxBufferId].DataPtr, PduInfo->SduLength);
                retVal = E_OK;
            }
        }
    } else {
        //IMPROVEMENT: Add DET error
    }
    SchM_Exit_PduR_EA_0();
    return retVal;
}

/** List has at least one TP module */
static boolean PduR_ARC_HasTpModuleDest(const PduRDestPdu_type * const *destPdus)
{
    boolean ret = FALSE;
    for (uint32 i = 0; ((ret==FALSE) && (destPdus[i] != NULL)); i++)
    {
        if (PduR_IsTpModule(destPdus[i]->DestModule))
        {
            ret = TRUE;
        }
    }
    return ret;
}

/** Find first upper module in list */
static const PduRDestPdu_type * PduR_ARC_FindUPDest(const PduRDestPdu_type * const *destPdus)
{
    const PduRDestPdu_type *dest = NULL;
    for (uint32 i = 0; (dest == NULL) && (destPdus[i] != NULL); i++)
    {
        if (PduR_IsUpModule(destPdus[i]->DestModule))
        {
            dest = destPdus[i];
        }
    }
    return dest;
}


// PduR Interface used by Upper modules

/* @req PDUR406 */
Std_ReturnType PduR_UpTransmit(PduIdType pduId, const PduInfoType* pduInfoPtr, uint8 serviceId) {
    return PduR_ARC_Transmit(pduId, pduInfoPtr, serviceId);
}

/* @req PDUR0769 */
Std_ReturnType PduR_UpCancelTransmit(PduIdType pduId, uint8 serviceId) {
    // IMPROVEMENT: Add support
    (void)pduId;
    (void)serviceId;
    return E_NOT_OK;
}

/* @req PDUR482 */
void PduR_UpChangeParameter(PduIdType pduId, TPParameterType parameter, uint16 value, uint8 serviceId) {
    // IMPROVEMENT: Add support
    (void)pduId;
    (void)parameter;
    (void)value;
    (void)serviceId;
}

/* @req PDUR0767 */
Std_ReturnType PduR_UpCancelReceive(PduIdType pduId, uint8 serviceId) {
    // IMPROVEMENT: Add support
    (void)pduId;
    (void)serviceId;
    return E_NOT_OK;
}


// PduR Interface used by Lower If modules

/* @req PDUR362 */
void PduR_LoIfRxIndication(PduIdType pduId, PduInfoType* pduInfoPtr, uint8 serviceId) {

    PduR_ARC_RxIndication(pduId, pduInfoPtr, serviceId);
/*lint -e818 Diclaring pduInfoPtr as const PduInfoType* will cause deviation in ASR API prototype */
}

/* @req PDUR365 */
void PduR_LoIfTxConfirmation(PduIdType pduId, uint8 serviceId) {
    uint8 dummy = 0;
    PduR_ARC_TxConfirmation(pduId, dummy, serviceId);
}

/* @req PDUR369 */
Std_ReturnType PduR_LoIfTriggerTransmit(PduIdType pduId, PduInfoType* pduInfoPtr, uint8 serviceId) {
    return PduR_ARC_TriggerTransmit(pduId, pduInfoPtr, serviceId);
}

/**
 * @brief Function to check minimum buffer size for reception
 * @param minBufSize Minimum buffer size computed
 * @param avblBufSize Available buffer size
 */
static inline void calculateMinBufferSize(PduLengthType *minBufSize, const PduLengthType *avblBufSize) {
    if (*minBufSize != 0 )  {
        if ( *avblBufSize < *minBufSize)  {
            *minBufSize = *avblBufSize;
        }
    } else {
        *minBufSize = *avblBufSize;
    }

}
// PduR Interface used by Lower Tp modules

/* @req PDUR512 */
BufReq_ReturnType PduR_LoTpCopyRxData(PduIdType pduId, PduInfoType* info, PduLengthType* bufferSizePtr, uint8 serviceId) {
    // Destination cases:
    // 1. Only Upper modules: route without PduR buffering
    // 2. At least one Lower TP module: route WITH PduR buffering
    /* !req PDUR0708 */
    const PduRRoutingPath_type *route;
    const PduRDestPdu_type * destination;
    PduLengthType tmpRxByteCnt;
    PduLengthType minBufSize;
    PduLengthType bufSize;
    BufReq_ReturnType retVal;
    uint8 i;

    PDUR_VALIDATE_INITIALIZED(serviceId,BUFREQ_NOT_OK);
    PDUR_VALIDATE_PDUID(serviceId, pduId, BUFREQ_NOT_OK);
    PDUR_VALIDATE_PDUPTR(serviceId, info, BUFREQ_NOT_OK);

    SchM_Enter_PduR_EA_0();

    route = PduRConfig->RoutingPaths[pduId];
    minBufSize = 0;
    bufSize = 0;
    retVal = BUFREQ_OK;

    for (i = 0; route->PduRDestPdus[i] != NULL; i++) {
        // For all upper destination modules
        if (PduR_IsUpModule(route->PduRDestPdus[i]->DestModule))
        {
            destination = route->PduRDestPdus[i];
            retVal = PduR_ARC_RouteCopyRxData(destination, info, &bufSize);
            if (retVal == BUFREQ_OK) {
                calculateMinBufferSize(&minBufSize, &bufSize);
            }else if (route->PduRDestinatnNum > 1){
                /* In case of gateway to multiple destinations:
                 * Even if one one copy fails for any reason (Busy, overflow etc.) set the status as BUFREQ_NOT_OK and terminate
                 */
                retVal = BUFREQ_NOT_OK;
                break;
            } else {
                /* Do nothing */
            }
        }
    }

    if ((BUFREQ_OK == retVal) && (TRUE == PduR_ARC_HasTpModuleDest(route->PduRDestPdus))) {
        retVal = PduR_ARC_CheckBufferStatus(pduId, info->SduLength);
        if ((BUFREQ_NOT_OK == retVal) && (PduRTpRouteBuffer(pduId) != NULL)) {
            /* @req PDUR687 */
            PduRTpRouteBuffer(pduId)->status = PDUR_BUFFER_FREE;
            PduRTpRouteBuffer(pduId)->nAcc = 0;
            PduRTpRouteBuffer(pduId) = NULL;
        }
        else if (BUFREQ_OK == retVal) {

            // Copy rx data to buffer
            memcpy(&PduRTpRouteBuffer(pduId)->pduInfoPtr->SduDataPtr[PduRTpRouteBuffer(pduId)->rxByteCount], info->SduDataPtr, info->SduLength);
            tmpRxByteCnt = PduRTpRouteBuffer(pduId)->rxByteCount;
            tmpRxByteCnt += info->SduLength;

            PduRTpRouteBuffer(pduId)->rxByteCount = tmpRxByteCnt;
            bufSize = PduRTpRouteBuffer(pduId)->pduInfoPtr->SduLength - tmpRxByteCnt;
            calculateMinBufferSize(&minBufSize, &bufSize);

            if(PduRConfig->RoutingPaths[pduId]->PduRDirectGateway !=TRUE){
                // Check whether PduRTpBuffer reached threshold
                if ((PduRConfig->RoutingPaths[pduId]->PduRTpThreshld <= tmpRxByteCnt) && (PduRTpRouteBuffer(pduId)->status == PDUR_BUFFER_RX_BUSY))
                {
                    /* @req PDUR317 */
                    PduRTpRouteBuffer(pduId)->status = PDUR_BUFFER_TX_READY;
                }
                // Transmit on lower TP destinations
                if (PduRTpRouteBuffer(pduId)->status == PDUR_BUFFER_TX_READY)
                {
                    for (i = 0; (route->PduRDestPdus[i] != NULL); i++) {
                        // For all lower destination modules
                        if (PduR_IsLoModule(route->PduRDestPdus[i]->DestModule))
                        {
                            destination = route->PduRDestPdus[i];
                            if (PDUR_E_OK != PduR_ARC_RouteTransmit(destination, PduRTpRouteBuffer(pduId)->pduInfoPtr)) {
                                retVal = BUFREQ_NOT_OK;
                            }
                            else
                            {
                                PduRTpRouteBuffer(pduId)->status = PDUR_BUFFER_TX_BUSY;
                            }
                        }
                    }
                }
            }
        } else {
            /* Do nothing */
        }
    }
    SchM_Exit_PduR_EA_0();
    // Out bufferSizePtr: available minimum buffer size
    if (bufferSizePtr != NULL) {
        *bufferSizePtr = minBufSize;
    }

    if (retVal != BUFREQ_OK)
    {
        REPORT_BUFFER_ERROR(serviceId);
    }
    return retVal;
}

/* @req PDUR375  */
void PduR_LoTpRxIndication(PduIdType id, NotifResultType result, uint8 serviceId) {
    PduR_ARC_TpRxIndication(id, result, serviceId);
}

/* @req PDUR507 */
BufReq_ReturnType PduR_LoTpStartOfReception(PduIdType pduId, PduLengthType TpSduLength, PduLengthType* bufferSizePtr, uint8 serviceId) {
    /* @req PDUR623 */
    /* @req PDUR0749 */
    // Destination cases:
    // 1. Only Upper modules: route without PduR buffering
    // 2. At least one Lower TP module: route WITH PduR buffering

    const PduRRoutingPath_type *route;
    const PduRDestPdu_type * destination;
    BufReq_ReturnType retVal;
    PduLengthType minBufSize;
    PduLengthType bufSize;
    uint8 i;
    boolean startOfRxOk; //Variable to indicate at least one start of reception is successful

    PDUR_VALIDATE_INITIALIZED(serviceId,BUFREQ_NOT_OK);
    PDUR_VALIDATE_PDUID(serviceId, pduId, BUFREQ_NOT_OK);

    route = PduRConfig->RoutingPaths[pduId];
    minBufSize = 0;
    retVal = BUFREQ_OK;
    startOfRxOk = FALSE;

    for (i = 0; route->PduRDestPdus[i] != NULL; i++) {
        // For all upper destination modules
        if (PduR_IsUpModule(route->PduRDestPdus[i]->DestModule))
        {
            destination = route->PduRDestPdus[i];
            retVal = PduR_ARC_RouteStartOfReception(destination, TpSduLength, &bufSize);
            if (retVal == BUFREQ_OK) {
                startOfRxOk = TRUE;
                calculateMinBufferSize(&minBufSize, &bufSize);
            }else if (route->PduRDestinatnNum > 1){
                /* In case of gateway to multiple destinations:
                 * Even if one start of reception fails for any reason (Busy, overflow etc.) set the status as BUFREQ_NOT_OK and terminate.
                 */
                retVal = BUFREQ_NOT_OK;
                break;
            } else {
                /* Do nothing */
            }
        }
    }

    /* Routing to lower destination modules will be triggered i.e transmission is initiated
     * 1. In case of gateway on fly: PduR_LoTpCopyRxData() when sufficient bytes are received
     * 2. In case of direct gateway: PduR_LoTpRxIndication() when all bytes are received
     *
     */
    if (  (BUFREQ_OK == retVal) && (TRUE == PduR_ARC_HasTpModuleDest(route->PduRDestPdus)) ) {
        /* If a gateway is already active (buffer not released). Return busy and LoTp is expected to try again */
        if (PduRTpRouteBuffer(pduId) != NULL) {
            retVal = BUFREQ_BUSY;
        } else {
            retVal = PduR_ARC_AllocateBuffer(pduId, TpSduLength);
            if (retVal == BUFREQ_OK) {
                bufSize = PduRTpRouteBuffer(pduId)->bufferSize;
                calculateMinBufferSize(&minBufSize, &bufSize);
            }else if (route->PduRDestinatnNum > 1){
                /* In case of gateway to multiple destinations:
                 * Even if one start of reception fails for any reason (Busy, overflow etc.) set the status as BUFREQ_NOT_OK and terminate
                 */
                retVal = BUFREQ_NOT_OK;
            } else {
                /* Do nothing */
            }
        }

    }


    if ((startOfRxOk == TRUE) && (retVal != BUFREQ_OK)) {
        /* If the start of reception for any upper layer module returns E_OK implies a buffer is locked for subsequent reception by that module.
         * Now the overall status of this API is not equal to BUFREQ_OK. So in order to unlock the buffers we need to call
         * Module_TpRxIndication(). Ex. Dcm_TpRxIndication().
         * Module_TpRxIndication() will be ignored by modules which did not return E_OK.
         */
        for (i = 0; route->PduRDestPdus[i] != NULL; i++) {
            // For all upper destination modules
            if (PduR_IsUpModule(route->PduRDestPdus[i]->DestModule))
            {
                destination = route->PduRDestPdus[i];
                PduR_ARC_RouteTpRxIndication(destination, NTFRSLT_E_NOT_OK );
            }
        }
    }

    // Out bufferSizePtr: available minimum buffer size
    if (bufferSizePtr != NULL) {
        *bufferSizePtr = minBufSize;
    }

    if (retVal != BUFREQ_OK) {
        REPORT_BUFFER_ERROR(serviceId);
    }
    return retVal;
}

/* @req PDUR518 */
BufReq_ReturnType PduR_LoTpCopyTxData(PduIdType pduId, PduInfoType* info, RetryInfoType* retry, PduLengthType* availableDataPtr, uint8 serviceId) {
    /* !req PDUR435 *//* Always forwarded to upper layer module */
    /* @req PDUR0676 */
    PDUR_VALIDATE_INITIALIZED(serviceId,BUFREQ_NOT_OK);

    PDUR_VALIDATE_PDUPTR(serviceId, info, BUFREQ_NOT_OK);

    PduIdType sourcePduId;
    uint8 destionId;

    destionId =0;                 //This is for the 1st destination
    if (pduId >= PDU_MAX_ROUTING_PATH) {
        destionId = (pduId >> 13U);     // This gives the destination number among the destinations configured.
        sourcePduId = (pduId & (uint16)PDUR_PDU_ID_MASK);           // This gives the Source PduId
    }else{
        sourcePduId= pduId;
    }

    PDUR_VALIDATE_PDUID(serviceId, sourcePduId, BUFREQ_NOT_OK);
    SchM_Enter_PduR_EA_0();

    BufReq_ReturnType retVal = BUFREQ_OK;
    const PduRRoutingPath_type *route = PduRConfig->RoutingPaths[sourcePduId];
    RetryInfoType checkedRetry;
    const uint8 *srcPtr;
    PduLengthType length;

    if (PduR_IsUpModule(route->SrcModule)) {
        // Upper module, just forward the copy request
        retVal = PduR_ARC_RouteCopyTxData(route, info, retry, availableDataPtr);
    } else if (PduR_IsLoModule(route->SrcModule) && ( (PduRTpRouteBuffer(sourcePduId) != NULL))) {
        /* Lower tp module, copy from existing transmit data buffer (gateway) */

        /* Check retry info, null ptr */
        if (retry != NULL) {
            checkedRetry.TpDataState = retry->TpDataState;
            checkedRetry.TxTpDataCnt = retry->TxTpDataCnt;
            if ((PduRConfig->RoutingPaths[sourcePduId]->PduRDirectGateway!= TRUE) && (retry->TpDataState != TP_DATACONF)) {
                /* @req PDUR690 */
                PduRTpRouteBuffer(sourcePduId)->status = PDUR_BUFFER_FREE;
                PduRTpRouteBuffer(sourcePduId)->nAcc = 0;
                PduRTpRouteBuffer(sourcePduId) = NULL;
                checkedRetry.TxTpDataCnt = 0;

                SchM_Exit_PduR_EA_0();
                /*lint -e{904} PERFORMANCE PduR_LoTpCopyTxData() will return E_NOT_OK because the TP buffers are cleared */
                return BUFREQ_NOT_OK;
            }
        }else {
            // Retry not supported, copy all
            checkedRetry.TpDataState = TP_DATACONF;
            checkedRetry.TxTpDataCnt = 0;
        }

        // Check retry info, data state
        if (checkedRetry.TpDataState != TP_DATARETRY)
        {
            // No retry, copy all
            checkedRetry.TxTpDataCnt = 0;
        }

        length = info->SduLength;
        if (NULL == PduRTpRouteBuffer(sourcePduId)) {
            retVal = BUFREQ_NOT_OK;
        }
        else if (   ( checkedRetry.TxTpDataCnt > PduRTpRouteBuffer(sourcePduId)->pduInfoPtr->SduLength) ||
               ( length > PduRTpRouteBuffer(sourcePduId)->bufferSize))
        {
            retVal = BUFREQ_NOT_OK;
        }
        /* Check if requested data is already buffered incase of gateway on fly */
        else if (((PduRConfig->RoutingPaths[sourcePduId]->PduRDirectGateway !=TRUE)) && ((PduRTpRouteBuffer(sourcePduId)->txByteCount[destionId] + length) > PduRTpRouteBuffer(sourcePduId)->rxByteCount))
        {
            retVal = BUFREQ_BUSY;
        }
        else if ((PduRConfig->RoutingPaths[sourcePduId]->PduRDirectGateway ==TRUE) && ((PduRTpRouteBuffer(sourcePduId)->txByteCount[destionId] + length) > PduRTpRouteBuffer(sourcePduId)->rxByteCount)){
            retVal = BUFREQ_NOT_OK;
        }

        else {
            //Check whether gateway
            if ((checkedRetry.TpDataState !=TP_DATARETRY))
            {
                /* @req PDUR707*/
                // Copy from PduR transmit buffer to Lower tp module buffer
                srcPtr = &PduRTpRouteBuffer(sourcePduId)->pduInfoPtr->SduDataPtr[PduRTpRouteBuffer(sourcePduId)->txByteCount[destionId]];
                PduRTpRouteBuffer(sourcePduId)->txByteCount[destionId] += length;
               }
            else {
                // Copy from PduR transmit buffer to Lower tp module buffer
                srcPtr = &PduRTpRouteBuffer(sourcePduId)->pduInfoPtr->SduDataPtr[checkedRetry.TxTpDataCnt];
                PduRTpRouteBuffer(sourcePduId)->txByteCount[destionId] =  checkedRetry.TxTpDataCnt + length;
            }

            memcpy(info->SduDataPtr, srcPtr, length);

            // Indicate remaining data in tx buffer
            if (availableDataPtr != NULL) {
                *availableDataPtr = (PduRTpRouteBuffer(sourcePduId)->pduInfoPtr->SduLength - PduRTpRouteBuffer(sourcePduId)->txByteCount[destionId]);
            }
            retVal = BUFREQ_OK;
        }
    } else {
        retVal = BUFREQ_NOT_OK;
    }

    SchM_Exit_PduR_EA_0();
    return retVal;
}

/* @req PDUR381 */
void PduR_LoTpTxConfirmation(PduIdType id, NotifResultType result, uint8 serviceId) {

    PduR_ARC_TpTxConfirmation(id, result, serviceId);
}

/**
 * Checks if a destination is enabled
 * @param destination
 * @return
 */
boolean PduRRoutingPathEnabled(const PduRDestPdu_type * destination)
{
#if (PDUR_MAX_NOF_ROUTING_PATH_GROUPS > 0)
    boolean enabled = TRUE;
    if( NULL != destination ) {
        if( 0 != destination->NofGroupRefs ) {
            for(PduR_RoutingPathGroupIdType i = 0; ((i < destination->NofGroupRefs) && (enabled==TRUE)); i++) {
                enabled = PdurRoutingGroupEnabled[destination->RoutingPathGroupRefs[i]];
            }
        } else {
            enabled = TRUE;
        }
    } else {
        /* Invalid destination. Cannot be enabled. */
        enabled = FALSE;
    }
    return enabled;
#else
    (void)destination;/*lint !e920*/
    return TRUE;
#endif
}

#if (PDUR_MAX_NOF_ROUTING_PATH_GROUPS > 0)
void PdurSetRoutingPathEnabled(PduR_RoutingPathGroupIdType id, boolean enabled)
{
    PdurRoutingGroupEnabled[id] = enabled;
}
#endif

#endif /* PDUR_ZERO_COST_OPERATION */
