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
 *  General requirements
 */
//lint -w2 Static code analysis warning level
/* Disable MISRA 2004 rule 16.2, MISRA 2012 rule 17.2.
 * This because of recursive calls to readDidData.
 *  */
//lint -estring(974,*recursive*)

/* @req DCM273 */ 
/* @req DCM272 */ 
/* @req DCM696 */
/* @req DCM039 */
/* @req DCM269 */
/* @req DCM271 */
/* @req DCM275 */
/* !req DCM038 Paged buffer not supported */
/* !req DCM085 */
/* @req DCM531 A jump to bootloader is possible only with services DiagnosticSessionControl and LinkControl services */
/* @req DCM527 At first call of an operation using the Dcm_OpStatusType, OpStatus should be DCM_INITIAL */
/* !req DCM528 E_FORCE_RCRRP not supported return for all operations */
/* !req DCM530 E_PENDING not supported return for all operations */
/* @req DCM077 When calling DEM for OBD services, DCM shall use the following values for the parameter DTCOrigin: Service $0A uses DEM_DTC_ORIGIN_PERMANENT_MEMORY All other services use DEM_DTC_ORIGIN_PRIMARY_MEMORY */
#include <string.h>
#include "Dcm.h"
#include "Dcm_Internal.h"
#if defined(DCM_USE_SERVICE_CLEARDIAGNOSTICINFORMATION) || defined(DCM_USE_SERVICE_READDTCINFORMATION) || defined(DCM_USE_SERVICE_CONTROLDTCSETTING)\
    || defined(DCM_USE_SERVICE_REQUESTPOWERTRAINFREEZEFRAMEDATA) || defined(DCM_USE_SERVICE_CLEAREMISSIONRELATEDDIAGNOSTICDATA)\
    || defined(DCM_USE_SERVICE_REQUESTEMISSIONRELATEDDIAGNOSTICTROUBLECODES) || defined(DCM_USE_SERVICE_REQUESTEMISSIONRELATEDDTCSDETECTEDDURINGCURRENTORLASTCOMPLETEDDRIVINGCYCLE)\
    || defined(DCM_USE_SERVICE_REQUESTEMISSIONRELATEDDTCSWITHPERMANENTSTATUS)
#if defined(USE_DEM)
#include "Dem.h"/* @req DCM332 */
#else
#warning "Dcm: UDS services 0x14, 0x18 and/or 0x85 will not work without Dem."
#warning "Dcm: OBD services $02, $03, $04, $07 and/or $0A will not work without Dem."
#endif
#endif
#include "MemMap.h"
#if defined(USE_MCU)
#include "Mcu.h"
#endif
#ifndef DCM_NOT_SERVICE_COMPONENT
#include "Rte_Dcm.h"
#endif

#if defined(USE_BSWM)
#include "BswM_DCM.h"
#endif

#if defined(DCM_USE_SERVICE_REQUESTTRANSFEREXIT) || defined(DCM_USE_SERVICE_TRANSFERDATA) || defined(DCM_USE_SERVICE_REQUESTDOWNLOAD) || defined(DCM_USE_SERVICE_REQUESTUPLOAD)
#define DCM_USE_UPLOAD_DOWNLOAD
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
#define SID_LEN 1u
#define SF_INDEX 1
#define SF_LEN 1
#define DTC_LEN 3
#define FF_REC_NUM_LEN 1

#define HALF_BYTE 							4
#define OFFSET_ONE_BYTE						8
#define OFFSET_TWO_BYTES 					16
#define OFFSET_THREE_BYTES					24

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
#define IS_VALID_IOCTRL_PARAM(_x) ((_x) <= DCM_SHORT_TERM_ADJUSTMENT)
#define TO_SIGNAL_BIT(_x) (uint8)(1u<<(7u-((_x)%8u)))

/*OBD RequestCurrentPowertrainDiagnosticData*/
#define PIDZERO								0
#define DATAZERO							0
#define INFOTYPE_ZERO						0
#define PID_LEN								1
#define RECORD_NUM_ZERO						0
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
#define IS_SUPPORTED_0x28_SUBFUNCTION(_x) ((_x) < 4u)
#define UDS_0x28_NOF_COM_TYPES 3u
#define UDS_0x28_NOF_SUB_FUNCTIONS 4u
#define IS_VALID_0x28_COM_TYPE(_x) (((_x) > 0u) && ((_x) < 4u))
/*OBD RequestCurrentPowertrainDiagnosticData*/
#define FF_NUM_LEN							1
#define OBD_DTC_LEN							2
#define OBD_SERVICE_TWO 					((uint8)0x02u)
#define MAX_PID_FFNUM_NUM					3
#define OBD_REQ_MESSAGE_LEN_TWO_MIN			3
#define DATA_ELEMENT_INDEX_OF_PID_NOT_SUPPORTED  0
#define OBD_SERVICE_2_PID_AND_FRAME_SIZE 2u
#define OBD_SERVICE_2_PID_INDEX 0u
#define OBD_SERVICE_2_FRAME_INDEX 1u

/*OBD RequestEmissionRelatedDiagnosticTroubleCodes service03 07*/
#define EMISSION_DTCS_HIGH_BYTE(dtc)		(((uint32)(dtc) >> 16) & 0xFFu)
#define EMISSION_DTCS_LOW_BYTE(dtc)			(((uint32)(dtc) >> 8) & 0xFFu)
#define OBD_RESPONSE_DTC_MAX_NUMS			126

/*OBD OnBoardMonitoringTestResultsSpecificMonitoredSystems service06*/
#define OBDMID_LEN							    1u
#define OBDMID_DATA_START_INDEX  			    1u
#define OBDM_TID_LEN						    1u
#define OBDM_UASID_LEN						    1u
#define OBDM_TESTRESULT_LEN					    6u
#define SUPPORTED_MAX_OBDMID_REQUEST_LEN   	    1u
#define SUPPORTED_OBDM_OUTPUT_LEN               (OBDM_TID_LEN + OBDM_UASID_LEN + OBDM_TESTRESULT_LEN)
#define SUPPORTED_OBDMIDS_DATA_LEN				4u
#define AVAIL_TO_SUPPORTED_OBDMID_OFFSET_MIN  	0x01
#define AVAIL_TO_SUPPORTED_OBDMID_OFFSET_MAX  	0x20
#define MAX_REQUEST_OBDMID_NUM 				    6u
#define IS_AVAILABILITY_OBDMID(_x)              ((0 == ((_x) % 0x20)) && ((_x) <= 0xE0))
#define OBDM_LSB_MASK				            0xFFu

/*OBD Requestvehicleinformation service09*/
#define OBD_TX_MAXLEN						0xFF
#define MAX_REQUEST_VEHINFO_NUM				6
#define OBD_SERVICE_FOUR 					0x04
#define OBD_VIN_LENGTH						17

#define IS_AVAILABILITY_PID(_x) ( (0 == ((_x) % 0x20)) && ((_x) <= 0xE0))
#define IS_AVAILABILITY_INFO_TYPE(_x) IS_AVAILABILITY_PID(_x)

#define BYTES_TO_DTC(hb, mb, lb)	(((uint32)(hb) << 16) | ((uint32)(mb) << 8) | (uint32)(lb))
#define DTC_HIGH_BYTE(dtc)			(((uint32)(dtc) >> 16) & 0xFFu)
#define DTC_MID_BYTE(dtc)			(((uint32)(dtc) >> 8) & 0xFFu)
#define DTC_LOW_BYTE(dtc)			((uint32)(dtc) & 0xFFu)

/* UDS ReadDataByPeriodicIdentifier */
#define TO_PERIODIC_DID(_x) (DCM_PERODICDID_HIGH_MASK + (uint16)(_x))
/* Maximum length for periodic Dids */
#define MAX_PDID_DATA_SIZE 7
/* CAN */
#define MAX_TYPE2_PERIODIC_DID_LEN_CAN 7
#define MAX_TYPE1_PERIODIC_DID_LEN_CAN 5

/* Flexray */
/* IMPROVEMENT: Maximum length for flexray? */
#define MAX_TYPE2_PERIODIC_DID_LEN_FLEXRAY 0
#define MAX_TYPE1_PERIODIC_DID_LEN_FLEXRAY 0

/* Ip */
/* IMPROVEMENT: Maximum length for ip? */
#define MAX_TYPE2_PERIODIC_DID_LEN_IP 0
#define MAX_TYPE1_PERIODIC_DID_LEN_IP 0

#define TIMER_DECREMENT(timer) \
        if (timer >= DCM_MAIN_FUNCTION_PERIOD_TIME_MS) { \
            timer = timer - DCM_MAIN_FUNCTION_PERIOD_TIME_MS; \
        } \

/* UDS Linkcontrol */
#define LINKCONTROL_SUBFUNC_VERIFY_BAUDRATE_TRANS_WITH_FIXED_BAUDRATE    0x01
#define LINKCONTROL_SUBFUNC_VERIFY_BAUDRATE_TRANS_WITH_SPECIFIC_BAUDRATE 0x02
#define LINKCONTROL_SUBFUNC_TRANSITION_BAUDRATE 0x03


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

typedef enum {
    DCM_JTB_IDLE,
    DCM_JTB_WAIT_RESPONSE_PENDING_TX_CONFIRM,
    DCM_JTB_EXECUTE,
    DCM_JTB_RESAPP_FINAL_RESPONSE_TX_CONFIRM,
    DCM_JTB_RESAPP_WAIT_RESPONSE_PENDING_TX_CONFIRM,
    DCM_JTB_RESAPP_ASSEMBLE_FINAL_RESPONSE
}DspJumpToBootState;

typedef struct {
    boolean pendingSessionChange;
    PduIdType sessionPduId;
    Dcm_SesCtrlType session;
    DspJumpToBootState jumpToBootState;
    const PduInfoType* pduRxData;
    PduInfoType* pduTxData;
    uint16 P2;
    uint16 P2Star;
    Dcm_ProtocolType protocolId;
} DspUdsSessionControlDataType;

typedef struct {
    PduIdType confirmPduId;
    DspJumpToBootState jumpToBootState;
    const PduInfoType* pduRxData;
    PduInfoType* pduTxData;
} DspUdsLinkControlDataType;

typedef struct {
    ReadDidPendingStateType state;
    const PduInfoType* pduRxData;
    PduInfoType* pduTxData;
    uint16 txWritePos;
    uint16 nofReadDids;
    uint16 reqDidIndex;
    uint16 pendingDid;
    uint16 pendingDataLength;
    uint16 pendingSignalIndex;
    uint16 pendingDataStartPos;
} DspUdsReadDidPendingType;

typedef enum {
    DCM_GENERAL_IDLE,
    DCM_GENERAL_PENDING,
    DCM_GENERAL_FORCE_RCRRP_AWAITING_SEND,
    DCM_GENERAL_FORCE_RCRRP,
} GeneralPendingStateType;

typedef struct {
    GeneralPendingStateType state;
    const PduInfoType* pduRxData;
    PduInfoType* pduTxData;
    uint8 pendingService;
} DspUdsGeneralPendingType;

typedef struct {
    boolean comControlPending;
    uint8 subFunction;
    uint8 subnet;
    uint8 comType;
    PduIdType confirmPdu;
    PduIdType rxPdu;
} DspUdsCommunicationControlDataType;

static DspUdsEcuResetDataType dspUdsEcuResetData;
static DspUdsSessionControlDataType dspUdsSessionControlData;
#if defined(DCM_USE_SERVICE_LINKCONTROL)
static DspUdsLinkControlDataType dspUdsLinkControlData;
#endif
static DspUdsReadDidPendingType dspUdsReadDidPending;
#ifdef DCM_USE_SERVICE_WRITEDATABYIDENTIFIER
static DspUdsGeneralPendingType dspUdsWriteDidPending;
#endif
static DspUdsGeneralPendingType dspUdsRoutineControlPending;
static DspUdsGeneralPendingType dspUdsSecurityAccessPending;
#if defined(DCM_USE_UPLOAD_DOWNLOAD)
static DspUdsGeneralPendingType dspUdsUploadDownloadPending;
#endif
#ifdef DCM_USE_SERVICE_COMMUNICATIONCONTROL
static DspUdsCommunicationControlDataType communicationControlData;
#endif


typedef enum {
    DELAY_TIMER_DEACTIVE,
    DELAY_TIMER_ON_BOOT_ACTIVE,
    DELAY_TIMER_ON_EXCEEDING_LIMIT_ACTIVE
}DelayTimerActivation;

typedef struct {
    uint8                           secAcssAttempts; //Counter for number of false attempts
    uint32                          timerSecAcssAttempt; //Timer after exceededNumberOfAttempts
    DelayTimerActivation            startDelayTimer; //Flag to indicate Delay timer is active
}DspUdsSecurityAccessChkParam;

typedef struct {
    boolean 						reqInProgress;
    Dcm_SecLevelType				reqSecLevel;
#if (DCM_SECURITY_EOL_INDEX != 0)
    DspUdsSecurityAccessChkParam    secFalseAttemptChk[DCM_SECURITY_EOL_INDEX];
    uint8                           currSecLevIdx; //Current index for secFalseAttemptChk
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

Dcm_DspMemoryStateType dspMemoryState;

typedef enum{
    DCM_DDD_SOURCE_DEFAULT,
    DCM_DDD_SOURCE_DID,
    DCM_DDD_SOURCE_ADDRESS
}Dcm_DspDDDSourceKindType;

typedef enum {
    PDID_ADDED = 0,
    PDID_UPDATED,
    PDID_BUFFER_FULL
}PdidEntryStatusType;

#if (DCM_PERIODIC_DID_SYNCH_SAMPLING == STD_ON)
typedef enum {
    PDID_NOT_SAMPLED = 0,
    PDID_SAMPLED_OK,
    PDID_SAMPLED_FAILED
}PdidSampleStatus;
#endif

typedef struct{
    uint32 PDidTxCounter;
    uint32 PDidTxPeriod;
    PduIdType PDidRxPduID;
    uint8 PeriodicDid;
#if (DCM_PERIODIC_DID_SYNCH_SAMPLING == STD_ON)
    uint8 PdidData[MAX_PDID_DATA_SIZE];
    uint8 PdidLength;
    PdidSampleStatus PdidSampleStatus;
#endif
}Dcm_pDidType;/* a type to save  the periodic DID and cycle */

typedef struct{
    Dcm_pDidType dspPDid[DCM_LIMITNUMBER_PERIODDATA];	/*a buffer to save the periodic DID and cycle   */
    uint8 PDidNofUsed;										/* note the number of periodic DID is used */
    uint8 nextStartIndex;
}Dsp_pDidRefType;

#if defined(DCM_USE_SERVICE_READDATABYPERIODICIDENTIFIER)
static Dsp_pDidRefType dspPDidRef;
#endif

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

#if defined(DCM_USE_SERVICE_INPUTOUTPUTCONTROLBYIDENTIFIER) && defined(DCM_USE_CONTROL_DIDS)
typedef uint8 Dcm_DspIOControlVector[(DCM_MAX_IOCONTROL_DID_SIGNALS + 7) / 8];
typedef struct {
    uint16 did;
    boolean controlActive;
    Dcm_DspIOControlVector activeSignalBitfield;
}Dcm_DspIOControlStateType;
static Dcm_DspIOControlStateType IOControlStateList[DCM_NOF_IOCONTROL_DIDS];

typedef struct {
    const PduInfoType* pduRxData;
    PduInfoType* pduTxData;
    uint16 pendingSignalIndex;
    boolean pendingControl;
    GeneralPendingStateType state;
    Dcm_DspIOControlVector signalAffected;
    boolean controlActivated;
}DspUdsIOControlPendingType;
static DspUdsIOControlPendingType IOControlData;
#endif

#if defined(USE_DEM) && defined(DCM_USE_SERVICE_CONTROLDTCSETTING)
typedef struct {
    Dem_DTCGroupType dtcGroup;
    Dem_DTCKindType dtcKind;
    boolean settingDisabled;
}Dcm_DspControlDTCSettingsType;
static Dcm_DspControlDTCSettingsType DspDTCSetting;
#endif

#if defined(DCM_USE_UPLOAD_DOWNLOAD)
typedef enum {
    DCM_NO_DATA_TRANSFER,
    DCM_UPLOAD,
    DCM_DOWNLOAD
}DcmDataTransferType;

typedef struct {
    uint32 nextAddress;
    uint8 blockSequenceCounter;
    DcmDataTransferType transferType;
    boolean firstBlockReceived;
    uint32 uplBytesLeft; /* Bytes left to be uploaded */
    uint32 uplMemBlockSize; /* block length is maxNumberOfBlockLength (ISO14229) minus SID and lengthFormatIdentifier */
}DcmTransferStatusType;

static DcmTransferStatusType TransferStatus;
#endif

static Dcm_ProgConditionsType GlobalProgConditions;

static GeneralPendingStateType ProgConditionStartupResponseState;

static boolean ProtocolStartRequested = FALSE;

#ifdef DCM_USE_SERVICE_COMMUNICATIONCONTROL
static Dcm_CommunicationModeType DspChannelComMode[DCM_NOF_COMM_CHANNELS];
#endif
/*
 * * static Function
 */
#ifndef DCM_USE_SERVICE_DYNAMICALLYDEFINEDATAIDENTIFIER
#define LookupDDD(_x,  _y ) FALSE
#define readDDDData(_x, _y, _z, _v) DCM_E_GENERALREJECT
#else
static boolean LookupDDD(uint16 didNr, const Dcm_DspDDDType **DDid);
#endif
static Dcm_NegativeResponseCodeType checkAddressRange(DspMemoryServiceType serviceType, uint8 memoryIdentifier, uint32 memoryAddress, uint32 length);
static const Dcm_DspMemoryRangeInfo* findRange(const Dcm_DspMemoryRangeInfo *memoryRangePtr, uint32 memoryAddress, uint32 length);
static Dcm_NegativeResponseCodeType writeMemoryData(Dcm_OpStatusType* OpStatus, uint8 memoryIdentifier, uint32 MemoryAddress, uint32 MemorySize, uint8 *SourceData);
static void DspCancelPendingDid(uint16 didNr, uint16 signalIndex, ReadDidPendingStateType pendingState, PduInfoType *pduTxData );
static void DspCancelPendingRoutine(const PduInfoType *pduRxData, PduInfoType *pduTxData);
static void DspCancelPendingSecurityAccess(const PduInfoType *pduRxData, PduInfoType *pduTxData);
static Dcm_NegativeResponseCodeType DspUdsSecurityAccessCompareKeySubFnc (const PduInfoType *pduRxData, PduInfoType *pduTxData, Dcm_SecLevelType requestedSecurityLevel, boolean isCancel);
static Dcm_NegativeResponseCodeType DspUdsSecurityAccessGetSeedSubFnc (const PduInfoType *pduRxData, PduInfoType *pduTxData, Dcm_SecLevelType requestedSecurityLevel, boolean isCancel);
#if defined(DCM_USE_UPLOAD_DOWNLOAD)
static void DspCancelPendingUploadDownload(uint8 SID);
#endif

#if defined(DCM_USE_SERVICE_INPUTOUTPUTCONTROLBYIDENTIFIER) && defined(DCM_USE_CONTROL_DIDS)
static void DspStopInputOutputControl(boolean checkSessionAndSecLevel);
static void DspIOControlStopActivated(uint16 didNr, Dcm_DspIOControlVector signalActivated);
static Std_ReturnType FunctionInputOutputControl(const Dcm_DspDataType *DataPtr, Dcm_IOControlParameterType action, Dcm_OpStatusType opStatus, uint8* controlOptionRecord, Dcm_NegativeResponseCodeType* responseCode);
#endif
#ifdef DCM_USE_SERVICE_READDATABYPERIODICIDENTIFIER
static boolean checkPDidSupported(uint16 pDid, uint16 *didLength, Dcm_NegativeResponseCodeType *responseCode);
static void DspPdidRemove(uint8 pDid, PduIdType rxPduId);
static void DspStopPeriodicDids(boolean checkSessionAndSecLevel);
static Dcm_NegativeResponseCodeType getPDidData(uint16 did, uint8 *data, uint16 bufSize, uint16 *dataLength);
#if (DCM_PERIODIC_DID_SYNCH_SAMPLING == STD_ON)
static void DspSamplePDids(uint32 period, PduIdType rxPduId);
#endif
#endif

#if defined(USE_DEM) && defined(DCM_USE_SERVICE_CONTROLDTCSETTING)
static void DspEnableDTCSetting(void);
#endif

static void SetOpStatusDependingOnState(DspUdsGeneralPendingType *pGeneralPending,Dcm_OpStatusType *opStatus, boolean isCancel);
#ifdef DCM_USE_SERVICE_COMMUNICATIONCONTROL
static Dcm_NegativeResponseCodeType DspInternalCommunicationControl(uint8 subFunction, uint8 subNet, uint8 comType, PduIdType rxPduId, boolean executeModeSwitch);
static void DspStopCommunicationControl(boolean checkSessionAndSecLevel);
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
#if defined(DCM_USE_SERVICE_INPUTOUTPUTCONTROLBYIDENTIFIER) && defined(DCM_USE_CONTROL_DIDS)
    /* DCM628 says that active control should be stopped on transition
     * to default session only. But stop it if active control is not
     * supported in the new session (which should be the current session
     * as it is assumed that session is changed before calling this function) or
     * in the new security level. */
    DspStopInputOutputControl(TRUE);
    IOControlData.state = DCM_GENERAL_IDLE;
#endif
#ifdef DCM_USE_SERVICE_READDATABYPERIODICIDENTIFIER
    DspStopPeriodicDids(TRUE);
#endif

#if defined(DCM_USE_SERVICE_RESPONSEONEVENT) && (DCM_ROE_INIT_ON_DSC == STD_ON)
    /* Stop the response on event service */
    /* NOTE: ROE actually already stopped it this function
     * is called due to session change on service request.
     * But stop it here since this function is called when
     * resetting to default session due to s3 timeout. */
    /* @req DCM618 */
    if (DCM_ROE_IsActive()) {
        (void)Dcm_StopROE();
    }
#endif
#if defined(USE_DEM) && defined(DCM_USE_SERVICE_CONTROLDTCSETTING)
    DspEnableDTCSetting();
#endif
#ifdef DCM_USE_SERVICE_COMMUNICATIONCONTROL
    DspStopCommunicationControl(TRUE);
#endif
    dspUdsSessionControlData.jumpToBootState = DCM_JTB_IDLE;
    dspUdsSessionControlData.pendingSessionChange = FALSE;
    ProgConditionStartupResponseState = DCM_GENERAL_IDLE;
}
/* Resets active diagnostics on protocol preemtion */
void DcmDspResetDiagnosticActivity(void)
{
#if defined(DCM_USE_SERVICE_INPUTOUTPUTCONTROLBYIDENTIFIER) && defined(DCM_USE_CONTROL_DIDS)
    DspStopInputOutputControl(FALSE);
    IOControlData.state = DCM_GENERAL_IDLE;
#endif
#ifdef DCM_USE_SERVICE_READDATABYPERIODICIDENTIFIER
    DspStopPeriodicDids(FALSE);
#endif
#if defined(DCM_USE_SERVICE_RESPONSEONEVENT) && (DCM_ROE_INIT_ON_DSC == STD_ON)
    /* NOTE: Actually not mentioned in spec. that ROE should be stopped
     * on protocol preemption. But it is reasonable to do it. */
    /* Stop the response on event service */
    if (DCM_ROE_IsActive()) {
        (void)Dcm_StopROE();
    }
#endif
#if defined(USE_DEM) && defined(DCM_USE_SERVICE_CONTROLDTCSETTING)
    DspEnableDTCSetting();
#endif
#ifdef DCM_USE_SERVICE_COMMUNICATIONCONTROL
    DspStopCommunicationControl(FALSE);
#endif
    dspUdsSessionControlData.jumpToBootState = DCM_JTB_IDLE;
    dspUdsSessionControlData.pendingSessionChange = FALSE;
    ProgConditionStartupResponseState = DCM_GENERAL_IDLE;
}

#ifdef DCM_USE_SERVICE_READDATABYPERIODICIDENTIFIER
typedef struct {
    PduIdType rxPduId;
    uint8 pDid;
}pDidType;

/**
 * Stops periodic Dids not supported in current session or security level
 * @param checkSessionAndSecLevel
 */
static void DspStopPeriodicDids(boolean checkSessionAndSecLevel)
{
    uint16 didLength;
    Dcm_NegativeResponseCodeType resp;
    pDidType pDidsToRemove[DCM_LIMITNUMBER_PERIODDATA];
    uint8 nofPDidsToRemove = 0;
    memset(pDidsToRemove, 0, sizeof(pDidsToRemove));
    if( checkSessionAndSecLevel && DsdDspCheckServiceSupportedInActiveSessionAndSecurity(SID_READ_DATA_BY_PERIODIC_IDENTIFIER) ) {
        for(uint8 i = 0; i < dspPDidRef.PDidNofUsed; i++) {
            resp = DCM_E_REQUESTOUTOFRANGE;
            if( !(checkPDidSupported(TO_PERIODIC_DID(dspPDidRef.dspPDid[i].PeriodicDid), &didLength, &resp) && (DCM_E_POSITIVERESPONSE == resp)) ) {
                /* Not supported */
                pDidsToRemove[nofPDidsToRemove].pDid = dspPDidRef.dspPDid[i].PeriodicDid;
                pDidsToRemove[nofPDidsToRemove++].rxPduId = dspPDidRef.dspPDid[i].PDidRxPduID;
            }
        }
        for( uint8 i = 0; i < nofPDidsToRemove; i++ ) {
            DspPdidRemove(pDidsToRemove[i].pDid, pDidsToRemove[i].rxPduId);
        }
    } else {
        /* Should not check session and security or service not supported in the current session or security.
         * Clear all. */
        memset(&dspPDidRef,0,sizeof(dspPDidRef));
    }
}
#endif

#ifdef DCM_USE_SERVICE_COMMUNICATIONCONTROL
static void DspStopCommunicationControl(boolean checkSessionAndSecLevel)
{
    const Dcm_ComMChannelConfigType * comMChannelCfg = Dcm_ConfigPtr->DcmComMChannelCfg;
    if( (FALSE == checkSessionAndSecLevel) || (FALSE == DsdDspCheckServiceSupportedInActiveSessionAndSecurity(SID_COMMUNICATION_CONTROL)) ) {
        while( TRUE != comMChannelCfg->Arc_EOL ) {
            if( DCM_ENABLE_RX_TX_NORM_NM != DspChannelComMode[comMChannelCfg->InternalIndex] ) {
                (void)comMChannelCfg->ModeSwitchFnc(RTE_MODE_DcmCommunicationControl_DCM_ENABLE_RX_TX_NORM_NM);
#if defined(USE_BSWM)
                BswM_Dcm_CommunicationMode_CurrentState(comMChannelCfg->NetworkHandle, DCM_ENABLE_RX_TX_NORM_NM);
#endif
                DspChannelComMode[comMChannelCfg->InternalIndex] = DCM_ENABLE_RX_TX_NORM_NM;
            }
            comMChannelCfg++;
        }
    }
}
#endif
/**
 * Init function for Dsp
 * @param firstCall
 */
void DspInit(boolean firstCall)
{
    dspUdsSecurityAccesData.reqInProgress = FALSE;
    dspUdsEcuResetData.resetPending = DCM_DSP_RESET_NO_RESET;
    dspUdsSessionControlData.pendingSessionChange = FALSE;
#if defined(DCM_USE_SERVICE_LINKCONTROL)
    dspUdsLinkControlData.jumpToBootState = DCM_JTB_IDLE;
#endif

#if defined(USE_DEM) && defined(DCM_USE_SERVICE_CONTROLDTCSETTING)
    if( firstCall ) {
        DspDTCSetting.settingDisabled = FALSE;
    }
#endif
    dspMemoryState = DCM_MEMORY_UNUSED;
    /* clear periodic send buffer */
#if defined(DCM_USE_SERVICE_READDATABYPERIODICIDENTIFIER)
    memset(&dspPDidRef,0,sizeof(dspPDidRef));
#endif
#ifdef DCM_USE_CONTROL_DIDS
    if(firstCall) {
        memset(IOControlStateList, 0, sizeof(IOControlStateList));
        IOControlData.state = DCM_GENERAL_IDLE;
    }
#endif
#ifdef DCM_USE_SERVICE_DYNAMICALLYDEFINEDATAIDENTIFIER
    /* clear dynamically Did buffer */
    memset(&dspDDD[0],0,sizeof(dspDDD));
#endif

#if (DCM_SECURITY_EOL_INDEX != 0)
    uint8 temp = 0;
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
#if defined(DCM_USE_UPLOAD_DOWNLOAD)
    TransferStatus.blockSequenceCounter = 0;
    TransferStatus.firstBlockReceived = FALSE;
    TransferStatus.transferType = DCM_NO_DATA_TRANSFER;
    dspUdsUploadDownloadPending.state = DCM_GENERAL_IDLE;
#endif
    dspUdsSessionControlData.jumpToBootState = DCM_JTB_IDLE;
    ProgConditionStartupResponseState = DCM_GENERAL_IDLE;
#ifdef DCM_USE_SERVICE_COMMUNICATIONCONTROL
    communicationControlData.comControlPending = FALSE;
    if (firstCall) {
        memset(DspChannelComMode, DCM_ENABLE_RX_TX_NORM_NM, sizeof(DspChannelComMode));
    }
#endif
}

/**
 * Function called first main function after init to allow Dsp to request
 * protocol start request
 */
void DspCheckProtocolStartRequests(void)
{
    /* @req DCM536 */
    if(DCM_WARM_START == Dcm_GetProgConditions(&GlobalProgConditions)) {
        /* Jump from bootloader */
#if 0
        /* !req DCM768 */
#if defined(USE_BSWM)
        if( progConditions.ApplUpdated ) {
            BswM_Dcm_ApplicationUpdated();
        }
#endif
#endif
        GlobalProgConditions.ApplUpdated = FALSE;
        if( (SID_DIAGNOSTIC_SESSION_CONTROL == GlobalProgConditions.Sid) || (SID_ECU_RESET == GlobalProgConditions.Sid)) {
            uint8 session = (SID_DIAGNOSTIC_SESSION_CONTROL == GlobalProgConditions.Sid) ? GlobalProgConditions.SubFncId : DCM_DEFAULT_SESSION;
            if( E_OK == DcmRequestStartProtocol(DCM_REQ_DSP, session,
                    GlobalProgConditions.ProtocolId, GlobalProgConditions.TesterSourceAdd, GlobalProgConditions.ResponseRequired) ) {
                ProtocolStartRequested = TRUE;
            } else {
                DCM_DET_REPORTERROR(DCM_GLOBAL_ID, DCM_E_UNEXPECTED_RESPONSE);
            }
        }
    }
}

/**
 * Protocol start notification
 * @param started
 */
void DcmDspProtocolStartNotification(boolean started)
{
    if( ProtocolStartRequested ) {
        if(started) {
            if( GlobalProgConditions.ResponseRequired ) {
                /* Wait until full communication has been indicated, then send a response to service */
                ProgConditionStartupResponseState = DCM_GENERAL_PENDING;
            }
        } else {
            DCM_DET_REPORTERROR(DCM_GLOBAL_ID, DCM_E_UNEXPECTED_RESPONSE);
        }
    } else {
        /* Have not requested a start.. */
        DCM_DET_REPORTERROR(DCM_GLOBAL_ID, DCM_E_UNEXPECTED_EXECUTION);
    }
}

/**
 * Get the rte mode corresponding to a reset type
 * @param resetType
 * @param rteMode
 * @return E_OK: rte mode found, E_NOT_OK: rte mode not found
 */
static Std_ReturnType getResetRteMode(Dcm_EcuResetType resetType, uint8 *rteMode)
{
    Std_ReturnType ret = E_OK;
    switch(resetType) {
        case DCM_HARD_RESET:
            *rteMode = RTE_MODE_DcmEcuReset_HARD;
            break;
        case DCM_KEY_OFF_ON_RESET:
            *rteMode = RTE_MODE_DcmEcuReset_KEYONOFF;
            break;
        case DCM_SOFT_RESET:
            *rteMode = RTE_MODE_DcmEcuReset_SOFT;
            break;
        default:
            ret = E_NOT_OK;
            break;
    }
    return ret;
}

/**
 * Main function for executing ECU reset
 */
