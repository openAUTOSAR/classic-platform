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

#include <assert.h>
#include <stdlib.h>
#include <string.h>

/* @req 4.0.3/IPDUM148 */
#include "IpduM.h"
//#include "IpduM_Cbk.h"
#include "PduR_IpduM.h"
#include "IpduM_Cfg.h"
#include "Det.h"
//#include "Com.h"
#include "Cpu.h"

IpduM_InitState ipdum_state = IPDUM_UNINIT;
static const IpduM_ConfigType *IpduM_Config;

#if (IPDUM_DEV_ERROR_DETECT == STD_ON ) /* @req 4.0.3/IPDUM028*/ /* @req 4.0.3/IPDUM026 */
#define VALIDATE(_exp,_api,_err) \
	if (!(_exp)) { \
		Det_ReportError(MODULE_ID_IPDUM, 0, _api, _err); \
		return E_NOT_OK; \
	}

#define VALIDATE_NO_RV(_exp,_api,_err ) \
	if( !(_exp) ) { \
		Det_ReportError(MODULE_ID_IPDUM, 0, _api, _err); \
		return; \
	}

#define VALIDATE_NO_RETURN(_exp,_api,_err ) \
	if( !(_exp) ) { \
		Det_ReportError(MODULE_ID_IPDUM, 0, _api, _err); \
	}


#define VALIDATE_INIT(_api)								VALIDATE(ipdum_state == IPDUM_INIT, _api, IPDUM_E_UNINIT) /* @req 4.0.3/IPDUM153 */
#define VALIDATE_INIT_NO_RV(_api)						VALIDATE_NO_RV(ipdum_state == IPDUM_INIT, _api, IPDUM_E_UNINIT) /* @req 4.0.3/IPDUM153 */
#define VALIDATE_PARAM_POINTER(_api, _pointer) 			VALIDATE(_pointer != NULL, _api, IPDUM_E_PARAM_POINTER) /* @req 4.0.3/IPDUM162 */
#define VALIDATE_PARAM_POINTER_NO_RV(_api, _pointer)	VALIDATE_NO_RV(_pointer != NULL, _api, IPDUM_E_PARAM_POINTER) /* @req 4.0.3/IPDUM162 */
#define VALIDATE_RX_ID(_api, _id)						VALIDATE(_id < IPDUM_N_RX_PATHWAYS, _api, IPDUM_E_PARAM) /* @req 4.0.3/IPDUM026 */
#define VALIDATE_RX_ID_NO_RV(_api, _id)					VALIDATE_NO_RV(_id < IPDUM_N_RX_PATHWAYS, _api, IPDUM_E_PARAM) /* @req 4.0.3/IPDUM026 */
#if IPDUM_N_TX_PATHWAYS > 0
#define VALIDATE_TX_PART_ID(_api, _id)					VALIDATE(_id < IPDUM_N_TX_PARTS, _api, IPDUM_E_PARAM) /* @req 4.0.3/IPDUM026 */
#define VALIDATE_TX_PATHWAY_ID(_api, _id)				VALIDATE(_id < IPDUM_N_TX_PATHWAYS, _api, IPDUM_E_PARAM) /* @req 4.0.3/IPDUM026 */
#define VALIDATE_TX_PATHWAY_ID_NO_RV(_api, _id)			VALIDATE_NO_RV(_id < IPDUM_N_TX_PATHWAYS, _api, IPDUM_E_PARAM) /* @req 4.0.3/IPDUM026 */
#else
#define VALIDATE_TX_PART_ID(_api, _id)
#define VALIDATE_TX_PATHWAY_ID(_api, _id)
#define VALIDATE_TX_PATHWAY_ID_NO_RV(_api, _id)
#endif

#define DET_REPORTERROR(_x,_y,_z,_q) Det_ReportError(_x, _y, _z, _q)

#else /* @req 4.0.3/IPDUM027 */
#define VALIDATE(_exp,_api,_err )
#define VALIDATE_NO_RV(_exp,_api,_err )
#define VALIDATE_NO_RETURN(_exp,_api,_err )
#define VALIDATE_INIT(_api)
#define VALIDATE_INIT_NO_RV(_api)
#define VALIDATE_PARAM_POINTER(_api, _pointer)
#define VALIDATE_PARAM_POINTER_NO_RV(_api, _pointer)
#define VALIDATE_RX_ID(_api, _id)
#define VALIDATE_RX_ID_NO_RV(_api, _id)
#define VALIDATE_TX_PART_ID(_api, _id)
#define VALIDATE_TX_PATHWAY_ID(_api, _id)
#define VALIDATE_TX_PATHWAY_ID_NO_RV(_api, _id)
#define DET_REPORTERROR(_x,_y,_z,_q)
#endif

