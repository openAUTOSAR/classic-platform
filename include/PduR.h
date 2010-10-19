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








#ifndef _PDUR_H_
#define _PDUR_H_

#define PDUR_VENDOR_ID			1
#define PDUR_AR_MAJOR_VERSION  2
#define PDUR_AR_MINOR_VERSION  2
#define PDUR_AR_PATCH_VERSION  2
#define PDUR_SW_MAJOR_VERSION  1
#define PDUR_SW_MINOR_VERSION  0
#define PDUR_SW_PATCH_VERSION  0



// ERROR CODES
#define PDUR_E_CONFIG_PTR_INVALID 	0x06
#define PDUR_E_INVALID_REQUEST 		0x01
#define PDUR_E_PDU_ID_INVALID		0x02
#define PDUR_E_TP_TX_REQ_REJECTED	0x03
#define PDUR_E_DATA_PTR_INVALID		0x05

#define PDUR_INSTANCE_ID	0

#include "Modules.h"

#include "PduR_Cfg.h"
#include "PduR_Types.h"
#include "PduR_PbCfg.h"

#include "PduR_Com.h"
#include "PduR_CanIf.h"
#include "PduR_LinIf.h"
#include "PduR_CanTp.h"
#include "PduR_Dcm.h"



/* Contain the current state of the PDU router. The router is uninitialized
 * until PduR_Init has been run.
 */
PduR_StateType PduRState;

extern const PduR_PBConfigType *PduRConfig;


#if (PDUR_DEV_ERROR_DETECT == STD_ON)

#define PDUR_DET_REPORTERROR(_x,_y,_z,_q) Det_ReportError(_x,_y,_z,_q)

// Define macro for state, parameter and data pointer checks.
// TODO Implement data range check if needed.
#define PduR_DevCheck(PduId,PduPtr,ApiId,...) \
	if (PduRState == PDUR_UNINIT || PduRState == PDUR_REDUCED) { \
		PDUR_DET_REPORTERROR(MODULE_ID_PDUR, PDUR_INSTANCE_ID, ApiId, PDUR_E_INVALID_REQUEST); \
		return __VA_ARGS__; \
	} \
	if (PduPtr == 0 && PDUR_DEV_ERROR_DETECT) { \
		PDUR_DET_REPORTERROR(MODULE_ID_PDUR, PDUR_INSTANCE_ID, ApiId, PDUR_E_DATA_PTR_INVALID); \
		return __VA_ARGS__; \
	} \
	if ((PduId >= PduRConfig->PduRRoutingTable->NRoutingPaths) && PDUR_DEV_ERROR_DETECT) { \
		PDUR_DET_REPORTERROR(MODULE_ID_PDUR, PDUR_INSTANCE_ID, ApiId, PDUR_E_PDU_ID_INVALID); \
		return __VA_ARGS__; \
	}


#else
#define PDUR_DET_REPORTERROR(_x,_y,_z,_q)
#define PduR_DevCheck(...)

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
uint32 PduR_GetConfigurationId();

void PduR_BufferQueue(PduRTxBuffer_type *Buffer, const uint8 * SduPtr);
void PduR_BufferDeQueue(PduRTxBuffer_type *Buffer, uint8 *SduPtr);
void PduR_BufferFlush(PduRTxBuffer_type *Buffer);
uint8 PduR_BufferIsFull(PduRTxBuffer_type *Buffer);

/*
 * Macros
 */
#define setTxConfP(R) R->PduRDestPdu.TxBufferRef->TxConfP = 1
#define clearTxConfP(R) R->PduRDestPdu.TxBufferRef->TxConfP = 0

#endif

extern PduR_FctPtrType PduR_StdCanFctPtrs;
extern PduR_FctPtrType PduR_StdLinFctPtrs;

#endif /* _PDUR_H_ */
