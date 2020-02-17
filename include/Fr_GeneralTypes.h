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

/** @reqSettings DEFAULT_SPECIFICATION_REVISION=4.0.3 */
/** @tagSettings DEFAULT_ARCHITECTURE=RH850F1H */

/* @req FR455 */
/* @req FR499 */
#ifndef FR_GENERAL_TYPES
#define FR_GENERAL_TYPES

#if defined(USE_DEM) || defined(CFG_FR_DEM_TEST)
#include "Dem.h"
#endif

/* @req FR102 */

/* @req FR117 */
/* @req FR110 */
/* @req FR077 */

/* @req FR505 */
typedef enum {
    FR_POCSTATE_CONFIG = 0,
    FR_POCSTATE_DEFAULT_CONFIG,
    FR_POCSTATE_HALT,
    FR_POCSTATE_NORMAL_ACTIVE,
    FR_POCSTATE_NORMAL_PASSIVE,
    FR_POCSTATE_READY,
    FR_POCSTATE_STARTUP,
    FR_POCSTATE_WAKEUP
}Fr_POCStateType;

/* @req FR506 */
typedef enum {
    FR_SLOTMODE_KEYSLOT,
    FR_SLOTMODE_ALL_PENDING,
    FR_SLOTMODE_ALL
}Fr_SlotModeType;

/* @req FR599 */
#define FR_SLOTMODE_SINGLE()  FR_SLOTMODE_KEYSLOT

/* @req FR507 */
typedef enum {
    FR_ERRORMODE_ACTIVE = 0,
    FR_ERRORMODE_PASSIVE,
    FR_ERRORMODE_COMM_HALT
}Fr_ErrorModeType;

/* @req FR508 */
typedef enum {
    FR_WAKEUP_UNDEFINED = 0,
    FR_WAKEUP_RECEIVED_HEADER,
    FR_WAKEUP_RECEIVED_WUP,
    FR_WAKEUP_COLLISION_HEADER,
    FR_WAKEUP_COLLISION_WUP,
    FR_WAKEUP_COLLISION_UNKNOWN,
    FR_WAKEUP_TRANSMITTED
}Fr_WakeupStatusType;

/* @req FR509 */
typedef enum {
    FR_STARTUP_UNDEFINED = 0,
    FR_STARTUP_COLDSTART_LISTEN,
    FR_STARTUP_INTEGRATION_COLDSTART_CHECK,
    FR_STARTUP_COLDSTART_JOIN,
    FR_STARTUP_COLDSTART_COLLISION_RESOLUTION,
    FR_STARTUP_COLDSTART_CONSISTENCY_CHECK,
    FR_STARTUP_INTEGRATION_LISTEN,
    FR_STARTUP_INITIALIZE_SCHEDULE,
    FR_STARTUP_INTEGRATION_CONSISTENCY_CHECK,
    FR_STARTUP_COLDSTART_GAP,
    FR_STARTUP_EXTERNAL_STARTUP
}Fr_StartupStateType;

/* !req FR510 */
typedef struct {
    boolean CHIHaltRequest;
    boolean ColdstartNoise;
    Fr_ErrorModeType ErrorMode;
    boolean Freeze;
    Fr_SlotModeType SlotMode;
    Fr_StartupStateType StartupState;
    Fr_POCStateType State;
    Fr_WakeupStatusType WakeupStatus;
    /*boolean CHIReadyRequest;  NOT SUPPORTED */
}Fr_POCStatusType;

/* @req FR511 */
typedef enum {
    FR_TRANSMITTED = 0,
    FR_NOT_TRANSMITTED
}Fr_TxLPduStatusType;

/* @req FR512 */
typedef enum {
    FR_RECEIVED = 0,
    FR_NOT_RECEIVED,
    FR_RECEIVED_MORE_DATA_AVAILABLE
}Fr_RxLPduStatusType;

/* @req FR468 */
/* @req FR514 */
typedef enum {
    FR_CHANNEL_A = 0,
    FR_CHANNEL_B,
    FR_CHANNEL_AB
}Fr_ChannelType;

