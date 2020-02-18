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

#ifndef DCM_LCFG_H_
#define DCM_LCFG_H_

/*
 *  General requirements
 */
/** @req 3.1.5/DCM075 */ /** @req 3.1.5/DCM076 */


#include "ComStack_Types.h"
#include "Dcm_Types.h"
#if defined(USE_NVM)
#include "NvM.h"
#endif
typedef uint8 Dcm_ProtocolTransTypeType;
#define DCM_PROTOCOL_TRANS_TYPE_1		1u
#define DCM_PROTOCOL_TRANS_TYPE_2		2u

typedef uint8 Dcm_ProtocolAddrTypeType;
#define DCM_PROTOCOL_FUNCTIONAL_ADDR_TYPE	1u
#define DCM_PROTOCOL_PHYSICAL_ADDR_TYPE		2u

#define DCM_PROTOCAL_TP_MAX_LENGTH 0x1000u
#define DCM_INVALID_PDU_ID 0xFFFFu

/*
 * Callback function prototypes
 */

// SessionControl
typedef Std_ReturnType (*Dcm_CallbackGetSesChgPermissionFncType)(Dcm_SesCtrlType sesCtrlTypeActive, Dcm_SesCtrlType sesCtrlTypeNew);
typedef Std_ReturnType (*Dcm_CallbackChangeIndicationFncType)(Dcm_SesCtrlType sesCtrlTypeOld, Dcm_SesCtrlType sesCtrlTypeNew);
typedef Std_ReturnType (*Dcm_CallbackConfirmationRespPendFncType)(Dcm_ConfirmationStatusType status);

// SecurityAccess_<LEVEL>
typedef Std_ReturnType (*Dcm_CallbackGetSeedFncTypeWithRecord)(const uint8 *securityAccessDataRecord, Dcm_OpStatusType OpStatus, uint8 *seed, Dcm_NegativeResponseCodeType *errorCode);
typedef Std_ReturnType (*Dcm_CallbackGetSeedFncTypeWithoutRecord)(Dcm_OpStatusType OpStatus, uint8 *seed, Dcm_NegativeResponseCodeType *errorCode);
typedef union {
    Dcm_CallbackGetSeedFncTypeWithRecord getSeedWithRecord;
    Dcm_CallbackGetSeedFncTypeWithoutRecord getSeedWithoutRecord;
}Dcm_CallbackGetSeedFncType;
typedef Std_ReturnType (*Dcm_CallbackCompareKeyFncType)(const uint8 *key, Dcm_OpStatusType OpStatus);

// PidServices_<PID>
typedef Std_ReturnType (*Dcm_CallbackGetPIDValueFncType)(uint8 *dataValueBuffer);

// DidServices_<DID>
typedef Std_ReturnType (*Dcm_SynchCallbackReadDataFncType)(uint8 *data);/* @req DCM793 */
typedef Std_ReturnType (*Dcm_AsynchCallbackReadDataFncType)(Dcm_OpStatusType OpStatus, uint8 *data);
typedef Std_ReturnType (*Dcm_FixLenCallbackWriteDataFncType)(const uint8 *data, Dcm_OpStatusType opStatus, Dcm_NegativeResponseCodeType *errorCode);/* @req DCM794 */
typedef Std_ReturnType (*Dcm_DynLenCallbackWriteDataFncType)(const uint8 *data, uint16 dataLength, Dcm_OpStatusType opStatus, Dcm_NegativeResponseCodeType *errorCode);
typedef Std_ReturnType (*Dcm_CallbackReadDataLengthFncType)(uint16 *didLength);/* @req DCM796 */
typedef Std_ReturnType (*Dcm_CallbackConditionCheckReadFncType)(Dcm_OpStatusType OpStatus, Dcm_NegativeResponseCodeType *errorCode);/* @req DCM797 */
typedef Std_ReturnType (*Dcm_FuncCallbackReturnControlToECUFncType)(Dcm_OpStatusType OpStatus, Dcm_NegativeResponseCodeType *errorCode);/* @req DCM799 */
typedef Std_ReturnType (*Dcm_FuncCallbackResetToDefaultFncType)(Dcm_OpStatusType OpStatus, Dcm_NegativeResponseCodeType *errorCode);/* @req DCM800 */
typedef Std_ReturnType (*Dcm_FuncCallbackFreezeCurrentStateFncType)(Dcm_OpStatusType OpStatus, Dcm_NegativeResponseCodeType *errorCode);/* @req DCM801 */
typedef Std_ReturnType (*Dcm_FuncCallbackShortTermAdjustmentFncType)(const uint8 *controlOptionRecord, Dcm_OpStatusType OpStatus, Dcm_NegativeResponseCodeType *errorCode);/* @req DCM802 */

typedef Std_ReturnType (*Dcm_EcuSignalCallbackReturnControlToECUFncType)(uint8 action, uint8 *controlOptionRecord);
typedef Std_ReturnType (*Dcm_EcuSignalCallbackResetToDefaultFncType)(uint8 action, uint8 *controlOptionRecord);
typedef Std_ReturnType (*Dcm_EcuSignalCallbackFreezeCurrentStateFncType)(uint8 action, uint8 *controlOptionRecord);
typedef Std_ReturnType (*Dcm_EcuSignalCallbackShortTermAdjustmentFncType)(uint8 action, uint8 *controlOptionRecord);

