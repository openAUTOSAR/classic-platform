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
 * NB! This file is for DCM internal use only and may only be included from DCM C-files!
 */



#ifndef DCM_INTERNAL_H_
#define DCM_INTERNAL_H_

#include "Dcm.h"
#if defined(DCM_USE_SERVICE_INPUTOUTPUTCONTROLBYIDENTIFIER) && (0 < DCM_NOF_IOCONTROL_DIDS)
#define DCM_USE_CONTROL_DIDS
#endif

#if  ( DCM_DEV_ERROR_DETECT == STD_ON )
#if defined(USE_DET)
#include "Det.h"
#endif


#define DCM_DET_REPORTERROR(_api,_err)                              \
        (void)Det_ReportError(DCM_MODULE_ID, 0, _api, _err);    \

#else

#define DCM_DET_REPORTERROR(_api,_err)

#endif

#define VALIDATE(_exp,_api,_err ) \
        if(!(_exp)) { \
          DCM_DET_REPORTERROR(_api, _err); \
          return E_NOT_OK; \
        }

#define VALIDATE_RV(_exp,_api,_err,_rv ) \
        if(!(_exp)) { \
          DCM_DET_REPORTERROR(_api, _err); \
          return _rv; \
        }

#define VALIDATE_NO_RV(_exp,_api,_err ) \
        if(!(_exp) ){ \
          DCM_DET_REPORTERROR(_api, _err); \
          return; \
        }
        
#define IS_PERIODIC_TX_PDU(_x) (((_x) >= DCM_FIRST_PERIODIC_TX_PDU) && ((_x) < (DCM_FIRST_PERIODIC_TX_PDU + DCM_NOF_PERIODIC_TX_PDU)))
#define TO_INTERNAL_PERIODIC_PDU(_x) ((_x) - DCM_FIRST_PERIODIC_TX_PDU)

// SID table
#define SID_DIAGNOSTIC_SESSION_CONTROL			0x10
#define SID_ECU_RESET							0x11
#define SID_CLEAR_DIAGNOSTIC_INFORMATION		0x14
#define SID_READ_DTC_INFORMATION				0x19
#define SID_READ_DATA_BY_IDENTIFIER				0x22
#define SID_READ_MEMORY_BY_ADDRESS				0x23
#define SID_READ_SCALING_DATA_BY_IDENTIFIER		0x24
#define SID_SECURITY_ACCESS						0x27
#define SID_COMMUNICATION_CONTROL               0x28
#define SID_READ_DATA_BY_PERIODIC_IDENTIFIER	0x2A
#define SID_DYNAMICALLY_DEFINE_DATA_IDENTIFIER	0x2C
#define SID_WRITE_DATA_BY_IDENTIFIER			0x2E
#define SID_INPUT_OUTPUT_CONTROL_BY_IDENTIFIER	0x2F
#define SID_ROUTINE_CONTROL						0x31
#define SID_WRITE_MEMORY_BY_ADDRESS				0x3D
#define SID_TESTER_PRESENT						0x3E
#define SID_NEGATIVE_RESPONSE					0x7F
#define SID_CONTROL_DTC_SETTING					0x85
#define SID_RESPONSE_ON_EVENT                   0x86
#define SID_LINK_CONTROL                        0x87

#define SID_REQUEST_DOWNLOAD					0x34
#define SID_REQUEST_UPLOAD						0x35
#define SID_TRANSFER_DATA						0x36
#define SID_REQUEST_TRANSFER_EXIT				0x37

//OBD SID TABLE
#define SID_REQUEST_CURRENT_POWERTRAIN_DIAGNOSTIC_DATA		0x01
#define SID_REQUEST_POWERTRAIN_FREEZE_FRAME_DATA			0x02
#define SID_CLEAR_EMISSION_RELATED_DIAGNOSTIC_INFORMATION	0x04
#define SID_REQUEST_EMISSION_RELATED_DIAGNOSTIC_TROUBLE_CODES		0x03
#define SID_REQUEST_ON_BOARD_MONITORING_TEST_RESULTS_SPECIFIC_MONITORED_SYSTEMS		0x06
#define SID_REQUEST_EMISSION_RELATED_DIAGNOSTIC_TROUBLE_CODES_DETECTED_DURING_CURRENT_OR_LAST_COMPLETED_DRIVING_CYCLE	0x07
#define SID_REQUEST_VEHICLE_INFORMATION		0x09
#define SID_REQUEST_EMISSION_RELATED_DIAGNOSTIC_TROUBLE_CODES_WITH_PERMANENT_STATUS 0x0A

