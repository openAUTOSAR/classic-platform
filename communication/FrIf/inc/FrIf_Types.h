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

#ifndef FRIF_TYPES_H_
#define FRIF_TYPES_H_

#if defined(USE_DEM)
#include "Dem.h"
#endif

#define FRIF_NO_FUNCTION_CALLOUT 0xFFu

/* @req FrIf05082 */ /* FrIf_camelCase naming convention used for types */
typedef enum {
    PDU_OUTDATED,
    PDU_UPDATED
} FrIf_PduUpdateBitType; /* Pdu status */

typedef enum {
    T100NS = 1,
    T200NS = 2,
    T400NS = 4
} FrIf_BitTimeType; /* Bit timing type */

typedef enum {
    FRIF_CHANNEL_A = FR_CHANNEL_A,
    FRIF_CHANNEL_B = FR_CHANNEL_B,
    FRIF_CHANNEL_AB = FR_CHANNEL_AB
} FrIf_ChannelType; /* FrIf channel types */

typedef enum {
    T12_5NS = 12,
    T25NS   = 25,
    T50NS   = 50
} FrIf_ClockPeriodType; /* Clock period */

typedef enum {
    DECOUPLED_TRANSMISSION,
    PREPARE_LPDU,
    RECEIVE_AND_INDICATE,
    RECEIVE_AND_STORE,
    RX_INDICATION,
    TX_CONFIRMATION,
    FREE_OP_A,
    FREE_OP_B
} FrIf_CommunicationActionType; /* Job communication action */

typedef enum {
	FRIF_BIG_ENDIAN,
	FRIF_LITTLE_ENDIAN
} FrIf_ByteOrderType; /* Byte order */

/* Function pointers */
typedef void (*FrIf_RxIndicationType)(PduIdType pduId , PduInfoType* pduInfo);
typedef Std_ReturnType (*FrIf_TriggerTransmitType)(PduIdType pduId, PduInfoType* pduInfo);
typedef void (*FrIf_TxConfirmationType)(PduIdType pduId);
/**
 * Structs
 */


#if 0
typedef struct {
    FrIf_ChannelType FrIf_ClusterChannel;
    const FrTrcvChannel * FrIf_FrTrcvChannelPtr;
} FrIf_TransceiverType;
#endif

/** FrIf Immediate TxPdu config */
typedef struct {
    uint8 FrIf_FrCtrlIdx;                                     /* Fr controller index */
    uint16  Fr_Buffer;                                       /* Reference to driver buffer */
}FrIf_ImmediateTxPduCfgType;

/** FrIfTxPdu container config structure */
typedef struct {
    uint8               FrIf_UserTriggerTransmitHandle; /* Callback handle for TriggerTransmit */
    uint8               FrIf_TxConfirmationHandle; /* Callback handle for transmit confirmation */
    uint8               FrIf_TxLength;          /* Transmit Pdu length */
    uint8               FrIf_CounterLimit;      /* Maximum limit of indication of ready PDU data to FrIf */
    boolean             FrIf_Confirm;           /* Confirmation enabled or not */
    boolean             FrIf_Immediate;         /* Immediate mode enabled or not */
    uint8 				FrIf_ImmediateTxCfgIdx; /* Index to immediate Tx configuration */
    boolean             FrIf_NoneMode;          /* None mode set or not */
} FrIf_TxPduType;

/** FrIfRxPdu container config structure */
typedef struct {
    uint8               FrIf_RxIndicationHandle;            /* Callback handle for receive indication */
    uint8               RxPduLength;                        /* Length of Pdu */
} FrIf_RxPduType;

/** FrIfPduDirection container config structure */
/*lint --e{9018} */
typedef union {
    const FrIf_RxPduType * FrIf_RxPduPtr;    /* Pointer to either Rx or Tx Pdu config structure */
    const FrIf_TxPduType * FrIf_TxPduPtr;
} FrIf_PduDirectionType;