typedef Std_ReturnType (*Dcm_SRCallbackReadDataFncType)(void *data);
typedef Std_ReturnType (*Dcm_SRCallbackWrite_UINT8_DataFncType)(uint8 data);
typedef Std_ReturnType (*Dcm_SRCallbackWrite_SINT8_DataFncType)(sint8 data);
typedef Std_ReturnType (*Dcm_SRCallbackWrite_UINT16_DataFncType)(uint16 data);
typedef Std_ReturnType (*Dcm_SRCallbackWrite_SINT16_DataFncType)(sint16 data);
typedef Std_ReturnType (*Dcm_SRCallbackWrite_UINT32_DataFncType)(uint32 data);
typedef Std_ReturnType (*Dcm_SRCallbackWrite_SINT32_DataFncType)(sint32 data);
typedef Std_ReturnType (*Dcm_SRCallbackWrite_BOOLEAN_DataFncType)(boolean data);

typedef Std_ReturnType (*Dcm_RoeActivateCallbackFncType)(uint8 RoeEventId, Dcm_RoeStateType state);


typedef Std_ReturnType (*Dcm_CallbackGetScalingInformationFncType)(Dcm_OpStatusType OpStatus, uint8 *scalingInfo, Dcm_NegativeResponseCodeType *errorCode);/* @req DCM798 */

// InfoTypeServices_<INFOTYPENUMBER>
typedef Std_ReturnType (*Dcm_CallbackGetInfoTypeValueFncType)(uint8 *dataValueBuffer);

// DTRServices
typedef Std_ReturnType (*Dcm_CallbackGetDTRValueFncType)(Dcm_OpStatusType OpStatus,uint16 *Testval, uint16 *Minlimit, uint16 *Maxlimit, uint8* Status);

// RoutineServices_<ROUTINENAME>
typedef Std_ReturnType (*Dcm_CallbackStartRoutineFncType)(uint8 *indata, Dcm_OpStatusType OpStatus, uint8 *outdata,
            uint16* currentDataLength, Dcm_NegativeResponseCodeType *errorCode, boolean changeEndianess);
typedef Std_ReturnType (*Dcm_CallbackStopRoutineFncType)(uint8 *indata, Dcm_OpStatusType OpStatus, uint8 *outdata,
            uint16* currentDataLength, Dcm_NegativeResponseCodeType *errorCode, boolean changeEndianess);
typedef Std_ReturnType (*Dcm_CallbackRequestResultRoutineFncType)(Dcm_OpStatusType OpStatus, uint8 *outdata,
            uint16* currentDataLength, Dcm_NegativeResponseCodeType *errorCode, boolean changeEndianess);

// RequestControlServices_<TID>
typedef Std_ReturnType (*Dcm_CallbackRequestControlType)(uint8 *outBuffer, uint8 *inBuffer);

// CallBackDCMRequestServices
typedef Std_ReturnType (*Dcm_CallbackStartProtocolFncType)(Dcm_ProtocolType protocolID);
typedef Std_ReturnType (*Dcm_CallbackStopProtocolFncType)(Dcm_ProtocolType protocolID);

// ServiceRequestIndication
typedef Std_ReturnType (*Dcm_CallbackNotificationIndicationFncType)(uint8 SID, const uint8* requestData, uint16 dataSize, uint8 reqType, uint16 soruceAddress, Dcm_NegativeResponseCodeType* ErrorCode);
typedef Std_ReturnType (*Dcm_CallbackNotificationConfirmationFncType)(uint8 SID, uint8 reqType, uint16 sourceAddress, Dcm_ConfirmationStatusType status);

// ResetService
typedef Std_ReturnType (*Dcm_CallbackEcuResetType)(uint8 resetType,	Dcm_NegativeResponseCodeType *errorCode);

//OBD service 0x04 condition check callback
typedef Std_ReturnType (*Dcm_DsdConditionGetFncType)(void);
typedef Std_ReturnType (*Dcm_DsdResetPidsFncType)(void);

typedef Std_ReturnType (*Dcm_DsdDspSidTabFncType)(Dcm_OpStatusType OpStatus, Dcm_MsgContextType* pMsgContext);/* @req DCM763 */

typedef Std_ReturnType (*Dcm_ComControlModeSwitchFcnType)(uint8 mode);

typedef enum {
    DATA_PORT_NO_PORT,
    DATA_PORT_BLOCK_ID,
    DATA_PORT_ASYNCH,
    DATA_PORT_SYNCH,
    DATA_PORT_ECU_SIGNAL,
    DATA_PORT_SR
}Dcm_DataPortType;