/* @req FrTrcv434 */
typedef enum {
    FRTRCV_TRCVMODE_NORMAL,
    FRTRCV_TRCVMODE_STANDBY,
    FRTRCV_TRCVMODE_SLEEP, // sleep is optional.
    FRTRCV_TRCVMODE_RECEIVEONLY // receive only is optional.
}FrTrcv_TrcvModeType;

/* @req FrTrcv435 */
/* @req FrTrcv074 */
typedef enum {
    FRTRCV_WU_NOT_SUPPORTED,
    FRTRCV_WU_BY_BUS,
    FRTRCV_WU_BY_PIN,
    FRTRCV_WU_INTERNALLY,
    FRTRCV_WU_RESET,
    FRTRCV_WU_POWER_ON
}FrTrcv_TrcvWUReasonType;

#if 0
typedef struct {
    //Implementation specific

    //This type contains the implementation-specific post build time configuration structure.
    //Only pointers of this type are allowed.
}FrIf_ConfigType;
#endif

typedef enum {
    FRIF_STATE_OFFLINE,
    FRIF_STATE_ONLINE
}FrIf_StateType;

typedef enum {
    FRIF_GOTO_OFFLINE,
    FRIF_GOTO_ONLINE
}FrIf_StateTransitionType;

typedef enum {
    FR_N1SAMPLES = 0,
    FR_N2SAMPLES,
    FR_N4SAMPLES
}Fr_SamplePerMicroTickType;

typedef enum {
    FR_T12_5NS = 0,
    FR_T25NS,
    FR_T50NS,
    FR_T100NS,
    FR_T200NS,
    FR_T400NS
}Fr_MicroTickType;