/** FrIPdu config */
typedef struct {
    const FrIf_PduDirectionType * FrIf_PduDirectionPtr; /* Maps to one FrIfPduDirection config structure */
    PduIdType              FrIf_PduId;                  /* Pdu-Id */
    PduIdType              FrIf_UpperLayerPduId;        /* Pdu Id of the upper layer*/
} FrIf_PduType;

/** FrIfPdusInFrame config structure */
typedef struct {
    const FrIf_PduType * FrIf_Pdu;               /* Reference to IPdu used by the frame */
    sint16      FrIf_PduUpdateBitOffset;        /* Update Bit Offset used */
    uint8       FrIf_PduOffset;                 /* Offset of Pdu within the Frame */
} FrIf_PdusIn_FrameType;

/** FrIfFrameStructure config structure */
typedef struct {
    const FrIf_PdusIn_FrameType * FrIf_PdusInFramePtr;       /* Reference to collection of IPdu composition */
    FrIf_ByteOrderType    FrIf_ByteOrder;                   /* Byte Order used */
} FrIf_FrameStructureType;



/** FrIfFrameTriggering config structure*/
typedef struct {
    const FrIf_FrameStructureType * FrIf_FrameStructureRef; /* Reference to FrIfFrameStructure */
    /* uint16           FrIf_MessageId;                    Message Id if this Frame is used in a dynamic segment*/
    uint16           FrIf_SlotId;                       /* Slot Id in which this Frame is Tx */
    uint16           FrIf_NumberPdusInFrame;            /* Number of IPdus part of a frame */
    uint16           FrIf_LPduIdx;                      /* FrIf LPdu index */
    uint8            FrIf_LSduLength;                   /* Total length of the Frame */
    uint8            FrIf_BaseCycle;                    /* Base cycle used to transmit the frame */
    uint8            FrIf_CycleRepetition;              /* Flexray repetition cycle */
    uint8            FrIf_FrameStructureIdx;            /* Flexray frame structure index */
    FrIf_ChannelType FrIf_Channel;                      /* Kind of channel on which this frame is transmitted A, B or AB */
    /* boolean          FrIf_PayloadPreamble;               Indicates whether payload preamble is present or not */
    /* boolean          FrIf_AllowDynamicLSduLength;        If dynamic frame length is supported */
    boolean          FrIf_AlwaysTransmit;               /* Whether Fr API needs to be called everytime before transmission */
} FrIf_FrameTriggeringType;

/** FrIfLPdu config structure */
typedef struct {
    const FrIf_FrameTriggeringType * FrIf_VBTriggeringRef;    /* Reference to FrIfFrameTriggering config structure */
    uint16  Fr_Buffer;                                       /* Reference to driver buffer */
    boolean FrIf_Reconfigurable;                             /* Determines whether this LPdu can be configured to map different FrIfFrameTriggering during run time */
} FrIf_LPduType;

/** FrIfController config structure */
typedef struct {
    const FrIf_LPduType * FrIf_LpduPtr;                          /* Pointer to FrIfLPdu */
    const FrIf_FrameTriggeringType * FrIf_FrameTriggeringPtr;    /* Pointer to FrIfFrameTriggering config structure contained by corresponding controller */
    //const FrIf_TransceiverType * FrIf_TransceiverPtr;
    uint8 FrIf_FrCtrlIdx;                                       /* Index of Fr controller */
    uint8 FrIf_LPduCount;                                       /* No. of LPdus */
    uint8 FrIf_CtrlIdx;											/* FrIf Ctrl id */
    uint8 FrIf_ClstrRefIdx;										/* Cluster Id */
} FrIf_ControllerType;

/** FrIfCommunicationOperation config structure */
typedef struct {
    const FrIf_LPduType * FrIf_LPduIdxRef;                   /* Reference to the LPdu */
    FrIf_CommunicationActionType FrIf_CommunicationAction;  /* Communication Action */
    uint16 FrIf_RxComOpMaxLoop;                             /* Maximum number of loops for receive & indicate */
    uint8 FrIf_CtrlIdx;                                     /* FrIf controller index */
    uint8 FrIf_FrameConfigIdx;                              /* Reference to Frame structure */
    uint8 FrIf_CommunicationOperationIdx;                   /* Index defining the order of Flexray communication operation */
} FrIf_CommunicationOperationType;