void DspResetMainFunction(void)
{
    Std_ReturnType result = E_NOT_OK;
    uint8 rteMode;
    if( (DCM_DSP_RESET_PENDING == dspUdsEcuResetData.resetPending) && (E_OK == getResetRteMode(dspUdsEcuResetData.resetType, &rteMode)) ) {
        /* IMPROVEMENT: Should be a call to SchM */
        result = Rte_Switch_DcmEcuReset_DcmEcuReset(rteMode);

        switch( result ) {
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

#if defined(DCM_USE_SERVICE_READMEMORYBYADDRESS) || defined(DCM_USE_SERVICE_WRITEMEMORYBYADDRESS)
/**
 * Main function for write/read memory
 */
void DspMemoryMainFunction(void)
{
    /* IMPROVEMENT: DCM_WRITE_FORCE_RCRRP */
    Dcm_ReturnWriteMemoryType WriteRet;
    Dcm_ReturnReadMemoryType ReadRet;
    switch(dspMemoryState)
    {
        case DCM_MEMORY_UNUSED:
            break;
        case DCM_MEMORY_READ:
            ReadRet = Dcm_ReadMemory(DCM_PENDING,0,0,0,0);
            if(ReadRet == DCM_READ_OK) {/*asynchronous writing is ok*/
                DsdDspProcessingDone(DCM_E_POSITIVERESPONSE);
                dspMemoryState = DCM_MEMORY_UNUSED;
            } else if(ReadRet == DCM_READ_FAILED) {
                /* @req DCM644 */
                DsdDspProcessingDone(DCM_E_GENERALPROGRAMMINGFAILURE);
                dspMemoryState = DCM_MEMORY_UNUSED;
            }
            break;
        case DCM_MEMORY_WRITE:
            WriteRet = Dcm_WriteMemory(DCM_PENDING,0,0,0,0);
            if(WriteRet == DCM_WRITE_OK) {/*asynchronous writing is ok*/
                DsdDspProcessingDone(DCM_E_POSITIVERESPONSE);
                dspMemoryState = DCM_MEMORY_UNUSED;
            } else if(WriteRet == DCM_WRITE_FAILED) {
                /* @req DCM643 */
                DsdDspProcessingDone(DCM_E_GENERALPROGRAMMINGFAILURE);
                dspMemoryState = DCM_MEMORY_UNUSED;
            }
            break;

        default:
            break;
            
    }
}
#endif

#if defined(DCM_USE_SERVICE_READDATABYPERIODICIDENTIFIER)
/**
 * Main function for reading periodic DIDs
 */
void DspPeriodicDIDMainFunction()
{
    boolean sentResponseThisLoop = FALSE;
    uint8 pDidIndex = dspPDidRef.nextStartIndex;
    if( 0 != dspPDidRef.PDidNofUsed ) {
        dspPDidRef.nextStartIndex %= dspPDidRef.PDidNofUsed;
    }
    for(uint8 i = 0; i < dspPDidRef.PDidNofUsed; i++) {
        if(dspPDidRef.dspPDid[pDidIndex].PDidTxPeriod > dspPDidRef.dspPDid[pDidIndex].PDidTxCounter) {
            dspPDidRef.dspPDid[pDidIndex].PDidTxCounter++;
        }
        if( dspPDidRef.dspPDid[pDidIndex].PDidTxPeriod <= dspPDidRef.dspPDid[pDidIndex].PDidTxCounter ) {
            if( sentResponseThisLoop  == FALSE ) {
#if (DCM_PERIODIC_DID_SYNCH_SAMPLING == STD_ON)
                if( PDID_NOT_SAMPLED == dspPDidRef.dspPDid[pDidIndex].PdidSampleStatus ) {
                    DspSamplePDids(dspPDidRef.dspPDid[pDidIndex].PDidTxPeriod, dspPDidRef.dspPDid[pDidIndex].PDidRxPduID);
                }
#endif
                if (E_OK == DslInternal_ResponseOnOneDataByPeriodicId(dspPDidRef.dspPDid[pDidIndex].PeriodicDid, dspPDidRef.dspPDid[pDidIndex].PDidRxPduID)){
                    dspPDidRef.dspPDid[pDidIndex].PDidTxCounter = 0;
                    /*AutoSar  DCM  8.10.5 */
                    sentResponseThisLoop = TRUE;
                    dspPDidRef.nextStartIndex = (pDidIndex + 1) % dspPDidRef.PDidNofUsed;
                }
            } else {
                /* Don't do anything - PDid will be sent next loop */
            }
        }
        pDidIndex++;
        pDidIndex %= dspPDidRef.PDidNofUsed;
    }
}
#endif

/**
 * Main function for reading DIDs
 */
void DspReadDidMainFunction(void) {
    if( DCM_READ_DID_IDLE != dspUdsReadDidPending.state ) {
        DspUdsReadDataByIdentifier(dspUdsReadDidPending.pduRxData, dspUdsReadDidPending.pduTxData);
    }
#ifdef DCM_USE_SERVICE_WRITEDATABYIDENTIFIER
    if( DCM_GENERAL_PENDING == dspUdsWriteDidPending.state ) {
        DspUdsWriteDataByIdentifier(dspUdsWriteDidPending.pduRxData, dspUdsWriteDidPending.pduTxData);
    }
#endif
}

/**
 * Main function for routine control
 */
void DspRoutineControlMainFunction(void) {
    if( DCM_GENERAL_FORCE_RCRRP_AWAITING_SEND == dspUdsRoutineControlPending.state){
        /* !req DCM528 */
        /* !req DCM529 Should wait until transmit has been confirmed */
        dspUdsRoutineControlPending.state = DCM_GENERAL_FORCE_RCRRP; // Do not try again until next main loop
    }
    else if( (DCM_GENERAL_PENDING == dspUdsRoutineControlPending.state) || (DCM_GENERAL_FORCE_RCRRP == dspUdsRoutineControlPending.state)) {
        DspUdsRoutineControl(dspUdsRoutineControlPending.pduRxData, dspUdsRoutineControlPending.pduTxData);
    }
}

/**
 * Main function for security access
 */
void DspSecurityAccessMainFunction(void) {
    if( DCM_GENERAL_PENDING == dspUdsSecurityAccessPending.state ) {
         DspUdsSecurityAccess(dspUdsSecurityAccessPending.pduRxData, dspUdsSecurityAccessPending.pduTxData);
    }
}

#if defined(DCM_USE_SERVICE_INPUTOUTPUTCONTROLBYIDENTIFIER) && defined(DCM_USE_CONTROL_DIDS)
/**
 * Main function for IO control
 */
void DspIOControlMainFunction(void) {
    if( DCM_GENERAL_PENDING == IOControlData.state ) {
        DspIOControlByDataIdentifier(IOControlData.pduRxData, IOControlData.pduTxData);
    }
}

/**
 * Function for canceling a pending IOControl
 * @param pduRxData
 * @param pduTxData
 */
static void DspCancelPendingIOControlByDataIdentifier(const PduInfoType *pduRxData, PduInfoType *pduTxData )
{
    Dcm_NegativeResponseCodeType responseCode;
    const Dcm_DspDidType *didPtr = NULL_PTR;
    uint16 didNr = (pduRxData->SduDataPtr[IOI_INDEX] << 8 & DCM_DID_HIGH_MASK) + (pduRxData->SduDataPtr[IOI_INDEX+1] & DCM_DID_LOW_MASK);
    if( lookupNonDynamicDid(didNr, &didPtr) ) {
        /* First of all return control to ECU on the ones already a activated */
        DspIOControlStopActivated(didNr, IOControlData.signalAffected);
        /* And cancel the pending one. */
        if( IOControlData.pendingSignalIndex < didPtr->DspNofSignals ) {
            const Dcm_DspDataType *dataPtr = didPtr->DspSignalRef[IOControlData.pendingSignalIndex].DspSignalDataRef;
            if( DCM_GENERAL_PENDING == IOControlData.state ) {
                if( TRUE == IOControlData.pendingControl ) {
                    /* Pending control */
                    if ( dataPtr->DspDataUsePort == DATA_PORT_ASYNCH) {
                        (void)FunctionInputOutputControl(dataPtr, pduRxData->SduDataPtr[IOCP_INDEX], DCM_CANCEL, NULL_PTR, &responseCode);
                    }
                }
                else {
                    /* It is a pending read. */
                    if( dataPtr->DspDataReadDataFnc.AsynchDataReadFnc != NULL_PTR ) {
                        if( DATA_PORT_ASYNCH == dataPtr->DspDataUsePort ) {
                            (void)dataPtr->DspDataReadDataFnc.AsynchDataReadFnc(DCM_CANCEL, NULL_PTR);
                        }
                    }
                }
            } else {
                /* Not in a pending state */
                DCM_DET_REPORTERROR(DCM_GLOBAL_ID, DCM_E_UNEXPECTED_EXECUTION);
            }
        } else {
            /* Invalid signal index */
            DCM_DET_REPORTERROR(DCM_GLOBAL_ID, DCM_E_UNEXPECTED_EXECUTION);
        }
    }
}
#endif

#ifdef DCM_USE_UPLOAD_DOWNLOAD
/**
 * Cancels pending upload/download service
 * @param SID
 */
static void DspCancelPendingUploadDownload(uint8 SID)
{
    Dcm_NegativeResponseCodeType respCode;
    uint32 blockSize;
    switch(SID) {
        case SID_REQUEST_DOWNLOAD:
            (void)Dcm_ProcessRequestDownload(DCM_CANCEL, 0, 0, 0, &blockSize, &respCode);
            break;
        case SID_REQUEST_UPLOAD:
            (void)Dcm_ProcessRequestUpload(DCM_CANCEL, 0, 0, 0, &respCode);
            break;
        case SID_TRANSFER_DATA:
            if (DCM_DOWNLOAD == TransferStatus.transferType) {
                (void)Dcm_WriteMemory(DCM_CANCEL, 0, 0, 0, 0);
            }

            if (DCM_UPLOAD == TransferStatus.transferType) {
                (void)Dcm_ReadMemory(DCM_CANCEL, 0, 0, 0, 0);
            }

            break;
        case SID_REQUEST_TRANSFER_EXIT:
            (void)Dcm_ProcessRequestTransferExit(DCM_CANCEL, 0, 0, &respCode);
            break;
        default:
            break;
    }
}

/**
 * Runs pending upload/download service
 * @param SID
 * @param pduRxData
 * @param pduTxData
 */
static void DspUploadDownload(uint8 SID, const PduInfoType *pduRxData, PduInfoType *pduTxData)
{
    switch(SID) {
#if defined(DCM_USE_SERVICE_REQUESTDOWNLOAD)
        case SID_REQUEST_DOWNLOAD:
            DspUdsRequestDownload(pduRxData, pduTxData);
            break;
#endif
#if defined(DCM_USE_SERVICE_REQUESTUPLOAD)
        case SID_REQUEST_UPLOAD:
            DspUdsRequestUpload(pduRxData, pduTxData);
            break;
#endif
#if defined(DCM_USE_SERVICE_TRANSFERDATA)
        case SID_TRANSFER_DATA:
            DspUdsTransferData(pduRxData, pduTxData);
            break;
#endif
#if defined(DCM_USE_SERVICE_REQUESTTRANSFEREXIT)
        case SID_REQUEST_TRANSFER_EXIT:
            DspUdsRequestTransferExit(pduRxData, pduTxData);
            break;
#endif
        default:
            break;
    }
}

/**
 * Main function for pending upload/download services
 */
void DspUploadDownloadMainFunction(void) {
    if( DCM_GENERAL_FORCE_RCRRP_AWAITING_SEND == dspUdsUploadDownloadPending.state) {
        /* !req DCM528 */
        /* !req DCM529 Should wait until transmit has been confirmed */
        dspUdsUploadDownloadPending.state = DCM_GENERAL_FORCE_RCRRP; // Do not try again until next main loop
    }
    else if( (DCM_GENERAL_PENDING == dspUdsUploadDownloadPending.state) || (DCM_GENERAL_FORCE_RCRRP == dspUdsUploadDownloadPending.state)) {
        DspUploadDownload(dspUdsUploadDownloadPending.pendingService, dspUdsUploadDownloadPending.pduRxData, dspUdsUploadDownloadPending.pduTxData);
    }
}
#endif

#if (DCM_USE_JUMP_TO_BOOT == STD_ON) || defined(DCM_USE_SERVICE_LINKCONTROL)
/**
 * Main function for jump to boot
 */
void DspJumpToBootMainFunction(void) {
    if( (DCM_JTB_RESAPP_ASSEMBLE_FINAL_RESPONSE == dspUdsSessionControlData.jumpToBootState) || ( DCM_JTB_EXECUTE == dspUdsSessionControlData.jumpToBootState )) {
         DspUdsDiagnosticSessionControl(dspUdsSessionControlData.pduRxData, dspUdsSessionControlData.sessionPduId,dspUdsSessionControlData.pduTxData, FALSE, FALSE);
    }
#if defined(DCM_USE_SERVICE_LINKCONTROL)
    if(DCM_JTB_EXECUTE == dspUdsLinkControlData.jumpToBootState) {
        DspUdsLinkControl(dspUdsLinkControlData.pduRxData, dspUdsLinkControlData.confirmPduId, dspUdsLinkControlData.pduTxData, FALSE);
    }
#endif

}
#endif

/**
 * Main function for sending response to service as a result to jump from boot
 */
void DspStartupServiceResponseMainFunction(void)
{
    if(DCM_GENERAL_PENDING == ProgConditionStartupResponseState) {
        Std_ReturnType reqRet = DslDspResponseOnStartupRequest(GlobalProgConditions.Sid, GlobalProgConditions.SubFncId, GlobalProgConditions.ProtocolId, (uint16)GlobalProgConditions.TesterSourceAdd);
        if(E_PENDING != reqRet) {
            if(E_OK != reqRet) {
                DCM_DET_REPORTERROR(DCM_GLOBAL_ID, DCM_E_UNEXPECTED_RESPONSE);
            }
            ProgConditionStartupResponseState = DCM_GENERAL_IDLE;
        }
    }
}

void DspPreDsdMain(void) {
    /* Should be called before DsdMain so that an internal request
     * may be processed directly */
#if defined(DCM_USE_SERVICE_READDATABYPERIODICIDENTIFIER)
    DspPeriodicDIDMainFunction();
#endif
#if  defined(DCM_USE_SERVICE_RESPONSEONEVENT) && (DCM_ROE_INTERNAL_DIDS == STD_ON)
    DCM_ROE_PollDataIdentifiers();
#endif
    /* Should be done before DsdMain so that we can fulfill
     * DCM719 (mode switch DcmEcuReset to EXECUTE next main function) */
#if (DCM_USE_JUMP_TO_BOOT == STD_ON) || defined(DCM_USE_SERVICE_LINKCONTROL)
    DspJumpToBootMainFunction();
#endif

    DspStartupServiceResponseMainFunction();

}
void DspMain(void)
{
#if (DCM_USE_SPLIT_TASK_CONCEPT == STD_ON)
    DsdSetDspProcessingActive(TRUE);
#endif
    DspResetMainFunction();
#if defined(DCM_USE_SERVICE_READMEMORYBYADDRESS) || defined(DCM_USE_SERVICE_WRITEMEMORYBYADDRESS)
    DspMemoryMainFunction();
#endif
    DspReadDidMainFunction();
    DspRoutineControlMainFunction();
#ifdef DCM_USE_UPLOAD_DOWNLOAD
    DspUploadDownloadMainFunction();
#endif
    DspSecurityAccessMainFunction();
#if defined(DCM_USE_SERVICE_INPUTOUTPUTCONTROLBYIDENTIFIER) && defined(DCM_USE_CONTROL_DIDS)
    DspIOControlMainFunction();
#endif
#if (DCM_USE_SPLIT_TASK_CONCEPT == STD_ON)
    DsdSetDspProcessingActive(FALSE);
#endif
}

void DspTimerMain(void)
{
#if (DCM_SECURITY_EOL_INDEX != 0)

    for (uint8 i = 0; i < DCM_SECURITY_EOL_INDEX; i++)
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
        DspCancelPendingDid(dspUdsReadDidPending.pendingDid, dspUdsReadDidPending.pendingSignalIndex ,dspUdsReadDidPending.state, dspUdsReadDidPending.pduTxData);
    }
    dspMemoryState = DCM_MEMORY_UNUSED;
    dspUdsEcuResetData.resetPending = DCM_DSP_RESET_NO_RESET;
    dspUdsReadDidPending.state = DCM_READ_DID_IDLE;
#ifdef DCM_USE_SERVICE_WRITEDATABYIDENTIFIER
    dspUdsWriteDidPending.state = DCM_GENERAL_IDLE;
#endif
    if( DCM_GENERAL_IDLE != dspUdsRoutineControlPending.state ) {
        DspCancelPendingRoutine(dspUdsRoutineControlPending.pduRxData, dspUdsRoutineControlPending.pduTxData);
    }
    dspUdsRoutineControlPending.state = DCM_GENERAL_IDLE;
    if( DCM_GENERAL_IDLE != dspUdsSecurityAccessPending.state ) {
        DspCancelPendingSecurityAccess(dspUdsSecurityAccessPending.pduRxData, dspUdsSecurityAccessPending.pduTxData);
    }
    dspUdsSecurityAccessPending.state = DCM_GENERAL_IDLE;
#if defined(DCM_USE_UPLOAD_DOWNLOAD)
    if( DCM_GENERAL_IDLE != dspUdsUploadDownloadPending.state ) {
        DspCancelPendingUploadDownload(dspUdsUploadDownloadPending.pendingService);
    }
    dspUdsUploadDownloadPending.state = DCM_GENERAL_IDLE;
#endif

#if defined(DCM_USE_SERVICE_INPUTOUTPUTCONTROLBYIDENTIFIER) && defined(DCM_USE_CONTROL_DIDS)
    if( DCM_GENERAL_PENDING == IOControlData.state ) {
        DspCancelPendingIOControlByDataIdentifier(IOControlData.pduRxData, IOControlData.pduTxData);
    }
    IOControlData.state = DCM_GENERAL_IDLE;
#endif
#if (DCM_USE_JUMP_TO_BOOT == STD_ON)
    dspUdsSessionControlData.jumpToBootState = DCM_JTB_IDLE;
#endif
#if defined(DCM_USE_SERVICE_LINKCONTROL)
    dspUdsLinkControlData.jumpToBootState  = DCM_JTB_IDLE;
#endif
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
            while ( ((*sessionLevelRefTable)->DspSessionLevel != DCM_ALL_SESSION_LEVEL) && ((*sessionLevelRefTable)->DspSessionLevel != currentSession) && (!(*sessionLevelRefTable)->Arc_EOL) ) {
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

/**
 * Checks if a session is supported
 * @param session
 * @return TRUE: Session supported, FALSE: Session not supported
 */
boolean DspDslCheckSessionSupported(uint8 session) {
    const Dcm_DspSessionRowType *sessionRow = Dcm_ConfigPtr->Dsp->DspSession->DspSessionRow;
    while ((sessionRow->DspSessionLevel != session) && (!sessionRow->Arc_EOL) ) {
        sessionRow++;
    }
    return (FALSE == sessionRow->Arc_EOL)? TRUE: FALSE;
}

/**
 * Sets the timing parameter inresponse to UDS service 0x10
 * @param SessionData
 * @param timingData
 * @return Length of timing parameters
 */
static uint8 DspSetSessionControlTiming(const DspUdsSessionControlDataType * SessionData, uint8 *timingData)
{
    uint8 timingLen = 0u;
    if( DCM_UDS_ON_CAN == SessionData->protocolId ) {
        timingData[0u] = SessionData->P2 >> 8u;
        timingData[1u] = SessionData->P2;
        uint16 p2ServerStarMax10ms = SessionData->P2Star / 10u;
        timingData[2u] = p2ServerStarMax10ms >> 8u;
        timingData[3u] = p2ServerStarMax10ms;
        timingLen = 4u;
    }
    return timingLen;
}

void DspUdsDiagnosticSessionControl(const PduInfoType *pduRxData, PduIdType txPduId, PduInfoType *pduTxData, boolean respPendOnTransToBoot, boolean internalStartupRequest)
{
    /* @req DCM250 */
    const Dcm_DspSessionRowType *sessionRow = Dcm_ConfigPtr->Dsp->DspSession->DspSessionRow;
    Dcm_SesCtrlType reqSessionType;
    Std_ReturnType result = E_OK;
    Dcm_ProtocolType activeProtocolID;
    uint8 timingParamLen;
    if( DCM_JTB_IDLE == dspUdsSessionControlData.jumpToBootState ) {
#if defined(DCM_USE_SERVICE_RESPONSEONEVENT) && (DCM_ROE_INIT_ON_DSC == STD_ON)
        /* Stop the response on event service */
        /* @req DCM597 */
        if (DCM_ROE_IsActive()) {
            (void)Dcm_StopROE();
        }
#endif
        if (pduRxData->SduLength == 2) {
            reqSessionType = pduRxData->SduDataPtr[1];
            // Check if type exist in session table
            while ((sessionRow->DspSessionLevel != reqSessionType) && (!sessionRow->Arc_EOL) ) {
                sessionRow++;
            }
            if (!sessionRow->Arc_EOL) {
#if (DCM_USE_JUMP_TO_BOOT == STD_ON)
                if(!internalStartupRequest) {
                    switch(sessionRow->DspSessionForBoot) {
                        case DCM_OEM_BOOT:
                        case DCM_OEM_BOOT_RESPAPP:
                            /* @req DCM532 */
                            result = Rte_Switch_DcmEcuReset_DcmEcuReset(RTE_MODE_DcmEcuReset_JUMPTOBOOTLOADER);
                            break;
                        case DCM_SYS_BOOT:
                        case DCM_SYS_BOOT_RESPAPP:
                            /* @req DCM592 */
                            result = Rte_Switch_DcmEcuReset_DcmEcuReset(RTE_MODE_DcmEcuReset_JUMPTOSYSSUPPLIERBOOTLOADER);
                            break;
                        case DCM_NO_BOOT:
                            result = E_OK;
                            break;
                        default:
                            result = E_NOT_OK;
                            break;
                    }
                }
#endif
                if (result == E_OK) {
                    dspUdsSessionControlData.session = reqSessionType;
                    dspUdsSessionControlData.sessionPduId = txPduId;
                    dspUdsSessionControlData.pduRxData = pduRxData;
                    dspUdsSessionControlData.pduTxData = pduTxData;
                    dspUdsSessionControlData.P2 = sessionRow->DspSessionP2ServerMax;
                    dspUdsSessionControlData.P2Star = sessionRow->DspSessionP2StarServerMax;

                    Std_ReturnType activeProtocolStatus = DslGetActiveProtocol(&activeProtocolID);
                    if( E_OK == activeProtocolStatus ) {
                        dspUdsSessionControlData.protocolId = activeProtocolID;
                    } else {
                        dspUdsSessionControlData.protocolId = 0xFF;
                    }
                    if( (DCM_NO_BOOT == sessionRow->DspSessionForBoot) || internalStartupRequest) {
                        dspUdsSessionControlData.pendingSessionChange = TRUE;
                        pduTxData->SduDataPtr[1] = reqSessionType;
                        timingParamLen = DspSetSessionControlTiming(&dspUdsSessionControlData, &pduTxData->SduDataPtr[2]);
                        pduTxData->SduLength = 2u + timingParamLen;
                        DsdDspProcessingDone(DCM_E_POSITIVERESPONSE);
                    } else {
#if (DCM_USE_JUMP_TO_BOOT == STD_ON)
                        GlobalProgConditions.ReprogramingRequest = (DCM_PROGRAMMING_SESSION == reqSessionType);
                        GlobalProgConditions.ResponseRequired = DsdDspGetResponseRequired();
                        GlobalProgConditions.Sid = SID_DIAGNOSTIC_SESSION_CONTROL;
                        GlobalProgConditions.SubFncId = reqSessionType;
                        if( E_OK == activeProtocolStatus ) {
                            GlobalProgConditions.ProtocolId = activeProtocolID;
                        }
                        uint16 srcAddr = DsdDspGetTesterSourceAddress();
                        GlobalProgConditions.TesterSourceAdd = (uint8)(srcAddr & 0xFF);
                        if( respPendOnTransToBoot ) {
                            /* @req 4.2.2/SWS_DCM_01177 */
                            if((sessionRow->DspSessionForBoot == DCM_OEM_BOOT_RESPAPP) || (sessionRow->DspSessionForBoot == DCM_SYS_BOOT_RESPAPP)){
                                dspUdsSessionControlData.jumpToBootState = DCM_JTB_RESAPP_WAIT_RESPONSE_PENDING_TX_CONFIRM;
                            }else {
                                dspUdsSessionControlData.jumpToBootState = DCM_JTB_WAIT_RESPONSE_PENDING_TX_CONFIRM;
                            }
                            /* Force response pending next main function and
                             * wait for tx confirmation*/
                            /* @req DCM654 */
                            DsdDspForceResponsePending();


                        } else {
                            /* Trigger mode switch next main function */
                            /* @req DCM719 */
                            /* @req DCM720 */
                            /* IMPROVEMENT: Add support for pending */
                            if((sessionRow->DspSessionForBoot == DCM_OEM_BOOT) || (sessionRow->DspSessionForBoot == DCM_SYS_BOOT)){
                                if(E_OK == Dcm_SetProgConditions(&GlobalProgConditions)) {
                                    dspUdsSessionControlData.jumpToBootState = DCM_JTB_EXECUTE;
                                } else {
                                    /* @req DCM715 */
                                    DsdDspProcessingDone(DCM_E_CONDITIONSNOTCORRECT);
                                }
                            }
                            /* @req 4.2.2/SWS_DCM_01178 */
                            else if((sessionRow->DspSessionForBoot == DCM_OEM_BOOT_RESPAPP) || (sessionRow->DspSessionForBoot == DCM_SYS_BOOT_RESPAPP)){
                                dspUdsSessionControlData.jumpToBootState = DCM_JTB_RESAPP_FINAL_RESPONSE_TX_CONFIRM;
                                pduTxData->SduDataPtr[1] = reqSessionType;
                                timingParamLen = DspSetSessionControlTiming(&dspUdsSessionControlData, &pduTxData->SduDataPtr[2]);
                                pduTxData->SduLength = 2u + timingParamLen;
                                DsdDspProcessingDone(DCM_E_POSITIVERESPONSE);
                            } else {
                                /* Avoid compiler message */
                            }
                        }
                        if( (E_OK != activeProtocolStatus) || (0 != (srcAddr & (uint16)~0xFF)) ) {
                            /* Failed to get the protocol id or the source address was too large */
                            DCM_DET_REPORTERROR(DCM_GLOBAL_ID, DCM_E_UNEXPECTED_EXECUTION);
                        }
#else
                        DCM_DET_REPORTERROR(DCM_GLOBAL_ID, DCM_E_UNEXPECTED_EXECUTION);
                        DsdDspProcessingDone(DCM_E_CONDITIONSNOTCORRECT);
#endif
                    }
                } else {
                    // IMPROVEMENT: Add handling of special case of E_FORCE_RCRRP
                    DsdDspProcessingDone(DCM_E_CONDITIONSNOTCORRECT);
                }
            } else {
                DsdDspProcessingDone(DCM_E_SUBFUNCTIONNOTSUPPORTED); /* @req DCM307 */
            }
        } else {
            // Wrong length
            DsdDspProcessingDone(DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT);
        }
    }
#if (DCM_USE_JUMP_TO_BOOT == STD_ON)
    else if(DCM_JTB_EXECUTE == dspUdsSessionControlData.jumpToBootState) {
        if(E_OK != Rte_Switch_DcmEcuReset_DcmEcuReset(RTE_MODE_DcmEcuReset_EXECUTE)) {
            DsdDspProcessingDone(DCM_E_CONDITIONSNOTCORRECT);
        }
        dspUdsSessionControlData.jumpToBootState = DCM_JTB_IDLE;
    }
    else if(DCM_JTB_RESAPP_ASSEMBLE_FINAL_RESPONSE == dspUdsSessionControlData.jumpToBootState) {
        dspUdsSessionControlData.jumpToBootState = DCM_JTB_RESAPP_FINAL_RESPONSE_TX_CONFIRM;
        pduTxData->SduDataPtr[1] = pduRxData->SduDataPtr[1];
        timingParamLen = DspSetSessionControlTiming(&dspUdsSessionControlData, &pduTxData->SduDataPtr[2]);
        pduTxData->SduLength = 2u + timingParamLen;
        DsdDspProcessingDone(DCM_E_POSITIVERESPONSE);
    }
#else
    (void)respPendOnTransToBoot;
#endif
}
#if defined(DCM_USE_SERVICE_LINKCONTROL)
/**
 * @brief Service 0x87 (Linkcontrol) processing.
 *
 * @note This service is only used for jumping to bootloader. For changing baudrate external service processing has to be used.
 *
 * @param[in]  pdRxData              Received PDU data.
 * @param[in]  txPduId               Id of the PDU TX.
 * @param[out] pduTxData             PDU data to be transmitted.
 * @param[in] respPendOnTransToBoot  Control flag to send NRC 0x78 when configured in session row.
 */
void DspUdsLinkControl(const PduInfoType *pduRxData, PduIdType txPduId, PduInfoType *pduTxData, boolean respPendOnTransToBoot)
{
    /* @req DCM743 */
    Std_ReturnType result = E_NOT_OK;
    Dcm_ProtocolType activeProtocolID;
    Std_ReturnType activeProtocolStatus;

    if( DCM_JTB_IDLE == dspUdsLinkControlData.jumpToBootState ) {
        if (pduRxData->SduDataPtr[1] == LINKCONTROL_SUBFUNC_VERIFY_BAUDRATE_TRANS_WITH_FIXED_BAUDRATE ||
            pduRxData->SduDataPtr[1] == LINKCONTROL_SUBFUNC_VERIFY_BAUDRATE_TRANS_WITH_SPECIFIC_BAUDRATE ||
            pduRxData->SduDataPtr[1] == LINKCONTROL_SUBFUNC_TRANSITION_BAUDRATE) {
            if ( (pduRxData->SduDataPtr[1] == LINKCONTROL_SUBFUNC_VERIFY_BAUDRATE_TRANS_WITH_FIXED_BAUDRATE && pduRxData->SduLength == 3) ||
                 (pduRxData->SduDataPtr[1] == LINKCONTROL_SUBFUNC_VERIFY_BAUDRATE_TRANS_WITH_SPECIFIC_BAUDRATE && pduRxData->SduLength == 5) ||
                 (pduRxData->SduDataPtr[1] == LINKCONTROL_SUBFUNC_TRANSITION_BAUDRATE && pduRxData->SduLength == 2) ) {
                /* @req DCM533 */
                /* @req DCM744 */
                result = Rte_Switch_DcmEcuReset_DcmEcuReset(RTE_MODE_DcmEcuReset_JUMPTOBOOTLOADER);

                if (result == E_OK) {
                    dspUdsLinkControlData.confirmPduId = txPduId;
                    dspUdsLinkControlData.pduRxData = pduRxData;
                    dspUdsLinkControlData.pduTxData = pduTxData;

                    GlobalProgConditions.ReprogramingRequest = FALSE;
                    GlobalProgConditions.ResponseRequired = DsdDspGetResponseRequired();
                    GlobalProgConditions.Sid = SID_LINK_CONTROL;
                    GlobalProgConditions.SubFncId = pduRxData->SduDataPtr[1];
                    activeProtocolStatus = DslGetActiveProtocol(&activeProtocolID);

                    if( E_OK == activeProtocolStatus ) {
                        GlobalProgConditions.ProtocolId = activeProtocolID;
                    }

                    GlobalProgConditions.TesterSourceAdd = (uint8)(DsdDspGetTesterSourceAddress() & 0xFF);

                    if( respPendOnTransToBoot ) {
                        dspUdsLinkControlData.jumpToBootState = DCM_JTB_WAIT_RESPONSE_PENDING_TX_CONFIRM;

                        /* Force response pending next main function and
                        * wait for tx confirmation*/
                        /* @req DCM654 */
                        DsdDspForceResponsePending();
                    }
                    else {
                        /* Trigger mode switch next main function */
                        /* @req DCM719 */
                        /* @req DCM720 */
                        if(E_OK == Dcm_SetProgConditions(&GlobalProgConditions)) {
                            dspUdsLinkControlData.jumpToBootState = DCM_JTB_EXECUTE;
                        }
                        else {
                            /* @req DCM715 */
                            DsdDspProcessingDone(DCM_E_CONDITIONSNOTCORRECT);
                        }
                    }
                }
                else {
                    DsdDspProcessingDone(DCM_E_CONDITIONSNOTCORRECT);
                }
            }
            else {
                /* Wrong length */
                DsdDspProcessingDone(DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT);
            }
        }
         else {
            /* Subfunction not supported */
            DsdDspProcessingDone(DCM_E_SUBFUNCTIONNOTSUPPORTED);
         }
    }
    else if(DCM_JTB_EXECUTE == dspUdsLinkControlData.jumpToBootState) {
        if(E_OK != Rte_Switch_DcmEcuReset_DcmEcuReset(RTE_MODE_DcmEcuReset_EXECUTE)) {
            DsdDspProcessingDone(DCM_E_CONDITIONSNOTCORRECT);
        }
        dspUdsLinkControlData.jumpToBootState = DCM_JTB_IDLE;
    }

}
#endif

void DspUdsEcuReset(const PduInfoType *pduRxData, PduIdType txPduId, PduInfoType *pduTxData, boolean startupResponseRequest)
{
    /* @req DCM260 */
    Dcm_EcuResetType reqResetType;
    uint8 rteMode;
    if (pduRxData->SduLength == 2) {
        reqResetType = (Dcm_EcuResetType)pduRxData->SduDataPtr[1];
        Std_ReturnType result = E_NOT_OK;
        /* @req DCM373 */
        /* IMPROVEMENT: Should be a call to SchM */
        if( FALSE == startupResponseRequest ) {
            if( (DCM_DISABLE_RAPID_POWER_SHUTDOWN != reqResetType) && (DCM_ENABLE_RAPID_POWER_SHUTDOWN != reqResetType) ) {
                if( E_OK == getResetRteMode(reqResetType, &rteMode) ) {
                    result = Rte_Switch_DcmEcuReset_DcmEcuReset(rteMode);

                    dspUdsEcuResetData.resetPduId = txPduId;
                    dspUdsEcuResetData.pduTxData = pduTxData;
                    dspUdsEcuResetData.resetType = reqResetType;

                    switch( result )  {
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
                } else {
                    /* Not supported */
                    DsdDspProcessingDone(DCM_E_SUBFUNCTIONNOTSUPPORTED);
                }
            } else {
                /* Not supported */
                /* IMPROVEMENT: Add support for rapid power shutdown */
                DsdDspProcessingDone(DCM_E_SUBFUNCTIONNOTSUPPORTED);
            }
        } else {
            /* Response to jump from boot. Create positive response */
            pduTxData->SduDataPtr[1] = reqResetType;
            pduTxData->SduLength = 2;
            DsdDspProcessingDone(DCM_E_POSITIVERESPONSE);
        }
    } else {
        // Wrong length
        DsdDspProcessingDone(DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT);
    }
}

#if defined(USE_DEM) && defined(DCM_USE_SERVICE_CLEARDIAGNOSTICINFORMATION)
void DspUdsClearDiagnosticInformation(const PduInfoType *pduRxData, PduInfoType *pduTxData)
{
    /* @req DCM247 */
    uint32 dtc;
    Dem_ReturnClearDTCType result;

    if (pduRxData->SduLength == 4) {
        dtc = BYTES_TO_DTC(pduRxData->SduDataPtr[1], pduRxData->SduDataPtr[2], pduRxData->SduDataPtr[3]);

        result = Dem_ClearDTC(dtc, DEM_DTC_FORMAT_UDS, DEM_DTC_ORIGIN_PRIMARY_MEMORY); /* @req DCM005 */

        switch (result) {
            case DEM_CLEAR_OK:
                /* Create positive response */
                /* @req DCM705 */
                pduTxData->SduLength = 1;
                DsdDspProcessingDone(DCM_E_POSITIVERESPONSE);
                break;
            case DEM_CLEAR_FAILED:
                DsdDspProcessingDone(DCM_E_CONDITIONSNOTCORRECT);/* @req DCM707 */
                break;
            default:
                /* @req DCM708 */
                /* !req DCM706 */
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
static Dcm_NegativeResponseCodeType udsReadDtcInfoSub_0x01_0x07_0x11_0x12(const PduInfoType *pduRxData, PduInfoType *pduTxData)
{
    typedef struct {
        uint8       SID;
        uint8       reportType;
        uint8       dtcStatusAvailabilityMask;
        uint8       dtcFormatIdentifier;
        uint8       dtcCountHighByte;
        uint8       dtcCountLowByte;
    } TxDataType;

    Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;
    Dem_ReturnSetFilterType setDtcFilterResult;
    uint16 numberOfFilteredDtc = 0;

    /* @req DCM700 */
    uint8 dtcStatusMask = pduRxData->SduDataPtr[1] == 0x07 ? pduRxData->SduDataPtr[3] : pduRxData->SduDataPtr[2];

    if (dtcStatusMask != 0x00) {

        // Setup the DTC filter
        switch (pduRxData->SduDataPtr[1]) {	/* @req DCM293 */
            case 0x01:	// reportNumberOfDTCByStatusMask
                setDtcFilterResult = Dem_SetDTCFilter(dtcStatusMask, DEM_DTC_KIND_ALL_DTCS, DEM_DTC_FORMAT_UDS, DEM_DTC_ORIGIN_PRIMARY_MEMORY, DEM_FILTER_WITH_SEVERITY_NO, VALUE_IS_NOT_USED, DEM_FILTER_FOR_FDC_NO);
                break;

            case 0x07:	// reportNumberOfDTCBySeverityMaskRecord
                setDtcFilterResult = Dem_SetDTCFilter(dtcStatusMask, DEM_DTC_KIND_ALL_DTCS, DEM_DTC_FORMAT_UDS, DEM_DTC_ORIGIN_PRIMARY_MEMORY, DEM_FILTER_WITH_SEVERITY_YES, pduRxData->SduDataPtr[2], DEM_FILTER_FOR_FDC_NO);
                break;

            case 0x11:	// reportNumberOfMirrorMemoryDTCByStatusMask
                setDtcFilterResult = Dem_SetDTCFilter(dtcStatusMask, DEM_DTC_KIND_ALL_DTCS, DEM_DTC_FORMAT_UDS, DEM_DTC_ORIGIN_MIRROR_MEMORY, DEM_FILTER_WITH_SEVERITY_NO, VALUE_IS_NOT_USED, DEM_FILTER_FOR_FDC_NO);
                break;

            case 0x12:	// reportNumberOfEmissionRelatedOBDDTCByStatusMask
                setDtcFilterResult = Dem_SetDTCFilter(dtcStatusMask, DEM_DTC_KIND_EMISSION_REL_DTCS, DEM_DTC_FORMAT_UDS, DEM_DTC_ORIGIN_PRIMARY_MEMORY, DEM_FILTER_WITH_SEVERITY_NO, VALUE_IS_NOT_USED, DEM_FILTER_FOR_FDC_NO);
                break;

            default:
                setDtcFilterResult = DEM_WRONG_FILTER;
                break;
        }

        if (setDtcFilterResult == DEM_FILTER_ACCEPTED) {
            Dem_ReturnGetNumberOfFilteredDTCType getNumerResult;

            /* @req DCM376 */
            getNumerResult = Dem_GetNumberOfFilteredDtc(&numberOfFilteredDtc);
            if (getNumerResult != DEM_NUMBER_OK) {
                responseCode = DCM_E_GENERALREJECT;
            }
        }
        else {
            responseCode = DCM_E_REQUESTOUTOFRANGE;
        }
    }


    if (responseCode == DCM_E_POSITIVERESPONSE) {

        uint8 dtcAvailablityMask;
        Std_ReturnType result;
        TxDataType *txData = (TxDataType*)pduTxData->SduDataPtr;

        result = Dem_GetDTCStatusAvailabilityMask(&dtcAvailablityMask);
        if (result != E_OK) {
            dtcAvailablityMask = 0;
        }

        // Create positive response (ISO 14229-1 table 251)
        txData->reportType = pduRxData->SduDataPtr[1];						// reportType
        txData->dtcStatusAvailabilityMask = dtcAvailablityMask;				// DTCStatusAvailabilityMask
        txData->dtcFormatIdentifier = Dem_GetTranslationType();				// DTCFormatIdentifier
        txData->dtcCountHighByte = (numberOfFilteredDtc >> 8);				// DTCCount high byte
        txData->dtcCountLowByte = (numberOfFilteredDtc & 0xFFu);			// DTCCount low byte
        pduTxData->SduLength = 6;

    }

    return responseCode;
}


static Dcm_NegativeResponseCodeType udsReadDtcInfoSub_0x02_0x0A_0x0F_0x13_0x15(const PduInfoType *pduRxData, PduInfoType *pduTxData)
{
    Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;
    Dem_ReturnSetFilterType setDtcFilterResult;
    uint16 nofFilteredDTCs = 0;

    if ((pduRxData->SduDataPtr[1] != 0x0A) && (pduRxData->SduDataPtr[1] != 0x15) && (pduRxData->SduDataPtr[2] == 0x00)) {
        uint16 currIndex = 1;
        uint8 dtcStatusMask;

        /* @req DCM377 */
        if (Dem_GetDTCStatusAvailabilityMask(&dtcStatusMask) != E_OK) {
            dtcStatusMask = 0;
        }

        // Create positive response (ISO 14229-1 table 252)
        pduTxData->SduDataPtr[currIndex++] = pduRxData->SduDataPtr[1];
        pduTxData->SduDataPtr[currIndex++] = dtcStatusMask;
        pduTxData->SduLength = currIndex;
    }
    else {
        // Setup the DTC filter
        /* @req DCM378 */
        switch (pduRxData->SduDataPtr[1])
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
            Dem_ReturnGetNextFilteredDTCType getNextFilteredDtcResult;
            uint32 dtc;
            Dem_EventStatusExtendedType dtcStatus;
            Std_ReturnType result;
            uint16 currIndex = 1;

            /* @req DCM377 */
            result = Dem_GetDTCStatusAvailabilityMask(&dtcStatusMask);
            if (result != E_OK) {
                dtcStatusMask = 0;
            }

            // Create positive response (ISO 14229-1 table 252)
            pduTxData->SduDataPtr[currIndex++] = pduRxData->SduDataPtr[1];
            pduTxData->SduDataPtr[currIndex++] = dtcStatusMask;

            if (dtcStatusMask != 0x00u) { /* @req DCM008 */
                if( DEM_NUMBER_OK == Dem_GetNumberOfFilteredDtc(&nofFilteredDTCs) ) {
                    if( ((nofFilteredDTCs * 4u) + currIndex) <= pduTxData->SduLength ) {
                        getNextFilteredDtcResult = Dem_GetNextFilteredDTC(&dtc, &dtcStatus);
                        while (getNextFilteredDtcResult == DEM_FILTERED_OK) {
                            pduTxData->SduDataPtr[currIndex++] = DTC_HIGH_BYTE(dtc);
                            pduTxData->SduDataPtr[currIndex++] = DTC_MID_BYTE(dtc);
                            pduTxData->SduDataPtr[currIndex++] = DTC_LOW_BYTE(dtc);
                            pduTxData->SduDataPtr[currIndex++] = dtcStatus;
                            getNextFilteredDtcResult = Dem_GetNextFilteredDTC(&dtc, &dtcStatus);
                        }

                        if (getNextFilteredDtcResult != DEM_FILTERED_NO_MATCHING_DTC) {
                            responseCode = DCM_E_REQUESTOUTOFRANGE;
                        }
                    }
                    else {
                        /* Tx buffer too small */
                        responseCode = DCM_E_RESPONSETOOLONG;
                    }
                }
                else {
                    /* Could not read number of filtered DTCs */
                    responseCode = DCM_E_REQUESTOUTOFRANGE;
                }
            }
            pduTxData->SduLength = currIndex;
        }
        else {
            responseCode = DCM_E_REQUESTOUTOFRANGE;
        }
    }

    return responseCode;
}

static Dcm_NegativeResponseCodeType udsReadDtcInfoSub_0x08(const PduInfoType *pduRxData, PduInfoType *pduTxData)
{
    Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;
    //lint -estring(920,pointer)  /* cast to void */
    (void)pduRxData;
    (void)pduTxData;
    //lint +estring(920,pointer)  /* cast to void */
    // IMPROVEMENT: Not supported yet, (DEM module does not currently support severity).
    responseCode = DCM_E_REQUESTOUTOFRANGE;

    return responseCode;
}


static Dcm_NegativeResponseCodeType udsReadDtcInfoSub_0x09(const PduInfoType *pduRxData, PduInfoType *pduTxData)
{
    Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;
    //lint -estring(920,pointer)  /* cast to void */
    (void)pduRxData;
    (void)pduTxData;
    //lint +estring(920,pointer)  /* cast to void */
    // IMPROVEMENT: Not supported yet, (DEM module does not currently support severity).
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
    switch (pduRxData->SduDataPtr[1]) { /* @req DCM378 */
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
    switch (pduRxData->SduDataPtr[5]) {
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
        Dem_EventStatusExtendedType statusOfDtc;

        uint32 dtc = BYTES_TO_DTC(pduRxData->SduDataPtr[2], pduRxData->SduDataPtr[3], pduRxData->SduDataPtr[4]);
        /* @req DCM295 */ 
        /* @req DCM475 */
        if (DEM_STATUS_OK == Dem_GetStatusOfDTC(dtc, dtcOrigin, &statusOfDtc)) {
            Dem_ReturnGetExtendedDataRecordByDTCType getExtendedDataRecordByDtcResult;
            uint16 recLength;
            uint16 txIndex = 6;
            boolean foundValidRecordNumber = FALSE;

            /* @req DCM297 */
            /* @req DCM474 */
            /* @req DCM386 */
            pduTxData->SduDataPtr[1] = pduRxData->SduDataPtr[1];			// Sub function
            pduTxData->SduDataPtr[2] = DTC_HIGH_BYTE(dtc);					// DTC high byte
            pduTxData->SduDataPtr[3] = DTC_MID_BYTE(dtc);					// DTC mid byte
            pduTxData->SduDataPtr[4] = DTC_LOW_BYTE(dtc);					// DTC low byte
            pduTxData->SduDataPtr[5] = statusOfDtc;							// DTC status
            for (uint8 recNum = startRecNum; (recNum <= endRecNum) && (DCM_E_POSITIVERESPONSE == responseCode); recNum++) {
                // Calculate what's left in buffer
                recLength = (pduTxData->SduLength > (txIndex + 1)) ? (pduTxData->SduLength - (txIndex + 1)) : 0u;
                /* @req DCM296 */
                /* @req DCM476 */
                /* @req DCM382 */
                /* !req DCM371 */
                getExtendedDataRecordByDtcResult = Dem_GetExtendedDataRecordByDTC(dtc, dtcOrigin, recNum, &pduTxData->SduDataPtr[txIndex+1], &recLength);
                switch(getExtendedDataRecordByDtcResult) {
                    case DEM_RECORD_OK:
                        foundValidRecordNumber = TRUE;
                        if (recLength > 0) {
                            pduTxData->SduDataPtr[txIndex++] = recNum;
                            /* Instead of calling Dem_GetSizeOfExtendedDataRecordByDTC() the result from Dem_GetExtendedDataRecordByDTC() is used */
                            /* !req DCM478 */
                            txIndex += recLength;
                        }
                        break;
                    case DEM_RECORD_BUFFERSIZE:
                        /* Tx buffer not big enough */
                        responseCode = DCM_E_RESPONSETOOLONG;
                        break;
                    default:
                        break;
                }
            }

            pduTxData->SduLength = txIndex;

            if (!foundValidRecordNumber) {
                responseCode = DCM_E_REQUESTOUTOFRANGE;
            }
        }
        else {
            /* !req DCM739 */
            responseCode = DCM_E_REQUESTOUTOFRANGE;
        }
    }

    return responseCode;
}

static Dcm_NegativeResponseCodeType udsReadDtcInfoSub_0x03(const PduInfoType *pduRxData, PduInfoType *pduTxData)
{
    Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;

    uint16 numFilteredRecords = 0;
    uint32 dtc = 0;
    uint8 recordNumber = 0;
    uint16 nofBytesCopied = 0;
    //lint -estring(920,pointer)  /* cast to void */
    (void)pduRxData;
    //lint +estring(920,pointer)  /* cast to void */
    /* @req DCM298 */
    if( DEM_FILTER_ACCEPTED == Dem_SetFreezeFrameRecordFilter(DEM_DTC_FORMAT_UDS, &numFilteredRecords) ) {
        if ( (SID_LEN + SF_LEN + (DTC_LEN + FF_REC_NUM_LEN)*numFilteredRecords) <= pduTxData->SduLength ) {
            for( uint16 i = 0; (i < numFilteredRecords) && (DCM_E_POSITIVERESPONSE == responseCode); i++ ) {
                /* @req DCM299 */
                if( DEM_FILTERED_OK == Dem_GetNextFilteredRecord(&dtc, &recordNumber) ) {
                    /* @req DCM300 */
                    pduTxData->SduDataPtr[SID_LEN + SF_LEN + nofBytesCopied++] = DTC_HIGH_BYTE(dtc);
                    pduTxData->SduDataPtr[SID_LEN + SF_LEN + nofBytesCopied++] = DTC_MID_BYTE(dtc);
                    pduTxData->SduDataPtr[SID_LEN + SF_LEN + nofBytesCopied++] = DTC_LOW_BYTE(dtc);
                    pduTxData->SduDataPtr[SID_LEN + SF_LEN + nofBytesCopied++] = recordNumber;
                } else {
                    /* !req DCM740 */
                    responseCode = DCM_E_REQUESTOUTOFRANGE;
                }
            }
        } else {
            responseCode = DCM_E_RESPONSETOOLONG;
        }
    } else {
        responseCode = DCM_E_REQUESTOUTOFRANGE;
    }

    pduTxData->SduDataPtr[0] = 0x59;    // positive response
    pduTxData->SduDataPtr[1] = 0x03;    // subid
    pduTxData->SduLength = SID_LEN + SF_LEN + nofBytesCopied;

    return responseCode;
}

static Dcm_NegativeResponseCodeType udsReadDtcInfoSub_0x04(const PduInfoType *pduRxData, PduInfoType *pduTxData)
{
    // 1. Only consider Negative Response 0x10
    /* @req DCM302 */
    /* @req DCM387 */
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

    GetStatusOfDtc = Dem_GetStatusOfDTC(DtcNumber, DEM_DTC_ORIGIN_PRIMARY_MEMORY, &DtcStatus); /* @req DCM383 */
    switch (GetStatusOfDtc) {
        case DEM_STATUS_OK:
            break;
        default:
            /* !req DCM739 */
            return DCM_E_REQUESTOUTOFRANGE;
    }

    RecordNumber = pduRxData->SduDataPtr[5];
    /* !req DCM372 */
    /* !req DCM702 */
    if( 0xFF == RecordNumber ) {
        /* Request for all freeze frames */
        GetFFbyDtcReturnCode = DEM_GET_FFDATABYDTC_WRONG_DTC;
        uint16 nofBytesCopied = 0;
        uint16 bufSizeLeft = 0;
        Dem_ReturnGetFreezeFrameDataByDTCType ret = DEM_GET_FFDATABYDTC_OK;
        for(uint8 record = 0; (record < RecordNumber) && (DEM_GET_FFDATABYDTC_BUFFERSIZE != GetFFbyDtcReturnCode); record++) { /* @req DCM385 */
            bufSizeLeft = pduTxData->SduLength - 6 - nofBytesCopied;
            ret = Dem_GetFreezeFrameDataByDTC(DtcNumber, DEM_DTC_ORIGIN_PRIMARY_MEMORY,
                    record, &pduTxData->SduDataPtr[6 + nofBytesCopied], &bufSizeLeft);

            switch(ret) {
                case DEM_GET_FFDATABYDTC_OK:
                    nofBytesCopied += bufSizeLeft;/* !req DCM441 */
                    /* At least one OK! */
                    GetFFbyDtcReturnCode = DEM_GET_FFDATABYDTC_OK;
                    break;
                case DEM_GET_FFDATABYDTC_BUFFERSIZE:
                    /* Tx buffer not big enough */
                    GetFFbyDtcReturnCode = DEM_GET_FFDATABYDTC_BUFFERSIZE;
                    break;
                default:
                    break;
            }

        }
        SizeOfTxBuf = nofBytesCopied;
    } else {
        GetFFbyDtcReturnCode = Dem_GetFreezeFrameDataByDTC(DtcNumber, DEM_DTC_ORIGIN_PRIMARY_MEMORY,
            RecordNumber, &pduTxData->SduDataPtr[6], &SizeOfTxBuf);/* @req DCM384 */
    }

    // Negative response
    switch (GetFFbyDtcReturnCode) {
        case DEM_GET_FFDATABYDTC_OK:
            // Positive response
            // See ISO 14229(2006) Table 254
            pduTxData->SduDataPtr[0] = 0x59;    // positive response
            pduTxData->SduDataPtr[1] = 0x04;    // subid
            pduTxData->SduDataPtr[2] = pduRxData->SduDataPtr[2];    // DTC
            pduTxData->SduDataPtr[3] = pduRxData->SduDataPtr[3];
            pduTxData->SduDataPtr[4] = pduRxData->SduDataPtr[4];
            pduTxData->SduDataPtr[5] = (uint8)DtcStatus;    //status
            pduTxData->SduLength = SizeOfTxBuf + 6;
            responseCode = DCM_E_POSITIVERESPONSE;
            break;
        case DEM_GET_FFDATABYDTC_BUFFERSIZE:
            responseCode = DCM_E_RESPONSETOOLONG;
            break;
        default:
            responseCode =  DCM_E_REQUESTOUTOFRANGE;
    }


    return responseCode;
}

static Dcm_NegativeResponseCodeType udsReadDtcInfoSub_0x05(const PduInfoType *pduRxData, PduInfoType *pduTxData)
{
    Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;
    //lint -estring(920,pointer)  /* cast to void */
    (void)pduRxData;
    (void)pduTxData;
    //lint +estring(920,pointer)  /* cast to void */
    // IMPROVEMENT: Add support
    responseCode = DCM_E_REQUESTOUTOFRANGE;

    return responseCode;
}

static Dcm_NegativeResponseCodeType udsReadDtcInfoSub_0x0B_0x0C_0x0D_0x0E(const PduInfoType *pduRxData, PduInfoType *pduTxData)
{
    Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;
    //lint -estring(920,pointer)  /* cast to void */
    (void)pduRxData;
    (void)pduTxData;
    //lint +estring(920,pointer)  /* cast to void */
    // IMPROVEMENT: Add support
    responseCode = DCM_E_REQUESTOUTOFRANGE;

    return responseCode;
}

static Dcm_NegativeResponseCodeType udsReadDtcInfoSub_0x14(const PduInfoType *pduRxData, PduInfoType *pduTxData)
{
    Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;
    //lint -estring(920,pointer)  /* cast to void */
    (void)pduRxData; /* Avoid compiler warning */
    (void)pduTxData; /* Avoid compiler warning */
    //lint +estring(920,pointer)  /* cast to void */
    // IMPROVEMENT: Add support
    /** !464 */
    responseCode = DCM_E_REQUESTOUTOFRANGE;

    return responseCode;
}


void DspUdsReadDtcInformation(const PduInfoType *pduRxData, PduInfoType *pduTxData)
{
    /* @req DCM248 */
    // Sub function number         0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F 10 11 12 13 14 15
    const uint8 sduLength[0x16] = {0, 3, 3, 2, 6, 3, 6, 4, 4, 5, 2, 2, 2, 2, 2, 3, 6, 3, 3, 3, 2, 2};
    /* Sub function number                 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F 10 11 12 13 14 15 */
    const boolean subFncSupported[0x16] = {0, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0,};

    Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;

    uint8 subFunctionNumber = pduRxData->SduDataPtr[1];

    // Check length
    if (subFunctionNumber <= 0x15) {
        if(subFncSupported[subFunctionNumber]) {
            if (pduRxData->SduLength == sduLength[subFunctionNumber]) {
                switch (subFunctionNumber) {
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
        /* Subfunction not supported */
        responseCode = DCM_E_SUBFUNCTIONNOTSUPPORTED;
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


static void DspCancelPendingDid(uint16 didNr, uint16 signalIndex, ReadDidPendingStateType pendingState, PduInfoType *pduTxData )
{
    const Dcm_DspDidType *didPtr = NULL_PTR;
    if( lookupNonDynamicDid(didNr, &didPtr) ) {
        if( signalIndex < didPtr->DspNofSignals ) {
            const Dcm_DspDataType *dataPtr = didPtr->DspSignalRef[signalIndex].DspSignalDataRef;
            if( DCM_READ_DID_PENDING_COND_CHECK == pendingState ) {
                if( dataPtr->DspDataConditionCheckReadFnc != NULL_PTR ) {
                    (void)dataPtr->DspDataConditionCheckReadFnc (DCM_CANCEL, pduTxData->SduDataPtr);
                }
            } else if( DCM_READ_DID_PENDING_READ_DATA == pendingState ) {
                if( dataPtr->DspDataReadDataFnc.AsynchDataReadFnc != NULL_PTR ) {
                    if( DATA_PORT_ASYNCH == dataPtr->DspDataUsePort ) {
                        (void)dataPtr->DspDataReadDataFnc.AsynchDataReadFnc(DCM_CANCEL, pduTxData->SduDataPtr);
                    }
                }
            } else {
                /* Not in a pending state */
                DCM_DET_REPORTERROR(DCM_GLOBAL_ID, DCM_E_UNEXPECTED_EXECUTION);
            }
        } else {
            /* Invalid signal index */
            DCM_DET_REPORTERROR(DCM_GLOBAL_ID, DCM_E_UNEXPECTED_EXECUTION);
        }
    }
}


/**
**		This Function for read Dynamically Did data buffer Sourced by Memory address using a didNr
**/
#ifdef DCM_USE_SERVICE_DYNAMICALLYDEFINEDATAIDENTIFIER
static Dcm_NegativeResponseCodeType readDDDData(Dcm_DspDDDType *DDidPtr, uint8 *Data, uint16 bufSize, uint16 *Length)
{
    uint8 i;
    uint8 dataCount;
    uint16 SourceDataLength = 0u;
    const Dcm_DspDidType *SourceDidPtr = NULL_PTR;
    const Dcm_DspSignalType *signalPtr;
    Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;
    *Length = 0u;
    uint8* nextDataSlot = Data;

    for(i = 0;(i < DCM_MAX_DDDSOURCE_NUMBER) && (DDidPtr->DDDSource[i].formatOrPosition != 0)
        &&(responseCode == DCM_E_POSITIVERESPONSE);i++)
    {
        if(DDidPtr->DDDSource[i].DDDTpyeID == DCM_DDD_SOURCE_ADDRESS) {
            responseCode = checkAddressRange(DCM_READ_MEMORY, DDidPtr->DDDSource[i].memoryIdentifier, DDidPtr->DDDSource[i].SourceAddressOrDid, DDidPtr->DDDSource[i].Size);
            if( responseCode == DCM_E_POSITIVERESPONSE ) {
                if( (DDidPtr->DDDSource[i].Size + *Length) <= bufSize ) {
                    (void)Dcm_ReadMemory(DCM_INITIAL,DDidPtr->DDDSource[i].memoryIdentifier,
                                            DDidPtr->DDDSource[i].SourceAddressOrDid,
                                            DDidPtr->DDDSource[i].Size,
                                            nextDataSlot);
                    nextDataSlot += DDidPtr->DDDSource[i].Size;
                    *Length += DDidPtr->DDDSource[i].Size;
                }
                else {
                    responseCode = DCM_E_RESPONSETOOLONG;
                }
            }
        }
        else if(DDidPtr->DDDSource[i].DDDTpyeID == DCM_DDD_SOURCE_DID) {
            
            if(lookupNonDynamicDid(DDidPtr->DDDSource[i].SourceAddressOrDid,&SourceDidPtr) && (NULL_PTR != SourceDidPtr->DspDidInfoRef->DspDidAccess.DspDidRead) ) {
                if(DspCheckSecurityLevel(SourceDidPtr->DspDidInfoRef->DspDidAccess.DspDidRead->DspDidReadSecurityLevelRef) != TRUE) {
                    responseCode = DCM_E_SECURITYACCESSDENIED;
                } else {
                    for( uint16 signal = 0; signal < SourceDidPtr->DspNofSignals; signal++ ) {
                        uint8 *didDataStart = nextDataSlot;
                        signalPtr = &SourceDidPtr->DspSignalRef[signal];
                        if( signalPtr->DspSignalDataRef->DspDataInfoRef->DspDidFixedLength || (DATA_PORT_SR == signalPtr->DspSignalDataRef->DspDataUsePort)) {
                            SourceDataLength = GetNofAffectedBytes(signalPtr->DspSignalDataRef->DspDataEndianess, signalPtr->DspSignalBitPosition, signalPtr->DspSignalDataRef->DspDataBitSize);
                        } else if( NULL_PTR != signalPtr->DspSignalDataRef->DspDataReadDataLengthFnc ) {
                            (void)signalPtr->DspSignalDataRef->DspDataReadDataLengthFnc(&SourceDataLength);
                        }
                        if( (SourceDidPtr->DspDidDataByteSize + *Length) <= bufSize ) {
                            if( (NULL_PTR != signalPtr->DspSignalDataRef->DspDataReadDataFnc.SynchDataReadFnc) && (SourceDataLength != 0) && (DCM_E_POSITIVERESPONSE == responseCode) ) {
                                if((DATA_PORT_SYNCH == signalPtr->DspSignalDataRef->DspDataUsePort) || (DATA_PORT_ECU_SIGNAL == signalPtr->DspSignalDataRef->DspDataUsePort)) {
                                    (void)signalPtr->DspSignalDataRef->DspDataReadDataFnc.SynchDataReadFnc(didDataStart + (signalPtr->DspSignalBitPosition / 8));
                                } else if(DATA_PORT_ASYNCH == signalPtr->DspSignalDataRef->DspDataUsePort) {
                                    (void)signalPtr->DspSignalDataRef->DspDataReadDataFnc.AsynchDataReadFnc(DCM_INITIAL, didDataStart + (signalPtr->DspSignalBitPosition / 8));
                                }
                                else if(DATA_PORT_SR == signalPtr->DspSignalDataRef->DspDataUsePort) {
                                    (void)ReadSRSignal(signalPtr->DspSignalDataRef, signalPtr->DspSignalBitPosition, didDataStart);
                                }

                            } else {
                                responseCode = DCM_E_REQUESTOUTOFRANGE;
                            }
                        } else {
                            /* IMPROVEMENT: Actually response may fit but implementation reads the
                             * complete DID to the buffer. */
                            responseCode = DCM_E_RESPONSETOOLONG;
                        }
                    }
                    if( DCM_E_POSITIVERESPONSE == responseCode ) {
                        for(dataCount = 0; dataCount < DDidPtr->DDDSource[i].Size; dataCount++) {
                            /* Shifting the data left by position (position 1 means index 0) */
                            nextDataSlot[dataCount] = nextDataSlot[dataCount + DDidPtr->DDDSource[i].formatOrPosition - 1];
                        }
                        nextDataSlot += DDidPtr->DDDSource[i].Size;
                        *Length += DDidPtr->DDDSource[i].Size;
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

#ifdef DCM_USE_SERVICE_READDATABYPERIODICIDENTIFIER
static Dcm_NegativeResponseCodeType checkDDDConditions(Dcm_DspDDDType *DDidPtr, uint16 *Length)
{
    const Dcm_DspDidType *SourceDidPtr = NULL_PTR;
    Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;
    *Length = 0u;

    for(uint8 i = 0u;(i < DCM_MAX_DDDSOURCE_NUMBER) && (DDidPtr->DDDSource[i].formatOrPosition != 0)
        &&(responseCode == DCM_E_POSITIVERESPONSE);i++)
    {
        *Length += DDidPtr->DDDSource[i].Size;
        if(DDidPtr->DDDSource[i].DDDTpyeID == DCM_DDD_SOURCE_ADDRESS) {
            responseCode = checkAddressRange(DCM_READ_MEMORY, DDidPtr->DDDSource[i].memoryIdentifier, DDidPtr->DDDSource[i].SourceAddressOrDid, DDidPtr->DDDSource[i].Size);
        } else if(DDidPtr->DDDSource[i].DDDTpyeID == DCM_DDD_SOURCE_DID) {
            if( lookupNonDynamicDid(DDidPtr->DDDSource[i].SourceAddressOrDid,&SourceDidPtr) && (NULL_PTR != SourceDidPtr->DspDidInfoRef->DspDidAccess.DspDidRead) ) {
                if( DspCheckSessionLevel(SourceDidPtr->DspDidInfoRef->DspDidAccess.DspDidRead->DspDidReadSessionRef) ) {
                    if( !DspCheckSecurityLevel(SourceDidPtr->DspDidInfoRef->DspDidAccess.DspDidRead->DspDidReadSecurityLevelRef) ) {
                        responseCode = DCM_E_SECURITYACCESSDENIED;
                    }
                } else {
                    responseCode = DCM_E_REQUESTOUTOFRANGE;
                }
            } else {
                responseCode = DCM_E_REQUESTOUTOFRANGE;
            }
        } else {
            responseCode = DCM_E_REQUESTOUTOFRANGE;
        }
    }
    return responseCode;
}
#endif

void DspUdsReadDataByIdentifier(const PduInfoType *pduRxData, PduInfoType *pduTxData)
{
    /* @req DCM253 */
    /* !req DCM561 */
    Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;
    uint16 nrOfDids;
    uint16 didNr;
    const Dcm_DspDidType *didPtr = NULL_PTR;
    Dcm_DspDDDType *DDidPtr = NULL_PTR;
    uint16 txPos = 1u;
    uint16 i;
    uint16 Length = 0u;
    boolean noRequestedDidSupported = TRUE;
    ReadDidPendingStateType pendingState = DCM_READ_DID_IDLE;
    uint16 nofDidsRead = 0u;
    uint16 reqDidStartIndex = 0u;
    uint16 nofDidsReadInPendingReq = 0u;
    uint16 pendingDid = 0u;
    uint16 pendingDataLen = 0u;
    uint16 pendingSigIndex = 0u;
    uint16 pendingDataStartPos = 0u;

    if ( (((pduRxData->SduLength - 1) % 2) == 0) && ( 0 != (pduRxData->SduLength - 1))) {
        nrOfDids = (pduRxData->SduLength - 1) / 2;
        if( DCM_READ_DID_IDLE != dspUdsReadDidPending.state ) {
            pendingState = dspUdsReadDidPending.state;
            txPos = dspUdsReadDidPending.txWritePos;
            nofDidsReadInPendingReq = dspUdsReadDidPending.nofReadDids;
            reqDidStartIndex = dspUdsReadDidPending.reqDidIndex;
            pendingDataLen = dspUdsReadDidPending.pendingDataLength;
            pendingSigIndex = dspUdsReadDidPending.pendingSignalIndex;
            pendingDataStartPos = dspUdsReadDidPending.pendingDataStartPos;
        }
        /* IMPROVEMENT: Check security level and session for all dids before trying to read data */
        for (i = reqDidStartIndex; (i < nrOfDids) && (responseCode == DCM_E_POSITIVERESPONSE); i++) {
            didNr = (uint16)((uint16)pduRxData->SduDataPtr[1 + (i * 2)] << 8) + pduRxData->SduDataPtr[2 + (i * 2)];
            if (lookupNonDynamicDid(didNr, &didPtr)) {	/* @req DCM438 */
                noRequestedDidSupported = FALSE;
                /* IMPROVEMENT: Check if the did has data or did ref. If not NRC here? */
                /* !req DCM481 */
                /* !req DCM482 */
                /* !req DCM483 */
                responseCode = readDidData(didPtr, pduTxData, &txPos, &pendingState, &pendingDid, &pendingSigIndex, &pendingDataLen, &nofDidsRead, nofDidsReadInPendingReq, &pendingDataStartPos);
                if( DCM_E_RESPONSEPENDING == responseCode ) {
                    dspUdsReadDidPending.reqDidIndex = i;
                } else {
                    /* No pending response */
                    nofDidsReadInPendingReq = 0u;
                    nofDidsRead = 0u;
                }
            } else if(LookupDDD(didNr,(const Dcm_DspDDDType **)&DDidPtr) == TRUE) {
                noRequestedDidSupported = FALSE;
                /* @req DCM651 */
                /* @req DCM652 */
                /* @req DCM653 */
                pduTxData->SduDataPtr[txPos++] = (uint8)((DDidPtr->DynamicallyDid>>8) & 0xFF);
                pduTxData->SduDataPtr[txPos++] = (uint8)(DDidPtr->DynamicallyDid & 0xFF);
                responseCode = readDDDData(DDidPtr, &(pduTxData->SduDataPtr[txPos]), pduTxData->SduLength - txPos, &Length);
                txPos += Length;
            } else {
                /* DID not found. */
            }
        }
    } else {
        responseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
    }
    if( (responseCode != DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT) && noRequestedDidSupported ) {
        /* @req DCM438 */
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
        dspUdsReadDidPending.pendingDataLength = pendingDataLen;
        dspUdsReadDidPending.pendingSignalIndex = pendingSigIndex;
        dspUdsReadDidPending.pendingDataStartPos = pendingDataStartPos;
    } else {
        dspUdsReadDidPending.state = DCM_READ_DID_IDLE;
        dspUdsReadDidPending.nofReadDids = 0;
        DsdDspProcessingDone(responseCode);
    }
}


static Dcm_NegativeResponseCodeType readDidScalingData(const Dcm_DspDidType *didPtr, const PduInfoType *pduTxData, uint16 *txPos)
{
    Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;
    const Dcm_DspDataType *dataPtr;

    if( (*txPos + didPtr->DspDidDataScalingInfoSize + 2) <= pduTxData->SduLength ) {
        pduTxData->SduDataPtr[(*txPos)++] = (didPtr->DspDidIdentifier >> 8) & 0xFFu;
        pduTxData->SduDataPtr[(*txPos)++] = didPtr->DspDidIdentifier & 0xFFu;

        for( uint16 i = 0u; (i < didPtr->DspNofSignals) && (DCM_E_POSITIVERESPONSE == responseCode); i++ ) {
            Std_ReturnType result;
            Dcm_NegativeResponseCodeType errorCode;
            dataPtr = didPtr->DspSignalRef[i].DspSignalDataRef;
            if( NULL_PTR != dataPtr->DspDataGetScalingInfoFnc ) {
                /* @req DCM394 */
                result = dataPtr->DspDataGetScalingInfoFnc(DCM_INITIAL, &pduTxData->SduDataPtr[*txPos], &errorCode);
                *txPos += dataPtr->DspDataInfoRef->DspDidScalingInfoSize;
                if ((result != E_OK) || (errorCode != DCM_E_POSITIVERESPONSE)) {
                    responseCode = DCM_E_CONDITIONSNOTCORRECT;
                }
            } else {
                /* No scaling info function. Is it correct to give negative response here? */
                responseCode = DCM_E_REQUESTOUTOFRANGE;
            }
        }
    } else {
        /* Not enough room in tx buffer */
        responseCode = DCM_E_RESPONSETOOLONG;
    }

    return responseCode;
}

void DspUdsReadScalingDataByIdentifier(const PduInfoType *pduRxData, PduInfoType *pduTxData)
{
    /* @req DCM258 */
    Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;
    uint16 didNr;
    const Dcm_DspDidType *didPtr = NULL_PTR;

    uint16 txPos = 1;

    if (pduRxData->SduLength == 3) {
        didNr = (uint16)((uint16)pduRxData->SduDataPtr[1] << 8) + pduRxData->SduDataPtr[2];
        if (lookupNonDynamicDid(didNr, &didPtr)) {
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

#ifdef DCM_USE_SERVICE_WRITEDATABYIDENTIFIER
#if defined(USE_NVM) && defined(DCM_USE_NVM_DID)
/**
 * Writes data to an NvM block
 * @param blockId
 * @param data
 * @param isPending
 * @return
 */
static Dcm_NegativeResponseCodeType writeDataToNvMBlock(NvM_BlockIdType blockId, uint8 *data, boolean isPending)
{
    Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;
    if( isPending ){
        NvM_RequestResultType errorStatus = NVM_REQ_NOT_OK;
        if (E_OK != NvM_GetErrorStatus(blockId, &errorStatus)){
            DCM_DET_REPORTERROR(DCM_GLOBAL_ID, DCM_E_UNEXPECTED_RESPONSE);
            responseCode = DCM_E_GENERALREJECT;
        } else {
            if (errorStatus == NVM_REQ_PENDING){
                responseCode = DCM_E_RESPONSEPENDING;
            } else if (errorStatus == NVM_REQ_OK) {
                responseCode = DCM_E_POSITIVERESPONSE;
            } else {
                DCM_DET_REPORTERROR(DCM_GLOBAL_ID, DCM_E_UNEXPECTED_RESPONSE);
                responseCode = DCM_E_GENERALREJECT;
            }
        }
        if( DCM_E_RESPONSEPENDING != responseCode ) {
            /* Done or something failed. Lock block. */
            NvM_SetBlockLockStatus(blockId, TRUE);
        }
    } else {
        NvM_SetBlockLockStatus(blockId, FALSE);
        if (E_OK == NvM_WriteBlock(blockId, data)) {
            responseCode = DCM_E_RESPONSEPENDING;
        } else {
            NvM_SetBlockLockStatus(blockId, TRUE);
            DCM_DET_REPORTERROR(DCM_GLOBAL_ID, DCM_E_UNEXPECTED_RESPONSE);
            responseCode = DCM_E_GENERALREJECT;
        }
    }
    return responseCode;
}
#endif

static Dcm_NegativeResponseCodeType writeDidData(const Dcm_DspDidType *didPtr, const PduInfoType *pduRxData, uint16 writeDidLen)
{
    Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;
    const Dcm_DspSignalType *signalPtr;
    if (didPtr->DspDidInfoRef->DspDidAccess.DspDidWrite != NULL_PTR ) {	/* @req DCM468 */
        if (DspCheckSessionLevel(didPtr->DspDidInfoRef->DspDidAccess.DspDidWrite->DspDidWriteSessionRef)) { /* @req DCM469 */
            if (DspCheckSecurityLevel(didPtr->DspDidInfoRef->DspDidAccess.DspDidWrite->DspDidWriteSecurityLevelRef)) { /* @req DCM470 */
                uint16 dataLen = 0;
                /* Check the size */
                /* @req DCM473 */
                if( (NULL_PTR != didPtr->DspSignalRef) &&
                        ( (didPtr->DspSignalRef[0].DspSignalDataRef->DspDataInfoRef->DspDidFixedLength && (writeDidLen == didPtr->DspDidDataByteSize)) ||
                          (!didPtr->DspSignalRef[0].DspSignalDataRef->DspDataInfoRef->DspDidFixedLength)) ) {
                    for( uint16 i = 0u; i < (didPtr->DspNofSignals) && (DCM_E_POSITIVERESPONSE == responseCode); i++ ) {
                        signalPtr = &didPtr->DspSignalRef[i];
                        if( !signalPtr->DspSignalDataRef->DspDataInfoRef->DspDidFixedLength ) {
                            dataLen = writeDidLen;
                        }
                        Std_ReturnType result;
                        if(signalPtr->DspSignalDataRef->DspDataUsePort == DATA_PORT_BLOCK_ID) {
#if defined(USE_NVM) && defined(DCM_USE_NVM_DID)
                            /* @req DCM541 */
                            responseCode = writeDataToNvMBlock(signalPtr->DspSignalDataRef->DspNvmUseBlockID,
                                                                &pduRxData->SduDataPtr[3 + (signalPtr->DspSignalBitPosition / 8)],
                                                                ((dspUdsWriteDidPending.state == DCM_GENERAL_PENDING)? TRUE: FALSE));
#endif
                        } else {
                            Dcm_OpStatusType opStatus = (Dcm_OpStatusType)DCM_INITIAL;
                            if(dspUdsWriteDidPending.state == DCM_GENERAL_PENDING){
                                opStatus = (Dcm_OpStatusType)DCM_PENDING;
                            }
                            if( NULL_PTR != signalPtr->DspSignalDataRef->DspDataWriteDataFnc.FixLenDataWriteFnc ) {
                                if(DATA_PORT_SR == signalPtr->DspSignalDataRef->DspDataUsePort) {
                                    result = WriteSRSignal(signalPtr->DspSignalDataRef, signalPtr->DspSignalBitPosition, &pduRxData->SduDataPtr[3]);
                                }
                                else {
                                    if(signalPtr->DspSignalDataRef->DspDataInfoRef->DspDidFixedLength) { /* @req DCM794 */
                                        /* @req DCM395 */
                                        result = signalPtr->DspSignalDataRef->DspDataWriteDataFnc.FixLenDataWriteFnc(&pduRxData->SduDataPtr[3 + (signalPtr->DspSignalBitPosition / 8)], opStatus, &responseCode);
                                    } else {
                                        /* @req DCM395 */
                                        result = signalPtr->DspSignalDataRef->DspDataWriteDataFnc.DynLenDataWriteFnc(&pduRxData->SduDataPtr[3 + (signalPtr->DspSignalBitPosition / 8)], dataLen, opStatus, &responseCode);
                                    }
                                }
                                if( DCM_E_RESPONSEPENDING == responseCode || E_PENDING == result ) {
                                    responseCode = DCM_E_RESPONSEPENDING;
                                } else if( result != E_OK && responseCode == DCM_E_POSITIVERESPONSE ) {
                                    responseCode = DCM_E_CONDITIONSNOTCORRECT;
                                }
                            } else {
                                /* No write function */
                                responseCode = DCM_E_REQUESTOUTOFRANGE;
                            }
                        }
                    }
                } else { // Length incorrect
                    responseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
                }
            } else {    // Not allowed in current security level
                responseCode = DCM_E_SECURITYACCESSDENIED;
            }
        } else {    // Not allowed in current session
            responseCode = DCM_E_REQUESTOUTOFRANGE;
        }
    } else {    // Read access not configured
        responseCode = DCM_E_REQUESTOUTOFRANGE;
    }

    return responseCode;
}

void DspUdsWriteDataByIdentifier(const PduInfoType *pduRxData, PduInfoType *pduTxData)
{
    /* @req DCM255 */
    Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;
    uint16 didNr;
    const Dcm_DspDidType *didPtr = NULL_PTR;
    uint16 didDataLength;

    didDataLength = pduRxData->SduLength - 3;
    didNr = (uint16)((uint16)pduRxData->SduDataPtr[1] << 8) + pduRxData->SduDataPtr[2];
    /* Check that did is supported.*/
    if (lookupNonDynamicDid(didNr, &didPtr)) {	/* @req DCM467 */
        responseCode = writeDidData(didPtr, pduRxData, didDataLength);/* !req DCM562 */
    } else { /* DID not supported */
        responseCode = DCM_E_REQUESTOUTOFRANGE;
    }

    if( DCM_E_RESPONSEPENDING != responseCode ) {
        if (responseCode == DCM_E_POSITIVERESPONSE) {
            pduTxData->SduLength = 3;
            pduTxData->SduDataPtr[1] = (uint8)((didNr >> 8) & 0xFFu);
            pduTxData->SduDataPtr[2] = (uint8)(didNr & 0xFFu);
        }

        dspUdsWriteDidPending.state = DCM_GENERAL_IDLE;
        DsdDspProcessingDone(responseCode);
    } else {
        dspUdsWriteDidPending.state = DCM_GENERAL_PENDING;
        dspUdsWriteDidPending.pduRxData = pduRxData;
        dspUdsWriteDidPending.pduTxData = pduTxData;
    }
}
#endif

/**
 * Cancels a pending security access request
 * @param pduRxData
 * @param pduTxData
 */
static void DspCancelPendingSecurityAccess(const PduInfoType *pduRxData, PduInfoType *pduTxData)
{
    // Check sub function range (0x01 to 0x42)
    if ((pduRxData->SduDataPtr[1] >= 0x01) && (pduRxData->SduDataPtr[1] <= 0x42)) {
        boolean isRequestSeed = ((pduRxData->SduDataPtr[1] & 0x01u) == 0x01u)? TRUE: FALSE;
        Dcm_SecLevelType requestedSecurityLevel = (pduRxData->SduDataPtr[1]+1)/2;

        if (isRequestSeed) {
            (void)DspUdsSecurityAccessGetSeedSubFnc(pduRxData, pduTxData, requestedSecurityLevel, TRUE);
        }
        else {
            (void)DspUdsSecurityAccessCompareKeySubFnc(pduRxData, pduTxData, requestedSecurityLevel, TRUE);
        }
    }
}

static Dcm_NegativeResponseCodeType DspUdsSecurityAccessGetSeedSubFnc (const PduInfoType *pduRxData, PduInfoType *pduTxData, Dcm_SecLevelType requestedSecurityLevel, boolean isCancel) {

    Dcm_NegativeResponseCodeType getSeedErrorCode;
    Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;
    uint8 cntSecRow = 0;
    Dcm_OpStatusType opStatus = (Dcm_OpStatusType)DCM_INITIAL;
    if(dspUdsSecurityAccessPending.state == DCM_GENERAL_PENDING){
        if( isCancel ) {
            opStatus = (Dcm_OpStatusType)DCM_CANCEL;
        } else {
            opStatus = (Dcm_OpStatusType)DCM_PENDING;
        }
    }

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
        if (dspUdsSecurityAccesData.secFalseAttemptChk[dspUdsSecurityAccesData.currSecLevIdx].startDelayTimer != DELAY_TIMER_DEACTIVE) {
            responseCode = DCM_E_REQUIREDTIMEDELAYNOTEXPIRED;
        }
        else
#endif
        {
            // Check length
            if (pduRxData->SduLength == (2 + securityRow->DspSecurityADRSize)) {    /* @req DCM321 RequestSeed */
                Dcm_SecLevelType activeSecLevel;
                Std_ReturnType result;
                result = Dcm_GetSecurityLevel(&activeSecLevel);
                if (result == E_OK) {
                    if( (2 + securityRow->DspSecuritySeedSize) <= pduTxData->SduLength ) {
                        if (requestedSecurityLevel == activeSecLevel) {     /* @req DCM323 */
                            pduTxData->SduDataPtr[1] = pduRxData->SduDataPtr[1];
                            // If same level set the seed to zeroes
                            memset(&pduTxData->SduDataPtr[2], 0, securityRow->DspSecuritySeedSize);
                            pduTxData->SduLength = 2 + securityRow->DspSecuritySeedSize;
                        } else {
                            // New security level ask for seed
                            if ((securityRow->GetSeed.getSeedWithoutRecord != NULL_PTR) || (securityRow->GetSeed.getSeedWithRecord != NULL_PTR)) {
                                Std_ReturnType getSeedResult;
                                if(securityRow->DspSecurityADRSize > 0) {
                                    /* @req DCM324 RequestSeed */
                                    getSeedResult = securityRow->GetSeed.getSeedWithRecord(&pduRxData->SduDataPtr[2], opStatus, &pduTxData->SduDataPtr[2], &getSeedErrorCode);
                                } else {
                                    /* @req DCM324 RequestSeed */
                                    getSeedResult = securityRow->GetSeed.getSeedWithoutRecord(opStatus, &pduTxData->SduDataPtr[2], &getSeedErrorCode);
                                }
                                if (getSeedResult == E_PENDING){
                                    responseCode = DCM_E_RESPONSEPENDING;
                                }
                                else if (getSeedResult == E_OK) {
                                    // Everything ok add sub function to tx message and send it.
                                    pduTxData->SduDataPtr[1] = pduRxData->SduDataPtr[1];
                                    pduTxData->SduLength = 2 + securityRow->DspSecuritySeedSize;

                                    dspUdsSecurityAccesData.reqSecLevel = requestedSecurityLevel;
                                    dspUdsSecurityAccesData.reqSecLevelRef = securityRow;
                                    dspUdsSecurityAccesData.reqInProgress = TRUE;
                                }
                                else {
                                    // GetSeed returned not ok
                                    responseCode = getSeedErrorCode; /* @req DCM659 */
                                }
                            } else {
                                responseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
                            }
                        }
                    } else {
                        /* Tx buffer not big enough */
                        responseCode = DCM_E_RESPONSETOOLONG;
                    }
                } else {
                    // NOTE: What to do?
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
        responseCode = DCM_E_SUBFUNCTIONNOTSUPPORTED;
    }

    return responseCode;
}

// CompareKey message
static Dcm_NegativeResponseCodeType DspUdsSecurityAccessCompareKeySubFnc (const PduInfoType *pduRxData, PduInfoType *pduTxData, Dcm_SecLevelType requestedSecurityLevel, boolean isCancel) {

    Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;
    Dcm_OpStatusType opStatus = (Dcm_OpStatusType)DCM_INITIAL;
    if(dspUdsSecurityAccessPending.state == DCM_GENERAL_PENDING){
        if( isCancel ) {
            opStatus = (Dcm_OpStatusType)DCM_CANCEL;
        } else {
            opStatus = (Dcm_OpStatusType)DCM_PENDING;
        }
    }

    if (requestedSecurityLevel == dspUdsSecurityAccesData.reqSecLevel) {
        /* Check whether senkey message is sent according to a valid sequence */
        if (dspUdsSecurityAccesData.reqInProgress) {

#if (DCM_SECURITY_EOL_INDEX != 0)
            //Check if a wait is required before accepting a request
            if (dspUdsSecurityAccesData.secFalseAttemptChk[dspUdsSecurityAccesData.currSecLevIdx].startDelayTimer != DELAY_TIMER_DEACTIVE) {
                responseCode = DCM_E_REQUIREDTIMEDELAYNOTEXPIRED;
            } else

#endif
            {
                if (pduRxData->SduLength == (2 + dspUdsSecurityAccesData.reqSecLevelRef->DspSecurityKeySize)) { /* @req DCM321 SendKey */
                    if ((dspUdsSecurityAccesData.reqSecLevelRef->CompareKey != NULL_PTR)) {
                        Std_ReturnType compareKeyResult;
                        compareKeyResult = dspUdsSecurityAccesData.reqSecLevelRef->CompareKey(&pduRxData->SduDataPtr[2], opStatus); /* @req DCM324 SendKey */
                        if( isCancel ) {
                            /* Ignore compareKeyResult on cancel  */
                            responseCode = DCM_E_POSITIVERESPONSE;
                        } else if (compareKeyResult == E_PENDING) {
                            responseCode = DCM_E_RESPONSEPENDING;
                        } else if (compareKeyResult == E_OK) {
                            /* Client should reiterate the process of getseed msg, if sendkey fails- ISO14229 */
                            dspUdsSecurityAccesData.reqInProgress = FALSE;
                          // Request accepted
                           // Kill timer
                           DslInternal_SetSecurityLevel(dspUdsSecurityAccesData.reqSecLevelRef->DspSecurityLevel); /* @req DCM325 */
                           pduTxData->SduDataPtr[1] = pduRxData->SduDataPtr[1];
                           pduTxData->SduLength = 2;
                        } else {
                           /* Client should reiterate the process of getseed msg, if sendkey fails- ISO14229 */
                           dspUdsSecurityAccesData.reqInProgress = FALSE;
                           responseCode = DCM_E_INVALIDKEY; /* @req DCM660 */
                        }
                    } else {
                       responseCode = DCM_E_CONDITIONSNOTCORRECT;
                    }
                } else {
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
                        if (Dcm_ConfigPtr->Dsp->DspSecurity->DspSecurityRow[dspUdsSecurityAccesData.currSecLevIdx].DspSecurityDelayTime > 0) {
                            responseCode = DCM_E_EXCEEDNUMBEROFATTEMPTS; // Delay time is optional according to ISO spec and so is the NRC
                        }
                    }
                }
#endif
            }
        } else {
            // sendKey request without a preceding requestSeed
            responseCode = DCM_E_REQUESTSEQUENCEERROR;
        }
    } else {
        // sendKey request without a preceding requestSeed
#if defined(CFG_DCM_SECURITY_ACCESS_NRC_FIX)
        /* Should this really give subFunctionNotSupported? */
        responseCode = DCM_E_SUBFUNCTIONNOTSUPPORTED;
#else
        responseCode = DCM_E_REQUESTSEQUENCEERROR;
#endif
    }

    return responseCode;
}

void DspUdsSecurityAccess(const PduInfoType *pduRxData, PduInfoType *pduTxData)
{
    /* @req DCM252 */
    Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;

    if( pduRxData->SduLength >= 2u) {
        /* Check sub function range (0x01 to 0x42) */
        if ((pduRxData->SduDataPtr[1] >= 0x01) && (pduRxData->SduDataPtr[1] <= 0x42)) {
            boolean isRequestSeed = pduRxData->SduDataPtr[1] & 0x01u;
            Dcm_SecLevelType requestedSecurityLevel = (pduRxData->SduDataPtr[1]+1)/2;
            if (isRequestSeed) {
                responseCode = DspUdsSecurityAccessGetSeedSubFnc(pduRxData, pduTxData, requestedSecurityLevel,FALSE);
            } else {
                responseCode = DspUdsSecurityAccessCompareKeySubFnc(pduRxData, pduTxData, requestedSecurityLevel,FALSE);
            }
        } else {
            responseCode = DCM_E_SUBFUNCTIONNOTSUPPORTED;
        }
    }
    else {
        responseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
    }

    if( DCM_E_RESPONSEPENDING != responseCode ) {
        dspUdsSecurityAccessPending.state = DCM_GENERAL_IDLE;
        DsdDspProcessingDone(responseCode);
    } else {
        dspUdsSecurityAccessPending.state = DCM_GENERAL_PENDING;
        dspUdsSecurityAccessPending.pduRxData = pduRxData;
        dspUdsSecurityAccessPending.pduTxData = pduTxData;
    }
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

static void SetOpStatusDependingOnState(DspUdsGeneralPendingType *pGeneralPending,Dcm_OpStatusType *opStatus, boolean isCancel)
{
    if(pGeneralPending->state == DCM_GENERAL_PENDING || pGeneralPending->state == DCM_GENERAL_FORCE_RCRRP){
        if( isCancel ) {
            *opStatus = (Dcm_OpStatusType)DCM_CANCEL;
        } else {
            if(pGeneralPending->state == DCM_GENERAL_FORCE_RCRRP) {
                *opStatus = (Dcm_OpStatusType)DCM_FORCE_RCRRP_OK;
            } else {
                *opStatus = (Dcm_OpStatusType)DCM_PENDING;
            }
        }
    }
}

static Dcm_NegativeResponseCodeType startRoutine(const Dcm_DspRoutineType *routinePtr, const PduInfoType *pduRxData, PduInfoType *pduTxData, boolean isCancel)
{
    Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;
    Std_ReturnType routineResult;
    Dcm_OpStatusType opStatus = (Dcm_OpStatusType)DCM_INITIAL;
    SetOpStatusDependingOnState(&dspUdsRoutineControlPending,&opStatus, isCancel);

    // startRoutine
    if ((routinePtr->DspStartRoutineFnc != NULL_PTR) && (routinePtr->DspRoutineInfoRef->DspStartRoutine != NULL_PTR)) {
        if ( (routinePtr->DspRoutineInfoRef->DspStartRoutine->DspStartRoutineCtrlOptRecSize + 4) <= pduRxData->SduLength ) {
            if ((routinePtr->DspRoutineInfoRef->DspStartRoutine->DspStartRoutineStsOptRecSize + 4) <= pduTxData->SduLength ) {
                uint16 currentDataLength = pduRxData->SduLength - 4;
                /* @req DCM400 */
                /* @req DCM401 */
                routineResult = routinePtr->DspStartRoutineFnc(&pduRxData->SduDataPtr[4], opStatus, &pduTxData->SduDataPtr[4], &currentDataLength, &responseCode, FALSE);
                if (routineResult == E_PENDING){
                    responseCode = DCM_E_RESPONSEPENDING;
                } else if (routineResult == E_FORCE_RCRRP) {
                    responseCode = DCM_E_FORCE_RCRRP;
                } else if (routineResult != E_OK) {
                    /* @req DCM668 */
                    if(DCM_E_POSITIVERESPONSE == responseCode) {
                        responseCode = DCM_E_CONDITIONSNOTCORRECT;
                    }
                } else {
                    responseCode = DCM_E_POSITIVERESPONSE;
                    pduTxData->SduLength = currentDataLength + 4;
                }

                if( (responseCode == DCM_E_POSITIVERESPONSE) && (pduTxData->SduLength > (routinePtr->DspRoutineInfoRef->DspStartRoutine->DspStartRoutineStsOptRecSize + 4)) ) {
                    DCM_DET_REPORTERROR(DCM_GLOBAL_ID, DCM_E_PARAM);
                    pduTxData->SduLength = routinePtr->DspRoutineInfoRef->DspStartRoutine->DspStartRoutineStsOptRecSize + 4;
                }
            } else {
                /* Tx buffer not big enough */
                responseCode = DCM_E_RESPONSETOOLONG;
            }
        } else {
            responseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
        }
    } else {
        responseCode = DCM_E_REQUESTOUTOFRANGE;
    }
    return responseCode;
}


static Dcm_NegativeResponseCodeType stopRoutine(const Dcm_DspRoutineType *routinePtr, const PduInfoType *pduRxData, PduInfoType *pduTxData, boolean isCancel)
{
    Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;
    Std_ReturnType routineResult;
    Dcm_OpStatusType opStatus = (Dcm_OpStatusType)DCM_INITIAL;
    SetOpStatusDependingOnState(&dspUdsRoutineControlPending,&opStatus, isCancel);

    // stopRoutine
    if ((routinePtr->DspStopRoutineFnc != NULL_PTR) && (routinePtr->DspRoutineInfoRef->DspRoutineStop != NULL_PTR)) {
        if ( (routinePtr->DspRoutineInfoRef->DspRoutineStop->DspStopRoutineCtrlOptRecSize + 4) <= pduRxData->SduLength ) {
            if ( (routinePtr->DspRoutineInfoRef->DspRoutineStop->DspStopRoutineStsOptRecSize + 4) <= pduTxData->SduLength ) {
                uint16 currentDataLength = pduRxData->SduLength - 4;
                /* @req DCM402 */
                /* @req DCM403 */
                routineResult = routinePtr->DspStopRoutineFnc(&pduRxData->SduDataPtr[4], opStatus, &pduTxData->SduDataPtr[4], &currentDataLength, &responseCode, FALSE);
                if (routineResult == E_PENDING){
                    responseCode = DCM_E_RESPONSEPENDING;
                } else if (routineResult == E_FORCE_RCRRP) {
                    responseCode = DCM_E_FORCE_RCRRP;
                } else if (routineResult != E_OK) {
                    /* @req DCM670 */
                    if(DCM_E_POSITIVERESPONSE == responseCode) {
                        responseCode = DCM_E_CONDITIONSNOTCORRECT;
                    }
                } else {
                    responseCode = DCM_E_POSITIVERESPONSE;
                    pduTxData->SduLength = currentDataLength + 4;
                }
                if(responseCode == DCM_E_POSITIVERESPONSE && pduTxData->SduLength > routinePtr->DspRoutineInfoRef->DspRoutineStop->DspStopRoutineStsOptRecSize + 4) {
                    DCM_DET_REPORTERROR(DCM_GLOBAL_ID, DCM_E_PARAM);
                    pduTxData->SduLength = routinePtr->DspRoutineInfoRef->DspRoutineStop->DspStopRoutineStsOptRecSize + 4;
                }
            } else {
                /* Tx buffer not big enough */
                responseCode = DCM_E_RESPONSETOOLONG;
            }
        } else {
            responseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
        }
    } else {
        /* @req 4.2.2/SWS_DCM_869 */
        responseCode = DCM_E_SUBFUNCTIONNOTSUPPORTED;
    }

    return responseCode;
}


static Dcm_NegativeResponseCodeType requestRoutineResults(const Dcm_DspRoutineType *routinePtr, PduInfoType *pduTxData, boolean isCancel)
{
    Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;
    Std_ReturnType routineResult;
    Dcm_OpStatusType opStatus = (Dcm_OpStatusType)DCM_INITIAL;
    SetOpStatusDependingOnState(&dspUdsRoutineControlPending,&opStatus, isCancel);

    // requestRoutineResults
    if ((routinePtr->DspRequestResultRoutineFnc != NULL_PTR) && (routinePtr->DspRoutineInfoRef->DspRoutineRequestRes != NULL_PTR)) {
        if ((routinePtr->DspRoutineInfoRef->DspRoutineRequestRes->DspReqResRtnCtrlOptRecSize + 4) <= pduTxData->SduLength) {
            uint16 currentDataLength = 0u;
            /* @req DCM404 */
            /* @req DCM405 */
            routineResult = routinePtr->DspRequestResultRoutineFnc(opStatus, &pduTxData->SduDataPtr[4], &currentDataLength, &responseCode, FALSE);
            if (routineResult == E_PENDING){
                responseCode = DCM_E_RESPONSEPENDING;
            } else if (routineResult == E_FORCE_RCRRP) {
                responseCode = DCM_E_FORCE_RCRRP;
            } else if (routineResult != E_OK) {
                /* @req DCM672 */
                if(DCM_E_POSITIVERESPONSE == responseCode) {
                    responseCode = DCM_E_CONDITIONSNOTCORRECT;
                }
            } else {
                responseCode = DCM_E_POSITIVERESPONSE;
                pduTxData->SduLength = currentDataLength + 4;
            }
            if( (responseCode == DCM_E_POSITIVERESPONSE) && (pduTxData->SduLength > routinePtr->DspRoutineInfoRef->DspRoutineRequestRes->DspReqResRtnCtrlOptRecSize + 4) ) {
                DCM_DET_REPORTERROR(DCM_GLOBAL_ID, DCM_E_PARAM);
                pduTxData->SduLength = routinePtr->DspRoutineInfoRef->DspRoutineRequestRes->DspReqResRtnCtrlOptRecSize + 4;
            }
        } else {
            responseCode = DCM_E_RESPONSETOOLONG;
        }
    } else {
        /* @req 4.2.2/SWS_DCM_869 */
        responseCode = DCM_E_SUBFUNCTIONNOTSUPPORTED;
    }

    return responseCode;
}

/**
 * Cancels a pending routine request
 * @param pduRxData
 * @param pduTxData
 */
static void DspCancelPendingRoutine(const PduInfoType *pduRxData, PduInfoType *pduTxData)
{
    uint8 subFunctionNumber = pduRxData->SduDataPtr[1];
    uint16 routineId = 0;
    if( (NULL != pduRxData) && (NULL != pduTxData)) {
        const Dcm_DspRoutineType *routinePtr = NULL_PTR;
        routineId = (uint16)((uint16)pduRxData->SduDataPtr[2] << 8) + pduRxData->SduDataPtr[3];
        if (lookupRoutine(routineId, &routinePtr)) {
            switch (subFunctionNumber) {
                case 0x01:  // startRoutine
                    (void)startRoutine(routinePtr, pduRxData, pduTxData, TRUE);
                    break;
                case 0x02:  // stopRoutine
                    (void)stopRoutine(routinePtr, pduRxData, pduTxData, TRUE);
                    break;
                case 0x03:  // requestRoutineResults
                    (void)requestRoutineResults(routinePtr, pduTxData, TRUE);
                    break;
                default:    // This shall never happen
                    break;
            }
        }
    } else {
        DCM_DET_REPORTERROR(DCM_GLOBAL_ID, DCM_E_PARAM);
    }
}
void DspUdsRoutineControl(const PduInfoType *pduRxData, PduInfoType *pduTxData)
{
    /* @req DCM257 */
    /* !req DCM701 */
    Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;
    uint8 subFunctionNumber = 0;
    uint16 routineId = 0u;
    const Dcm_DspRoutineType *routinePtr = NULL_PTR;

    if (pduRxData->SduLength >= 4) {
        subFunctionNumber = pduRxData->SduDataPtr[1];
        if ((subFunctionNumber > 0) && (subFunctionNumber < 4)) {
            routineId = (uint16)((uint16)pduRxData->SduDataPtr[2] << 8) + pduRxData->SduDataPtr[3];
            /* @req DCM568 */
            /* !req DCM569 */
            if (lookupRoutine(routineId, &routinePtr)) {
                if (DspCheckSessionLevel(routinePtr->DspRoutineInfoRef->DspRoutineAuthorization.DspRoutineSessionRef)) {/* @req DCM570 */
                    if (DspCheckSecurityLevel(routinePtr->DspRoutineInfoRef->DspRoutineAuthorization.DspRoutineSecurityLevelRef)) {/* @req DCM571 */
                        switch (subFunctionNumber) {
                            case 0x01:	// startRoutine
                                responseCode = startRoutine(routinePtr, pduRxData, pduTxData, FALSE);
                                break;

                            case 0x02:	// stopRoutine
                                responseCode = stopRoutine(routinePtr, pduRxData, pduTxData, FALSE);
                                break;

                            case 0x03:	// requestRoutineResults
                                responseCode =  requestRoutineResults(routinePtr, pduTxData, FALSE);
                                break;

                            default:	// This shall never happen
                                responseCode = DCM_E_SUBFUNCTIONNOTSUPPORTED;
                                break;
                        }
                    } else {	// Not allowed in current security level
                        responseCode = DCM_E_SECURITYACCESSDENIED;
                    }
                } else {	// Not allowed in current session
                    responseCode = DCM_E_REQUESTOUTOFRANGE;
                }
            } else {	// Unknown routine identifier
                responseCode = DCM_E_REQUESTOUTOFRANGE;
            }
        } else {	// Sub function not supported
            responseCode = DCM_E_SUBFUNCTIONNOTSUPPORTED;
        }
    } else {
        // Wrong length
        responseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
    }

    if( DCM_E_RESPONSEPENDING == responseCode ) {
        dspUdsRoutineControlPending.state = DCM_GENERAL_PENDING;
        dspUdsRoutineControlPending.pduRxData = pduRxData;
        dspUdsRoutineControlPending.pduTxData = pduTxData;
    } else if(DCM_E_FORCE_RCRRP == responseCode) {
        dspUdsRoutineControlPending.state = DCM_GENERAL_FORCE_RCRRP_AWAITING_SEND;
        dspUdsRoutineControlPending.pduRxData = pduRxData;
        dspUdsRoutineControlPending.pduTxData = pduTxData;
        /* @req DCM669 */
        /* @req DCM671 */
        /* @req DCM673 */
        DsdDspForceResponsePending();
    } else {
        if (responseCode == DCM_E_POSITIVERESPONSE) {
            // Add header to the positive response message
            pduTxData->SduDataPtr[1] = subFunctionNumber;
            pduTxData->SduDataPtr[2] = (routineId >> 8) & 0xFFu;
            pduTxData->SduDataPtr[3] = routineId & 0xFFu;
        }

        dspUdsRoutineControlPending.state = DCM_GENERAL_IDLE;
        DsdDspProcessingDone(responseCode);
    }
}


void DspUdsTesterPresent(const PduInfoType *pduRxData, PduInfoType *pduTxData)
{
    /* @req DCM251 */
    if (pduRxData->SduLength == 2) {
        switch (pduRxData->SduDataPtr[1]) {
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
    } else {
        // Wrong length
        DsdDspProcessingDone(DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT);
    }
}

#if defined(USE_DEM) && defined(DCM_USE_SERVICE_CONTROLDTCSETTING)
static void DspEnableDTCSetting(void)
{
    /* @req Dcm751 */
    if( DspDTCSetting.settingDisabled ) {
        Dcm_SesCtrlType currentSession = DCM_DEFAULT_SESSION;
        if( E_OK != DslGetSesCtrlType(&currentSession) ) {
            currentSession = DCM_DEFAULT_SESSION;
        }
        if(((DCM_DEFAULT_SESSION == currentSession) || !DsdDspCheckServiceSupportedInActiveSessionAndSecurity(SID_CONTROL_DTC_SETTING)) ) {
            if( DEM_CONTROL_DTC_STORAGE_OK != Dem_EnableDTCSetting(DspDTCSetting.dtcGroup, DspDTCSetting.dtcKind) ) {
                DCM_DET_REPORTERROR(DCM_GLOBAL_ID, DCM_E_UNEXPECTED_RESPONSE);
            }
            (void)Rte_Switch_DcmControlDTCSetting_DcmControlDTCSetting(RTE_MODE_DcmControlDTCSetting_ENABLEDTCSETTING);
            DspDTCSetting.settingDisabled = FALSE;
        }
    }
}
void DspUdsControlDtcSetting(const PduInfoType *pduRxData, PduInfoType *pduTxData)
{
    /* @req DCM249 */
    Dem_ReturnControlDTCStorageType resultCode;

    if (pduRxData->SduLength == 2) {
        switch (pduRxData->SduDataPtr[1]) {
            case 0x01:	// ON
                resultCode = Dem_EnableDTCSetting(DEM_DTC_GROUP_ALL_DTCS, DEM_DTC_KIND_ALL_DTCS);		/* @req DCM304 */
                if (resultCode == DEM_CONTROL_DTC_STORAGE_OK) {
                    pduTxData->SduDataPtr[1] = 0x01;
                    pduTxData->SduLength = 2;
                    DspDTCSetting.settingDisabled = FALSE;
                    /* @req DCM783 */
                    (void)Rte_Switch_DcmControlDTCSetting_DcmControlDTCSetting(RTE_MODE_DcmControlDTCSetting_ENABLEDTCSETTING);
                    DsdDspProcessingDone(DCM_E_POSITIVERESPONSE);
                } else {
                    DsdDspProcessingDone(DCM_E_REQUESTOUTOFRANGE);
                }
                break;

            case 0x02:	// OFF
                resultCode = Dem_DisableDTCSetting(DEM_DTC_GROUP_ALL_DTCS, DEM_DTC_KIND_ALL_DTCS);		/* @req DCM406 */
                if (resultCode == DEM_CONTROL_DTC_STORAGE_OK) {
                    pduTxData->SduDataPtr[1] = 0x02;
                    pduTxData->SduLength = 2;
                    DspDTCSetting.settingDisabled = TRUE;
                    DspDTCSetting.dtcGroup = DEM_DTC_GROUP_ALL_DTCS;
                    DspDTCSetting.dtcKind = DEM_DTC_KIND_ALL_DTCS;
                    /* @req DCM784 */
                    (void)Rte_Switch_DcmControlDTCSetting_DcmControlDTCSetting(RTE_MODE_DcmControlDTCSetting_DISABLEDTCSETTING);
                    DsdDspProcessingDone(DCM_E_POSITIVERESPONSE);
                } else {
                    DsdDspProcessingDone(DCM_E_REQUESTOUTOFRANGE);
                }
                break;

            default:
                DsdDspProcessingDone(DCM_E_SUBFUNCTIONNOTSUPPORTED);
                break;
        }
    } else {
        // Wrong length
        DsdDspProcessingDone(DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT);
    }
}
#endif


#ifdef DCM_USE_SERVICE_RESPONSEONEVENT
void DspResponseOnEvent(const PduInfoType *pduRxData, PduIdType rxPduId, PduInfoType *pduTxData) {

    uint8 eventWindowTime = 0;
    uint16 eventTypeRecord = 0;
    uint8 serviceToRespondToLength = 0;
    uint8 storageState = 0;

    /* The minimum request includes Sid, Sub function, and event window time */
    if (pduRxData->SduLength >= 3) {

        storageState = pduRxData->SduDataPtr[1] & 0x40; /* Bit 6 */
        eventWindowTime = pduRxData->SduDataPtr[2];

        switch ( pduRxData->SduDataPtr[1] & 0x3F) {

            case 0x00:  /* Stop */
                DsdDspProcessingDone(DCM_ROE_Stop(storageState, eventWindowTime, pduTxData));
                break;

            case 0x01:  /* onDTCStatusChanged */
                DsdDspProcessingDone(DCM_E_SUBFUNCTIONNOTSUPPORTED); //Not Implemented
                break;

            case 0x02:  /* onTimerInterrupt */
                DsdDspProcessingDone(DCM_E_SUBFUNCTIONNOTSUPPORTED); //Not Implemented
                break;

            case 0x03:  /* OnChangeOfDataIdentifier */
                /* @req Dcm522 */
                eventTypeRecord = (pduRxData->SduDataPtr[3] << 8) +  pduRxData->SduDataPtr[4];
                serviceToRespondToLength = pduRxData->SduLength - 5;
                DsdDspProcessingDone(
                        DCM_ROE_AddDataIdentifierEvent(eventWindowTime,
                                                       storageState,
                                                       eventTypeRecord,
                                                       &pduRxData->SduDataPtr[5],
                                                       serviceToRespondToLength,
                                                       pduTxData));
                break;

            case 0x04:  /* reportActiveEvents */
                DsdDspProcessingDone(DCM_ROE_GetEventList(storageState, pduTxData));
                break;

            case 0x05:  /* startResponseOnEvent */
                DsdDspProcessingDone(DCM_ROE_Start(storageState, eventWindowTime, rxPduId, pduTxData));
                break;

            case 0x06:  /* clearResponseOnEvent */
                DsdDspProcessingDone(DCM_ROE_ClearEventList(storageState, eventWindowTime, pduTxData));
                break;

            case 0x07:  /* onComparisonOfValue */
                DsdDspProcessingDone(DCM_E_SUBFUNCTIONNOTSUPPORTED); //Not Implemented
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


void DspDcmConfirmation(PduIdType confirmPduId, NotifResultType result)
{
    DslResetSessionTimeoutTimer(); /* @req DCM141 */
#if (DCM_USE_JUMP_TO_BOOT == STD_ON)
    if (confirmPduId == dspUdsSessionControlData.sessionPduId) {
        if( DCM_JTB_RESAPP_FINAL_RESPONSE_TX_CONFIRM == dspUdsSessionControlData.jumpToBootState ) {
            /* IMPROVEMENT: Add support for pending */
            /* @req 4.2.2/SWS_DCM_01179 */
            /* @req 4.2.2/SWS_DCM_01180 */
            /* @req 4.2.2/SWS_DCM_01181 */

            if( NTFRSLT_OK == result ) {
                if(E_OK == Dcm_SetProgConditions(&GlobalProgConditions)) {
                    (void)Rte_Switch_DcmEcuReset_DcmEcuReset(RTE_MODE_DcmEcuReset_EXECUTE);
                }
                else {
                    /* Final response has already been sent. */
                    DCM_DET_REPORTERROR(DCM_GLOBAL_ID, DCM_E_INTEGRATION_ERROR);
                }
            }
        }
        dspUdsSessionControlData.jumpToBootState = DCM_JTB_IDLE;
    }
#endif

    if (confirmPduId == dspUdsEcuResetData.resetPduId) {
        if ( DCM_DSP_RESET_WAIT_TX_CONF == dspUdsEcuResetData.resetPending) {
            if( NTFRSLT_OK == result ) {
                /* IMPROVEMENT: Should be a call to SchM */
                (void)Rte_Switch_DcmEcuReset_DcmEcuReset(RTE_MODE_DcmEcuReset_EXECUTE);/* @req DCM594 */
            }
            dspUdsEcuResetData.resetPending = DCM_DSP_RESET_NO_RESET;
        }
    }

    if ( TRUE == dspUdsSessionControlData.pendingSessionChange ) {
        if (confirmPduId == dspUdsSessionControlData.sessionPduId) {
            if( NTFRSLT_OK == result ) {
                /* @req DCM311 */
                DslSetSesCtrlType(dspUdsSessionControlData.session);
            }
            dspUdsSessionControlData.pendingSessionChange = FALSE;
        }
    }
#ifdef DCM_USE_SERVICE_COMMUNICATIONCONTROL
    if(communicationControlData.comControlPending) {
        if( confirmPduId == communicationControlData.confirmPdu ) {
            if(NTFRSLT_OK == result) {
                (void)DspInternalCommunicationControl(communicationControlData.subFunction, communicationControlData.subnet, communicationControlData.comType, communicationControlData.rxPdu, TRUE);
            }
            communicationControlData.comControlPending = FALSE;
        }
    }
#endif
}

#if (DCM_USE_JUMP_TO_BOOT == STD_ON) || defined(DCM_USE_SERVICE_LINKCONTROL)
void DspResponsePendingConfirmed(PduIdType confirmPduId)
{
    if ( (DCM_JTB_WAIT_RESPONSE_PENDING_TX_CONFIRM == dspUdsSessionControlData.jumpToBootState) || (DCM_JTB_RESAPP_WAIT_RESPONSE_PENDING_TX_CONFIRM == dspUdsSessionControlData.jumpToBootState) ) {
        if (confirmPduId == dspUdsSessionControlData.sessionPduId) {
            if( DCM_JTB_WAIT_RESPONSE_PENDING_TX_CONFIRM == dspUdsSessionControlData.jumpToBootState ) {
                /* @req DCM654 */
                /* @req DCM535 */
                /* IMPROVEMENT: Add support for pending */
                Std_ReturnType status = E_NOT_OK;
                if(E_OK == Dcm_SetProgConditions(&GlobalProgConditions)) {

                    if(E_OK == Rte_Switch_DcmEcuReset_DcmEcuReset(RTE_MODE_DcmEcuReset_EXECUTE)) {
                        status = E_OK;
                    }
                }
                if( E_OK != status ) {
                    /* @req DCM715 */
                    DsdDspProcessingDone(DCM_E_CONDITIONSNOTCORRECT);
                }
                dspUdsSessionControlData.jumpToBootState = DCM_JTB_IDLE;
            }
            else if (DCM_JTB_RESAPP_WAIT_RESPONSE_PENDING_TX_CONFIRM == dspUdsSessionControlData.jumpToBootState ){
                dspUdsSessionControlData.jumpToBootState = DCM_JTB_RESAPP_ASSEMBLE_FINAL_RESPONSE;
            }
            else {
                /* Do nothing */
            }
        }
    }
#if defined(DCM_USE_SERVICE_LINKCONTROL)
    if( DCM_JTB_WAIT_RESPONSE_PENDING_TX_CONFIRM == dspUdsLinkControlData.jumpToBootState ) {
        Std_ReturnType status = E_NOT_OK;

        if(E_OK == Dcm_SetProgConditions(&GlobalProgConditions)) {
                if(E_OK == Rte_Switch_DcmEcuReset_DcmEcuReset(RTE_MODE_DcmEcuReset_EXECUTE)) {
                    status = E_OK;
                }
        }
        if( E_OK != status ) {
            /* @req DCM715 */
            DsdDspProcessingDone(DCM_E_CONDITIONSNOTCORRECT);
        }
        dspUdsLinkControlData.jumpToBootState = DCM_JTB_IDLE;
    }
#endif
}
#endif

static Dcm_NegativeResponseCodeType readMemoryData( Dcm_OpStatusType *OpStatus,
                                                    uint8 memoryIdentifier,
                                                    uint32 MemoryAddress,
                                                    uint32 MemorySize,
                                                    PduInfoType *pduTxData)
{
    Dcm_ReturnReadMemoryType ReadRet;
    Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;
    *OpStatus = DCM_INITIAL;
    /* @req DCM495 */
    ReadRet = Dcm_ReadMemory(*OpStatus,memoryIdentifier,
                                    MemoryAddress,
                                    MemorySize,
                                    &pduTxData->SduDataPtr[1]);
    if(DCM_READ_FAILED == ReadRet) {
        responseCode = DCM_E_GENERALPROGRAMMINGFAILURE;  /*@req Dcm644*/
    }
    if (DCM_READ_PENDING == ReadRet) {
        *OpStatus = DCM_READ_PENDING;
    }	
    return responseCode;
}

static Dcm_NegativeResponseCodeType checkAddressRange(DspMemoryServiceType serviceType, uint8 memoryIdentifier, uint32 memoryAddress, uint32 length) {
    const Dcm_DspMemoryIdInfo *dspMemoryInfo = Dcm_ConfigPtr->Dsp->DspMemory->DspMemoryIdInfo;
    const Dcm_DspMemoryRangeInfo *memoryRangeInfo = NULL_PTR;
    Dcm_NegativeResponseCodeType diagResponseCode = DCM_E_REQUESTOUTOFRANGE;

    for( ; (dspMemoryInfo->Arc_EOL == FALSE) && (memoryRangeInfo == NULL_PTR); dspMemoryInfo++ ) {
        if( ((TRUE == Dcm_ConfigPtr->Dsp->DspMemory->DcmDspUseMemoryId) && (dspMemoryInfo->MemoryIdValue == memoryIdentifier))
            || (FALSE == Dcm_ConfigPtr->Dsp->DspMemory->DcmDspUseMemoryId) ) {

            if( DCM_READ_MEMORY == serviceType ) {
                /* @req DCM493 */
                memoryRangeInfo = findRange( dspMemoryInfo->pReadMemoryInfo, memoryAddress, length );
            }
            else {
                /* @req DCM489 */
                memoryRangeInfo = findRange( dspMemoryInfo->pWriteMemoryInfo, memoryAddress, length );
            }

            if( NULL_PTR != memoryRangeInfo ) {
                /* @req DCM490 */
                /* @req DCM494 */
                if( DspCheckSecurityLevel(memoryRangeInfo->pSecurityLevel)) {
                    /* Range is ok */
                    diagResponseCode = DCM_E_POSITIVERESPONSE;
                }
                else {
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
    const Dcm_DspMemoryRangeInfo *memoryRange = NULL_PTR;

    for( ; (memoryRangePtr->Arc_EOL == FALSE) && (memoryRange == NULL_PTR); memoryRangePtr++ ) {
        /*@req DCM493*/
        if((memoryAddress >= memoryRangePtr->MemoryAddressLow)
            && (memoryAddress <= memoryRangePtr->MemoryAddressHigh)
            && (memoryAddress + length - 1 <= memoryRangePtr->MemoryAddressHigh)) {
            memoryRange = memoryRangePtr;
        }
    }

    return memoryRange;
}

void DspUdsWriteMemoryByAddress(const PduInfoType *pduRxData, PduInfoType *pduTxData)
{
    /* @req DCM488 */
    Dcm_NegativeResponseCodeType diagResponseCode;
    uint8 sizeFormat = 0u;
    uint8 addressFormat = 0u;
    uint32 memoryAddress = 0u;
    uint32 length = 0u;
    uint8 i = 0u;
    uint8 memoryIdentifier = 0u; /* Should be 0 if DcmDspUseMemoryId == FALSE */
    Dcm_OpStatusType OpStatus = DCM_INITIAL;
    uint8 addressOffset;

    if( pduRxData->SduLength > ALFID_INDEX ) {
        sizeFormat = ((uint8)(pduRxData->SduDataPtr[ALFID_INDEX] & DCM_FORMAT_HIGH_MASK)) >> 4;	/*@req UDS_REQ_0x23_1 & UDS_REQ_0x23_5*/;
        addressFormat = ((uint8)(pduRxData->SduDataPtr[ALFID_INDEX])) & DCM_FORMAT_LOW_MASK;   /*@req UDS_REQ_0x23_1 & UDS_REQ_0x23_5*/;
        if((addressFormat != 0) && (sizeFormat != 0)) {
            if(addressFormat + sizeFormat + SID_LEN + ALFID_LEN <= pduRxData->SduLength) {
                if( TRUE == Dcm_ConfigPtr->Dsp->DspMemory->DcmDspUseMemoryId ) {
                    memoryIdentifier = pduRxData->SduDataPtr[ADDR_START_INDEX];
                    addressOffset = 1u;
                }
                else {
                    addressOffset = 0u;
                }

                /* Parse address */
                for(i = addressOffset; i < addressFormat; i++) {
                    memoryAddress <<= 8;
                    memoryAddress += (uint32)(pduRxData->SduDataPtr[ADDR_START_INDEX + i]);
                }

                /* Parse size */
                for(i = 0; i < sizeFormat; i++) {
                    length <<= 8;
                    length += (uint32)(pduRxData->SduDataPtr[ADDR_START_INDEX + addressFormat + i]);
                }

                if( addressFormat + sizeFormat + SID_LEN + ALFID_LEN + length == pduRxData->SduLength ) {

                    diagResponseCode = checkAddressRange(DCM_WRITE_MEMORY, memoryIdentifier, memoryAddress, length);
                    if( DCM_E_POSITIVERESPONSE == diagResponseCode ) {
                        if( (SID_LEN + ALFID_LEN + addressFormat + sizeFormat) <= pduTxData->SduLength ) {
                            diagResponseCode = writeMemoryData(&OpStatus, memoryIdentifier, memoryAddress, length,
                                                        &pduRxData->SduDataPtr[SID_LEN + ALFID_LEN + addressFormat + sizeFormat]);
                        }
                        else {
                            /* Not enough room in the tx buffer */
                            diagResponseCode = DCM_E_RESPONSETOOLONG;
                        }
                    }

                }
                else {
                    diagResponseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
                }
            }
            else {
                diagResponseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
            }
        }
        else {
            diagResponseCode = DCM_E_REQUESTOUTOFRANGE;  /*UDS_REQ_0x23_10*/
        }
    }
    else {
        diagResponseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
    }

    if(DCM_E_POSITIVERESPONSE == diagResponseCode) {
        pduTxData->SduLength = SID_LEN + ALFID_LEN + addressFormat + sizeFormat;
        pduTxData->SduDataPtr[ALFID_INDEX] = pduRxData->SduDataPtr[ALFID_INDEX];
        for(i = 0; i < addressFormat + sizeFormat; i++) {
            pduTxData->SduDataPtr[ADDR_START_INDEX + i] = pduRxData->SduDataPtr[ADDR_START_INDEX + i];
            if(OpStatus != DCM_WRITE_PENDING) {
                DsdDspProcessingDone(diagResponseCode);
            }
            else {
                dspMemoryState=DCM_MEMORY_WRITE;
            }
        }
    }
    else {
        DsdDspProcessingDone(diagResponseCode);
    }
}

void DspUdsReadMemoryByAddress(const PduInfoType *pduRxData, PduInfoType *pduTxData)
{
    /* @req DCM492 */
    Dcm_NegativeResponseCodeType diagResponseCode;
    uint8 sizeFormat;
    uint8 addressFormat;
    uint32 memoryAddress = 0;
    uint32 length = 0;
    uint8 i;
    uint8 memoryIdentifier = 0; /* Should be 0 if DcmDspUseMemoryId == FALSE */
    Dcm_OpStatusType OpStatus = DCM_INITIAL;
    uint8 addressOffset;

    if( pduRxData->SduLength > ALFID_INDEX ) {
        sizeFormat = ((uint8)(pduRxData->SduDataPtr[ALFID_INDEX] & DCM_FORMAT_HIGH_MASK)) >> 4;	/*@req UDS_REQ_0x23_1 & UDS_REQ_0x23_5*/;
        addressFormat = ((uint8)(pduRxData->SduDataPtr[ALFID_INDEX])) & DCM_FORMAT_LOW_MASK;   /*@req UDS_REQ_0x23_1 & UDS_REQ_0x23_5*/;
        if((addressFormat != 0) && (sizeFormat != 0)) {
            if(addressFormat + sizeFormat + SID_LEN + ALFID_LEN == pduRxData->SduLength) {
                if( TRUE == Dcm_ConfigPtr->Dsp->DspMemory->DcmDspUseMemoryId ) {
                    memoryIdentifier = pduRxData->SduDataPtr[ADDR_START_INDEX];
                    addressOffset = 1;
                }
                else {
                    addressOffset = 0;
                }

                /* Parse address */
                for(i = addressOffset; i < addressFormat; i++) {
                    memoryAddress <<= 8;
                    memoryAddress += (uint32)(pduRxData->SduDataPtr[ADDR_START_INDEX + i]);
                }

                /* Parse size */
                for(i = 0; i < sizeFormat; i++) {
                    length <<= 8;
                    length += (uint32)(pduRxData->SduDataPtr[ADDR_START_INDEX + addressFormat + i]);
                }

                if((length <= (DCM_PROTOCAL_TP_MAX_LENGTH - SID_LEN)) && (length <= (pduTxData->SduLength - SID_LEN)) ) {
                    diagResponseCode = checkAddressRange(DCM_READ_MEMORY, memoryIdentifier, memoryAddress, length);
                    if( DCM_E_POSITIVERESPONSE == diagResponseCode ) {
                        diagResponseCode = readMemoryData(&OpStatus, memoryIdentifier, memoryAddress, length, pduTxData);
                    }
                }
                else {
                    diagResponseCode = DCM_E_RESPONSETOOLONG;
                }
            }
            else {
                diagResponseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
            }
        }
        else {
            diagResponseCode = DCM_E_REQUESTOUTOFRANGE;  /*UDS_REQ_0x23_10*/
        }
    }
    else {
        diagResponseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
    }

    if(DCM_E_POSITIVERESPONSE == diagResponseCode) {
        pduTxData->SduLength = SID_LEN + length;
        if(OpStatus == DCM_READ_PENDING) {
            dspMemoryState = DCM_MEMORY_READ;
        }
        else {
            DsdDspProcessingDone(DCM_E_POSITIVERESPONSE);
        }
    }
    else {
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
    /* @req DCM491 */
    writeRet = Dcm_WriteMemory(*OpStatus,
                                memoryIdentifier,
                                MemoryAddress,
                                MemorySize,
                                SourceData);
    if(DCM_WRITE_FAILED == writeRet) {
        /* @req DCM643 */
        responseCode = DCM_E_GENERALPROGRAMMINGFAILURE;   /*@req UDS_REQ_0X3D_16 */
    }
    else if(DCM_WRITE_PENDING == writeRet) {
        *OpStatus = DCM_PENDING;
    }
    else {
        responseCode = DCM_E_POSITIVERESPONSE;
    }
    
    return responseCode;
}

#if defined(DCM_USE_SERVICE_DYNAMICALLYDEFINEDATAIDENTIFIER) && defined(DCM_USE_SERVICE_READDATABYPERIODICIDENTIFIER)
/**
 * Checks if a periodic Did is setup for periodic transmission
 * @param PeriodicDid
 * @return TRUE: PDid setup, FALSE: PDid not setup
 */
static boolean isInPDidBuffer(uint8 PeriodicDid)
{
    boolean ret = FALSE;
    for(uint8 i = 0; (i < dspPDidRef.PDidNofUsed) && (ret == FALSE); i++) {
        if(PeriodicDid == dspPDidRef.dspPDid[i].PeriodicDid) {
            ret = TRUE;
        }
    }
    return ret;
}
#endif

#ifdef DCM_USE_SERVICE_READDATABYPERIODICIDENTIFIER
/**
 * Checks if a periodic Did is setup for transmission on connection
 * corresponding to the the rxPduId. If so index in PDid buffer is returned
 * @param PeriodicDid
 * @param rxPduId
 * @param postion
 * @return TRUE: PDid setup, FALSE: PDid not setup
 */
static boolean checkPeriodicIdentifierBuffer(uint8 PeriodicDid, PduIdType rxPduId, uint8 *postion)
{
    boolean ret = FALSE;
    for(uint8 i = 0; (i < dspPDidRef.PDidNofUsed) && (ret == FALSE); i++) {
        if((PeriodicDid == dspPDidRef.dspPDid[i].PeriodicDid) && (rxPduId == dspPDidRef.dspPDid[i].PDidRxPduID)) {
            ret = TRUE;
            *postion = i;
        }
    }

    return ret;
}

/**
 * Removes a Pdid, for the corresponding rxPduId, from
 * transmission
 * @param pDid
 * @param rxPduId
 */
static void DspPdidRemove(uint8 pDid, PduIdType rxPduId)
{
    uint8 pos = 0;
    if( checkPeriodicIdentifierBuffer(pDid, rxPduId, &pos) ) {
        dspPDidRef.PDidNofUsed--;
        dspPDidRef.dspPDid[pos].PeriodicDid = dspPDidRef.dspPDid[dspPDidRef.PDidNofUsed].PeriodicDid;
        dspPDidRef.dspPDid[pos].PDidTxCounter = dspPDidRef.dspPDid[dspPDidRef.PDidNofUsed].PDidTxCounter;
        dspPDidRef.dspPDid[pos].PDidTxPeriod = dspPDidRef.dspPDid[dspPDidRef.PDidNofUsed].PDidTxPeriod;
        dspPDidRef.dspPDid[pos].PDidRxPduID = dspPDidRef.dspPDid[dspPDidRef.PDidNofUsed].PDidRxPduID;
#if (DCM_PERIODIC_DID_SYNCH_SAMPLING == STD_ON)
        dspPDidRef.dspPDid[pos].PdidLength = dspPDidRef.dspPDid[dspPDidRef.PDidNofUsed].PdidLength;
        dspPDidRef.dspPDid[pos].PdidSampleStatus = dspPDidRef.dspPDid[dspPDidRef.PDidNofUsed].PdidSampleStatus;
        if( PDID_SAMPLED_OK == dspPDidRef.dspPDid[dspPDidRef.PDidNofUsed].PdidSampleStatus ) {
            memcpy(dspPDidRef.dspPDid[pos].PdidData, dspPDidRef.dspPDid[dspPDidRef.PDidNofUsed].PdidData, dspPDidRef.dspPDid[dspPDidRef.PDidNofUsed].PdidLength);
        }
        dspPDidRef.dspPDid[dspPDidRef.PDidNofUsed].PdidLength = 0;
        dspPDidRef.dspPDid[dspPDidRef.PDidNofUsed].PdidSampleStatus = PDID_NOT_SAMPLED;
#endif
        dspPDidRef.dspPDid[dspPDidRef.PDidNofUsed].PeriodicDid = 0;
        dspPDidRef.dspPDid[dspPDidRef.PDidNofUsed].PDidTxCounter = 0;
        dspPDidRef.dspPDid[dspPDidRef.PDidNofUsed].PDidTxPeriod = 0;
        dspPDidRef.dspPDid[dspPDidRef.PDidNofUsed].PDidRxPduID = 0;
        if(dspPDidRef.nextStartIndex >= dspPDidRef.PDidNofUsed) {
            dspPDidRef.nextStartIndex = 0;
        }
    }
}

/**
 * Adds a PDid for periodic transmission or updates period if already setup
 * @param pDid
 * @param rxPduId
 * @param periodicTransmitCounter
 * @return PDID_BUFFER_FULL: Could not add PDid, PDID_ADDED: PDid was added, PDID_UPDATED: Period was updated to already setup PDid
 */
static PdidEntryStatusType DspPdidAddOrUpdate(uint8 pDid, PduIdType rxPduId, uint32 periodicTransmitCounter)
{
    uint8 indx = 0;
    PdidEntryStatusType ret = PDID_BUFFER_FULL;
    if( checkPeriodicIdentifierBuffer(pDid, rxPduId, &indx) ) {
        if( 0 != periodicTransmitCounter ) {
            dspPDidRef.dspPDid[indx].PDidTxPeriod = periodicTransmitCounter;
            dspPDidRef.dspPDid[indx].PDidTxCounter = 0;
#if (DCM_PERIODIC_DID_SYNCH_SAMPLING == STD_ON)
            dspPDidRef.dspPDid[indx].PdidSampleStatus = PDID_NOT_SAMPLED;
#endif
        }
        ret = PDID_UPDATED;
    } else if(dspPDidRef.PDidNofUsed < DCM_LIMITNUMBER_PERIODDATA) {
        dspPDidRef.dspPDid[dspPDidRef.PDidNofUsed].PeriodicDid = pDid;
        dspPDidRef.dspPDid[dspPDidRef.PDidNofUsed].PDidTxCounter = 0;
        dspPDidRef.dspPDid[dspPDidRef.PDidNofUsed].PDidTxPeriod = periodicTransmitCounter;
        dspPDidRef.dspPDid[dspPDidRef.PDidNofUsed].PDidRxPduID = rxPduId;
#if (DCM_PERIODIC_DID_SYNCH_SAMPLING == STD_ON)
        dspPDidRef.dspPDid[dspPDidRef.PDidNofUsed].PdidSampleStatus = PDID_NOT_SAMPLED;
#endif
        dspPDidRef.PDidNofUsed++;
        ret = PDID_ADDED;
    }
    return ret;
}

#if (DCM_PERIODIC_DID_SYNCH_SAMPLING == STD_ON)

/**
 * Synchronizes PDids for a specific period and rxPduId
 * @param period
 * @param rxPduId
 */
static void DspSynchronizePdids(uint32 period, PduIdType rxPduId)
{
    for(uint8 i = 0; i < dspPDidRef.PDidNofUsed; i++) {
        if( (dspPDidRef.dspPDid[i].PDidTxPeriod == period) && (dspPDidRef.dspPDid[i].PDidRxPduID == rxPduId) ) {
            dspPDidRef.dspPDid[i].PDidTxCounter = 0;
            dspPDidRef.dspPDid[i].PdidSampleStatus = PDID_NOT_SAMPLED;
        }
    }
}
/**
 * Reads PDid for a specific rxPduId from buffer
 * @param Pdid
 * @param rxPduId
 * @param data
 * @param dataLength
 * @return DCM_E_POSITIVERESPONSE: PDid read, DCM_E_REQUESTOUTOFRANGE: PDid not found in buffer or has not been sampled
 */
static Dcm_NegativeResponseCodeType getPDidDataFromBuffer(uint8 Pdid, PduIdType rxPduId, uint8 *data, uint16 *dataLength)
{
    /* NOTE: Once a PDid is read using this function
     * the PDid will be marked as not sampled. */
    Dcm_NegativeResponseCodeType responseCode = DCM_E_REQUESTOUTOFRANGE;
    boolean found = FALSE;
    for(uint8 i = 0; (i < dspPDidRef.PDidNofUsed) && !found; i++) {
        if( (dspPDidRef.dspPDid[i].PeriodicDid == Pdid) && (dspPDidRef.dspPDid[i].PDidRxPduID == rxPduId)) {
            found = TRUE;
            if( PDID_SAMPLED_OK == dspPDidRef.dspPDid[i].PdidSampleStatus ) {
                memcpy(data, dspPDidRef.dspPDid[i].PdidData, dspPDidRef.dspPDid[i].PdidLength);
                *dataLength = dspPDidRef.dspPDid[i].PdidLength;
                dspPDidRef.dspPDid[i].PdidSampleStatus = PDID_NOT_SAMPLED;
                responseCode = DCM_E_POSITIVERESPONSE;
            }
        }
    }
    return responseCode;
}

/**
 * Samples PDid for a specific period and rxPduId
 * @param period
 * @param rxPduId
 */
static void DspSamplePDids(uint32 period, PduIdType rxPduId) {
    for(uint8 i = 0; i < dspPDidRef.PDidNofUsed; i++) {
        if( (dspPDidRef.dspPDid[i].PDidTxPeriod == period) && (dspPDidRef.dspPDid[i].PDidRxPduID == rxPduId) ) {
            uint16 pdidLength = 0;
            if( DCM_E_POSITIVERESPONSE == getPDidData(TO_PERIODIC_DID(dspPDidRef.dspPDid[i].PeriodicDid), dspPDidRef.dspPDid[i].PdidData, MAX_PDID_DATA_SIZE, &pdidLength) ) {
                /* Assuming max length will fit in uint8 to save ram.. */
                dspPDidRef.dspPDid[i].PdidLength = (uint8)pdidLength;
                dspPDidRef.dspPDid[i].PdidSampleStatus = PDID_SAMPLED_OK;
            } else {
                dspPDidRef.dspPDid[i].PdidSampleStatus = PDID_SAMPLED_FAILED;
            }
        }
    }
}
#endif

/**
 * Reads data for a non-dynamic PDid
 * @param PDidPtr
 * @param Data
 * @param Length
 * @return DCM_E_POSITIVERESPONSE: PDid read OK, DCM_E_REQUESTOUTOFRANGE: PDid read failed
 */
static Dcm_NegativeResponseCodeType readPeriodDidData(const Dcm_DspDidType *PDidPtr, uint8 *Data, uint16 bufSize, uint16 *Length)
{
    Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;
    PduInfoType myPdu;
    uint16 txPos = 0u;
    ReadDidPendingStateType pendingState = DCM_READ_DID_IDLE;
    uint16 pendingDataLen = 0u;
    uint16 pendingSignalIndex = 0u;
    uint16 didDataStartPos = 0u;
    *Length = 0u;
    if (PDidPtr->DspDidInfoRef->DspDidAccess.DspDidRead != NULL_PTR ) {
        myPdu.SduDataPtr = Data;
        myPdu.SduLength = bufSize;
        responseCode = getDidData(PDidPtr, &myPdu, &txPos, &pendingState, &pendingDataLen, &pendingSignalIndex, &didDataStartPos, FALSE);
        if( DCM_E_POSITIVERESPONSE == responseCode ) {
            *Length = txPos;
        }
        else if(DCM_E_RESPONSEPENDING == responseCode) {
            DspCancelPendingDid(PDidPtr->DspDidIdentifier, pendingSignalIndex, pendingState, &myPdu);
            responseCode = DCM_E_REQUESTOUTOFRANGE;
        }
        else {
            responseCode = DCM_E_REQUESTOUTOFRANGE;
        }
    } else {
        responseCode = DCM_E_REQUESTOUTOFRANGE;
    }
    return responseCode;
}

/**
 * Removes PDid(s) from transmission
 * @param pduRxData
 * @param pduTxData
 * @param rxPduId
 */
static void ClearPeriodicIdentifier(const PduInfoType *pduRxData,PduInfoType *pduTxData, PduIdType rxPduId )
{
    uint16 PdidNumber;
    uint8 i;
    if( pduRxData->SduDataPtr[1] == DCM_PERIODICTRANSMIT_STOPSENDING_MODE ) {
        PdidNumber = pduRxData->SduLength - 2;
        for(i = 0u;i < PdidNumber; i++) {
            DspPdidRemove(pduRxData->SduDataPtr[2 + i], rxPduId);
        }
        pduTxData->SduLength = 1;
    }
}

/**
 * Gets the maximum length of a PDid for a transmission type on the active protocol
 * @param txType
 * @return
 */
static uint16 DspGetProtocolMaxPDidLength(Dcm_ProtocolTransTypeType txType)
{
    uint16 maxLength = 0u;
    Dcm_ProtocolType activeProtocol;
    if(E_OK == DslGetActiveProtocol(&activeProtocol)) {
        switch(activeProtocol) {
            case DCM_OBD_ON_CAN:
            case DCM_UDS_ON_CAN:
            case DCM_ROE_ON_CAN:
            case DCM_PERIODICTRANS_ON_CAN:
                maxLength = (DCM_PROTOCOL_TRANS_TYPE_2 == txType) ? MAX_TYPE2_PERIODIC_DID_LEN_CAN : MAX_TYPE1_PERIODIC_DID_LEN_CAN;
                break;
            case DCM_OBD_ON_FLEXRAY:
            case DCM_UDS_ON_FLEXRAY:
            case DCM_ROE_ON_FLEXRAY:
            case DCM_PERIODICTRANS_ON_FLEXRAY:
                maxLength = (DCM_PROTOCOL_TRANS_TYPE_2 == txType) ? MAX_TYPE2_PERIODIC_DID_LEN_FLEXRAY : MAX_TYPE1_PERIODIC_DID_LEN_FLEXRAY;
                break;
            case DCM_OBD_ON_IP:
            case DCM_UDS_ON_IP:
            case DCM_ROE_ON_IP:
            case DCM_PERIODICTRANS_ON_IP:
                maxLength = (DCM_PROTOCOL_TRANS_TYPE_2 == txType) ? MAX_TYPE2_PERIODIC_DID_LEN_IP : MAX_TYPE1_PERIODIC_DID_LEN_IP;
                break;
            default:
                break;
        }
    }
    return maxLength;
}

/**
 * Checks if a PDid is supported. If supported, the length of the PDid is returned
 * @param pDid
 * @param didLength
 * @param responseCode
 * @return TRUE: PDid supported, FALSE: PDid not supported
 */
static boolean checkPDidSupported(uint16 pDid, uint16 *didLength, Dcm_NegativeResponseCodeType *responseCode)
{
    const Dcm_DspDidType *PDidPtr = NULL_PTR;
    Dcm_DspDDDType *DDidPtr = NULL_PTR;
    boolean didSupported = FALSE;
    boolean isDynamicDid = FALSE;

    if( lookupNonDynamicDid(pDid, &PDidPtr) ) {
        didSupported = TRUE;
    } else if(lookupDynamicDid(pDid, &PDidPtr)) {
        didSupported = TRUE;
        isDynamicDid = TRUE;
    }

    *responseCode = DCM_E_REQUESTOUTOFRANGE;
    if(didSupported && (NULL_PTR != PDidPtr) && (NULL_PTR != PDidPtr->DspDidInfoRef->DspDidAccess.DspDidRead)) {
        /* @req DCM721 */
        if(DspCheckSessionLevel(PDidPtr->DspDidInfoRef->DspDidAccess.DspDidRead->DspDidReadSessionRef)) {
            /* @req DCM722 */
            *responseCode = DCM_E_SECURITYACCESSDENIED;
            if(DspCheckSecurityLevel(PDidPtr->DspDidInfoRef->DspDidAccess.DspDidRead->DspDidReadSecurityLevelRef)) {
                *responseCode = DCM_E_POSITIVERESPONSE;
                if( isDynamicDid ) {
                    *responseCode = DCM_E_REQUESTOUTOFRANGE;
                    if( LookupDDD(pDid, (const Dcm_DspDDDType **)&DDidPtr) ) {
                        /* It is a dynamically defined did */
                        /* @req DCM721 */
                        /* @req DCM722 */
                        *responseCode = checkDDDConditions(DDidPtr, didLength);
                    }
                }
            }
        }
    }
    return didSupported;
}

/**
 * Reads data of a PDid
 * @param did
 * @param data
 * @param dataLength
 * @return DCM_E_POSITIVERESPONSE: Read OK, other: Read failed
 */
static Dcm_NegativeResponseCodeType getPDidData(uint16 did, uint8 *data, uint16 bufSize, uint16 *dataLength)
{
    Dcm_NegativeResponseCodeType responseCode;
    const Dcm_DspDidType *PDidPtr = NULL_PTR;
    Dcm_DspDDDType *DDidPtr = NULL_PTR;
    if( lookupNonDynamicDid(did, &PDidPtr) ) {
        responseCode = readPeriodDidData(PDidPtr, data, bufSize, dataLength);
    } else if( LookupDDD(did, (const Dcm_DspDDDType **)&DDidPtr) ) {
        responseCode = readDDDData(DDidPtr, data, bufSize, dataLength);
    } else {
        responseCode = DCM_E_REQUESTOUTOFRANGE;
    }
    return responseCode;
}

/**
 * Implements UDS service 0x2A (readDataByPeriodicIdentifier)
 * @param pduRxData
 * @param pduTxData
 * @param rxPduId
 * @param txType
 * @param internalRequest
 */

void DspReadDataByPeriodicIdentifier(const PduInfoType *pduRxData, PduInfoType *pduTxData, PduIdType rxPduId, Dcm_ProtocolTransTypeType txType, boolean internalRequest)
{
    /* @req DCM254 */
    uint8 PDidLowByte;
    uint16 PdidNumber;
    uint32 periodicTransmitCounter = 0u;
    uint16 DataLength = 0u;
    Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;
    boolean secAccOK = FALSE;
    boolean requestOK = FALSE;
    boolean requestHasSupportedDid = FALSE;
    uint8 nofPdidsAdded = 0u;
    uint8 pdidsAdded[DCM_LIMITNUMBER_PERIODDATA];
    uint16 maxDidLen = 0u;
    boolean supressNRC = FALSE;
    memset(pdidsAdded, 0, DCM_LIMITNUMBER_PERIODDATA);
    if(pduRxData->SduLength > 2) {
        switch(pduRxData->SduDataPtr[1]) {
            case DCM_PERIODICTRANSMIT_DEFAULT_MODE:
                periodicTransmitCounter = 0;
                responseCode = internalRequest ? responseCode:DCM_E_REQUESTOUTOFRANGE;
                break;
            case DCM_PERIODICTRANSMIT_SLOW_MODE:
                periodicTransmitCounter = DCM_PERIODICTRANSMIT_SLOW;
                break;
            case DCM_PERIODICTRANSMIT_MEDIUM_MODE:
                periodicTransmitCounter = DCM_PERIODICTRANSMIT_MEDIUM;
            break;
            case DCM_PERIODICTRANSMIT_FAST_MODE:
                periodicTransmitCounter = DCM_PERIODICTRANSMIT_FAST;
                break;
            case DCM_PERIODICTRANSMIT_STOPSENDING_MODE:
                ClearPeriodicIdentifier(pduRxData,pduTxData, rxPduId);
                break;
            default:
                responseCode = DCM_E_REQUESTOUTOFRANGE;
                break;
        }
        if((pduRxData->SduDataPtr[1] != DCM_PERIODICTRANSMIT_STOPSENDING_MODE) && (DCM_E_POSITIVERESPONSE == responseCode)) {
            maxDidLen = DspGetProtocolMaxPDidLength(txType);
            PdidNumber = pduRxData->SduLength - 2;
            /* Check the dids in the request. Must be "small" enough to fit in the response frame.
             * If there are more dids in the request than we can handle, we only give negative response code
             * if the number of supported dids in the request are greater than the number of entries left
             * in our buffer. */
            for( uint8 indx = 0u; (indx < PdidNumber) && (DCM_E_POSITIVERESPONSE == responseCode); indx++ ) {
                PDidLowByte = pduRxData->SduDataPtr[2 + indx];
                uint16 didLength = 0u;
                Dcm_NegativeResponseCodeType resp = DCM_E_POSITIVERESPONSE;
                if(checkPDidSupported(TO_PERIODIC_DID(PDidLowByte), &didLength, &resp)) {
                    requestHasSupportedDid = TRUE;
                    secAccOK = secAccOK || (DCM_E_SECURITYACCESSDENIED != resp);
                    if((DCM_E_POSITIVERESPONSE == resp) && (didLength <= maxDidLen)) {
                        requestOK = TRUE;
                        PdidEntryStatusType pdidStatus = DspPdidAddOrUpdate(PDidLowByte, rxPduId, periodicTransmitCounter);
                        if( PDID_ADDED == pdidStatus) {
                            pdidsAdded[nofPdidsAdded++] = PDidLowByte;
                        } else if( PDID_BUFFER_FULL == pdidStatus ){
                            /* Would exceed the maximum number of periodic dids.
                             * Clear the ones added now. */
                            for( uint8 idx = 0u; idx < nofPdidsAdded; idx++ ) {
                                DspPdidRemove(pdidsAdded[idx], rxPduId);
                            }
                            responseCode = DCM_E_REQUESTOUTOFRANGE;
                            requestOK = FALSE;
                        } else {
                            /* PDid was updated */
                        }
                    }
                }
            }
            if( requestOK ) {
                /* Request contained at least one supported DID
                 * accessible in the current session and security level */
                if( internalRequest) {
                    /* Internal request, we should transmit */
                    uint8 dataStartIndex = 1; /* Type 1*/
                    if( (1 != PdidNumber) || (0 != periodicTransmitCounter) ) {
                        /* Internal request with more than one pdid, not expected */
                        DCM_DET_REPORTERROR(DCM_GLOBAL_ID, DCM_E_UNEXPECTED_EXECUTION);
                    }
                    if (DCM_PROTOCOL_TRANS_TYPE_2 == txType) {
                        dataStartIndex = 0;
                        memset(pduTxData->SduDataPtr, 0, 8); /* The buffer is always 8 bytes */
                    }
                    supressNRC = TRUE;
#if (DCM_PERIODIC_DID_SYNCH_SAMPLING == STD_ON)
                    responseCode =  getPDidDataFromBuffer(PDidLowByte, rxPduId, &pduTxData->SduDataPtr[dataStartIndex + 1], &DataLength);
#else
                    responseCode = getPDidData(TO_PERIODIC_DID(PDidLowByte), &pduTxData->SduDataPtr[dataStartIndex + 1], pduTxData->SduLength - dataStartIndex, &DataLength);
#endif
                    if(responseCode != DCM_E_POSITIVERESPONSE) {
                        /* NOTE: If a read returns error, should we really remove the did? */
                        DspPdidRemove(PDidLowByte, rxPduId);
                    } else {
                        pduTxData->SduDataPtr[dataStartIndex] = PDidLowByte;
                        if (DCM_PROTOCOL_TRANS_TYPE_2 == txType) {
                            /* Make sure that unused bytes are 0 */
                            for(uint16 byteIndex = (DataLength + dataStartIndex + 1); byteIndex < (maxDidLen + dataStartIndex + 1); byteIndex++) {
                                pduTxData->SduDataPtr[byteIndex] = 0;
                            }
                            pduTxData->SduLength = maxDidLen + dataStartIndex + 1;
                        } else {
                            pduTxData->SduLength = DataLength + dataStartIndex + 1;
                        }
                    }
                } else {
#if (DCM_PERIODIC_DID_SYNCH_SAMPLING == STD_ON)
                    /*  Something was updated or added. Reset timers for Pdids with this
                     * period to simplify synchronized sampling. */
                    DspSynchronizePdids(periodicTransmitCounter, rxPduId);
#endif
                    pduTxData->SduLength = 1;
                }
            } else {
                if(requestHasSupportedDid && !secAccOK) {
                    /* Request contained supported did(s) but none had access in the current security level */
                    /* @req DCM721 */
                    responseCode = DCM_E_SECURITYACCESSDENIED;
                } else {
                    /* No did available in current session, buffer overflow or no did in request will fit in single frame */
                    /* @req DCM722 */
                    responseCode = DCM_E_REQUESTOUTOFRANGE;
                }
            }
        }
    } else if( (pduRxData->SduLength == 2) && (pduRxData->SduDataPtr[1] == DCM_PERIODICTRANSMIT_STOPSENDING_MODE) ) {
        memset(&dspPDidRef,0,sizeof(dspPDidRef));
        pduTxData->SduLength = 1;
    } else  {
        responseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
    }
    DsdDspProcessingDone_ReadDataByPeriodicIdentifier(responseCode, supressNRC);
}
#endif

#ifdef DCM_USE_SERVICE_DYNAMICALLYDEFINEDATAIDENTIFIER
static Dcm_NegativeResponseCodeType dynamicallyDefineDataIdentifierbyDid(uint16 DDIdentifier,const PduInfoType *pduRxData,PduInfoType *pduTxData)
{
    uint8 i;
    uint16 SourceDidNr;
    const Dcm_DspDidType *SourceDid = NULL_PTR;
    Dcm_DspDDDType *DDid = NULL_PTR;
    uint16 SourceLength = 0u;
    uint16 DidLength = 0u;
    uint16 Length = 0u;
    uint8 Num = 0u;
    Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;
    const Dcm_DspSignalType *signalPtr;

    if(FALSE == LookupDDD(DDIdentifier, (const Dcm_DspDDDType **)&DDid)) {
        while((Num < DCM_MAX_DDD_NUMBER) && (dspDDD[Num].DynamicallyDid != 0 )) {
            Num++;
        }
        if(Num >= DCM_MAX_DDD_NUMBER) {
            responseCode = DCM_E_REQUESTOUTOFRANGE;
        } else {
            DDid = &dspDDD[Num];
        }
    } else {
        while((SourceLength < DCM_MAX_DDDSOURCE_NUMBER) && (DDid->DDDSource[SourceLength].formatOrPosition != 0 )) {
            SourceLength++;
        }
    }
    if(responseCode == DCM_E_POSITIVERESPONSE) {
        Length = (pduRxData->SduLength - SID_AND_DDDID_LEN) /SDI_AND_MS_LEN;
        if(((Length*SDI_AND_MS_LEN) == (pduRxData->SduLength - SID_AND_DDDID_LEN)) && (Length != 0)) {
            if((Length + SourceLength) <= DCM_MAX_DDDSOURCE_NUMBER) {
                for(i = 0;(i < Length) && (responseCode == DCM_E_POSITIVERESPONSE);i++) {
                    SourceDidNr = (((uint16)pduRxData->SduDataPtr[SID_AND_DDDID_LEN + i*SDI_AND_MS_LEN] << 8) & DCM_DID_HIGH_MASK) + (((uint16)pduRxData->SduDataPtr[(5 + i*SDI_AND_MS_LEN)]) & DCM_DID_LOW_MASK);
                    if( lookupNonDynamicDid(SourceDidNr, &SourceDid) && (NULL_PTR != SourceDid->DspDidInfoRef->DspDidAccess.DspDidRead) ) {/*UDS_REQ_0x2C_4*/
                        /* @req DCM725 */
                        if(DspCheckSessionLevel(SourceDid->DspDidInfoRef->DspDidAccess.DspDidRead->DspDidReadSessionRef)) {
                            /* @req DCM726 */
                            if(DspCheckSecurityLevel(SourceDid->DspDidInfoRef->DspDidAccess.DspDidRead->DspDidReadSecurityLevelRef)) {
                                DidLength = 0;
                                uint16 tempSize = 0;
                                for( uint16 sigIndex = 0; (sigIndex < SourceDid->DspNofSignals) && (responseCode == DCM_E_POSITIVERESPONSE); sigIndex++ ) {
                                    tempSize = 0;
                                    signalPtr = &SourceDid->DspSignalRef[sigIndex];
                                    if( signalPtr->DspSignalDataRef->DspDataInfoRef->DspDidFixedLength || (DATA_PORT_SR == signalPtr->DspSignalDataRef->DspDataUsePort)) {
                                        tempSize = GetNofAffectedBytes(signalPtr->DspSignalDataRef->DspDataEndianess, signalPtr->DspSignalBitPosition, signalPtr->DspSignalDataRef->DspDataBitSize);
                                    } else if( NULL_PTR != signalPtr->DspSignalDataRef->DspDataReadDataLengthFnc ) {
                                        (void)signalPtr->DspSignalDataRef->DspDataReadDataLengthFnc(&tempSize);
                                    }
                                    tempSize += (signalPtr->DspSignalBitPosition / 8);
                                    if( tempSize > DidLength ) {
                                        DidLength = tempSize;
                                    }
                                }
                                if(DidLength != 0) {
                                    if((pduRxData->SduDataPtr[SID_AND_SDI_LEN + i*SDI_AND_MS_LEN] != 0) &&
                                        (pduRxData->SduDataPtr[SID_AND_PISDR_LEN + i*SDI_AND_MS_LEN] != 0) &&
                                        (((uint16)pduRxData->SduDataPtr[SID_AND_SDI_LEN + i*SDI_AND_MS_LEN] + (uint16)pduRxData->SduDataPtr[SID_AND_PISDR_LEN + i*SID_AND_DDDID_LEN] - 1) <= DidLength))
                                    {
                                        DDid->DDDSource[i + SourceLength].formatOrPosition = pduRxData->SduDataPtr[SID_AND_SDI_LEN + i*SDI_AND_MS_LEN];
                                        DDid->DDDSource[i + SourceLength].Size = pduRxData->SduDataPtr[SID_AND_PISDR_LEN + i*SDI_AND_MS_LEN];
                                        DDid->DDDSource[i + SourceLength].SourceAddressOrDid = SourceDid->DspDidIdentifier;
                                        DDid->DDDSource[i + SourceLength].DDDTpyeID = DCM_DDD_SOURCE_DID;
                                    } else {
                                        /*UDS_REQ_0x2C_6*/
                                        responseCode = DCM_E_REQUESTOUTOFRANGE;
                                    }
                                } else {
                                    /*UDS_REQ_0x2C_14*/
                                    responseCode = DCM_E_REQUESTOUTOFRANGE;
                                }
                            } else {
                                responseCode = DCM_E_SECURITYACCESSDENIED;
                            }
                        } else {
                            /*UDS_REQ_0x2C_19,DCM726*/
                            responseCode = DCM_E_REQUESTOUTOFRANGE;
                        }
                    } else {
                        /*DCM725*/
                        responseCode = DCM_E_REQUESTOUTOFRANGE;
                    }
                }
            } else {
                /*UDS_REQ_0x2C_13*/
                responseCode = DCM_E_REQUESTOUTOFRANGE;
            }
        } else {
            /*UDS_REQ_0x2C_11*/
            responseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
        }
        if(responseCode == DCM_E_POSITIVERESPONSE) {
            DDid->DynamicallyDid = DDIdentifier;
            pduTxData->SduDataPtr[1] = DCM_DDD_SUBFUNCTION_DEFINEBYDID;
        }
    }
    if(responseCode == DCM_E_POSITIVERESPONSE) {
        pduTxData->SduDataPtr[1] = DCM_DDD_SUBFUNCTION_DEFINEBYDID;
    }
    
    return responseCode;
}

static Dcm_NegativeResponseCodeType dynamicallyDefineDataIdentifierbyAddress(uint16 DDIdentifier,const PduInfoType *pduRxData,PduInfoType *pduTxData)
{
    uint16 numNewDefinitions;
    uint16 numEarlierDefinitions = 0u;
    Dcm_DspDDDType *DDid = NULL_PTR;
    uint8 Num = 0u;
    uint8 definitionIndex;
    Dcm_NegativeResponseCodeType diagResponseCode = DCM_E_POSITIVERESPONSE;
    uint8 sizeFormat;
    uint8 addressFormat;
    uint32 memoryAddress = 0u;
    uint32 length = 0u;
    uint8 i;
    uint8 memoryIdentifier = 0u; /* Should be 0 if DcmDspUseMemoryId == FALSE */
    uint8 addressOffset;
    
    if(FALSE == LookupDDD(DDIdentifier, (const Dcm_DspDDDType **)&DDid)) {
        while((Num < DCM_MAX_DDD_NUMBER) && (dspDDD[Num].DynamicallyDid != 0 )) {
            Num++;
        }
        if(Num >= DCM_MAX_DDD_NUMBER) {
            diagResponseCode = DCM_E_REQUESTOUTOFRANGE;
        }
        else {
            DDid = &dspDDD[Num];
        }
    }
    else {
        while((numEarlierDefinitions < DCM_MAX_DDDSOURCE_NUMBER) && (DDid->DDDSource[numEarlierDefinitions].formatOrPosition != 0 )) {
            numEarlierDefinitions++;
        }
        if(numEarlierDefinitions >= DCM_MAX_DDDSOURCE_NUMBER) {
            diagResponseCode = DCM_E_REQUESTOUTOFRANGE;
        }
    }

    if( diagResponseCode == DCM_E_POSITIVERESPONSE ) {
        if( pduRxData->SduLength > DYNDEF_ALFID_INDEX ) {
            sizeFormat = ((uint8)(pduRxData->SduDataPtr[DYNDEF_ALFID_INDEX] & DCM_FORMAT_HIGH_MASK)) >> 4;	/*@req UDS_REQ_0x23_1 & UDS_REQ_0x23_5*/;
            addressFormat = ((uint8)(pduRxData->SduDataPtr[DYNDEF_ALFID_INDEX])) & DCM_FORMAT_LOW_MASK;   /*@req UDS_REQ_0x23_1 & UDS_REQ_0x23_5*/;
            if((addressFormat != 0) && (sizeFormat != 0)) {
                numNewDefinitions = (pduRxData->SduLength - (SID_LEN + SF_LEN + DDDDI_LEN + ALFID_LEN) ) / (sizeFormat + addressFormat);
                if( (numNewDefinitions != 0) &&
                    ((SID_LEN + SF_LEN + DDDDI_LEN + ALFID_LEN + numNewDefinitions * (sizeFormat + addressFormat)) == pduRxData->SduLength) ) {
                    if( (numEarlierDefinitions+numNewDefinitions) <= DCM_MAX_DDDSOURCE_NUMBER ) {
                        for( definitionIndex = 0; (definitionIndex < numNewDefinitions) && (diagResponseCode == DCM_E_POSITIVERESPONSE); definitionIndex++ ) {

                            if( TRUE == Dcm_ConfigPtr->Dsp->DspMemory->DcmDspUseMemoryId ) {
                                memoryIdentifier = pduRxData->SduDataPtr[DYNDEF_ADDRESS_START_INDEX + definitionIndex * (sizeFormat + addressFormat)];
                                addressOffset = 1;
                            }
                            else {
                                addressOffset = 0;
                            }

                            /* Parse address */
                            memoryAddress = 0u;
                            for(i = addressOffset; i < addressFormat; i++) {
                                memoryAddress <<= 8;
                                memoryAddress += (uint32)(pduRxData->SduDataPtr[DYNDEF_ADDRESS_START_INDEX + definitionIndex * (sizeFormat + addressFormat) + i]);
                            }

                            /* Parse size */
                            length = 0;
                            for(i = 0; i < sizeFormat; i++) {
                                length <<= 8;
                                length += (uint32)(pduRxData->SduDataPtr[DYNDEF_ADDRESS_START_INDEX + definitionIndex * (sizeFormat + addressFormat) + addressFormat + i]);
                            }

                            diagResponseCode = checkAddressRange(DCM_READ_MEMORY, memoryIdentifier, memoryAddress, length);
                            if( DCM_E_POSITIVERESPONSE == diagResponseCode ) {
                                DDid->DDDSource[definitionIndex + numEarlierDefinitions].formatOrPosition = pduRxData->SduDataPtr[DYNDEF_ALFID_INDEX];
                                DDid->DDDSource[definitionIndex + numEarlierDefinitions].memoryIdentifier = memoryIdentifier;
                                DDid->DDDSource[definitionIndex + numEarlierDefinitions].SourceAddressOrDid = memoryAddress;
                                DDid->DDDSource[definitionIndex + numEarlierDefinitions].Size = length;
                                DDid->DDDSource[definitionIndex + numEarlierDefinitions].DDDTpyeID = DCM_DDD_SOURCE_ADDRESS;
                            }
                        }
                        if(diagResponseCode == DCM_E_POSITIVERESPONSE) {
                            DDid->DynamicallyDid = DDIdentifier;
                        }
                        else
                        {
                            for( definitionIndex = 0; (definitionIndex < numNewDefinitions); definitionIndex++ ) {
                                DDid->DDDSource[definitionIndex + numEarlierDefinitions].formatOrPosition = 0x00;
                                DDid->DDDSource[definitionIndex + numEarlierDefinitions].memoryIdentifier = 0x00;
                                DDid->DDDSource[definitionIndex + numEarlierDefinitions].SourceAddressOrDid = 0x00000000;
                                DDid->DDDSource[definitionIndex + numEarlierDefinitions].Size = 0x0000;
                                DDid->DDDSource[definitionIndex + numEarlierDefinitions].DDDTpyeID = DCM_DDD_SOURCE_DEFAULT;
                            }
                        }
                    }
                    else {
                        diagResponseCode = DCM_E_REQUESTOUTOFRANGE;
                    }
                }
                else {
                    diagResponseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
                }
            }
            else {
                diagResponseCode = DCM_E_REQUESTOUTOFRANGE;  /*UDS_REQ_0x23_10*/
            }
        }
        else {
            diagResponseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
        }
    }


    if(diagResponseCode == DCM_E_POSITIVERESPONSE) {
        pduTxData->SduDataPtr[SF_INDEX] = DCM_DDD_SUBFUNCTION_DEFINEBYADDRESS;
    }
    
    return diagResponseCode;
}


/*
    DESCRIPTION:
         UDS Service 0x2c - Clear dynamically Did
*/
static Dcm_NegativeResponseCodeType ClearDynamicallyDefinedDid(uint16 DDIdentifier,const PduInfoType *pduRxData, PduInfoType * pduTxData)
{
    /*UDS_REQ_0x2C_5*/
    sint8 i;
    uint8 j;
    Dcm_DspDDDType *DDid = NULL_PTR;
    Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;

    if(pduRxData->SduLength == 4) {
        if(TRUE == LookupDDD(DDIdentifier, (const Dcm_DspDDDType **)&DDid)) {
    #if defined(DCM_USE_SERVICE_READDATABYPERIODICIDENTIFIER)
            if((isInPDidBuffer(pduRxData->SduDataPtr[3]) == TRUE) && (pduRxData->SduDataPtr[2] == 0xF2)) {
                /*UDS_REQ_0x2C_9*/
                responseCode = DCM_E_REQUESTOUTOFRANGE;
            } else
    #endif
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
                            }else{
                                /*do nothing */
                            }
                        }
                    }
                }
            }
        } else {
            responseCode = DCM_E_REQUESTOUTOFRANGE;	/* DDDid not found */
        }
    } else {
		responseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
	}
	if(responseCode == DCM_E_POSITIVERESPONSE) {
		pduTxData->SduDataPtr[1] = DCM_DDD_SUBFUNCTION_CLEAR;
		pduTxData->SduLength = 2;
	}
	
	return responseCode;
}

void DspDynamicallyDefineDataIdentifier(const PduInfoType *pduRxData,PduInfoType *pduTxData)
{
    /*UDS_REQ_0x2C_1 */
    /* @req DCM259 */
    uint16 i;
    uint16 DDIdentifier;
    boolean PeriodicUse = FALSE;
    Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;
    const Dcm_DspDidType *dDidPtr = NULL_PTR;

    if(pduRxData->SduLength > 2) {
        /* Check if DDID is in the range 0xF200-0xF3FF */
        DDIdentifier = ((((uint16)pduRxData->SduDataPtr[2]) << 8) & DCM_DID_HIGH_MASK) + (pduRxData->SduDataPtr[3] & DCM_DID_LOW_MASK);
        /* @req DCM723 */
        if( lookupDynamicDid(DDIdentifier, &dDidPtr) && (NULL_PTR != dDidPtr) && (NULL_PTR != dDidPtr->DspDidInfoRef->DspDidAccess.DspDidWrite) && DspCheckSessionLevel(dDidPtr->DspDidInfoRef->DspDidAccess.DspDidWrite->DspDidWriteSessionRef)) {
            /* @req DCM724 */
            if( DspCheckSecurityLevel(dDidPtr->DspDidInfoRef->DspDidAccess.DspDidWrite->DspDidWriteSecurityLevelRef) ) {
                switch(pduRxData->SduDataPtr[1]) {	/*UDS_REQ_0x2C_2*//* @req DCM646 */
                    case DCM_DDD_SUBFUNCTION_DEFINEBYDID:
                        responseCode  = dynamicallyDefineDataIdentifierbyDid(DDIdentifier,pduRxData,pduTxData);
                        break;
                    case DCM_DDD_SUBFUNCTION_DEFINEBYADDRESS:
                        responseCode = dynamicallyDefineDataIdentifierbyAddress(DDIdentifier,pduRxData,pduTxData);
                        break;
                    case DCM_DDD_SUBFUNCTION_CLEAR:/* @req DCM647 */
                        responseCode = ClearDynamicallyDefinedDid(DDIdentifier, pduRxData,pduTxData);
                        break;
                    default:
                        responseCode = DCM_E_SUBFUNCTIONNOTSUPPORTED;
                        /*UDS_REQ_0x2C_10*/
                        break;
                }
            } else {
                responseCode = DCM_E_SECURITYACCESSDENIED;
            }
        } else {
            responseCode = DCM_E_REQUESTOUTOFRANGE;
        }
        if(responseCode == DCM_E_POSITIVERESPONSE) {
            pduTxData->SduDataPtr[2] = pduRxData->SduDataPtr[2];
            pduTxData->SduDataPtr[3] = pduRxData->SduDataPtr[3];
            pduTxData->SduLength = 4;
        }
    } else if((pduRxData->SduLength == 2)&&(pduRxData->SduDataPtr[1] == DCM_DDD_SUBFUNCTION_CLEAR)) {
        /*UDS_REQ_0x2C_7*/
#if defined(DCM_USE_SERVICE_READDATABYPERIODICIDENTIFIER)
        for(i = 0;i < DCM_MAX_DDD_NUMBER;i++) {
            if(isInPDidBuffer((uint8)(dspDDD[i].DynamicallyDid & DCM_DID_LOW_MASK)) == TRUE) {
                PeriodicUse = TRUE;
            }
        }
#endif
        if(PeriodicUse == FALSE) {
            memset(dspDDD,0,sizeof(dspDDD));
            pduTxData->SduDataPtr[1] = DCM_DDD_SUBFUNCTION_CLEAR;
            pduTxData->SduLength = 2;
        } else {
            responseCode = DCM_E_CONDITIONSNOTCORRECT;
        }
    } else {
        /*UDS_REQ_0x2C_11*/
        responseCode =  DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
    }
    DsdDspProcessingDone(responseCode);
}
#endif

#if defined(DCM_USE_SERVICE_INPUTOUTPUTCONTROLBYIDENTIFIER) && defined(DCM_USE_CONTROL_DIDS)
/**
 * Function for return active control to ECU
 * @param checkSessionAndSecLevel
 */
static void DspStopInputOutputControl(boolean checkSessionAndSecLevel)
{
    const Dcm_DspDidControlType *DidControl = NULL_PTR;
    const Dcm_DspDidType *DidPtr = NULL_PTR;
    const Dcm_DspSignalType *signalPtr;
    Dcm_NegativeResponseCodeType responseCode;
    /* @req DCM628 */
    boolean serviceSupported = DsdDspCheckServiceSupportedInActiveSessionAndSecurity(SID_INPUT_OUTPUT_CONTROL_BY_IDENTIFIER);
    for(uint16 i = 0u; i < DCM_NOF_IOCONTROL_DIDS; i++) {
        if( IOControlStateList[i].controlActive ) {
            /* Control not in the hands of the ECU.
             * Return it. */
            if(lookupNonDynamicDid(IOControlStateList[i].did, &DidPtr)) {
                DidControl = DidPtr->DspDidInfoRef->DspDidAccess.DspDidControl;
                if(NULL_PTR != DidControl) {
                    boolean returnToECU = TRUE;
                    if(serviceSupported && checkSessionAndSecLevel) {
                        /* Should check if supported in session and security level */
                        if( DspCheckSessionLevel(DidControl->DspDidControlSessionRef) && DspCheckSecurityLevel(DidControl->DspDidControlSecurityLevelRef) ) {
                            /* Control is supported in current session and security level.
                             * Do not return control to ECU. */
                            returnToECU = FALSE;
                        }
                    }
                    if( returnToECU ) {
                        /* Return control to the ECU */
                        for( uint16 sigIndex = 0; sigIndex < DidPtr->DspNofSignals; sigIndex++ ) {
                            if( IOControlStateList[i].activeSignalBitfield[sigIndex/8] & TO_SIGNAL_BIT(sigIndex) ) {
                                signalPtr = &DidPtr->DspSignalRef[sigIndex];
                                if ((signalPtr->DspSignalDataRef->DspDataUsePort == DATA_PORT_ECU_SIGNAL) && (signalPtr->DspSignalDataRef->DspDataReturnControlToEcuFnc.EcuSignalReturnControlToECUFnc != NULL_PTR)){
                                    (void)signalPtr->DspSignalDataRef->DspDataReturnControlToEcuFnc.EcuSignalReturnControlToECUFnc(DCM_RETURN_CONTROL_TO_ECU, NULL_PTR);
                                }
                                else if(signalPtr->DspSignalDataRef->DspDataReturnControlToEcuFnc.FuncReturnControlToECUFnc != NULL_PTR) {
                                    (void)signalPtr->DspSignalDataRef->DspDataReturnControlToEcuFnc.FuncReturnControlToECUFnc(DCM_INITIAL, &responseCode);
                                }
                                IOControlStateList[i].activeSignalBitfield[sigIndex/8] &= ~(TO_SIGNAL_BIT(sigIndex));
                            }
                        }
                        IOControlStateList[i].controlActive = FALSE;
                    } else {
                        /* Control is supported in the current session and security level */
                    }
                } else {
                    /* No control access. */
                    DCM_DET_REPORTERROR(DCM_GLOBAL_ID, DCM_E_UNEXPECTED_EXECUTION);
                    IOControlStateList[i].controlActive = FALSE;
                }
            } else {
                /* Did not found in config. Strange.. */
                IOControlStateList[i].controlActive = FALSE;
                DCM_DET_REPORTERROR(DCM_GLOBAL_ID, DCM_E_UNEXPECTED_EXECUTION);
            }
        }
    }
}

/**
 * Function to mark did signals as currently under control.
 * @param didNr
 * @param signalAffected
 * @param active
 */
static void DspIOControlSetActive(uint16 didNr, Dcm_DspIOControlVector signalAffected, boolean active)
{
    uint16 unusedIndex = DCM_NOF_IOCONTROL_DIDS;
    uint16 didIndex = DCM_NOF_IOCONTROL_DIDS;
    boolean indexOK = TRUE;
    uint16 indx = 0u;

    for(uint16 i = 0u; (i < DCM_NOF_IOCONTROL_DIDS) && (DCM_NOF_IOCONTROL_DIDS == didIndex); i++) {
        if(didNr == IOControlStateList[i].did) {
            didIndex = i;
        } else if( (DCM_NOF_IOCONTROL_DIDS == unusedIndex) && !IOControlStateList[i].controlActive ) {
            unusedIndex = i;
        }
    }

    if( DCM_NOF_IOCONTROL_DIDS > didIndex ) {
        indx = didIndex;
    } else if( active && (DCM_NOF_IOCONTROL_DIDS > unusedIndex) ) {
        indx = unusedIndex;
    } else {
        indexOK = FALSE;
    }

    if( indexOK ) {
        /* Did was in list or found and unused slot and should activate */

        IOControlStateList[indx].controlActive = FALSE;
        IOControlStateList[indx].did = didNr;
        for( uint8 byteIndex = 0; byteIndex < sizeof(Dcm_DspIOControlVector); byteIndex++ ) {
            if( active ) {
                IOControlStateList[indx].activeSignalBitfield[byteIndex] |= signalAffected[byteIndex];
            } else {
                IOControlStateList[indx].activeSignalBitfield[byteIndex] &= ~(signalAffected[byteIndex]);
            }
            if(IOControlStateList[indx].activeSignalBitfield[byteIndex]) {
                IOControlStateList[indx].controlActive = TRUE;
            }
        }
    } else if( active ) {
        /* Should set control active but could not find an entry
         * to use. */
        DCM_DET_REPORTERROR(DCM_GLOBAL_ID, DCM_E_UNEXPECTED_EXECUTION);
    }
}

/**
 * Function used for returning control to ECU for data where
 * control has been taken over but a positive response has not yet been sent.
 * @param didNr
 * @param signalActivated
 */
static void DspIOControlStopActivated(uint16 didNr, Dcm_DspIOControlVector signalActivated)
{
    const Dcm_DspDidType *DidPtr = NULL_PTR;
    const Dcm_DspSignalType *signalPtr;
    Dcm_NegativeResponseCodeType responseCode;
    boolean done = FALSE;
    Dcm_DspIOControlVector currentlyActive;
    memset(currentlyActive, 0, sizeof(Dcm_DspIOControlVector));
    if(lookupNonDynamicDid(didNr, &DidPtr)) {
        for(uint16 i = 0; (i < DCM_NOF_IOCONTROL_DIDS) && !done; i++) {
            if(didNr == IOControlStateList[i].did) {
                memcpy(currentlyActive, IOControlStateList[i].activeSignalBitfield, sizeof(Dcm_DspIOControlVector));
                done = TRUE;
            }
        }
    }

    if(NULL_PTR != DidPtr) {
        for( uint16 sigIndex = 0; sigIndex < DidPtr->DspNofSignals; sigIndex++ ) {
            if( (0 != (currentlyActive[sigIndex/8] & TO_SIGNAL_BIT(sigIndex))) &&
                (0 != (signalActivated[sigIndex/8] & TO_SIGNAL_BIT(sigIndex))) ) {
                /* Control was activated and was activated again by current request */
                signalPtr = &DidPtr->DspSignalRef[sigIndex];
                if ((signalPtr->DspSignalDataRef->DspDataUsePort == DATA_PORT_ECU_SIGNAL) && (signalPtr->DspSignalDataRef->DspDataReturnControlToEcuFnc.EcuSignalReturnControlToECUFnc != NULL_PTR)){
                    (void)signalPtr->DspSignalDataRef->DspDataReturnControlToEcuFnc.EcuSignalReturnControlToECUFnc(DCM_RETURN_CONTROL_TO_ECU, NULL_PTR);
                }
                else if(signalPtr->DspSignalDataRef->DspDataReturnControlToEcuFnc.FuncReturnControlToECUFnc != NULL_PTR) {
                    (void)signalPtr->DspSignalDataRef->DspDataReturnControlToEcuFnc.FuncReturnControlToECUFnc(DCM_INITIAL, &responseCode);
                }
            }
        }
    }
}

/* This is used when the port is USE_ECU_SIGNAL i.e. calling IOHWAB */
static Std_ReturnType EcuSignalInputOutputControl(const Dcm_DspDataType *DataPtr, Dcm_IOControlParameterType action, uint8* controlOptionRecord, Dcm_NegativeResponseCodeType* responseCode) {
    /* @req DCM580 */
    *responseCode = DCM_E_REQUESTOUTOFRANGE; // Value to use if no callback found
    Std_ReturnType retVal = E_NOT_OK;
    /* @req DCM579 */
    switch(action) {
        case DCM_RETURN_CONTROL_TO_ECU:
            if (DataPtr->DspDataReturnControlToEcuFnc.EcuSignalReturnControlToECUFnc != NULL_PTR) {
                *responseCode = DCM_E_POSITIVERESPONSE;
                retVal = DataPtr->DspDataReturnControlToEcuFnc.EcuSignalReturnControlToECUFnc(DCM_RETURN_CONTROL_TO_ECU, NULL_PTR);
            }
            break;
        case DCM_RESET_TO_DEFAULT:
            if (DataPtr->DspDataResetToDefaultFnc.EcuSignalResetToDefaultFnc != NULL_PTR) {
                *responseCode = DCM_E_POSITIVERESPONSE;
                retVal = DataPtr->DspDataResetToDefaultFnc.EcuSignalResetToDefaultFnc(DCM_RESET_TO_DEFAULT, NULL_PTR);
            }
            break;
        case DCM_FREEZE_CURRENT_STATE:
            if (DataPtr->DspDataFreezeCurrentStateFnc.EcuSignalFreezeCurrentStateFnc != NULL_PTR) {
                *responseCode = DCM_E_POSITIVERESPONSE;
                retVal = DataPtr->DspDataFreezeCurrentStateFnc.EcuSignalFreezeCurrentStateFnc(DCM_FREEZE_CURRENT_STATE, NULL_PTR);
            }
            break;
        case DCM_SHORT_TERM_ADJUSTMENT:
            if (DataPtr->DspDataShortTermAdjustmentFnc.EcuSignalShortTermAdjustmentFnc != NULL_PTR) {
                *responseCode = DCM_E_POSITIVERESPONSE;
                retVal = DataPtr->DspDataShortTermAdjustmentFnc.EcuSignalShortTermAdjustmentFnc(DCM_SHORT_TERM_ADJUSTMENT, controlOptionRecord);
            }
            break;
        default:
            break;
    }

    return retVal;
}

/* This is used when the port is not USE_ECU_SIGNAL */
static Std_ReturnType FunctionInputOutputControl(const Dcm_DspDataType *DataPtr, Dcm_IOControlParameterType action, Dcm_OpStatusType opStatus, uint8* controlOptionRecord, Dcm_NegativeResponseCodeType* responseCode) {

    *responseCode = DCM_E_REQUESTOUTOFRANGE; // Value to use if no callback found
    Std_ReturnType retVal = E_NOT_OK;
    /* @req DCM579 */
    switch(action) {
        case DCM_RETURN_CONTROL_TO_ECU:
            if(DataPtr->DspDataReturnControlToEcuFnc.FuncReturnControlToECUFnc != NULL_PTR) {
                retVal = DataPtr->DspDataReturnControlToEcuFnc.FuncReturnControlToECUFnc(opStatus ,responseCode);
            }
            break;
        case DCM_RESET_TO_DEFAULT:
            if(DataPtr->DspDataResetToDefaultFnc.FuncResetToDefaultFnc != NULL_PTR) {
                retVal = DataPtr->DspDataResetToDefaultFnc.FuncResetToDefaultFnc(opStatus ,responseCode);
            }
            break;
        case DCM_FREEZE_CURRENT_STATE:
            if(DataPtr->DspDataFreezeCurrentStateFnc.FuncFreezeCurrentStateFnc != NULL_PTR) {
                retVal = DataPtr->DspDataFreezeCurrentStateFnc.FuncFreezeCurrentStateFnc(opStatus ,responseCode);
            }
            break;
        case DCM_SHORT_TERM_ADJUSTMENT:
            if(DataPtr->DspDataShortTermAdjustmentFnc.FuncShortTermAdjustmentFnc != NULL_PTR) {
                retVal = DataPtr->DspDataShortTermAdjustmentFnc.FuncShortTermAdjustmentFnc(controlOptionRecord, opStatus, responseCode);
            }
            break;
        default:
            break;
    }

    return retVal;
}
#endif

#ifdef DCM_USE_SERVICE_INPUTOUTPUTCONTROLBYIDENTIFIER
#ifdef DCM_USE_CONTROL_DIDS
/**
 * Runs IO control for a DID
 * @param DidPtr
 * @param action
 * @param Data
 * @param pendingPtr
 * @param initOpStatus
 * @param ctrlEnableMaskPtr
 * @return
 */
static Dcm_NegativeResponseCodeType IOControlExecute(const Dcm_DspDidType *DidPtr, Dcm_IOControlParameterType action, uint8 *Data, DspUdsIOControlPendingType *pendingPtr, Dcm_OpStatusType initOpStatus, uint8 *ctrlEnableMaskPtr)
{
    Std_ReturnType retVal = E_OK;
    Dcm_OpStatusType opStatus = initOpStatus;
    Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;
    const Dcm_DspSignalType *signalPtr;
    /* @req DCM680 */
    for(uint16 sigIndex = pendingPtr->pendingSignalIndex; (sigIndex < DidPtr->DspNofSignals) && (E_OK == retVal) && (DCM_E_POSITIVERESPONSE == responseCode); sigIndex++) {
        signalPtr = &DidPtr->DspSignalRef[sigIndex];
        /* @req DCM581 */
        if( (NULL_PTR == ctrlEnableMaskPtr) || (ctrlEnableMaskPtr[sigIndex/8] & TO_SIGNAL_BIT(sigIndex)) ) {
            /* @req DCM396 */
            /* @req DCM397 */
            /* @req DCM398 */
            /* @req DCM399 */
            if ( signalPtr->DspSignalDataRef->DspDataUsePort == DATA_PORT_ECU_SIGNAL) {
                retVal = EcuSignalInputOutputControl(signalPtr->DspSignalDataRef, action, &Data[(signalPtr->DspSignalBitPosition / 8)], &responseCode);
            } else {
                retVal = FunctionInputOutputControl(signalPtr->DspSignalDataRef, action, opStatus, &Data[(signalPtr->DspSignalBitPosition / 8)], &responseCode);
            }
            pendingPtr->state = DCM_GENERAL_IDLE;
            pendingPtr->pendingSignalIndex = 0;
            switch(retVal) {
                case E_OK:
                    pendingPtr->signalAffected[sigIndex/8] |= TO_SIGNAL_BIT(sigIndex);
                    pendingPtr->controlActivated = TRUE;
                    opStatus = DCM_INITIAL;
                    responseCode = DCM_E_POSITIVERESPONSE;
                    break;
                case E_PENDING:
                    pendingPtr->pendingControl = TRUE;
                    pendingPtr->pendingSignalIndex = sigIndex;
                    pendingPtr->state = DCM_GENERAL_PENDING;
                    responseCode = DCM_E_RESPONSEPENDING;
                    break;
                case E_NOT_OK:
                    /* Keep response code */
                    if( DCM_E_POSITIVERESPONSE == responseCode ) {
                        responseCode = DCM_E_REQUESTOUTOFRANGE;
                    }
                    break;
                default:
                    responseCode = DCM_E_REQUESTOUTOFRANGE;
                    break;
            }
        }
    }
    return responseCode;
}

/**
 * Function used by IO control for reading DID data
 * @param DidPtr
 * @param Data
 * @param pendingPtr
 * @param initOpStatus
 * @return
 */
static Dcm_NegativeResponseCodeType IOControlReadData(const Dcm_DspDidType *DidPtr, uint8 *Data, DspUdsIOControlPendingType *pendingPtr, Dcm_OpStatusType initOpStatus)
{
    /* @req DCM682 */
    const Dcm_DspSignalType *signalPtr;
    Dcm_OpStatusType opStatus = initOpStatus;
    Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;
    Std_ReturnType retVal = E_OK;
    for( uint16 sigIndex = pendingPtr->pendingSignalIndex; (sigIndex < DidPtr->DspNofSignals) && (E_OK == retVal); sigIndex++ ) {
        signalPtr = &DidPtr->DspSignalRef[sigIndex];
        if( signalPtr->DspSignalDataRef->DspDataReadDataFnc.SynchDataReadFnc != NULL_PTR ) {
            if( (DATA_PORT_SYNCH == signalPtr->DspSignalDataRef->DspDataUsePort) || (DATA_PORT_ECU_SIGNAL == signalPtr->DspSignalDataRef->DspDataUsePort) ) {
                if( E_OK != signalPtr->DspSignalDataRef->DspDataReadDataFnc.SynchDataReadFnc(&Data[(signalPtr->DspSignalBitPosition / 8)])) {
                    /* Synch port cannot return E_PENDING */
                    DCM_DET_REPORTERROR(DCM_GLOBAL_ID, DCM_E_UNEXPECTED_RESPONSE);
                    retVal = E_NOT_OK;
                }
            } else if(DATA_PORT_ASYNCH == signalPtr->DspSignalDataRef->DspDataUsePort) {
                retVal = signalPtr->DspSignalDataRef->DspDataReadDataFnc.AsynchDataReadFnc(opStatus, &Data[(signalPtr->DspSignalBitPosition / 8)]);
            } else {
                /* Port not supported */
                DCM_DET_REPORTERROR(DCM_GLOBAL_ID, DCM_E_CONFIG_INVALID);
                retVal = E_NOT_OK;
            }
            pendingPtr->state = DCM_GENERAL_IDLE;
            opStatus = DCM_INITIAL;
            switch(retVal) {
                case E_OK:
                    break;
                case E_PENDING:
                    pendingPtr->pendingControl = FALSE;
                    pendingPtr->pendingSignalIndex = sigIndex;
                    pendingPtr->state = DCM_GENERAL_PENDING;
                    responseCode = DCM_E_RESPONSEPENDING;
                    break;
                case E_NOT_OK:
                    responseCode = DCM_E_CONDITIONSNOTCORRECT;
                    break;
                default:
                    responseCode = DCM_E_REQUESTOUTOFRANGE;
                    break;
            }
        } else {
            /* No read function */
            responseCode = DCM_E_REQUESTOUTOFRANGE;
        }
    }
    return responseCode;
}
#endif /* DCM_USE_CONTROL_DIDS */

/**
 * Implements UDS service 0x2F
 * @param pduRxData
 * @param pduTxData
 */
void DspIOControlByDataIdentifier(const PduInfoType *pduRxData,PduInfoType *pduTxData)
{
    /* @req DCM256 */
#ifdef DCM_USE_CONTROL_DIDS
    uint16 didSize = 0u;
    uint16 didNr = 0u;
    boolean didFound;
    const Dcm_DspDidType *DidPtr = NULL_PTR;
    const Dcm_DspDidControlType *DidControl = NULL_PTR;
    Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;
    Dcm_OpStatusType opStatus = DCM_PENDING;
    if( DCM_GENERAL_IDLE == IOControlData.state ) {
        IOControlData.pendingSignalIndex = 0u;
        IOControlData.pendingControl = TRUE;
        IOControlData.controlActivated = FALSE;
        memset(IOControlData.signalAffected, 0 , sizeof(IOControlData.signalAffected));
        opStatus = DCM_INITIAL;
    }
    IOControlData.pduRxData = pduRxData;
    IOControlData.pduTxData = pduTxData;
    didNr = (pduRxData->SduDataPtr[IOI_INDEX] << 8 & DCM_DID_HIGH_MASK) + (pduRxData->SduDataPtr[IOI_INDEX+1] & DCM_DID_LOW_MASK);
    didFound = lookupNonDynamicDid(didNr, &DidPtr);
    if( TRUE == lookupNonDynamicDid(didNr, &DidPtr) ) {
        didFound = TRUE;
        didSize = DidPtr->DspDidDataByteSize;
    }
    if( TRUE == IOControlData.pendingControl ) {
        if(pduRxData->SduLength >= (SID_LEN + IOI_LEN + IOCP_LEN)) {
            /* @req DCM563 */
            /* !req DCM564 */
            if(TRUE == didFound) {
                DidControl = DidPtr->DspDidInfoRef->DspDidAccess.DspDidControl;
                /* @req DCM565 */
                if((NULL_PTR != DidControl) && (DidPtr->DspNofSignals != 0) && IS_VALID_IOCTRL_PARAM(pduRxData->SduDataPtr[IOCP_INDEX]))  {
                    /* @req DCM566 */
                    if(TRUE == DspCheckSessionLevel(DidControl->DspDidControlSessionRef)) {
                        /* @req DCM567 */
                        if(TRUE == DspCheckSecurityLevel(DidControl->DspDidControlSecurityLevelRef))  {
                            uint8 *maskPtr = NULL_PTR;
                            boolean requestOk = FALSE;
                            uint16 controlOptionSize = (DCM_SHORT_TERM_ADJUSTMENT == pduRxData->SduDataPtr[IOCP_INDEX]) ? DidPtr->DspDidDataByteSize : 0u;
                            uint16 controlEnableMaskSize = (DidPtr->DspNofSignals > 1) ? ((DidPtr->DspNofSignals + 7) / 8) : 0u;
                            requestOk = (pduRxData->SduLength == (SID_LEN + IOI_LEN + IOCP_LEN + controlOptionSize + controlEnableMaskSize));
                            maskPtr = (DidPtr->DspNofSignals > 1) ? (&pduRxData->SduDataPtr[COR_INDEX + controlOptionSize]) : NULL_PTR;

                            if( requestOk ) {
                                if( pduTxData->SduLength >= (SID_LEN + IOI_LEN + IOCP_LEN + didSize) ) {
                                    responseCode = IOControlExecute(DidPtr, pduRxData->SduDataPtr[IOCP_INDEX], &pduRxData->SduDataPtr[COR_INDEX], &IOControlData, opStatus, maskPtr);
                                    if( DCM_E_POSITIVERESPONSE == responseCode ) {
                                        opStatus = DCM_INITIAL;
                                    }
                                } else {
                                    /* Tx buffer not big enough */
                                    responseCode = DCM_E_RESPONSETOOLONG;
                                }
                            } else {
                                responseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
                            }
                        } else {
                            responseCode = DCM_E_SECURITYACCESSDENIED;
                        }
                    } else {
                        responseCode = DCM_E_REQUESTOUTOFRANGE;
                    }
                } else {
                    responseCode = DCM_E_REQUESTOUTOFRANGE;
                }
            } else {
                responseCode = DCM_E_REQUESTOUTOFRANGE;
            }
        } else {
            responseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
        }
    }
    if( DCM_E_POSITIVERESPONSE == responseCode ) {
        pduTxData->SduLength = SID_LEN + IOI_LEN + IOCP_LEN + didSize;
        pduTxData->SduDataPtr[1] = pduRxData->SduDataPtr[1];// did
        pduTxData->SduDataPtr[2] = pduRxData->SduDataPtr[2];// did
        pduTxData->SduDataPtr[3] = pduRxData->SduDataPtr[IOCP_INDEX];
        // IMPROVEMENT: rework this totally: use the read did implementation
        if( NULL_PTR != DidPtr ) {
            responseCode = IOControlReadData(DidPtr, &pduTxData->SduDataPtr[4], &IOControlData, opStatus);
        }
    }
    switch(pduRxData->SduDataPtr[IOCP_INDEX]) {
        case DCM_RETURN_CONTROL_TO_ECU:
            DspIOControlSetActive(didNr, IOControlData.signalAffected, FALSE);
            break;
        case DCM_RESET_TO_DEFAULT:
        case DCM_FREEZE_CURRENT_STATE:
        case DCM_SHORT_TERM_ADJUSTMENT:
            DspIOControlSetActive(didNr, IOControlData.signalAffected, TRUE);
            break;
        default:
            break;
    }
    if( DCM_E_POSITIVERESPONSE == responseCode ) {
        switch(pduRxData->SduDataPtr[IOCP_INDEX]) {
            case DCM_RETURN_CONTROL_TO_ECU:
                DspIOControlSetActive(didNr, IOControlData.signalAffected, FALSE);
                break;
            case DCM_RESET_TO_DEFAULT:
            case DCM_FREEZE_CURRENT_STATE:
            case DCM_SHORT_TERM_ADJUSTMENT:
                DspIOControlSetActive(didNr, IOControlData.signalAffected, TRUE);
                break;
            default:
                break;
        }
    } else if( IOControlData.controlActivated && (DCM_RETURN_CONTROL_TO_ECU != pduRxData->SduDataPtr[IOCP_INDEX])  && (DCM_E_RESPONSEPENDING != responseCode) ) {
        /* Will give negative response. Disable any control which was activated by this request */
        DspIOControlStopActivated(didNr, IOControlData.signalAffected);
    }
    if(DCM_E_RESPONSEPENDING != responseCode) {
        DsdDspProcessingDone(responseCode);
    }
#else
    /* No control dids configured */
    (void)pduTxData;
    if(pduRxData->SduLength >= (SID_LEN + IOI_LEN + IOCP_LEN)) {
        DsdDspProcessingDone(DCM_E_REQUESTOUTOFRANGE);
    }
    else {
        DsdDspProcessingDone(DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT);
    }
#endif
}
#endif

#ifdef DCM_USE_SERVICE_COMMUNICATIONCONTROL
/**
 * Get the rte and dcm modes correcponding to received request
 * @param subFunction
 * @param comType
 * @param rteMode
 * @param dcmMode
 * @return E_OK: modes found, E_NOT: Modes NOT found
 */
Std_ReturnType getCommunicationControlModes(uint8 subFunction, uint8 comType, uint8 *rteMode, Dcm_CommunicationModeType *dcmMode)
{
    const uint8 RteModeMap[UDS_0x28_NOF_COM_TYPES][UDS_0x28_NOF_SUB_FUNCTIONS] = {
            {RTE_MODE_DcmCommunicationControl_DCM_ENABLE_RX_TX_NORM, RTE_MODE_DcmCommunicationControl_DCM_ENABLE_RX_DISABLE_TX_NORM, RTE_MODE_DcmCommunicationControl_DCM_DISABLE_RX_ENABLE_TX_NORM, RTE_MODE_DcmCommunicationControl_DCM_DISABLE_RX_TX_NORMAL},/* Normal communication */
            {RTE_MODE_DcmCommunicationControl_DCM_ENABLE_RX_TX_NM, RTE_MODE_DcmCommunicationControl_DCM_ENABLE_RX_DISABLE_TX_NM, RTE_MODE_DcmCommunicationControl_DCM_DISABLE_RX_ENABLE_TX_NM, RTE_MODE_DcmCommunicationControl_DCM_DISABLE_RX_TX_NM},/* Network management communication */
            {RTE_MODE_DcmCommunicationControl_DCM_ENABLE_RX_TX_NORM_NM, RTE_MODE_DcmCommunicationControl_DCM_ENABLE_RX_DISABLE_TX_NORM_NM, RTE_MODE_DcmCommunicationControl_DCM_DISABLE_RX_ENABLE_TX_NORM_NM, RTE_MODE_DcmCommunicationControl_DCM_DISABLE_RX_TX_NORM_NM} /* Network management and normal communication */
    };
    const uint8 DcmModeMap[UDS_0x28_NOF_COM_TYPES][UDS_0x28_NOF_SUB_FUNCTIONS] = {
            {DCM_ENABLE_RX_TX_NORM, DCM_ENABLE_RX_DISABLE_TX_NORM, DCM_DISABLE_RX_ENABLE_TX_NORM, DCM_DISABLE_RX_TX_NORMAL},/* Normal communication */
            {DCM_ENABLE_RX_TX_NM, DCM_ENABLE_RX_DISABLE_TX_NM, DCM_DISABLE_RX_ENABLE_TX_NM, DCM_DISABLE_RX_TX_NM},/* Network management communication */
            {DCM_ENABLE_RX_TX_NORM_NM, DCM_ENABLE_RX_DISABLE_TX_NORM_NM, DCM_DISABLE_RX_ENABLE_TX_NORM_NM, DCM_DISABLE_RX_TX_NORM_NM} /* Network management and normal communication */
    };
    Std_ReturnType ret = E_NOT_OK;
    if( IS_SUPPORTED_0x28_SUBFUNCTION(subFunction) && IS_VALID_0x28_COM_TYPE(comType) ) {
        *rteMode = RteModeMap[comType - 1][subFunction];
        *dcmMode = DcmModeMap[comType - 1][subFunction];
        ret = E_OK;
    }
    return ret;
}

/**
 * Runs service communication control
 */
static Dcm_NegativeResponseCodeType DspInternalCommunicationControl(uint8 subFunction, uint8 subNet, uint8 comType, PduIdType rxPduId, boolean executeModeSwitch)
{
    const Dcm_DspComControlAllChannelType *AllChannels;
    const Dcm_DspComControlSpecificChannelType *SpecificChannels;
    uint8 rteMode;
    Dcm_CommunicationModeType dcmMode;
    Dcm_NegativeResponseCodeType responseCode = DCM_E_REQUESTOUTOFRANGE;
    if(E_OK == getCommunicationControlModes(subFunction, (comType), &rteMode, &dcmMode)) {
        if(0xFu == subNet) {
            /* Channel which request was received on */
            /* @req DCM785 */
            const Dcm_DslProtocolRxType *protocolRx = NULL_PTR;
            if( (E_OK == DsdDspGetCurrentProtocolRx(rxPduId, &protocolRx)) && (NULL_PTR != protocolRx) ) {
                if( executeModeSwitch ) {
                    (void)Dcm_ConfigPtr->DcmComMChannelCfg[protocolRx->ComMChannelInternalIndex].ModeSwitchFnc(rteMode);
                    DspChannelComMode[protocolRx->ComMChannelInternalIndex] = dcmMode;
#if defined(USE_BSWM)
                    BswM_Dcm_CommunicationMode_CurrentState(Dcm_ConfigPtr->DcmComMChannelCfg[protocolRx->ComMChannelInternalIndex].NetworkHandle, dcmMode);
#endif
                }
                responseCode = DCM_E_POSITIVERESPONSE;
            }
        } else {
            if(NULL_PTR != Dcm_ConfigPtr->Dsp->DspComControl) {
                if( 0u == subNet ) {
                    /* All channels */
                    /* @req DCM512 */
                    AllChannels = Dcm_ConfigPtr->Dsp->DspComControl->DspControlAllChannel;
                    while(!AllChannels->Arc_EOL) {
                        if( executeModeSwitch ) {
                            (void)Dcm_ConfigPtr->DcmComMChannelCfg[AllChannels->ComMChannelIndex].ModeSwitchFnc(rteMode);
                            DspChannelComMode[AllChannels->ComMChannelIndex] = dcmMode;
#if defined(USE_BSWM)
                            BswM_Dcm_CommunicationMode_CurrentState(Dcm_ConfigPtr->DcmComMChannelCfg[AllChannels->ComMChannelIndex].NetworkHandle, dcmMode);
#endif
                        }
                        AllChannels++;
                        responseCode = DCM_E_POSITIVERESPONSE;
                    }
                } else {
                    /* Specific channels */
                    /* @req DCM786 */
                    SpecificChannels = Dcm_ConfigPtr->Dsp->DspComControl->DspControlSpecificChannel;
                    while(!SpecificChannels->Arc_EOL) {
                        if( subNet == SpecificChannels->SubnetNumber ) {
                            if( executeModeSwitch ) {
                                (void)Dcm_ConfigPtr->DcmComMChannelCfg[SpecificChannels->ComMChannelIndex].ModeSwitchFnc(rteMode);
                                DspChannelComMode[SpecificChannels->ComMChannelIndex] = dcmMode;
#if defined(USE_BSWM)
                                BswM_Dcm_CommunicationMode_CurrentState(Dcm_ConfigPtr->DcmComMChannelCfg[SpecificChannels->ComMChannelIndex].NetworkHandle, dcmMode);
#endif
                            }
                            responseCode = DCM_E_POSITIVERESPONSE;
                        }
                        SpecificChannels++;
                    }
                }
            }
        }
    }
    return responseCode;
}
/**
 * Implements UDS service 0x28 (CommunicationControl)
 * @param pduRxData
 * @param pduTxData
 */
void DspCommunicationControl(const PduInfoType *pduRxData,PduInfoType *pduTxData, PduIdType rxPduId, PduIdType txConfirmId)
{
    /* @req DCM511 */
    /* !req DCM753 */
    Dcm_NegativeResponseCodeType responseCode = DCM_E_REQUESTOUTOFRANGE;
    uint8 subFunction = pduRxData->SduDataPtr[SF_INDEX];
    uint8 subNet;
    uint8 comType;
    if(pduRxData->SduLength == 3) {
        if( IS_SUPPORTED_0x28_SUBFUNCTION(subFunction) ) {
            subNet = pduRxData->SduDataPtr[CC_CTP_INDEX] >> 4u;
            comType = pduRxData->SduDataPtr[CC_CTP_INDEX] & 3u;
            responseCode = DspInternalCommunicationControl(subFunction, subNet, comType, rxPduId, FALSE);

            if(DCM_E_POSITIVERESPONSE == responseCode) {
                communicationControlData.comControlPending = TRUE;
                communicationControlData.subFunction = subFunction;
                communicationControlData.subnet = subNet;
                communicationControlData.comType = comType;
                communicationControlData.confirmPdu = txConfirmId;
                communicationControlData.rxPdu = rxPduId;
            } else {
                communicationControlData.comControlPending = FALSE;
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
	const Dcm_DspPidType *dspPid = Dcm_ConfigPtr->Dsp->DspPid;
    boolean pidFound = FALSE;

    if(dspPid != NULL_PTR) {
        for ( ;(!dspPid->Arc_EOL); dspPid++) {
            if ((dspPid->DspPidIdentifier == pidId) &&
                    ((DCM_SERVICE_01_02 == dspPid->DspPidService) || (service == dspPid->DspPidService)) &&
                    (*dspPid->Arc_DcmPidEnabled == TRUE))
            {
                pidFound = TRUE;
                *PidPtr = dspPid;
                /* terminate the loop using break statement */
                break;
            }
            else {
                /*do nothing*/
            }
        }
    }
    else {
        /*do nothing*/
    }

    return pidFound;
}
#endif

#if defined(DCM_USE_SERVICE_ONBOARDMONITORINGTESTRESULTSSPECIFICMONITOREDSYSTEMS)
/**
 * Looks up for the OBDMID available in the configuration list(OBD Service $06)
 * @param obdmid
 * @param obdmidPtr
 * @return TRUE (found) or FALSE (not found)
 */
static boolean lookupObdmid(uint8 obdmid, const Dcm_DspTestResultObdmidType **obdmidPtr)
{
    boolean obdidFound = FALSE;
    const Dcm_DspTestResultObdmidType *dspObdmid = Dcm_ConfigPtr->Dsp->DspTestResultByObdmid->DspTestResultObdmidTid;
    if(dspObdmid != NULL_PTR){
        do{
            if(dspObdmid->DspTestResultObdmid == obdmid){
                obdidFound = TRUE;
                *obdmidPtr = dspObdmid;
                break;
            }
            dspObdmid++;
        }while(!dspObdmid->Arc_EOL);
    }
    return obdidFound;
}

/**
 * Fills the 4 byte data according to the result data required by ISO 15031-5 (OBD Service $06)
 * depending up on the supported OBDMIDs
 * @param obdmid
 * @param data
 * @return TRUE (Updated) or FALSE (No update)
 */
static boolean setAvailabilityObdmidValue(uint8 obdmid, uint32 *data)
{
    uint8 shift;
    uint32 obdmidData = 0u;
    uint32 temp;
    boolean setOk = FALSE;
    const Dcm_DspTestResultObdmidType *dspObdmid = Dcm_ConfigPtr->Dsp->DspTestResultByObdmid->DspTestResultObdmidTid;

    if(dspObdmid != NULL_PTR) {
        while (0 == dspObdmid->Arc_EOL) {
            if((dspObdmid->DspTestResultObdmid >= (obdmid + AVAIL_TO_SUPPORTED_OBDMID_OFFSET_MIN)) &&
                /*	ISO 15031-5 says to do <= for the below line, but configurator does not supoort it and hence there is elseif part */
               (dspObdmid->DspTestResultObdmid < (obdmid + AVAIL_TO_SUPPORTED_OBDMID_OFFSET_MAX))) {
                shift = dspObdmid->DspTestResultObdmid - obdmid;
                temp = (uint32)1 << (AVAIL_TO_SUPPORTED_OBDMID_OFFSET_MAX - shift);
                obdmidData |= temp;
            } else if(dspObdmid->DspTestResultObdmid >  (obdmid + AVAIL_TO_SUPPORTED_OBDMID_OFFSET_MAX)) {/* check any subsequent range */
                obdmidData |= (uint32)1;
            } else {
                /*do nothing*/
            }
            dspObdmid++;
        }
    }
    if(0 != obdmidData) {
        setOk = TRUE;
    }
    (*data) = obdmidData; /* 0 if not found */

    return setOk;
}
#endif
#ifdef DCM_USE_SERVICE_REQUESTCURRENTPOWERTRAINDIAGNOSTICDATA
static boolean setAvailabilityPidValue(uint8 Pid, Dcm_PidServiceType service, uint32 *Data)
{
    uint8 shift;
    uint32 pidData = 0u;
    uint32 temp;
    boolean setOk = TRUE;
    const Dcm_DspPidType *dspPid = Dcm_ConfigPtr->Dsp->DspPid;

    if(dspPid != NULL_PTR) {
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
    /* !req DCM243 */
    /* !req DCM621 */
    /* !req DCM622 */
    uint16 nofAvailabilityPids = 0u;
    uint16 findPid = 0u;
    uint16 txPos = SID_LEN;
    uint32 DATA = 0u;
    uint16 txLength = SID_LEN;
    uint16 pidNum = pduRxData->SduLength - SID_LEN;
    const Dcm_DspPidType *sourcePidPtr = NULL_PTR;
    Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;
    /* @req OBD_REQ_3 */
    if((pduRxData->SduLength >= OBD_REQ_MESSAGE_LEN_ONE_MIN) && (pduRxData->SduLength <= OBD_REQ_MESSAGE_LEN_MAX)) {
        for(uint16 i = 0u; i < pidNum; i++) { /*figure out the txLength to be sent*/
            /*situation of availability Pids*/
            if( IS_AVAILABILITY_PID(pduRxData->SduDataPtr[i + 1]) ) {
                nofAvailabilityPids++; /*used to judge if the message is valid, if nofAvailabilityPids != 0 or Pidnum, invalid*/
                txLength += PID_LEN + SUPPRTED_PIDS_DATA_LEN;
            }
            /*situation of supported Pids*/
            else if(TRUE == lookupPid(pduRxData->SduDataPtr[i + 1], DCM_SERVICE_01, &sourcePidPtr)) {
                txLength += PID_LEN + sourcePidPtr->DspPidSize;
            }
            else  {
                /*do nothing*/
            }
        }
        /*@req OBD_DCM_REQ_7*/
        if(txLength <= pduTxData->SduLength) { /*if txLength is smaller than the configured length*/
            if(pidNum == nofAvailabilityPids) { /*check if all the request PIDs are the 0x00...0xE0 format*/
                /* @req DCM407 */
                for(uint16 i = 0;i < pidNum;i++) {	/*Check the PID configuration,find which PIDs were configured for 0x00,0x20,0x40 respectively,and fill in the pduTxBuffer,and count the txLength*/
                    /*@OBD_DCM_REQ_3,@OBD_DCM_REQ_6*/
                    if(TRUE == setAvailabilityPidValue(pduRxData->SduDataPtr[i + 1], DCM_SERVICE_01, &DATA)) {
                        pduTxData->SduDataPtr[txPos++] = pduRxData->SduDataPtr[i + 1];
                        /*take every byte of uint32 DATA,and fill in txbuffer*/
                        pduTxData->SduDataPtr[txPos++] = (uint8)(((DATA) & (OBD_DATA_LSB_MASK << OFFSET_THREE_BYTES)) >> OFFSET_THREE_BYTES);
                        pduTxData->SduDataPtr[txPos++] = (uint8)(((DATA) & (OBD_DATA_LSB_MASK << OFFSET_TWO_BYTES)) >> OFFSET_TWO_BYTES);
                        pduTxData->SduDataPtr[txPos++] = (uint8)(((DATA) & (OBD_DATA_LSB_MASK << OFFSET_ONE_BYTE)) >> OFFSET_ONE_BYTE);
                        pduTxData->SduDataPtr[txPos++] = (uint8)((DATA) & OBD_DATA_LSB_MASK);
                    }
                    else if(PIDZERO == pduRxData->SduDataPtr[i + 1]) {
                        pduTxData->SduDataPtr[txPos++] = pduRxData->SduDataPtr[i + 1];
                        pduTxData->SduDataPtr[txPos++] = DATAZERO;
                        pduTxData->SduDataPtr[txPos++] = DATAZERO;
                        pduTxData->SduDataPtr[txPos++] = DATAZERO;
                        pduTxData->SduDataPtr[txPos++] = DATAZERO;
                    }
                    else {
                        findPid++;
                    }
                }
            }
            else if(0 == nofAvailabilityPids) { /*check if all the request PIDs are the supported PIDs,like 0x01,0x02...*/
                for(uint16 i = 0u; i < pidNum; i++) {
                    if(TRUE == lookupPid(pduRxData->SduDataPtr[i + 1], DCM_SERVICE_01, &sourcePidPtr)) {
                        /*@req OBD_DCM_REQ_3,OBD_DCM_REQ_5,OBD_DCM_REQ_8*//* @req OBD_REQ_2 */
                        /* !req DCM623 */
                        if(E_OK == sourcePidPtr->DspGetPidValFnc(&pduTxData->SduDataPtr[txPos+1])) {/* @req DCM408 */
                            pduTxData->SduDataPtr[txPos] = pduRxData->SduDataPtr[i + 1];
                            txPos += (sourcePidPtr->DspPidSize + 1);
                        }
                        else {
                            responseCode = DCM_E_CONDITIONSNOTCORRECT;
                            break;
                        }
                    }
                    else {
                        findPid++;
                    }
                }
            }
            else {
                responseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
            }
            if(pidNum == findPid) {
                responseCode = DCM_E_REQUESTOUTOFRANGE;
            }
            else {
                /*do nothing*/
            }
        }
        else {
            responseCode = DCM_E_REQUESTOUTOFRANGE;
        }
    }
    else {
        responseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
    }
    if(DCM_E_POSITIVERESPONSE == responseCode)
    {
        pduTxData->SduLength = txPos;
    }
    else {
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
    /* !req DCM244 */
    /* @req DCM287 */
    uint16 nofAvailabilityPids = 0u;
    uint16 findPid = 0u;
    uint32 dtc = 0;
    uint32 supportBitfield = 0u;
    uint16 txPos = SID_LEN;
    uint16 txLength = SID_LEN;
    uint16 messageLen = pduRxData->SduLength;
    const Dcm_DspPidType *sourcePidPtr = NULL_PTR;
    Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;

    /* @req OBD_REQ_6 */
    if((messageLen >= OBD_REQ_MESSAGE_LEN_TWO_MIN ) && (messageLen <= OBD_REQ_MESSAGE_LEN_MAX ) && (((messageLen - 1) % 2) == 0)) {
        uint16 pidNum = ((messageLen - 1) >> 1);
        const uint8* PIDAndFramePtr = &pduRxData->SduDataPtr[1];
        /*find out PID and FFnum*/
        for(uint16 i = 0u; i < pidNum; i++) {
            /* Calulate tx length */
            if(PIDAndFramePtr[OBD_SERVICE_2_PID_INDEX] == OBD_SERVICE_TWO) {
                txLength += PID_LEN + FF_NUM_LEN + OBD_DTC_LEN;
            }
            else if( IS_AVAILABILITY_PID(PIDAndFramePtr[OBD_SERVICE_2_PID_INDEX]) ) {
                nofAvailabilityPids++; /*used to judge if the message is valid, if nofAvailabilityPids != 0 or Pidnum, invalid*/
                txLength += PID_LEN + SUPPRTED_PIDS_DATA_LEN;
            }
            else if(TRUE == lookupPid(PIDAndFramePtr[OBD_SERVICE_2_PID_INDEX], DCM_SERVICE_02, &sourcePidPtr)) {
                txLength += PID_LEN + FF_NUM_LEN + sourcePidPtr->DspPidSize;
            }
            else {
                /*do nothing*/
            }
            PIDAndFramePtr += OBD_SERVICE_2_PID_AND_FRAME_SIZE;
        }
        /*@req OBD_DCM_REQ_7*/
        if(txLength <= (pduTxData->SduLength)) {
            if(pidNum == nofAvailabilityPids) { /*check if all the request PIDs are the 0x00...0xE0 format*/
                PIDAndFramePtr = &pduRxData->SduDataPtr[1];
                /* @req DCM284 */
                for(uint16 i = 0; i < pidNum; i++) {	/*Check the PID configuration,find which PIDs were configured for 0x00,0x20,0x40 respectively,and fill in the pduTxBuffer,and count the txLength*/
                    if(PIDAndFramePtr[OBD_SERVICE_2_FRAME_INDEX] == RECORD_NUM_ZERO) { /* @req DCM409 */
                        if(TRUE == setAvailabilityPidValue(PIDAndFramePtr[OBD_SERVICE_2_PID_INDEX], DCM_SERVICE_02, &supportBitfield)) {
                            pduTxData->SduDataPtr[txPos++] = PIDAndFramePtr[OBD_SERVICE_2_PID_INDEX];
                            pduTxData->SduDataPtr[txPos++] = RECORD_NUM_ZERO;
                            /*take every byte of uint32 DATA,and fill in txbuffer*/
                            pduTxData->SduDataPtr[txPos++] = (uint8)(((supportBitfield) & (OBD_DATA_LSB_MASK << OFFSET_THREE_BYTES)) >> OFFSET_THREE_BYTES);
                            pduTxData->SduDataPtr[txPos++] = (uint8)(((supportBitfield) & (OBD_DATA_LSB_MASK << OFFSET_TWO_BYTES)) >> OFFSET_TWO_BYTES);
                            pduTxData->SduDataPtr[txPos++] = (uint8)(((supportBitfield) & (OBD_DATA_LSB_MASK << OFFSET_ONE_BYTE)) >> OFFSET_ONE_BYTE);
                            pduTxData->SduDataPtr[txPos++] = (uint8)((supportBitfield) & OBD_DATA_LSB_MASK);
                        } else if(PIDZERO == PIDAndFramePtr[OBD_SERVICE_2_PID_INDEX]) {
                            pduTxData->SduDataPtr[txPos++] = PIDAndFramePtr[OBD_SERVICE_2_PID_INDEX];
                            pduTxData->SduDataPtr[txPos++] = RECORD_NUM_ZERO;
                            pduTxData->SduDataPtr[txPos++] = DATAZERO;
                            pduTxData->SduDataPtr[txPos++] = DATAZERO;
                            pduTxData->SduDataPtr[txPos++] = DATAZERO;
                            pduTxData->SduDataPtr[txPos++] = DATAZERO;
                        } else {
                            findPid++;
                        }
                    }
                    else {
                        /*@req OBD_DCM_REQ_11*/
                        responseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
                        break;
                    }
                    PIDAndFramePtr += OBD_SERVICE_2_PID_AND_FRAME_SIZE;
                }
            }
            else if(0 == nofAvailabilityPids) { /*check if all the request PIDs are the supported PIDs,like 0x01,0x02...*/
                PIDAndFramePtr = &pduRxData->SduDataPtr[1];
                for(uint16 i = 0; i < pidNum; i++) {
                    /*@req OBD_DCM_REQ_10*/
                    if(PIDAndFramePtr[OBD_SERVICE_2_FRAME_INDEX] == RECORD_NUM_ZERO) { /* @req DCM409 */
                        uint8 bufSize = 0;
                        if(PIDAndFramePtr[OBD_SERVICE_2_PID_INDEX] == OBD_SERVICE_TWO) {
                            /*@req OBD_DCM_REQ_12,@OBD_DCM_REQ_13,@OBD_DCM_REQ_14*/
                            if(E_OK == Dem_GetDTCOfOBDFreezeFrame(RECORD_NUM_ZERO, &dtc)) { /* @req DCM279 */
                                pduTxData->SduDataPtr[txPos++] = PIDAndFramePtr[OBD_SERVICE_2_PID_INDEX];
                                pduTxData->SduDataPtr[txPos++] = RECORD_NUM_ZERO;
                                pduTxData->SduDataPtr[txPos++] = (uint8)(((dtc) & (OBD_DATA_LSB_MASK << OFFSET_TWO_BYTES)) >> OFFSET_TWO_BYTES);
                                pduTxData->SduDataPtr[txPos++] = (uint8)(((dtc) & (OBD_DATA_LSB_MASK << OFFSET_ONE_BYTE)) >> OFFSET_ONE_BYTE);
                            }
                            /*if the DTC did not cause the stored FF,DTC of 0x0000 should be returned*/
                            /* @req OBD_REQ_5 */
                            else {
                                pduTxData->SduDataPtr[txPos++] = PIDAndFramePtr[OBD_SERVICE_2_PID_INDEX];
                                pduTxData->SduDataPtr[txPos++] = RECORD_NUM_ZERO;
                                pduTxData->SduDataPtr[txPos++] = 0x00;
                                pduTxData->SduDataPtr[txPos++] = 0x00;
                            }
                        }
                        /*req OBD_DCM_REQ_17*/
                        else {
                            /*@req OBD_DCM_REQ_28*/
                            pduTxData->SduDataPtr[txPos++] = PIDAndFramePtr[OBD_SERVICE_2_PID_INDEX];
                            pduTxData->SduDataPtr[txPos++] = RECORD_NUM_ZERO;
                            bufSize = (uint8)(pduTxData->SduLength - txPos);
                            /*@req OBD_DCM_REQ_15,OBD_DCM_REQ_16*//* @req OBD_REQ_4 */
                            /* IMPROVEMENT: Dem_GetOBDFreezeFrameData should be called for each data element in the
                             * Pid. Parameter DataElementIndexOfPid should be the index of the data element
                             * within the Pid. But currently only one data element per Pid is supported. */
                            /* @req DCM286 */
                            if(E_OK == Dem_ReadDataOfOBDFreezeFrame(PIDAndFramePtr[OBD_SERVICE_2_PID_INDEX], DATA_ELEMENT_INDEX_OF_PID_NOT_SUPPORTED, &(pduTxData->SduDataPtr[txPos]), &bufSize)) {
                                txPos += bufSize;
                            }
                            else {
                                txPos -= 2;
                                findPid++;
                            }
                        }
                    }
                    else {
                        /*@req OBD_DCM_REQ_11*/
                        responseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
                        break;
                    }
                    PIDAndFramePtr += OBD_SERVICE_2_PID_AND_FRAME_SIZE;
                }

            }
            else {
                responseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
            }
            if(pidNum == findPid) {
                responseCode = DCM_E_REQUESTOUTOFRANGE;
            }
            else {
                /*do nothing*/
            }
        }
        else {
            responseCode = DCM_E_REQUESTOUTOFRANGE;
        }
    }
    else {
        responseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
    }
    if(DCM_E_POSITIVERESPONSE == responseCode) {
        pduTxData->SduLength = txPos;
    }
    else {
        /*do nothing*/
    }
    DsdDspProcessingDone(responseCode);

    return;
}
#endif

#if defined(USE_DEM) && defined(DCM_USE_SERVICE_CLEAREMISSIONRELATEDDIAGNOSTICDATA)
/**
 * Implements OBD service $04 - Clear/Reset emission-related diagnostic information
 * @param pduRxData
 * @param pduTxData
 */
void DspObdClearEmissionRelatedDiagnosticData(const PduInfoType *pduRxData,PduInfoType *pduTxData)
{
    /* !req DCM246 */
    uint16 messageLen = pduRxData->SduLength;
    Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;

    if(messageLen == SID_LEN ) {
        /* @req DCM004 */
        /* @req DCM413 */
        if(DEM_CLEAR_OK == Dem_ClearDTC(DEM_DTC_GROUP_ALL_DTCS, DEM_DTC_FORMAT_OBD, DEM_DTC_ORIGIN_PRIMARY_MEMORY)) {
            /*do nothing*/
        }
        else {
            /* @req DCM704 */
            /* !req DCM703 */
            responseCode = DCM_E_CONDITIONSNOTCORRECT;
        }
    }
    else {
        responseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
    }

    if(DCM_E_POSITIVERESPONSE == responseCode) {
        pduTxData->SduLength = SID_LEN;
    }

    DsdDspProcessingDone(responseCode);

    return;
}
#endif

#if defined(USE_DEM)
#if defined(DCM_USE_SERVICE_REQUESTEMISSIONRELATEDDIAGNOSTICTROUBLECODES) || \
    defined(DCM_USE_SERVICE_REQUESTEMISSIONRELATEDDTCSDETECTEDDURINGCURRENTORLASTCOMPLETEDDRIVINGCYCLE) || \
    defined(DCM_USE_SERVICE_REQUESTEMISSIONRELATEDDTCSWITHPERMANENTSTATUS)
/**
 * Reads DTCs from Dem according to diag request
 * @param pduTxData
 * @param setDtcFilterResult
 * @return NRC
 */
static Dcm_NegativeResponseCodeType OBD_Sevice_03_07_0A(PduInfoType *pduTxData, Dem_ReturnSetFilterType setDtcFilterResult)
{
    Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;

    if (setDtcFilterResult == DEM_FILTER_ACCEPTED) {
        uint32 dtc;
        Dem_EventStatusExtendedType dtcStatus;
        uint8 nrOfDtcs = 0;
        uint16 indx = 2;

        while ((Dem_GetNextFilteredDTC(&dtc, &dtcStatus)) == DEM_FILTERED_OK) {

            if((indx + LENGTH_OF_DTC) >= (pduTxData->SduLength)) {
                responseCode = DCM_E_REQUESTOUTOFRANGE;
                break;
            }
            /* @req OBD_REQ_9 */
            pduTxData->SduDataPtr[indx] = (uint8)EMISSION_DTCS_HIGH_BYTE(dtc);
            pduTxData->SduDataPtr[1+indx] = (uint8)EMISSION_DTCS_LOW_BYTE(dtc);
            indx += LENGTH_OF_DTC;
            nrOfDtcs++;

        }
        /* @req OBD_REQ_8 */
        if(responseCode == DCM_E_POSITIVERESPONSE) {
            pduTxData->SduLength = indx;
            pduTxData->SduDataPtr[1] = nrOfDtcs;
        }

    }
    else {
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
    /* !req DCM245 */
    uint16 messageLen = pduRxData->SduLength;
    Dem_ReturnSetFilterType setDtcFilterResult;
    Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;

    if(messageLen == SID_LEN ) {
        /*"confirmed" diagnostic trouble codes*/
        /*@req OBD_DCM_REQ_1*/	/*@req OBD_DCM_REQ_24*/
        /* @req DCM289 */
        setDtcFilterResult = Dem_SetDTCFilter(DEM_CONFIRMED_DTC, DEM_DTC_KIND_EMISSION_REL_DTCS, DEM_DTC_FORMAT_OBD,
                                            DEM_DTC_ORIGIN_PRIMARY_MEMORY, DEM_FILTER_WITH_SEVERITY_NO,
                                            VALUE_IS_NOT_USED, DEM_FILTER_FOR_FDC_NO);

        responseCode = OBD_Sevice_03_07_0A(pduTxData,setDtcFilterResult);

    }
    else {
        responseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
    }

    DsdDspProcessingDone(responseCode);

}
#endif

#if defined(DCM_USE_SERVICE_ONBOARDMONITORINGTESTRESULTSSPECIFICMONITOREDSYSTEMS)
/**
 * Realisation of OBD Service $06,
 * provides OBD monitoring results for the requested OBDMID with the assist of registered SWCs
 * Provides Availabilty OBDMID list
 * loop backed here if dummy mode is ON
 * @param pduRxData
 * @param pduTxData
 * @return none
 */
void DspObdRequestOnBoardMonitoringTestResultsService06(const PduInfoType *pduRxData,PduInfoType *pduTxData){
    /* !req DCM414 */
    uint16 i;
    uint16 nofAvailabilityObdmids = 0u;
    uint8  unFoundObdmid = 0u;
    uint16 unFoundTid = 0u;
    uint16 txPos = SID_LEN;
    uint32 supportBitfield = 0u;
    uint16 txLength = SID_LEN;
    uint16 obdmidNum = pduRxData->SduLength - SID_LEN;
    const  Dcm_DspTestResultObdmidType *sourceObdmidPtr = NULL_PTR;
    Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;
    Dcm_OpStatusType opStatus = (Dcm_OpStatusType)DCM_INITIAL;
    uint16 testval = 0u;
    uint16 testMinlimit = 0u;
    uint16 testMaxlimit = 0u;
    DTRStatusType dtrStatus = 0;
    if((pduRxData->SduLength >= OBD_REQ_MESSAGE_LEN_ONE_MIN) && (pduRxData->SduLength <= OBD_REQ_MESSAGE_LEN_MAX)) {
        for(i = 0u; i < obdmidNum; i++) {
            /*situation of availability Obdmids*/
            if( IS_AVAILABILITY_OBDMID(pduRxData->SduDataPtr[OBDMID_DATA_START_INDEX + i]) ) {
                txLength += OBDMID_LEN + SUPPORTED_OBDMIDS_DATA_LEN;
                nofAvailabilityObdmids++;
            }
            /*situation of supported Obdmids*/
            else if(obdmidNum == SUPPORTED_MAX_OBDMID_REQUEST_LEN ) {
                if(TRUE == lookupObdmid(pduRxData->SduDataPtr[OBDMID_DATA_START_INDEX + i],&sourceObdmidPtr)){
                    txLength += (sourceObdmidPtr->DspTestResultTidSize * (OBDMID_LEN + SUPPORTED_OBDM_OUTPUT_LEN));
                }else {
                    responseCode = DCM_E_REQUESTOUTOFRANGE;/* request for not supported obdmid - (ignore)*/
                }
            }
            else  {
                responseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;/* request for more than one supported OBDMID or mixed request..(ignore) */
            }
        }
        /* @req DCM415 */
        if(responseCode == DCM_E_POSITIVERESPONSE){ /* check request is valid before validating the length */
            if(txLength <= pduTxData->SduLength) { /*if txLength is smaller than the configured length*/
                if(obdmidNum == nofAvailabilityObdmids) { /*check if all the request OBDMIDs are Availabilty OBDMIDs*/
                    for(i = 0;i < obdmidNum;i++) {	/*Check the OBDMID configuration,find which OBDMIDs were configured for 0x00,0x20,0x40 respectively,and fill in the pduTxBuffer,and count the txLength*/
                        if(TRUE == setAvailabilityObdmidValue(pduRxData->SduDataPtr[OBDMID_DATA_START_INDEX + i], &supportBitfield)) {
                            pduTxData->SduDataPtr[txPos++] = pduRxData->SduDataPtr[OBDMID_DATA_START_INDEX + i];
                            /*take every byte of supportBitfield,and fill in txbuffer*/
                            pduTxData->SduDataPtr[txPos++] = (uint8)(((supportBitfield) & (OBD_DATA_LSB_MASK << OFFSET_THREE_BYTES)) >> OFFSET_THREE_BYTES);
                            pduTxData->SduDataPtr[txPos++] = (uint8)(((supportBitfield) & (OBD_DATA_LSB_MASK << OFFSET_TWO_BYTES)) >> OFFSET_TWO_BYTES);
                            pduTxData->SduDataPtr[txPos++] = (uint8)(((supportBitfield) & (OBD_DATA_LSB_MASK << OFFSET_ONE_BYTE)) >> OFFSET_ONE_BYTE);
                            pduTxData->SduDataPtr[txPos++] = (uint8) ((supportBitfield) & (OBD_DATA_LSB_MASK));
                        }
                        else {
                            unFoundObdmid++; /* no range supported (ignore)*/
                        }
                    }
                }
                /* @req DCM416 */
                else {/* Non availabilty OBDMID request*/
                    if(sourceObdmidPtr != NULL_PTR) {
                        for(i = 0; i < sourceObdmidPtr->DspTestResultTidSize; i++){
                            if( E_OK == sourceObdmidPtr->DspTestResultObdmidTidRef[i].DspGetDTRValueFnc(opStatus,&testval,&testMinlimit,&testMaxlimit,&dtrStatus)) {
                                if(DCM_DTRSTATUS_VISIBLE == dtrStatus){
                                    pduTxData->SduDataPtr[txPos++] = pduRxData->SduDataPtr[OBDMID_DATA_START_INDEX];
                                    pduTxData->SduDataPtr[txPos++] = sourceObdmidPtr->DspTestResultObdmidTidRef[i].DspTestResultTestId;
                                    pduTxData->SduDataPtr[txPos++] = sourceObdmidPtr->DspTestResultObdmidTidRef[i].DspTestResultUaSid;
                                    pduTxData->SduDataPtr[txPos++] = (uint8)((testval >> OFFSET_ONE_BYTE) & OBDM_LSB_MASK);
                                    pduTxData->SduDataPtr[txPos++] = (uint8)(testval & OBDM_LSB_MASK);
                                    pduTxData->SduDataPtr[txPos++] = (uint8)((testMinlimit >> OFFSET_ONE_BYTE) & OBDM_LSB_MASK);
                                    pduTxData->SduDataPtr[txPos++] = (uint8)(testMinlimit & OBDM_LSB_MASK);
                                    pduTxData->SduDataPtr[txPos++] = (uint8)((testMaxlimit >> OFFSET_ONE_BYTE) & OBDM_LSB_MASK);
                                    pduTxData->SduDataPtr[txPos++] = (uint8)(testMaxlimit & OBDM_LSB_MASK);
                                } else{            /* else ignore updating the result */
                                    unFoundTid++; /* not visible */
                                }
                            }else{
                                unFoundTid++; /* Wrong response though not intended */
                            }
                        }
                        if(sourceObdmidPtr->DspTestResultTidSize == unFoundTid) { /* Response for the test results from the registered application/s are either seems NOT OK */
                            responseCode = DCM_E_CONDITIONSNOTCORRECT;            /* (or)  DTRStatus seems DCM_DTRSTATUS_INVISIBLE for all the requests */
                        }
                    }
                }
                if(obdmidNum == unFoundObdmid) { /* No obdMid found */
                    responseCode = DCM_E_REQUESTOUTOFRANGE;
                }
            }
            else {
                responseCode = DCM_E_REQUESTOUTOFRANGE; /*if txLength is not smaller than the configured length*/
            }
        }
    }
    else {
        /* Invalid message */
        responseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
    }
    if(DCM_E_POSITIVERESPONSE == responseCode){
        pduTxData->SduLength = txPos;
    }
    DsdDspProcessingDone(responseCode);
    return;
}
#endif

#if defined(USE_DEM) && defined(DCM_USE_SERVICE_REQUESTEMISSIONRELATEDDTCSDETECTEDDURINGCURRENTORLASTCOMPLETEDDRIVINGCYCLE)
/*@req OBD_DCM_REQ_25*//* @req OBD_REQ_12 */
void  DspObdRequestEmissionRelatedDiagnosticTroubleCodesService07(const PduInfoType *pduRxData, PduInfoType *pduTxData)
{
    /* !req DCM410 */
    uint16 messageLen = pduRxData->SduLength;
    Dem_ReturnSetFilterType setDtcFilterResult;
    Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;

    if(messageLen == SID_LEN ) {
        /*"pending" diagnostic trouble codes*/
        /*@req OBD_DCM_REQ_1*/	/*@req OBD_DCM_REQ_26*/
        /* @req DCM412 */
        setDtcFilterResult = Dem_SetDTCFilter(DEM_PENDING_DTC, DEM_DTC_KIND_EMISSION_REL_DTCS, DEM_DTC_FORMAT_OBD,
                                            DEM_DTC_ORIGIN_PRIMARY_MEMORY, DEM_FILTER_WITH_SEVERITY_NO,
                                            VALUE_IS_NOT_USED, DEM_FILTER_FOR_FDC_NO);

        responseCode = OBD_Sevice_03_07_0A(pduTxData,setDtcFilterResult);

    }
    else {
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

    while ((dspVehInfo->DspVehInfoType != InfoType) && ((dspVehInfo->Arc_EOL) == FALSE)) {
        dspVehInfo++;
    }
    if ((dspVehInfo->Arc_EOL) == FALSE) {
        InfoTypeFound = TRUE;
        *InfoTypePtr = dspVehInfo;
    }

    return InfoTypeFound;
}

static boolean setAvailabilityInfoTypeValue(uint8 InfoType, uint32 *DATABUF)
{
    uint8 shift;
    uint32 databuf = 0u;
    uint32 temp;
    boolean setInfoTypeOk = TRUE;
    const Dcm_DspVehInfoType *dspVehInfo = Dcm_ConfigPtr->Dsp->DspVehInfo;

    if(dspVehInfo != NULL_PTR) {
        while ((dspVehInfo->DspVehInfoType != FALSE) &&  ((dspVehInfo->Arc_EOL) == FALSE)) {
            if((dspVehInfo->DspVehInfoType >= (InfoType + AVAIL_TO_SUPPORTED_INFOTYPE_OFFSET_MIN)) &&
                    (dspVehInfo->DspVehInfoType <= (InfoType + AVAIL_TO_SUPPORTED_INFOTYPE_OFFSET_MAX))) {
                shift = dspVehInfo->DspVehInfoType - InfoType;
                temp = (uint32)1 << (AVAIL_TO_SUPPORTED_INFOTYPE_OFFSET_MAX - shift);
                databuf |= temp;
            }
            else if( dspVehInfo->DspVehInfoType > (InfoType + AVAIL_TO_SUPPORTED_INFOTYPE_OFFSET_MAX)) {
                databuf |= (uint32)0x01;
            }
            else {
                /*do nothing*/
            }
            dspVehInfo++;
        }

        if(databuf == 0) {
            setInfoTypeOk = FALSE;
        }
        else {
            /*do nothing*/
        }
    }
    else {
        setInfoTypeOk = FALSE;
    }

    (*DATABUF) = databuf;

    return setInfoTypeOk;

}

/**
 * Function for reading VIT data
 * @param VIT
 * @param dataBuffer
 * @param sourceVehInfoPtr
 * @return E_OK: Read OK, E_NOT_OK: Read failed
 */
static Std_ReturnType readVITData(const uint8 VIT, uint8 *dataBuffer, const Dcm_DspVehInfoType *sourceVehInfoPtr)
{
    Std_ReturnType ret = E_OK;
    uint16 dataPos = 0u;
    const Dcm_DspVehInfoDataType *dataItems = sourceVehInfoPtr->DspVehInfoDataItems;
    for( uint8 dataIdx = 0u; (dataIdx < sourceVehInfoPtr->DspVehInfoNumberOfDataItems) && (E_OK == ret); dataIdx++ ) {
        /* @req DCM423 */
        if (dataItems[dataIdx].DspGetVehInfoTypeFnc(&dataBuffer[dataPos]) != E_OK) {
            if( VIT == 0x02 ) { /* Special for read VIN fail,  customer's requirement*/
                memset(&dataBuffer[dataPos], 0xFF, dataItems[dataIdx].DspVehInfoSize);
            }
            else {
                ret = E_NOT_OK;
            }
        }
        dataPos += dataItems[dataIdx].DspVehInfoSize;
    }
    return ret;
}

/*@req OBD_DCM_REQ_27*//*@req OBD_REQ_13*/
void DspObdRequestVehicleInformation(const PduInfoType *pduRxData, PduInfoType *pduTxData)
{
    /* !req DCM421 */
    uint8 nofAvailabilityInfoTypes = 0u;
    uint16 txPos = SID_LEN;
    uint32 DATABUF;
    uint8 findNum = 0u;
    uint16 InfoTypeNum = pduRxData->SduLength - 1u;
    const Dcm_DspVehInfoType *sourceVehInfoPtr = NULL_PTR;
    Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;

    /*@req OBD_REQ_14*/
    if((pduRxData->SduLength >= OBD_REQ_MESSAGE_LEN_ONE_MIN) && (pduRxData->SduLength <= OBD_REQ_MESSAGE_LEN_MAX )) {
        for( uint16 i = 0u; i < InfoTypeNum; i++ ) {
            if( IS_AVAILABILITY_INFO_TYPE(pduRxData->SduDataPtr[i + 1]) ) {
                nofAvailabilityInfoTypes++;
            }
        }

        /*@req OBD_DCM_REQ_29*/
        if(InfoTypeNum == nofAvailabilityInfoTypes) { /*check if all the request PIDs are the 0x00...0xE0 format*/
            /* @req DCM422 */
            for(uint16 i = 0u; i < InfoTypeNum; i++) { /*Check the PID configuration,find which PIDs were configured for 0x00,0x20,0x40 respectively,and fill in the pduTxBuffer,and count the txLength*/
                if(TRUE == setAvailabilityInfoTypeValue(pduRxData->SduDataPtr[i + 1], &DATABUF)) {
                    pduTxData->SduDataPtr[txPos++] = pduRxData->SduDataPtr[i + 1];
                    /*take every byte of uint32 DTC,and fill in txbuffer*/
                    pduTxData->SduDataPtr[txPos++] = (uint8)((DATABUF & (OBD_DATA_LSB_MASK << OFFSET_THREE_BYTES)) >> OFFSET_THREE_BYTES);
                    pduTxData->SduDataPtr[txPos++] = (uint8)((DATABUF & (OBD_DATA_LSB_MASK << OFFSET_TWO_BYTES)) >> OFFSET_TWO_BYTES);
                    pduTxData->SduDataPtr[txPos++] = (uint8)((DATABUF & (OBD_DATA_LSB_MASK << OFFSET_ONE_BYTE)) >> OFFSET_ONE_BYTE);
                    pduTxData->SduDataPtr[txPos++] = (uint8)(DATABUF & OBD_DATA_LSB_MASK);
                }
                else if(INFOTYPE_ZERO == pduRxData->SduDataPtr[i + 1]) {
                    pduTxData->SduDataPtr[txPos++] = pduRxData->SduDataPtr[i + 1];
                    pduTxData->SduDataPtr[txPos++] = DATAZERO;
                    pduTxData->SduDataPtr[txPos++] = DATAZERO;
                    pduTxData->SduDataPtr[txPos++] = DATAZERO;
                    pduTxData->SduDataPtr[txPos++] = DATAZERO;
                }
                else {
                    findNum++;
                }
            }
        }
        /*@req OBD_DCM_REQ_28*/
        else if(nofAvailabilityInfoTypes == 0) { /*check if all the request PIDs are the supported VINs,like 0x01,0x02...*/
            /*@req OBD_REQ_15*/
            if(pduRxData->SduLength == OBD_REQ_MESSAGE_LEN_ONE_MIN) {
                if(TRUE == lookupInfoType(pduRxData->SduDataPtr[1], &sourceVehInfoPtr )) {
                    if( pduTxData->SduLength >= (3u + sourceVehInfoPtr->DspVehInfoTotalSize) ) {/* 3 = SID */
                        /* Insert Info type in response */
                        pduTxData->SduDataPtr[txPos++] = pduRxData->SduDataPtr[1];
                        /* Insert the number of data items in the response */
                        /*@req OBD_DCM_REQ_30*/
                        pduTxData->SduDataPtr[txPos++] = sourceVehInfoPtr->DspVehInfoNumberOfDataItems;/* @req DCM684 */
                        if( E_OK != readVITData(pduRxData->SduDataPtr[1], &pduTxData->SduDataPtr[txPos], sourceVehInfoPtr) ) {
                            responseCode = DCM_E_CONDITIONSNOTCORRECT;
                        }
                        else {
                            txPos += sourceVehInfoPtr->DspVehInfoTotalSize;
                        }
                    } else {
                        responseCode = DCM_E_REQUESTOUTOFRANGE;
                    }
                }
                else {
                    findNum++;
                }
            }
            /*@req OBD_REQ_16*/
            else {
                responseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
            }
        }
        else {
            responseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
        }

        if(findNum == InfoTypeNum) {
            responseCode = DCM_E_REQUESTOUTOFRANGE;
        }
        else {
            /* do nothing */
        }
    }
    else {
        responseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
    }

    if(DCM_E_POSITIVERESPONSE == responseCode) {
        pduTxData->SduLength = txPos;
    }
    else {
        /* do nothing */
    }

    DsdDspProcessingDone(responseCode);
}
#endif

#if defined(USE_DEM) && defined(DCM_USE_SERVICE_REQUESTEMISSIONRELATEDDTCSWITHPERMANENTSTATUS)
/**
 * Implements OBD service $0A
 * @param pduRxData
 * @param pduTxData
 */
void  DspObdRequestEmissionRelatedDiagnosticTroubleCodesWithPermanentStatus(const PduInfoType *pduRxData, PduInfoType *pduTxData)
{
    /* !req DCM411 */
    uint16 messageLen = pduRxData->SduLength;
    Dem_ReturnSetFilterType setDtcFilterResult;
    Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;

    if(messageLen == SID_LEN ) {
        /*"confirmed" diagnostic trouble codes*/
        /* @req DCM330 */
        setDtcFilterResult = Dem_SetDTCFilter(DEM_DTC_STATUS_MASK_ALL, DEM_DTC_KIND_EMISSION_REL_DTCS, DEM_DTC_FORMAT_OBD,
                                            DEM_DTC_ORIGIN_PERMANENT_MEMORY, DEM_FILTER_WITH_SEVERITY_NO,
                                            VALUE_IS_NOT_USED, DEM_FILTER_FOR_FDC_NO);

        responseCode = OBD_Sevice_03_07_0A(pduTxData,setDtcFilterResult);

    }
    else {
        responseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
    }

    DsdDspProcessingDone(responseCode);

}
#endif



uint32 DspRoutineInfoReadUnsigned(uint8 *data, uint16 bitOffset, uint8 size, boolean changeEndian) {
    uint32 retVal = 0u;
    const uint16 little_endian = 0x1u;
    if(size > 32) {
        DCM_DET_REPORTERROR(DCM_GLOBAL_ID, DCM_E_CONFIG_INVALID);
        return 0;
    }
    if((uint8)changeEndian ^ *((uint8*)&little_endian)) {
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
    uint32 mask = 0xFFFFFFFFul << (size - 1);
    if(retVal & mask) {
        // result is negative
        retVal &= mask;
    }
    return (sint32)retVal;
}

void DspRoutineInfoWrite(uint32 val, uint8 *data, uint16 bitOffset, uint8 size, boolean changeEndian) {
    const uint16 little_endian = 0x1u;
    if((uint8)changeEndian ^ *((uint8*)&little_endian)) {
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

#ifdef DCM_USE_SERVICE_REQUESTDOWNLOAD
void DspUdsRequestDownload(const PduInfoType *pduRxData, PduInfoType *pduTxData)
{
    /* @req DCM496 */
    Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;
    Std_ReturnType ret;
    uint32 memoryAddress = 0u;
    uint32 memorySize = 0u;
    uint32 blockSize = 0u;
    Dcm_OpStatusType opStatus = (Dcm_OpStatusType)DCM_INITIAL;
    SetOpStatusDependingOnState(&dspUdsUploadDownloadPending, &opStatus, FALSE);
    if(pduRxData->SduLength >= 5) {
        uint8 dataFormatIdentifier = pduRxData->SduDataPtr[1];
        uint8 nofSizeBytes = ((uint8)(pduRxData->SduDataPtr[2] & DCM_FORMAT_HIGH_MASK)) >> 4;
        uint8 nofAddrBytes = ((uint8)(pduRxData->SduDataPtr[2])) & DCM_FORMAT_LOW_MASK;
        if( pduRxData->SduLength == (3 + nofSizeBytes + nofAddrBytes) && (0 < nofSizeBytes) && (4 >= nofSizeBytes) &&  (0 < nofAddrBytes) && (4 >= nofAddrBytes)) {

            for(uint8 idx = 0; idx < nofAddrBytes; idx++) {
                memoryAddress <<= 8;
                memoryAddress += (pduRxData->SduDataPtr[3 + idx]);
            }
            for(uint8 idx = 0; idx < nofSizeBytes; idx++) {
                memorySize <<= 8;
                memorySize += (pduRxData->SduDataPtr[3 + nofAddrBytes + idx]);
            }
            if( DCM_INITIAL == opStatus ) {
                ret = Dcm_ProcessRequestDownload(opStatus, dataFormatIdentifier, memoryAddress, memorySize, &blockSize, &responseCode);
            } else {
                ret = Dcm_ProcessRequestDownload(opStatus, 0, 0, 0, &blockSize, &responseCode);
            }

            switch (ret) {
                case E_OK:
                    pduTxData->SduDataPtr[1] = 0x40;
                    pduTxData->SduDataPtr[2] = (uint8)((blockSize>>24) & 0xff);
                    pduTxData->SduDataPtr[3] = (uint8)((blockSize>>16) & 0xff);
                    pduTxData->SduDataPtr[4] = (uint8)((blockSize>>8) & 0xff);
                    pduTxData->SduDataPtr[5] = (uint8)(blockSize & 0xff);
                    pduTxData->SduLength = 6;
                    responseCode = DCM_E_POSITIVERESPONSE;
                    TransferStatus.transferType = DCM_DOWNLOAD;
                    TransferStatus.blockSequenceCounter = 1;
                    TransferStatus.firstBlockReceived = FALSE;
                    TransferStatus.nextAddress = memoryAddress;
                    break;
                case E_PENDING:
                    responseCode = DCM_E_RESPONSEPENDING;
                    break;
                case E_FORCE_RCRRP:
                    responseCode = DCM_E_FORCE_RCRRP;
                    break;
                case E_NOT_OK:
                    /* @req DCM757 */
                    break;
                default:
                    DCM_DET_REPORTERROR(DCM_GLOBAL_ID, DCM_E_UNEXPECTED_RESPONSE);
                    responseCode = DCM_E_REQUESTOUTOFRANGE;
                    break;
            }
        } else {
            responseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
        }
    } else {
        responseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
    }
    if( DCM_E_RESPONSEPENDING == responseCode ) {
        dspUdsUploadDownloadPending.state = DCM_GENERAL_PENDING;
        dspUdsUploadDownloadPending.pduRxData = pduRxData;
        dspUdsUploadDownloadPending.pduTxData = pduTxData;
        dspUdsUploadDownloadPending.pendingService = SID_REQUEST_DOWNLOAD;
    } else if(DCM_E_FORCE_RCRRP == responseCode) {
        dspUdsUploadDownloadPending.state = DCM_GENERAL_FORCE_RCRRP_AWAITING_SEND;
        dspUdsUploadDownloadPending.pduRxData = pduRxData;
        dspUdsUploadDownloadPending.pduTxData = pduTxData;
        dspUdsUploadDownloadPending.pendingService = SID_REQUEST_DOWNLOAD;
        DsdDspForceResponsePending();
    } else {
        dspUdsUploadDownloadPending.state = DCM_GENERAL_IDLE;
        DsdDspProcessingDone(responseCode);
    }
}
#endif

#ifdef DCM_USE_SERVICE_REQUESTUPLOAD
/**
 * @brief The client requests the negotiation of a data transfer from the server to the client.
 *
 * @param[in] pduRxData Received PDU data.
 * @param[out] pduTxData PDU data to be transfered.
 *
 */
void DspUdsRequestUpload(const PduInfoType *pduRxData, PduInfoType *pduTxData)
{
    /* @req DCM499 */
    Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;
    Std_ReturnType ret;
    uint32 memoryAddress = 0u;
    uint32 memorySize = 0u;
    Dcm_OpStatusType opStatus = (Dcm_OpStatusType)DCM_INITIAL;
    uint32 maxNumberOfBlockLength;
    SetOpStatusDependingOnState(&dspUdsUploadDownloadPending, &opStatus, FALSE);

    if(pduRxData->SduLength >= 5) {
        uint8 dataFormatIdentifier = pduRxData->SduDataPtr[1];
        uint8 nofSizeBytes = ((uint8)(pduRxData->SduDataPtr[2] & DCM_FORMAT_HIGH_MASK)) >> 4;
        uint8 nofAddrBytes = ((uint8)(pduRxData->SduDataPtr[2])) & DCM_FORMAT_LOW_MASK;

        if( pduRxData->SduLength == (3 + nofSizeBytes + nofAddrBytes) && (0 < nofSizeBytes) && (4 >= nofSizeBytes) &&  (0 < nofAddrBytes) && (4 >= nofAddrBytes)) {
            for(uint8 idx = 0; idx < nofAddrBytes; idx++) {
                memoryAddress <<= 8;
                memoryAddress += (pduRxData->SduDataPtr[3 + idx]);
            }
            for(uint8 idx = 0; idx < nofSizeBytes; idx++) {
                memorySize <<= 8;
                memorySize += (pduRxData->SduDataPtr[3 + nofAddrBytes + idx]);
                TransferStatus.uplBytesLeft = memorySize;
            }
            if( DCM_INITIAL == opStatus ) {
                ret = Dcm_ProcessRequestUpload(opStatus, dataFormatIdentifier, memoryAddress, memorySize, &responseCode);
            } else {
                ret = Dcm_ProcessRequestUpload(opStatus, 0, 0, 0, &responseCode);
            }
            switch (ret) {
                case E_OK:
                    maxNumberOfBlockLength = (uint32) pduTxData->SduLength;
                    pduTxData->SduDataPtr[1] = 0x40;
                    pduTxData->SduDataPtr[2] = (uint8)((maxNumberOfBlockLength>>24) & 0xff);
                    pduTxData->SduDataPtr[3] = (uint8)((maxNumberOfBlockLength>>16) & 0xff);
                    pduTxData->SduDataPtr[4] = (uint8)((maxNumberOfBlockLength>>8) & 0xff);
                    pduTxData->SduDataPtr[5] = (uint8)(maxNumberOfBlockLength & 0xff);
                    pduTxData->SduLength = 6;
                    responseCode = DCM_E_POSITIVERESPONSE;
                    TransferStatus.transferType = DCM_UPLOAD;
                    TransferStatus.blockSequenceCounter = 1;
                    TransferStatus.firstBlockReceived = FALSE;
                    TransferStatus.nextAddress = memoryAddress;
                    TransferStatus.uplMemBlockSize = maxNumberOfBlockLength -2;
                    break;
                case E_PENDING:
                    responseCode = DCM_E_RESPONSEPENDING;
                    break;
                case E_FORCE_RCRRP:
                    responseCode = DCM_E_FORCE_RCRRP;
                    break;
                case E_NOT_OK:
                    /* @req DCM758 */
                    break;
                default:
                    DCM_DET_REPORTERROR(DCM_GLOBAL_ID, DCM_E_UNEXPECTED_RESPONSE);
                    responseCode = DCM_E_REQUESTOUTOFRANGE;
                    break;
            }
        } else {
           responseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
        }

        if( DCM_E_RESPONSEPENDING == responseCode ) {
           dspUdsUploadDownloadPending.state = DCM_GENERAL_PENDING;
           dspUdsUploadDownloadPending.pduRxData = pduRxData;
           dspUdsUploadDownloadPending.pduTxData = pduTxData;
           dspUdsUploadDownloadPending.pendingService = SID_REQUEST_UPLOAD;
        } else if(DCM_E_FORCE_RCRRP == responseCode) {
           dspUdsUploadDownloadPending.state = DCM_GENERAL_FORCE_RCRRP_AWAITING_SEND;
           dspUdsUploadDownloadPending.pduRxData = pduRxData;
           dspUdsUploadDownloadPending.pduTxData = pduTxData;
           dspUdsUploadDownloadPending.pendingService = SID_REQUEST_UPLOAD;
           DsdDspForceResponsePending();
        } else {
           dspUdsUploadDownloadPending.state = DCM_GENERAL_IDLE;
           DsdDspProcessingDone(responseCode);
        }
    }
}
#endif

#ifdef DCM_USE_SERVICE_TRANSFERDATA


void DspUdsTransferData(const PduInfoType *pduRxData, PduInfoType *pduTxData)
{
    /* @req DCM502 */
    uint16 respParamRecLen;
    Dcm_OpStatusType opStatus = (Dcm_OpStatusType)DCM_INITIAL;
    SetOpStatusDependingOnState(&dspUdsUploadDownloadPending, &opStatus, FALSE);
    Dcm_ReturnReadMemoryType readRet;

    Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;
    if(DCM_NO_DATA_TRANSFER != TransferStatus.transferType) {
        if((pduRxData->SduLength >= 2 && DCM_UPLOAD == TransferStatus.transferType) ||
           (pduRxData->SduLength >= 3 && DCM_DOWNLOAD == TransferStatus.transferType) ) {
            respParamRecLen = pduTxData->SduLength - 2;
            if( (pduRxData->SduDataPtr[1] == TransferStatus.blockSequenceCounter)) {
                if( DCM_DOWNLOAD == TransferStatus.transferType ) {
                    Dcm_ReturnWriteMemoryType writeRet;
                    /* @req DCM503 */
                    if( DCM_INITIAL == opStatus ) {
                        writeRet = Dcm_WriteMemory(opStatus, 0, TransferStatus.nextAddress, pduRxData->SduLength - 2, &pduRxData->SduDataPtr[2]);
                    } else {
                        writeRet = Dcm_WriteMemory(opStatus, 0, 0, 0, 0);
                    }
                    switch(writeRet) {
                        case DCM_WRITE_OK:
                            TransferStatus.blockSequenceCounter++;
                            TransferStatus.firstBlockReceived = TRUE;
                            TransferStatus.nextAddress += pduRxData->SduLength - 2;
                            pduTxData->SduDataPtr[1] = pduRxData->SduDataPtr[1];
                            Dcm_Arc_GetDownloadResponseParameterRecord(&pduTxData->SduDataPtr[2], &respParamRecLen);
                            pduTxData->SduLength = 2 + respParamRecLen;
                            break;
                        case DCM_WRITE_FORCE_RCRRP:
                            responseCode = DCM_E_FORCE_RCRRP;
                            break;
                        case DCM_WRITE_PENDING:
                            responseCode = DCM_E_RESPONSEPENDING;
                            break;
                        default:
                            responseCode = DCM_E_GENERALPROGRAMMINGFAILURE;/* @req DCM643 */
                            break;
                    }
                }
                else if ( DCM_UPLOAD == TransferStatus.transferType ) {
                    uint32 readMemSize = 0;
                    /* @req DCM504 */
                    if( DCM_INITIAL == opStatus ) {
                        readMemSize = MIN((TransferStatus.uplMemBlockSize), (TransferStatus.uplBytesLeft));
                        readRet = Dcm_ReadMemory(opStatus, 0, TransferStatus.nextAddress, readMemSize, &pduTxData->SduDataPtr[2]);
                    } else {
                        readRet = Dcm_ReadMemory(opStatus, 0, 0, 0, 0);
                    }
                    switch(readRet) {
                        case DCM_READ_OK:
                            TransferStatus.firstBlockReceived = TRUE; /* I know, it should be block send, but we reuse the same control flag */
                            TransferStatus.nextAddress += readMemSize;
                            TransferStatus.uplBytesLeft -= readMemSize;
                            respParamRecLen = readMemSize;
                            pduTxData->SduDataPtr[1] = TransferStatus.blockSequenceCounter;
                            TransferStatus.blockSequenceCounter++;
                            pduTxData->SduLength = 2 + respParamRecLen;
                            break;
                        case DCM_READ_PENDING:
                            responseCode = DCM_E_RESPONSEPENDING;
                            break;
                        default:
                            responseCode = DCM_E_GENERALREJECT;  /* @req DCM644 */
                            break;
                        }

                }
                else {
                    responseCode = DCM_E_REQUESTOUTOFRANGE;
                }
            } else {
                uint8 okCounter;
                if(0 == TransferStatus.blockSequenceCounter) {
                    okCounter = 0xFF;
                } else {
                    okCounter = TransferStatus.blockSequenceCounter - 1;
                }
                /* Allow the same sequence counter again (but do nothing) as long as we have received the first block. */
                /* @req DCM645 */
                if ( (FALSE == TransferStatus.firstBlockReceived) || (pduRxData->SduDataPtr[1] != okCounter) ) {
                    responseCode = DCM_E_WRONGBLOCKSEQUENCECOUNTER;
                } else {
                    pduTxData->SduDataPtr[1] = pduRxData->SduDataPtr[1];
                    if( DCM_DOWNLOAD == TransferStatus.transferType ) {
                        Dcm_Arc_GetDownloadResponseParameterRecord(&pduTxData->SduDataPtr[2], &respParamRecLen);
                    } else if ( DCM_UPLOAD == TransferStatus.transferType ) {
                        readRet = Dcm_ReadMemory(opStatus, 0, TransferStatus.nextAddress, MIN((TransferStatus.uplMemBlockSize), (TransferStatus.uplBytesLeft)), &pduTxData->SduDataPtr[2]);

                        switch(readRet) {
                            case DCM_READ_OK:
                                break;
                            case DCM_READ_PENDING:
                                responseCode = DCM_E_RESPONSEPENDING;
                                break;
                            default:
                                responseCode = DCM_E_GENERALREJECT;  /* @req DCM644 */
                                break;
                        }
                    }
                    pduTxData->SduLength = 2 + respParamRecLen;
                }
            }
        } else {
            responseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
        }
    } else {
        responseCode = DCM_E_REQUESTSEQUENCEERROR;
    }

    if( DCM_E_RESPONSEPENDING == responseCode ) {
        dspUdsUploadDownloadPending.state = DCM_GENERAL_PENDING;
        dspUdsUploadDownloadPending.pduRxData = pduRxData;
        dspUdsUploadDownloadPending.pduTxData = pduTxData;
        dspUdsUploadDownloadPending.pendingService = SID_TRANSFER_DATA;
    }
    else if(DCM_E_FORCE_RCRRP == responseCode)
    {
        dspUdsUploadDownloadPending.state = DCM_GENERAL_FORCE_RCRRP_AWAITING_SEND;
        dspUdsUploadDownloadPending.pduRxData = pduRxData;
        dspUdsUploadDownloadPending.pduTxData = pduTxData;
        dspUdsUploadDownloadPending.pendingService = SID_TRANSFER_DATA;
        DsdDspForceResponsePending();
    }
    else {
        dspUdsUploadDownloadPending.state = DCM_GENERAL_IDLE;

        DsdDspProcessingDone(responseCode);
    }
}
#endif

#ifdef DCM_USE_SERVICE_REQUESTTRANSFEREXIT
void DspUdsRequestTransferExit(const PduInfoType *pduRxData, PduInfoType *pduTxData)
{
    /* @req DCM505 */
    Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;
    Std_ReturnType ret;
    Dcm_OpStatusType opStatus = (Dcm_OpStatusType)DCM_INITIAL;
    SetOpStatusDependingOnState(&dspUdsUploadDownloadPending, &opStatus, FALSE);
    if(pduRxData->SduLength >= 1) {
        if( DCM_NO_DATA_TRANSFER != TransferStatus.transferType ) {
            if( DCM_INITIAL == opStatus ) {
                ret = Dcm_ProcessRequestTransferExit(DCM_INITIAL, &pduRxData->SduDataPtr[1], pduRxData->SduLength - 1, &responseCode);
            } else {
                ret = Dcm_ProcessRequestTransferExit(opStatus, 0, 0, &responseCode);
            }

            switch (ret) {
                case E_OK:
                    TransferStatus.transferType = DCM_NO_DATA_TRANSFER;
                    uint16 respParamRecLen = pduTxData->SduLength;
                    Dcm_Arc_GetTransferExitResponseParameterRecord(&pduTxData->SduDataPtr[1], &respParamRecLen);
                    pduTxData->SduLength = 1 + respParamRecLen;
                    responseCode = DCM_E_POSITIVERESPONSE;
                    break;
                case E_PENDING:
                    responseCode = DCM_E_RESPONSEPENDING;
                    break;
                case E_FORCE_RCRRP:
                    responseCode = DCM_E_FORCE_RCRRP;
                    break;
                case E_NOT_OK:
                    /* @req DCM759 */
                    break;
                default:
                    DCM_DET_REPORTERROR(DCM_GLOBAL_ID, DCM_E_UNEXPECTED_RESPONSE);
                    responseCode = DCM_E_REQUESTOUTOFRANGE;
                    break;
            }
        } else {
            responseCode = DCM_E_REQUESTSEQUENCEERROR;
        }
    } else {
        responseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
    }
    if( DCM_E_RESPONSEPENDING == responseCode ) {
        dspUdsUploadDownloadPending.state = DCM_GENERAL_PENDING;
        dspUdsUploadDownloadPending.pduRxData = pduRxData;
        dspUdsUploadDownloadPending.pduTxData = pduTxData;
        dspUdsUploadDownloadPending.pendingService = SID_REQUEST_TRANSFER_EXIT;
    } else if(DCM_E_FORCE_RCRRP == responseCode) {
        dspUdsUploadDownloadPending.state = DCM_GENERAL_FORCE_RCRRP_AWAITING_SEND;
        dspUdsUploadDownloadPending.pduRxData = pduRxData;
        dspUdsUploadDownloadPending.pduTxData = pduTxData;
        dspUdsUploadDownloadPending.pendingService = SID_REQUEST_TRANSFER_EXIT;
        DsdDspForceResponsePending();
    } else {
        dspUdsUploadDownloadPending.state = DCM_GENERAL_IDLE;
        DsdDspProcessingDone(responseCode);
    }
}
#endif
