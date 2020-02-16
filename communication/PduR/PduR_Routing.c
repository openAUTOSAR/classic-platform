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


#include "PduR.h"


#include <string.h>
#include "debug.h"
#if defined(USE_DEM)
#include "Dem.h"
#endif


#if PDUR_CANIF_SUPPORT == STD_ON
#include "CanIf.h"
#endif
#if PDUR_CANTP_SUPPORT == STD_ON
#include "CanTp.h"
#endif
#if PDUR_LINIF_SUPPORT == STD_ON
#include "LinIf.h"
#endif
#if PDUR_COM_SUPPORT == STD_ON
#include "Com.h"
#endif
#if PDUR_DCM_SUPPORT == STD_ON
#include "Dcm.h"
#endif
#if PDUR_SOAD_SUPPORT == STD_ON
#include "SoAd.h"
#endif
#if PDUR_J1939TP_SUPPORT == STD_ON
#include "J1939Tp.h"
#endif
#if PDUR_IPDUM_SUPPORT == STD_ON
#include "IpduM.h"
#endif
#if PDUR_FRIF_SUPPORT == STD_ON
#include "FrIf.h"
#endif
#if PDUR_FRTP_SUPPORT == STD_ON
#include "FrTp.h"
#endif

#if PDUR_ZERO_COST_OPERATION == STD_OFF

Std_ReturnType PduR_ARC_RouteTransmit(const PduRDestPdu_type * destination, const PduInfoType * pduInfo) {

    /* @req PDUR160 */
    /* @req PDUR629 */
    /* @req PDUR0745 */
    /* @req PDUR625 */

    Std_ReturnType retVal = E_NOT_OK;
    switch (destination->DestModule) {
    case ARC_PDUR_CANIF:
#if PDUR_CANIF_SUPPORT == STD_ON
        retVal = CanIf_Transmit(destination->DestPduId, pduInfo);
#endif
        break;
    case ARC_PDUR_COM:
#if PDUR_COM_SUPPORT == STD_ON
        Com_RxIndication(destination->DestPduId, (PduInfoType *)pduInfo);
#endif
        break;
    case ARC_PDUR_LINIF:
#if PDUR_LINIF_SUPPORT == STD_ON
        retVal = LinIf_Transmit(destination->DestPduId, pduInfo);
#endif
        break;
    case ARC_PDUR_CANTP:
#if PDUR_CANTP_SUPPORT == STD_ON
        retVal = CanTp_Transmit(destination->DestPduId, pduInfo);
#endif
        break;
    case ARC_PDUR_SOADIF:
#if PDUR_SOAD_SUPPORT == STD_ON
        retVal = SoAdIf_Transmit(destination->DestPduId, pduInfo);
#endif
        break;
    case ARC_PDUR_SOADTP:
#if PDUR_SOAD_SUPPORT == STD_ON
        retVal = SoAdTp_Transmit(destination->DestPduId, pduInfo);
#endif
        break;
    case ARC_PDUR_J1939TP:
#if PDUR_J1939TP_SUPPORT == STD_ON
        retVal = J1939Tp_Transmit(destination->DestPduId, pduInfo);
#endif
        break;
    case ARC_PDUR_IPDUM:
#if PDUR_IPDUM_SUPPORT == STD_ON
        retVal = IpduM_Transmit(destination->DestPduId, pduInfo);
#endif
        break;
    case ARC_PDUR_FRIF:
#if PDUR_FRIF_SUPPORT == STD_ON
        retVal = FrIf_Transmit(destination->DestPduId, pduInfo);
#endif
        break;
    case ARC_PDUR_FRTP:
#if PDUR_FRTP_SUPPORT == STD_ON
        retVal = FrTp_Transmit(destination->DestPduId, pduInfo);
#endif
        break;
    default:
        retVal = E_NOT_OK;
        break;
    }
    // TODO error reporting here.
    return retVal;
}

