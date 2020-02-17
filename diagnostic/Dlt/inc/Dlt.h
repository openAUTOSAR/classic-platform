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

/*
 * Diagnostic Log & Trace module
 *
 */

/*
 *  General requirements
 */

#ifndef DLT_H
#define DLT_H

#include "Std_Types.h"
/** @req SWS_Dlt_00482 */
#include "Rte_Dlt_Type.h"
#if defined(USE_DEM)
#include "Dem.h"
#endif

#define DLT_MODULE_ID            55u
#define DLT_VENDOR_ID            60u

/* Implementation version */
#define DLT_SW_MAJOR_VERSION     1u
#define DLT_SW_MINOR_VERSION     0u
#define DLT_SW_PATCH_VERSION     0u

/* AUTOSAR specification document version */
#define DLT_AR_MAJOR_VERSION                4u
#define DLT_AR_MINOR_VERSION                0u
#define DLT_AR_RELEASE_REVISION_VERSION     3u

/* Error Codes */
/* @req SWS_Dlt_00447 */
#define DLT_E_WRONG_PARAMETERS              0x01u
#define DLT_E_ERROR_IN_PROV_SERVICE         0x02u
#define DLT_E_COM_FAILURE                   0x03u
#define DLT_E_ERROR_TO_MANY_CONTEXT         0x04u
#define DLT_E_MSG_LOOSE                     0x05u
#define DLT_E_PARAM_POINTER                 0x06u

/** Service id's */
#define DLT_INIT_SERVICE_ID                     0x01u /* @req SWS_Dlt_00239 */
#define DLT_GETVERSIONINFO_SERVICE_ID           0x02u /* @req SWS_Dlt_00271 */
#define DLT_DETFORWARDERRORTRACE_SERVICE_ID     0x07u /* @req SWS_Dlt_00432 */
#define DLT_DEMTRIGGERONEVENTSTATUS_SERVICE_ID  0x15u /* @req SWS_Dlt_00470 */
#define DLT_SENDLOGMESSAGE_SERVICE_ID           0x03u /* @req SWS_Dlt_00241 */

/* @req SWS_Dlt_00238 */
#define DLT_E_OK 0U
#define DLT_E_MSG_TOO_LARGE 1U
#define DLT_E_CONTEXT_ALREADY_REG 2U
#define DLT_E_UNKNOWN_SESSION_ID 3U
#define DLT_E_IF_NOT_AVAILABLE 4U
#define DLT_E_IF_BUSY 5U
#define DLT_E_ERROR_UNKNOWN 6U
#define DLT_E_PENDING 7U
#define DLT_E_NOT_IN_VERBOSE_MOD 8U

/* @req SWS_Dlt_00437 */
typedef struct {
    const char *EcuId;
} Dlt_ConfigType;

typedef enum {
    DLT_TYPE_LOG, DLT_TYPE_APP_TRACE, DLT_TYPE_NW_TRACE, DLT_TYPE_CONTROL,
} Dlt_MessageType;

/* @req SWS_Dlt_00458 */
/* @req SWS_Dlt_00094 */
/* @req SWS_Dlt_00319 */
/* @req SWS_Dlt_00320 */
typedef struct {
    uint8 HeaderType;
    uint8 MessageCounter;
    uint16 Length;
} Dlt_StandardHeaderType;

#define DLT_UEH  (1u<<0u)
#define DLT_MSBF (1u<<1u)
#define DLT_WEID (1u<<2u)
#define DLT_WSID (1u<<3u)
#define DLT_WTMS (1u<<4u)
#define DLT_VERS (1u<<5u)

typedef struct {
    uint8 MSIN;
    uint8 NOAR;
    uint8 APID[4];
    uint8 CTID[4];
} Dlt_ExtendedHeaderType;

#include "Dlt_cfg.h"
#include "Dlt_PBcfg.h"

/*
 * Implemented functions
 ****************************/

