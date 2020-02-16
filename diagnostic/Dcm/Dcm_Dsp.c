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

//lint -esym(754, SID)	//Structure member SID not used in udsReadDtcInfoSub_0x01_0x07_0x11_0x12() and udsReadDtcInfoSub_0x02_0x0A_0x0F_0x13_0x15()


/*
 *  General requirements
 */
/** @req DCM273 */ /** @req DCM272 */
/** @req DCM039 */ /** @req DCM038 */ /** @req DCM269 */
/** @req DCM271 */ /** @req DCM274 */ /** @req DCM275 */
/** @req DCM007 */
#include <string.h>
#include "Dcm.h"
#include "Dcm_Internal.h"
#if defined(DCM_USE_SERVICE_CLEARDIAGNOSTICINFORMATION) || defined(DCM_USE_SERVICE_READDTCINFORMATION) || defined(DCM_USE_SERVICE_CONTROLDTCSETTING)
#if defined(USE_DEM)
#include "Dem.h"
#else
#warning Dcm: UDS services ClearDiagnosticInformation, ReadDTCInformation and/or ControlDTCSetting will not work without Dem.
#endif
#endif
#include "MemMap.h"
#if defined(USE_MCU)
#include "Mcu.h"
#endif
/*
 * Macros
 */
#define ZERO_SUB_FUNCTION				0x00
#define DCM_FORMAT_LOW_MASK			0x0F
#define DCM_FORMAT_HIGH_MASK			0xF0
#define DCM_MEMORY_ADDRESS_MASK		0x00FFFFFF
#define DCM_DID_HIGH_MASK 				0xFF00			
#define DCM_DID_LOW_MASK				0xFF
#define DCM_PERODICDID_HIGH_MASK		0xF200
#define SID_AND_DDDID_LEN   0x4
#define SDI_AND_MS_LEN   0x4

#define SID_AND_SDI_LEN   0x6
#define SID_AND_PISDR_LEN   0x7

/* == Parser macros == */
/* General */
#define SID_INDEX 0
#define SID_LEN 1
#define SF_INDEX 1
#define SF_LEN 1
#define PID_BUFFER_SIZE 255 // OBD
#define DTC_LEN 3
#define FF_REC_NUM_LEN 1
/* Read/WriteMemeoryByAddress */
#define ALFID_INDEX 1
#define ALFID_LEN 1
#define ADDR_START_INDEX 2
/* DynamicallyDefineDataByIdentifier */
#define DDDDI_INDEX 2
#define DDDDI_LEN 2
#define DYNDEF_ALFID_INDEX 4
#define DYNDEF_ADDRESS_START_INDEX 5
/* InputOutputControlByIdentifier */
#define IOI_INDEX 1
#define IOI_LEN 2
#define IOCP_INDEX 3
#define IOCP_LEN 1
#define COR_INDEX 4

/*OBD RequestCurrentPowertrainDiagnosticData*/
#define PIDZERO								0
#define DATAZERO							0
#define INFOTYPE_ZERO						0
#define PID_LEN								1
#define RECORD_NUM 							0
#define HALF_BYTE 							4
#define OFFSET_ONE_BYTE						8
#define OFFSET_TWO_BYTES 					16
#define OFFSET_THREE_BYTES					24
#define SUPPRTED_PIDS_DATA_LEN				4
#define LEAST_BIT_MASK  					((uint8)0x01u)
#define OBD_DATA_LSB_MASK 					((uint32)0x000000FFu)
#define OBD_REQ_MESSAGE_LEN_ONE_MIN 		2
#define OBD_REQ_MESSAGE_LEN_MAX  			7
#define AVAIL_TO_SUPPORTED_PID_OFFSET_MIN  	0x01
#define AVAIL_TO_SUPPORTED_PID_OFFSET_MAX  	0x20
#define AVAIL_TO_SUPPORTED_INFOTYPE_OFFSET_MIN  	0x01
#define AVAIL_TO_SUPPORTED_INFOTYPE_OFFSET_MAX  	0x20
#define MAX_REQUEST_PID_NUM 				6
#define LENGTH_OF_DTC  						2

/* CommunicationControl */
#define CC_CTP_INDEX 2
#define COMM_CTRL_ISO_RES_SF_LOW    0x04
#define COMM_CTRL_ISO_RES_SF_HIGH   0x3F
#define COMM_CTRL_ISO_RES_SF        0x7F
#define IS_IN_ISO_RESERVED_RANGE(_x)    ((_x >= COMM_CTRL_ISO_RES_SF_LOW) && (_x <= COMM_CTRL_ISO_RES_SF_HIGH))
#define IS_ISO_RESERVED(_x) (IS_IN_ISO_RESERVED_RANGE(_x) || (COMM_CTRL_ISO_RES_SF == _x))

/*OBD RequestCurrentPowertrainDiagnosticData*/
#define FF_NUM_LEN							1
#define OBD_DTC_LEN							2
#define OBD_SERVICE_TWO 					((uint8)0x02u)
#define MAX_PID_FFNUM_NUM					3
#define OBD_REQ_MESSAGE_LEN_TWO_MIN			3
#define DATA_ELEMENT_INDEX_OF_PID_NOT_SUPPORTED  0

#if defined(DEM_MAX_NR_OF_PIDS_IN_FREEZEFRAME_DATA)
#define DCM_MAX_PID_NUM_IN_FF				DEM_MAX_NR_OF_PIDS_IN_FREEZEFRAME_DATA
#else
#define DCM_MAX_PID_NUM_IN_FF				0
#endif

/*OBD RequestEmissionRelatedDiagnosticTroubleCodes service03 07*/
#define EMISSION_DTCS_HIGH_BYTE(dtc)		(((uint32)(dtc) >> 8) & 0xFFu)
#define EMISSION_DTCS_LOW_BYTE(dtc)			((uint32)(dtc) & 0xFFu)
#define OBD_RESPONSE_DTC_MAX_NUMS			126

/*OBD Requestvehicleinformation service09*/
#define OBD_TX_MAXLEN						0xFF
#define MAX_REQUEST_VEHINFO_NUM				6
#define OBD_SERVICE_FOUR 					0x04
#define OBD_VIN_LENGTH						17

#define IS_AVAILABILITY_PID(_x) ( (0 == (_x % 0x20)) && (_x <= 0xE0))
#define IS_AVAILABILITY_INFO_TYPE(_x) IS_AVAILABILITY_PID(_x)

#define BYTES_TO_DTC(hb, mb, lb)	(((uint32)(hb) << 16) | ((uint32)(mb) << 8) | (uint32)(lb))
#define DTC_HIGH_BYTE(dtc)			(((uint32)(dtc) >> 16) & 0xFFu)
#define DTC_MID_BYTE(dtc)			(((uint32)(dtc) >> 8) & 0xFFu)
#define DTC_LOW_BYTE(dtc)			((uint32)(dtc) & 0xFFu)

#define TIMER_DECREMENT(timer) \
        if (timer >= DCM_MAIN_FUNCTION_PERIOD_TIME_MS) { \
            timer = timer - DCM_MAIN_FUNCTION_PERIOD_TIME_MS; \
        } \

typedef enum {
	DCM_READ_MEMORY = 0,
	DCM_WRITE_MEMORY,
} DspMemoryServiceType;

typedef enum {
	DCM_DSP_RESET_NO_RESET,
	DCM_DSP_RESET_PENDING,
	DCM_DSP_RESET_WAIT_TX_CONF,
} DcmDspResetStateType;

typedef struct {
	DcmDspResetStateType resetPending;
	PduIdType resetPduId;
	PduInfoType *pduTxData;
	Dcm_EcuResetType resetType;
} DspUdsEcuResetDataType;

typedef struct {
	boolean sessionPending;
	PduIdType sessionPduId;
	Dcm_SesCtrlType session;
} DspUdsSessionControlDataType;

typedef enum {
	DCM_READ_DID_IDLE,
	DCM_READ_DID_PENDING_COND_CHECK,
	DCM_READ_DID_PENDING_READ_DATA
} ReadDidPendingStateType;

typedef enum {
    DCM_WRITE_DID_IDLE,
    DCM_WRITE_DID_PENDING,
} WriteDidPendingStateType;

typedef struct {
	ReadDidPendingStateType state;
	const PduInfoType* pduRxData;
	PduInfoType* pduTxData;
	uint16 txWritePos;
	uint16 nofReadDids;
	uint16 reqDidIndex;
	uint16 pendingDid;
	uint16 pendingDidLength;
} DspUdsReadDidPendingType;

typedef struct {
    WriteDidPendingStateType state;
    const PduInfoType* pduRxData;
    PduInfoType* pduTxData;
} DspUdsWriteDidPendingType;
static DspUdsEcuResetDataType dspUdsEcuResetData;
static DspUdsSessionControlDataType dspUdsSessionControlData;
static DspUdsReadDidPendingType dspUdsReadDidPending;
static DspUdsWriteDidPendingType dspUdsWriteDidPending;

typedef enum {
    DELAY_TIMER_DEACTIVE,
    DELAY_TIMER_ON_BOOT_ACTIVE,
    DELAY_TIMER_ON_EXCEEDING_LIMIT_ACTIVE
}DelayTimerActivation;

typedef struct {
    uint8_t                         secAcssAttempts; //Counter for number of false attempts
    uint32_t                        timerSecAcssAttempt; //Timer after exceededNumberOfAttempts
    DelayTimerActivation            startDelayTimer; //Flag to indicate Delay timer is active
}DspUdsSecurityAccessChkParam;

typedef struct {
	boolean 						reqInProgress;
	Dcm_SecLevelType				reqSecLevel;
#if (DCM_SECURITY_EOL_INDEX != 0)
	DspUdsSecurityAccessChkParam    secFalseAttemptChk[DCM_SECURITY_EOL_INDEX];
    uint8_t                         currSecLevIdx; //Current index for secFalseAttemptChk
#endif
	const Dcm_DspSecurityRowType	*reqSecLevelRef;
} DspUdsSecurityAccessDataType;

static DspUdsSecurityAccessDataType dspUdsSecurityAccesData;

typedef enum{
	DCM_MEMORY_UNUSED,
	DCM_MEMORY_READ,
	DCM_MEMORY_WRITE,
	DCM_MEMORY_FAILED	
}Dcm_DspMemoryStateType;
static Dcm_DspMemoryStateType dspMemoryState;

typedef enum{
	DCM_DDD_SOURCE_DEFAULT,
	DCM_DDD_SOURCE_DID,
	DCM_DDD_SOURCE_ADDRESS
}Dcm_DspDDDSourceKindType;

typedef struct{
	uint32 PDidTxCounter;
	uint32 PDidTxCounterLimit;
	uint8  PeriodicDid;
}Dcm_pDidType;/* a type to save  the periodic DID and cycle */

typedef struct{
	Dcm_pDidType dspPDid[DCM_LIMITNUMBER_PERIODDATA];	/*a buffer to save the periodic DID and cycle   */
	uint8 PDidNr;										/* note the number of periodic DID is used */
}Dsp_pDidRefType;

static Dsp_pDidRefType dspPDidRef;

typedef struct{
	uint8   formatOrPosition;						/*note the formate of address and size*/
	uint8	memoryIdentifier;
	uint32 SourceAddressOrDid;								/*note the memory address */
	uint16 Size;										/*note the memory size */
	Dcm_DspDDDSourceKindType DDDTpyeID;
}Dcm_DspDDDSourceType;

typedef struct{
	uint16 DynamicallyDid;
	Dcm_DspDDDSourceType DDDSource[DCM_MAX_DDDSOURCE_NUMBER];
}Dcm_DspDDDType;

#ifdef DCM_USE_SERVICE_DYNAMICALLYDEFINEDATAIDENTIFIER
static Dcm_DspDDDType dspDDD[DCM_MAX_DDD_NUMBER];
#endif

#ifdef DCM_USE_SERVICE_INPUTOUTPUTCONTROLBYIDENTIFIER
#if 0 < DCM_NOF_IOCONTROL_DIDS
#define DCM_USE_CONTROL_DIDS
typedef struct {
    uint16 did;
    boolean controlActive;
}Dcm_DspIOControlStateType;
static Dcm_DspIOControlStateType IOControlStateList[DCM_NOF_IOCONTROL_DIDS];
#endif
#endif

/*
 * * static Function
 */

#ifndef DCM_USE_SERVICE_DYNAMICALLYDEFINEDATAIDENTIFIER
#define LookupDDD(_x,  _y ) FALSE
#define readDDDData(_x, _y, _z) DCM_E_GENERALREJECT
#else
static boolean LookupDDD(uint16 didNr, const Dcm_DspDDDType **DDid);
#endif
static Dcm_NegativeResponseCodeType checkAddressRange(DspMemoryServiceType serviceType, uint8 memoryIdentifier, uint32 memoryAddress, uint32 length);
static const Dcm_DspMemoryRangeInfo* findRange(const Dcm_DspMemoryRangeInfo *memoryRangePtr, uint32 memoryAddress, uint32 length);
static Dcm_NegativeResponseCodeType writeMemoryData(Dcm_OpStatusType* OpStatus, uint8 memoryIdentifier, uint32 MemoryAddress, uint32 MemorySize, uint8 *SourceData);
static void DspCancelPendingDid(uint16 didNr, ReadDidPendingStateType pendingState, PduInfoType *pduTxData );

#ifdef DCM_USE_CONTROL_DIDS
static void DspStopInputOutputControl(boolean checkSessionAndSecLevel);
#endif
/*
*   end  
*/

//
// This function reset diagnostic activity on session transition.
//This function should be called after the session and security level have been changed
//
//
void DspResetDiagnosticActivityOnSessionChange(Dcm_SesCtrlType newSession)
{
    (void)newSession;
#ifdef DCM_USE_CONTROL_DIDS
    /* DCM628 says that active control should be stopped on transition
     * to default session only. But stop it if active control is not
     * supported in the new session (which should be the current session
     * as it is assumed that session is changed before calling this function) or
     * in the new security level. */
    DspStopInputOutputControl(TRUE);
#endif
}

void DcmDspResetDiagnosticActivity(void)
{
#ifdef DCM_USE_CONTROL_DIDS
    DspStopInputOutputControl(FALSE);
#endif
}

void DspInit(boolean firstCall)
{
	dspUdsSecurityAccesData.reqInProgress = FALSE;
	dspUdsEcuResetData.resetPending = DCM_DSP_RESET_NO_RESET;
	dspUdsSessionControlData.sessionPending = FALSE;

	dspMemoryState=DCM_MEMORY_UNUSED;
	/* clear periodic send buffer */
	memset(&dspPDidRef,0,sizeof(dspPDidRef));

#ifdef DCM_USE_SERVICE_DYNAMICALLYDEFINEDATAIDENTIFIER
	/* clear dynamically Did buffer */
	memset(&dspDDD[0],0,sizeof(dspDDD));
#endif

#if (DCM_SECURITY_EOL_INDEX != 0)
	uint8_t temp = 0;
	if (firstCall) {
	    //Reset the security access attempts
	    do {
	        dspUdsSecurityAccesData.secFalseAttemptChk[temp].secAcssAttempts = 0;
	        if (Dcm_ConfigPtr->Dsp->DspSecurity->DspSecurityRow[temp].DspSecurityDelayTimeOnBoot >= DCM_MAIN_FUNCTION_PERIOD_TIME_MS) {
	            dspUdsSecurityAccesData.secFalseAttemptChk[temp].timerSecAcssAttempt = Dcm_ConfigPtr->Dsp->DspSecurity->DspSecurityRow[temp].DspSecurityDelayTimeOnBoot;
	            dspUdsSecurityAccesData.secFalseAttemptChk[temp].startDelayTimer = DELAY_TIMER_ON_BOOT_ACTIVE;
	        }
	        else {
	            dspUdsSecurityAccesData.secFalseAttemptChk[temp].startDelayTimer = DELAY_TIMER_DEACTIVE;
	        }
	        temp++;
	    } while (temp < DCM_SECURITY_EOL_INDEX);
	    dspUdsSecurityAccesData.currSecLevIdx = 0;
	}
#else
	(void)firstCall;
#endif
}

void DspResetMainFunction(void)
{
	if( DCM_DSP_RESET_PENDING == dspUdsEcuResetData.resetPending )
	{
		switch( DcmE_EcuReset(dspUdsEcuResetData.resetType) )
		{
		case E_OK:
			dspUdsEcuResetData.resetPending = DCM_DSP_RESET_WAIT_TX_CONF;
			// Create positive response
			dspUdsEcuResetData.pduTxData->SduDataPtr[1] = dspUdsEcuResetData.resetType;
			dspUdsEcuResetData.pduTxData->SduLength = 2;
			DsdDspProcessingDone(DCM_E_POSITIVERESPONSE);
			break;
		case E_PENDING:
			dspUdsEcuResetData.resetPending = DCM_DSP_RESET_PENDING;
			break;
		case E_NOT_OK:
		default:
			dspUdsEcuResetData.resetPending = DCM_DSP_RESET_NO_RESET;
			DsdDspProcessingDone(DCM_E_CONDITIONSNOTCORRECT);
			break;
		}
	}
}

void DspMemoryMainFunction(void)
{
	Dcm_ReturnWriteMemoryType WriteRet;
	Dcm_ReturnReadMemoryType ReadRet;
	switch(dspMemoryState)
	{
		case DCM_MEMORY_UNUSED:
			break;
		case DCM_MEMORY_READ:
			ReadRet = Dcm_ReadMemory(DCM_PENDING,0,0,0,0);
			if(ReadRet == DCM_READ_OK)/*asynchronous writing is ok*/
			{
				DsdDspProcessingDone(DCM_E_POSITIVERESPONSE);
				dspMemoryState = DCM_MEMORY_UNUSED;
			}
			if(ReadRet == DCM_READ_FAILED)
			{
				DsdDspProcessingDone(DCM_E_GENERALPROGRAMMINGFAILURE);
				dspMemoryState = DCM_MEMORY_UNUSED;
			}
			break;
		case DCM_MEMORY_WRITE:
			WriteRet = Dcm_WriteMemory(DCM_PENDING,0,0,0,0);
			if(WriteRet == DCM_WRITE_OK)/*asynchronous writing is ok*/
			{
				DsdDspProcessingDone(DCM_E_POSITIVERESPONSE);
				dspMemoryState = DCM_MEMORY_UNUSED;
			}
			if(WriteRet == DCM_WRITE_FAILED)
			{
				DsdDspProcessingDone(DCM_E_GENERALPROGRAMMINGFAILURE);
				dspMemoryState = DCM_MEMORY_UNUSED;
			}
			break;

			default:
			break;
			
	}
}
void DspPeriodicDIDMainFunction()
{
	uint8 i;
	boolean sentResponseThisLoop = FALSE;

	for(i = 0;i < dspPDidRef.PDidNr; i++)
	{
		if(dspPDidRef.dspPDid[i].PDidTxCounterLimit > dspPDidRef.dspPDid[i].PDidTxCounter)
		{
			dspPDidRef.dspPDid[i].PDidTxCounter++;
		}
		else
		{
			if( sentResponseThisLoop  == FALSE ) {
				if (E_OK == DslInternal_ResponseOnOneDataByPeriodicId(dspPDidRef.dspPDid[i].PeriodicDid)){
					dspPDidRef.dspPDid[i].PDidTxCounter = 0;
					/*AutoSar  DCM  8.10.5 */
					sentResponseThisLoop = TRUE;
				}
			}
			else {
				/* Don't do anything - PDid will be sent next loop */
			}
		}	
	}
}

void DspReadDidMainFunction(void) {
	if( DCM_READ_DID_IDLE != dspUdsReadDidPending.state ) {
		DspUdsReadDataByIdentifier(dspUdsReadDidPending.pduRxData, dspUdsReadDidPending.pduTxData);
	}
	if( DCM_WRITE_DID_PENDING == dspUdsWriteDidPending.state ) {
		DspUdsWriteDataByIdentifier(dspUdsWriteDidPending.pduRxData, dspUdsWriteDidPending.pduTxData);
	}
}

void DspMain(void)
{
	DspResetMainFunction();
	DspMemoryMainFunction();
	DspPeriodicDIDMainFunction();
	DspReadDidMainFunction();

#if (DCM_SECURITY_EOL_INDEX != 0)

	for (uint8_t i=0; i < DCM_SECURITY_EOL_INDEX; i++)
	{
	    //Check if a wait is required before accepting a request
	   switch (dspUdsSecurityAccesData.secFalseAttemptChk[i].startDelayTimer) {

	       case DELAY_TIMER_ON_BOOT_ACTIVE:
	       case DELAY_TIMER_ON_EXCEEDING_LIMIT_ACTIVE:
	           TIMER_DECREMENT(dspUdsSecurityAccesData.secFalseAttemptChk[i].timerSecAcssAttempt);
	           if (dspUdsSecurityAccesData.secFalseAttemptChk[i].timerSecAcssAttempt < DCM_MAIN_FUNCTION_PERIOD_TIME_MS) {
	               dspUdsSecurityAccesData.secFalseAttemptChk[i].startDelayTimer = DELAY_TIMER_DEACTIVE;
	           }
	           break;

	       case DELAY_TIMER_DEACTIVE:
	       default:
	           break;
	   }
	}
#endif
}

void DspCancelPendingRequests(void)
{
    if( DCM_READ_DID_IDLE != dspUdsReadDidPending.state ) {
        /* DidRead was in pending state, cancel it */
        DspCancelPendingDid(dspUdsReadDidPending.pendingDid, dspUdsReadDidPending.state, dspUdsReadDidPending.pduTxData);
    }
    dspMemoryState = DCM_MEMORY_UNUSED;
    dspUdsEcuResetData.resetPending = DCM_DSP_RESET_NO_RESET;
    dspUdsReadDidPending.state = DCM_READ_DID_IDLE;
    dspUdsWriteDidPending.state = DCM_WRITE_DID_IDLE;
}

boolean DspCheckSessionLevel(Dcm_DspSessionRowType const* const* sessionLevelRefTable)
{
	Std_ReturnType returnStatus;
	boolean levelFound = FALSE;
	Dcm_SesCtrlType currentSession;

	returnStatus = DslGetSesCtrlType(&currentSession);
	if (returnStatus == E_OK) {
	    if( (*sessionLevelRefTable)->Arc_EOL ) {
	        /* No session reference configured, no check should be done. */
	        levelFound = TRUE;
	    } else {
            while ( ((*sessionLevelRefTable)->DspSessionLevel != currentSession) && (!(*sessionLevelRefTable)->Arc_EOL) ) {
                sessionLevelRefTable++;
            }

            if (!(*sessionLevelRefTable)->Arc_EOL) {
                levelFound = TRUE;
            }
	    }
	}

	return levelFound;
}


boolean DspCheckSecurityLevel(Dcm_DspSecurityRowType const* const* securityLevelRefTable)
{
	Std_ReturnType returnStatus;
	boolean levelFound = FALSE;
	Dcm_SecLevelType currentSecurityLevel;

	returnStatus = DslGetSecurityLevel(&currentSecurityLevel);
	if (returnStatus == E_OK) {
	    if( (*securityLevelRefTable)->Arc_EOL ) {
            /* No security level reference configured, no check should be done. */
            levelFound = TRUE;
	    } else {
            while ( ((*securityLevelRefTable)->DspSecurityLevel != currentSecurityLevel) && (!(*securityLevelRefTable)->Arc_EOL) ) {
                securityLevelRefTable++;
            }
            if (!(*securityLevelRefTable)->Arc_EOL) {
                levelFound = TRUE;
            }
	    }
	}

	return levelFound;
}


static Std_ReturnType askApplicationForSessionPermission(Dcm_SesCtrlType newSessionLevel)
{
	Std_ReturnType returnCode = E_OK;
	const Dcm_DslSessionControlType *sesControl = Dcm_ConfigPtr->Dsl->DslSessionControl;
	Dcm_SesCtrlType currentSessionLevel;
	Std_ReturnType result;

	while ( (!sesControl->Arc_EOL) && (returnCode != E_SESSION_NOT_ALLOWED)) {
		if (sesControl->GetSesChgPermission != NULL) {
			result = Dcm_GetSesCtrlType(&currentSessionLevel);
			if (result == E_OK) {
				result = sesControl->GetSesChgPermission(currentSessionLevel ,newSessionLevel);
				if (result != E_OK) {
					returnCode = result;
				}
			} else {
				returnCode = E_NOT_OK;
			}
		}
		sesControl++;
	}

	return returnCode;
}


