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

/** @addtogroup CanIf CAN Interface
 *  @{ */

/** @file CanIf_ConfigTypes.h
 *  Definitions of configuration parameters for CAN Interface.
 */

#ifndef CANIF_CONFIGTYPES_H_
#define CANIF_CONFIGTYPES_H_

#include "EcuM_Types.h"



#define NO_UPPER_LAYER_PDU 0xFFFFu

/** Type of the upper layer interfacing this module */
typedef enum {
    CANIF_USER_TYPE_CAN_NM,
    CANIF_USER_TYPE_CAN_TP,
    CANIF_USER_TYPE_CAN_PDUR,
    CANIF_USER_TYPE_J1939TP,
    CANIF_USER_TYPE_CAN_SPECIAL
} CanIf_UserTypeType;

/** Defines if PDU Can id can be changed at runtime. */
typedef enum {
  CANIF_PDU_TYPE_STATIC = 0,
  CANIF_PDU_TYPE_DYNAMIC       /**< Not supported */
} CanIf_PduTypeType;

/** PDU Can id type */
typedef enum {
  CANIF_CAN_ID_TYPE_29 = 0,
  CANIF_CAN_FD_ID_TYPE_29,
  CANIF_CAN_ID_TYPE_11,
  CANIF_CAN_FD_ID_TYPE_11
} CanIf_CanIdTypeType;

/** Defining if the CanIf PDU is referring a basic or full CAN Hardware object */
typedef enum {
    CANIF_HANDLE_TYPE_BASIC,
    CANIF_HANDLE_TYPE_FULL
} CanIf_HohType;


//-------------------------------------------------------------------
/*
 * CanIfHrhRangeConfig container
 */

/** Parameters for configuring Can id ranges. Not supported. */
typedef struct {
    /** Lower CAN Identifier of a receive CAN L-PDU for identifier range
     *  definition, in which all CAN Ids shall pass the software filtering. Range: 11
     *  Bit for Standard CAN Identifier 29 Bit for Extended CAN Identifer */
    uint32 CanIfRxPduLowerCanId;

    /** Upper CAN Identifier of a receive CAN L-PDU for identifier range
     *  definition, in which all CAN Ids shall pass the software filtering. Range: 11
     *  Bit for Standard CAN Identifier 29 Bit for Extended CAN Identifer */
    uint32 CanIfRxPduUpperCanId;
} CanIf_HrhRangeConfigType;

//-------------------------------------------------------------------
/*
 * CanIfRxPduConfig container
 */


/** Definition of Rx PDU (Protocol Data Unit). */
typedef struct {

  /** CAN Identifier of Receive CAN L-PDUs used by the CAN Interface. These are used
   *  both for single ids and for ranges.
   *  Exa: Software Filtering. Range: 11 Bit For Standard CAN Identifier ... 29 Bit For
   *  Extended CAN identifier */
    uint32      CanIfCanRxPduLowerCanId;
    uint32      CanIfCanRxPduUpperCanId;
    /** Index into CanIfUserCddIndidcations specifying indication services to target
     *  upper layers (PduRouter, CanNm, CanTp and ComplexDeviceDrivers). If parameter
     *  is NO_FUNCTION_CALLOUT no call-out function is configured. */
    uint32 CanIfUserRxIndication;

    /** ECU wide unique, symbolic handle for receive CAN L-PDU. The
     *  CanRxPduId is configurable at pre-compile and post-built time. It shall fulfill
     *  ANSI/AUTOSAR definitions for constant defines. Range: 0..max. number
     *  of defined CanRxPduIds */
    PduIdType CanIfCanRxPduId;

  /** Data Length code of received CAN L-PDUs used by the CAN Interface.
   *  Exa: DLC check. The data area size of a CAN L-PDU can have a range
   *  from 0 to 8 bytes.    uint8       CanIfCanRxPduDlc; */
    uint8       CanIfCanRxPduDlc;

#if ( CANIF_CANPDUID_READDATA_API == STD_ON )
  /** Enables and disables the Rx buffering for reading of received L-PDU data.
   *  True: Enabled False: Disabled */
    boolean     CanIfReadRxPduData;
#endif

#if ( CANIF_READRXPDU_NOTIF_STATUS_API == STD_ON )
  /** CanIfReadRxPduNotifyStatus {CANIF_READRXPDU_NOTIFY_STATUS}
   *  Enables and disables receive indication for each receive CAN L-PDU for
   *  reading its' notification status. True: Enabled False: Disabled */
    boolean     CanIfReadRxPduNotifyStatus;
#endif

    /* CanId Extended or Standard */
    boolean CanIdIsExtended;

#if (CANIF_OSEKNM_SUPPORT == STD_ON)
    /* Check if an indication to OsekNm is required */
    boolean OsekNmRxIndicationSupport;
#endif



} CanIf_RxPduConfigType;
//-------------------------------------------------------------------
/*
 * CanIfInitHrhConfig container
 */
