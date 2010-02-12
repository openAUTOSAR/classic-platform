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

#define PDUR_VENDOR_ID			    1
#define PDUR_AR_MAJOR_VERSION  2
#define PDUR_AR_MINOR_VERSION 	2
#define PDUR_AR_PATCH_VERSION	2
#define PDUR_SW_MAJOR_VERSION  3
#define PDUR_SW_MINOR_VERSION 	0
#define PDUR_SW_PATCH_VERSION	2

#include "Trace.h"

#include "PduR_Cfg.h"
#include "PduR_Types.h"

#ifndef PDUR_ZERO_COST_OPERATION
#include "PduR_PbCfg.h"
#endif

#include "PduR_Com.h"
#include "PduR_CanIf.h"
#include "PduR_LinIf.h"

/* Contain the current state of the PDU router. The router is uninitialized
 * until PduR_Init has been run.
 */
PduR_StateType PduRState;

extern const PduR_PBConfigType *PduRConfig;


#ifdef PDUR_PRINT_DEBUG_STATEMENTS
/* A simple debug macro to be used instead of printf(). This way all print
 * statements are turned off if PDUR_PRINT_DEBUG_STATEMENTS is undefined.
 */
//#include <stdio.h>
#define debug(...) simple_printf(__VA_ARGS__)

#else
#define debug(...)

#endif

#ifdef PDUR_REENTRANCY_CHECK
/*
 * The macros Enter and Exit performs the ReEntrancy check of the PDU router functions.
 * Enter shall be called at the beginning of the function with the current PduId and the wanted
 * return value (possibly nothing for void methods).
 * Exit should be called at the end of the function where reentrancy is desirable.
 */
#define Enter(PduId,...) \
	static uint8 entered;\
	static PduIdType enteredId;\
	if (entered && enteredId == PduId) { \
		debug("Function already entered. EnteredId: %d, CurrentId: %d. Exiting.\n", enteredId, PduId); \
		return __VA_ARGS__; \
	} else { \
		entered = 1; \
		enteredId = PduId; \
	} \


#define Exit() \
	entered = 0; \

#else
#define Enter(...)
#define Exit()
#endif

#ifdef PDUR_DEV_ERROR_DETECT

#undef DET_REPORTERROR
#define DET_REPORTERROR(_x,_y,_z,_q) Det_ReportError(_x,_y,_z,_q)

// Define macro for state, parameter and data pointer checks.
// TODO Implement data range check if needed.
#define DevCheck(PduId,PduPtr,ApiId,...) \
	if (PduRState == PDUR_UNINIT || PduRState == PDUR_REDUCED) { \
		DET_REPORTERROR(PDUR_MODULE_ID, PDUR_INSTANCE_ID, ApiId, PDUR_E_INVALID_REQUEST); \
		DEBUG(DEBUG_LOW,"PDU Router not initialized. Routing request ignored.\n"); \
		Exit(); \
		return __VA_ARGS__; \
	} \
	if (PduPtr == 0 && PDUR_DEV_ERROR_DETECT) { \
		DET_REPORTERROR(PDUR_MODULE_ID, PDUR_INSTANCE_ID, ApiId, PDUR_E_DATA_PTR_INVALID); \
		Exit(); \
		return __VA_ARGS__; \
	} \
	if ((PduId >= PduRConfig->PduRRoutingTable->NRoutingPaths) && PDUR_DEV_ERROR_DETECT) { \
		DET_REPORTERROR(PDUR_MODULE_ID, PDUR_INSTANCE_ID, ApiId, PDUR_E_PDU_ID_INVALID); \
		Exit(); \
		return __VA_ARGS__; \
	} \


#else
#undef DET_REPORTERROR
#define DET_REPORTERROR(_x,_y,_z,_q)
#define DevCheck(...)

#endif

Std_ReturnType PduR_CancelTransmitRequest(
		PduR_CancelReasonType PduCancelReason, PduIdType PduId);
void PduR_ChangeParameterRequest(PduR_ParameterValueType PduParameterValue,
		PduIdType PduId);

/* Zero Cost Operation function definitions
 * These macros replaces the original functions if zero cost
 * operation is desired. */
#ifdef PDUR_ZERO_COST_OPERATION
#define PduR_Init(...)
#define PduR_GetVersionInfo(...)
#define PduR_GetConfigurationId(...) 0

#else // Not zero cost operation
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