void DspUdsDiagnosticSessionControl(const PduInfoType *pduRxData, PduIdType txPduId, PduInfoType *pduTxData)
{
	/** @req DCM250 */
	const Dcm_DspSessionRowType *sessionRow = Dcm_ConfigPtr->Dsp->DspSession->DspSessionRow;
	Dcm_SesCtrlType reqSessionType;
	Std_ReturnType result;
	Dcm_ProtocolType activeProtocolID;

	if (pduRxData->SduLength == 2) {
		reqSessionType = pduRxData->SduDataPtr[1];
		// Check if type exist in session table
		while ((sessionRow->DspSessionLevel != reqSessionType) && (!sessionRow->Arc_EOL) ) {
			sessionRow++;
		}

		if (!sessionRow->Arc_EOL) {
			result = askApplicationForSessionPermission(reqSessionType);
			if (result == E_OK) {
				DslSetSesCtrlType(reqSessionType);		/** @req DCM311 */

				dspUdsSessionControlData.sessionPending = TRUE;
				dspUdsSessionControlData.session = reqSessionType;
				dspUdsSessionControlData.sessionPduId = txPduId;

				pduTxData->SduDataPtr[1] = reqSessionType;

				if( E_OK == DslGetActiveProtocol(&activeProtocolID) ) {
					// Create positive response
					if( DCM_UDS_ON_CAN == activeProtocolID ) {
						pduTxData->SduDataPtr[2] = sessionRow->DspSessionP2ServerMax >> 8;
						pduTxData->SduDataPtr[3] = sessionRow->DspSessionP2ServerMax;
						uint16_t p2ServerStarMax10ms = sessionRow->DspSessionP2StarServerMax / 10;
						pduTxData->SduDataPtr[4] = p2ServerStarMax10ms >> 8;
						pduTxData->SduDataPtr[5] = p2ServerStarMax10ms;
						pduTxData->SduLength = 6;
					}
					else {
						pduTxData->SduLength = 2;
					}
				}
				else {
					pduTxData->SduLength = 2;
				}
				DsdDspProcessingDone(DCM_E_POSITIVERESPONSE);
			}
			else {
				// TODO: Add handling of special case of E_FORCE_RCRRP (Dcm138)
				DsdDspProcessingDone(DCM_E_CONDITIONSNOTCORRECT);	/** @req DCM308 */
			}
		}
		else {
			DsdDspProcessingDone(DCM_E_SUBFUNCTIONNOTSUPPORTED);	/** @req DCM307 */
		}
	}
	else {
		// Wrong length
		DsdDspProcessingDone(DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT);
	}
}


void DspUdsEcuReset(const PduInfoType *pduRxData, PduIdType txPduId, PduInfoType *pduTxData)
{
	/** @req DCM260 */
	uint8 reqResetType;

	if (pduRxData->SduLength == 2) {
		reqResetType = pduRxData->SduDataPtr[1];

		switch (reqResetType)
		{
		case DCM_HARD_RESET:
		case DCM_KEY_OFF_ON_RESET:
		case DCM_SOFT_RESET:
			// TODO: Ask application for permission (Dcm373) (Dcm375) (Dcm377)

			dspUdsEcuResetData.resetPduId = txPduId;
			dspUdsEcuResetData.pduTxData = pduTxData;
			dspUdsEcuResetData.resetType = reqResetType;

			switch( DcmE_EcuReset(dspUdsEcuResetData.resetType) )
			{
			case E_OK:
				dspUdsEcuResetData.resetPending = DCM_DSP_RESET_WAIT_TX_CONF;
				// Create positive response
				pduTxData->SduDataPtr[1] = reqResetType;
				pduTxData->SduLength = 2;
				DsdDspProcessingDone(DCM_E_POSITIVERESPONSE);
				break;
			case E_PENDING:
				dspUdsEcuResetData.resetPending = DCM_DSP_RESET_PENDING;
				break;
			case E_NOT_OK:
			default:
				dspUdsEcuResetData.resetPending = DCM_DSP_RESET_NO_RESET;
				DsdDspProcessingDone(DCM_E_CONDITIONSNOTCORRECT);
				break;
			}
			break;

		default:
			DsdDspProcessingDone(DCM_E_SUBFUNCTIONNOTSUPPORTED);
			break;
		}
	}
	else {
		// Wrong length
		DsdDspProcessingDone(DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT);
	}
}

#if defined(USE_DEM) && defined(DCM_USE_SERVICE_CLEARDIAGNOSTICINFORMATION)
void DspUdsClearDiagnosticInformation(const PduInfoType *pduRxData, PduInfoType *pduTxData)
{
	/** @req DCM247 */
	uint32 dtc;
	Dem_ReturnClearDTCType result;

	if (pduRxData->SduLength == 4) {
		dtc = BYTES_TO_DTC(pduRxData->SduDataPtr[1], pduRxData->SduDataPtr[2], pduRxData->SduDataPtr[3]);

		result = Dem_ClearDTC(dtc, DEM_DTC_FORMAT_UDS, DEM_DTC_ORIGIN_PRIMARY_MEMORY); /** @req DCM005 */

		switch (result)
		{
		case DEM_CLEAR_OK:
			// Create positive response
			pduTxData->SduLength = 1;
			DsdDspProcessingDone(DCM_E_POSITIVERESPONSE);
			break;

		default:
			DsdDspProcessingDone(DCM_E_REQUESTOUTOFRANGE);
			break;
		}
	}
	else {
		// Wrong length
		DsdDspProcessingDone(DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT);
	}
}
#endif

#if defined(USE_DEM) && defined(DCM_USE_SERVICE_READDTCINFORMATION)
//TODO: Fix this
#if 0
static void udsReportDtc(uint32 dtc, uint8 *buffer)
{
	switch( DEM_TYPE_OF_DTC_SUPPORTED )
	{
	case DEM_ISO14229_1: // UDS
		buffer[0] = DTC_HIGH_BYTE(dtc);
		buffer[1] = DTC_MID_BYTE(dtc);
		buffer[2] = DTC_LOW_BYTE(dtc);
		break;
	case DEM_ISO15031_6: // OBD
		buffer[0] = DTC_MID_BYTE(dtc);
		buffer[1] = DTC_LOW_BYTE(dtc);
		buffer[2] = 0x00;
		break;
	default:
		break;
	}
}
#endif
static Dcm_NegativeResponseCodeType udsReadDtcInfoSub_0x01_0x07_0x11_0x12(const PduInfoType *pduRxData, PduInfoType *pduTxData)
{
	typedef struct {
		uint8		SID;
		uint8		reportType;
		uint8 		dtcStatusAvailabilityMask;
		uint8		dtcFormatIdentifier;
		uint8		dtcCountHighByte;
		uint8		dtcCountLowByte;
	} TxDataType;

	Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;
	Dem_ReturnSetFilterType setDtcFilterResult;

	// Setup the DTC filter
	switch (pduRxData->SduDataPtr[1]) 	/** @req DCM293 */
	{
	case 0x01:	// reportNumberOfDTCByStatusMask
		setDtcFilterResult = Dem_SetDTCFilter(pduRxData->SduDataPtr[2], DEM_DTC_KIND_ALL_DTCS, DEM_DTC_FORMAT_UDS, DEM_DTC_ORIGIN_PRIMARY_MEMORY, DEM_FILTER_WITH_SEVERITY_NO, VALUE_IS_NOT_USED, DEM_FILTER_FOR_FDC_NO);
		break;

	case 0x07:	// reportNumberOfDTCBySeverityMaskRecord
		setDtcFilterResult = Dem_SetDTCFilter(pduRxData->SduDataPtr[3], DEM_DTC_KIND_ALL_DTCS, DEM_DTC_FORMAT_UDS, DEM_DTC_ORIGIN_PRIMARY_MEMORY, DEM_FILTER_WITH_SEVERITY_YES, pduRxData->SduDataPtr[2], DEM_FILTER_FOR_FDC_NO);
		break;

	case 0x11:	// reportNumberOfMirrorMemoryDTCByStatusMask
		setDtcFilterResult = Dem_SetDTCFilter(pduRxData->SduDataPtr[2], DEM_DTC_KIND_ALL_DTCS, DEM_DTC_FORMAT_UDS, DEM_DTC_ORIGIN_MIRROR_MEMORY, DEM_FILTER_WITH_SEVERITY_NO, VALUE_IS_NOT_USED, DEM_FILTER_FOR_FDC_NO);
		break;

	case 0x12:	// reportNumberOfEmissionRelatedOBDDTCByStatusMask
		setDtcFilterResult = Dem_SetDTCFilter(pduRxData->SduDataPtr[2], DEM_DTC_KIND_EMISSION_REL_DTCS, DEM_DTC_FORMAT_UDS, DEM_DTC_ORIGIN_PRIMARY_MEMORY, DEM_FILTER_WITH_SEVERITY_NO, VALUE_IS_NOT_USED, DEM_FILTER_FOR_FDC_NO);
		break;

	default:
		setDtcFilterResult = DEM_WRONG_FILTER;
		break;
	}

	if (setDtcFilterResult == DEM_FILTER_ACCEPTED) {
		Std_ReturnType result;
		Dem_ReturnGetNumberOfFilteredDTCType getNumerResult;
		uint16 numberOfFilteredDtc;
		uint8 dtcStatusMask;
		//lint --e(826)	PC-Lint exception - Suspicious pointer conversion
		//lint --e(927)	PC-Lint exception - Pointer to pointer cast
		TxDataType *txData = (TxDataType*)pduTxData->SduDataPtr;

		/** @req DCM376 */
		getNumerResult = Dem_GetNumberOfFilteredDtc(&numberOfFilteredDtc);
		if (getNumerResult == DEM_NUMBER_OK) {
			result = Dem_GetDTCStatusAvailabilityMask(&dtcStatusMask);
			if (result != E_OK) {
				dtcStatusMask = 0;
			}

			// Create positive response (ISO 14229-1 table 251)
			txData->reportType = pduRxData->SduDataPtr[1];						// reportType
			txData->dtcStatusAvailabilityMask = dtcStatusMask;					// DTCStatusAvailabilityMask
			txData->dtcFormatIdentifier = Dem_GetTranslationType();				// DTCFormatIdentifier
			txData->dtcCountHighByte = (numberOfFilteredDtc >> 8);				// DTCCount high byte
			txData->dtcCountLowByte = (numberOfFilteredDtc & 0xFFu);			// DTCCount low byte
			pduTxData->SduLength = 6;
		} else {
			// TODO: What to do?
			responseCode = DCM_E_GENERALREJECT;
		}
	}
	else {
		responseCode = DCM_E_REQUESTOUTOFRANGE;
	}

	return responseCode;
}


static Dcm_NegativeResponseCodeType udsReadDtcInfoSub_0x02_0x0A_0x0F_0x13_0x15(const PduInfoType *pduRxData, PduInfoType *pduTxData)
{
	Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;
	Dem_ReturnSetFilterType setDtcFilterResult;

	typedef struct {
		uint8		dtcHighByte;
		uint8		dtcMiddleByte;
		uint8		dtcLowByte;
		uint8		statusOfDtc;
	} dtcAndStatusRecordType;

	typedef struct {
		uint8					SID;
		uint8					reportType;
		uint8 					dtcStatusAvailabilityMask;
		dtcAndStatusRecordType	dtcAndStatusRecord[];
	} TxDataType;

	// Setup the DTC filter
	switch (pduRxData->SduDataPtr[1]) 	/** @req DCM378 */
	{
	case 0x02:	// reportDTCByStatusMask
		setDtcFilterResult = Dem_SetDTCFilter(pduRxData->SduDataPtr[2], DEM_DTC_KIND_ALL_DTCS, DEM_DTC_FORMAT_UDS, DEM_DTC_ORIGIN_PRIMARY_MEMORY, DEM_FILTER_WITH_SEVERITY_NO, VALUE_IS_NOT_USED, DEM_FILTER_FOR_FDC_NO);
		break;

	case 0x0A:	// reportSupportedDTC
		setDtcFilterResult = Dem_SetDTCFilter(DEM_DTC_STATUS_MASK_ALL, DEM_DTC_KIND_ALL_DTCS, DEM_DTC_FORMAT_UDS, DEM_DTC_ORIGIN_PRIMARY_MEMORY, DEM_FILTER_WITH_SEVERITY_NO, VALUE_IS_NOT_USED, DEM_FILTER_FOR_FDC_NO);
		break;

	case 0x0F:	// reportMirrorMemoryDTCByStatusMask
		setDtcFilterResult = Dem_SetDTCFilter(pduRxData->SduDataPtr[2], DEM_DTC_KIND_ALL_DTCS, DEM_DTC_FORMAT_UDS, DEM_DTC_ORIGIN_MIRROR_MEMORY, DEM_FILTER_WITH_SEVERITY_NO, VALUE_IS_NOT_USED, DEM_FILTER_FOR_FDC_NO);
		break;

	case 0x13:	// reportEmissionRelatedOBDDTCByStatusMask
		setDtcFilterResult = Dem_SetDTCFilter(pduRxData->SduDataPtr[2], DEM_DTC_KIND_EMISSION_REL_DTCS, DEM_DTC_FORMAT_UDS, DEM_DTC_ORIGIN_PRIMARY_MEMORY, DEM_FILTER_WITH_SEVERITY_NO, VALUE_IS_NOT_USED, DEM_FILTER_FOR_FDC_NO);
		break;

	case 0x15:	// reportDTCWithPermanentStatus
		setDtcFilterResult = Dem_SetDTCFilter(DEM_DTC_STATUS_MASK_ALL, DEM_DTC_KIND_ALL_DTCS, DEM_DTC_FORMAT_UDS, DEM_DTC_ORIGIN_PERMANENT_MEMORY, DEM_FILTER_WITH_SEVERITY_NO, VALUE_IS_NOT_USED, DEM_FILTER_FOR_FDC_NO);
		break;

	default:
		setDtcFilterResult = DEM_WRONG_FILTER;
		break;
	}

	if (setDtcFilterResult == DEM_FILTER_ACCEPTED) {
		uint8 dtcStatusMask;
		//lint --e(826)	PC-Lint exception - Suspicious pointer conversion
		//lint --e(927)	PC-Lint exception - Pointer to pointer cast
		TxDataType *txData = (TxDataType*)pduTxData->SduDataPtr;
		Dem_ReturnGetNextFilteredDTCType getNextFilteredDtcResult;
		uint32 dtc;
		Dem_EventStatusExtendedType dtcStatus;
		uint16 nrOfDtcs = 0;
		Std_ReturnType result;

		/** @req DCM377 */
		result = Dem_GetDTCStatusAvailabilityMask(&dtcStatusMask);
		if (result != E_OK) {
			dtcStatusMask = 0;
		}

		// Create positive response (ISO 14229-1 table 252)
		txData->reportType = pduRxData->SduDataPtr[1];
		txData->dtcStatusAvailabilityMask = dtcStatusMask;

		if (dtcStatusMask != 0x00) {	/** @req DCM008 */
			getNextFilteredDtcResult = Dem_GetNextFilteredDTC(&dtc, &dtcStatus);
			while (getNextFilteredDtcResult == DEM_FILTERED_OK) {
				//TODO: udsReportDtc(dtc, (uint8*)&txData->dtcAndStatusRecord[nrOfDtcs]);
				txData->dtcAndStatusRecord[nrOfDtcs].dtcHighByte = DTC_HIGH_BYTE(dtc);
				txData->dtcAndStatusRecord[nrOfDtcs].dtcMiddleByte = DTC_MID_BYTE(dtc);
				txData->dtcAndStatusRecord[nrOfDtcs].dtcLowByte = DTC_LOW_BYTE(dtc);
				txData->dtcAndStatusRecord[nrOfDtcs].statusOfDtc = dtcStatus;
				nrOfDtcs++;
				getNextFilteredDtcResult = Dem_GetNextFilteredDTC(&dtc, &dtcStatus);
			}

			if (getNextFilteredDtcResult != DEM_FILTERED_NO_MATCHING_DTC) {
				responseCode = DCM_E_REQUESTOUTOFRANGE;
			}
		}
		pduTxData->SduLength = (PduLengthType)(3 + (nrOfDtcs * sizeof(dtcAndStatusRecordType)));
	}
	else {
		responseCode = DCM_E_REQUESTOUTOFRANGE;
	}

	return responseCode;
}

// PC-Lint (715 etc): Remove errors until function is filled.
//lint -e{715, 838, 818}		Symbol not referenced, responseCode not used, txData should be const
static Dcm_NegativeResponseCodeType udsReadDtcInfoSub_0x08(const PduInfoType *pduRxData, PduInfoType *pduTxData)
{
	Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;

	(void)pduRxData;
	(void)pduTxData;

	// TODO: Not supported yet, (DEM module does not currently support severity).
	responseCode = DCM_E_REQUESTOUTOFRANGE;

	return responseCode;
}


// PC-Lint (715 etc): Remove errors until function is filled.
//lint -e{715, 838, 818}		Symbol not referenced, responseCode not used, txData should be const
static Dcm_NegativeResponseCodeType udsReadDtcInfoSub_0x09(const PduInfoType *pduRxData, PduInfoType *pduTxData)
{
	Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;

	(void)pduRxData;
	(void)pduTxData;

	// TODO: Not supported yet, (DEM module does not currently support severity).
	responseCode = DCM_E_REQUESTOUTOFRANGE;

	return responseCode;
}


static Dcm_NegativeResponseCodeType udsReadDtcInfoSub_0x06_0x10(const PduInfoType *pduRxData, PduInfoType *pduTxData)
{
	Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;
	Dem_DTCOriginType dtcOrigin;
	uint8 startRecNum;
	uint8 endRecNum;

	// Switch on sub function
	switch (pduRxData->SduDataPtr[1]) 	/** @req DCM378 */
	{
	case 0x06:	// reportDTCExtendedDataRecordByDTCNumber
		dtcOrigin = DEM_DTC_ORIGIN_PRIMARY_MEMORY;
		break;

	case 0x10:	// reportMirrorMemoryDTCExtendedDataRecordByDTCNumber
		dtcOrigin = DEM_DTC_ORIGIN_MIRROR_MEMORY;
		break;

	default:
		responseCode = DCM_E_SUBFUNCTIONNOTSUPPORTED;
		dtcOrigin = 0;
		break;
	}

	// Switch on record number
	switch (pduRxData->SduDataPtr[5])
	{
	case 0xFF:	// Report all Extended Data Records for a particular DTC
		startRecNum = 0x00;
		endRecNum = 0xEF;
		break;

	case 0xFE:	// Report all OBD Extended Data Records for a particular DTC
		startRecNum = 0x90;
		endRecNum = 0xEF;
		break;

	default:	// Report one specific Extended Data Records for a particular DTC
		startRecNum = pduRxData->SduDataPtr[5];
		endRecNum = startRecNum;
		break;
	}

	if (responseCode == DCM_E_POSITIVERESPONSE) {
		Dem_ReturnGetStatusOfDTCType getStatusOfDtcResult;
		uint32 dtc;
		Dem_EventStatusExtendedType statusOfDtc;

		dtc = BYTES_TO_DTC(pduRxData->SduDataPtr[2], pduRxData->SduDataPtr[3], pduRxData->SduDataPtr[4]);
		getStatusOfDtcResult = Dem_GetStatusOfDTC(dtc, dtcOrigin, &statusOfDtc); /** @req DCM295 */ /** @req DCM475 */
		if (getStatusOfDtcResult == DEM_STATUS_OK) {
			Dem_ReturnGetExtendedDataRecordByDTCType getExtendedDataRecordByDtcResult;
			uint8 recNum;
			uint16 recLength;
			uint16 txIndex = 6;
			boolean foundValidRecordNumber = FALSE;

			/** @req DCM297 */ /** @req DCM474 */ /** @req DCM386 */
			pduTxData->SduDataPtr[1] = pduRxData->SduDataPtr[1];			// Sub function
			//TODO: udsReportDtc(dtc, &pduTxData->SduDataPtr[2]);
			pduTxData->SduDataPtr[2] = DTC_HIGH_BYTE(dtc);					// DTC high byte
			pduTxData->SduDataPtr[3] = DTC_MID_BYTE(dtc);					// DTC mid byte
			pduTxData->SduDataPtr[4] = DTC_LOW_BYTE(dtc);					// DTC low byte
			pduTxData->SduDataPtr[5] = statusOfDtc;							// DTC status
			for (recNum = startRecNum; recNum <= endRecNum; recNum++) {
				recLength = pduTxData->SduLength - (txIndex + 1);	// Calculate what's left in buffer
				/** @req DCM296 */ /** @req DCM476 */ /** @req DCM382 */
				getExtendedDataRecordByDtcResult = Dem_GetExtendedDataRecordByDTC(dtc, dtcOrigin, recNum, &pduTxData->SduDataPtr[txIndex+1], &recLength);
				if (getExtendedDataRecordByDtcResult == DEM_RECORD_OK) {
				    foundValidRecordNumber = TRUE;
	                if (recLength > 0) {
					pduTxData->SduDataPtr[txIndex++] = recNum;
					/* Instead of calling Dem_GetSizeOfExtendedDataRecordByDTC() the result from Dem_GetExtendedDataRecordByDTC() is used */
					/** @req DCM478 */ /** @req DCM479 */ /** @req DCM480 */
					txIndex += recLength;
				}
				}
			}

			pduTxData->SduLength = txIndex;

			if (!foundValidRecordNumber) {
			    responseCode = DCM_E_REQUESTOUTOFRANGE;
			}
		}
		else {
			responseCode = DCM_E_REQUESTOUTOFRANGE;
		}
	}

	return responseCode;
}


// PC-Lint (715 etc): Remove errors until function is filled.
//lint -e{715, 838, 818}		Symbol not referenced, responseCode not used, txData should be const
static Dcm_NegativeResponseCodeType udsReadDtcInfoSub_0x03(const PduInfoType *pduRxData, PduInfoType *pduTxData)
{
	Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;

	uint16 numFilteredRecords = 0;
	uint32 dtc = 0;
	uint8 recordNumber = 0;
	uint16 nofBytesCopied = 0;
	(void)pduRxData;
	/* @req DCM298 */
	if( (DEM_FILTER_ACCEPTED == Dem_SetFreezeFrameRecordFilter(DEM_DTC_FORMAT_UDS, &numFilteredRecords)) &&
	        ( (SID_LEN + SF_LEN + (DTC_LEN + FF_REC_NUM_LEN)*numFilteredRecords) <= pduTxData->SduLength )) {
	    for( uint16 i = 0; (i < numFilteredRecords) && (DCM_E_POSITIVERESPONSE == responseCode); i++ ) {
	        /* @req DCM299 */
	        if( DEM_FILTERED_OK == Dem_GetNextFilteredRecord(&dtc, &recordNumber) ) {
	            /* @req DCM300 */
	            pduTxData->SduDataPtr[SID_LEN + SF_LEN + nofBytesCopied++] = DTC_HIGH_BYTE(dtc);
	            pduTxData->SduDataPtr[SID_LEN + SF_LEN + nofBytesCopied++] = DTC_MID_BYTE(dtc);
	            pduTxData->SduDataPtr[SID_LEN + SF_LEN + nofBytesCopied++] = DTC_LOW_BYTE(dtc);
	            pduTxData->SduDataPtr[SID_LEN + SF_LEN + nofBytesCopied++] = recordNumber;
	        } else {
	            responseCode = DCM_E_REQUESTOUTOFRANGE;
	        }
	    }
	} else {
	    responseCode = DCM_E_REQUESTOUTOFRANGE;
	}

    pduTxData->SduDataPtr[0] = 0x59;    // positive response
    pduTxData->SduDataPtr[1] = 0x03;    // subid
    pduTxData->SduLength = SID_LEN + SF_LEN + nofBytesCopied;

	return responseCode;
}