typedef union {
    Dcm_SynchCallbackReadDataFncType  SynchDataReadFnc;    // (0..1)
    Dcm_AsynchCallbackReadDataFncType AsynchDataReadFnc;    // (0..1)
    Dcm_SRCallbackReadDataFncType   SRDataReadFnc;
} Dcm_CallbackReadDataFncType;
typedef union {
    Dcm_FixLenCallbackWriteDataFncType  FixLenDataWriteFnc;    // (0..1)
    Dcm_DynLenCallbackWriteDataFncType DynLenDataWriteFnc;    // (0..1)
    Dcm_SRCallbackWrite_UINT8_DataFncType SRDataWriteFnc_UINT8;
    Dcm_SRCallbackWrite_SINT8_DataFncType SRDataWriteFnc_SINT8;
    Dcm_SRCallbackWrite_UINT16_DataFncType SRDataWriteFnc_UINT16;
    Dcm_SRCallbackWrite_SINT16_DataFncType SRDataWriteFnc_SINT16;
    Dcm_SRCallbackWrite_UINT32_DataFncType SRDataWriteFnc_UINT32;
    Dcm_SRCallbackWrite_SINT32_DataFncType SRDataWriteFnc_SINT32;
    Dcm_SRCallbackWrite_BOOLEAN_DataFncType SRDataWriteFnc_BOOLEAN;

} Dcm_CallbackWriteDataFncType;
typedef union {
    Dcm_FuncCallbackResetToDefaultFncType  FuncResetToDefaultFnc;    // (0..1)
    Dcm_EcuSignalCallbackResetToDefaultFncType EcuSignalResetToDefaultFnc;    // (0..1)
} Dcm_CallbackResetToDefaultFncType;
typedef union {
    Dcm_FuncCallbackReturnControlToECUFncType  FuncReturnControlToECUFnc;    // (0..1)
    Dcm_EcuSignalCallbackReturnControlToECUFncType EcuSignalReturnControlToECUFnc;    // (0..1)
} Dcm_CallbackReturnControlToECUFncType;
typedef union {
    Dcm_FuncCallbackShortTermAdjustmentFncType  FuncShortTermAdjustmentFnc;    // (0..1)
    Dcm_EcuSignalCallbackShortTermAdjustmentFncType EcuSignalShortTermAdjustmentFnc;    // (0..1)
} Dcm_CallbackShortTermAdjustmentFncType;
typedef union {
    Dcm_FuncCallbackFreezeCurrentStateFncType  FuncFreezeCurrentStateFnc;    // (0..1)
    Dcm_EcuSignalCallbackFreezeCurrentStateFncType EcuSignalFreezeCurrentStateFnc;    // (0..1)
} Dcm_CallbackFreezeCurrentStateFncType;



/*
 * DCM configurations
 */

/*******
 * DSP *
 *******/
typedef enum {
    DCM_NO_BOOT,
    DCM_OEM_BOOT,
    DCM_SYS_BOOT,
    DCM_OEM_BOOT_RESPAPP,
    DCM_SYS_BOOT_RESPAPP
}Dcm_DspSessionForBootType;
// 10.2.44
typedef struct {
    uint32                      DspSessionP2ServerMax;      // (1)
    uint32                      DspSessionP2StarServerMax;  // (1)
    Dcm_SesCtrlType             DspSessionLevel;            // (1)
    Dcm_SesCtrlType             ArcDspRteSessionLevelName;
    Dcm_DspSessionForBootType   DspSessionForBoot;
    boolean			            Arc_EOL;
} Dcm_DspSessionRowType;

// 10.2.42
typedef struct {
    uint32							DspSecurityADRSize;			// (0..1)
    uint32							DspSecurityDelayTimeOnBoot;	// (1)
    uint32							DspSecurityDelayTime;		// (1)
    Dcm_SecLevelType				DspSecurityLevel;			// (1)
    uint8							DspSecurityNumAttDelay;		// (1)
    uint8							DspSecurityNumAttLock;		// (1)
    uint8							DspSecuritySeedSize;		// (1)
    uint8							DspSecurityKeySize;			// (1)
    Dcm_CallbackGetSeedFncType		GetSeed;
    Dcm_CallbackCompareKeyFncType	CompareKey;
    boolean							Arc_EOL;
} Dcm_DspSecurityRowType;


// 10.2.26
typedef struct {
    const Dcm_DspSessionRowType				* const *DspDidControlSessionRef;		// (1..*)
    const Dcm_DspSecurityRowType			* const *DspDidControlSecurityLevelRef;	// (1..*)
} Dcm_DspDidControlType;

// 10.2.27
typedef struct {
    const Dcm_DspSessionRowType		* const *DspDidReadSessionRef;			// (1..*)
    const Dcm_DspSecurityRowType	* const *DspDidReadSecurityLevelRef;	// (1..*)
} Dcm_DspDidReadType;

// 10.2.28
typedef struct {
    const Dcm_DspSessionRowType		* const *DspDidWriteSessionRef;		// (1..*)
    const Dcm_DspSecurityRowType	* const *DspDidWriteSecurityLevelRef;	// (1..*)
} Dcm_DspDidWriteType; /** @req DCM616 */

// 10.2.25
typedef struct {
    // Containers
    const Dcm_DspDidReadType		*DspDidRead;	// (0..1)
    const Dcm_DspDidWriteType		*DspDidWrite;	// (0..1)
    const Dcm_DspDidControlType		*DspDidControl;	// (0..1)
} Dcm_DspDidAccessType;

