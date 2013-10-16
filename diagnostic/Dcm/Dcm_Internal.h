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

/*
 * NB! This file is for DCM internal use only and may only be included from DCM C-files!
 */



#ifndef DCM_INTERNAL_H_
#define DCM_INTERNAL_H_


#if  ( DCM_DEV_ERROR_DETECT == STD_ON )
#if defined(USE_DET)
#include "Det.h"
#endif
#define VALIDATE(_exp,_api,_err ) \
        if( !(_exp) ) { \
          Det_ReportError(MODULE_ID_DCM, 0, _api, _err); \
          return E_NOT_OK; \
        }

#define VALIDATE_RV(_exp,_api,_err,_rv ) \
        if( !(_exp) ) { \
          Det_ReportError(MODULE_ID_DCM, 0, _api, _err); \
          return _rv; \
        }

#define VALIDATE_NO_RV(_exp,_api,_err ) \
  if( !(_exp) ) { \
          Det_ReportError(MODULE_ID_DCM, 0, _api, _err); \
          return; \
        }
#define DET_REPORTERROR(_x,_y,_z,_q) Det_ReportError(_x, _y, _z, _q)

#else
#define VALIDATE(_exp,_api,_err )
#define VALIDATE_RV(_exp,_api,_err,_rv )
#define VALIDATE_NO_RV(_exp,_api,_err )
#define DET_REPORTERROR(_x,_y,_z,_q)
#endif


// SID table
#define SID_DIAGNOSTIC_SESSION_CONTROL			0x10
#define SID_ECU_RESET							0x11
#define SID_CLEAR_DIAGNOSTIC_INFORMATION		0x14
#define SID_READ_DTC_INFORMATION				0x19
#define SID_READ_DATA_BY_IDENTIFIER				0x22
#define SID_READ_MEMORY_BY_ADDRESS				0x23
#define SID_READ_SCALING_DATA_BY_IDENTIFIER		0x24
#define SID_SECURITY_ACCESS						0x27
#define SID_COMMUNICATION_CONTROL				0x28
#define SID_READ_DATA_BY_PERIODIC_IDENTIFIER	0x2A
#define SID_DYNAMICALLY_DEFINE_DATA_IDENTIFIER	0x2C
#define SID_WRITE_DATA_BY_IDENTIFIER			0x2E
#define SID_INPUT_OUTPUT_CONTROL_BY_IDENTIFIER	0x2F
#define SID_ROUTINE_CONTROL						0x31
#define SID_WRITE_MEMORY_BY_ADDRESS				0x3D
#define SID_TESTER_PRESENT						0x3E
#define SID_NEGATIVE_RESPONSE					0x7F
#define SID_CONTROL_DTC_SETTING					0x85

//OBD SID TABLE
#define SID_REQUEST_CURRENT_POWERTRAIN_DIAGNOSTIC_DATA		0x01
#define SID_REQUEST_POWERTRAIN_FREEZE_FRAME_DATA			0x02
#define SID_CLEAR_EMISSION_RELATED_DIAGNOSTIC_INFORMATION	0x04
#define SID_REQUEST_EMISSION_RELATED_DIAGNOSTIC_TROUBLE_CODES		0x03
#define SID_REQUEST_EMISSION_RELATED_DIAGNOSTIC_TROUBLE_CODES_DETECTED_DURING_CURRENT_OR_LAST_COMPLETED_DRIVING_CYCLE	0x07
#define SID_REQUEST_VEHICLE_INFORMATION		0x09

// Misc definitions
#define SUPPRESS_POS_RESP_BIT		(uint8)0x80
#define SID_RESPONSE_BIT			(uint8)0x40
#define VALUE_IS_NOT_USED			(uint8)0x00

typedef enum {
	DSD_TX_RESPONSE_READY,
	DSD_TX_RESPONSE_SUPPRESSED
} DsdProcessingDoneResultType;

/*
 * DSP
 */