void PduR_ARC_RouteRxIndication(const PduRDestPdu_type * destination, const PduInfoType *PduInfo) {

	switch (destination->DestModule) {
	case ARC_PDUR_COM:
#if PDUR_COM_SUPPORT == STD_ON
		Com_RxIndication(destination->DestPduId, (PduInfoType *)PduInfo);
#endif
		break;
	case ARC_PDUR_DCM:
#if PDUR_DCM_SUPPORT == STD_ON
		Dcm_TpRxIndication(destination->DestPduId, *PduInfo->SduDataPtr);
#endif
		break;
	case ARC_PDUR_IPDUM:
#if PDUR_IPDUM_SUPPORT == STD_ON
		IpduM_RxIndication(destination->DestPduId, (PduInfoType *)PduInfo);
#endif
		break;
	default:
		break;
	}
	// TODO error reporting here.
}

void PduR_ARC_RouteTxConfirmation(const PduRRoutingPath_type *route, uint8 result) {

	/* @req PDUR627 */
	switch (route->SrcModule) {
	case ARC_PDUR_COM:
#if PDUR_COM_SUPPORT == STD_ON
		Com_TxConfirmation(route->SrcPduId);
#endif
		break;
	case ARC_PDUR_DCM:
#if PDUR_DCM_SUPPORT == STD_ON
		Dcm_TpTxConfirmation(route->SrcPduId, result);
#endif
		break;
	case ARC_PDUR_IPDUM:
#if PDUR_IPDUM_SUPPORT == STD_ON
		IpduM_TxConfirmation(route->SrcPduId);
#endif
		break;
	default:
		break;
	}
	// TODO error reporting here.
}

Std_ReturnType PduR_ARC_RouteTriggerTransmit(const PduRRoutingPath_type *route, PduInfoType * pduInfo) {
	Std_ReturnType retVal = E_NOT_OK;
	switch (route->SrcModule) {
	case ARC_PDUR_COM:
#if PDUR_COM_SUPPORT == STD_ON
		retVal = Com_TriggerTransmit(route->SrcPduId, pduInfo);
#endif
		break;
	case ARC_PDUR_IPDUM:
	#if PDUR_IPDUM_SUPPORT == STD_ON
		retVal = IpduM_TriggerTransmit(route->SrcPduId, pduInfo);
	#endif
			break;
	default:
		retVal = E_NOT_OK;
		break;
	}
	// TODO error reporting here.
	return retVal;
}

BufReq_ReturnType PduR_ARC_RouteCopyRxData(const PduRDestPdu_type * destination, PduInfoType* info, PduLengthType* bufferSizePtr) {
    BufReq_ReturnType retVal = BUFREQ_NOT_OK;
    switch (destination->DestModule) {
    case ARC_PDUR_DCM:
#if PDUR_DCM_SUPPORT == STD_ON
        retVal = Dcm_CopyRxData(destination->DestPduId, info, bufferSizePtr);
#endif
        break;
    default:
        break;
    }
    return retVal;
}

BufReq_ReturnType PduR_ARC_RouteCopyTxData(const PduRRoutingPath_type *route, PduInfoType* info, RetryInfoType* retry, PduLengthType* availableDataPtr) {
    BufReq_ReturnType retVal = BUFREQ_NOT_OK;
    switch (route->SrcModule) {
    case ARC_PDUR_DCM:
#if PDUR_DCM_SUPPORT == STD_ON
        retVal = Dcm_CopyTxData(route->SrcPduId, info, retry, availableDataPtr);
#endif
        break;
    default:
        break;
    }
    return retVal;
}

BufReq_ReturnType PduR_ARC_RouteStartOfReception(const PduRDestPdu_type * destination, PduLengthType TpSduLength, PduLengthType* bufferSizePtr) {
    BufReq_ReturnType retVal = BUFREQ_NOT_OK;
    switch (destination->DestModule) {
    case ARC_PDUR_DCM:
#if PDUR_DCM_SUPPORT == STD_ON
        retVal = Dcm_StartOfReception(destination->DestPduId, TpSduLength, bufferSizePtr);
#endif
        break;
    default:
        break;
    }
    return retVal;
}

#endif