#define GETBITS(source, pos, mask)		(( *( (uint8 *)source  + (pos / 8) ) >> (pos % 8)) & mask)

#define HAS_STATIC_PART(_txPathway)		(_txPathway->staticPart != NULL)

static void CopySegment(const IpduM_Segment *segment, uint8 *dest, const uint8 *source) {
	uint8 byteNr = 0;
	uint8 mask = 0;
	while (byteNr < segment->maskLength) {
		mask = segment->mask[byteNr];
		*(dest + byteNr + segment->startByte) = (*(source + byteNr + segment->startByte) & mask) | (*(dest + byteNr + segment->startByte) & ~mask);
		byteNr++;
	}
}

static void CopyTxPart(const IpduM_TxPart *txPart, const uint8 *source) {
	imask_t irq_state;
	Irq_Save(irq_state);
	const IpduM_TxPathway *txPathway = txPart->txPathway;
	for (uint16 i = 0; i < txPart->nSegments; i++) {
		const IpduM_Segment *segment = &(txPart->segments[i]);
		CopySegment(segment, txPathway->buffer, source);
	}
	Irq_Restore(irq_state);
}

static Std_ReturnType FetchTxPart(const IpduM_TxPart *txPart) {

	Std_ReturnType ret;

	PduInfoType pduInfo = {
		.SduDataPtr = IpduM_Config->txBuffer,
		.SduLength  = txPart->txPathway->pduSize
	};

	imask_t irq_state;
	Irq_Save(irq_state);

	ret = PduR_IpduMTriggerTransmit(txPart->pduId, &pduInfo);

	/*lint -save -e522 */
	CopyTxPart(txPart, pduInfo.SduDataPtr);
	/*lint -restore */
	Irq_Restore(irq_state);

	return ret;

}


/* @req 4.0.3/IPDUM033 */
/* @req 4.0.3/IPDUM083 */
/* @req 4.0.3/IPDUM067 */
/* @req 4.0.3/IPDUM068 */
/* @req 4.0.3/IPDUM143 */
//lint -esym(904, IpduM_Init) //PC-Lint Exception of rule 14.7
void IpduM_Init(const IpduM_ConfigType *config) {
	// Com_Init() and PduR_Init() has to be called before IpduM_Init(), see chapter 7.3 of IpduM SRS.
	Std_ReturnType status;
	VALIDATE_PARAM_POINTER_NO_RV(IPDUM_API_INIT, config);
	IpduM_Config = config;


	for (uint16 i = 0; i < IPDUM_N_TX_PATHWAYS; i++) {
		const IpduM_TxPathway *txPathway = &(IpduM_Config->txPathways[i]);
		memset(txPathway->buffer, txPathway->IpduMIPduUnusedAreasDefault, txPathway->pduSize);

		status = FetchTxPart(*(txPathway->activeDynamicPart));
		// Need to do something with return value - seems appropiate to set DET error
		VALIDATE_NO_RETURN(status == E_OK, IPDUM_API_INIT, IPDUM_E_PARAM);

		if (HAS_STATIC_PART(txPathway)) {
			status = FetchTxPart(txPathway->staticPart);
			// Need to do something with return value - seems appropiate to set DET error
			VALIDATE_NO_RETURN(status == E_OK, IPDUM_API_INIT, IPDUM_E_PARAM);
		}
	}

	ipdum_state = IPDUM_INIT;
}


static boolean shouldTransmit(const IpduM_TxPart *txPart) {
	const IpduM_TxPathway *txPathway = txPart->txPathway;
	return ((txPathway->trigger == IPDUM_STATIC_OR_DYNAMIC_PART_TRIGGER)
		||  (txPathway->trigger == IPDUM_STATIC_PART_TRIGGER && txPart->type == IPDUM_STATIC)
		||  (txPathway->trigger == IPDUM_DYNAMIC_PART_TRIGGER && txPart->type == IPDUM_DYNAMIC));
}