// 10.2.24
typedef struct {
    boolean					DspDidDynamicllyDefined;	// (1)
    // Containers
    Dcm_DspDidAccessType	DspDidAccess;				// (1)
} Dcm_DspDidInfoType;

typedef struct {
    boolean                 DspDidFixedLength;          // (1)
    uint8                   DspDidScalingInfoSize;      // (0..1)
}Dcm_DspDataInfoType;

typedef enum {
    DCM_BIG_ENDIAN,
    DCM_LITTLE_ENDIAN,
    DCM_ENDIAN_NOT_USED
}DcmDspDataEndianessType;

typedef enum {
    DCM_BOOLEAN,
    DCM_UINT8,
    DCM_UINT16,
    DCM_UINT32,
    DCM_SINT8,
    DCM_SINT16,
    DCM_SINT32,
    DCM_UINT8_N
}DcmDspDataType;

typedef struct {
    const Dcm_DspDataInfoType                   *DspDataInfoRef;
    Dcm_CallbackReadDataLengthFncType           DspDataReadDataLengthFnc;        // (0..1)
    Dcm_CallbackConditionCheckReadFncType       DspDataConditionCheckReadFnc;    // (0..1)
    Dcm_CallbackReadDataFncType                 DspDataReadDataFnc;              // (0..1)
    Dcm_CallbackWriteDataFncType                DspDataWriteDataFnc;             // (0..1)
    Dcm_CallbackGetScalingInformationFncType    DspDataGetScalingInfoFnc;        // (0..1)
    Dcm_CallbackFreezeCurrentStateFncType       DspDataFreezeCurrentStateFnc;    // (0..1)
    Dcm_CallbackResetToDefaultFncType           DspDataResetToDefaultFnc;        // (0..1)
    Dcm_CallbackReturnControlToECUFncType       DspDataReturnControlToEcuFnc;    // (0..1)
    Dcm_CallbackShortTermAdjustmentFncType      DspDataShortTermAdjustmentFnc;   // (0..1)
    Dcm_DataPortType                            DspDataUsePort;                  // (1)
    uint16                                      DspDataBitSize;
#if defined(USE_NVM)
    NvM_BlockIdType DspNvmUseBlockID;
#endif
    DcmDspDataType DspDataType;
    DcmDspDataEndianessType DspDataEndianess;
}Dcm_DspDataType;

typedef struct {
    const Dcm_DspDataType *DspSignalDataRef;
    const uint16 DspSignalBitPosition;
}Dcm_DspSignalType;

// 10.2.22
typedef struct Dcm_DspDidType {
    const Dcm_DspDidInfoType					*DspDidInfoRef;					// (1)
    const struct Dcm_DspDidType					* const *DspDidRef;				// (0..*)
    const Dcm_DspSignalType                     * const DspSignalRef;
    uint16                                      DspDidIdentifier;               // (1)
    uint16                                      DspNofSignals;
    uint16                                      DspDidDataByteSize;
    uint16                                      DspDidDataScalingInfoSize;
    Dcm_RoeActivateCallbackFncType              DspDidRoeActivateFnc;           // (0..1)
    uint8                                       DspDidRoeEventId;               // (0..1)
    // Containers
    boolean										Arc_EOL;
} Dcm_DspDidType;

// 10.2.30
typedef struct {
    const Dcm_DspSessionRowType		* const *DspEcuResetSessionRef;			// (1..*)
    const Dcm_DspSecurityRowType	* const *DspEcuResetSecurityLevelRef;	// (1..*)
} Dcm_DspEcuResetType;

// 10.2.31
typedef struct {
    boolean							DspPidUsePort;		// (1)
    uint8							DspPidIdentifier;	// (1)
    uint8							DspPidSize; 		// (1)
    Dcm_PidServiceType				DspPidService;
    Dcm_CallbackGetPIDValueFncType	DspGetPidValFnc;	// (1)
    const boolean                   *Arc_DcmPidEnabled; // (0..1)
    boolean							Arc_EOL;
} Dcm_DspPidType;

// 10.2.33
typedef struct {
    boolean							DspDTCInfoSubFuncSupp;		// (1)
    uint8							DspDTCInfoSubFuncLevel;		// (1)
    const Dcm_DspSecurityRowType	* const *DspDTCInfoSecLevelRef;	// (1..*)
} Dcm_DspReadDTCRowType;

// 10.2.32
typedef struct {
    // Containers
    Dcm_DspReadDTCRowType *DspReadDTCRow; // (0..*)
} Dcm_DspReadDTCType;

// 10.2.34
typedef struct {
    uint8 DspRequestControl;				// (1)
    uint8 DspRequestControlOutBufferSize;	// (1)
    uint8 DspRequestControlTestId;			// (1)
} Dcm_DspRequestControlType;

// 10.2.37
typedef struct {
    const Dcm_DspSessionRowType		* const *DspRoutineSessionRef;			// (1..*)
    const Dcm_DspSecurityRowType	* const *DspRoutineSecurityLevelRef;	// (1..*)
} Dcm_DspRoutineAuthorizationType; /** @req DCM644 */

// 10.2.38
typedef struct {
    uint8 DspReqResRtnCtrlOptRecSize; // (1)
} Dcm_DspRoutineRequestResType;