// Misc definitions
#define SUPPRESS_POS_RESP_BIT		(uint8)0x80
#define SID_RESPONSE_BIT			(uint8)0x40
#define VALUE_IS_NOT_USED			(uint8)0x00

// This diag request error code is not an error and is therefore not allowed to be used by SWCs, only used internally in DCM.
// It is therefore not defined in Rte_Dcm.h
#define DCM_E_RESPONSEPENDING       ((Dcm_NegativeResponseCodeType)0x78)
#define DCM_E_FORCE_RCRRP ((Dcm_NegativeResponseCodeType)0x01)/* Not an actual response code. Used internally */

#define DID_IS_IN_DYNAMIC_RANGE(_x) (((_x) >= 0xF200) && ((_x) <= 0xF3FF))

typedef enum {
    DSD_TX_RESPONSE_READY,
    DSD_TX_RESPONSE_SUPPRESSED
} DsdProcessingDoneResultType;


typedef enum {
    DCM_READ_DID_IDLE,
    DCM_READ_DID_PENDING_COND_CHECK,
    DCM_READ_DID_PENDING_READ_DATA
} ReadDidPendingStateType;

typedef enum {
    DCM_NO_COM = 0,
    DCM_SILENT_COM,
    DCM_FULL_COM
}DcmComModeType;

typedef enum {
    DCM_REQ_DSP = 0,
#if defined(DCM_USE_SERVICE_RESPONSEONEVENT) && defined(USE_NVM)
    DCM_REQ_ROE,
#endif
    DCM_NOF_REQUESTERS
}DcmProtocolRequesterType;
/*
 * DSP
 */
void DspInit(boolean firstCall);
void DspMain(void);
void DspPreDsdMain(void);
void DspTimerMain(void);
void DspUdsDiagnosticSessionControl(const PduInfoType *pduRxData, PduIdType txPduId, PduInfoType *pduTxData, boolean respPendOnTransToBoot, boolean internalStartupRequest);
#if defined(DCM_USE_SERVICE_LINKCONTROL)
void DspUdsLinkControl(const PduInfoType *pduRxData, PduIdType txPduId, PduInfoType *pduTxData, boolean respPendOnTransToBoot);
#endif
void DspUdsEcuReset(const PduInfoType *pduRxData, PduIdType txPduId, PduInfoType *pduTxData, boolean startupResponseRequest);
void DspUdsClearDiagnosticInformation(const PduInfoType *pduRxData, PduInfoType *pduTxData);
void DspUdsSecurityAccess(const PduInfoType *pduRxData, PduInfoType *pduTxData);
void DspUdsTesterPresent(const PduInfoType *pduRxData, PduInfoType *pduTxData);
void DspUdsReadDtcInformation(const PduInfoType *pduRxData, PduInfoType *pduTxData);
void DspUdsReadDataByIdentifier(const PduInfoType *pduRxData, PduInfoType *pduTxData);
void DspUdsReadScalingDataByIdentifier(const PduInfoType *pduRxData, PduInfoType *pduTxData);
#ifdef DCM_USE_SERVICE_WRITEDATABYIDENTIFIER
void DspUdsWriteDataByIdentifier(const PduInfoType *pduRxData, PduInfoType *pduTxData);
#endif
void DspUdsControlDtcSetting(const PduInfoType *pduRxData, PduInfoType *pduTxData);
void DspResponseOnEvent(const PduInfoType *pduRxData, PduIdType rxPduId, PduInfoType *pduTxData);
void DspUdsRoutineControl(const PduInfoType *pduRxData, PduInfoType *pduTxData);
void DspDcmConfirmation(PduIdType confirmPduId, NotifResultType result);
void DspUdsReadMemoryByAddress(const PduInfoType *pduRxData, PduInfoType *pduTxData);
void DspUdsWriteMemoryByAddress(const PduInfoType *pduRxData, PduInfoType *pduTxData);
void DspReadDataByPeriodicIdentifier(const PduInfoType *pduRxData,PduInfoType *pduTxData, PduIdType rxPduId, Dcm_ProtocolTransTypeType txType, boolean internalRequest);
void DspDynamicallyDefineDataIdentifier(const PduInfoType *pduRxData,PduInfoType *pduTxData);
void DspIOControlByDataIdentifier(const PduInfoType *pduRxData,PduInfoType *pduTxData);
void DcmDspResetDiagnosticActivity(void);
void DspResetDiagnosticActivityOnSessionChange(Dcm_SesCtrlType newSession);
void DspCommunicationControl(const PduInfoType *pduRxData,PduInfoType *pduTxData, PduIdType rxPduId, PduIdType txConfirmId);
void DspUdsTransferData(const PduInfoType *pduRxData,PduInfoType *pduTxData);
void DspUdsRequestTransferExit(const PduInfoType *pduRxData,PduInfoType *pduTxData);
void DspUdsRequestDownload(const PduInfoType *pduRxData,PduInfoType *pduTxData);
void DspUdsRequestUpload(const PduInfoType *pduRxData, PduInfoType *pduTxData);
void DcmDspProtocolStartNotification(boolean started);

