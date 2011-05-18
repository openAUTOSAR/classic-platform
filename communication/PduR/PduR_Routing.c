/* -------------------------------- Arctic Core ------------------------------
 * Arctic Core - the open source AUTOSAR platform http://arccore.com
 *
 * Copyright (C) 2009  ArcCore AB <contact@arccore.com>
 *
 * This source code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by the
 * Free Software Foundation; See <http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 * -------------------------------- Arctic Core ------------------------------*/


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


#if PDUR_ZERO_COST_OPERATION == STD_OFF

Std_ReturnType PduR_ARC_RouteTransmit(const PduRDestPdu_type * destination, const PduInfoType * pduInfo) {

	switch (destination->DestModule) {
	case ARC_PDUR_CANIF:
#if PDUR_CANIF_SUPPORT == STD_ON
		return CanIf_Transmit(destination->DestPduId, pduInfo);
#endif
		break;
	case ARC_PDUR_LINIF:
#if PDUR_LINIF_SUPPORT == STD_ON
		return LinIf_Transmit(destination->DestPduId, pduInfo);
#endif
		break;
	case ARC_PDUR_CANTP:
#if PDUR_CANTP_SUPPORT == STD_ON
		return CanTp_Transmit(destination->DestPduId, pduInfo);
#endif
		break;
	case ARC_PDUR_SOADIF:
#if PDUR_SOAD_SUPPORT == STD_ON
		return SoAdIf_Transmit(destination->DestPduId, pduInfo);
#endif
		break;
	case ARC_PDUR_SOADTP:
#if PDUR_SOAD_SUPPORT == STD_ON
		return SoAdTp_Transmit(destination->DestPduId, pduInfo);
#endif
		break;
	default:
		break;
	}
	// TODO error reporting here.
	return E_NOT_OK;
}

void PduR_ARC_RouteRxIndication(const PduRDestPdu_type * destination, const PduInfoType *PduInfo) {

	switch (destination->DestModule) {
	case ARC_PDUR_COM:
#if PDUR_COM_SUPPORT == STD_ON
		Com_RxIndication(destination->DestPduId, PduInfo);
#endif
		break;
	case ARC_PDUR_DCM:
#if PDUR_DCM_SUPPORT == STD_ON
		Dcm_RxIndication(destination->DestPduId, *PduInfo->SduDataPtr);
#endif
		break;
	default:
		break;
	}
	// TODO error reporting here.
}

void PduR_ARC_RouteTxConfirmation(const PduRRoutingPath_type *route, uint8 result) {
	switch (route->SrcModule) {
	case ARC_PDUR_COM:
#if PDUR_COM_SUPPORT == STD_ON
		Com_TxConfirmation(route->SrcPduId);
#endif
		break;
	case ARC_PDUR_DCM:
#if PDUR_DCM_SUPPORT == STD_ON
		Dcm_TxConfirmation(route->SrcPduId, result);
#endif
		break;
	default:
		break;
	}
	// TODO error reporting here.
}

Std_ReturnType PduR_ARC_RouteTriggerTransmit(const PduRRoutingPath_type *route, PduInfoType * pduInfo) {
	switch (route->SrcModule) {
	case ARC_PDUR_COM:
#if PDUR_COM_SUPPORT == STD_ON
		return Com_TriggerTransmit(route->SrcPduId, pduInfo);
#endif
		break;
	default:
		break;
	}
	// TODO error reporting here.
	return E_NOT_OK;
}

BufReq_ReturnType PduR_ARC_RouteProvideRxBuffer(const PduRDestPdu_type * destination, PduLengthType TpSduLength, PduInfoType** PduInfoPtr) {
	switch (destination->DestModule) {
	case ARC_PDUR_DCM:
#if PDUR_DCM_SUPPORT == STD_ON
		return Dcm_ProvideRxBuffer(destination->DestPduId, TpSduLength, PduInfoPtr);
#endif
		break;
	default:
		break;
	}
	// TODO error reporting here.
	return BUFREQ_NOT_OK;
}

BufReq_ReturnType PduR_ARC_RouteProvideTxBuffer(const PduRRoutingPath_type *route, PduLengthType TpSduLength, PduInfoType** PduInfoPtr) {
	switch (route->SrcModule) {
	case ARC_PDUR_DCM:
#if PDUR_DCM_SUPPORT == STD_ON
		return Dcm_ProvideTxBuffer(route->SrcPduId, PduInfoPtr, TpSduLength);
#endif
		break;
	default:
		break;
	}
	// TODO error reporting here.
	return BUFREQ_NOT_OK;
}

#endif