/** Definition of Hardware Receive Handle */
typedef struct {
    /** Defines the HRH type i.e, whether its a BasicCan or FullCan. If BasicCan is
     *  configured, software filtering is enabled. */
    CanIf_HohType CanIfHrhType;

    /** Selects the hardware receive objects by using the HRH range/list from
     *  CAN Driver configuration to define, for which HRH a software filtering has
     *  to be performed at during receive processing. True: Software filtering is
     *  enabled False: Software filtering is disabled */
    boolean  CanIfSoftwareFilterHrh;

    /** Reference to controller Id to which the HRH belongs to. A controller can
     *  contain one or more HRHs. */
    CanIf_Arc_ChannelIdType CanIfHrhCanCtrlIdRef;

    /** Defines the parameters required for configuring multiple
     *  CANID ranges for a given same HRH. */
    const CanIf_HrhRangeConfigType *CanIfHrhRangeConfig;
    const CanIf_RxPduConfigType* RxPduList;
    uint16 NofRxPdus;
} CanIf_HrhConfigType;

//-------------------------------------------------------------------
/*
 * CanIfInitHthConfig container
 */
/** Definition of Hardware Transmit Handle */
typedef struct {
  /** Defines the HTH type i.e, whether its a BasicCan or FullCan. */
  CanIf_HohType CanIfHthType;

  /** Reference to controller Id to which the HTH belongs to. A controller
   *  can contain one or more HTHs */
  CanIf_Arc_ChannelIdType CanIfCanControllerIdRef;

  /** The parameter refers to a particular HTH object in the CAN Driver Module
   *  configuration. The HTH id is unique in a given CAN Driver. The HTH Ids
   *  are defined in the CAN Driver Module and hence it is derived from CAN
   *  Driver Configuration. */
  Can_HwHandleType CanIfHthIdSymRef ;

} CanIf_HthConfigType;

//-------------------------------------------------------------------
/*
 * CanIfInitHohConfig container
 */
/** Definition of Hardware Object Handle. */
typedef struct {

  /** This container contains contiguration parameters for each hardware receive object. */
  const CanIf_HrhConfigType *CanIfHrhConfig;

  /** This container contains parameters releated to each HTH */
  const CanIf_HthConfigType *CanIfHthConfig;

  const uint16 *CanHohToCanIfHrhMap;
  uint32 HrhListSize;
} CanIf_InitHohConfigType;

//-------------------------------------------------------------------
/*
 * CanIfTxBuffer container
 */
/** Definition of Tx Buffer. */
typedef struct {
    const CanIf_HthConfigType *CanIfBufferHthRef;
    uint16 CanIfBufferSize;
    uint8 CanIf_Arc_BufferId;
} CanIf_TxBufferConfigType;

//-------------------------------------------------------------------
/*
 * CanIfTxPduConfig container
 */

typedef void (*CanIfUserTxConfirmationType)(PduIdType);
typedef void (*CanIfUserRxIndicationType)(PduIdType , PduInfoType*);
typedef void (*CanIfBusOffNotificationType)(uint8 Controller);
typedef void (*CanIfWakeUpNotificationType)();
typedef void (*CanIfWakeupValidNotificationType)();
typedef void (*CanIfControllerModeIndicationType)(uint8 ControllerId, CanIf_ControllerModeType ControllerMode);
#if !defined(CFG_MCAL_EXTERNAL)
typedef void (*CanIfErrorNotificatonType)(uint8,Can_Arc_ErrorType);
#endif