/* @req 4.0.3/IPDUM015 */
/* @req 4.0.3/IPDUM017 */
/* @req 4.0.3/IPDUM021 */
/* @req 4.0.3/IPDUM019 */
/* @req 4.0.3/IPDUM020 */
/* @req 4.0.3/IPDUM152 */
/* @req 4.0.3/IPDUM168 */
/* @req 4.0.3/IPDUM023 */
//lint -esym(904, IpduM_Transmit) //PC-Lint Exception of rule 14.7
Std_ReturnType IpduM_Transmit(PduIdType IpduMTxPduId, const PduInfoType *PduInfoPtr) {
	// Is called with a COM IPdu ID
	Std_ReturnType status;
	VALIDATE_INIT(IPDUM_API_TRANSMIT);
	VALIDATE_PARAM_POINTER(IPDUM_API_TRANSMIT, PduInfoPtr);
	VALIDATE_TX_PART_ID(IPDUM_API_TRANSMIT, IpduMTxPduId);
	const IpduM_TxPart *txPart = &(IpduM_Config->txParts[IpduMTxPduId]);
	const IpduM_TxPathway *txPathway = txPart->txPathway;

	// Do not transmit if we are waiting for a tx confirmation
	if (txPathway->IpduMTxConfirmationTimeout > 0 && *(txPathway->timeoutCounter) > 0) {
		return E_NOT_OK;
	}

	// Copy the part that we received.
	/*lint -save -e522 */
	CopyTxPart(txPart, PduInfoPtr->SduDataPtr);
	/*lint -restore  */

	// If this is a dynamic part, set to the current active dynamic part.
	if (txPart->type == IPDUM_DYNAMIC) {
		(*(txPathway->activeDynamicPart)) = txPart;
	}

	// Should we trigger a transmit?
	if (!shouldTransmit(txPart)) {
		return E_OK;
	}

	// If this is the static part, then update the dynamic part if it has JIT.
	if (txPart->type == IPDUM_STATIC && (*(txPathway->activeDynamicPart))->IpduMJitUpdate) {
		status = FetchTxPart(*(txPathway->activeDynamicPart));
		// Need to do something with return value - seems appropiate to set DET error
		VALIDATE_NO_RETURN(status == E_OK, IPDUM_API_TRANSMIT, IPDUM_E_PARAM);
	}

	// If this is the dynamic part, then update the static part if it has JIT.
	if (txPart->type == IPDUM_DYNAMIC && HAS_STATIC_PART(txPathway) && txPathway->staticPart->IpduMJitUpdate) {
		status = FetchTxPart(txPathway->staticPart);
		// Need to do something with return value - seems appropiate to set DET error
		VALIDATE_NO_RETURN(status == E_OK, IPDUM_API_TRANSMIT, IPDUM_E_PARAM);
	}

	// Start timeout for TX-confirmation observation
	if (txPathway->IpduMTxConfirmationTimeout > 0) {
		*(txPathway->timeoutCounter) = txPathway->IpduMTxConfirmationTimeout;
	}

	// Trigger the transmit
	PduInfoType pduInfo = {
		.SduLength 		= PduInfoPtr->SduLength,
		.SduDataPtr		= txPathway->buffer
	};
	return PduR_IpduMTransmit(txPathway->IpduMOutgoingPduId, &pduInfo);
}


/* @req 4.0.3/IPDUM041 */
/* @req 4.0.3/IPDUM042 */
/* @req 4.0.3/IPDUM086 */
//lint -esym(904, IpduM_RxIndication) //PC-Lint Exception of rule 14.7
void IpduM_RxIndication(PduIdType RxPduId, PduInfoType *PduInfoPtr) {
	VALIDATE_INIT_NO_RV(IPDUM_API_RX_INDICATION);
	VALIDATE_PARAM_POINTER_NO_RV(IPDUM_API_RX_INDICATION, PduInfoPtr);
	VALIDATE_RX_ID_NO_RV(IPDUM_API_RX_INDICATION, RxPduId);
	if (PduInfoPtr->SduLength == 0) {
		return; // Silently ignore pdus of size 0. See chapter 7.5 of IpduM specification.
	}
	const IpduM_RxPathway *rxPath = &(IpduM_Config->rxPathways[RxPduId]);

	if (rxPath->hasStaticPart) {
		PduR_IpduMRxIndication(rxPath->IpduMOutgoingStaticPduId, PduInfoPtr);
	}

	uint8 mask = 0, i = 0;
	while (i < rxPath->IpduMSelectorFieldLength) {
		mask |= (uint8)(1u << i++);
	}
	uint8 selector = GETBITS(PduInfoPtr->SduDataPtr, rxPath->IpduMSelectorFieldPosition, mask);

	for (i = 0; i < rxPath->nDynamicParts; i++) {
		const IpduM_RxDynamicPart *dynamicPart = &(rxPath->dynamicParts[i]);
		if (dynamicPart->IpduMRxSelectorValue == selector) {
			PduR_IpduMRxIndication(dynamicPart->IpduMOutgoingDynamicPduId, PduInfoPtr);
			break;
		}
	}
}