// 10.2.39
typedef struct {
    uint8 DspStopRoutineCtrlOptRecSize;	// (1)
    uint8 DspStopRoutineStsOptRecSize;	// (1)
} Dcm_DspRoutineStopType;

// 10.2.40
typedef struct {
    uint32 DspStartRoutineCtrlOptRecSize;	// (1)
    uint32 DspStartRoutineStsOptRecSize;		// (1)
} Dcm_DspStartRoutineType;

// 10.2.36
typedef struct {
    // Containers
    const Dcm_DspRoutineAuthorizationType	DspRoutineAuthorization;	// (1)
    const Dcm_DspStartRoutineType			*DspStartRoutine;			// (1)
    const Dcm_DspRoutineStopType			*DspRoutineStop;			// (0..1)
    const Dcm_DspRoutineRequestResType		*DspRoutineRequestRes;		// (0..1)
} Dcm_DspRoutineInfoType;

// 10.2.35
typedef struct {
    boolean									DspRoutineUsePort;			// (1)
    uint16									DspRoutineIdentifier;		// (1)
    const Dcm_DspRoutineInfoType			*DspRoutineInfoRef;			// (1)
    Dcm_CallbackStartRoutineFncType			DspStartRoutineFnc;			// (0..1)
    Dcm_CallbackStopRoutineFncType			DspStopRoutineFnc;			// (0..1)
    Dcm_CallbackRequestResultRoutineFncType	DspRequestResultRoutineFnc;	// (0..1)
    boolean									Arc_EOL;
} Dcm_DspRoutineType;

// 10.2.41
typedef struct {
    // Containers
    const Dcm_DspSecurityRowType *DspSecurityRow; // (0..31)
} Dcm_DspSecurityType;

// 10.2.43
typedef struct {
    // Containers
    const Dcm_DspSessionRowType *DspSessionRow; // (0..31)
} Dcm_DspSessionType;

typedef struct {
    uint8                            DspTestResultUaSid;	        // (1)
    uint8                            DspTestResultTestId;	        // (1)
    /* Defined internally */
    Dcm_CallbackGetDTRValueFncType   DspGetDTRValueFnc;	            // (1)
} Dcm_DspTestResultObdmidTidType;

typedef struct {
    uint8							 DspTestResultObdmid;			// (1)
    uint8                            DspTestResultTidSize;          // (1)
    const Dcm_DspTestResultObdmidTidType   *DspTestResultObdmidTidRef;    // (1..*)
    boolean Arc_EOL;
} Dcm_DspTestResultObdmidType;

typedef struct {
    // Containers
    const Dcm_DspTestResultObdmidType	*DspTestResultObdmidTid;	// (1..*)
} Dcm_DspTestResultByObdmidType;

typedef struct {
    Dcm_CallbackGetInfoTypeValueFncType DspGetVehInfoTypeFnc;   // (1)
    uint8                               DspVehInfoSize;         // (1)
}Dcm_DspVehInfoDataType;

// 10.2.48
typedef struct {
    const Dcm_DspVehInfoDataType        *DspVehInfoDataItems;
    uint16                              DspVehInfoTotalSize;
    uint8								DspVehInfoType;			// (1)
    uint8								DspVehInfoNumberOfDataItems;
    boolean Arc_EOL;
} Dcm_DspVehInfoType;

// 10.2.21
typedef struct {
    uint32 MemoryAddressHigh;
    uint32 MemoryAddressLow;
    /*DcmDspMemoryRangeRuleRef * pRule;*/
    const Dcm_DspSecurityRowType * const *pSecurityLevel;
    boolean Arc_EOL;
} Dcm_DspMemoryRangeInfo;

typedef struct {
      uint8 MemoryIdValue;
     const Dcm_DspMemoryRangeInfo *pReadMemoryInfo;
     const Dcm_DspMemoryRangeInfo *pWriteMemoryInfo;
     boolean										Arc_EOL;
} Dcm_DspMemoryIdInfo;


typedef struct {
    boolean								DcmDspUseMemoryId;
    const Dcm_DspMemoryIdInfo			*DspMemoryIdInfo;
    
}Dcm_DspMemoryType;

typedef struct {
    uint8                           ComMChannelIndex;
    boolean                         Arc_EOL;
}Dcm_DspComControlAllChannelType;

typedef struct {
    uint8                           ComMChannelIndex;
    uint8                           SubnetNumber;
    boolean                         Arc_EOL;
}Dcm_DspComControlSpecificChannelType;

typedef struct {
   const Dcm_DspComControlAllChannelType        *DspControlAllChannel;
   const Dcm_DspComControlSpecificChannelType   *DspControlSpecificChannel;
}Dcm_DspComControlType;

