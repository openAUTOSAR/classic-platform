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
/* @req PDUR233 */


#include "PduR.h"

#include <string.h>
#include "debug.h"
#include "Det.h"
#if defined(USE_DEM)
#include "Dem.h"
#endif

#if PDUR_ZERO_COST_OPERATION == STD_OFF

#define PduRTpBuffer(_id) (&PduR_RamBufCfg.TpBuffers[_id])
#define PduRTpRouteBuffer(_id) (PduR_RamBufCfg.TpRouteBuffers[_id])

#define HAS_BUFFER_STATUS(_pduId, _status)  (_pduId < PduR_RamBufCfg.NTpRouteBuffers && PduRTpRouteBuffer(_pduId) != NULL && PduRTpRouteBuffer(_pduId)->status == _status)
#define REPORT_BUFFER_ERROR(_serviceId) PDUR_DET_REPORTERROR(MODULE_ID_PDUR, PDUR_INSTANCE_ID, _serviceId, PDUR_E_BUFFER_ERROR);
#define DET_REPORTERROR(_x,_y,_z,_q) Det_ReportError(_x, _y, _z, _q)

// Static function prototypes
static boolean PduR_ARC_HasTpModuleDest(const PduRDestPdu_type * const *destPdus);
static const PduRDestPdu_type * PduR_ARC_FindUPDest(const PduRDestPdu_type * const *destPdus);
static BufReq_ReturnType PduR_ARC_AllocateBuffer(PduIdType PduId, PduLengthType TpSduLength);
static BufReq_ReturnType PduR_ARC_CheckBufferStatus(PduIdType PduId, uint16 length);
static BufReq_ReturnType PduR_ARC_ReleaseRxBuffer(PduIdType PduId);
static BufReq_ReturnType PduR_ARC_ReleaseTxBuffer(PduIdType PduId);
static void PduR_ARC_RxIndicationDirect(const PduRDestPdu_type * destination, const PduInfoType *PduInfo);
static void PduR_ARC_RxIndicationTT(const PduRDestPdu_type * destination, const PduInfoType *PduInfo);