void DspInit(void);
void DspMain(void);
void DspUdsDiagnosticSessionControl(const PduInfoType *pduRxData, PduIdType txPduId, PduInfoType *pduTxData);
void DspUdsEcuReset(const PduInfoType *pduRxData, PduIdType txPduId, PduInfoType *pduTxData);
void DspUdsClearDiagnosticInformation(const PduInfoType *pduRxData, PduInfoType *pduTxData);
void DspUdsSecurityAccess(const PduInfoType *pduRxData, PduInfoType *pduTxData);
void DspUdsTesterPresent(const PduInfoType *pduRxData, PduInfoType *pduTxData);
void DspUdsReadDtcInformation(const PduInfoType *pduRxData, PduInfoType *pduTxData);
void DspUdsReadDataByIdentifier(const PduInfoType *pduRxData, PduInfoType *pduTxData);
void DspUdsReadScalingDataByIdentifier(const PduInfoType *pduRxData, PduInfoType *pduTxData);
void DspUdsWriteDataByIdentifier(const PduInfoType *pduRxData, PduInfoType *pduTxData);
void DspUdsControlDtcSetting(const PduInfoType *pduRxData, PduInfoType *pduTxData);
void DspUdsRoutineControl(const PduInfoType *pduRxData, PduInfoType *pduTxData);
void DspDcmConfirmation(PduIdType confirmPduId);
void DspUdsReadMemoryByAddress(const PduInfoType *pduRxData, PduInfoType *pduTxData);
void DspUdsWriteMemoryByAddress(const PduInfoType *pduRxData, PduInfoType *pduTxData);
void DspReadDataByPeriodicIdentifier(const PduInfoType *pduRxData,PduInfoType *pduTxData);
void DspDynamicallyDefineDataIdentifier(const PduInfoType *pduRxData,PduInfoType *pduTxData);
void DspIOControlByDataIdentifier(const PduInfoType *pduRxData,PduInfoType *pduTxData);
void DspCommunicationControl(const PduInfoType *pduRxData,PduInfoType *pduTxData);

// OBD stack interface
void DspObdRequestCurrentPowertrainDiagnosticData(const PduInfoType *pduRxData,PduInfoType *pduTxData);
void DspObdRequsetPowertrainFreezeFrameData(const PduInfoType *pduRxData,PduInfoType *pduTxData);
void DspObdClearEmissionRelatedDiagnosticData(const PduInfoType *pduRxData,PduInfoType *pduTxData);
void DspObdRequestEmissionRelatedDiagnosticTroubleCodes(const PduInfoType *pduRxData,PduInfoType *pduTxData);
void DspObdRequestEmissionRelatedDiagnosticTroubleCodesService07(const PduInfoType *pduRxData,PduInfoType *pduTxData);
void DspObdRequestvehicleinformation(const PduInfoType *pduRxData,PduInfoType *pduTxData);

boolean DspCheckSessionLevel(Dcm_DspSessionRowType const* const* sessionLevelRefTable);
boolean DspCheckSecurityLevel(Dcm_DspSecurityRowType const* const* securityLevelRefTable);
void DspCancelPendingRequests(void);

/*
 * DSD
 */
void DsdInit(void);
void DsdMain(void);
void DsdHandleRequest(void);
void DsdDspProcessingDone(Dcm_NegativeResponseCodeType responseCode);
void DsdDataConfirmation(PduIdType confirmPduId, NotifResultType result);
void DsdDslDataIndication(const PduInfoType *pduRxData, const Dcm_DsdServiceTableType *protocolSIDTable, Dcm_ProtocolAddrTypeType addrType, PduIdType txPduId, PduInfoType *pduTxData, PduIdType rxContextPduId);


/*
 * DSL
 */
void DslInit(void);
void DslMain(void);
void DslHandleResponseTransmission(void);
void DslDsdProcessingDone(PduIdType rxPduIdRef, DsdProcessingDoneResultType responseResult);
void DslGetCurrentServiceTable(const Dcm_DsdServiceTableType **currentServiceTable);

BufReq_ReturnType DslProvideRxBufferToPdur(PduIdType dcmRxPduId, PduLengthType tpSduLength, const PduInfoType **pduInfoPtr);
void DslRxIndicationFromPduR(PduIdType dcmRxPduId, NotifResultType result);
Std_ReturnType DslGetActiveProtocol(Dcm_ProtocolType *protocolId);
void DslSetSecurityLevel(Dcm_SecLevelType secLevel);
Std_ReturnType DslGetSecurityLevel(Dcm_SecLevelType *secLevel);
void DslSetSesCtrlType(Dcm_SesCtrlType sesCtrl);
Std_ReturnType DslGetSesCtrlType(Dcm_SesCtrlType *sesCtrlType);
BufReq_ReturnType DslProvideTxBuffer(PduIdType dcmTxPduId, const PduInfoType **pduInfoPtr, PduLengthType length);
void DslTxConfirmation(PduIdType dcmTxPduId, NotifResultType result);
Std_ReturnType DslInternal_ResponseOnOneDataByPeriodicId(uint8 PericodID);
void DslResetSessionTimeoutTimer(void);
PduIdType DsdDslGetCurrentTxPduId(void);


#endif /* DCM_INTERNAL_H_ */