// 10.2.21
typedef struct {
    uint8								DspMaxDidToRead; // (0..1)
    // Containers
    const Dcm_DspDidType				*DspDid;	// (0..*)
    const Dcm_DspDidInfoType			*DspDidInfo;			// (0..*)
    const Dcm_DspEcuResetType			*DspEcuReset;			// (0..*)
    const Dcm_DspPidType				*DspPid;				// (0..*)
    const Dcm_DspReadDTCType			*DspReadDTC;			// (1)
    const Dcm_DspRequestControlType		*DspRequestControl;		// (0..*)
    const Dcm_DspRoutineType			*DspRoutine;			// (0..*)
    const Dcm_DspRoutineInfoType		*DspRoutineInfo;		// (0..*)
    const Dcm_DspSecurityType			*DspSecurity;			// (0..*)
    const Dcm_DspSessionType			*DspSession;			// (1)
    const Dcm_DspTestResultByObdmidType	*DspTestResultByObdmid;	// (1)
    const Dcm_DspVehInfoType			*DspVehInfo;
    const Dcm_DspMemoryType             *DspMemory;
    const Dcm_DspComControlType         *DspComControl;
} Dcm_DspType;

/*******
 * DSD *
 *******/
typedef Std_ReturnType (*DsdSubServiceFncType)(Dcm_OpStatusType opStatus, Dcm_MsgContextType *msg);/* @req DCM764 */

typedef struct {
    uint8                           DsdSubServiceId;
    DsdSubServiceFncType            DsdSubServiceFnc;
    const Dcm_DspSecurityRowType    * const *DsdSubServiceSecurityLevelRef; // (1..*)
    const Dcm_DspSessionRowType     * const *DsdSubServiceSessionLevelRef;      // (1..*)
    boolean                         Arc_EOL;
} Dcm_DsdSubServiceType;

// 10.2.4 DcmDsdService
typedef struct {
    uint8							DsdSidTabServiceId;				// (1)
    boolean							DsdSidTabSubfuncAvail;			// (1)
    const Dcm_DspSecurityRowType	* const *DsdSidTabSecurityLevelRef;	// (1..*)
    const Dcm_DspSessionRowType		* const *DsdSidTabSessionLevelRef;		// (1..*)
    // Containers
    Dcm_DsdDspSidTabFncType         DspSidTabFnc;
    const Dcm_DsdSubServiceType     *const DsdSubServiceList;
    boolean							Arc_EOL;
} Dcm_DsdServiceType;

// 10.2.3 DcmDsdServiceTable
typedef struct {
    uint8						DsdSidTabId; // (1)
    // Containers
    const Dcm_DsdServiceType	*DsdService; // (1..*)
    boolean						Arc_EOL;
} Dcm_DsdServiceTableType;

// 10.2.2 DcmDsd
typedef struct {
    // Containers
    const Dcm_DsdServiceTableType *DsdServiceTable; // (1..256)
} Dcm_DsdType;




/*******
 * DSL *
 *******/

typedef enum
{
    BUFFER_AVAILABLE,
    BUFFER_BUSY
}Dcm_DslBufferStatusType;


typedef enum
{
    NOT_IN_USE,	// The buffer is not used (it is available).
    IN_USE,
    PROVIDED_TO_PDUR, // The buffer is currently in use by PDUR.
    DSD_PENDING_RESPONSE_SIGNALED, // Signals have been received saying the buffer contain valid data.
    DCM_TRANSMIT_SIGNALED, // The DCM has been asked to transfer the response, system is now waiting for TP layer to reqest Tx buffer.
    PROVIDED_TO_DSD,	// The buffer is currently in use by DSD.
    PREEMPT_TRANSMIT_NRC,//when preemption happens,then sent NRC 0x21 to OBD tester
#if (DCM_USE_SPLIT_TASK_CONCEPT == STD_ON)
    PENDING_BUFFER_RELEASE,
#endif
    UNDEFINED_USAGE
}Dcm_DslBufferUserType;

typedef struct {
    Dcm_DslBufferStatusType status; // Flag for buffer in use.
    PduLengthType nofBytesHandled;
    PduIdType DcmRxPduId; //The external buffer is locked to this PDU id
#if (DCM_USE_SPLIT_TASK_CONCEPT == STD_ON)
    boolean dspProcessingActive;/* Flag indicating that a dsp processing is currently active and buffers can be accessed only by dsp */
#endif
} Dcm_DslBufferRuntimeType;

// 10.2.6
typedef struct {
    uint8						DslBufferID;	// (1) // Kept for reference, will be removed (polite calls will be made).
    uint16						DslBufferSize;	// (1)
    PduInfoType					pduInfo;
    Dcm_DslBufferRuntimeType	*externalBufferRuntimeData;
} Dcm_DslBufferType;

// 10.2.7
typedef struct {
    Dcm_CallbackStartProtocolFncType	StartProtocol;
    Dcm_CallbackStopProtocolFncType		StopProtocol;
    boolean								Arc_EOL;
} Dcm_DslCallbackDCMRequestServiceType;

// 10.2.8
typedef struct {
    boolean	DslDiagRespForceRespPendEn;	// (1)
    uint8	DslDiagRespMaxNumRespPend;	// (1)
} Dcm_DslDiagRespType;

// 10.2.18
typedef struct {
//    uint16			TimStrP2ServerMax;		// (1)
//    uint16			TimStrP2ServerMin;		// (1)
    uint32			TimStrP2ServerAdjust;	// (1)
    uint32			TimStrP2StarServerAdjust;	// (1)
    uint32 			TimStrS3Server;			// (1)
    const boolean	Arc_EOL;
} Dcm_DslProtocolTimingRowType;