static BufReq_ReturnType PduR_ARC_AllocateBuffer(PduIdType PduId, PduLengthType TpSduLength) {
	BufReq_ReturnType retVal = BUFREQ_BUSY;
	for (uint8 i = 0; i < PduR_RamBufCfg.NTpBuffers; i++) {
		if (PduRTpBuffer(i)->status == PDUR_BUFFER_FREE) {
			if (PduRTpBuffer(i)->bufferSize < TpSduLength) {
				retVal = BUFREQ_OVFL;
			} else {
				PduRTpRouteBuffer(PduId) = PduRTpBuffer(i);
				PduRTpBuffer(i)->pduInfoPtr->SduLength = TpSduLength;
				PduRTpRouteBuffer(PduId)->status = PDUR_BUFFER_RX_READY;
				PduRTpRouteBuffer(PduId)->txByteCount = 0;
				PduRTpRouteBuffer(PduId)->rxByteCount = 0;
				retVal = BUFREQ_OK;
				break;
			}
		}
	}
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
	else if (PduRTpRouteBuffer(PduId)->status == PDUR_BUFFER_TX_BUSY
			|| PduRTpRouteBuffer(PduId)->status == PDUR_BUFFER_RX_BUSY) {

		retVal = BUFREQ_OK;
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

	Std_ReturnType retVal = E_OK;
	const PduRRoutingPath_type *route = PduRConfig->RoutingPaths[PduId];
	for (uint8 i = 0; route->PduRDestPdus[i] != NULL; i++) {
		const PduRDestPdu_type * destination = route->PduRDestPdus[i];

		retVal |= PduR_ARC_RouteTransmit(destination, PduInfo);
	}
	return retVal;
}

static void PduR_ARC_RxIndicationTT(const PduRDestPdu_type * destination, const PduInfoType *PduInfo) {
	Std_ReturnType retVal = E_OK;

	/* @req PDUR160 */
	/* @req PDUR661 */
	/* @req PDUR662 */
	if (!memcpy(PduR_RamBufCfg.TxBuffers[destination->TxBufferId], PduInfo->SduDataPtr, PduInfo->SduLength)){ retVal |= E_NOT_OK;}
	retVal |= PduR_ARC_RouteTransmit(destination, PduInfo);

	if (retVal != E_OK) {
		DET_REPORTERROR(MODULE_ID_PDUR, 0, PDUR_SERVICEID_CANIFRXINDICATION, PDUR_E_PDU_INSTANCES_LOST);
	}
	/*
	// This is a gateway request which uses trigger transmit data provision. PDUR255
	if (destination->TxBufferRef->TxConfP) { // Transfer confirmation pending.
		// Enqueue the new I-PDU. This will flush the buffer if it is full according to the buffer specification.
		PduR_BufferQueue(destination->TxBufferRef, PduInfo->SduDataPtr);
		// TODO report PDUR_E_PDU_INSTANCE_LOST to DEM if needed.
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


	Std_ReturnType retVal = PduR_ARC_RouteTransmit(destination, PduInfo);
	if (retVal != E_OK) {
		DET_REPORTERROR(MODULE_ID_PDUR, 0, PDUR_SERVICEID_CANIFRXINDICATION, PDUR_E_PDU_INSTANCES_LOST);
	}
}

static void PduR_ARC_RxIndicationWithUpBuffer(PduIdType PduId, const PduInfoType* PduInfo, uint8 serviceId) {
	const PduRRoutingPath_type *route = PduRConfig->RoutingPaths[PduId];
	const PduRDestPdu_type * destination;

	if (PduRTpRouteBuffer(PduId) == NULL) {
	    REPORT_BUFFER_ERROR(serviceId);
	    return ;
	}
	// Save current buffer
	PduInfoType *upBuffer = PduRTpRouteBuffer(PduId)->pduInfoPtr;
	BufReq_ReturnType retVal = PduR_ARC_ReleaseRxBuffer(PduId);
	if (retVal != BUFREQ_BUSY) {
		REPORT_BUFFER_ERROR(serviceId);
		return;
	}

	// Get a new PduR buffer
	retVal = PduR_ARC_AllocateBuffer(PduId, upBuffer->SduLength);
	if (retVal != BUFREQ_OK) {
		REPORT_BUFFER_ERROR(serviceId);
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
		Std_ReturnType status = PduR_ARC_RouteTransmit(d, PduRTpRouteBuffer(PduId)->pduInfoPtr);
		if(status!=E_OK){
			// TODO: do error reporting?
		}
	}

}

void PduR_ARC_TpRxIndication(PduIdType PduId, NotifResultType Result, uint8 serviceId) {
	PDUR_VALIDATE_INITIALIZED(serviceId);
	PDUR_VALIDATE_PDUID(serviceId, PduId);

	PduInfoType PduInfo = {
		.SduDataPtr = &Result,
		.SduLength = 0 // To fix PC-Lint 785
	};

	if (Result != NTFRSLT_OK) {
		// There was an error in the lower layer while receiving the PDU.
		// Release any buffers and notify upper layers
		(void)PduR_ARC_ReleaseRxBuffer(PduId);
		for (uint8 i = 0; PduRConfig->RoutingPaths[PduId]->PduRDestPdus[i] != NULL; i++) {
			const PduRDestPdu_type * destination = PduRConfig->RoutingPaths[PduId]->PduRDestPdus[i];
			if (PduR_IsUpModule(destination->DestModule)) {
				PduR_ARC_RouteRxIndication(destination, &PduInfo);
			}
			else if (PduRTpRouteBuffer(PduId) == NULL) {
			    REPORT_BUFFER_ERROR(serviceId);
			}
			else {
				//Gateway on Tp
	        	/* @req PDUR689 */
	        	PduRTpRouteBuffer(PduId)->status = PDUR_BUFFER_FREE;
	        	PduRTpRouteBuffer(PduId)->nAcc = 0;
	        	PduRTpRouteBuffer(PduId) = NULL;
			}
		}

	} else {
		PduR_ARC_RxIndication(PduId, &PduInfo, serviceId);
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

                if (PduR_IsTpModule(destination->DestModule)) { // TP Gateway
                    if (HAS_BUFFER_STATUS(PduId, PDUR_BUFFER_RX_BUSY)) {
						/* Buffer needs to be set to PDUR_BUFFER_TX_READY to be gatewayed */
                    	(void)PduR_ARC_ReleaseRxBuffer(PduId);
                        // Transmit previous rx buffer
                        Std_ReturnType status = PduR_ARC_RouteTransmit(destination, PduRTpRouteBuffer(PduId)->pduInfoPtr);
                        if(status!=E_OK){
                            // TODO: do error reporting?
                        }
                    }

                } else if (destination->DataProvision == PDUR_TRIGGER_TRANSMIT) {
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

void PduR_ARC_TxConfirmation(PduIdType PduId, uint8 result, uint8 serviceId) {
	PDUR_VALIDATE_INITIALIZED(serviceId);
	PDUR_VALIDATE_PDUID(serviceId, PduId);

	const PduRRoutingPath_type *route = PduRConfig->RoutingPaths[PduId];

	/* @req PDUR627 */

	if (PduR_IsUpModule(route->SrcModule)) {
		PduR_ARC_RouteTxConfirmation(route, result);

	} else if (PduR_IsLoModule(route->SrcModule) && HAS_BUFFER_STATUS(PduId, PDUR_BUFFER_TX_BUSY)) {
		PduRTpRouteBuffer(PduId)->nAcc++;

		uint8 nDests = 0;
		for (uint8 i = 0; route->PduRDestPdus[i] != NULL; i++) {
			if (!PduR_IsUpModule(route->PduRDestPdus[i]->DestModule)) nDests++;
		}

		if (PduRTpRouteBuffer(PduId)->nAcc >= nDests) {
	        // Release any buffer held by this route
	        /* @req PDUR637 */
			BufReq_ReturnType status = PduR_ARC_ReleaseTxBuffer(PduId);
			if (status != BUFREQ_OK) {
				REPORT_BUFFER_ERROR(serviceId);
			}
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

	Std_ReturnType retVal = E_OK;

	/* @req PDUR0766 */
	/* @req PDUR661 */
	/* @req PDUR773 */
	const PduRRoutingPath_type *route = PduRConfig->RoutingPaths[PduId];
	const PduRDestPdu_type * destination = route->PduRDestPdus[0];

	if (PduR_IsUpModule(route->SrcModule)) {
		retVal |= PduR_ARC_RouteTriggerTransmit(route, PduInfo);

	} else if (PduR_IsLoModule(route->SrcModule)) {
		if (destination->DataProvision == PDUR_TRIGGER_TRANSMIT) {
			if (!memcpy((void *)PduInfo->SduDataPtr, (void *)PduR_RamBufCfg.TxBuffers[destination->TxBufferId], PduInfo->SduLength)) {
				retVal = E_NOT_OK;
			}
		}
	} else {
		// TODO: Do nothing???? or retVal = E_OK or E_NOT_OK?
	}
	return retVal;
}

/** List has at least one TP module */
static boolean PduR_ARC_HasTpModuleDest(const PduRDestPdu_type * const *destPdus)
{
    boolean ret = FALSE;
    for (int i=0; (!ret) && (destPdus[i]!=NULL); i++)
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
    for (int i=0; (dest==NULL) && (destPdus[i]!=NULL); i++)
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
    // TODO: implement
	(void)pduId;
	(void)serviceId;
    return E_NOT_OK;
}

/* @req PDUR482 */
void PduR_UpChangeParameter(PduIdType pduId, TPParameterType parameter, uint16 value, uint8 serviceId) {
    // TODO: implement
	(void)pduId;
	(void)parameter;
	(void)value;
	(void)serviceId;
}

/* @req PDUR0767 */
Std_ReturnType PduR_UpCancelReceive(PduIdType pduId, uint8 serviceId) {
    // TODO: implement
	(void)pduId;
	(void)serviceId;
    return E_NOT_OK;
}


// PduR Interface used by Lower If modules

/* @req PDUR362 */
void PduR_LoIfRxIndication(PduIdType pduId, PduInfoType* pduInfoPtr, uint8 serviceId) {
    PduR_ARC_RxIndication(pduId, pduInfoPtr, serviceId);
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

// PduR Interface used by Lower Tp modules

/* @req PDUR512 */
BufReq_ReturnType PduR_LoTpCopyRxData(PduIdType pduId, PduInfoType* info, PduLengthType* bufferSizePtr, uint8 serviceId) {
    // Destination cases:
    // 1. Only Upper modules: route without PduR buffering
    // 2. At least one Lower TP module: route WITH PduR buffering

	uint8 tmpRxByteCnt;
	PDUR_VALIDATE_INITIALIZED(serviceId,BUFREQ_NOT_OK);
    PDUR_VALIDATE_PDUID(serviceId, pduId, BUFREQ_NOT_OK);
    PDUR_VALIDATE_PDUPTR(serviceId, info, BUFREQ_NOT_OK);
    if (info == NULL) {
        // In case no DET
        return BUFREQ_E_NOT_OK;
    }

    BufReq_ReturnType retVal = BUFREQ_OK;
    const PduRRoutingPath_type *route = PduRConfig->RoutingPaths[pduId];
    PduLengthType minBufSize = 0;
    PduLengthType bufSize = 0;
    BufReq_ReturnType r;

    for (uint8 i = 0; route->PduRDestPdus[i] != NULL; i++) {
        // For all upper destination modules
        if (PduR_IsUpModule(route->PduRDestPdus[i]->DestModule))
        {
            const PduRDestPdu_type * destination = route->PduRDestPdus[i];
            r = PduR_ARC_RouteCopyRxData(destination, info, &bufSize);
            if (r != BUFREQ_OK) {
                retVal = r;
            }
            else if (bufSize < minBufSize || minBufSize == 0) {
                minBufSize = bufSize;
            }
        }
    }

    if (PduR_ARC_HasTpModuleDest(route->PduRDestPdus)) {
    	retVal = PduR_ARC_CheckBufferStatus(pduId, info->SduLength);
        if (BUFREQ_NOT_OK == retVal && PduRTpRouteBuffer(pduId) != NULL) {
        	/* @req PDUR687 */
        	PduRTpRouteBuffer(pduId)->status = PDUR_BUFFER_FREE;
        	PduRTpRouteBuffer(pduId)->nAcc = 0;
        	PduRTpRouteBuffer(pduId) = NULL;
        }
        else if (BUFREQ_OK == retVal) {

        	// Copy rx data to buffer
        	memcpy((PduRTpRouteBuffer(pduId)->pduInfoPtr->SduDataPtr + PduRTpRouteBuffer(pduId)->rxByteCount ), info->SduDataPtr, info->SduLength);
        	tmpRxByteCnt = PduRTpRouteBuffer(pduId)->rxByteCount;
        	tmpRxByteCnt += info->SduLength;

        	// Check whether PduRTpBuffer reached threshold
            if ((PduRConfig->RoutingPaths[pduId]->PduRTpThreshld <= tmpRxByteCnt) && PduRTpRouteBuffer(pduId)->status == PDUR_BUFFER_RX_BUSY)
            {
            	/* @req PDUR317 */
            	PduRTpRouteBuffer(pduId)->status = PDUR_BUFFER_TX_READY;
        	}
            // Transmit on lower TP destinations
            if (PduRTpRouteBuffer(pduId)->status == PDUR_BUFFER_TX_READY)
            {
				for (uint8 i = 0; (route->PduRDestPdus[i] != NULL); i++) {
					// For all lower destination modules
					if (PduR_IsLoModule(route->PduRDestPdus[i]->DestModule))
					{
						const PduRDestPdu_type * destination = route->PduRDestPdus[i];
						r = PduR_ARC_RouteTransmit(destination, PduRTpRouteBuffer(pduId)->pduInfoPtr);
						if (r != BUFREQ_OK) {
							retVal = r;
						}
						else
						{
							PduRTpRouteBuffer(pduId)->status = PDUR_BUFFER_TX_BUSY;
						}
					}
				}
            }
            /* Gateway on fly applicable only from one LoTp to other without any uppermodule */
            if (retVal == BUFREQ_OK)
            {
            	PduRTpRouteBuffer(pduId)->rxByteCount = tmpRxByteCnt;
            	/* Update availablebuffersize for gateway on fly */
            	minBufSize = (PduRConfig->RoutingPaths[pduId]->PduRTpThreshld == 0) ? minBufSize : (PduRTpRouteBuffer(pduId)->pduInfoPtr->SduLength - tmpRxByteCnt);
            }
        }
    }

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
    // Destination cases:
    // 1. Only Upper modules: route without PduR buffering
    // 2. At least one Lower TP module: route WITH PduR buffering

    PDUR_VALIDATE_INITIALIZED(serviceId,BUFREQ_NOT_OK);
    PDUR_VALIDATE_PDUID(serviceId, pduId, BUFREQ_NOT_OK);

    BufReq_ReturnType retVal = BUFREQ_OK;
    const PduRRoutingPath_type *route = PduRConfig->RoutingPaths[pduId];
    PduLengthType minBufSize = 0;
    PduLengthType bufSize;
    BufReq_ReturnType r;

    for (uint8 i = 0; route->PduRDestPdus[i] != NULL; i++) {
        // For all upper destination modules
        if (PduR_IsUpModule(route->PduRDestPdus[i]->DestModule))
        {
            const PduRDestPdu_type * destination = route->PduRDestPdus[i];
            r = PduR_ARC_RouteStartOfReception(destination, TpSduLength, &bufSize);
            if (r != BUFREQ_OK) {
                retVal = r;
            }
            else if (bufSize < minBufSize || minBufSize == 0) {
                minBufSize = bufSize;
            }
        }
    }

    if (PduR_ARC_HasTpModuleDest(route->PduRDestPdus)) {
        r = PduR_ARC_AllocateBuffer(pduId, TpSduLength);
        if (r != BUFREQ_OK) {
            retVal = r;
        }
        else if (PduRTpRouteBuffer(pduId)->bufferSize < minBufSize || minBufSize == 0) {
            minBufSize = PduRTpRouteBuffer(pduId)->bufferSize;
        }
    }

    // Lower destination modules will be routed to when there is actual data to copy, i.e. in the LoTp_CopyRxData call.

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
    PDUR_VALIDATE_INITIALIZED(serviceId,BUFREQ_NOT_OK);
    PDUR_VALIDATE_PDUID(serviceId, pduId, BUFREQ_NOT_OK);
    PDUR_VALIDATE_PDUPTR(serviceId, info, BUFREQ_NOT_OK);
    if (info == NULL) {
        // In case no DET
        return BUFREQ_E_NOT_OK;
    }

    BufReq_ReturnType retVal = BUFREQ_OK;
    const PduRRoutingPath_type *route = PduRConfig->RoutingPaths[pduId];
    RetryInfoType checkedRetry;
    uint8 *srcPtr;
    uint32 length;

    if (PduR_IsUpModule(route->SrcModule)) {
        // Upper module, just forward the copy request
        retVal = PduR_ARC_RouteCopyTxData(route, info, retry, availableDataPtr);
    }
    else if (PduR_IsLoModule(route->SrcModule) && ( (PduRTpRouteBuffer(pduId) != NULL))) {
        // Lower tp module, copy from existing transmit data buffer (gateway)

        // Check retry info, null ptr
        if (retry != NULL) {
        	checkedRetry.TpDataState = retry->TpDataState;
        	checkedRetry.TxTpDataCnt = retry->TxTpDataCnt;
        	if ((PduRConfig->RoutingPaths[pduId]->PduRTpThreshld != 0) && (retry->TpDataState != TP_DATACONF)) {
        		/* @req PDUR690 */
            	PduRTpRouteBuffer(pduId)->status = PDUR_BUFFER_FREE;
            	PduRTpRouteBuffer(pduId)->nAcc = 0;
            	PduRTpRouteBuffer(pduId) = NULL;
            	checkedRetry.TxTpDataCnt = 0;
            	retVal = BUFREQ_NOT_OK;
        	}
        }
        else {
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

        // Check if copy range is out of bounds
        length = info->SduLength - checkedRetry.TxTpDataCnt;
        if (NULL == PduRTpRouteBuffer(pduId)) {
        	retVal = BUFREQ_NOT_OK;
        }
        else if (   ( checkedRetry.TxTpDataCnt > PduRTpRouteBuffer(pduId)->pduInfoPtr->SduLength) ||
               ( length > PduRTpRouteBuffer(pduId)->bufferSize))
        {
            retVal = BUFREQ_NOT_OK;
        }
        /* Check if requested data is already buffered incase of gateway on fly */
        else if (PduRConfig->RoutingPaths[pduId]->PduRTpThreshld != 0 && (PduRTpRouteBuffer(pduId)->txByteCount + length) > PduRTpRouteBuffer(pduId)->rxByteCount)
        {
        	retVal = BUFREQ_BUSY;
        }

        else {
        	//Check whether gateway on-fly
        	if (PduRConfig->RoutingPaths[pduId]->PduRTpThreshld != 0)
        	{
        		/* @req PDUR707*/
        		// Copy from PduR transmit buffer to Lower tp module buffer
        		srcPtr = &PduRTpRouteBuffer(pduId)->pduInfoPtr->SduDataPtr[PduRTpRouteBuffer(pduId)->txByteCount];
        		PduRTpRouteBuffer(pduId)->txByteCount += length;
           	}
        	else {
        		// Copy from PduR transmit buffer to Lower tp module buffer
        		srcPtr = &PduRTpRouteBuffer(pduId)->pduInfoPtr->SduDataPtr[checkedRetry.TxTpDataCnt];

        	}

            memcpy(info->SduDataPtr, srcPtr, length);

            // Indicate remaining data in tx buffer
            if (availableDataPtr != NULL) {
                *availableDataPtr = (PduRTpRouteBuffer(pduId)->pduInfoPtr->SduLength - PduRTpRouteBuffer(pduId)->txByteCount);
            }
            retVal = BUFREQ_OK;
        }
    } else {
        retVal = BUFREQ_NOT_OK;
    }
    return retVal;
}

/* @req PDUR381 */
void PduR_LoTpTxConfirmation(PduIdType id, NotifResultType result, uint8 serviceId) {
		PduR_ARC_TxConfirmation(id, result, serviceId);
}


#endif