// PC-Lint (715 etc): Remove errors until function is filled.
//lint -e{715, 838, 818}		Symbol not referenced, responseCode not used, txData should be const
static Dcm_NegativeResponseCodeType udsReadDtcInfoSub_0x04(const PduInfoType *pduRxData, PduInfoType *pduTxData)
{
	// 1. Only consider Negative Response 0x10

	Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;
	uint32 DtcNumber = 0;
	uint8 RecordNumber = 0;
	uint16 SizeOfTxBuf = pduTxData->SduLength;
	Dem_ReturnGetFreezeFrameDataByDTCType GetFFbyDtcReturnCode = DEM_GET_FFDATABYDTC_OK;
	Dem_ReturnGetStatusOfDTCType GetStatusOfDtc = DEM_STATUS_OK;
	Dem_EventStatusExtendedType DtcStatus = 0;

	// Now let's assume DTC has 3 bytes.
	DtcNumber = (((uint32)pduRxData->SduDataPtr[2])<<16) +
				(((uint32)pduRxData->SduDataPtr[3])<<8) +
				((uint32)pduRxData->SduDataPtr[4]);

    GetStatusOfDtc = Dem_GetStatusOfDTC(DtcNumber, DEM_DTC_ORIGIN_PRIMARY_MEMORY, &DtcStatus); /** @req DCM383 */
    switch (GetStatusOfDtc) {
        case DEM_STATUS_OK:
            break;
        default:
            return DCM_E_REQUESTOUTOFRANGE;
    }

	RecordNumber = pduRxData->SduDataPtr[5];

	if( 0xFF == RecordNumber ) {
		/* Request for all freeze frames */
		GetFFbyDtcReturnCode = DEM_GET_FFDATABYDTC_WRONG_DTC;
		uint16 nofBytesCopied = 0;
		uint16 bufSizeLeft = 0;
		Dem_ReturnGetFreezeFrameDataByDTCType ret = DEM_GET_FFDATABYDTC_OK;
		for(uint8 record = 0; record < RecordNumber; record++) { /* @req DCM385 */
			bufSizeLeft = pduTxData->SduLength - 6 - nofBytesCopied;
			ret = Dem_GetFreezeFrameDataByDTC(DtcNumber, DEM_DTC_ORIGIN_PRIMARY_MEMORY,
					record, &pduTxData->SduDataPtr[6 + nofBytesCopied], &bufSizeLeft);
			if( DEM_GET_FFDATABYDTC_OK == ret ) {
				nofBytesCopied += bufSizeLeft;
				/* At least one OK! */
				GetFFbyDtcReturnCode = DEM_GET_FFDATABYDTC_OK;
			}
		}
		SizeOfTxBuf = nofBytesCopied;
	} else {
		GetFFbyDtcReturnCode = Dem_GetFreezeFrameDataByDTC(DtcNumber, DEM_DTC_ORIGIN_PRIMARY_MEMORY,
			RecordNumber, &pduTxData->SduDataPtr[6], &SizeOfTxBuf);
	}
	// Negative response
	switch (GetFFbyDtcReturnCode) {
		case DEM_GET_FFDATABYDTC_OK:
			pduTxData->SduLength = SizeOfTxBuf + 6;
			break;
		default:
			return DCM_E_REQUESTOUTOFRANGE;
	}

	// Positive response
	// See ISO 14229(2006) Table 254
	pduTxData->SduDataPtr[0] = 0x59;	// positive response
	pduTxData->SduDataPtr[1] = 0x04;	// subid
	pduTxData->SduDataPtr[2] = pduRxData->SduDataPtr[2];	// DTC
	pduTxData->SduDataPtr[3] = pduRxData->SduDataPtr[3];
	pduTxData->SduDataPtr[4] = pduRxData->SduDataPtr[4];
	pduTxData->SduDataPtr[5] = (uint8)DtcStatus;	//status
	return responseCode;
}


// PC-Lint (715 etc): Remove errors until function is filled.
//lint -e{715, 838, 818}		Symbol not referenced, responseCode not used, txData should be const
static Dcm_NegativeResponseCodeType udsReadDtcInfoSub_0x05(const PduInfoType *pduRxData, PduInfoType *pduTxData)
{
	Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;

	(void)pduRxData;
	(void)pduTxData;

	// TODO: Not supported yet
	responseCode = DCM_E_REQUESTOUTOFRANGE;

	return responseCode;
}


// PC-Lint (715 etc): Remove errors until function is filled.
//lint -e{715, 838, 818}		Symbol not referenced, responseCode not used, txData should be const
static Dcm_NegativeResponseCodeType udsReadDtcInfoSub_0x0B_0x0C_0x0D_0x0E(const PduInfoType *pduRxData, PduInfoType *pduTxData)
{
	Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;

	(void)pduRxData;
	(void)pduTxData;

	// TODO: Not supported yet
	responseCode = DCM_E_REQUESTOUTOFRANGE;

	return responseCode;
}


// PC-Lint (715 etc): Remove errors until function is filled.
//lint -e{715, 838, 818}		Symbol not referenced, responseCode not used, txData should be const
static Dcm_NegativeResponseCodeType udsReadDtcInfoSub_0x14(const PduInfoType *pduRxData, PduInfoType *pduTxData)
{
	Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;
	(void)pduRxData; /* Avoid compiler warning */
	(void)pduTxData; /* Avoid compiler warning */

	// TODO: Not supported yet
	/** !464 */
	responseCode = DCM_E_REQUESTOUTOFRANGE;

	return responseCode;
}


void DspUdsReadDtcInformation(const PduInfoType *pduRxData, PduInfoType *pduTxData)
{
	/** @req DCM248 */
	// Sub function number         0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F 10 11 12 13 14 15
	const uint8 sduLength[0x16] = {0, 3, 3, 2, 6, 3, 6, 4, 4, 5, 2, 2, 2, 2, 2, 3, 6, 3, 3, 3, 2, 2};
    /* Sub function number                 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F 10 11 12 13 14 15 */
    const boolean subFncSupported[0x16] = {1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0,};

	Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;

	uint8 subFunctionNumber = pduRxData->SduDataPtr[1];

	// Check length
	if (subFunctionNumber <= 0x15) {
        if(subFncSupported[subFunctionNumber]) {
		if (pduRxData->SduLength == sduLength[subFunctionNumber]) {
			switch (subFunctionNumber)
			{
			case 0x01:	// reportNumberOfDTCByStatusMask
			case 0x07:	// reportNumberOfDTCBySeverityMaskRecord
			case 0x11:	// reportNumberOfMirrorMemoryDTCByStatusMask
			case 0x12:	// reportNumberOfEmissionRelatedOBDDTCByStatusMask
				responseCode = udsReadDtcInfoSub_0x01_0x07_0x11_0x12(pduRxData, pduTxData);
				break;

			case 0x02:	// reportDTCByStatusMask
			case 0x0A:	// reportSupportedDTC
			case 0x0F:	// reportMirrorMemoryDTCByStatusMask
			case 0x13:	// reportEmissionRelatedOBDDTCByStatusMask
			case 0x15:	// reportDTCWithPermanentStatus
				responseCode = udsReadDtcInfoSub_0x02_0x0A_0x0F_0x13_0x15(pduRxData, pduTxData);
				break;

			case 0x08:	// reportDTCBySeverityMaskRecord
				responseCode = udsReadDtcInfoSub_0x08(pduRxData, pduTxData);
				break;

			case 0x09:	// reportSeverityInformationOfDTC
				responseCode = udsReadDtcInfoSub_0x09(pduRxData, pduTxData);
				break;

			case 0x06:	// reportDTCExtendedDataRecordByDTCNumber
			case 0x10:	// reportMirrorMemoryDTCExtendedDataRecordByDTCNumber
				responseCode = udsReadDtcInfoSub_0x06_0x10(pduRxData, pduTxData);
				break;

			case 0x03:	// reportDTCSnapshotIdentidication
				responseCode = udsReadDtcInfoSub_0x03(pduRxData, pduTxData);
				break;

			case 0x04:	// reportDTCSnapshotByDtcNumber
				responseCode = udsReadDtcInfoSub_0x04(pduRxData, pduTxData);
				break;

			case 0x05:	// reportDTCSnapshotRecordNumber
				responseCode = udsReadDtcInfoSub_0x05(pduRxData, pduTxData);
				break;

			case 0x0B:	// reportFirstTestFailedDTC
			case 0x0C:	// reportFirstConfirmedDTC
			case 0x0D:	// reportMostRecentTestFailedDTC
			case 0x0E:	// reportMostRecentConfirmedDTC
				responseCode = udsReadDtcInfoSub_0x0B_0x0C_0x0D_0x0E(pduRxData, pduTxData);
				break;

			case 0x14:	// reportDTCFaultDetectionCounter
				responseCode = udsReadDtcInfoSub_0x14(pduRxData, pduTxData);
				break;

			default:
				// Unknown sub function
				responseCode = DCM_E_REQUESTOUTOFRANGE;
				break;
			}
		}
		else {
			// Wrong length
			responseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
		}
        } else {
            /* Subfunction not supported */
            responseCode = DCM_E_SUBFUNCTIONNOTSUPPORTED;
        }
	}
	else {
		// Sub function out of range
		responseCode = DCM_E_REQUESTOUTOFRANGE;
	}

	DsdDspProcessingDone(responseCode);
}
#endif

/**
**		This Function for check the pointer of Dynamically Did Sourced by Did buffer using a didNr
**/
#ifdef DCM_USE_SERVICE_DYNAMICALLYDEFINEDATAIDENTIFIER
static boolean LookupDDD(uint16 didNr,  const Dcm_DspDDDType **DDid )	
{
	uint8 i;
	boolean ret = FALSE;
	const Dcm_DspDDDType* DDidptr = &dspDDD[0];
	

	if (didNr < DCM_PERODICDID_HIGH_MASK) {
		return ret;
	}


	for(i = 0;((i < DCM_MAX_DDD_NUMBER) && (ret == FALSE)); i++)
	{
		if(DDidptr->DynamicallyDid == didNr)
		{
			ret = TRUE;
		
		}
		else
		{
			DDidptr++;
		}
	}
	if(ret == TRUE)
	{
		*DDid = DDidptr;
	}

	return ret;
}
#endif

static boolean lookupDid(uint16 didNr, const Dcm_DspDidType **didPtr)
{
	const Dcm_DspDidType *dspDid = Dcm_ConfigPtr->Dsp->DspDid;
	boolean didFound = FALSE;

	while ((dspDid->DspDidIdentifier != didNr) &&  (!dspDid->Arc_EOL)) {
		dspDid++;
	}

	if (!dspDid->Arc_EOL && (!dspDid->DspDidInfoRef->DspDidDynamicllyDefined)) {
		didFound = TRUE;
		*didPtr = dspDid;
	}

	return didFound;
}

static void DspCancelPendingDid(uint16 didNr, ReadDidPendingStateType pendingState, PduInfoType *pduTxData )
{
    const Dcm_DspDidType *didPtr = NULL;
    if( lookupDid(didNr, &didPtr) ) {
        if( DCM_READ_DID_PENDING_COND_CHECK == pendingState ) {
            if( didPtr->DspDidConditionCheckReadFnc != NULL ) {
                (void)didPtr->DspDidConditionCheckReadFnc(DCM_CANCEL, pduTxData->SduDataPtr);
            }
        } else if( DCM_READ_DID_PENDING_READ_DATA == pendingState ) {
            if( didPtr->DspDidReadDataFnc.AsynchDataReadFnc != NULL ) {
                if( DATA_PORT_ASYNCH == didPtr->DspDidUsePort ) {
                    (void)didPtr->DspDidReadDataFnc.AsynchDataReadFnc(DCM_CANCEL, pduTxData->SduDataPtr);
                }
            }
        } else {
            /* Not in a pending state */
            DET_REPORTERROR(MODULE_ID_DCM, 0, DCM_GLOBAL_ID, DCM_E_UNEXPECTED_EXECUTION);
        }
    }
}

static Dcm_NegativeResponseCodeType getDidData(const Dcm_DspDidType *didPtr, PduInfoType *pduTxData, uint16 *txPos, ReadDidPendingStateType *pendingState, uint16 *pendingDidLen)
{
    Dcm_NegativeResponseCodeType errorCode = DCM_E_POSITIVERESPONSE;
    Dcm_OpStatusType opStatus = DCM_INITIAL;
    Std_ReturnType result = E_OK;
    if( (DCM_READ_DID_PENDING_COND_CHECK == *pendingState) || (DCM_READ_DID_PENDING_READ_DATA == *pendingState) ) {
        opStatus = DCM_PENDING;
    }
    if( (DCM_READ_DID_PENDING_COND_CHECK == *pendingState) || (DCM_READ_DID_IDLE == *pendingState) ) {
        result = didPtr->DspDidConditionCheckReadFnc(opStatus, &errorCode);
        if(DATA_PORT_ASYNCH == didPtr->DspDidUsePort) {
            if( E_PENDING == result ) {
                *pendingState = DCM_READ_DID_PENDING_COND_CHECK;
            } else {
                *pendingState = DCM_READ_DID_IDLE;
                opStatus = DCM_INITIAL;
            }
        } else if(DATA_PORT_SYNCH == didPtr->DspDidUsePort) {
            if( E_PENDING == result ) {
                DET_REPORTERROR(MODULE_ID_DCM, 0, DCM_GLOBAL_ID, DCM_E_UNEXPECTED_RESPONSE);
                result = E_NOT_OK;
            }
        } else {
            /* Port not supported */
            DET_REPORTERROR(MODULE_ID_DCM, 0, DCM_GLOBAL_ID, DCM_E_CONFIG_INVALID);
            result = E_NOT_OK;
        }
    } else {
        /* Condition check function already called with positive return */
        result = E_OK;
        errorCode = DCM_E_POSITIVERESPONSE;
    }

    if ((result == E_OK) && (errorCode == DCM_E_POSITIVERESPONSE)) {    /** @req DCM439 */
        uint16 didLen = 0;
        if (didPtr->DspDidInfoRef->DspDidFixedLength) {  /** @req DCM436 */
            didLen = didPtr->DspDidSize;
        } else {
            if (didPtr->DspDidReadDataLengthFnc != NULL) {
                if( DCM_READ_DID_IDLE == *pendingState ) {
                    /* ReadDataLengthFunction is only allowed to return E_OK  */
                    if(E_OK != didPtr->DspDidReadDataLengthFnc(&didLen)) {
                        DET_REPORTERROR(MODULE_ID_DCM, 0, DCM_GLOBAL_ID, DCM_E_UNEXPECTED_RESPONSE);
                        result = E_NOT_OK;
                    }
                } else {
                    /* Read length function has already been called */
                    didLen = *pendingDidLen;
                }
            }
        }
        if( E_OK == result ) {
            // Now ready for reading the data!
            if ((*txPos + didLen + 2) <= pduTxData->SduLength) {
                pduTxData->SduDataPtr[(*txPos)++] = (didPtr->DspDidIdentifier >> 8) & 0xFFu;
                pduTxData->SduDataPtr[(*txPos)++] = didPtr->DspDidIdentifier & 0xFFu;

                /** @req DCM437 */
                if(DATA_PORT_SYNCH == didPtr->DspDidUsePort ) {
                    /* Synch read function is only allowed to return E_OK */
                    if(E_OK != didPtr->DspDidReadDataFnc.SynchDataReadFnc(&pduTxData->SduDataPtr[*txPos]) ) {
                        DET_REPORTERROR(MODULE_ID_DCM, 0, DCM_GLOBAL_ID, DCM_E_UNEXPECTED_RESPONSE);
                        result = E_NOT_OK;
                    }
                } else if( DATA_PORT_ASYNCH == didPtr->DspDidUsePort ) {
                    result = didPtr->DspDidReadDataFnc.AsynchDataReadFnc(opStatus, &pduTxData->SduDataPtr[*txPos]);
                } else {
                    /* Port not supported */
                    DET_REPORTERROR(MODULE_ID_DCM, 0, DCM_GLOBAL_ID, DCM_E_CONFIG_INVALID);
                    result = E_NOT_OK;
                }
                *txPos += didLen;

                if( E_PENDING == result ) {
                    *pendingState = DCM_READ_DID_PENDING_READ_DATA;
                    *txPos -= (didLen+ 2);
                    *pendingDidLen = didLen;
                    errorCode = DCM_E_RESPONSEPENDING;
                }
                else if (result != E_OK) {
                    errorCode = DCM_E_CONDITIONSNOTCORRECT;
                } else {
                    /* Did successfully read */
                    *pendingState = DCM_READ_DID_IDLE;
                }
            } else { // tx buffer full
                errorCode = DCM_E_REQUESTOUTOFRANGE;
            }
        } else {// Invalid return from readLenFunction
            errorCode = DCM_E_CONDITIONSNOTCORRECT;
        }
    } else if( E_PENDING == result ) {
        errorCode = DCM_E_RESPONSEPENDING;
    } else {    // CheckRead failed
        errorCode = DCM_E_CONDITIONSNOTCORRECT;
    }

    return errorCode;
}

static Dcm_NegativeResponseCodeType readDidData(const Dcm_DspDidType *didPtr, PduInfoType *pduTxData, uint16 *txPos,
        ReadDidPendingStateType *pendingState, uint16 *pendingDid, uint16 *pendingDidLen, uint16 *didIndex, uint16 didStartIndex)
{
	Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;

    boolean didOnlyRefsDids = (!didPtr->DspDidHasData && !didPtr->DspDidRef[0]->Arc_EOL);
    if ((didPtr->DspDidInfoRef->DspDidAccess.DspDidRead != NULL) &&
            (((didPtr->DspDidConditionCheckReadFnc != NULL) && (didPtr->DspDidReadDataFnc.AsynchDataReadFnc != NULL)) || didOnlyRefsDids) ) {	/** @req DCM433 */
        if (DspCheckSessionLevel(didPtr->DspDidInfoRef->DspDidAccess.DspDidRead->DspDidReadSessionRef)) { /** @req DCM434 */
            if (DspCheckSecurityLevel(didPtr->DspDidInfoRef->DspDidAccess.DspDidRead->DspDidReadSecurityLevelRef)) { /** @req DCM435 */
                if( !didOnlyRefsDids && (*didIndex >= didStartIndex)) {
                    /* Get the data if available */
                    responseCode = getDidData(didPtr, pduTxData, txPos, pendingState, pendingDidLen);
                    if(DCM_E_RESPONSEPENDING == responseCode) {
                        *pendingDid = didPtr->DspDidIdentifier;
                    } else if( DCM_E_POSITIVERESPONSE == responseCode ) {
                        /* Data successfully read */
                        (*didIndex)++;
                    }
                } else {
                    /* This did only references other dids or did already read. */
                    if( *didIndex >= didStartIndex ) {
                        /* Not already read. */
                        if ((*txPos + 2) <= pduTxData->SduLength) {
                            pduTxData->SduDataPtr[(*txPos)++] = (didPtr->DspDidIdentifier >> 8) & 0xFFu;
                            pduTxData->SduDataPtr[(*txPos)++] = didPtr->DspDidIdentifier & 0xFFu;
                            responseCode = DCM_E_POSITIVERESPONSE;
                        } else {
                            /* Tx buffer full. */
                            responseCode = DCM_E_REQUESTOUTOFRANGE;
                        }
                    }
                    (*didIndex)++;
                }
            }
            else {	// Not allowed in current security level
                responseCode = DCM_E_SECURITYACCESSDENIED;
            }
        }
        else {	// Not allowed in current session
            responseCode = DCM_E_SERVICENOTSUPPORTEDINACTIVESESSION;
        }
    }
	else {	// Read access not configured
        responseCode = DCM_E_REQUESTOUTOFRANGE;
	}

    for (uint16 i = 0; (!didPtr->DspDidRef[i]->Arc_EOL) && (DCM_E_POSITIVERESPONSE == responseCode); i++) {
        /* Recurse trough the rest of the dids. *//** @req DCM440 */
        responseCode = readDidData(didPtr->DspDidRef[i], pduTxData, txPos, pendingState, pendingDid, pendingDidLen, didIndex, didStartIndex);
    }

    return responseCode;
}

/**
**		This Function for read Dynamically Did data buffer Sourced by Memory address using a didNr
**/
#ifdef DCM_USE_SERVICE_DYNAMICALLYDEFINEDATAIDENTIFIER
static Dcm_NegativeResponseCodeType readDDDData(Dcm_DspDDDType *DDidPtr, uint8 *Data, uint16 *Length)
{
	uint8 i;
	uint8 dataCount;
	uint16 SourceDataLength = 0;
	const Dcm_DspDidType *SourceDidPtr = NULL;
	Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;
	*Length = 0;
	uint8_t* nextDataSlot = Data;

	for(i = 0;(i < DCM_MAX_DDDSOURCE_NUMBER) && (DDidPtr->DDDSource[i].formatOrPosition != 0)
		&&(responseCode == DCM_E_POSITIVERESPONSE);i++)
	{
		if(DDidPtr->DDDSource[i].DDDTpyeID == DCM_DDD_SOURCE_ADDRESS)
		{
			responseCode = checkAddressRange(DCM_READ_MEMORY, DDidPtr->DDDSource[i].memoryIdentifier, DDidPtr->DDDSource[i].SourceAddressOrDid, DDidPtr->DDDSource[i].Size);
			if( responseCode == DCM_E_POSITIVERESPONSE ) {
				(void)Dcm_ReadMemory(DCM_INITIAL,DDidPtr->DDDSource[i].memoryIdentifier,
										DDidPtr->DDDSource[i].SourceAddressOrDid,
										DDidPtr->DDDSource[i].Size,
										nextDataSlot);
				nextDataSlot += DDidPtr->DDDSource[i].Size;
				*Length += DDidPtr->DDDSource[i].Size;
			}
		}
		else if(DDidPtr->DDDSource[i].DDDTpyeID == DCM_DDD_SOURCE_DID)
		{
			
			if(lookupDid(DDidPtr->DDDSource[i].SourceAddressOrDid,&SourceDidPtr) == TRUE)
			{
				if(DspCheckSecurityLevel(SourceDidPtr->DspDidInfoRef->DspDidAccess.DspDidRead->DspDidReadSecurityLevelRef) != TRUE)
				{
					responseCode = DCM_E_SECURITYACCESSDENIED;
				}
				else
				{
					if(SourceDidPtr->DspDidInfoRef->DspDidFixedLength == TRUE)
					{
						SourceDataLength = SourceDidPtr->DspDidSize;
					}
					else
					{
						if(SourceDidPtr->DspDidReadDataLengthFnc != NULL)
						{
							(void)SourceDidPtr->DspDidReadDataLengthFnc(&SourceDataLength);
						}
					}
					if((SourceDidPtr->DspDidReadDataFnc.SynchDataReadFnc != NULL) && (SourceDataLength != 0) && (DCM_E_POSITIVERESPONSE == responseCode))
					{
					    if(DATA_PORT_SYNCH == SourceDidPtr->DspDidUsePort) {
					        (void)SourceDidPtr->DspDidReadDataFnc.SynchDataReadFnc(nextDataSlot);
					    } else if(DATA_PORT_ASYNCH == SourceDidPtr->DspDidUsePort) {
					        (void)SourceDidPtr->DspDidReadDataFnc.AsynchDataReadFnc(DCM_INITIAL, nextDataSlot);
					    }
						for(dataCount = 0; dataCount < DDidPtr->DDDSource[i].Size; dataCount++)
						{
							/* Shifting the data left by position (position 1 means index 0) */
							nextDataSlot[dataCount] = nextDataSlot[dataCount + DDidPtr->DDDSource[i].formatOrPosition - 1];
						}
						nextDataSlot += DDidPtr->DDDSource[i].Size;
						*Length += DDidPtr->DDDSource[i].Size;
					}
					else
					{
						responseCode = DCM_E_REQUESTOUTOFRANGE;
					}
				}
			}
			else
			{
				responseCode = DCM_E_REQUESTOUTOFRANGE;
			}
		}
		else
		{
			
			responseCode = DCM_E_REQUESTOUTOFRANGE;	
		}
	}
	return responseCode;
}
#endif