#define NO_FUNCTION_CALLOUT 0xFFFFFFFF

/** Definition of Tx PDU (Protocol Data Unit). */
typedef struct {

    /** Reference to buffer which defines the hardware object or the pool of hardware objects
     *  configured for transmission. The buffer refers HTH Id, to which the L-
     *  PDU belongs to. */
    const CanIf_TxBufferConfigType *CanIfTxPduBufferRef;

    /** Index into CanIfUserCddConfirmations specifying indication services to target
     *  upper layers (PduRouter, CanNm, CanTp and ComplexDeviceDrivers). If parameter
     *  is NO_FUNCTION_CALLOUT no call-out function is configured. */
    uint32 CanIfUserTxConfirmation;

    /** CAN Identifier of transmit CAN L-PDUs used by the CAN Driver for CAN L-
     *  PDU transmission. Range: 11 Bit For Standard CAN Identifier ... 29 Bit For
     *  Extended CAN identifier */
    uint32  CanIfCanTxPduIdCanId;

    /** ECU wide unique, symbolic handle for transmit CAN L-PDU. The
     *  CanIfCanTxPduId is configurable at pre-compile and post-built time.
     *  Range: 0..max. number of CantTxPduIds   PduIdType   CanTxPduId; */
    PduIdType CanIfTxPduId;

    /** CAN Identifier of transmit CAN L-PDUs used by the CAN Driver for CAN L-
     *  PDU transmission.
     *  EXTENDED_CAN  The CANID is of type Extended (29 bits).
     *  STANDARD_CAN  The CANID is of type Standard (11 bits). */
    CanIf_CanIdTypeType		CanIfTxPduIdCanIdType;

    /** Defines the type of each transmit CAN L-PDU.
     *  DYNAMIC  CAN ID is defined at runtime.
     *  STATIC  CAN ID is defined at compile-time. */
    CanIf_PduTypeType       CanIfCanTxPduType;


    /** Data length code (in bytes) of transmit CAN L-PDUs used by the CAN
     *  Driver for CAN L-PDU transmission. The data area size of a CAN L-Pdu
     *  can have a range from 0 to 8 bytes. */
    uint8       CanIfCanTxPduIdDlc;

#if ( CANIF_READTXPDU_NOTIFY_STATUS_API == STD_ON )
    /** Enables and disables transmit confirmation for each transmit CAN L-PDU
     *  for reading its notification status. True: Enabled False: Disabled */
    boolean     CanIfReadTxPduNotifyStatus;
#endif

    /** PDU that is allowed by PN filter */
    boolean CanIfTxPduPnFilterEnable;

#if (CANIF_OSEKNM_SUPPORT == STD_ON)
    /* OsekNm Tx confirmation required */
    boolean OsekNmTxConfirmationSupport;
#endif
} CanIf_TxPduConfigType;



//-------------------------------------------------------------------
/*
 * CanIf_TransceiverChannelConfig container
 */
typedef struct {
    /* Wake up support indication */
    boolean CanIfTrcvWakeupSupport;
    /* Reference to underlying Can Transceiver unit */
    uint8  CanIfTrcvCanTrcvIdRef;
    /* Symbolic name for CanIf transceiver channel */
    uint8  CanIfTrcvId;
    /* Wakeup Source for the relevant channel if available*/
    EcuM_WakeupSourceType CanIfTrcvCanTrcvWakeupSrc;
    /* Reference to related CanIfCtrl channel */
    uint8 CanIfTrcvChnlToCanIfCtrlChnlRef;
} CanIf_TransceiverChannelConfigType;


/** Callout functions with respect to the upper layers. This callout functions
 *  defined in this container are common to all configured underlying CAN
 *  Drivers / CAN Transceiver Drivers. */
