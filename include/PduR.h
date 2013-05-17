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








#ifndef PDUR_H
#define PDUR_H

#include "Modules.h"

#define PDUR_VENDOR_ID		   VENDOR_ID_ARCCORE
#define PDUR_AR_MAJOR_VERSION  3
#define PDUR_AR_MINOR_VERSION  1
#define PDUR_AR_PATCH_VERSION  5
#define PDUR_SW_MAJOR_VERSION  2
#define PDUR_SW_MINOR_VERSION  0
#define PDUR_SW_PATCH_VERSION  0



// ERROR CODES
#define PDUR_E_CONFIG_PTR_INVALID 		0x06
#define PDUR_E_INVALID_REQUEST 			0x01
#define PDUR_E_PDU_ID_INVALID			0x02
#define PDUR_E_TP_TX_REQ_REJECTED		0x03
#define PDUR_E_DATA_PTR_INVALID			0x05
#define PDUR_E_BUFFER_ERROR				0x06

#define PDUR_INSTANCE_ID	0

#include "Modules.h"

#include "PduR_Cfg.h"
#include "PduR_Types.h"
#include "PduR_PbCfg.h"

#include "PduR_If.h"

#if PDUR_COM_SUPPORT == STD_ON
#include "PduR_Com.h"
#endif
#if PDUR_CANIF_SUPPORT == STD_ON
#include "PduR_CanIf.h"
#endif
#if PDUR_LINIF_SUPPORT == STD_ON
#include "PduR_LinIf.h"
#endif
#if PDUR_CANTP_SUPPORT == STD_ON
#include "PduR_CanTp.h"
#endif
#if PDUR_J1939TP_SUPPORT == STD_ON
#include "PduR_J1939Tp.h"
#endif
#if PDUR_DCM_SUPPORT == STD_ON
#include "PduR_Dcm.h"
#endif
#if PDUR_SOAD_SUPPORT == STD_ON
#include "PduR_SoAd.h"
#endif
#if PDUR_J1939TP_SUPPORT == STD_ON
#include "PduR_J1939Tp.h"
#endif


/* Contain the current state of the PDU router. The router is uninitialized
 * until PduR_Init has been run.
 */
//PduR_StateType PduRState;

extern const PduR_PBConfigType *PduRConfig;

/*
 * The state of the PDU router.
 */
extern PduR_StateType PduRState;

#define PduR_IsUpModule(_mod) ((_mod > ARC_PDUR_UP_MODULES) && (_mod < ARC_PDUR_LOIF_MODULES))
#define PduR_IsIfModule(_mod) ((_mod > ARC_PDUR_LOIF_MODULES) && (_mod < ARC_PDUR_LOTP_MODULES))
#define PduR_IsTpModule(_mod) ((_mod > ARC_PDUR_LOTP_MODULES) && (_mod < ARC_PDUR_END_OF_MODULES))
#define PduR_IsLoModule(_mod) (PduR_IsIfModule(_mod) || PduR_IsTpModule(_mod))


#if (PDUR_DEV_ERROR_DETECT == STD_ON)

#define PDUR_DET_REPORTERROR(_x,_y,_z,_o) Det_ReportError(_x,_y,_z,_o)

#define PDUR_VALIDATE_INITIALIZED(_api,...) \
	if ((PduRState == PDUR_UNINIT) || (PduRState == PDUR_REDUCED)) { \
		Det_ReportError(MODULE_ID_PDUR, PDUR_INSTANCE_ID, _api, PDUR_E_INVALID_REQUEST); \
		return __VA_ARGS__; \
	}

#define PDUR_VALIDATE_PDUPTR(_api, _pduPtr, ...) \
	if ((_pduPtr == NULL) && (PDUR_DEV_ERROR_DETECT)) { \
		Det_ReportError(MODULE_ID_PDUR, PDUR_INSTANCE_ID, _api, PDUR_E_DATA_PTR_INVALID); \
		return __VA_ARGS__; \
	}

#define PDUR_VALIDATE_PDUID(_api, _pduId, ...) \
	if ((_pduId >= PduRConfig->NRoutingPaths) && PDUR_DEV_ERROR_DETECT) { \
		Det_ReportError(MODULE_ID_PDUR, PDUR_INSTANCE_ID, _api, PDUR_E_PDU_ID_INVALID); \
		return __VA_ARGS__; \
	}


#else
#define PDUR_DET_REPORTERROR(_x,_y,_z,_o)
#define PDUR_VALIDATE_INITIALIZED(_api,...)
#define PDUR_VALIDATE_PDUPTR(_api, _pduPtr, ...)
#define PDUR_VALIDATE_PDUID(_api, _pduId, ...)

#endif

Std_ReturnType PduR_CancelTransmitRequest(
		PduR_CancelReasonType PduCancelReason, PduIdType PduId);
void PduR_ChangeParameterRequest(PduR_ParameterValueType PduParameterValue,
		PduIdType PduId);

/* Zero Cost Operation function definitions
 * These macros replaces the original functions if zero cost
 * operation is desired. */
#if PDUR_ZERO_COST_OPERATION == STD_ON
#define PduR_Init(...)
#define PduR_GetVersionInfo(...)
#define PduR_GetConfigurationId(...) 0

#else // Not zero cost operation
//#error fail
void PduR_Init(const PduR_PBConfigType* ConfigPtr);
void PduR_GetVersionInfo(Std_VersionInfoType* versionInfo);
uint32 PduR_GetConfigurationId(void);

void PduR_BufferInc(PduRTxBuffer_type *Buffer, uint8 **ptr);
void PduR_BufferQueue(PduRTxBuffer_type *Buffer, const uint8 * SduPtr);
void PduR_BufferDeQueue(PduRTxBuffer_type *Buffer, uint8 *SduPtr);
void PduR_BufferFlush(PduRTxBuffer_type *Buffer);
uint8 PduR_BufferIsFull(PduRTxBuffer_type *Buffer);
void PduR_LoIfRxIndication(PduIdType PduId, const uint8* SduPtr);
void PduR_LoIfTxConfirmation(PduIdType PduId);
void PduR_LoIfTriggerTransmit(PduIdType PduId, uint8* SduPtr);

/*
 * Macros
 */
#define setTxConfP(_B) (_B->TxConfP = 1)
#define clearTxConfP(_B) (_B->TxConfP = 0)

#endif

#endif /* PDUR_H */