// OBD stack interface
void DspObdRequestCurrentPowertrainDiagnosticData(const PduInfoType *pduRxData,PduInfoType *pduTxData);
void DspObdRequestPowertrainFreezeFrameData(const PduInfoType *pduRxData,PduInfoType *pduTxData);
void DspObdClearEmissionRelatedDiagnosticData(const PduInfoType *pduRxData,PduInfoType *pduTxData);
void DspObdRequestEmissionRelatedDiagnosticTroubleCodes(const PduInfoType *pduRxData,PduInfoType *pduTxData);
void DspObdRequestOnBoardMonitoringTestResultsService06(const PduInfoType *pduRxData,PduInfoType *pduTxData);
void DspObdRequestEmissionRelatedDiagnosticTroubleCodesService07(const PduInfoType *pduRxData,PduInfoType *pduTxData);
void DspObdRequestVehicleInformation(const PduInfoType *pduRxData,PduInfoType *pduTxData);
void DspObdRequestEmissionRelatedDiagnosticTroubleCodesWithPermanentStatus(const PduInfoType *pduRxData,PduInfoType *pduTxData);

boolean DspCheckSessionLevel(Dcm_DspSessionRowType const* const* sessionLevelRefTable);
boolean DspCheckSecurityLevel(Dcm_DspSecurityRowType const* const* securityLevelRefTable);
void DspCancelPendingRequests(void);
#if (DCM_USE_JUMP_TO_BOOT == STD_ON) || defined(DCM_USE_SERVICE_LINKCONTROL)
void DspResponsePendingConfirmed(PduIdType confirmPduId);
#endif
boolean DspDslCheckSessionSupported(uint8 session);
void DspCheckProtocolStartRequests(void);
/*
 * DSD
 */