/** FrIfJob config structure */
typedef struct {
    const FrIf_CommunicationOperationType * FrIf_CommunicationOperationPtr;  /* Reference to communication operations */
    uint32 FrIf_Macrotick;                                                  /* Macro tick offset */
    uint32 FrIf_NbrOfOperations;                                            /* No. of communication operations executed by the job */
    uint8 FrIf_Cycle;                                                       /* Cycle in which communication operation is executed */
} FrIf_JobType;

/** FrIfJobList config structure */
typedef struct {
    FrIf_JobType const* FrIf_JobPtr;                /* Reference to FrIfJobs */
    uint8 FrIf_NbrOfJobs;                           /* Number of FrIfJobs */
    uint8 FrIf_FrAbsTimerIdx;						/*  Index of the Fr Absolute timer */
    uint8 FrIf_AbsTimerIdx;                         /* Index of the FrIf Absolute timer */
} FrIf_JobListType;

#if defined(USE_DEM)
/** Dem Event parameter reference for Cluster */
typedef struct {
    Dem_EventIdType FrIf_ACSEventRefChnlA; /* Chnl aggregated Error Chnl A */
    Dem_EventIdType FrIf_ACSEventRefChnlB; /* Chnl aggregated Error Chnl B */
    Dem_EventIdType FrIf_NITEventRefChnlA; /* NIT Error Chnl A */
    Dem_EventIdType FrIf_NITEventRefChnlB; /* NIT Error Chnl B */
    Dem_EventIdType FrIf_SWEventRefChnlA; /* Symbol window Error Chnl A */
    Dem_EventIdType FrIf_SWEventRefChnlB; /* Symbol window Error Chnl B */
}FrIf_ClusterDemEventParamRefType;
#endif