/* @req 4.0.3/IPDUM088 */
/* @req 4.0.3/IPDUM087 */
/* @req 4.0.3/IPDUM022 */
/* @req 4.0.3/IPDUM024 */
//lint -esym(904, IpduM_TxConfirmation) //PC-Lint Exception of rule 14.7
void IpduM_TxConfirmation(PduIdType TxPduId) {
	VALIDATE_INIT_NO_RV(IPDUM_API_TX_CONFIRMATION);
	VALIDATE_TX_PATHWAY_ID_NO_RV(IPDUM_API_TX_CONFIRMATION, TxPduId);
	const IpduM_TxPathway *txPathway = &(IpduM_Config->txPathways[TxPduId]);

	// Ignore unexpected tx confirmations
	if (txPathway->IpduMTxConfirmationTimeout > 0 && *(txPathway->timeoutCounter) == 0) {
		return;
	}

	if ((*(txPathway->activeDynamicPart))->useConfirmation) {
		PduR_IpduMTxConfirmation((*(txPathway->activeDynamicPart))->pduId);
	}
	if (HAS_STATIC_PART(txPathway) && txPathway->staticPart->useConfirmation) {
		PduR_IpduMTxConfirmation(txPathway->staticPart->pduId);
	}

	// Rest tx confirmation timeout
	if (txPathway->IpduMTxConfirmationTimeout > 0) {
		imask_t state;
		Irq_Save(state);
		*(txPathway->timeoutCounter) = 0;
		Irq_Restore(state);
	}
}

/* @req 4.0.3/IPDUM090 */
/* @req 4.0.3/IPDUM091 */
/* @req 4.0.3/IPDUM089 */
/* @req 4.0.3/IPDUM169 */
//lint -esym(904, IpduM_TriggerTransmit) //PC-Lint Exception of rule 14.7
Std_ReturnType IpduM_TriggerTransmit(PduIdType TxPduId, PduInfoType *PduInfoPtr) {
	Std_ReturnType status;
	VALIDATE_INIT(IPDUM_API_TRIGGER_TRANSMIT);
	VALIDATE_PARAM_POINTER(IPDUM_API_TRIGGER_TRANSMIT, PduInfoPtr);
	VALIDATE_TX_PATHWAY_ID(IPDUM_API_TRIGGER_TRANSMIT, TxPduId);
	const IpduM_TxPathway *txPathway = &(IpduM_Config->txPathways[TxPduId]);

	imask_t state;
	Irq_Save(state);

	if ((*(txPathway->activeDynamicPart))->IpduMJitUpdate) {
		status = FetchTxPart(*(txPathway->activeDynamicPart));
		// Need to do something with return value - seems appropiate to set DET error
		VALIDATE_NO_RETURN(status == E_OK, IPDUM_API_TRIGGER_TRANSMIT, IPDUM_E_PARAM);
	}

	if (HAS_STATIC_PART(txPathway) && txPathway->staticPart->IpduMJitUpdate){
		status = FetchTxPart(txPathway->staticPart);
		// Need to do something with return value - seems appropiate to set DET error
		VALIDATE_NO_RETURN(status == E_OK, IPDUM_API_TRIGGER_TRANSMIT, IPDUM_E_PARAM);
	}

	memcpy(PduInfoPtr->SduDataPtr, txPathway->buffer, txPathway->pduSize);
	Irq_Restore(state);
	PduInfoPtr->SduLength = txPathway->pduSize;

	return E_OK;
}

/* @req 4.0.3/IPDUM101 */
//lint -esym(904, IpduM_MainFunction) //PC-Lint Exception of rule 14.7
void IpduM_MainFunction(void) {
	VALIDATE_INIT_NO_RV(IPDUM_API_MAIN);

	for (uint16 i = 0; i < IPDUM_N_TX_PATHWAYS; i++) {
		const IpduM_TxPathway *txPathway = &(IpduM_Config->txPathways[i]);

		imask_t state;
		Irq_Save(state);
		if (txPathway->IpduMTxConfirmationTimeout > 0 && *(txPathway->timeoutCounter) > 0) {
			(*(txPathway->timeoutCounter))--;
		}
		Irq_Restore(state);
	}
}