void DsdInit(void);
void DsdMain(void);
void DsdHandleRequest(void);
void DsdDspProcessingDone(Dcm_NegativeResponseCodeType responseCode);
void DsdDspProcessingDone_ReadDataByPeriodicIdentifier(Dcm_NegativeResponseCodeType responseCode, boolean supressNRC);
void DsdDataConfirmation(PduIdType confirmPduId, NotifResultType result);
void DsdDslDataIndication(const PduInfoType *pduRxData, const Dcm_DsdServiceTableType *protocolSIDTable, Dcm_ProtocolAddrTypeType addrType, PduIdType txPduId, PduInfoType *pduTxData, PduIdType rxContextPduId, Dcm_ProtocolTransTypeType txType, boolean internalRequest, boolean sendRespPendOnTransToBoot, boolean startupResponseRequest);
PduIdType DsdDslGetCurrentTxPduId(void);
#if defined(DCM_USE_SERVICE_READDATABYPERIODICIDENTIFIER) || defined(DCM_USE_SERVICE_COMMUNICATIONCONTROL) || (defined(USE_DEM) && defined(DCM_USE_SERVICE_CONTROLDTCSETTING)) || (defined(DCM_USE_SERVICE_INPUTOUTPUTCONTROLBYIDENTIFIER) && defined(DCM_USE_CONTROL_DIDS))
boolean DsdDspCheckServiceSupportedInActiveSessionAndSecurity(uint8 sid);
#endif
void DsdDspForceResponsePending(void);
boolean DsdDspGetResponseRequired(void);
#if (DCM_USE_JUMP_TO_BOOT == STD_ON) || defined(DCM_USE_SERVICE_LINKCONTROL)
void DsdResponsePendingConfirmed(PduIdType confirmPduId, NotifResultType result);
uint16 DsdDspGetTesterSourceAddress(void);
#endif
void DsdExternalSetNegResponse(const Dcm_MsgContextType* pMsgContext, Dcm_NegativeResponseCodeType ErrorCode);
void DsdExternalProcessingDone(const Dcm_MsgContextType* pMsgContext);
boolean DsdLookupSubService(uint8 SID, const Dcm_DsdServiceType *sidConfPtr, uint8 subService, const Dcm_DsdSubServiceType **subServicePtr, Dcm_NegativeResponseCodeType *respCode);
#if (DCM_USE_SPLIT_TASK_CONCEPT == STD_ON)
void DsdSetDspProcessingActive(boolean state);
#endif
void DsdCancelPendingExternalServices(void);
#ifdef DCM_USE_SERVICE_COMMUNICATIONCONTROL
Std_ReturnType DsdDspGetCurrentProtocolRx(PduIdType rxPduId, const Dcm_DslProtocolRxType **protocolRx);
#endif
void DsdClearSuppressPosRspMsg(void);

/*
 * DSL
 */
void DslInit(void);
void DslMain(void);
void DslHandleResponseTransmission(void);
void DslDsdProcessingDone(PduIdType rxPduIdRef, DsdProcessingDoneResultType responseResult);

void DslTpRxIndicationFromPduR(PduIdType dcmRxPduId, NotifResultType result, boolean internalRequest, boolean startupResponseRequest);
Std_ReturnType DslGetActiveProtocol(Dcm_ProtocolType *protocolId);
void DslInternal_SetSecurityLevel(Dcm_SecLevelType secLevel);
Std_ReturnType DslGetSecurityLevel(Dcm_SecLevelType *secLevel);
void DslSetSesCtrlType(Dcm_SesCtrlType sesCtrl);
Std_ReturnType DslGetSesCtrlType(Dcm_SesCtrlType *sesCtrlType);
void DslTpTxConfirmation(PduIdType dcmTxPduId, NotifResultType result);
Std_ReturnType DslInternal_ResponseOnOneDataByPeriodicId(uint8 PericodID, PduIdType rxPduId);
Std_ReturnType DslInternal_ResponseOnOneEvent(PduIdType rxPduId, uint8* request, uint8 requestLength);
Std_ReturnType DslDspSilentlyStartProtocol(uint8 session, uint8 protocolId, uint16 testerSourceAddress, boolean requestFullComm);
Std_ReturnType DslDspResponseOnStartupRequest(uint8 sid, uint8 subFnc, uint8 protocolId, uint16 testerSourceAddress);
void DslComModeEntered(uint8 network, DcmComModeType comMode);
void DslResetSessionTimeoutTimer(void);
boolean DslPduRPduUsedForType2Tx(PduIdType pdurTxPduId, PduIdType *dcmTxPduId);
#ifdef DCM_USE_SERVICE_COMMUNICATIONCONTROL
Std_ReturnType DslDsdGetProtocolRx(PduIdType rxPduId, const Dcm_DslProtocolRxType **protocolRx);
#endif

BufReq_ReturnType DslCopyDataToRxBuffer(PduIdType dcmRxPduId, const PduInfoType *pduInfoPtr, PduLengthType *rxBufferSizePtr);
BufReq_ReturnType DslStartOfReception(PduIdType dcmRxPduId, PduLengthType tpSduLength, PduLengthType *rxBufferSizePtr, boolean internalRequest);
BufReq_ReturnType DslCopyTxData(PduIdType dcmTxPduId, PduInfoType *pduInfoPtr, RetryInfoType *periodData, PduLengthType *txDataCntPtr);