typedef struct {
    /** Name of target BusOff notification services to target upper layers
     *  (PduRouter, CanNm, CanTp and ComplexDeviceDrivers). */
    void (*CanIfBusOffNotification)(uint8 Controller);

    /** Name of target wakeup notification services to target upper layers
     *  e.g Ecu_StateManager. If parameter is 0
     *  no call-out function is configured. */
    void (*CanIfWakeUpNotification)();

    /** Name of target wakeup validation notification services to target upper
     *  layers (ECU State Manager). If parameter is 0 no call-out function is
     *  configured. */
    void (*CanIfWakeupValidNotification)();

    /** Name of target controller mode indication services to target upper
     *  layers (CanSm). If parameter is 0 no call-out function is
     *  configured. */
    void (*CanIfControllerModeIndication)(uint8 ControllerId, CanIf_ControllerModeType ControllerMode);

    /** ArcCore ext. */
#if !defined(CFG_MCAL_EXTERNAL)
    void (*CanIfErrorNotificaton)(uint8,Can_Arc_ErrorType);
#endif

#if defined (USE_CANTRCV)
    /** Notification when Transceiver wake up flag is set  */
    void (*CanIfTrcvWakeFlagNotification)(uint8 Transceiver);

    /** Notification when Transceiver wake up flag is cleared */
    void (*CanIfTrcvClearWakeFlagNotification)(uint8 Transceiver);

    /** Notification when PN communication mode is available */
    void (*CanIfTrcvConfirmPnAvailabilityNotification)(uint8 TransceiverId);

    /** Notification when transceiver mode changes */
    void(*CanIfTrcvModeChangeNotification)( uint8 TransceiverId, CanTrcv_TrcvModeType TransceiverMode );
#endif /* USE_CANTRCV */

} CanIf_DispatchConfigType;

/** This container contains the references to the configuration setup of each
 *  underlying CAN driver. */
typedef struct {
    /** Not used. */
    uint32 CanIfConfigSet;

    /** Size of Rx PDU list. */
    uint32 CanIfNumberOfCanRxPduIds;
    /** Size of Tx PDU list. */
    uint32 CanIfNumberOfCanTXPduIds;
    /** Not used */
    uint32 CanIfNumberOfDynamicCanTXPduIds;

    uint16 CanIfNumberOfTxBuffers;

    //
    // Containers
    //
    /* Tx Buffer List */
    const CanIf_TxBufferConfigType *CanIfBufferCfgPtr;

  /** Hardware Object Handle list */
    const CanIf_InitHohConfigType *CanIfHohConfigPtr;

  /** Rx PDU's list */
    const CanIf_RxPduConfigType *CanIfRxPduConfigPtr;

  /** Tx PDU's list */
#if (CANIF_ARC_RUNTIME_PDU_CONFIGURATION == STD_OFF)
    const CanIf_TxPduConfigType *CanIfTxPduConfigPtr;
#else
    CanIf_TxPduConfigType *CanIfTxPduConfigPtr;
#endif

} CanIf_InitConfigType;

/*
 *
 */
/** CanIf channel configuration */
typedef struct {
    const CanIf_TxBufferConfigType* const *TxBufferRefList;
    const CanIf_RxPduConfigType* RxPduList;
    uint16 NofRxPdus;
    EcuM_WakeupSourceType CanIfCtrlWakeUpSrc;
    uint8 CanControllerId;
    uint8 NofTxBuffers;
#if (CANIF_OSEKNM_SUPPORT == STD_ON)
    uint8 CanIfOsekNmNetId; /* OsekNm network identifier */
    uint8 CanIfOsekNmNodeIdMask; /* OsekNM source node Id mask */
#endif
    boolean CanIfCtrlWakeUpSupport;
    /** Whether PN filter is  required for the controller */
    boolean CanIfCtrlPnFilterSet;
} CanIf_Arc_ChannelConfigType;

/** Top level config container. */
/* !req 4.0.3/CANIF523 */
typedef struct {

    /** This container contains the init parameters of the CAN Interface. */
    const CanIf_InitConfigType 		*InitConfig;

    /** Reference to tranceiver channel list */
    const CanIf_TransceiverChannelConfigType *CanIfTransceiverConfig;

    /** ArcCore: Contains channel config (including the mapping from CanIf-specific Channels to Can Controllers) */
    const CanIf_Arc_ChannelConfigType *Arc_ChannelConfig;

} CanIf_ConfigType;


#endif /* CANIF_CONFIGTYPES_H_ */
/** @} */