void DspUdsReadDataByIdentifier(const PduInfoType *pduRxData, PduInfoType *pduTxData)
{
	/** @req DCM253 */
	Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;
	uint16 nrOfDids;
	uint16 didNr;
	const Dcm_DspDidType *didPtr = NULL;
	Dcm_DspDDDType *DDidPtr=NULL;
	uint16 txPos = 1;
	uint16 i;
	uint16 Length;
	boolean noRequestedDidSupported = TRUE;
	ReadDidPendingStateType pendingState = DCM_READ_DID_IDLE;
	uint16 nofDidsRead = 0;
	uint16 reqDidStartIndex = 0;
	uint16 nofDidsReadInPendingReq = 0;
	uint16 pendingDid = 0;
	uint16 pendingDidLen = 0;

	if ( (((pduRxData->SduLength - 1) % 2) == 0) && ( 0 != (pduRxData->SduLength - 1))) {
		nrOfDids = (pduRxData->SduLength - 1) / 2;
	    if( DCM_READ_DID_IDLE != dspUdsReadDidPending.state ) {
	        pendingState = dspUdsReadDidPending.state;
	        txPos = dspUdsReadDidPending.txWritePos;
	        nofDidsReadInPendingReq = dspUdsReadDidPending.nofReadDids;
	        reqDidStartIndex = dspUdsReadDidPending.reqDidIndex;
	        pendingDidLen = dspUdsReadDidPending.pendingDidLength;
	    }
		/* TODO: Check security level and session for all dids before trying to read data */
	    for (i = reqDidStartIndex; (i < nrOfDids) && (responseCode == DCM_E_POSITIVERESPONSE); i++) {
			didNr = (uint16)((uint16)pduRxData->SduDataPtr[1 + (i * 2)] << 8) + pduRxData->SduDataPtr[2 + (i * 2)];
            if (lookupDid(didNr, &didPtr)) {	/** @req DCM438 */
                noRequestedDidSupported = FALSE;
                responseCode = readDidData(didPtr, pduTxData, &txPos, &pendingState, &pendingDid, &pendingDidLen, &nofDidsRead, nofDidsReadInPendingReq);
                if( DCM_E_RESPONSEPENDING == responseCode ) {
                    dspUdsReadDidPending.reqDidIndex = i;
                } else {
                    /* No pending response */
                    nofDidsReadInPendingReq = 0;
                    nofDidsRead = 0;
                }
            } else if(LookupDDD(didNr,(const Dcm_DspDDDType **)&DDidPtr) == TRUE) {
				noRequestedDidSupported = FALSE;
				/*DCM 651,DCM 652*/
				pduTxData->SduDataPtr[txPos] = (uint8)((DDidPtr->DynamicallyDid>>8) & 0xFF);
				txPos++;
				pduTxData->SduDataPtr[txPos] = (uint8)(DDidPtr->DynamicallyDid & 0xFF);
				txPos++;
				responseCode = readDDDData(DDidPtr,&(pduTxData->SduDataPtr[txPos]), &Length);
				txPos = txPos + Length;
			} else {
				/* DID not found. */
			}
		}
	} else {
		responseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
	}
	if( noRequestedDidSupported ) {
		/** @req DCM438 */
		/* None of the Dids in the request found. */
		responseCode = DCM_E_REQUESTOUTOFRANGE;
	}
	if (DCM_E_POSITIVERESPONSE == responseCode) {
		pduTxData->SduLength = txPos;
	}

	if( DCM_E_RESPONSEPENDING == responseCode) {
		dspUdsReadDidPending.state = pendingState;
		dspUdsReadDidPending.pduRxData = (PduInfoType*)pduRxData;
		dspUdsReadDidPending.pduTxData = pduTxData;
		dspUdsReadDidPending.nofReadDids = nofDidsRead;
		dspUdsReadDidPending.txWritePos = txPos;
		dspUdsReadDidPending.pendingDid = pendingDid;
		dspUdsReadDidPending.pendingDidLength = pendingDidLen;
	} else {
		dspUdsReadDidPending.state = DCM_READ_DID_IDLE;
		dspUdsReadDidPending.nofReadDids = 0;
		DsdDspProcessingDone(responseCode);
	}
}


static Dcm_NegativeResponseCodeType readDidScalingData(const Dcm_DspDidType *didPtr, const PduInfoType *pduTxData, uint16 *txPos)
{
	Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;

	if (didPtr->DspDidGetScalingInfoFnc != NULL) {
		uint16 scalingInfoLen;

		scalingInfoLen = didPtr->DspDidInfoRef->DspDidScalingInfoSize;
		if ((*txPos + scalingInfoLen + 2) <= pduTxData->SduLength) {
			Std_ReturnType result;
			Dcm_NegativeResponseCodeType errorCode;

			pduTxData->SduDataPtr[*txPos] = (didPtr->DspDidIdentifier >> 8) & 0xFFu;
			(*txPos)++;
			pduTxData->SduDataPtr[*txPos] = didPtr->DspDidIdentifier & 0xFFu;
			(*txPos)++;
			result = didPtr->DspDidGetScalingInfoFnc(DCM_INITIAL, &pduTxData->SduDataPtr[*txPos], &errorCode);	/** @req DCM394 */
			*txPos += scalingInfoLen;

			if ((result != E_OK) || (errorCode != DCM_E_POSITIVERESPONSE)) {
				responseCode = DCM_E_CONDITIONSNOTCORRECT;
			}
		}
		else { // tx buffer full
			responseCode = DCM_E_REQUESTOUTOFRANGE;
		}
	}
	else {	// DspDidGetScalingInfoFnc null pointer
		responseCode = DCM_E_CONDITIONSNOTCORRECT;
	}

	return responseCode;
}

void DspUdsReadScalingDataByIdentifier(const PduInfoType *pduRxData, PduInfoType *pduTxData)
{
	/** @req DCM258 */
	Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;
	uint16 didNr;
	const Dcm_DspDidType *didPtr = NULL;

	uint16 txPos = 1;

	if (pduRxData->SduLength == 3) {
		didNr = (uint16)((uint16)pduRxData->SduDataPtr[1] << 8) + pduRxData->SduDataPtr[2];
		if (lookupDid(didNr, &didPtr)) {
			responseCode = readDidScalingData(didPtr, pduTxData, &txPos);
		}
		else { // DID not found
			responseCode = DCM_E_REQUESTOUTOFRANGE;
		}

		if (responseCode == DCM_E_POSITIVERESPONSE) {
			pduTxData->SduLength = txPos;
		}
	}
	else {
		// Length not ok
		responseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
	}

	DsdDspProcessingDone(responseCode);
}


static Dcm_NegativeResponseCodeType writeDidData(const Dcm_DspDidType *didPtr, const PduInfoType *pduRxData, uint16 writeDidLen)
{
	Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;

	if ((didPtr->DspDidInfoRef->DspDidAccess.DspDidWrite != NULL) && (didPtr->DspDidWriteDataFnc.FixLenDataWriteFnc != NULL)) {	/** @req DCM468 */
		if (DspCheckSessionLevel(didPtr->DspDidInfoRef->DspDidAccess.DspDidWrite->DspDidWriteSessionRef)) { /** @req DCM469 */
			if (DspCheckSecurityLevel(didPtr->DspDidInfoRef->DspDidAccess.DspDidWrite->DspDidWriteSecurityLevelRef)) { /** @req DCM470 */

					uint16 didLen = 0;
					if (didPtr->DspDidInfoRef->DspDidFixedLength) {	/** @req DCM472 */
						didLen = didPtr->DspDidSize;
					} else {
						didLen = writeDidLen;
					}

					if (didLen == writeDidLen) {	/** @req DCM473 */
						Std_ReturnType result;
						if(didPtr->DspDidInfoRef->DspDidFixedLength) { /** @req DCM794 */
							result = didPtr->DspDidWriteDataFnc.FixLenDataWriteFnc(&pduRxData->SduDataPtr[3], DCM_INITIAL, &responseCode);	/** @req DCM395 */
						} else {
							result = didPtr->DspDidWriteDataFnc.DynLenDataWriteFnc(&pduRxData->SduDataPtr[3], didLen, DCM_INITIAL, &responseCode);	/** @req DCM395 */
						}
						if( result != E_OK && responseCode == DCM_E_POSITIVERESPONSE ) {
							responseCode = DCM_E_CONDITIONSNOTCORRECT;
						}
						else if( DCM_E_RESPONSEPENDING == responseCode || E_PENDING == result ) {
							responseCode = DCM_E_RESPONSEPENDING;
						}
					}
					else { // Length incorrect
						responseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
					}

				}
			else {	// Not allowed in current security level
				responseCode = DCM_E_SECURITYACCESSDENIED;
			}
		}
		else {	// Not allowed in current session
			responseCode = DCM_E_SERVICENOTSUPPORTEDINACTIVESESSION;
		}
	}
	else {	// Read access not configured
		responseCode = DCM_E_REQUESTOUTOFRANGE;
	}

	return responseCode;
}

void DspUdsWriteDataByIdentifier(const PduInfoType *pduRxData, PduInfoType *pduTxData)
{
	/** @req DCM255 */
	Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;
	uint16 didNr;
	const Dcm_DspDidType *didPtr = NULL;

	uint16 didDataLength;

	didDataLength = pduRxData->SduLength - 3;
	didNr = (uint16)((uint16)pduRxData->SduDataPtr[1] << 8) + pduRxData->SduDataPtr[2];
	if (lookupDid(didNr, &didPtr)) {	/** @req DCM467 */
		responseCode = writeDidData(didPtr, pduRxData, didDataLength);
	}
	else { // DID not found
		responseCode = DCM_E_REQUESTOUTOFRANGE;
	}

	if( DCM_E_RESPONSEPENDING != responseCode ) {
		if (responseCode == DCM_E_POSITIVERESPONSE) {
			pduTxData->SduLength = 3;
			pduTxData->SduDataPtr[1] = (didNr >> 8) & 0xFFu;
			pduTxData->SduDataPtr[2] = didNr & 0xFFu;
		}

		dspUdsWriteDidPending.state = DCM_WRITE_DID_IDLE;
		DsdDspProcessingDone(responseCode);
	}
	else {
		dspUdsWriteDidPending.state = DCM_WRITE_DID_PENDING;
		dspUdsWriteDidPending.pduRxData = pduRxData;
		dspUdsWriteDidPending.pduTxData = pduTxData;
	}
}

Dcm_NegativeResponseCodeType DspUdsSecurityAccessGetSeedSubFnc (const PduInfoType *pduRxData, PduInfoType *pduTxData, Dcm_SecLevelType requestedSecurityLevel) {

    Dcm_NegativeResponseCodeType getSeedErrorCode;
    Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;
    uint8_t cntSecRow = 0;

    // requestSeed message
    // Check if type exist in security table
    const Dcm_DspSecurityRowType *securityRow = &Dcm_ConfigPtr->Dsp->DspSecurity->DspSecurityRow[0];
    while ((securityRow->DspSecurityLevel != requestedSecurityLevel) && (!securityRow->Arc_EOL)) {
        securityRow++;
        cntSecRow++; // Get the index of the security config
    }
    if (!securityRow->Arc_EOL) {

#if (DCM_SECURITY_EOL_INDEX != 0)

        //Check if a wait is required before accepting a request
        dspUdsSecurityAccesData.currSecLevIdx = cntSecRow;
        if (dspUdsSecurityAccesData.secFalseAttemptChk[dspUdsSecurityAccesData.currSecLevIdx].startDelayTimer == DELAY_TIMER_ON_BOOT_ACTIVE) {
            responseCode = DCM_E_REQUIREDTIMEDELAYNOTEXPIRED;
        }
        else if (dspUdsSecurityAccesData.secFalseAttemptChk[dspUdsSecurityAccesData.currSecLevIdx].startDelayTimer == DELAY_TIMER_ON_EXCEEDING_LIMIT_ACTIVE) {
            responseCode = DCM_E_EXCEEDNUMBEROFATTEMPTS;
        }
        else
#endif
        {
            // Check length
            if (pduRxData->SduLength == (2 + securityRow->DspSecurityADRSize)) {    /** @req DCM321.RequestSeed */
                Dcm_SecLevelType activeSecLevel;
                Std_ReturnType result;
                result = Dcm_GetSecurityLevel(&activeSecLevel);
                if (result == E_OK) {
                    if (requestedSecurityLevel == activeSecLevel) {     /** @req DCM323 */
                        pduTxData->SduDataPtr[1] = pduRxData->SduDataPtr[1];
                        // If same level set the seed to zeroes
                        memset(&pduTxData->SduDataPtr[2], 0, securityRow->DspSecuritySeedSize);
                        pduTxData->SduLength = 2 + securityRow->DspSecuritySeedSize;
                    } else {
                        // New security level ask for seed
                        if (securityRow->GetSeed.getSeedWithoutRecord != NULL) {
                            Std_ReturnType getSeedResult;
                            if(securityRow->DspSecurityADRSize > 0) {
                                getSeedResult = securityRow->GetSeed.getSeedWithRecord(&pduRxData->SduDataPtr[2], DCM_INITIAL, &pduTxData->SduDataPtr[2], &getSeedErrorCode); /** @req DCM324.RequestSeed */
                            } else {
                                getSeedResult = securityRow->GetSeed.getSeedWithoutRecord(DCM_INITIAL, &pduTxData->SduDataPtr[2], &getSeedErrorCode); /** @req DCM324.RequestSeed */
                            }
                            if ((getSeedResult == E_OK) && (getSeedErrorCode == E_OK)) {
                                // Everything ok add sub function to tx message and send it.
                                pduTxData->SduDataPtr[1] = pduRxData->SduDataPtr[1];
                                pduTxData->SduLength = 2 + securityRow->DspSecuritySeedSize;

                                dspUdsSecurityAccesData.reqSecLevel = requestedSecurityLevel;
                                dspUdsSecurityAccesData.reqSecLevelRef = securityRow;
                                dspUdsSecurityAccesData.reqInProgress = TRUE;
                            }
                            else {
                                // GetSeed returned not ok
                                responseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
                            }
                        } else {
                            responseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
                        }
                    }
                } else {
                    // TODO: What to do?
                    responseCode = DCM_E_GENERALREJECT;
                }
            }
            else {
                // Length not ok
                responseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
            }
        }
    }
    else {
        // Requested security level not configured
        responseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
    }

    return responseCode;
}

// sendKey message
Dcm_NegativeResponseCodeType DspUdsSecurityAccessSendKeySubFnc (const PduInfoType *pduRxData, PduInfoType *pduTxData, Dcm_SecLevelType requestedSecurityLevel) {

    Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;

    /* Check whether senkey message is sent according to a valid sequence */
    if (dspUdsSecurityAccesData.reqInProgress && (requestedSecurityLevel == dspUdsSecurityAccesData.reqSecLevel)) {

        /* Client should reiterate the process of getseed msg, if sendkey fails- ISO14229 */
        dspUdsSecurityAccesData.reqInProgress = FALSE;

#if (DCM_SECURITY_EOL_INDEX != 0)
        //Check if a wait is required before accepting a request
        if (dspUdsSecurityAccesData.secFalseAttemptChk[dspUdsSecurityAccesData.currSecLevIdx].startDelayTimer == DELAY_TIMER_ON_BOOT_ACTIVE) {
            responseCode = DCM_E_REQUIREDTIMEDELAYNOTEXPIRED;
        }
        else if (dspUdsSecurityAccesData.secFalseAttemptChk[dspUdsSecurityAccesData.currSecLevIdx].startDelayTimer == DELAY_TIMER_ON_EXCEEDING_LIMIT_ACTIVE) {
            responseCode = DCM_E_EXCEEDNUMBEROFATTEMPTS;
        }
        else
#endif
        {
            if (pduRxData->SduLength == (2 + dspUdsSecurityAccesData.reqSecLevelRef->DspSecurityKeySize)) { /** @req DCM321.SendKey */
                if (dspUdsSecurityAccesData.reqSecLevelRef->CompareKey != NULL) {
                       Std_ReturnType compareKeyResult;
                       compareKeyResult = dspUdsSecurityAccesData.reqSecLevelRef->CompareKey(&pduRxData->SduDataPtr[2]); /** @req DCM324.SendKey */
                       if (compareKeyResult == E_OK) {
                           // Request accepted
                           // Kill timer
                           DslSetSecurityLevel(dspUdsSecurityAccesData.reqSecLevelRef->DspSecurityLevel); /** @req DCM325 */
                           pduTxData->SduDataPtr[1] = pduRxData->SduDataPtr[1];
                           pduTxData->SduLength = 2;
                       }
                       else {
                           responseCode = DCM_E_INVALIDKEY; /** @req DCM660 */
                       }
                   } else {
                       responseCode = DCM_E_CONDITIONSNOTCORRECT;
                   }
            }
            else {
                // Length not ok
                responseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
            }

#if (DCM_SECURITY_EOL_INDEX != 0)
            //Count the false access attempts -> Only send invalid keys events according to ISO 14229
            if (responseCode == DCM_E_INVALIDKEY) {
                dspUdsSecurityAccesData.secFalseAttemptChk[dspUdsSecurityAccesData.currSecLevIdx].secAcssAttempts += 1;

                if (dspUdsSecurityAccesData.secFalseAttemptChk[dspUdsSecurityAccesData.currSecLevIdx].secAcssAttempts >= dspUdsSecurityAccesData.reqSecLevelRef->DspSecurityNumAttDelay) {
                    //Enable delay timer
                    if (Dcm_ConfigPtr->Dsp->DspSecurity->DspSecurityRow[dspUdsSecurityAccesData.currSecLevIdx].DspSecurityDelayTime >= DCM_MAIN_FUNCTION_PERIOD_TIME_MS) {
                        dspUdsSecurityAccesData.secFalseAttemptChk[dspUdsSecurityAccesData.currSecLevIdx].startDelayTimer = DELAY_TIMER_ON_EXCEEDING_LIMIT_ACTIVE;
                        dspUdsSecurityAccesData.secFalseAttemptChk[dspUdsSecurityAccesData.currSecLevIdx].timerSecAcssAttempt = Dcm_ConfigPtr->Dsp->DspSecurity->DspSecurityRow[dspUdsSecurityAccesData.currSecLevIdx].DspSecurityDelayTime;
                    }
                    dspUdsSecurityAccesData.secFalseAttemptChk[dspUdsSecurityAccesData.currSecLevIdx].secAcssAttempts  = 0;
                }
            }
#endif
        }
    }
    else {
        // sendKey request without a preceding requestSeed
        responseCode = DCM_E_REQUESTSEQUENCEERROR;
    }

    return responseCode;
}

void DspUdsSecurityAccess(const PduInfoType *pduRxData, PduInfoType *pduTxData)
{
	/** @req DCM252 */
	Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;

	// Check sub function range (0x01 to 0x42)
	if ((pduRxData->SduDataPtr[1] >= 0x01) && (pduRxData->SduDataPtr[1] <= 0x42)) {
		boolean isRequestSeed = pduRxData->SduDataPtr[1] & 0x01u;
		Dcm_SecLevelType requestedSecurityLevel = (pduRxData->SduDataPtr[1]+1)/2;


		if (isRequestSeed) {
		    responseCode = DspUdsSecurityAccessGetSeedSubFnc(pduRxData, pduTxData, requestedSecurityLevel);
		}
		else {
		    responseCode = DspUdsSecurityAccessSendKeySubFnc(pduRxData, pduTxData, requestedSecurityLevel);
		}
	}
	else {
		responseCode = DCM_E_SUBFUNCTIONNOTSUPPORTED;
	}

	DsdDspProcessingDone(responseCode);
}


static boolean lookupRoutine(uint16 routineId, const Dcm_DspRoutineType **routinePtr)
{
	const Dcm_DspRoutineType *dspRoutine = Dcm_ConfigPtr->Dsp->DspRoutine;
	boolean routineFound = FALSE;

	while ((dspRoutine->DspRoutineIdentifier != routineId) &&  (!dspRoutine->Arc_EOL)) {
		dspRoutine++;
	}

	if (!dspRoutine->Arc_EOL) {
		routineFound = TRUE;
		*routinePtr = dspRoutine;
	}

	return routineFound;
}


static Dcm_NegativeResponseCodeType startRoutine(const Dcm_DspRoutineType *routinePtr, const PduInfoType *pduRxData, PduInfoType *pduTxData)
{
	Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;
	Std_ReturnType routineResult;

	// startRoutine
	if ((routinePtr->DspStartRoutineFnc != NULL) && (routinePtr->DspRoutineInfoRef->DspStartRoutine != NULL)) {
		if (((routinePtr->DspRoutineInfoRef->DspStartRoutine->DspStartRoutineCtrlOptRecSize + 4) <= pduRxData->SduLength)
			&& ((routinePtr->DspRoutineInfoRef->DspStartRoutine->DspStartRoutineStsOptRecSize + 4) <= pduTxData->SduLength)) {
			uint16 currentDataLength = pduRxData->SduLength - 4;
			routineResult = routinePtr->DspStartRoutineFnc(&pduRxData->SduDataPtr[4], DCM_INITIAL, &pduTxData->SduDataPtr[4], &currentDataLength, &responseCode, FALSE);	/** @req DCM400 */ /** @req DCM401 */
			pduTxData->SduLength = currentDataLength + 4;
			if (routineResult != E_OK && responseCode == DCM_E_POSITIVERESPONSE) {
				responseCode = DCM_E_CONDITIONSNOTCORRECT;
			}
			if(responseCode == DCM_E_POSITIVERESPONSE && pduTxData->SduLength > routinePtr->DspRoutineInfoRef->DspStartRoutine->DspStartRoutineStsOptRecSize + 4) {
			    DET_REPORTERROR(MODULE_ID_DCM, 0, DCM_GLOBAL_ID, DCM_E_PARAM);
			    pduTxData->SduLength = routinePtr->DspRoutineInfoRef->DspStartRoutine->DspStartRoutineStsOptRecSize + 4;
			}
		}
		else {
			responseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
		}
	}
	else {
		responseCode = DCM_E_REQUESTOUTOFRANGE;
	}
	return responseCode;
}


static Dcm_NegativeResponseCodeType stopRoutine(const Dcm_DspRoutineType *routinePtr, const PduInfoType *pduRxData, PduInfoType *pduTxData)
{
	Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;
	Std_ReturnType routineResult;

	// stopRoutine
	if ((routinePtr->DspStopRoutineFnc != NULL) && (routinePtr->DspRoutineInfoRef->DspRoutineStop != NULL)) {
		if (((routinePtr->DspRoutineInfoRef->DspRoutineStop->DspStopRoutineCtrlOptRecSize + 4) <= pduRxData->SduLength)
			&& ((routinePtr->DspRoutineInfoRef->DspRoutineStop->DspStopRoutineStsOptRecSize + 4) <= pduTxData->SduLength)) {
			uint16 currentDataLength = pduRxData->SduLength - 4;
			routineResult = routinePtr->DspStopRoutineFnc(&pduRxData->SduDataPtr[4], DCM_INITIAL, &pduTxData->SduDataPtr[4], &currentDataLength, &responseCode, FALSE);	/** @req DCM402 */ /** @req DCM403 */
			pduTxData->SduLength = currentDataLength + 4;
			if (routineResult != E_OK && responseCode == DCM_E_POSITIVERESPONSE) {
				responseCode = DCM_E_CONDITIONSNOTCORRECT;
			}
			if(responseCode == DCM_E_POSITIVERESPONSE && pduTxData->SduLength > routinePtr->DspRoutineInfoRef->DspRoutineStop->DspStopRoutineStsOptRecSize + 4) {
			    DET_REPORTERROR(MODULE_ID_DCM, 0, DCM_GLOBAL_ID, DCM_E_PARAM);
			    pduTxData->SduLength = routinePtr->DspRoutineInfoRef->DspRoutineStop->DspStopRoutineStsOptRecSize + 4;
			}
		}
		else {
			responseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
		}
	}
	else {
		responseCode = DCM_E_REQUESTOUTOFRANGE;
	}

    return responseCode;
}


static Dcm_NegativeResponseCodeType requestRoutineResults(const Dcm_DspRoutineType *routinePtr, PduInfoType *pduTxData)
{
	Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;
	Std_ReturnType routineResult;

	// requestRoutineResults
	if ((routinePtr->DspRequestResultRoutineFnc != NULL) && (routinePtr->DspRoutineInfoRef->DspRoutineRequestRes != NULL)) {
		if ((routinePtr->DspRoutineInfoRef->DspRoutineRequestRes->DspReqResRtnCtrlOptRecSize + 4) <= pduTxData->SduLength) {
			uint16 currentDataLength = 0;
			routineResult = routinePtr->DspRequestResultRoutineFnc(DCM_INITIAL, &pduTxData->SduDataPtr[4], &currentDataLength, &responseCode, FALSE);	/** @req DCM404 */ /** @req DCM405 */
			pduTxData->SduLength = currentDataLength + 4;
			if (routineResult != E_OK && responseCode == DCM_E_POSITIVERESPONSE) {
				responseCode = DCM_E_CONDITIONSNOTCORRECT;
			}
			if(responseCode == DCM_E_POSITIVERESPONSE && pduTxData->SduLength > routinePtr->DspRoutineInfoRef->DspRoutineRequestRes->DspReqResRtnCtrlOptRecSize + 4) {
			    DET_REPORTERROR(MODULE_ID_DCM, 0, DCM_GLOBAL_ID, DCM_E_PARAM);
			    pduTxData->SduLength = routinePtr->DspRoutineInfoRef->DspRoutineRequestRes->DspReqResRtnCtrlOptRecSize + 4;
			}
		}
		else {
			responseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
		}
	}
	else {
		responseCode = DCM_E_REQUESTOUTOFRANGE;
	}

    return responseCode;
}