/* !req FR657 - Fr_ReadCCConfig not implemented*/
/* Macro name,                                      Value,  Mapps to configuration parameter */
#define FR_CIDX_GDCYCLE                         (uint8)0U   /* 0, FrIfGdCycle */
#define FR_CIDX_PMICROPERCYCLE                  (uint8)1U   /* 1, FrPMicroPerCycle */
#define FR_CIDX_PDLISTENTIMEOUT                 (uint8)2U   /* 2, FrPdListenTimeout */
#define FR_CIDX_GMACROPERCYCLE                  (uint8)3U   /* 3, FrIfGMacroPerCycle */
#define FR_CIDX_GDMACROTICK                     (uint8)4U   /* 4, FrIfGdMacrotick */
#define FR_CIDX_GNUMBEROFMINISLOTS              (uint8)5U   /* 5, FrIfGNumberOfMinislots */
#define FR_CIDX_GNUMBEROFSTATICSLOTS            (uint8)6U   /* 6, FrIfGNumberOfStaticSlots */
#define FR_CIDX_GDNIT                           (uint8)7U   /* 7, FrIfGdNit */
#define FR_CIDX_GDSTATICSLOT                    (uint8)8U   /* 8, FrIfGdStaticSlot */
#define FR_CIDX_GDWAKEUPRXWINDOW                (uint8)9U   /* 9, FrIfGdWakeupRxWindow */
#define FR_CIDX_PKEYSLOTID                      (uint8)10U  /* 10, FrPKeySlotId */
#define FR_CIDX_PLATESTTX                       (uint8)11U  /* 11, FrPLatestTx */
#define FR_CIDX_POFFSETCORRECTIONOUT            (uint8)12U  /* 12, FrPOffsetCorrectionOut */
#define FR_CIDX_POFFSETCORRECTIONSTART          (uint8)13U  /* 13, FrPOffsetCorrectionStart */
#define FR_CIDX_PRATECORRECTIONOUT              (uint8)14U  /* 14, FrPRateCorrectionOut */
#define FR_CIDX_PSECONDKEYSLOTID                (uint8)15U  /* 15, FrPSecondKeySlotId */
#define FR_CIDX_PDACCEPTEDSTARTUPRANGE          (uint8)16U  /* 16, FrPdAcceptedStartupRange */
#define FR_CIDX_GCOLDSTARTATTEMPTS              (uint8)17U  /* 17, FrIfGColdStartAttempts */
#define FR_CIDX_GCYCLECOUNTMAX                  (uint8)18U  /* 18, FrIfGCycleCountMax */
#define FR_CIDX_GLISTENNOISE                    (uint8)19U  /* 19, FrIfGListenNoise */
#define FR_CIDX_GMAXWITHOUTCLOCKCORRECTFATAL    (uint8)20U  /* 20, FrIfGMaxWithoutClockCorrectFatal */
#define FR_CIDX_GMAXWITHOUTCLOCKCORRECTPASSIVE  (uint8)21U  /* 21, FrIfGMaxWithoutClockCorrectPassive */
#define FR_CIDX_GNETWORKMANAGEMENTVECTORLENGTH  (uint8)22U  /* 22, FrIfGNetworkManagementVectorLength */
#define FR_CIDX_GPAYLOADLENGTHSTATIC            (uint8)23U  /* 23, FrIfGPayloadLengthStatic */
#define FR_CIDX_GSYNCFRAMEIDCOUNTMAX            (uint8)24U  /* 24, FrIfGSyncFrameIDCountMax */
#define FR_CIDX_GDACTIONPOINTOFFSET             (uint8)25U  /* 25, FrIfGdActionPointOffset */
#define FR_CIDX_GDBIT                           (uint8)26U  /* 26, FrIfGdBit */
#define FR_CIDX_GDCASRXLOWMAX                   (uint8)27U  /* 27, FrIfGdCasRxLowMax */
#define FR_CIDX_GDDYNAMICSLOTIDLEPHASE          (uint8)28U  /* 28, FrIfGdDynamicSlotIdlePhase */
#define FR_CIDX_GDMINISLOTACTIONPOINTOFFSET     (uint8)29U  /* 29, FrIfGdMiniSlotActionPointOffset */
#define FR_CIDX_GDMINISLOT                      (uint8)30U  /* 30, FrIfGdMinislot */
#define FR_CIDX_GDSAMPLECLOCKPERIOD             (uint8)31U  /* 31, FrIfGdSampleClockPeriod */
#define FR_CIDX_GDSYMBOLWINDOW                  (uint8)32U  /* 32, FrIfGdSymbolWindow */
#define FR_CIDX_GDSYMBOLWINDOWACTIONPOINTOFFSET (uint8)33U  /* 33, FrIfGdSymbolWindowActionPointOffset */
#define FR_CIDX_GDTSSTRANSMITTER                (uint8)34U  /* 34, FrIfGdTssTransmitter */
#define FR_CIDX_GDWAKEUPRXIDLE                  (uint8)35U  /* 35, FrIfGdWakeupRxIdle */
#define FR_CIDX_GDWAKEUPRXLOW                   (uint8)36U  /* 36, FrIfGdWakeupRxLow */
#define FR_CIDX_GDWAKEUPTXACTIVE                (uint8)37U  /* 37, FrIfGdWakeupTxActive */
#define FR_CIDX_GDWAKEUPTXIDLE                  (uint8)38U  /* 38, FrIfGdWakeupTxIdle */
#define FR_CIDX_PALLOWPASSIVETOACTIVE           (uint8)39U  /* 39, FrPAllowPassiveToActive */
#define FR_CIDX_PCHANNELS                       (uint8)40U  /* 40, FrPChannels */
#define FR_CIDX_PCLUSTERDRIFTDAMPING            (uint8)41U  /* 41, FrPClusterDriftDamping */
#define FR_CIDX_PDECODINGCORRECTION             (uint8)42U  /* 42, FrPDecodingCorrection */
#define FR_CIDX_PDELAYCOMPENSATIONA             (uint8)43U  /* 43, FrPDelayCompensationA */
#define FR_CIDX_PDELAYCOMPENSATIONB             (uint8)44U  /* 44, FrPDelayCompensationB */
#define FR_CIDX_PMACROINITIALOFFSETA            (uint8)45U  /* 45, FrPMacroInitialOffsetA */
#define FR_CIDX_PMACROINITIALOFFSETB            (uint8)46U  /* 46, FrPMacroInitialOffsetB */
#define FR_CIDX_PMICROINITIALOFFSETA            (uint8)47U  /* 47, FrPMicroInitialOffsetA */
#define FR_CIDX_PMICROINITIALOFFSETB            (uint8)48U  /* 48, FrPMicroInitialOffsetB */
#define FR_CIDX_PPAYLOADLENGTHDYNMAX            (uint8)49U  /* 49, FrPPayloadLengthDynMax */
#define FR_CIDX_PSAMPLESPERMICROTICK            (uint8)50U  /* 50, FrPSamplesPerMicrotick */
#define FR_CIDX_PWAKEUPCHANNEL                  (uint8)51U  /* 51, FrPWakeupChannel */
#define FR_CIDX_PWAKEUPPATTERN                  (uint8)52U  /* 52, FrPWakeupPattern */
#define FR_CIDX_PDMICROTICK                     (uint8)53U  /* 53, FrPdMicrotick */
#define FR_CIDX_GDIGNOREAFTERTX                 (uint8)54U  /* 54, FrIfGdIgnoreAfterTx */
#define FR_CIDX_PALLOWHALTDUETOCLOCK            (uint8)55U  /* 55, FrPAllowHaltDueToClock */
#define FR_CIDX_PEXTERNALSYNC                   (uint8)56U  /* 56, FrPExternalSync */
#define FR_CIDX_PFALLBACKINTERNAL               (uint8)57U  /* 57, FrPFallBackInternal */
#define FR_CIDX_PKEYSLOTONLYENABLED             (uint8)58U  /* 58, FrPKeySlotOnlyEnabled */
#define FR_CIDX_PKEYSLOTUSEDFORSTARTUP          (uint8)59U  /* 59, FrPKeySlotUsedForStartup */
#define FR_CIDX_PKEYSLOTUSEDFORSYNC             (uint8)60U  /* 60, FrPKeySlotUsedForSync */
#define FR_CIDX_PNMVECTOREARLYUPDATE            (uint8)61U  /* 61, FrPNmVectorEarlyUpdate */
#define FR_CIDX_PTWOKEYSLOTMODE                 (uint8)62U  /* 62, FrPTwoKeySlotMode */