// 10.2.17
typedef struct {
    const Dcm_DslProtocolTimingRowType *DslProtocolTimingRow; // (0..*)
} Dcm_DslProtocolTimingType;

// 10.2.16
typedef uint8 Dcm_DslResponseOnEventType;

/* Makes it possible to cross-reference structures. */
typedef struct Dcm_DslMainConnectionType_t Dcm_DslMainConnectionType;
typedef struct Dcm_DslProtocolRxType_t Dcm_DslProtocolRxType;

// 10.2.13
struct Dcm_DslProtocolRxType_t {
    const Dcm_DslMainConnectionType	*DslMainConnectionParent;				// (1) /* Cross reference. */
    const Dcm_ProtocolAddrTypeType	DslProtocolAddrType;					// (1)
    const PduIdType					DcmDslProtocolRxPduId;					// (1)
    const uint8                     ComMChannelInternalIndex;
    const boolean					Arc_EOL;
};

/* Makes it possible to cross-reference structures. */
//typedef struct Dcm_DslMainConnectionType_t Dcm_DslMainConnectionType;
typedef struct Dcm_DslProtocolTxType_t Dcm_DslProtocolTxType;

// 10.2.14
struct Dcm_DslProtocolTxType_t {
    const Dcm_DslMainConnectionType	*DslMainConnectionParent;	// (1) /* Cross reference. */
    const PduIdType					DcmDslProtocolTxPduId;		// (1) /* Will be removed (polite), kept for reference. */
    const boolean					Arc_EOL;
};


/* Make it possible to cross reference. */
typedef struct Dcm_DslConnectionType_t Dcm_DslConnectionType;
/* Make it possible to cross reference. */
typedef struct Dcm_DslProtocolRowType_t Dcm_DslProtocolRowType;

typedef struct {
    const Dcm_DslProtocolRowType *DslProtocolRow;
    const PduIdType PduRTxPduId;
    const PduIdType DcmTxPduId;
    const boolean Arc_EOL;
}Dcm_DslPeriodicTxType;
// 10.2.15
typedef struct {
    const Dcm_DslProtocolRowType *DslProtocolRow;
    const Dcm_DslPeriodicTxType *TxPduList;
    const boolean Arc_EOL;
} Dcm_DslPeriodicTransmissionType;
// 10.2.12
struct Dcm_DslMainConnectionType_t { // Cross referenced from Dcm_DslProtocolRxType_t.
    const Dcm_DslConnectionType				*DslConnectionParent; // Cross reference.
    const Dcm_DslPeriodicTransmissionType	*DslPeriodicTransmissionConRef;	// (0..1)
    // Containers
    const Dcm_DslProtocolRxType				*DslPhysicalProtocolRx;
    const Dcm_DslProtocolTxType				*DslProtocolTx;					// (1)
    uint16                                  DslRxTesterSourceAddress;       // (1)
};

// 10.2.11
struct Dcm_DslConnectionType_t {
    // Containers
    const Dcm_DslProtocolRowType			*DslProtocolRow;			// Cross reference.
    const Dcm_DslMainConnectionType			*DslMainConnection;			// (1)
    const Dcm_DslPeriodicTransmissionType	*DslPeriodicTransmission;	// (0..1)
    boolean 								Arc_EOL;
};

typedef enum {
    DCM_IDLE = 0, /* Not in use. */
    DCM_WAITING_DIAGNOSTIC_RESPONSE, /* A diagnostic request has been forwarded to the DSD, and DSL is waiting for response. */
    DCM_DIAGNOSTIC_RESPONSE_PENDING, /* A diagnostic response has been deployed to the external buffer and is waiting to be transmitted. */
    DCM_TRANSMITTING_EXTERNAL_BUFFER_DATA_TO_PDUR, /* We are in the process of transmitting a diagnostic response most likely that reside in the external buffer, from DSD to PDUR. */
    DCM_TRANSMITTING_LOCAL_BUFFER_DATA_TO_PDUR /*  */
} Dcm_DslProtocolStateType;

typedef enum {
    DCM_DSL_PDUR_DCM_IDLE = 0,

    DCM_DSL_PDUR_TRANSMIT_INDICATED = 1,
    DCM_DSL_PDUR_TRANSMIT_TX_BUFFER_PROVIDED = 2,

    DCM_DSL_RECEPTION_INDICATED = 3,
    DCM_DSL_RX_BUFFER_PROVIDED = 4
} Dcm_DslPdurCommuncationState;


// This buffer is used for implement 7.2.4.3 (Concurrent "tester present").

#define DCM_DSL_LOCAL_BUFFER_LENGTH 8

typedef struct {
    Dcm_DslBufferUserType	status;
    uint8					buffer[DCM_DSL_LOCAL_BUFFER_LENGTH];
    PduLengthType			messageLenght;
    PduInfoType				PduInfo;
    PduLengthType			nofBytesHandled;
    Dcm_NegativeResponseCodeType responseCode;
    PduIdType 				DcmRxPduId; //The local buffer is locked to this PDU id
} Dcm_DslLocalBufferType;