/** FrIfCluster config structure */
typedef struct {
    const FrIf_ControllerType * const * FrIf_ControllerPtr;          /* Reference to FrIfControllerConfig structure */
    const FrIf_JobListType * FrIf_JobListPtr;                /* Reference to FrIfJobList config structure */
#if defined(USE_DEM)
    const FrIf_ClusterDemEventParamRefType * FrIf_ClusterDemEventParamRef; /* Reference to FrIfClusterDemEventParameterRefs */
#endif
    uint16 FrIf_GMacroPerCycle;                             /* No of macro ticks in one cycle */
    uint32 FrIf_MainFunctionPeriod;                        /* Cycle time for FrIf main function */
    uint32 FrIf_MaxIsrDelay;                                /* Max delay in macro ticks for execution of ISR job list execution after abs timer was triggered */
    uint32 FrIf_GdCycle;                                   /* Length of cycle in seconds*/
    uint32 FrIf_GdMacrotick;                               /* Length of macro tick in seconds */
    uint16 FrIf_GNumberOfStaticSlots;                       /* No. of static slots in static segment */
    uint8 FrIf_ClstIdx;                                     /* Cluster index */
    uint8 FrIf_ControllerCount;                             /* No of controllers supported by cluster */
    uint8 FrIf_GdStaticSlots;                               /* Duration of static slot */
    /* boolean FrIf_DetectNITError;                             Indicates whether NIT error status of each cluster shall be detected or not */
    /* Fr_ChannelType FrIf_GChannels;                           Channels used by the cluster */
    /* uint8 FrIf_GColdStartAttempts;                           Maximum attempts by a node to start cluster by schedule synchronization */
    /* uint8 FrIf_GCycleCountMax;                               Maximum cycle counter */
    /* uint8 FrIf_GListenNoise;                                 upper limit for startup listen timeout and wake up listen timeout */
    /* uint8 FrIf_GMaxWithoutClockCorrectFatal;                 Threshold used by clockcorrectionfailed counter. Action when this value is reached normal active/passive to halt state*/
    /* uint8 FrIf_GMaxWithoutClockCorrectPassive;               Threshold used by clockcorrectionfailed counter. Action when this value is reached normal active to normal passive state */
    /* uint8 FrIf_GNetworkManagementVectorLength;               Length of NM vector length */
    /* uint16 FrIf_GNumberOfMinislots;                          No. of mini slots in dynamic segment */

    /* uint8 FrIf_GPayloadLengthStatic;                         Payload length of a static frame */
    /* uint8 FrIf_GSyncFrameIDCountMax;                         Max number of sync frame Ids in a cluster */
    /* uint8 FrIf_GdActionPointOffset;                          Offset from the beginning of static slot for action point (in macro tick) */
    /* FrIf_BitTimeType FrIf_GdBit;                             Nominal bit time in seconds */
    /* uint8 FrIf_GdCasRxLowMax;                                Upper limit for CAS acceptance window */

    /* uint8 FrIf_GdDynamicSlotIdlePhase;                       Duration of idle phase within a dynamic slot */
    /* uint8 FrIf_GdIgnoreAfterTx;                              Duration for which bit strobing is paused after Tx */

    /* uint8 FrIf_GdMiniSlotActionPointOffset;                  Offset from the beginning of a mini slot for the action point (in macro ticks) */
    /* uint8 FrIf_GdMinislot;                                   Duraiton of Mini slot */
    /* uint16 FrIf_GdNit;                                       Duration of network idle time */
    /* FrIf_ClockPeriodType FrIf_GdSampleClockPeriod;           Sample clock period */

    /* uint16 FrIf_GdSymbolWindow;                              Duration of symbol window */
    /* uint8 FrIf_GdSymbolWindowActionPointOffset;              Offset from the beginning of symbol window for the action poin (in macro tick) */
    /* uint8 FrIf_GdTssTransmitter;                             No of bits in transmit start sequence */
    /* uint8 FrIf_GdWakeupRxIdle;                               No. of bits used by node to test duration of idle or high phase of a received wake up */
    /* uint8 FrIf_GdWakeupRxLow;                                No. of bits used by node to test duration of low phase of a received wake up */
    /* uint16 FrIf_GdWakeupRxWindow;                            The size of window used to detect wakeups */
    /* uint8 FrIf_GdWakeupTxActive;                             No. of bits used by the node to transmit Low phase of a wake up symbol and high and low phases of WUDOP */
    /* uint8 FrIf_GdWakeupTxIdle;                               No. of bits used by the node to transmit idle part of a wake up symbol */
    /* uint32 FrIf_SafetyMargin;                                Additional time span in macro ticks to set joblistpointer to next job when job list execution has been resynchronized */
} FrIf_ClusterType;