// Controller configuration
typedef struct {
        uint32  FrCtrlIdx;
        boolean FrPAllowHaltDueToClock;
        uint32  FrPAllowPassiveToActive;
        Fr_ChannelType FrPChannels;
        uint32  FrPClusterDriftDamping;
        uint32  FrPDecodingCorrection;
        uint32  FrPDelayCompensationA;
        uint32  FrPDelayCompensationB;
        boolean FrPExternalSync;
        boolean FrPFallBackInternal;
        uint32  FrPKeySlotId;
        boolean FrPKeySlotOnlyEnabled;      /* Maps to 2.1 pSingleSlotEnabled */
        boolean FrPKeySlotUsedForStartup;
        boolean FrPKeySlotUsedForSync;
        uint32  FrPLatestTx;
        uint32  FrPMacroInitialOffsetA;
        uint32  FrPMacroInitialOffsetB;
        uint32  FrPMicroInitialOffsetA;
        uint32  FrPMicroInitialOffsetB;
        uint32  FrPMicroPerCycle;
        uint32  FrPNmVectorEarlyUpdate;
        uint32  FrPOffsetCorrectionOut;
        uint32  FrPOffsetCorrectionStart;
        uint32  FrPPayloadLengthDynMax;
        uint32  FrPRateCorrectionOut;
        Fr_SamplePerMicroTickType FrPSamplesPerMicrotick;
        uint32  FrPSecondKeySlotId;
        boolean FrPTwoKeySlotMode;
        Fr_ChannelType FrPWakeupChannel;
        uint32  FrPWakeupPattern;
        uint32  FrPdAcceptedStartupRange;
        uint32  FrPdListenTimeout;
        Fr_MicroTickType FrPdMicrotick;
        uint8  FrArcAbsTimerMaxIdx;
#if defined(USE_DEM) || defined(CFG_FR_DEM_TEST)
        Dem_EventIdType FrDemEventParamRef;
#endif
        //FrFifo -- Not supported yet
}Fr_CtrlConfigParametersType;