void DspUdsRoutineControl(const PduInfoType *pduRxData, PduInfoType *pduTxData)
{
	/** @req DCM257 */
	Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;
	uint8 subFunctionNumber = 0;
	uint16 routineId = 0;
	const Dcm_DspRoutineType *routinePtr = NULL;

	if (pduRxData->SduLength >= 4) {
		subFunctionNumber = pduRxData->SduDataPtr[1];
		if ((subFunctionNumber > 0) && (subFunctionNumber < 4)) {
			routineId = (uint16)((uint16)pduRxData->SduDataPtr[2] << 8) + pduRxData->SduDataPtr[3];
			if (lookupRoutine(routineId, &routinePtr)) {
				if (DspCheckSessionLevel(routinePtr->DspRoutineInfoRef->DspRoutineAuthorization.DspRoutineSessionRef)) {
					if (DspCheckSecurityLevel(routinePtr->DspRoutineInfoRef->DspRoutineAuthorization.DspRoutineSecurityLevelRef)) {
						switch (subFunctionNumber) {
						case 0x01:	// startRoutine
							responseCode = startRoutine(routinePtr, pduRxData, pduTxData);
							break;

						case 0x02:	// stopRoutine
							responseCode = stopRoutine(routinePtr, pduRxData, pduTxData);
							break;

						case 0x03:	// requestRoutineResults
							responseCode =  requestRoutineResults(routinePtr, pduTxData);
							break;

						default:	// This shall never happen
							responseCode = DCM_E_SUBFUNCTIONNOTSUPPORTED;
							break;
						}
					}
					else {	// Not allowed in current security level
						responseCode = DCM_E_SECURITYACCESSDENIED;
					}
				}
				else {	// Not allowed in current session
					responseCode = DCM_E_SERVICENOTSUPPORTEDINACTIVESESSION;
				}
			}
			else {	// Unknown routine identifier
				responseCode = DCM_E_REQUESTOUTOFRANGE;
			}
		}
		else {	// Sub function not supported
			responseCode = DCM_E_SUBFUNCTIONNOTSUPPORTED;
		}
	}
	else {
		// Wrong length
		responseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
	}

	if (responseCode == DCM_E_POSITIVERESPONSE) {
		// Add header to the positive response message
		pduTxData->SduDataPtr[1] = subFunctionNumber;
		pduTxData->SduDataPtr[2] = (routineId >> 8) & 0xFFu;
		pduTxData->SduDataPtr[3] = routineId & 0xFFu;
	}

	DsdDspProcessingDone(responseCode);
}


void DspUdsTesterPresent(const PduInfoType *pduRxData, PduInfoType *pduTxData)
{
	/** @req DCM251 */
	if (pduRxData->SduLength == 2) {
		switch (pduRxData->SduDataPtr[1])
		{
		case ZERO_SUB_FUNCTION:
			DslResetSessionTimeoutTimer();
			// Create positive response
			pduTxData->SduDataPtr[1] = ZERO_SUB_FUNCTION;
			pduTxData->SduLength = 2;
			DsdDspProcessingDone(DCM_E_POSITIVERESPONSE);
			break;

		default:
			DsdDspProcessingDone(DCM_E_SUBFUNCTIONNOTSUPPORTED);
			break;
		}
	}
	else {
		// Wrong length
		DsdDspProcessingDone(DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT);
	}
}

#if defined(USE_DEM) && defined(DCM_USE_SERVICE_CONTROLDTCSETTING)
void DspUdsControlDtcSetting(const PduInfoType *pduRxData, PduInfoType *pduTxData)
{
	/** @req DCM249 */
	Dem_ReturnControlDTCStorageType resultCode;

	if (pduRxData->SduLength == 2) {
		switch (pduRxData->SduDataPtr[1])
		{
		case 0x01:	// ON
			resultCode = Dem_EnableDTCSetting(DEM_DTC_GROUP_ALL_DTCS, DEM_DTC_KIND_ALL_DTCS);		/** @req DCM304 */
			if (resultCode == DEM_CONTROL_DTC_STORAGE_OK) {
				pduTxData->SduDataPtr[1] = 0x01;
				pduTxData->SduLength = 2;
				DsdDspProcessingDone(DCM_E_POSITIVERESPONSE);
			}
			else {
				DsdDspProcessingDone(DCM_E_REQUESTOUTOFRANGE);
			}
			break;

		case 0x02:	// OFF
			resultCode = Dem_DisableDTCSetting(DEM_DTC_GROUP_ALL_DTCS, DEM_DTC_KIND_ALL_DTCS);		/** @req DCM406 */
			if (resultCode == DEM_CONTROL_DTC_STORAGE_OK) {
				pduTxData->SduDataPtr[1] = 0x02;
				pduTxData->SduLength = 2;
				DsdDspProcessingDone(DCM_E_POSITIVERESPONSE);
			}
			else {
				DsdDspProcessingDone(DCM_E_REQUESTOUTOFRANGE);
			}
			break;

		default:
			DsdDspProcessingDone(DCM_E_SUBFUNCTIONNOTSUPPORTED);
			break;
		}
	}
	else {
		// Wrong length
		DsdDspProcessingDone(DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT);
	}
}
#endif

void DspDcmConfirmation(PduIdType confirmPduId)
{
	DslResetSessionTimeoutTimer(); /** @req DCM141 */
	if ( DCM_DSP_RESET_WAIT_TX_CONF == dspUdsEcuResetData.resetPending ) {
		if (confirmPduId == dspUdsEcuResetData.resetPduId) {
			dspUdsEcuResetData.resetPending = DCM_DSP_RESET_NO_RESET;
			DcmE_EcuPerformReset(dspUdsEcuResetData.resetType);
			if(DCM_HARD_RESET == dspUdsEcuResetData.resetType) {
#if defined(USE_MCU) && ( MCU_PERFORM_RESET_API == STD_ON )
				Mcu_PerformReset();
#else
				DET_REPORTERROR(MODULE_ID_DCM, 0, DCM_UDS_RESET_ID, DCM_E_NOT_SUPPORTED);
#endif
			}
		}
	}

	if (dspUdsSessionControlData.sessionPending) {
		if (confirmPduId == dspUdsSessionControlData.sessionPduId) {
			dspUdsSessionControlData.sessionPending = FALSE;
			Dcm_DiagnosticSessionControl(dspUdsSessionControlData.session);
		}
	}
}

static Dcm_NegativeResponseCodeType readMemoryData( Dcm_OpStatusType *OpStatus,
													uint8 memoryIdentifier,
													uint32 MemoryAddress,
													uint32 MemorySize,
													PduInfoType *pduTxData)
{
	Dcm_ReturnReadMemoryType ReadRet;
	Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;
	*OpStatus = DCM_INITIAL;
	ReadRet = Dcm_ReadMemory(*OpStatus,memoryIdentifier,
									MemoryAddress,
									MemorySize,
									&pduTxData->SduDataPtr[1]);
	if(DCM_READ_FAILED == ReadRet)
	{
		responseCode = DCM_E_GENERALPROGRAMMINGFAILURE;  /*@req Dcm644*/
	}
	if (DCM_READ_PENDING == ReadRet)
	{
		*OpStatus = DCM_READ_PENDING;
	}	
	return responseCode;
}

static Dcm_NegativeResponseCodeType checkAddressRange(DspMemoryServiceType serviceType, uint8 memoryIdentifier, uint32 memoryAddress, uint32 length) {
	const Dcm_DspMemoryIdInfo *dspMemoryInfo = Dcm_ConfigPtr->Dsp->DspMemory->DspMemoryIdInfo;
	const Dcm_DspMemoryRangeInfo *memoryRangeInfo = NULL;
	Dcm_NegativeResponseCodeType diagResponseCode = DCM_E_REQUESTOUTOFRANGE;

	for( ; (dspMemoryInfo->Arc_EOL == FALSE) && (memoryRangeInfo == NULL); dspMemoryInfo++ )
	{
		if( ((TRUE == Dcm_ConfigPtr->Dsp->DspMemory->DcmDspUseMemoryId) && (dspMemoryInfo->MemoryIdValue == memoryIdentifier))
			|| (FALSE == Dcm_ConfigPtr->Dsp->DspMemory->DcmDspUseMemoryId) )
		{
			if( DCM_READ_MEMORY == serviceType )
			{
				memoryRangeInfo = findRange( dspMemoryInfo->pReadMemoryInfo, memoryAddress, length );
			}
			else
			{
				memoryRangeInfo = findRange( dspMemoryInfo->pWriteMemoryInfo, memoryAddress, length );
			}

			if( NULL != memoryRangeInfo )
			{
			    if( memoryRangeInfo->pSecurityLevel[0]->Arc_EOL || DspCheckSecurityLevel(memoryRangeInfo->pSecurityLevel))
				{
					/* Range is ok */
					diagResponseCode = DCM_E_POSITIVERESPONSE;
				}
				else
				{
					diagResponseCode = DCM_E_SECURITYACCESSDENIED;
				}
			}
			else {
				/* Range was not configured for read/write */
				diagResponseCode = DCM_E_REQUESTOUTOFRANGE;
			}
		}
		else {
			/* No memory with this id found */
			diagResponseCode = DCM_E_REQUESTOUTOFRANGE;
		}
	}
	return diagResponseCode;
}

static const Dcm_DspMemoryRangeInfo* findRange(const Dcm_DspMemoryRangeInfo *memoryRangePtr, uint32 memoryAddress, uint32 length)
{
	const Dcm_DspMemoryRangeInfo *memoryRange = NULL;

	for( ; (memoryRangePtr->Arc_EOL == FALSE) && (memoryRange == NULL); memoryRangePtr++ )
	{
		/*@req DCM493*/
		if((memoryAddress >= memoryRangePtr->MemoryAddressLow)
			&& (memoryAddress <= memoryRangePtr->MemoryAddressHigh)
			&& (memoryAddress + length - 1 <= memoryRangePtr->MemoryAddressHigh))
		{
			memoryRange = memoryRangePtr;
		}
	}

	return memoryRange;
}

void DspUdsWriteMemoryByAddress(const PduInfoType *pduRxData, PduInfoType *pduTxData)
{
	Dcm_NegativeResponseCodeType diagResponseCode;
	uint8 sizeFormat = 0;
	uint8 addressFormat = 0;
	uint32 memoryAddress = 0;
	uint32 length = 0;
	uint8 i = 0;
	uint8 memoryIdentifier = 0; /* Should be 0 if DcmDspUseMemoryId == FALSE */
	Dcm_OpStatusType OpStatus = DCM_INITIAL;
	uint8 addressOffset;

	if( pduRxData->SduLength > ALFID_INDEX )
	{
		sizeFormat = ((uint8)(pduRxData->SduDataPtr[ALFID_INDEX] & DCM_FORMAT_HIGH_MASK)) >> 4;	/*@req UDS_REQ_0x23_1 & UDS_REQ_0x23_5*/;
		addressFormat = ((uint8)(pduRxData->SduDataPtr[ALFID_INDEX])) & DCM_FORMAT_LOW_MASK;   /*@req UDS_REQ_0x23_1 & UDS_REQ_0x23_5*/;
		if((addressFormat != 0) && (sizeFormat != 0))
		{
			if(addressFormat + sizeFormat + SID_LEN + ALFID_LEN <= pduRxData->SduLength)
			{
				if( TRUE == Dcm_ConfigPtr->Dsp->DspMemory->DcmDspUseMemoryId ) {
					memoryIdentifier = pduRxData->SduDataPtr[ADDR_START_INDEX];
					addressOffset = 1;
				}
				else {
					addressOffset = 0;
				}

				/* Parse address */
				for(i = addressOffset; i < addressFormat; i++)
				{
					memoryAddress <<= 8;
					memoryAddress += (uint32)(pduRxData->SduDataPtr[ADDR_START_INDEX + i]);
				}

				/* Parse size */
				for(i = 0; i < sizeFormat; i++)
				{
					length <<= 8;
					length += (uint32)(pduRxData->SduDataPtr[ADDR_START_INDEX + addressFormat + i]);
				}

				if( addressFormat + sizeFormat + SID_LEN + ALFID_LEN + length == pduRxData->SduLength )
				{

					diagResponseCode = checkAddressRange(DCM_WRITE_MEMORY, memoryIdentifier, memoryAddress, length);
					if( DCM_E_POSITIVERESPONSE == diagResponseCode )
					{
						diagResponseCode = writeMemoryData(&OpStatus, memoryIdentifier, memoryAddress, length,
													&pduRxData->SduDataPtr[SID_LEN + ALFID_LEN + addressFormat + sizeFormat]);
					}

				}
				else
				{
					diagResponseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
				}
			}
			else
			{
				diagResponseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
			}
		}
		else
		{
			diagResponseCode = DCM_E_REQUESTOUTOFRANGE;  /*UDS_REQ_0x23_10*/
		}
	}
	else
	{
		diagResponseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
	}

	if(DCM_E_POSITIVERESPONSE == diagResponseCode)
	{
		pduTxData->SduLength = SID_LEN + ALFID_LEN + addressFormat + sizeFormat;
		pduTxData->SduDataPtr[ALFID_INDEX] = pduRxData->SduDataPtr[ALFID_INDEX];
		for(i = 0; i < addressFormat + sizeFormat; i++)
		{
			pduTxData->SduDataPtr[ADDR_START_INDEX + i] = pduRxData->SduDataPtr[ADDR_START_INDEX + i];
			if(OpStatus != DCM_WRITE_PENDING)
			{
				DsdDspProcessingDone(diagResponseCode);
			}
			else
			{
        		dspMemoryState=DCM_MEMORY_WRITE;
			}
		}
	}
	else
	{
		DsdDspProcessingDone(diagResponseCode);
	}
}

/*@req Dcm442,DCM492*/
void DspUdsReadMemoryByAddress(const PduInfoType *pduRxData, PduInfoType *pduTxData)
{
	Dcm_NegativeResponseCodeType diagResponseCode;
	uint8 sizeFormat;
	uint8 addressFormat;
	uint32 memoryAddress = 0;
	uint32 length = 0;
	uint8 i;
	uint8 memoryIdentifier = 0; /* Should be 0 if DcmDspUseMemoryId == FALSE */
	Dcm_OpStatusType OpStatus = DCM_INITIAL;
	uint8 addressOffset;

	if( pduRxData->SduLength > ALFID_INDEX )
	{
		sizeFormat = ((uint8)(pduRxData->SduDataPtr[ALFID_INDEX] & DCM_FORMAT_HIGH_MASK)) >> 4;	/*@req UDS_REQ_0x23_1 & UDS_REQ_0x23_5*/;
		addressFormat = ((uint8)(pduRxData->SduDataPtr[ALFID_INDEX])) & DCM_FORMAT_LOW_MASK;   /*@req UDS_REQ_0x23_1 & UDS_REQ_0x23_5*/;
		if((addressFormat != 0) && (sizeFormat != 0))
		{
			if(addressFormat + sizeFormat + SID_LEN + ALFID_LEN == pduRxData->SduLength)
			{
				if( TRUE == Dcm_ConfigPtr->Dsp->DspMemory->DcmDspUseMemoryId ) {
					memoryIdentifier = pduRxData->SduDataPtr[ADDR_START_INDEX];
					addressOffset = 1;
				}
				else {
					addressOffset = 0;
				}

				/* Parse address */
				for(i = addressOffset; i < addressFormat; i++)
				{
					memoryAddress <<= 8;
					memoryAddress += (uint32)(pduRxData->SduDataPtr[ADDR_START_INDEX + i]);
				}

				/* Parse size */
				for(i = 0; i < sizeFormat; i++)
				{
					length <<= 8;
					length += (uint32)(pduRxData->SduDataPtr[ADDR_START_INDEX + addressFormat + i]);
				}

				if(length <= (DCM_PROTOCAL_TP_MAX_LENGTH - SID_LEN) )
				{
					diagResponseCode = checkAddressRange(DCM_READ_MEMORY, memoryIdentifier, memoryAddress, length);
					if( DCM_E_POSITIVERESPONSE == diagResponseCode )
					{
						diagResponseCode = readMemoryData(&OpStatus, memoryIdentifier, memoryAddress, length, pduTxData);
					}
				}
				else {
					diagResponseCode = DCM_E_REQUESTOUTOFRANGE;
				}
			}
			else
			{
				diagResponseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
			}
		}
		else
		{
			diagResponseCode = DCM_E_REQUESTOUTOFRANGE;  /*UDS_REQ_0x23_10*/
		}
	}
	else
	{
		diagResponseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
	}

	if(DCM_E_POSITIVERESPONSE == diagResponseCode)
	{
		pduTxData->SduLength = SID_LEN + length;
		if(OpStatus == DCM_READ_PENDING)
		{
			dspMemoryState = DCM_MEMORY_READ;
		}
		else
		{
			DsdDspProcessingDone(DCM_E_POSITIVERESPONSE);
		}
	}
	else
	{
		DsdDspProcessingDone(diagResponseCode);
	}
}

static Dcm_NegativeResponseCodeType writeMemoryData(Dcm_OpStatusType* OpStatus,
												uint8 memoryIdentifier,
												uint32 MemoryAddress,
												uint32 MemorySize,
												uint8 *SourceData)
{
	Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;
	Dcm_ReturnWriteMemoryType writeRet;
	*OpStatus = DCM_INITIAL;
	writeRet = Dcm_WriteMemory(*OpStatus,
								memoryIdentifier,
								MemoryAddress,
								MemorySize,
								SourceData);
	if(DCM_WRITE_FAILED == writeRet)
	{
		responseCode = DCM_E_GENERALPROGRAMMINGFAILURE;   /*@req UDS_REQ_0X3D_16,DCM643*/
	}
	else if(DCM_WRITE_PENDING == writeRet)
	{
		*OpStatus = DCM_PENDING;
	}
	else
	{
		responseCode = DCM_E_POSITIVERESPONSE;
	}
	
	return responseCode;
}

static boolean checkPeriodicIdentifierBuffer(uint8 PeriodicDid,uint8 Length,uint8 *postion)
{
	uint8 i;
	boolean ret = FALSE;
	for(i = 0;(i < Length) && (ret == FALSE);i++)
	{
		if(PeriodicDid == dspPDidRef.dspPDid[i].PeriodicDid)
		{
			ret = TRUE;
			*postion = i;
		}
	}
	
	return ret;
}

static void ClearPeriodicIdentifierBuffer(uint8 BufferEnd,uint8 postion)
{
	dspPDidRef.dspPDid[postion].PeriodicDid = dspPDidRef.dspPDid[BufferEnd ].PeriodicDid;
	dspPDidRef.dspPDid[postion].PDidTxCounter = dspPDidRef.dspPDid[BufferEnd].PDidTxCounter;
	dspPDidRef.dspPDid[postion].PDidTxCounterLimit = dspPDidRef.dspPDid[BufferEnd].PDidTxCounterLimit;
	dspPDidRef.dspPDid[BufferEnd].PeriodicDid = 0;
	dspPDidRef.dspPDid[BufferEnd].PDidTxCounter = 0;
	dspPDidRef.dspPDid[BufferEnd ].PDidTxCounterLimit = 0;
}

static Dcm_NegativeResponseCodeType readPeriodDidData(const Dcm_DspDidType *PDidPtr, uint8 *Data,uint16 *Length)
{
	Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;
	if ((PDidPtr->DspDidInfoRef->DspDidAccess.DspDidRead != NULL) 
		&& (PDidPtr->DspDidConditionCheckReadFnc != NULL) 
		&& (PDidPtr->DspDidReadDataFnc.SynchDataReadFnc != NULL) )
	{	
		if (DspCheckSessionLevel(PDidPtr->DspDidInfoRef->DspDidAccess.DspDidRead->DspDidReadSessionRef)) 
		{ 
			if (DspCheckSecurityLevel(PDidPtr->DspDidInfoRef->DspDidAccess.DspDidRead->DspDidReadSecurityLevelRef)) 
			{
				Std_ReturnType result = E_NOT_OK;
				Dcm_NegativeResponseCodeType errorCode = DCM_E_POSITIVERESPONSE;
				result = PDidPtr->DspDidConditionCheckReadFnc(DCM_INITIAL, &errorCode);
				if( (E_PENDING == result) && (DATA_PORT_ASYNCH != PDidPtr->DspDidUsePort)) {
				    /* Synch port cannot return E_PENDING */
				    DET_REPORTERROR(MODULE_ID_DCM, 0, DCM_GLOBAL_ID, DCM_E_UNEXPECTED_RESPONSE);
				    result = E_NOT_OK;
				}
				if ((result == E_OK) && (errorCode == DCM_E_POSITIVERESPONSE))
				{
					result = E_NOT_OK;
					if (PDidPtr->DspDidInfoRef->DspDidFixedLength)
					{	
						*Length= PDidPtr->DspDidSize;
						result = E_OK;
					}
					else
					{
						if(PDidPtr->DspDidReadDataLengthFnc!=NULL)
						{
						    result = PDidPtr->DspDidReadDataLengthFnc(Length);
							if( E_OK != result ) {
							    /* Read function is only allowed to return E_OK */
							    DET_REPORTERROR(MODULE_ID_DCM, 0, DCM_GLOBAL_ID, DCM_E_UNEXPECTED_RESPONSE);
							    result = E_NOT_OK;
							}
						}
						else
						{
							responseCode = DCM_E_GENERALREJECT;
						}
					}
					if (result == E_OK) 
					{
					    if( DATA_PORT_SYNCH == PDidPtr->DspDidUsePort ) {
					        result = PDidPtr->DspDidReadDataFnc.SynchDataReadFnc(Data);
					        if( E_OK != result ) {
					            /* Synch port cannot return E_PENDING */
					            DET_REPORTERROR(MODULE_ID_DCM, 0, DCM_GLOBAL_ID, DCM_E_UNEXPECTED_RESPONSE);
					            result = E_NOT_OK;
					        }
					    } else if(DATA_PORT_ASYNCH == PDidPtr->DspDidUsePort) {
					        result = PDidPtr->DspDidReadDataFnc.AsynchDataReadFnc(DCM_INITIAL, Data);
					    } else {
					        /* Port not supported */
					        DET_REPORTERROR(MODULE_ID_DCM, 0, DCM_GLOBAL_ID, DCM_E_CONFIG_INVALID);
					        result = E_NOT_OK;
					    }
						if (result != E_OK)
						{
							responseCode = DCM_E_REQUESTOUTOFRANGE;
						}
					}
					else
					{
						responseCode = DCM_E_REQUESTOUTOFRANGE;
					}
				}
				else
				{
					responseCode = DCM_E_REQUESTOUTOFRANGE;
				}
			}
			else
			{
				responseCode = DCM_E_SECURITYACCESSDENIED;
			}
		}
		else
		{
			responseCode = DCM_E_REQUESTOUTOFRANGE;
		}
	}
	else
	{
		responseCode = DCM_E_REQUESTOUTOFRANGE;
	}
	return responseCode;
}

