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

/** @addtogroup Can CAN Driver
 *  @{ */

/** @file Can_Cfg.h
 * Definitions of configuration parameters for CAN Driver.
 */

#ifndef CAN_CFG_H_
#define CAN_CFG_H_

/** Number of controller configs. */
#define CAN_ARC_CTRL_CONFIG_CNT   2

/** Enable Development Error Trace. */
#define CAN_DEV_ERROR_DETECT          STD_ON
/** Build version info API. */
#define CAN_VERSION_INFO_API          STD_ON
/** Not supported. */
#define CAN_MULTIPLEXED_TRANSMISSION  STD_OFF
/** Not supported. */
#define CAN_WAKEUP_SUPPORT            STD_OFF
/** Not supported. */
#define CAN_HW_TRANSMIT_CANCELLATION   STD_OFF
/** Not supported. */
#define CAN_TIMEOUT_DURATION          100

#define INTERRUPT               0
#define POLLING                 1

// Can controller
/** Bus off handling. Polling not supported */
#define CAN_BUSOFF_PROCESSING         INTERRUPT  // INTERRUPT/POLLING
/** Not supported. */
#define CAN_CONTROLLER_ACTIVATION     OFF
/** Not used. @see Can_ControllerConfigType */
#define CAN_CONTROLLER_BAUD_RATE      125000
/** Not used. @see CanControllerIdType */
#define CAN_DRIVER_CONTROLLER_ID      0
/** Not used. @see Can_ControllerConfigType */
#define CAN_CONTROLLER_PROP_SEG       4
/** Not used. @see Can_ControllerConfigType */
#define CAN_CONTROLLER_PHASE1_SEG     4
/** Not used. @see Can_ControllerConfigType */
#define CAN_CONTROLLER_PHASE2_SEG     4
/** Not used. @see Can_ControllerConfigType */
#define CAN_CONTROLLER_TIME_QUANTA    4
/** Rx handling. Polling not supported. */
#define CAN_RX_PROCESSING             INTERRUPT
/** Tx handling. Polling not supported. */
#define CAN_TX_PROCESSING             INTERRUPT
/** Wakeup handling. Polling not supported. */
#define CAN_WAKEUP_PROCESSING         INTERRUPT

/** Available HW controllers. */
typedef enum {
  CAN_CTRL_A = 0,
  CAN_CTRL_B,
  CAN_CTRL_C,
  CAN_CTRL_D,
  CAN_CTRL_E,
  CAN_CTRL_F,
  CAN_CONTROLLER_CNT  
}CanControllerIdType;

/** CAN id types. */
typedef enum {
  CAN_ID_TYPE_EXTENDED,
  CAN_ID_TYPE_MIXED,
  CAN_ID_TYPE_STANDARD,
} Can_IdTypeType;

/** CAN HW object types. */
typedef enum {
  CAN_OBJECT_TYPE_RECEIVE,
  CAN_OBJECT_TYPE_TRANSMIT,
} Can_ObjectTypeType;

/** HW object Can type. Full not supported. */
typedef enum {
  CAN_ARC_HANDLE_TYPE_BASIC,
  CAN_ARC_HANDLE_TYPE_FULL
} Can_Arc_HohType;

// HTH definitions
// Due to effiency: Start with index 0 and don't use any holes in the enumeration
/** Transmit object id:s */
typedef enum {
  CAN_HTH_A_1 = 0,
  CAN_HTH_C_1,
  NUM_OF_HTHS
} Can_Arc_HTHType;

// HRH definitions
// Due to effiency: Start with index 0 and don't use any holes in the enumeration
/** Receive object id:s */
typedef enum {
  CAN_HRH_A_1 = 0,
  CAN_HRH_C_1,
  NUM_OF_HRHS
} Can_Arc_HRHType;

// Non-standard type
/** Container for callback configuration. */
typedef struct {
  void (*CancelTxConfirmation)( const Can_PduType *);               /**< Not supported. */
  void (*RxIndication)( uint8 ,Can_IdType ,uint8 , const uint8 * ); /**< Called on successful reception of a PDU. */
  void (*ControllerBusOff)(uint8);                                  /**< Called on BusOff. */
  void (*TxConfirmation)(PduIdType);                                /**< Called on successful transmission of a PDU. */
  void (*ControllerWakeup)(uint8);                                  /**< Not supported. */
  void (*Arc_Error)(uint8,Can_Arc_ErrorType);                       /**< Called on HW error. */
} Can_CallbackType;

/*
 * CanGeneral Container
 */

/** Container for parameters related to the CAN Driver. */
typedef struct {
  /** Specifies the InstanceId of this module instance. If only one instance is
   *  present it shall have the Id 0 */
  int CanIndex;

#if 0 // This is only used by the config tool
  //  This parameter describes the period for cyclic call to
  //  Can_MainFunction_Busoff. Unit is seconds.
  float CanMainFunctionBusoffPeriod;
  //  This parameter describes the period for cyclic call to
  //  Can_MainFunction_Read. Unit is seconds.
  float CanMainFunctionReadPeriod;
  // This parameter describes the period for cyclic call to
  // Can_MainFunction_Wakeup. Unit is seconds.
  float CanMainFunctionWakeupPeriod;
  //  This parameter describes the period for cyclic call to
  //  Can_MainFunction_Write. Unit is seconds.
  float CanMainFunctionWritePeriod;
#endif


#if ( CAN_TIMEOUT_DURATION == STD_ON )
  //  Specifies the maximum number of loops for blocking function until a
  //  timeout is raised in short term wait loops.
  uint32 CanTimeoutDurationFactor;
#endif

} Can_GeneralType;