/** Function pointers each driver api's, refer to FrIf05079 and FrIf05382*/
typedef struct {
    /* Controller Initialization */
    Std_ReturnType (*Fr_ControllerInit)(uint8 Fr_CtrlIdx);

    /* Communication Initialization */
    Std_ReturnType (*Fr_StartCommunication)(uint8 Fr_CtrlIdx);
    Std_ReturnType (*Fr_AllowColdstart)(uint8 Fr_CtrlIdx);

    /* Transmit/Receive */
    Std_ReturnType (*Fr_TransmitTxLPdu)(uint8 Fr_CtrlIdx, uint16 Fr_LPduIdx, const uint8* Fr_LSduPtr, uint8 Fr_LSduLength);
    Std_ReturnType (*Fr_ReceiveRxLPdu)(uint8 Fr_CtrlIdx, uint16 Fr_LPduIdx, uint8* Fr_LSduPtr, Fr_RxLPduStatusType* Fr_RxLPduStatusPtr, uint8* Fr_LSduLengthPtr);
    Std_ReturnType (*Fr_CheckTxLPduStatus)(uint8 Fr_CtrlIdx, uint16 Fr_LPduIdx, Fr_TxLPduStatusType* Fr_TxLPduStatusPtr);
    Std_ReturnType (*Fr_PrepareLPdu)(uint8 Fr_CtrlIdx, uint16 Fr_LPduIdx);


    /* Absolute Timers  */
    Std_ReturnType (*Fr_SetAbsoluteTimer)(uint8 Fr_CtrlIdx, uint8 Fr_AbsTimerIdx, uint8 Fr_Cycle, uint16 Fr_Offset);
    Std_ReturnType (*Fr_EnableAbsoluteTimerIRQ)(uint8 Fr_CtrlIdx, uint8 Fr_AbsTimerIdx);
    Std_ReturnType (*Fr_AckAbsoluteTimerIRQ)(uint8 Fr_CtrlIdx, uint8 Fr_AbsTimerIdx);
    Std_ReturnType (*Fr_DisableAbsoluteTimerIRQ)(uint8 Fr_CtrlIdx, uint8 Fr_AbsTimerIdx);
    Std_ReturnType (*Fr_CancelAbsoluteTimer)(uint8 Fr_CtrlIdx, uint8 Fr_AbsTimerIdx);
    Std_ReturnType (*Fr_GetAbsoluteTimerIRQStatus)(uint8 Fr_CtrlIdx, uint8 Fr_AbsTimerIdx,boolean* FrIf_IRQStatusPtr);

    /* Other */
    Std_ReturnType (*Fr_GetGlobalTime)(uint8 Fr_CtrlIdx, uint8* Fr_CyclePtr, uint16* Fr_MacroTickPtr);
    Std_ReturnType (*Fr_SetWakeupChannel)(uint8 Fr_CtrlIdx, Fr_ChannelType Fr_ChnlIdx);
    Std_ReturnType (*Fr_GetPOCStatus)(uint8 Fr_CtrlIdx, Fr_POCStatusType *Fr_POCStatusPtr);

    /* Halt communication */
    Std_ReturnType (*Fr_HaltCommunication)(uint8 Fr_CtrlIdx);
    /* Abort communication */
    Std_ReturnType (*Fr_AbortCommunication)(uint8 Fr_CtrlIdx);
    /* Send Wake Up */
    Std_ReturnType (*Fr_SendWUP)(uint8 Fr_CtrlIdx);
} FrIf_FrAPIType;

/** Driver config */
typedef struct {
    const FrIf_FrAPIType *FrIf_FrAPIPtr;        /* this driver's api*/
    uint8 FrIf_CCPerDriver;                      /* No. of CCs handled by a driver */
} FrIf_DriverConfigType;


typedef struct {
    const FrIf_ClusterType * FrIf_ClusterPtr;				/* Reference to cluster cfg */
    const FrIf_ControllerType * FrIf_CtrlPtr;				/* Reference to controller cfg */
    const FrIf_DriverConfigType * Fr_Driver_ConfigPtr;		/* Fr driver cfg */
    const FrIf_TxPduType * FrIf_TxPduCfgPtr;					/* Reference to Tx Pdu cfg */
    const FrIf_RxPduType * FrIf_RxPduCfgPtr;					/* Reference to Rx Pdu cfg */
    const FrIf_RxIndicationType * FrIf_RxIndicationFncs;	/* Reference to Cfg for Rx indication functions */
    const FrIf_TriggerTransmitType * FrIf_TriggerTransmitFncs; /* Reference to Cfg for Trigger Tx functions */
    const FrIf_TxConfirmationType * FrIf_TxConfirmationFncs; /* Reference to Cfg of Tx confirmation functions */
    const FrIf_ImmediateTxPduCfgType * FrIf_ImmediateTxPduCfgPtr; /* Reference to immediate Tx cfg list */
    uint8 FrIf_ClusterCount;	/* No. of clusters */
    uint8 FrIf_CtrlCount;   	/* No. of controllers */
    uint8 FrIf_TxPduCount; 		/* No. of Tx Pdus */
    uint8 FrIf_RxPduCount; 		/* No. of Rx Pdus */

} FrIf_ConfigType;

#endif /* FRIF_TYPES_H_ */