static Dcm_NegativeResponseCodeType DspSavePeriodicData(uint16 didNr, uint32 periodicTransmitCounter,uint8 PdidBufferNr)
{
    Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;
    const Dcm_DspDidType *SourceDid = NULL;
    Dcm_DspDDDType *DDidPtr = NULL;

    if (TRUE == lookupDid(didNr, &SourceDid))
    {
        if(DspCheckSessionLevel(SourceDid->DspDidInfoRef->DspDidAccess.DspDidRead->DspDidReadSessionRef) == TRUE)
        {
            if(DspCheckSecurityLevel(SourceDid->DspDidInfoRef->DspDidAccess.DspDidRead->DspDidReadSecurityLevelRef) == TRUE)
            {
                Std_ReturnType result = E_NOT_OK;
                Dcm_NegativeResponseCodeType errorCode = DCM_E_POSITIVERESPONSE;
                
                if(SourceDid->DspDidConditionCheckReadFnc != NULL)
                {
                    result = SourceDid->DspDidConditionCheckReadFnc(DCM_INITIAL, &errorCode);
                    if( (E_PENDING == result) && (DATA_PORT_ASYNCH != SourceDid->DspDidUsePort)) {
                        /* Synch port cannot return E_PENDING */
                        DET_REPORTERROR(MODULE_ID_DCM, 0, DCM_GLOBAL_ID, DCM_E_UNEXPECTED_RESPONSE);
                        result = E_NOT_OK;
                    }
                }
                if ((result != E_OK) || (errorCode != DCM_E_POSITIVERESPONSE))
                {
                    responseCode = DCM_E_REQUESTOUTOFRANGE;
                }
                if((SourceDid->DspDidInfoRef->DspDidFixedLength!=TRUE) && (SourceDid->DspDidReadDataLengthFnc == NULL))
                {
                    responseCode = DCM_E_GENERALREJECT;
                }
            }
            else
            {
                responseCode = DCM_E_SECURITYACCESSDENIED;
            }
        }
        else
        {
            responseCode = DCM_E_REQUESTOUTOFRANGE;
        }
    }
    else if(LookupDDD(didNr, (const Dcm_DspDDDType **)&DDidPtr) == TRUE)
    {
        responseCode = DCM_E_POSITIVERESPONSE;
    }
    else
    {
        responseCode = DCM_E_REQUESTOUTOFRANGE;
    }
    if(responseCode == DCM_E_POSITIVERESPONSE)
    {
        dspPDidRef.dspPDid[PdidBufferNr].PeriodicDid = (uint8)didNr & DCM_DID_LOW_MASK;
        dspPDidRef.dspPDid[PdidBufferNr].PDidTxCounter = 0;
        dspPDidRef.dspPDid[PdidBufferNr].PDidTxCounterLimit = periodicTransmitCounter;
    }
    return responseCode;
}
static void ClearPeriodicIdentifier(const PduInfoType *pduRxData,PduInfoType *pduTxData )
{
	uint16 PdidNumber;
	uint8 PDidLowByte;
	uint8 PdidPostion;
	uint8 i;
	if( pduRxData->SduDataPtr[1] == DCM_PERIODICTRANSMIT_STOPSENDING_MODE )
	{
		PdidNumber = pduRxData->SduLength - 2;
		for(i = 0;i < PdidNumber;i++)
		{
			PDidLowByte = pduRxData->SduDataPtr[2 + i];
			if(checkPeriodicIdentifierBuffer(PDidLowByte,dspPDidRef.PDidNr,&PdidPostion) == TRUE)
			{
				dspPDidRef.PDidNr--;
				ClearPeriodicIdentifierBuffer(dspPDidRef.PDidNr, PdidPostion);
				
			}		
		}
		pduTxData->SduLength = 1;
	}
}
/*
	DESCRIPTION:
		 UDS Service 0x2a - Read Data By Periodic Identifier
*/
void DspReadDataByPeriodicIdentifier(const PduInfoType *pduRxData,PduInfoType *pduTxData)
{
    /** @req DCM254 */
    uint8 PDidLowByte;
    uint16 PdidNumber;
    uint8 PdidPostion;
    uint16 i;
    uint8 PdidBufferNr;
    uint32 periodicTransmitCounter = 0;
    uint16 DataLength;
    Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;
    const Dcm_DspDidType *PDidPtr = NULL;
    Dcm_DspDDDType *DDidPtr = NULL;
    PdidBufferNr = dspPDidRef.PDidNr;
    if(pduRxData->SduLength > 2)
    {

        switch(pduRxData->SduDataPtr[1])
        {
            case DCM_PERIODICTRANSMIT_DEFAULT_MODE:
                periodicTransmitCounter = 0;
                break;
            case DCM_PERIODICTRANSMIT_SLOWRATE_MODE:
                periodicTransmitCounter = DCM_PERIODICTRANSMIT_SLOW;
                break;
                case DCM_PERIODICTRANSMIT_MEDIUM_MODE:
                    periodicTransmitCounter = DCM_PERIODICTRANSMIT_MEDIUM;
                break;
            case DCM_PERIODICTRANSMIT_FAST_MODE:
                periodicTransmitCounter = DCM_PERIODICTRANSMIT_FAST;
                break;
            case DCM_PERIODICTRANSMIT_STOPSENDING_MODE:
                ClearPeriodicIdentifier(pduRxData,pduTxData);
                break;
            default:
                responseCode = DCM_E_REQUESTOUTOFRANGE;
                break;
        }
        if((pduRxData->SduDataPtr[1] != DCM_PERIODICTRANSMIT_STOPSENDING_MODE) && responseCode == DCM_E_POSITIVERESPONSE)
        {
            PdidNumber = pduRxData->SduLength - 2;
            if(1 == PdidNumber)
            {
                PDidLowByte = pduRxData->SduDataPtr[2];
                if(checkPeriodicIdentifierBuffer(PDidLowByte,dspPDidRef.PDidNr,&PdidPostion) == TRUE)
                {
                    if(0 == periodicTransmitCounter)
                    {
                        if (TRUE == lookupDid(((uint16)PDidLowByte + 0xF200), &PDidPtr))   /*UDS_REQ_0x2A_1*/
                        {
                            pduTxData->SduDataPtr[1] = PDidLowByte;
                            responseCode = readPeriodDidData(PDidPtr,&pduTxData->SduDataPtr[2],&DataLength);
                            pduTxData->SduLength = DataLength + 2;
                        }
                        else if(TRUE == LookupDDD((0xF200 + (uint16)PDidLowByte), (const Dcm_DspDDDType **)&DDidPtr))
                        {
                            pduTxData->SduDataPtr[1] = PDidLowByte;
                            responseCode = readDDDData(DDidPtr,&pduTxData->SduDataPtr[2],&DataLength);
                            pduTxData->SduLength = DataLength + 2;
                        }
                        else
                        {
                            responseCode = DCM_E_REQUESTOUTOFRANGE;
                        }
                        if(responseCode != DCM_E_POSITIVERESPONSE)
                        {
                            dspPDidRef.PDidNr--;
                            ClearPeriodicIdentifierBuffer(dspPDidRef.PDidNr,PdidPostion);
                        }
                    }
                    else
                    {
                        dspPDidRef.dspPDid[PdidPostion].PDidTxCounterLimit = periodicTransmitCounter;
                        pduTxData->SduLength = 1;
                    }
                }
                else
                {
                    responseCode = DspSavePeriodicData((DCM_PERODICDID_HIGH_MASK + (uint16)PDidLowByte),periodicTransmitCounter,PdidBufferNr);
                    PdidBufferNr++;
                    pduTxData->SduLength = 1;
                }
            }
            else if(((PdidNumber + PdidBufferNr) <= DCM_LIMITNUMBER_PERIODDATA) && (responseCode == DCM_E_POSITIVERESPONSE))	/*UDS_REQ_0x2A_6*/
            {
                for(i = 0;(i < PdidNumber)&&(responseCode == DCM_E_POSITIVERESPONSE);i++)
                {
                    PDidLowByte = pduRxData->SduDataPtr[2 + i];
                    if(checkPeriodicIdentifierBuffer(PDidLowByte,PdidBufferNr,&PdidPostion) == TRUE)
                    {
                        if(dspPDidRef.dspPDid[PdidPostion].PDidTxCounterLimit != periodicTransmitCounter)
                        {
                            dspPDidRef.dspPDid[PdidPostion].PDidTxCounterLimit = periodicTransmitCounter;
                        }
                    }
                    else
                    {
                        responseCode = DspSavePeriodicData((0xF200 + (uint16)PDidLowByte),periodicTransmitCounter,PdidBufferNr);
                        PdidBufferNr++;
                    }
                    pduTxData->SduLength = 1;
                }
            }
            else
            {
                responseCode = DCM_E_REQUESTOUTOFRANGE;
            }
            if(responseCode == DCM_E_POSITIVERESPONSE)
            {
                dspPDidRef.PDidNr = PdidBufferNr;
            }
        }
    }
    else if((pduRxData->SduLength == 2)&&(pduRxData->SduDataPtr[1] == DCM_PERIODICTRANSMIT_STOPSENDING_MODE))
    {
        memset(&dspPDidRef,0,sizeof(dspPDidRef));
        pduTxData->SduLength = 1;
    }
    else
    {
        responseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
    }
    DsdDspProcessingDone(responseCode);
}

#ifdef DCM_USE_SERVICE_DYNAMICALLYDEFINEDATAIDENTIFIER
static Dcm_NegativeResponseCodeType dynamicallyDefineDataIdentifierbyDid(uint16 DDIdentifier,const PduInfoType *pduRxData,PduInfoType *pduTxData)
{
	uint8 i;
	uint16 SourceDidNr;
	const Dcm_DspDidType *SourceDid = NULL;
	Dcm_DspDDDType *DDid = NULL;
	uint16 SourceLength = 0;
	uint16 DidLength = 0;
	uint16 Length = 0;
	uint8 Num = 0;
	Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;

	if(FALSE == LookupDDD(DDIdentifier, (const Dcm_DspDDDType **)&DDid))
	{
		while((Num < DCM_MAX_DDD_NUMBER) && (dspDDD[Num].DynamicallyDid != 0 ))
		{
			Num++;
		}
		if(Num >= DCM_MAX_DDD_NUMBER)
		{
			responseCode = DCM_E_REQUESTOUTOFRANGE;
		}
		else
		{
			DDid = &dspDDD[Num];
		}
	}
	else
	{
		while((SourceLength < DCM_MAX_DDDSOURCE_NUMBER) && (DDid->DDDSource[SourceLength].formatOrPosition != 0 ))
		{
			SourceLength++;
		}
		
	}
	if(responseCode == DCM_E_POSITIVERESPONSE)
	{
		Length = (pduRxData->SduLength - SID_AND_DDDID_LEN) /SDI_AND_MS_LEN;
		if(((Length*SDI_AND_MS_LEN) == (pduRxData->SduLength - SID_AND_DDDID_LEN)) && (Length != 0))
		{
			if((Length + SourceLength) <= DCM_MAX_DDDSOURCE_NUMBER)
			{
				for(i = 0;(i < Length) && (responseCode == DCM_E_POSITIVERESPONSE);i++)
				{
					SourceDidNr = (((uint16)pduRxData->SduDataPtr[SID_AND_DDDID_LEN + i*SDI_AND_MS_LEN] << 8) & DCM_DID_HIGH_MASK) + (((uint16)pduRxData->SduDataPtr[(5 + i*SDI_AND_MS_LEN)]) & DCM_DID_LOW_MASK);
					if(TRUE == lookupDid(SourceDidNr, &SourceDid))/*UDS_REQ_0x2C_4*/
					{	
						if(DspCheckSessionLevel(SourceDid->DspDidInfoRef->DspDidAccess.DspDidRead->DspDidReadSessionRef))
						{
							if(DspCheckSecurityLevel(SourceDid->DspDidInfoRef->DspDidAccess.DspDidRead->DspDidReadSecurityLevelRef))
							{
								if(SourceDid->DspDidInfoRef->DspDidFixedLength == TRUE)
								{
									DidLength = SourceDid->DspDidSize;
								}
								else
								{
									if(	SourceDid->DspDidReadDataLengthFnc != NULL)
									{
										(void)SourceDid->DspDidReadDataLengthFnc(&DidLength);
									}
									else
									{
										DidLength = 0;	
									}
								}
								if(DidLength != 0)
								{
									if((pduRxData->SduDataPtr[SID_AND_SDI_LEN + i*SDI_AND_MS_LEN] != 0) &&
										(pduRxData->SduDataPtr[SID_AND_PISDR_LEN + i*SDI_AND_MS_LEN] != 0) &&
										(((uint16)pduRxData->SduDataPtr[SID_AND_SDI_LEN + i*SDI_AND_MS_LEN] + (uint16)pduRxData->SduDataPtr[SID_AND_PISDR_LEN + i*SID_AND_DDDID_LEN] - 1) <= DidLength))
									{
										DDid->DDDSource[i + SourceLength].formatOrPosition = pduRxData->SduDataPtr[SID_AND_SDI_LEN + i*SDI_AND_MS_LEN];
										DDid->DDDSource[i + SourceLength].Size = pduRxData->SduDataPtr[SID_AND_PISDR_LEN + i*SDI_AND_MS_LEN];
										DDid->DDDSource[i + SourceLength].SourceAddressOrDid = SourceDid->DspDidIdentifier;
										DDid->DDDSource[i + SourceLength].DDDTpyeID = DCM_DDD_SOURCE_DID;
									}
									else
									{
										/*UDS_REQ_0x2C_6*/
										responseCode = DCM_E_REQUESTOUTOFRANGE;
									}
									
								}
								else
								{
									/*UDS_REQ_0x2C_14*/
									responseCode = DCM_E_REQUESTOUTOFRANGE;
								}
							}
							else
							{
								responseCode = DCM_E_SECURITYACCESSDENIED;
							}
						}
						else
						{
							/*UDS_REQ_0x2C_19,DCM726*/
							responseCode = DCM_E_REQUESTOUTOFRANGE;
						}
					}
					else
					{
						/*DCM725*/
						responseCode = DCM_E_REQUESTOUTOFRANGE;
					}
				}
			}
			else
			{
				/*UDS_REQ_0x2C_13*/
				responseCode = DCM_E_REQUESTOUTOFRANGE;
			}
		}
		else
		{
			/*UDS_REQ_0x2C_11*/
			responseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
		}
		if(responseCode == DCM_E_POSITIVERESPONSE)
		{
			DDid->DynamicallyDid = DDIdentifier;
			pduTxData->SduDataPtr[1] = DCM_DDD_SUBFUNCTION_DEFINEBYDID;
		}
	}
	if(responseCode == DCM_E_POSITIVERESPONSE)
	{
		pduTxData->SduDataPtr[1] = DCM_DDD_SUBFUNCTION_DEFINEBYDID;
	}
	
	return responseCode;
}

static Dcm_NegativeResponseCodeType dynamicallyDefineDataIdentifierbyAddress(uint16 DDIdentifier,const PduInfoType *pduRxData,PduInfoType *pduTxData)
{
	uint16 numNewDefinitions;
	uint16 numEarlierDefinitions = 0;
	Dcm_DspDDDType *DDid = NULL;
	uint8 Num = 0;
	uint8 definitionIndex;
	Dcm_NegativeResponseCodeType diagResponseCode = DCM_E_POSITIVERESPONSE;
	uint8 sizeFormat;
	uint8 addressFormat;
	uint32 memoryAddress = 0;
	uint32 length = 0;
	uint8 i;
	uint8 memoryIdentifier = 0; /* Should be 0 if DcmDspUseMemoryId == FALSE */
	uint8 addressOffset;
	
	if(FALSE == LookupDDD(DDIdentifier, (const Dcm_DspDDDType **)&DDid))
	{
		while((Num < DCM_MAX_DDD_NUMBER) && (dspDDD[Num].DynamicallyDid != 0 ))
		{
			Num++;
		}
		if(Num >= DCM_MAX_DDD_NUMBER)
		{
			diagResponseCode = DCM_E_REQUESTOUTOFRANGE;
		}
		else
		{
			DDid = &dspDDD[Num];
		}
	}
	else
	{
		while((numEarlierDefinitions < DCM_MAX_DDDSOURCE_NUMBER) && (DDid->DDDSource[numEarlierDefinitions].formatOrPosition != 0 ))
		{
			numEarlierDefinitions++;
		}
		if(numEarlierDefinitions >= DCM_MAX_DDDSOURCE_NUMBER)
		{
			diagResponseCode = DCM_E_REQUESTOUTOFRANGE;
		}
	}

	if( diagResponseCode == DCM_E_POSITIVERESPONSE )
	{
		if( pduRxData->SduLength > DYNDEF_ALFID_INDEX )
		{
			sizeFormat = ((uint8)(pduRxData->SduDataPtr[DYNDEF_ALFID_INDEX] & DCM_FORMAT_HIGH_MASK)) >> 4;	/*@req UDS_REQ_0x23_1 & UDS_REQ_0x23_5*/;
			addressFormat = ((uint8)(pduRxData->SduDataPtr[DYNDEF_ALFID_INDEX])) & DCM_FORMAT_LOW_MASK;   /*@req UDS_REQ_0x23_1 & UDS_REQ_0x23_5*/;
			if((addressFormat != 0) && (sizeFormat != 0))
			{
				numNewDefinitions = (pduRxData->SduLength - (SID_LEN + SF_LEN + DDDDI_LEN + ALFID_LEN) ) / (sizeFormat + addressFormat);
				if( (numNewDefinitions != 0) &&
					((SID_LEN + SF_LEN + DDDDI_LEN + ALFID_LEN + numNewDefinitions * (sizeFormat + addressFormat)) == pduRxData->SduLength) )
				{
					if( (numEarlierDefinitions+numNewDefinitions) <= DCM_MAX_DDDSOURCE_NUMBER )
					{
						for( definitionIndex = 0; (definitionIndex < numNewDefinitions) && (diagResponseCode == DCM_E_POSITIVERESPONSE); definitionIndex++ )
						{

							if( TRUE == Dcm_ConfigPtr->Dsp->DspMemory->DcmDspUseMemoryId ) {
								memoryIdentifier = pduRxData->SduDataPtr[DYNDEF_ADDRESS_START_INDEX + definitionIndex * (sizeFormat + addressFormat)];
								addressOffset = 1;
							}
							else {
								addressOffset = 0;
							}

							/* Parse address */
							memoryAddress = 0;
							for(i = addressOffset; i < addressFormat; i++)
							{
								memoryAddress <<= 8;
								memoryAddress += (uint32)(pduRxData->SduDataPtr[DYNDEF_ADDRESS_START_INDEX + definitionIndex * (sizeFormat + addressFormat) + i]);
							}

							/* Parse size */
							length = 0;
							for(i = 0; i < sizeFormat; i++)
							{
								length <<= 8;
								length += (uint32)(pduRxData->SduDataPtr[DYNDEF_ADDRESS_START_INDEX + definitionIndex * (sizeFormat + addressFormat) + addressFormat + i]);
							}

							diagResponseCode = checkAddressRange(DCM_READ_MEMORY, memoryIdentifier, memoryAddress, length);
							if( DCM_E_POSITIVERESPONSE == diagResponseCode )
							{
								DDid->DDDSource[definitionIndex + numEarlierDefinitions].formatOrPosition = pduRxData->SduDataPtr[DYNDEF_ALFID_INDEX];
								DDid->DDDSource[definitionIndex + numEarlierDefinitions].memoryIdentifier = memoryIdentifier;
								DDid->DDDSource[definitionIndex + numEarlierDefinitions].SourceAddressOrDid = memoryAddress;
								DDid->DDDSource[definitionIndex + numEarlierDefinitions].Size = length;
								DDid->DDDSource[definitionIndex + numEarlierDefinitions].DDDTpyeID = DCM_DDD_SOURCE_ADDRESS;
							}
						}
						if(diagResponseCode == DCM_E_POSITIVERESPONSE)
						{
							DDid->DynamicallyDid = DDIdentifier;
						}
						else
						{
							for( definitionIndex = 0; (definitionIndex < numNewDefinitions); definitionIndex++ )
							{
								DDid->DDDSource[definitionIndex + numEarlierDefinitions].formatOrPosition = 0x00;
								DDid->DDDSource[definitionIndex + numEarlierDefinitions].memoryIdentifier = 0x00;
								DDid->DDDSource[definitionIndex + numEarlierDefinitions].SourceAddressOrDid = 0x00000000;
								DDid->DDDSource[definitionIndex + numEarlierDefinitions].Size = 0x0000;
								DDid->DDDSource[definitionIndex + numEarlierDefinitions].DDDTpyeID = DCM_DDD_SOURCE_DEFAULT;
							}
						}
					}
					else
					{
						diagResponseCode = DCM_E_REQUESTOUTOFRANGE;
					}
				}
				else
				{
					diagResponseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
				}
			}
			else
			{
				diagResponseCode = DCM_E_REQUESTOUTOFRANGE;  /*UDS_REQ_0x23_10*/
			}
		}
		else
		{
			diagResponseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
		}
	}


	if(diagResponseCode == DCM_E_POSITIVERESPONSE)
	{
		pduTxData->SduDataPtr[SF_INDEX] = DCM_DDD_SUBFUNCTION_DEFINEBYADDRESS;
	}
	
	return diagResponseCode;
}


/*
	DESCRIPTION:
		 UDS Service 0x2c - Clear dynamically Did
*/
static Dcm_NegativeResponseCodeType CleardynamicallyDid(uint16 DDIdentifier,const PduInfoType *pduRxData, PduInfoType * pduTxData)
{
	/*UDS_REQ_0x2C_5*/
	sint8 i, j;
	uint8 position;
	Dcm_DspDDDType *DDid = NULL;
	Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;
	
	if(pduRxData->SduLength == 4)
	{
		if(TRUE == LookupDDD(DDIdentifier, (const Dcm_DspDDDType **)&DDid))
		{
			if((checkPeriodicIdentifierBuffer(pduRxData->SduDataPtr[3], dspPDidRef.PDidNr, &position) == TRUE)&&(pduRxData->SduDataPtr[2] == 0xF2))
			{
				/*UDS_REQ_0x2C_9*/
				responseCode = DCM_E_REQUESTOUTOFRANGE;
			}
			else
			{
				memset(DDid, 0, sizeof(Dcm_DspDDDType));
				for(i = DCM_MAX_DDD_NUMBER - 1;i >= 0 ;i--) {	/* find the first DDDid from bottom */
					if (0 != dspDDD[i].DynamicallyDid) {
						for (j = 0; j <DCM_MAX_DDD_NUMBER; j++) { /* find the first empty slot from top */
							if (j >= i) {
								/* Rearrange finished */
								pduTxData->SduDataPtr[1] = DCM_DDD_SUBFUNCTION_CLEAR;
								pduTxData->SduLength = 2;
								return responseCode;
							}
							else if (0 == dspDDD[j].DynamicallyDid) {	/* find, exchange */
								memcpy(&dspDDD[j], (Dcm_DspDDDType*)&dspDDD[i], sizeof(Dcm_DspDDDType));
								memset((Dcm_DspDDDType*)&dspDDD[i], 0, sizeof(Dcm_DspDDDType));
							}
						}
					}
				}
			}
		}
		else{
			responseCode = DCM_E_REQUESTOUTOFRANGE;	/* DDDid not found */
		}
	}

	else
	{
		responseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
	}
	if(responseCode == DCM_E_POSITIVERESPONSE)
	{
		pduTxData->SduDataPtr[1] = DCM_DDD_SUBFUNCTION_CLEAR;
		pduTxData->SduLength = 2;
	}
	
	return responseCode;
}

void DspDynamicallyDefineDataIdentifier(const PduInfoType *pduRxData,PduInfoType *pduTxData)
{
	/*UDS_REQ_0x2C_1,DCM 259*/
	uint16 i;
	uint8 Position;
	uint16 DDIdentifier;
	boolean PeriodicUse = FALSE;
	Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;

	if(pduRxData->SduLength > 2)
	{
		/* Check if DDID equals 0xF2 or 0xF3 */
		DDIdentifier = ((((uint16)pduRxData->SduDataPtr[2]) << 8) & DCM_DID_HIGH_MASK) + (pduRxData->SduDataPtr[3] & DCM_DID_LOW_MASK);
		if((pduRxData->SduDataPtr[2] == 0xF2) || (pduRxData->SduDataPtr[2] == 0xF3))
		{
			switch(pduRxData->SduDataPtr[1])	/*UDS_REQ_0x2C_2,DCM 646*/
			{
				case DCM_DDD_SUBFUNCTION_DEFINEBYDID:
					responseCode  = dynamicallyDefineDataIdentifierbyDid(DDIdentifier,pduRxData,pduTxData);
					break;
				case DCM_DDD_SUBFUNCTION_DEFINEBYADDRESS:
					responseCode = dynamicallyDefineDataIdentifierbyAddress(DDIdentifier,pduRxData,pduTxData);
					break;
				case DCM_DDD_SUBFUNCTION_CLEAR:
					responseCode = CleardynamicallyDid(DDIdentifier,pduRxData,pduTxData);
					break;
				default:
					responseCode = DCM_E_SUBFUNCTIONNOTSUPPORTED;
					/*UDS_REQ_0x2C_10*/
					break;		
			}
		}
		else
		{
			responseCode = DCM_E_REQUESTOUTOFRANGE;
		}
		if(responseCode == DCM_E_POSITIVERESPONSE)
		{
			pduTxData->SduDataPtr[2] = pduRxData->SduDataPtr[2];
			pduTxData->SduDataPtr[3] = pduRxData->SduDataPtr[3];
			pduTxData->SduLength = 4;
		}
	}
	else if((pduRxData->SduLength == 2)&&(pduRxData->SduDataPtr[1] == DCM_DDD_SUBFUNCTION_CLEAR))
	{
		/*UDS_REQ_0x2C_7*/
		for(i = 0;i < DCM_MAX_DDD_NUMBER;i++)
		{
			if(checkPeriodicIdentifierBuffer((uint8)(dspDDD[i].DynamicallyDid & DCM_DID_LOW_MASK),dspPDidRef.PDidNr,&Position) == TRUE)
			{
				PeriodicUse = TRUE;
			}
		}
		if(PeriodicUse == FALSE)
		{
			memset(dspDDD,0,sizeof(dspDDD));
			pduTxData->SduDataPtr[1] = DCM_DDD_SUBFUNCTION_CLEAR;
			pduTxData->SduLength = 2;
		}
		else
		{
			responseCode = DCM_E_CONDITIONSNOTCORRECT;
		}
	}
	else
	{
		/*UDS_REQ_0x2C_11*/
		responseCode =  DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
	}
	DsdDspProcessingDone(responseCode);
}
#endif

#if defined(DCM_USE_SERVICE_INPUTOUTPUTCONTROLBYIDENTIFIER)
static const Dcm_DspDidControlRecordSizesType* getControlRecordSizesForControlParameter(uint8 controlParam, const Dcm_DspDidControlType *DidControl)
{
	switch( controlParam )
	{
	case DCM_RETURN_CONTROL_TO_ECU:
		return DidControl->DspDidReturnControlToEcu;
	case DCM_RESET_TO_DEFAULT:
		return DidControl->DspDidResetToDefault;
	case DCM_FREEZE_CURRENT_STATE:
		return DidControl->DspDidFreezeCurrentState;
	case DCM_SHORT_TERM_ADJUSTMENT:
		return DidControl->DspDidShortTermAdjustment;
	default:
		return NULL;
	}
}
#endif

