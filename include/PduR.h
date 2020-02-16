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


/* @req PDUR292 */
/* @req PDUR0778 */

#ifndef PDUR_H
#define PDUR_H

#include "Modules.h"

#define PDUR_VENDOR_ID		   VENDOR_ID_ARCCORE
#define PDUR_AR_RELEASE_MAJOR_VERSION      4
#define PDUR_AR_RELEASE_MINOR_VERSION      0
#define PDUR_AR_RELEASE_REVISION_VERSION   3

#define PDUR_MODULE_ID         MODULE_ID_PDUR
#define PDUR_AR_MAJOR_VERSION  PDUR_AR_RELEASE_MAJOR_VERSION
#define PDUR_AR_MINOR_VERSION  PDUR_AR_RELEASE_MINOR_VERSION
#define PDUR_AR_PATCH_VERSION  PDUR_AR_RELEASE_REVISION_VERSION
#define PDUR_SW_MAJOR_VERSION  1
#define PDUR_SW_MINOR_VERSION  2
#define PDUR_SW_PATCH_VERSION  0



// ERROR CODES
/* @req PDUR231 */
#define PDUR_E_CONFIG_PTR_INVALID 				0x00
#define PDUR_E_INVALID_REQUEST 					0x01
#define PDUR_E_PDU_ID_INVALID					0x02
#define PDUR_E_TP_TX_REQ_REJECTED				0x03
#define PDUR_E_PARAM_INVALID        			0x04
#define PDUR_E_DUPLICATE_IPDU_ID    			0x06
#define PDUR_E_IPDU_TOO_LONG					0x07
#define PDUR_E_ROUTING_PATH_GROUP_ID_INVALID  	0x08
#define PDUR_E_NULL_POINTER         			0x09
#define PDUR_E_PDU_INSTANCES_LOST				0x0a
#define PDUR_E_INIT_FAILED						0x0b
#define PDUR_E_DATA_PTR_INVALID			        PDUR_E_NULL_POINTER
#define PDUR_E_BUFFER_ERROR				        0x0c

#define PDUR_INSTANCE_ID	0

#define PDUR_SERVICEID_CANIFRXINDICATION        0x00

/* @req PDUR132 */

#include "PduR_Types.h"
#include "PduR_Cfg.h"

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
#if PDUR_IPDUM_SUPPORT == STD_ON
#include "PduR_IpduM.h"
#endif
#if PDUR_FRIF_SUPPORT == STD_ON
#include "PduR_FrIf.h"
#endif
#if PDUR_FRTP_SUPPORT == STD_ON
#include "PduR_FrTp.h"
#endif
#if PDUR_LINTP_SUPPORT == STD_ON
#include "PduR_LinTp.h"
#endif


/* Contain the current state of the PDU router. The router is uninitialized
 * until PduR_Init has been run.
 */
//PduR_StateType PduRState;

extern const PduR_PBConfigType *PduRConfig;

/*
 *  Allocated RAM buffers for PduR
 */
extern const PduR_RamBufCfgType PduR_RamBufCfg;

/*
 * The state of the PDU router.
 */
extern PduR_StateType PduRState;

#define PduR_IsUpModule(_mod) ((_mod > ARC_PDUR_UP_MODULES) && (_mod < ARC_PDUR_LOIF_MODULES))
#define PduR_IsIfModule(_mod) ((_mod > ARC_PDUR_LOIF_MODULES) && (_mod < ARC_PDUR_LOTP_MODULES))
#define PduR_IsTpModule(_mod) ((_mod > ARC_PDUR_LOTP_MODULES) && (_mod < ARC_PDUR_END_OF_MODULES))
#define PduR_IsLoModule(_mod) (PduR_IsIfModule(_mod) || PduR_IsTpModule(_mod))


/* @req PDUR101 */
/* @req PDUR331 */
#if (PDUR_DEV_ERROR_DETECT == STD_ON)

#define PDUR_DET_REPORTERROR(_x,_y,_z,_o) Det_ReportError(_x,_y,_z,_o)

#define PDUR_VALIDATE_INITIALIZED(_api,...) \
	if ((PduRState == PDUR_UNINIT) || (PduRState == PDUR_REDUCED)) { \
		Det_ReportError(MODULE_ID_PDUR, PDUR_INSTANCE_ID, _api, PDUR_E_INVALID_REQUEST); \
		return __VA_ARGS__; \
	}

#define PDUR_VALIDATE_PDUPTR(_api, _pduPtr, ...) \
	if ((_pduPtr == NULL) && (PDUR_DEV_ERROR_DETECT)) { \
		Det_ReportError(MODULE_ID_PDUR, PDUR_INSTANCE_ID, _api, PDUR_E_NULL_POINTER); \
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

/* Zero Cost Operation function definitions
 * These macros replaces the original functions if zero cost
 * operation is desired. */
#if PDUR_ZERO_COST_OPERATION == STD_ON
#define PduR_Init(...)
#define PduR_GetVersionInfo(...)
#define PduR_GetConfigurationId(...) 0

#else // Not zero cost operation
//#error fail

/* @req PDUR334 */
void PduR_Init(const PduR_PBConfigType* ConfigPtr);

#if PDUR_VERSION_INFO_API == STD_ON
void PduR_GetVersionInfo(Std_VersionInfoType* versionInfo);
#endif

uint32 PduR_GetConfigurationId(void);
void PduR_EnableRouting(PduR_RoutingPathGroupIdType id);
void PduR_DisableRouting(PduR_RoutingPathGroupIdType id);

void PduR_BufferInc(PduRTxBuffer_type *Buffer, uint8 **ptr);
void PduR_BufferQueue(PduRTxBuffer_type *Buffer, const uint8 * SduPtr);
void PduR_BufferDeQueue(PduRTxBuffer_type *Buffer, uint8 *SduPtr);
void PduR_BufferFlush(PduRTxBuffer_type *Buffer);
uint8 PduR_BufferIsFull(PduRTxBuffer_type *Buffer);

/*
 * Macros
 */
#define setTxConfP(_B) (_B->TxConfP = 1)
#define clearTxConfP(_B) (_B->TxConfP = 0)

#endif

#endif /* PDUR_H */