typedef struct {
    PduIdType				diagReqestRxPduId;  // Tester request PduId.
    uint32					stateTimeoutCount; // Counter for timeout.
    Dcm_DslBufferUserType	externalRxBufferStatus;
    PduInfoType				diagnosticRequestFromTester;
    PduInfoType				diagnosticResponseFromDsd;
    Dcm_DslBufferUserType	externalTxBufferStatus;
    boolean					protocolStarted; // Has the protocol been started?
    Dcm_DslLocalBufferType	localRxBuffer;
    Dcm_DslLocalBufferType	localTxBuffer;
    boolean					diagnosticActiveComM; //
    uint32					S3ServerTimeoutCount;
    boolean					S3ServerStarted;
    uint8					responsePendingCount;
    Dcm_SecLevelType		securityLevel;
    Dcm_SesCtrlType			sessionControl;
    Dcm_DslLocalBufferType  PeriodicTxBuffer;
    uint32					preemptTimeoutCount;
    PduIdType               diagResponseTxPduId;
    boolean                 isType2Tx;
} Dcm_DslRunTimeProtocolParametersType;

// 10.2.10
struct Dcm_DslProtocolRowType_t { // Cross referenced from Dcm_DslConnectionType_t.
    Dcm_ProtocolType						DslProtocolID; // (1)
    boolean									DslProtocolIsParallelExecutab; // (1)
    uint16									DslProtocolPreemptTimeout; // (1)
    uint8									DslProtocolPriority; // (1)
    Dcm_ProtocolTransTypeType				DslProtocolTransType; // (1)
    const Dcm_DslBufferType					*DslProtocolRxBufferID; // (1)
    const Dcm_DslBufferType					*DslProtocolTxBufferID; // (1)
    const Dcm_DsdServiceTableType			*DslProtocolSIDTable; // (1)
    const Dcm_DslProtocolTimingRowType		*DslProtocolTimeLimit; // (0..1)
    // Containers
    const Dcm_DslConnectionType				*DslConnections; // (1..*)/* Only main connections */
    // Reference to runtime parameters to this protocol.
    Dcm_DslRunTimeProtocolParametersType	*DslRunTimeProtocolParameters; // Maybe this needs to change to index.
    boolean                                 DslSendRespPendOnTransToBoot;
    boolean									Arc_EOL;
};

// 10.2.9
typedef struct {
    // Containers
    const Dcm_DslProtocolRxType		*DslProtocolRxGlobalList;	// (1..*)  A polite list for all RX protocol configurations.
    const Dcm_DslProtocolTxType		*DslProtocolTxGlobalList;	// (1..*)  A polite list for all TX protocol configurations.
    const Dcm_DslPeriodicTxType     *DslProtocolPeriodicTxGlobalList;
    const Dcm_DslProtocolRowType	*DslProtocolRowList;		// (1..*)
} Dcm_DslProtocolType;

// 10.2.19
typedef struct {
    Dcm_CallbackNotificationIndicationFncType	Indication;
    Dcm_CallbackNotificationConfirmationFncType Confirmation;
    boolean							Arc_EOL;
} Dcm_DslServiceRequestNotificationType;


// 10.2.5
typedef struct {
    // Containers
    const Dcm_DslBufferType						*DslBuffer;						// (1..256)
    const Dcm_DslCallbackDCMRequestServiceType	*DslCallbackDCMRequestService;	// (1..*)
    const Dcm_DslDiagRespType					*DslDiagResp;					// (1)
    const Dcm_DslProtocolType					*DslProtocol;					// (1)
    const Dcm_DslProtocolTimingType				*DslProtocolTiming;				// (1)
    const Dcm_DslServiceRequestNotificationType	*DslServiceRequestNotification;	// (0..*)
} Dcm_DslType;

typedef struct {
    Dcm_ComControlModeSwitchFcnType ModeSwitchFnc;
    NetworkHandleType NetworkHandle;
    uint8 InternalIndex;
    boolean Arc_EOL;
}Dcm_ComMChannelConfigType;

// 10.2.1 Dcm
typedef struct {
    // Containers
    const Dcm_DspType *Dsp; //	(1)
    const Dcm_DsdType *Dsd; //	(1)
    const Dcm_DslType *Dsl; //	(1)
    const Dcm_ComMChannelConfigType *DcmComMChannelCfg;
} Dcm_ConfigType;

#define DCM_ROE_MAX_SERVICE_TO_RESPOND_TO_LENGTH 10   /* Max number of bytes in the service to respond to  */
typedef struct {
    uint16 DID;
    uint8 ServiceToRespondToLength;
    uint8 ServiceToRespondTo[DCM_ROE_MAX_SERVICE_TO_RESPOND_TO_LENGTH];
}Dcm_ArcROEDidType;

typedef struct {
    const Dcm_ArcROEDidType *ROEDids;
    uint8 NofROEDids;
}Dcm_ArcROEDidPreconfigType;

/*
 * Make the DCM_Config and Dcm_ConfigPtr visible for others.
 */
extern const Dcm_ConfigType DCM_Config;
extern const Dcm_ConfigType *Dcm_ConfigPtr;


#endif /*DCM_LCFG_H_*/