#if defined(DCM_USE_SERVICE_INPUTOUTPUTCONTROLBYIDENTIFIER) && defined(DCM_USE_CONTROL_DIDS)
static void DspStopInputOutputControl(boolean checkSessionAndSecLevel)
{
    const Dcm_DspDidControlType *DidControl = NULL;
    const Dcm_DspDidType *DidPtr = NULL;
    Dcm_NegativeResponseCodeType responseCode;
    /* @req DCM628 */
    for(uint16 i = 0; i < DCM_NOF_IOCONTROL_DIDS; i++) {
        if( IOControlStateList[i].controlActive ) {
            /* Control not in the hands of the ECU.
             * Return it. */
            if(lookupDid(IOControlStateList[i].did, &DidPtr)) {
                DidControl = DidPtr->DspDidInfoRef->DspDidAccess.DspDidControl;
                if(NULL != DidControl) {
                    boolean returnToECU = TRUE;
                    if(checkSessionAndSecLevel) {
                        /* Should check if supported in session and security level */
                        if( DspCheckSessionLevel(DidControl->DspDidControlSessionRef) && DspCheckSecurityLevel(DidControl->DspDidControlSecurityLevelRef) ) {
                            /* Control is supported in current session and security level.
                             * Do not return control to ECU. */
                            returnToECU = FALSE;
                        }
                    }
                    if( returnToECU ) {
                        /* Return control to the ECU */
                        if(DidPtr->DspDidReturnControlToEcuFnc != NULL) {
                            (void)DidPtr->DspDidReturnControlToEcuFnc(DCM_INITIAL, &responseCode);
                        }
                        IOControlStateList[i].controlActive = FALSE;
                    } else {
                        /* Control is supported in the current session and security level */
                    }
                } else {
                    /* No control access. */
                    DET_REPORTERROR(MODULE_ID_DCM, 0, DCM_GLOBAL_ID, DCM_E_UNEXPECTED_EXECUTION);
                    IOControlStateList[i].controlActive = FALSE;
                }
            } else {
                /* Did not found in config. Strange.. */
                IOControlStateList[i].controlActive = FALSE;
                DET_REPORTERROR(MODULE_ID_DCM, 0, DCM_GLOBAL_ID, DCM_E_UNEXPECTED_EXECUTION);
            }
        }
    }
}

static void DspIOControlSetActive(uint16 didNr, boolean active)
{
    boolean done = FALSE;
    if( active ) {
        /* Check if did already in list */
        for(uint16 i = 0; (i < DCM_NOF_IOCONTROL_DIDS) && !done; i++) {
            if(didNr == IOControlStateList[i].did) {
                IOControlStateList[i].controlActive = TRUE;
                done = TRUE;
            }
        }
        if( !done ) {
            /* Was not in list. Find an unused slot */
            for(uint16 i = 0; (i < DCM_NOF_IOCONTROL_DIDS) && !done; i++) {
                if( !IOControlStateList[i].controlActive ) {
                    IOControlStateList[i].did = didNr;
                    IOControlStateList[i].controlActive = TRUE;
                    done = TRUE;
                }
            }
        }
    } else {
        for(uint16 i = 0; (i < DCM_NOF_IOCONTROL_DIDS) && !done; i++) {
            if( didNr == IOControlStateList[i].did ) {
                IOControlStateList[i].controlActive = FALSE;
                done = TRUE;
            }
        }
    }
    if( active && !done ) {
        /* Should set control active but could not find an entry
         * to use. */
        DET_REPORTERROR(MODULE_ID_DCM, 0, DCM_GLOBAL_ID, DCM_E_UNEXPECTED_EXECUTION);
    }
}
#endif

#ifdef DCM_USE_SERVICE_INPUTOUTPUTCONTROLBYIDENTIFIER
void DspIOControlByDataIdentifier(const PduInfoType *pduRxData,PduInfoType *pduTxData)
{
    Std_ReturnType retVal = E_OK;
    uint8 controlStatusRecordSize = 0;
    uint16 didNr;
    const Dcm_DspDidType *DidPtr = NULL;
    const Dcm_DspDidControlType *DidControl = NULL;
    const Dcm_DspDidControlRecordSizesType* controlRecordSizes = NULL;
    Dcm_NegativeResponseCodeType responseCode = DCM_E_REQUESTOUTOFRANGE;

    if(pduRxData->SduLength >= SID_LEN + IOI_LEN + IOCP_LEN)
    {
        didNr = (pduRxData->SduDataPtr[IOI_INDEX] << 8 & DCM_DID_HIGH_MASK) + (pduRxData->SduDataPtr[IOI_INDEX+1] & DCM_DID_LOW_MASK);
        if(TRUE == lookupDid(didNr, &DidPtr))
        {
            DidControl = DidPtr->DspDidInfoRef->DspDidAccess.DspDidControl;
            if(NULL != DidControl)
            {
                if(TRUE == DspCheckSessionLevel(DidControl->DspDidControlSessionRef))
                {
                    if(TRUE == DspCheckSecurityLevel(DidControl->DspDidControlSecurityLevelRef))
                    {
                        controlRecordSizes = getControlRecordSizesForControlParameter(pduRxData->SduDataPtr[IOCP_INDEX], DidControl);
                        if( controlRecordSizes != NULL )
                        {
                            uint8 *maskPtr = NULL;
                            boolean requestOk = FALSE;
                            controlStatusRecordSize = controlRecordSizes->DspDidControlStatusRecordSize;
                            if( pduRxData->SduLength == SID_LEN + IOI_LEN + IOCP_LEN + controlRecordSizes->DspDidControlOptionRecordSize ) {
                                maskPtr = NULL;
                                requestOk = TRUE;
                            } else if( pduRxData->SduLength == SID_LEN + IOI_LEN + IOCP_LEN + controlRecordSizes->DspDidControlOptionRecordSize + controlRecordSizes->DspDidControlEnableMaskRecordSize) {
                                maskPtr = &pduRxData->SduDataPtr[COR_INDEX + controlRecordSizes->DspDidControlOptionRecordSize];
                                requestOk = TRUE;
                            }
                            (void)maskPtr;/* Remove warning.. */
                            if( requestOk) {
                                // todo: Loop over all datas when implementing support for more than one data, and use the mask bits
                                responseCode = DCM_E_REQUESTOUTOFRANGE; // Value to use if no callback found
                                switch(pduRxData->SduDataPtr[IOCP_INDEX])
                                {
                                case DCM_RETURN_CONTROL_TO_ECU:
                                    if(DidPtr->DspDidReturnControlToEcuFnc != NULL)
                                    {
                                        responseCode = DCM_E_POSITIVERESPONSE;
                                        retVal = DidPtr->DspDidReturnControlToEcuFnc(DCM_INITIAL ,&responseCode);
                                    }
                                    break;
                                case DCM_RESET_TO_DEFAULT:
                                    if(DidPtr->DspDidResetToDefaultFnc != NULL)
                                    {
                                        responseCode = DCM_E_POSITIVERESPONSE;
                                        retVal = DidPtr->DspDidResetToDefaultFnc(DCM_INITIAL ,&responseCode);
                                    }
                                    break;
                                case DCM_FREEZE_CURRENT_STATE:
                                    if(DidPtr->DspDidFreezeCurrentStateFnc != NULL)
                                    {
                                        responseCode = DCM_E_POSITIVERESPONSE;
                                        retVal = DidPtr->DspDidFreezeCurrentStateFnc(DCM_INITIAL ,&responseCode);
                                    }
                                    break;
                                case DCM_SHORT_TERM_ADJUSTMENT:
                                    if(DidPtr->DspDidShortTermAdjustmentFnc != NULL)
                                    {
                                        responseCode = DCM_E_POSITIVERESPONSE;
                                        uint8* controlOptionRecord = &pduRxData->SduDataPtr[COR_INDEX];
                                        retVal = DidPtr->DspDidShortTermAdjustmentFnc(controlOptionRecord, DCM_INITIAL, &responseCode);
                                    }
                                    break;
                                default:
                                    break;
                                }
#ifdef DCM_USE_CONTROL_DIDS
                                if( (E_OK == retVal) && (DCM_E_POSITIVERESPONSE == responseCode) ) {
                                    switch(pduRxData->SduDataPtr[IOCP_INDEX])
                                    {
                                    case DCM_RETURN_CONTROL_TO_ECU:
                                        DspIOControlSetActive(didNr, FALSE);
                                        break;
                                    case DCM_RESET_TO_DEFAULT:
                                    case DCM_FREEZE_CURRENT_STATE:
                                    case DCM_SHORT_TERM_ADJUSTMENT:
                                        DspIOControlSetActive(didNr, TRUE);
                                        break;
                                    default:
                                        break;
                                    }
                                }
#endif
                            } else {
                                responseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
                            }
                            if(responseCode == DCM_E_POSITIVERESPONSE && retVal != E_OK) {
                                responseCode = DCM_E_CONDITIONSNOTCORRECT;
                            }
                        }
                        else
                        {
                            responseCode = DCM_E_REQUESTOUTOFRANGE;
                        }
                    }
                    else
                    {
                        responseCode = DCM_E_SECURITYACCESSDENIED;
                    }
                }
                else
                {
                    responseCode = DCM_E_REQUESTOUTOFRANGE;
                }
            }
            else
            {
                responseCode = DCM_E_REQUESTOUTOFRANGE;
            }
        }
        else
        {
            responseCode = DCM_E_REQUESTOUTOFRANGE;
        }
    }
    else
    {
        responseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
    }
    if(responseCode == DCM_E_POSITIVERESPONSE) {
        if( pduTxData->SduLength >= SID_LEN + IOI_LEN + IOCP_LEN + controlStatusRecordSize ) {
            pduTxData->SduLength = SID_LEN + IOI_LEN + IOCP_LEN + controlStatusRecordSize;
            // did
            pduTxData->SduDataPtr[1] = pduRxData->SduDataPtr[1];
            pduTxData->SduDataPtr[2] = pduRxData->SduDataPtr[2];
            pduTxData->SduDataPtr[3] = pduRxData->SduDataPtr[IOCP_INDEX];
            // todo: rework this totally: use the read did implementation
            if((NULL != DidPtr) && (DidPtr->DspDidReadDataFnc.SynchDataReadFnc != NULL)) {
                if( DATA_PORT_SYNCH == DidPtr->DspDidUsePort ) {
                    retVal = DidPtr->DspDidReadDataFnc.SynchDataReadFnc(&pduTxData->SduDataPtr[4]);
                    if( E_PENDING == retVal ) {
                        /* Synch port cannot return E_PENDING */
                        DET_REPORTERROR(MODULE_ID_DCM, 0, DCM_GLOBAL_ID, DCM_E_UNEXPECTED_RESPONSE);
                        retVal = E_NOT_OK;
                    }
                } else if(DATA_PORT_ASYNCH == DidPtr->DspDidUsePort) {
                    retVal = DidPtr->DspDidReadDataFnc.AsynchDataReadFnc(DCM_INITIAL, &pduTxData->SduDataPtr[4]);
                } else {
                    /* Port not supported */
                    DET_REPORTERROR(MODULE_ID_DCM, 0, DCM_GLOBAL_ID, DCM_E_CONFIG_INVALID);
                    retVal = E_NOT_OK;
                }
                if(E_OK != retVal) {
                    responseCode = DCM_E_CONDITIONSNOTCORRECT;
                }
            } else {
                /* No read function */
                responseCode = DCM_E_REQUESTOUTOFRANGE;
            }
        } else {
            /* Tx buffer not big enough */
            responseCode = DCM_E_REQUESTOUTOFRANGE;
        }
    }
    DsdDspProcessingDone(responseCode);
}
#endif

#ifdef DCM_USE_SERVICE_COMMUNICATIONCONTROL
void DspCommunicationControl(const PduInfoType *pduRxData,PduInfoType *pduTxData)
{
    Dcm_NegativeResponseCodeType responseCode = DCM_E_REQUESTOUTOFRANGE;
    uint8 subFunction = pduRxData->SduDataPtr[SF_INDEX];
    if(pduRxData->SduLength == 3) {
        if( !IS_ISO_RESERVED(subFunction) ) {
            Dcm_E_CommunicationControl(subFunction, pduRxData->SduDataPtr[CC_CTP_INDEX], &responseCode);
            /* Check the response code to make sure that the callout did not set it
             * to something invalid.
             * Valid response codes positiveResponse, conditionsNotCorrect
             * subFunctionNotSupported and requestOutOfRange */
            if( !((DCM_E_POSITIVERESPONSE == responseCode) ||
                    (DCM_E_REQUESTOUTOFRANGE == responseCode) ||
                    (DCM_E_CONDITIONSNOTCORRECT == responseCode) ||
                    (DCM_E_SUBFUNCTIONNOTSUPPORTED == responseCode)) ) {
                /* Response invalid. Override it and report to Det. */
                DCM_DET_REPORTERROR(DCM_UDS_COMMUNICATION_CONTROL_ID, DCM_E_UNEXPECTED_RESPONSE);
                responseCode = DCM_E_REQUESTOUTOFRANGE;
            } else {
                /* Valid response. */
            }
        } else {
            /* ISO reserved for future definition */
            responseCode = DCM_E_SUBFUNCTIONNOTSUPPORTED;
        }
    } else {
        /* Length not correct */
        responseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
    }
    if(responseCode == DCM_E_POSITIVERESPONSE)
    {
        pduTxData->SduLength = SID_LEN + SF_LEN;
        pduTxData->SduDataPtr[SF_INDEX] = pduRxData->SduDataPtr[SF_INDEX];
    }
    DsdDspProcessingDone(responseCode);
}
#endif

#if defined(DCM_USE_SERVICE_REQUESTCURRENTPOWERTRAINDIAGNOSTICDATA) || defined(DCM_USE_SERVICE_REQUESTPOWERTRAINFREEZEFRAMEDATA)
static boolean lookupPid(uint8 pidId, Dcm_PidServiceType service, const Dcm_DspPidType **PidPtr)
{
    boolean pidFound = FALSE;
    const Dcm_DspPidType *dspPid = Dcm_ConfigPtr->Dsp->DspPid;

    if(dspPid != NULL)
    {
        while ((dspPid->DspPidIdentifier != pidId) && (!dspPid->Arc_EOL))
        {
            dspPid++;
        }
        if ((!dspPid->Arc_EOL) && ((DCM_SERVICE_01_02 == dspPid->DspPidService) || (service == dspPid->DspPidService) ))
        {
            pidFound = TRUE;
            *PidPtr = dspPid;
        }
        else
        {
            /*do nothing*/
        }
    }
    else
    {
        /*do nothing*/
    }

    return pidFound;
}
#endif

#ifdef DCM_USE_SERVICE_REQUESTCURRENTPOWERTRAINDIAGNOSTICDATA
static boolean setAvailabilityPidValue(uint8 Pid, Dcm_PidServiceType service, uint32 *Data)
{
    uint8 shift;
    uint32 pidData = 0;
    uint32 temp;
    boolean setOk = TRUE;
    const Dcm_DspPidType *dspPid = Dcm_ConfigPtr->Dsp->DspPid;

    if(dspPid != NULL) {
        while (0 == dspPid->Arc_EOL) {
            if( (DCM_SERVICE_01_02 == dspPid->DspPidService) || (service == dspPid->DspPidService) ) {
                if((dspPid->DspPidIdentifier >= (Pid + AVAIL_TO_SUPPORTED_PID_OFFSET_MIN)) && (dspPid->DspPidIdentifier <= (Pid + AVAIL_TO_SUPPORTED_PID_OFFSET_MAX))) {
                    shift = dspPid->DspPidIdentifier - Pid;
                    temp = (uint32)1 << (AVAIL_TO_SUPPORTED_PID_OFFSET_MAX - shift);
                    pidData |= temp;
                } else if(dspPid->DspPidIdentifier > (Pid + AVAIL_TO_SUPPORTED_PID_OFFSET_MAX)) {
                    pidData |= (uint32)1;
                } else {
                    /*do nothing*/
                }
            }
            dspPid++;
        }
    } else {
        setOk = FALSE;
    }

    if(0 == pidData) {
        setOk = FALSE;
    } else {
        /*do nothing*/
    }
    (*Data) = pidData;
    
    return setOk;
}

/*@req OBD_DCM_REQ_2*//* @req OBD_REQ_1 */
void DspObdRequestCurrentPowertrainDiagnosticData(const PduInfoType *pduRxData,PduInfoType *pduTxData)
{
	uint16 i ;
	uint16 nofAvailabilityPids = 0;
	uint16 findPid = 0;
	uint16 txPos = SID_LEN;
	uint32 DATA = 0;
	uint8 txBuffer[255] = {0};
	uint16 txLength = SID_LEN;
	uint8 requestPid[MAX_REQUEST_PID_NUM];
	uint16 pidNum = pduRxData->SduLength - SID_LEN;
	const Dcm_DspPidType *sourcePidPtr = NULL;
	Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;
	/* @req OBD_REQ_3 */
	if((pduRxData->SduLength >= OBD_REQ_MESSAGE_LEN_ONE_MIN) && (pduRxData->SduLength <= OBD_REQ_MESSAGE_LEN_MAX))
	{
		i = pidNum;
		while(i > 0)
		{
			requestPid[i - 1] = pduRxData->SduDataPtr[i];
			if( IS_AVAILABILITY_PID(requestPid[i - 1]) )
			{
			    nofAvailabilityPids++; /*used to judge if the message is valid, if nofAvailabilityPids != 0 or Pidnum, invalid*/
			}
			else
			{
				/*do nothing*/
			}
			i--;
		}
		for(i = 0;i < pidNum;i++)                                        /*figure out the txLength to be sent*/
		{
			/*situation of supported Pids*/
			if(TRUE == lookupPid(requestPid[i], DCM_SERVICE_01, &sourcePidPtr))
			{
				txLength += PID_LEN + sourcePidPtr->DspPidSize;
			}
			/*situation of availability Pids*/
			else if( IS_AVAILABILITY_PID(requestPid[i]) )
			{
				txLength += PID_LEN + SUPPRTED_PIDS_DATA_LEN;
			}
			else 
			{
				/*do nothing*/
			}
		}
		/*@req OBD_DCM_REQ_7*/
		if(txLength <= pduTxData->SduLength)			 	/*if txLength is smaller than the configured length*/
		{
			if(pidNum == nofAvailabilityPids) /*check if all the request PIDs are the 0x00...0xE0 format*/
			{
				for(i = 0;i < pidNum;i++)		/*Check the PID configuration,find which PIDs were configured for 0x00,0x20,0x40 respectively,and fill in the pduTxBuffer,and count the txLength*/
				{
					/*@OBD_DCM_REQ_3,@OBD_DCM_REQ_6*/
					if(TRUE == setAvailabilityPidValue(requestPid[i], DCM_SERVICE_01, &DATA))
					{						
						pduTxData->SduDataPtr[txPos++] = requestPid[i];
						/*take every byte of uint32 DATA,and fill in txbuffer*/
						pduTxData->SduDataPtr[txPos++] = (uint8)(((DATA) & (OBD_DATA_LSB_MASK << OFFSET_THREE_BYTES)) >> OFFSET_THREE_BYTES);
						pduTxData->SduDataPtr[txPos++] = (uint8)(((DATA) & (OBD_DATA_LSB_MASK << OFFSET_TWO_BYTES)) >> OFFSET_TWO_BYTES);
						pduTxData->SduDataPtr[txPos++] = (uint8)(((DATA) & (OBD_DATA_LSB_MASK << OFFSET_ONE_BYTE)) >> OFFSET_ONE_BYTE);
						pduTxData->SduDataPtr[txPos++] = (uint8)((DATA) & OBD_DATA_LSB_MASK);
					}
					else if(PIDZERO == requestPid[i])
					{
						pduTxData->SduDataPtr[txPos++] = requestPid[i];
						pduTxData->SduDataPtr[txPos++] = DATAZERO;
						pduTxData->SduDataPtr[txPos++] = DATAZERO;
						pduTxData->SduDataPtr[txPos++] = DATAZERO;
						pduTxData->SduDataPtr[txPos++] = DATAZERO;
					}
					else
					{
						findPid++;
					}
				}
			}
			else if(0 == nofAvailabilityPids) /*check if all the request PIDs are the supported PIDs,like 0x01,0x02...*/
			{
				for(i = 0;i < pidNum;i++)
				{
					if(TRUE == lookupPid(requestPid[i], DCM_SERVICE_01, &sourcePidPtr))
					{
						/*@req OBD_DCM_REQ_3,OBD_DCM_REQ_5,OBD_DCM_REQ_8*//* @req OBD_REQ_2 */
						if(E_OK == sourcePidPtr->DspGetPidValFnc(txBuffer))
						{
							pduTxData->SduDataPtr[txPos] = requestPid[i];
							txPos++;
							(void)memcpy(&(pduTxData->SduDataPtr[txPos]),txBuffer,sourcePidPtr->DspPidSize);
							txPos += sourcePidPtr->DspPidSize;
						}
						else
						{
							responseCode = DCM_E_CONDITIONSNOTCORRECT;
							break;
						}

					}
					else
					{
						findPid++;
					}
				}
			}
			else
			{
				responseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
			}
			if(pidNum == findPid)
			{
				responseCode = DCM_E_REQUESTOUTOFRANGE;
			}
			else
			{
				/*do nothing*/
			}
		}
		else
		{
			responseCode = DCM_E_REQUESTOUTOFRANGE;
		}
	}
	else
	{
		responseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
	}
	if(DCM_E_POSITIVERESPONSE == responseCode)
	{
		pduTxData->SduLength = txPos;
	}
	else
	{
		/*do nothing*/
	}
	DsdDspProcessingDone(responseCode);

	return;
}
#endif

#if defined(USE_DEM) && defined(DCM_USE_SERVICE_REQUESTPOWERTRAINFREEZEFRAMEDATA)