typedef struct {
    uint32 FrTrigObjectId;
    boolean FrTrigAllowDynamicLen;
    boolean FrTrigAlwaysTransmit;
    uint32  FrTrigBaseCycle;
    Fr_ChannelType  FrTrigChannel;
    uint32  FrTrigCycleRepetition;
    boolean FrTrigPayloadPreamble;
    uint32  FrTrigSlotId;
    uint32  FrTrigLSduLength;
    uint32 FrTrigIsTx;
#if defined(USE_DEM) || defined(CFG_FR_DEM_TEST)
    Dem_EventIdType FrTrigDemFTSlotStatusRef;
#endif
}Fr_FrIfTriggeringConfType;

typedef struct {
    uint16 FrMsgBufferIdx;
    uint32 FrDataPartitionAddr;
    uint32 FrCurrentLengthSetup;
}Fr_MessageBufferConfigType;

typedef struct {
    uint32 FrNbrTrigConfiged;
    uint32 FrNbrTrigStatic;
    const Fr_FrIfTriggeringConfType *FrTrigConfPtr;
    Fr_MessageBufferConfigType *FrMsgBufferCfg;
}Fr_FrIfCCTriggeringType;


typedef struct {
    boolean FrLpdIsReconf;
    uint32 FrLpdTriggIdx;
}Fr_FrIfLPduType;


typedef struct {
    const uint16 FrNbrLPdusConfigured;
    const Fr_FrIfLPduType *FrLpdu;
}Fr_FrIfLPduContainerType;

typedef struct {
    uint32 FrClusterGColdStartAttempts;
    uint32 FrClusterGListenNoise;
    uint32 FrClusterGMaxWithoutClockCorrectPassive;
    uint32 FrClusterGMaxWithoutClockCorrectFatal;
    uint32 FrClusterGNetworkManagementVectorLength;
    uint32 FrClusterGdTSSTransmitter;
    uint32 FrClusterGdCasRxLowMax;
    Fr_MicroTickType FrClusterGdSampleClockPeriod;
    uint32 FrClusterGdSymbolWindow;
    uint32 FrClusterGdWakeupRxIdle;
    uint32 FrClusterGdWakeupRxLow;
    uint32 FrClusterGdWakeupTxIdle;
    uint32 FrClusterGdWakeupTxActive;           /* Maps to 2.1 gdWakeupSymbolTxLow */
    uint32 FrClusterGPayloadLengthStatic;
    uint32 FrClusterGMacroPerCycle;
    uint32 FrClusterGSyncFrameIDCountMax;       /* Maps to 2.1 gSyncNodeMax */
    uint32 FrClusterGdNit;
    uint32 FrClusterGdStaticSlot;
    uint32 FrClusterGNumberOfStaticSlots;
    uint32 FrClusterGdMiniSlotActionPointOffset;
    uint32 FrClusterGNumberOfMinislots;
    uint32 FrClusterGdActionPointOffset;
    uint32 FrClusterGdDynamicSlotIdlePhase;
    uint32 FrClusterGdMinislot;
    uint8 FrClusterGCycleCountMax;
    Fr_MicroTickType FrClusterGdBit;
    float32 FrClusterGdCycle;
    uint32 FrClusterGdIgnoreAfterTx;
    float32 FrClusterGdMacrotick;
    uint32 FrClusterGdSymbolWindowActionPointOffset;
    uint32 FrClusterGdWakeupRxWindow;
}Fr_FrIfClusterConfigType;


#endif /*FR_GENERAL_TYPES*/