#if (DCM_MANUFACTURER_NOTIFICATION == STD_ON) || (DCM_USE_JUMP_TO_BOOT == STD_ON) || defined(DCM_USE_SERVICE_LINKCONTROL) || (defined(DCM_USE_SERVICE_RESPONSEONEVENT) && defined(USE_NVM))
Std_ReturnType Arc_DslGetRxConnectionParams(PduIdType rxPduId, uint16* sourceAddress, Dcm_ProtocolAddrTypeType* reqType, Dcm_ProtocolType *protocolId);
#endif

void DslDsdSendResponsePending(PduIdType rxPduId);
#if (DCM_USE_SPLIT_TASK_CONCEPT == STD_ON)
void DslSetDspProcessingActive(PduIdType dcmRxPduId, boolean state);
#endif
boolean lookupNonDynamicDid(uint16 didNr, const Dcm_DspDidType **didPtr);
boolean lookupDynamicDid(uint16 didNr, const Dcm_DspDidType **didPtr);
Dcm_NegativeResponseCodeType readDidData(const Dcm_DspDidType *didPtr, PduInfoType *pduTxData, uint16 *txPos,
        ReadDidPendingStateType *pendingState, uint16 *pendingDid, uint16 *pendingSignalIndex, uint16 *pendingDataLen, uint16 *didIndex, uint16 didStartIndex, uint16 *didDataStartPos);
Dcm_NegativeResponseCodeType getDidData(const Dcm_DspDidType *didPtr, const PduInfoType *pduTxData, uint16 *txPos,
                                        ReadDidPendingStateType *pendingState, uint16 *pendingDidLen, uint16 *pendingSignalIndex,  uint16 *didDataStartPos, boolean includeDID);

void getDidLength(const Dcm_DspDidType *didPtr, uint16 *length, uint16* nofDatas);


#ifdef DCM_USE_SERVICE_RESPONSEONEVENT

/* DCM ROE */
void DCM_ROE_Init(void);
void DCM_ROE_PollDataIdentifiers(void);
boolean DCM_ROE_IsActive(void);

Dcm_NegativeResponseCodeType DCM_ROE_Start(uint8 storageState, uint8 eventWindowTime, PduIdType rxPduId, PduInfoType *pduTxData);
Dcm_NegativeResponseCodeType DCM_ROE_Stop(uint8 storageState, uint8 eventWindowTime, PduInfoType *pduTxData);
Dcm_NegativeResponseCodeType DCM_ROE_ClearEventList(uint8 storageState, uint8 eventWindowTime, PduInfoType *pduTxData);
Dcm_NegativeResponseCodeType DCM_ROE_GetEventList(uint8 storageState, PduInfoType *pduTxData);
Dcm_NegativeResponseCodeType DCM_ROE_AddDataIdentifierEvent(uint8 eventWindowTime,
                                                            uint8 storageState,
                                                            uint16 eventTypeRecord,
                                                            const uint8* serviceToRespondTo,
                                                            uint8 serviceToRespondToLength,
                                                            PduInfoType *pduTxData);
void Dcm_ROE_MainFunction(void);
void DcmRoeProtocolStartNotification(boolean started);
void DcmRoeCheckProtocolStartRequest(void);
#endif

void DcmResetDiagnosticActivity(void);
void DcmResetDiagnosticActivityOnSessionChange(Dcm_SesCtrlType newSession);
void DcmCancelPendingRequests(void);

Std_ReturnType DcmRequestStartProtocol(DcmProtocolRequesterType requester, uint8 session, uint8 testerSrcAddr, uint8 protocolId, boolean requestFullComm);
void DcmSetProtocolStartRequestsAllowed(boolean allowed);
void DcmExecuteStartProtocolRequest(void);

Std_ReturnType ReadSRSignal(const Dcm_DspDataType *dataPtr, uint16 bitPosition, uint8 *dataBuffer);
Std_ReturnType WriteSRSignal(const Dcm_DspDataType *dataPtr, uint16 bitPosition, const uint8 *dataBuffer);
uint16 GetNofAffectedBytes(DcmDspDataEndianessType endianess, uint16 startBitPos, uint16 bitLength);
#endif /* DCM_INTERNAL_H_ */