/*@req OBD_DCM_REQ_9*/
void DspObdRequestPowertrainFreezeFrameData(const PduInfoType *pduRxData,PduInfoType *pduTxData)
{
    uint16 i ;
    uint16 j = 0;
    uint16 nofAvailabilityPids = 0;
    uint16 findPid = 0;
    uint32 dtc = 0;
    uint32 supportBitfield = 0;
    uint16 txPos = SID_LEN;
    uint16 txLength = SID_LEN;
    uint8 requestPid[DCM_MAX_PID_NUM_IN_FF];
    uint8 requestFFNum[DCM_MAX_PID_NUM_IN_FF];
    uint16 messageLen = pduRxData->SduLength;
    const Dcm_DspPidType *sourcePidPtr = NULL;
    Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;

    /* @req OBD_REQ_6 */
    if((messageLen >= OBD_REQ_MESSAGE_LEN_TWO_MIN ) && (messageLen <= OBD_REQ_MESSAGE_LEN_MAX ) && (((messageLen - 1) % 2) == 0))
    {
        uint16 pidNum = ((messageLen - 1) >> 1);

        /*find out PID and FFnum*/
        for(i = 0;i < pidNum;i++)
        {
            requestPid[i] = pduRxData->SduDataPtr[j + 1];
            if( IS_AVAILABILITY_PID(requestPid[i]) )
            {
                nofAvailabilityPids++; /*used to judge if the message is valid, if nofAvailabilityPids != 0 or Pidnum, invalid*/
            }
            requestFFNum[i] = pduRxData->SduDataPtr[j + 2];
            j += 2;
        }
        /*count txLength*/
        for(i = 0;i < pidNum;i++)
        {
            if(requestPid[i] == OBD_SERVICE_TWO)
            {
                txLength += PID_LEN + FF_NUM_LEN + OBD_DTC_LEN;
            }
            else if( IS_AVAILABILITY_PID(requestPid[i]) )
            {
                txLength += PID_LEN + SUPPRTED_PIDS_DATA_LEN;
            }
            else if(TRUE == lookupPid(requestPid[i], DCM_SERVICE_02, &sourcePidPtr))
            {
                txLength += PID_LEN + FF_NUM_LEN + sourcePidPtr->DspPidSize;
            }
            else
            {
                /*do nothing*/
            }
        }
        /*@req OBD_DCM_REQ_7*/
        if(txLength <= (pduTxData->SduLength))
        {
            if(pidNum == nofAvailabilityPids) /*check if all the request PIDs are the 0x00...0xE0 format*/
            {
                for(i = 0;i < pidNum;i++)		/*Check the PID configuration,find which PIDs were configured for 0x00,0x20,0x40 respectively,and fill in the pduTxBuffer,and count the txLength*/
                {
                    if(requestFFNum[i] == RECORD_NUM)
                    {
                        if(TRUE == setAvailabilityPidValue(requestPid[i], DCM_SERVICE_02, &supportBitfield)) {
                            pduTxData->SduDataPtr[txPos++] = requestPid[i];
                            pduTxData->SduDataPtr[txPos++] = requestFFNum[i];
                            /*take every byte of uint32 DATA,and fill in txbuffer*/
                            pduTxData->SduDataPtr[txPos++] = (uint8)(((supportBitfield) & (OBD_DATA_LSB_MASK << OFFSET_THREE_BYTES)) >> OFFSET_THREE_BYTES);
                            pduTxData->SduDataPtr[txPos++] = (uint8)(((supportBitfield) & (OBD_DATA_LSB_MASK << OFFSET_TWO_BYTES)) >> OFFSET_TWO_BYTES);
                            pduTxData->SduDataPtr[txPos++] = (uint8)(((supportBitfield) & (OBD_DATA_LSB_MASK << OFFSET_ONE_BYTE)) >> OFFSET_ONE_BYTE);
                            pduTxData->SduDataPtr[txPos++] = (uint8)((supportBitfield) & OBD_DATA_LSB_MASK);
                        } else if(PIDZERO == requestPid[i]) {
                            pduTxData->SduDataPtr[txPos++] = requestPid[i];
                            pduTxData->SduDataPtr[txPos++] = requestFFNum[i];
                            pduTxData->SduDataPtr[txPos++] = DATAZERO;
                            pduTxData->SduDataPtr[txPos++] = DATAZERO;
                            pduTxData->SduDataPtr[txPos++] = DATAZERO;
                            pduTxData->SduDataPtr[txPos++] = DATAZERO;
                        } else {
                            findPid++;
                        }
                    }
                    else
                    {
                        /*@req OBD_DCM_REQ_11*/
                        responseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
                        break;
                    }

                }
            }
            else if(0 == nofAvailabilityPids) /*check if all the request PIDs are the supported PIDs,like 0x01,0x02...*/
            {
                for(i = 0;i < pidNum;i++)
                {
                    /*@req OBD_DCM_REQ_10*/
                    if(requestFFNum[i] == RECORD_NUM)
                    {
                        uint8 bufSize = 0;
                        if(requestPid[i] == OBD_SERVICE_TWO)
                        {
                            /*@req OBD_DCM_REQ_12,@OBD_DCM_REQ_13,@OBD_DCM_REQ_14*/
                            if(E_OK == Dem_GetDTCOfOBDFreezeFrame(requestFFNum[i],&dtc))
                            {
                                pduTxData->SduDataPtr[txPos++] = requestPid[i];
                                pduTxData->SduDataPtr[txPos++] = requestFFNum[i];
                                pduTxData->SduDataPtr[txPos++] = (uint8)(((dtc) & (OBD_DATA_LSB_MASK << OFFSET_ONE_BYTE)) >> OFFSET_ONE_BYTE);
                                pduTxData->SduDataPtr[txPos++] = (uint8)((dtc) & OBD_DATA_LSB_MASK);
                            }
                            /*if the DTC did not cause the stored FF,DTC of 0x0000 should be returned*/
                            /* @req OBD_REQ_5 */
                            else
                            {
                                pduTxData->SduDataPtr[txPos++] = requestPid[i];
                                pduTxData->SduDataPtr[txPos++] = requestFFNum[i];
                                pduTxData->SduDataPtr[txPos++] = 0x00;
                                pduTxData->SduDataPtr[txPos++] = 0x00;
                            }
                        }
                        /*req OBD_DCM_REQ_17*/
                        else
                        {
                            /*@req OBD_DCM_REQ_28*/
                            pduTxData->SduDataPtr[txPos++] = requestPid[i];
                            pduTxData->SduDataPtr[txPos++] = requestFFNum[i];
                            bufSize = (uint8)(pduTxData->SduLength - txPos);
                            /*@req OBD_DCM_REQ_15,OBD_DCM_REQ_16*//* @req OBD_REQ_4 */
                            /* TODO: Dem_GetOBDFreezeFrameData should be called for each data element in the
                             * Pid. Parameter DataElementIndexOfPid should be the index of the data element
                             * within the Pid. But currently only one data element per Pid is supported. */
                            if(E_OK == Dem_GetOBDFreezeFrameData(requestPid[i], DATA_ELEMENT_INDEX_OF_PID_NOT_SUPPORTED, &(pduTxData->SduDataPtr[txPos]), &bufSize))
                            {
                                txPos += bufSize;
                            }
                            else
                            {
                                txPos -= 2;
                                findPid++;
                            }
                        }
                    }
                    else
                    {
                        /*@req OBD_DCM_REQ_11*/
                        responseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
                        break;
                    }
                }

            }
            else
            {
                responseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
            }
            if(pidNum == findPid)
            {
                responseCode = DCM_E_REQUESTOUTOFRANGE;
            }
            else
            {
                /*do nothing*/
            }
        }
        else
        {
            responseCode = DCM_E_REQUESTOUTOFRANGE;
        }
    }
    else
    {
        responseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
    }
    if(DCM_E_POSITIVERESPONSE == responseCode)
    {
        pduTxData->SduLength = txPos;
    }
    else
    {
        /*do nothing*/
    }
    DsdDspProcessingDone(responseCode);

    return;
}
#endif

#if defined(USE_DEM) && defined(DCM_USE_SERVICE_CLEAREMISSIONRELATEDDIAGNOSTICDATA)
static boolean lookupService(uint8 serviceId,const Dcm_DsdServiceType **dsdService)
{
	boolean serviceFind = FALSE;
	const Dcm_DsdServiceTableType *ServiceTable = Dcm_ConfigPtr->Dsd->DsdServiceTable;
	const Dcm_DsdServiceType *dsdServicePtr = NULL;

	if(ServiceTable != NULL)
	{
		while(0 == ServiceTable->Arc_EOL)
		{
			if(ServiceTable->DsdService != NULL)
			{
				dsdServicePtr = ServiceTable->DsdService;
				while((serviceId != dsdServicePtr->DsdSidTabServiceId) && (0 == dsdServicePtr->Arc_EOL))
				{
					dsdServicePtr++;
				}
				if((serviceId == dsdServicePtr->DsdSidTabServiceId) && (0 == dsdServicePtr->Arc_EOL))
				{
					*dsdService = dsdServicePtr;
					serviceFind = TRUE;
				}
			}

			ServiceTable++;
		}
	}

	return serviceFind;
}

/*@req OBD_DCM_REQ_19*//* @req OBD_REQ_10 */
void DspObdClearEmissionRelatedDiagnosticData(const PduInfoType *pduRxData,PduInfoType *pduTxData)
{
	uint16 messageLen = pduRxData->SduLength;
	const Dcm_DsdServiceType *dsdService = NULL;
	Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;

	if(messageLen == SID_LEN )
	{
		if(TRUE == lookupService(OBD_SERVICE_FOUR, &dsdService))
		{
			if(dsdService->ArcDcmDsdSidConditionCheckFnc != NULL)
			{
				/* @req OBD_REQ_11 */
				if(E_OK == dsdService->ArcDcmDsdSidConditionCheckFnc())
				{
					/*@req OBD_DCM_REQ_1,OBD_DCM_REQ_20,OBD_DCM_REQ_21*/
					if(DEM_CLEAR_OK == Dem_ClearDTC(DEM_DTC_GROUP_ALL_DTCS, DEM_DTC_KIND_EMISSION_REL_DTCS, DEM_DTC_ORIGIN_PRIMARY_MEMORY))
					{
						/*do nothing*/
					}
					else
					{
						responseCode = DCM_E_CONDITIONSNOTCORRECT;
					}
				}
				else
				{
					responseCode = DCM_E_CONDITIONSNOTCORRECT;
				}

			}
			if(dsdService->resetPids != NULL)
			{
				if(E_OK != dsdService->resetPids())
				{
					responseCode = DCM_E_CONDITIONSNOTCORRECT;
				}
			}
		}
	}
	else
	{
		responseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
	}

	/*@req OBD_DCM_REQ_22*/
	if(DCM_E_POSITIVERESPONSE == responseCode)
	{
		pduTxData->SduLength = SID_LEN;
	}
	else
	{
		/*do nothing*/
	}
	DsdDspProcessingDone(responseCode);

	return;
}
#endif

#if defined(USE_DEM)
#if defined(DCM_USE_SERVICE_REQUESTEMISSIONRELATEDDIAGNOSTICTROUBLECODES) || defined(DCM_USE_SERVICE_REQUESTEMISSIONRELATEDDTCSDETECTEDDURINGCURRENTORLASTCOMPLETEDDRIVINGCYCLE)
static Dcm_NegativeResponseCodeType OBD_Sevice_03_07(PduInfoType *pduTxData, Dem_ReturnSetFilterType setDtcFilterResult)
{
    Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;

    if (setDtcFilterResult == DEM_FILTER_ACCEPTED)
    {
        uint32 dtc;
        Dem_EventStatusExtendedType dtcStatus;
        uint8 nrOfDtcs = 0;
        uint8 index = 2;

        while ((Dem_GetNextFilteredDTC(&dtc, &dtcStatus)) == DEM_FILTERED_OK)
        {

            if((index + LENGTH_OF_DTC) >= (pduTxData->SduLength))
            {
                responseCode = DCM_E_REQUESTOUTOFRANGE;
                break;
            }
            /* @req OBD_REQ_9 */
            pduTxData->SduDataPtr[index] = (uint8)EMISSION_DTCS_HIGH_BYTE(dtc);
            pduTxData->SduDataPtr[1+index] = (uint8)EMISSION_DTCS_LOW_BYTE(dtc);
            index += LENGTH_OF_DTC;
            nrOfDtcs++;

        }
        /* @req OBD_REQ_8 */
        if(responseCode == DCM_E_POSITIVERESPONSE)
        {
            pduTxData->SduLength = (PduLengthType)(index);
            pduTxData->SduDataPtr[1] = nrOfDtcs;
        }

    }
    else
    {
        responseCode = DCM_E_CONDITIONSNOTCORRECT;
    }

    return responseCode;

}
#endif
#endif

#if defined(USE_DEM) && defined(DCM_USE_SERVICE_REQUESTEMISSIONRELATEDDIAGNOSTICTROUBLECODES)
/*@req OBD_DCM_REQ_23*//* @req OBD_REQ_7 */
void  DspObdRequestEmissionRelatedDiagnosticTroubleCodes(const PduInfoType *pduRxData, PduInfoType *pduTxData)
{
	uint16 messageLen = pduRxData->SduLength;
	Dem_ReturnSetFilterType setDtcFilterResult;
	Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;

	if(messageLen == SID_LEN )
	{
		/*"confirmed" diagnostic trouble codes*/
		/*@req OBD_DCM_REQ_1*/	/*@req OBD_DCM_REQ_24*/
		setDtcFilterResult = Dem_SetDTCFilter(DEM_CONFIRMED_DTC, DEM_DTC_KIND_EMISSION_REL_DTCS, DEM_DTC_FORMAT_OBD,
											DEM_DTC_ORIGIN_PRIMARY_MEMORY, DEM_FILTER_WITH_SEVERITY_NO,
											VALUE_IS_NOT_USED, DEM_FILTER_FOR_FDC_NO);

		responseCode = OBD_Sevice_03_07(pduTxData,setDtcFilterResult);

	}
	else
	{
		responseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
	}

	DsdDspProcessingDone(responseCode);

}
#endif

#if defined(USE_DEM) && defined(DCM_USE_SERVICE_REQUESTEMISSIONRELATEDDTCSDETECTEDDURINGCURRENTORLASTCOMPLETEDDRIVINGCYCLE)
/*@req OBD_DCM_REQ_25*//* @req OBD_REQ_12 */
void  DspObdRequestEmissionRelatedDiagnosticTroubleCodesService07(const PduInfoType *pduRxData, PduInfoType *pduTxData)
{
	uint16 messageLen = pduRxData->SduLength;
	Dem_ReturnSetFilterType setDtcFilterResult;
	Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;

	if(messageLen == SID_LEN )
	{
		/*"pending" diagnostic trouble codes*/
		/*@req OBD_DCM_REQ_1*/	/*@req OBD_DCM_REQ_26*/
		setDtcFilterResult = Dem_SetDTCFilter(DEM_PENDING_DTC, DEM_DTC_KIND_EMISSION_REL_DTCS, DEM_DTC_FORMAT_OBD,
											DEM_DTC_ORIGIN_PRIMARY_MEMORY, DEM_FILTER_WITH_SEVERITY_NO,
											VALUE_IS_NOT_USED, DEM_FILTER_FOR_FDC_NO);

		responseCode = OBD_Sevice_03_07(pduTxData,setDtcFilterResult);

	}
	else
	{
		responseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
	}

  DsdDspProcessingDone(responseCode);
  
  return;

}
#endif

#ifdef DCM_USE_SERVICE_REQUESTVEHICLEINFORMATION
static boolean lookupInfoType(uint8 InfoType, const Dcm_DspVehInfoType **InfoTypePtr)
{
	const Dcm_DspVehInfoType *dspVehInfo = Dcm_ConfigPtr->Dsp->DspVehInfo;
	boolean InfoTypeFound = FALSE;

	while ((dspVehInfo->DspVehInfoType != InfoType) && ((dspVehInfo->Arc_EOL) == FALSE))
	{
		dspVehInfo++;
	}
	if ((dspVehInfo->Arc_EOL) == FALSE)
	{
		InfoTypeFound = TRUE;
		*InfoTypePtr = dspVehInfo;
	}

	return InfoTypeFound;
}

static boolean setAvailabilityInfoTypeValue(uint8 InfoType, uint32 *DATABUF)
{
	uint8 shift;
	uint32 databuf = 0;
	uint32 temp;
	boolean setInfoTypeOk = TRUE;
	const Dcm_DspVehInfoType *dspVehInfo = Dcm_ConfigPtr->Dsp->DspVehInfo;

    if(dspVehInfo != NULL)
    {
        while ((dspVehInfo->DspVehInfoType != FALSE) &&  ((dspVehInfo->Arc_EOL) == FALSE))
        {
            if((dspVehInfo->DspVehInfoType >= (InfoType + AVAIL_TO_SUPPORTED_INFOTYPE_OFFSET_MIN)) && (dspVehInfo->DspVehInfoType <= (InfoType + AVAIL_TO_SUPPORTED_INFOTYPE_OFFSET_MAX)))
            {
                shift = dspVehInfo->DspVehInfoType - InfoType;
                temp = (uint32)1 << (AVAIL_TO_SUPPORTED_INFOTYPE_OFFSET_MAX - shift);
                databuf |= temp;
            }
            else if( dspVehInfo->DspVehInfoType > (InfoType + AVAIL_TO_SUPPORTED_INFOTYPE_OFFSET_MAX))
            {
                databuf |= (uint32)0x01;
            }
            else
            {
                /*do nothing*/
            }
            dspVehInfo++;
        }

        if(databuf == 0)
        {
            setInfoTypeOk = FALSE;
        }
        else
        {
            /*do nothing*/
        }
    }
    else
    {
        setInfoTypeOk = FALSE;
    }

    (*DATABUF) = databuf;

    return setInfoTypeOk;

}

/*@req OBD_DCM_REQ_27*//*@req OBD_REQ_13*/
void DspObdRequestVehicleInformation(const PduInfoType *pduRxData, PduInfoType *pduTxData)
{
	uint8 i ;
	uint8 nofAvailabilityInfoTypes = 0;
	uint8 requestInfoType[MAX_REQUEST_VEHINFO_NUM];
	uint16 txPos = SID_LEN;
	uint32 DATABUF;
	uint8 txBuffer[PID_BUFFER_SIZE];
	uint8 findNum = 0;
	uint16 InfoTypeNum = pduRxData->SduLength - 1;
	const Dcm_DspVehInfoType *sourceVehInfoPtr = NULL;
	Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;

	/*@req OBD_REQ_14*/
	if((pduRxData->SduLength >= OBD_REQ_MESSAGE_LEN_ONE_MIN) && (pduRxData->SduLength <= OBD_REQ_MESSAGE_LEN_MAX ))
	{
		i = (uint8)InfoTypeNum;
		while(i > 0)
		{
			requestInfoType[i - 1] = pduRxData->SduDataPtr[i];
			if( IS_AVAILABILITY_INFO_TYPE(requestInfoType[i - 1]) )
			{
			    nofAvailabilityInfoTypes++;
			}
			else
			{
			}
			i--;
		 }

		/*@req OBD_DCM_REQ_29*/
		if(InfoTypeNum == nofAvailabilityInfoTypes)	/*check if all the request PIDs are the 0x00...0xE0 format*/
		{
			for(i = 0;i < InfoTypeNum;i++)		/*Check the PID configuration,find which PIDs were configured for 0x00,0x20,0x40 respectively,and fill in the pduTxBuffer,and count the txLength*/
			{
				if(TRUE == setAvailabilityInfoTypeValue(requestInfoType[i], &DATABUF))
				{
					pduTxData->SduDataPtr[txPos++] = requestInfoType[i];
					/*take every byte of uint32 DTC,and fill in txbuffer*/
					pduTxData->SduDataPtr[txPos++] = (uint8)((DATABUF & (OBD_DATA_LSB_MASK << OFFSET_THREE_BYTES)) >> OFFSET_THREE_BYTES);
					pduTxData->SduDataPtr[txPos++] = (uint8)((DATABUF & (OBD_DATA_LSB_MASK << OFFSET_TWO_BYTES)) >> OFFSET_TWO_BYTES);
					pduTxData->SduDataPtr[txPos++] = (uint8)((DATABUF & (OBD_DATA_LSB_MASK << OFFSET_ONE_BYTE)) >> OFFSET_ONE_BYTE);
					pduTxData->SduDataPtr[txPos++] = (uint8)(DATABUF & OBD_DATA_LSB_MASK);
				}
				else if(INFOTYPE_ZERO == requestInfoType[i])
				{
					pduTxData->SduDataPtr[txPos++] = requestInfoType[i];
					pduTxData->SduDataPtr[txPos++] = DATAZERO;
					pduTxData->SduDataPtr[txPos++] = DATAZERO;
					pduTxData->SduDataPtr[txPos++] = DATAZERO;
					pduTxData->SduDataPtr[txPos++] = DATAZERO;
				}
				else
				{
					findNum++;
				}
			}
		}
		/*@req OBD_DCM_REQ_28*/
		else if(nofAvailabilityInfoTypes == 0) /*check if all the request PIDs are the supported VINs,like 0x01,0x02...*/
		{
			/*@req OBD_REQ_15*/
			if(pduRxData->SduLength == OBD_REQ_MESSAGE_LEN_ONE_MIN)
			{
				if(TRUE == lookupInfoType(requestInfoType[i], &sourceVehInfoPtr ))
				{

					if (sourceVehInfoPtr->DspGetVehInfoTypeFnc(txBuffer) != E_OK) 
					{
						if( requestInfoType[i] == 0x02 )	/* Special for read VIN fail,  customer's requirement*/
						{
							for(uint8 j = 0; j < (OBD_VIN_LENGTH*sourceVehInfoPtr->DspVehInfoNumberOfDataItems);j++)
							{
								txBuffer[j] = 0xff;
							}
						}
						else
						{
							responseCode = DCM_E_CONDITIONSNOTCORRECT;
						}
					}

					pduTxData->SduDataPtr[txPos] = requestInfoType[i];

					txPos++;
					/*@req OBD_DCM_REQ_30*/
					pduTxData->SduDataPtr[txPos] = sourceVehInfoPtr->DspVehInfoNumberOfDataItems;
					txPos++;
					(void)memcpy(&(pduTxData->SduDataPtr[txPos]), txBuffer, (sourceVehInfoPtr->DspVehInfoSize * (sourceVehInfoPtr->DspVehInfoNumberOfDataItems)));

					txPos += (sourceVehInfoPtr->DspVehInfoSize * (sourceVehInfoPtr->DspVehInfoNumberOfDataItems)) ;
					if(txPos >= ((pduTxData->SduLength)))
					{
						responseCode = DCM_E_REQUESTOUTOFRANGE;
					}
				}
				else
				{
					findNum++;
				}
			}
			/*@req OBD_REQ_16*/
			else
			{
				responseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
			}
	 	}
		else
		{
			responseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
		}

		if(findNum == InfoTypeNum)
		{
			responseCode = DCM_E_REQUESTOUTOFRANGE;
		}
		else
		{
			/* do nothing */
		}
	}
	else
	{
		responseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
	}

	if(DCM_E_POSITIVERESPONSE == responseCode)
	{
		pduTxData->SduLength = txPos;
	}
	else
	{
		/* do nothing */
	}

	DsdDspProcessingDone(responseCode);

}
#endif

uint32 DspRoutineInfoReadUnsigned(uint8 *data, uint16 bitOffset, uint8 size, boolean changeEndian) {
    uint32 retVal = 0;
    const uint16 little_endian = 0x1;
    if(size > 32) {
        DET_REPORTERROR(MODULE_ID_DCM, 0, DCM_GLOBAL_ID, DCM_E_CONFIG_INVALID);
        return 0;
    }
    if(changeEndian ^ *((uint8*)&little_endian)) {
        // read little endian
        for(int i = 0; i < size / 8; i++) {
            retVal = (retVal << 8) | (data+bitOffset/8 + size/8 - 1)[-i];
        }
    } else {
        // read big endian
        for(int i = 0; i < size / 8; i++) {
            retVal = (retVal << 8) | (data+bitOffset/8)[i];
        }
    }
	return retVal;
}
sint32 DspRoutineInfoRead(uint8 *data, uint16 bitOffset, uint8 size, boolean changeEndian) {
    uint32 retVal = DspRoutineInfoReadUnsigned(data, bitOffset, size, changeEndian);
    uint32 mask = 0xFFFFFFFF << (size - 1);
    if(retVal & mask) {
        // result is negative
        retVal &= mask;
    }
    return (sint32)retVal;
}
void DspRoutineInfoWrite(uint32 val, uint8 *data, uint16 bitOffset, uint8 size, boolean changeEndian) {
    const uint16 little_endian = 0x1;
    if(changeEndian ^ *((uint8*)&little_endian)) {
        // write little endian
        for(int i = 0; i < size / 8; i++) {
            (data+bitOffset/8)[i] = 0xFF & val;
            val = val >> 8;
        }
    } else {
        for(int i = 0; i < size / 8; i++) {
            (data+(bitOffset + size)/8 - 1)[-i] = 0xFF & val;
            val = val >> 8;
        }
    }
}

#define UDS_0x34_MAX_NUM_BLOCK_LEN   (1440 + 2)
#define UDS_0x35_MAX_NUM_BLOCK_LEN    (128 + 2)

void DspUdsRequestDownload(const PduInfoType *pduRxData, PduInfoType *pduTxData)
{
	Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;
	(void)pduRxData;
	// Prepare positive response message
    pduTxData->SduLength = 4;   //SID + LengthFormatIdentifier + maxNumberOfBlockLength
    pduTxData->SduDataPtr[1] = 0x20;    //currently we set maxNumberOfBlock as 4090(0x0FFA), so it takes 2 bytes
    pduTxData->SduDataPtr[2] = (uint8)(UDS_0x34_MAX_NUM_BLOCK_LEN >> 8);
    pduTxData->SduDataPtr[3] = (uint8)(UDS_0x34_MAX_NUM_BLOCK_LEN & 0xFF);

    DsdDspProcessingDone(responseCode);
}

void DspUdsRequestUpload(const PduInfoType *pduRxData, PduInfoType *pduTxData)
{
	Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;
	(void)pduRxData;
	// Prepare positive response message
    pduTxData->SduLength = 3;   //SID + LengthFormatIdentifier + maxNumberOfBlockLength
    pduTxData->SduDataPtr[1] = 0x10;    //currently we set maxNumberOfBlock as 130(0x82), so it takes 1 bytes
    pduTxData->SduDataPtr[2] = (uint8)UDS_0x35_MAX_NUM_BLOCK_LEN;

    DsdDspProcessingDone(responseCode);
}

void DspUdsTransferData(const PduInfoType *pduRxData, PduInfoType *pduTxData)
{
	Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;

	pduTxData->SduLength = (uint8)UDS_0x35_MAX_NUM_BLOCK_LEN;
    pduTxData->SduDataPtr[1] = pduRxData->SduDataPtr[1];

    DsdDspProcessingDone(responseCode);
}

void DspUdsRequestTransferExit(const PduInfoType *pduRxData, PduInfoType *pduTxData)
{
	Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;
	(void)pduRxData;
    // Prepare positive response
    pduTxData->SduLength = 2;
    pduTxData->SduDataPtr[1] = 0x00;    //transferResponseParameterRecore, user-defined

    DsdDspProcessingDone(responseCode);
}