#if ( DLT_VERSION_INFO_API == STD_ON)
/**
 * Gets the version info
 * @param versioninfo - struct holding the version info
 */
/* @req SWS_Dlt_00271 */
#if ( DLT_VERSION_INFO_API == STD_ON )
#define Dlt_GetVersionInfo(_vi) STD_GET_VERSION_INFO(_vi,DLT)
#endif /* DLT_VERSION_INFO_API */
#endif

void Dlt_Init(const Dlt_ConfigType* ConfigPtr);
Dlt_ReturnType Dlt_SendLogMessage(Dlt_SessionIDType session_id, const Dlt_MessageLogInfoType* log_info,
        const uint8* log_data, uint16 log_data_length);

Dlt_ReturnType Dlt_SendTraceMessage(Dlt_SessionIDType session_id, const Dlt_MessageTraceInfoType* trace_info,
        const uint8* trace_data, uint16 trace_data_length);

Dlt_ReturnType Dlt_RegisterContext(Dlt_SessionIDType session_id, Dlt_ApplicationIDType app_id,
        Dlt_ContextIDType context_id, const uint8* app_description, uint8 len_app_description,
        const uint8* context_description, uint8 len_context_description);

#if defined(USE_DEM)
void Dlt_DemTriggerOnEventStatus(Dem_EventIdType EventId,
        Dem_EventStatusExtendedType EventStatusByteOld,
        Dem_EventStatusExtendedType EventStatusByteNew);
#endif

void Dlt_DetForwardErrorTrace(uint16 ModuleId, uint8 InstanceId, uint8 ApiId, uint8 ErrorId);

#include "ComStack_Types.h"
/* Interfaces provided by Dlt core module for internal use with Dlt
 communication module */
/* @req SWS_Dlt_00272 */
void Dlt_ComRxIndication(PduIdType DltRxPduId, Std_ReturnType Result);
/* @req SWS_Dlt_00273 */
void Dlt_ComTxConfirmation(PduIdType DltTxPduId, Std_ReturnType Result);
/* @req SWS_Dlt_00515 */
BufReq_ReturnType DltCom_CopyRxData(PduIdType id, const PduInfoType* pduInfoPtr, PduLengthType* bufferSizePtr);
/* @req SWS_Dlt_00516 */
BufReq_ReturnType DltCom_CopyTxData(PduIdType id, const PduInfoType* pduInfoPtr, RetryInfoType* retryInfoPtr,
        PduLengthType* availableDataPtr);
/* @req SWS_Dlt_00517 */
BufReq_ReturnType DltCom_StartOfReception(PduIdType id, const PduInfoType* infoPtr, PduLengthType tpSduLength,
        PduLengthType* bufferSizePtr);
/* @req SWS_Dlt_00263 */
Std_ReturnType DltCom_Transmit(PduIdType DltTxPduId, const PduInfoType* PduInfoPtr);
/* !req SWS_Dlt_00264 */
BufReq_ReturnType DltCom_CancelTransmitRequest(PduIdType id);

/* !req SWS_Dlt_00265 */
Dlt_ReturnType DltCom_SetInterfaceStatus(uint8 com_interface[4], uint8 new_status);

Std_ReturnType DltCom_ReceiveIndication(PduIdType DltRxPduId, const PduInfoType* PduInfoPtr);

void DltCom_Init(void);

#define DLT_RESP_OK 0U
#define DLT_RESP_NOT_SUPPORTED 1U
#define DLT_RESP_ERROR 2U

#define DLT_ARC_MAGIC_CONTROL_NUMBER 0xCCCC
#define DLT_DEM_MESSAGE_ID 0x00000001u
#define DLT_DET_MESSAGE_ID 0x00000002u

void Dlt_ArcProcessIncomingMessage(const Dlt_StandardHeaderType *header, const uint8 *payload);

boolean Dlt_ArcIsDltConnected(void);

#endif /* DLT_H */