/** Type for Can id filter mask */
typedef uint32 Can_FilterMaskType;

/*
 * CanHardwareObject container
 */

/** Container for CAN Hardware Object parameters */
typedef struct Can_HardwareObjectStruct {
  /** Specifies the type (Full-CAN or Basic-CAN) of a hardware object. */
  Can_Arc_HohType CanHandleType;

  /** Specifies whether the IdValue is of type - standard identifier - extended
   *  identifier - mixed mode */
  Can_IdTypeType CanIdType;

  /** Specifies (together with the filter mask) the identifiers range that passes
   *  the hardware filter. */
  uint32 CanIdValue;

  /** Holds the handle ID of HRH or HTH. The value of this parameter is unique
   *  in a given CAN Driver, and it should start with 0 and continue without any
   *  gaps. The HRH and HTH Ids are defined under two different name-spaces.
   *  Example: HRH0-0, HRH1-1, HTH0-2, HTH1-3 */
  uint32 CanObjectId;

  /** Specifies if the HardwareObject is used as Transmit or as Receive object */
  Can_ObjectTypeType CanObjectType;

  /** Reference to the filter mask that is used for hardware filtering togerther
   *  with the CAN_ID_VALUE */
  Can_FilterMaskType *CanFilterMaskRef;

  /** A "1" in this mask tells the driver that that HW Message Box should be
   *  occupied by this Hoh. A "1" in bit 31(ppc) occupies Mb 0 in HW. */
  uint32 Can_Arc_MbMask;

  /** End Of List. Set to TRUE if this is the last object in the list. */
  boolean Can_Arc_EOL;

} Can_HardwareObjectType;


/*
 * CanController container
 */
typedef enum {
  CAN_ARC_PROCESS_TYPE_INTERRUPT,
  CAN_ARC_PROCESS_TYPE_POLLING,
} Can_Arc_ProcessType;

/** Container for configuration of a controller. */
typedef struct {

  /** Enables / disables API Can_MainFunction_BusOff() for handling busoff
   *  events in polling mode. Polling not supported. */
  Can_Arc_ProcessType CanBusOffProcessing;

  /** Defines if a CAN controller is used in the configuration. */
  boolean CanControllerActivation;

  /** Specifies the buadrate of the controller in kbps. */
  uint32 CanControllerBaudRate;

  /** This parameter provides the controller ID which is unique in a given CAN
   *  Driver. The value for this parameter starts with 0 and continue without any
   *  gaps. */
  CanControllerIdType CanControllerId;

  /** Specifies propagation delay in time quantas. */
  uint32 CanControllerPropSeg;

  /** Specifies phase segment 1 in time quantas. */
  uint32 CanControllerSeg1;

  /** Specifies phase segment 2 in time quantas. */
  uint32 CanControllerSeg2;

  /** Specifies the time quanta for the controller. The calculation of the resulting
   *  prescaler value depending on module clocking and time quanta shall be
   *  done offline Hardware specific. */
  uint32 CanControllerTimeQuanta;

  /** Enables / disables API Can_MainFunction_Read() for handling PDU
   *  reception events in polling mode. Polling not supported. */
  Can_Arc_ProcessType CanRxProcessing;

  /** Enables / disables API Can_MainFunction_Write() for handling PDU
   *  transmission events in polling mode. Polling not supported. */
  Can_Arc_ProcessType CanTxProcessing;

  /** Enables / disables API Can_MainFunction_Wakeup() for handling wakeup
   *  events in polling mode. Polling not supported. */
  Can_Arc_ProcessType CanWakeupProcessing;

  /** Reference to the CPU clock configuration, which is set in the MCU driver
   *  configuration */
  uint32 CanCpuClockRef;

  /** This parameter contains a reference to the Wakeup Source for this
   *  controller as defined in the ECU State Manager. Implementation Type:
   *  reference to EcuM_WakeupSourceType. Not supported. */
  uint32 CanWakeupSourceRef;

  //
  // ArcCore stuff
  //

  /** List of Hardware Object id's that belong to this controller. */
  const Can_HardwareObjectType  *Can_Arc_Hoh;

  /** Enable controller self reception. */
  boolean Can_Arc_Loopback;

  /** Set this to use the fifo */
  boolean Can_Arc_Fifo;

} Can_ControllerConfigType;



/** Container for controller parameters. */
typedef struct {
  const Can_ControllerConfigType *CanController;
  
  // Callbacks( Extension )
  const Can_CallbackType *CanCallbacks;
} Can_ConfigSetType;

/** Top level container for parameters. */
typedef struct {
  /** Controller parameters. */
  const Can_ConfigSetType   *CanConfigSet;
  /** Driver parameters. */
  const Can_GeneralType   *CanGeneral;

} Can_ConfigType;

/** Top level container for parameters. */
extern const Can_ConfigType CanConfigData;
/** For direct access to controller list */
extern const Can_ControllerConfigType CanControllerConfigData[];
/** Container for controller parameters. */
extern const Can_ConfigSetType Can_ConfigSet;


#endif /*CAN_CFG_H_*/
/** @} */
