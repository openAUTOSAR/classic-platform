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








#warning "This default file may only be used as an example!"

#ifndef CAN_CFG_H_
#define CAN_CFG_H_

// Number of controller configs
#define CAN_ARC_CTRL_CONFIG_CNT   2

#define CAN_DEV_ERROR_DETECT          STD_ON
#define CAN_VERSION_INFO_API          STD_ON
#define CAN_MULTIPLEXED_TRANSMISSION  STD_ON    // Makes no differens in the code
#define CAN_WAKEUP_SUPPORT            STD_OFF   // Not supported
#define CAN_HW_TRANSMIT_CANCELLATION   STD_OFF  // Not supported

// loop cnt.. very strange timeout
#define CAN_TIMEOUT_DURATION          100

#define INTERRUPT               0
#define POLLING                 1

// Can controller
#define CAN_BUSOFF_PROCESSING         INTERRUPT  // INTERRUPT/POLLING
#define CAN_CONTROLLER_ACTIVATION     OFF
#define CAN_CONTROLLER_BAUD_RATE      125000
#define CAN_DRIVER_CONTROLLER_ID      0
#define CAN_CONTROLLER_PROP_SEG       4
#define CAN_CONTROLLER_PHASE1_SEG     4
#define CAN_CONTROLLER_PHASE2_SEG     4
#define CAN_CONTROLLER_TIME_QUANTA    4
#define CAN_RX_PROCESSING             INTERRUPT
#define CAN_TX_PROCESSING             INTERRUPT
#define CAN_WAKEUP_PROCESSING         INTERRUPT

typedef enum {
  CAN_CTRL_A = 0,
  CAN_CTRL_B,
  CAN_CTRL_C,
  CAN_CTRL_D,
  CAN_CTRL_E,
  CAN_CTRL_F,
  CAN_CONTROLLER_CNT  
}CanControllerIdType;

typedef enum {
  CAN_ID_TYPE_EXTENDED,
  CAN_ID_TYPE_MIXED,
  CAN_ID_TYPE_STANDARD,
} Can_IdTypeType;

typedef enum {
  CAN_OBJECT_TYPE_RECEIVE,
  CAN_OBJECT_TYPE_TRANSMIT,
} Can_ObjectTypeType;

typedef enum {
  CAN_ARC_HANDLE_TYPE_BASIC,
  CAN_ARC_HANDLE_TYPE_FULL
} Can_Arc_HohType;

// HTH definitions
// Due to effiency: Start with index 0 and don't use any holes in the enumeration
typedef enum {
  CAN_HTH_A_1 = 0,
  CAN_HTH_C_1,
  NUM_OF_HTHS
} Can_Arc_HTHType;

// HRH definitions
// Due to effiency: Start with index 0 and don't use any holes in the enumeration
typedef enum {
  CAN_HRH_A_1 = 0,
  CAN_HRH_C_1,
  NUM_OF_HRHS
} Can_Arc_HRHType;

// Non-standard type
typedef struct {
  void (*CancelTxConfirmation)( const Can_PduType *);
  void (*RxIndication)( uint8 ,Can_IdType ,uint8 , const uint8 * );
  void (*ControllerBusOff)(uint8);
  void (*TxConfirmation)(PduIdType);
  void (*ControllerWakeup)(uint8);
  void (*Arc_Error)(uint8,Can_Arc_ErrorType);
} Can_CallbackType;

/*
 * CanGeneral Container
 */

// This container contains the parameters related each CAN Driver Unit.
typedef struct {
  //  Specifies the InstanceId of this module instance. If only one instance is
  //  present it shall have the Id 0
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


/*
 * CanFilterMask container
 */
typedef uint32 Can_FilterMaskType;

/*
 * CanHardwareObject container
 */

//This container contains the configuration (parameters) of CAN Hardware
//Objects.
typedef struct Can_HardwareObjectStruct {
  // Specifies the type (Full-CAN or Basic-CAN) of a hardware object.
  Can_Arc_HohType CanHandleType;

  // Specifies whether the IdValue is of type - standard identifier - extended
  // identifier - mixed mode ImplementationType: Can_IdType
  Can_IdTypeType CanIdType;

  //  Specifies (together with the filter mask) the identifiers range that passes
  //  the hardware filter.
  uint32 CanIdValue;

  //  Holds the handle ID of HRH or HTH. The value of this parameter is unique
  //  in a given CAN Driver, and it should start with 0 and continue without any
  //  gaps. The HRH and HTH Ids are defined under two different name-spaces.
  //  Example: HRH0-0, HRH1-1, HTH0-2, HTH1-3
  uint32 CanObjectId;

  // Specifies if the HardwareObject is used as Transmit or as Receive object
  Can_ObjectTypeType CanObjectType;

  // Reference to the filter mask that is used for hardware filtering togerther
  // with the CAN_ID_VALUE
  Can_FilterMaskType *CanFilterMaskRef;

  // A "1" in this mask tells the driver that that HW Message Box should be
  // occupied by this Hoh. A "1" in bit 31(ppc) occupies Mb 0 in HW.
  uint32 Can_Arc_MbMask;

  // End Of List. Set to TRUE is this is the last object in the list.
  boolean Can_Arc_EOL;

} Can_HardwareObjectType;


/*
 * CanController container
 */
typedef enum {
  CAN_ARC_PROCESS_TYPE_INTERRUPT,
  CAN_ARC_PROCESS_TYPE_POLLING,
} Can_Arc_ProcessType;

typedef struct {

  //  Enables / disables API Can_MainFunction_BusOff() for handling busoff
  //  events in polling mode.
  // INTERRUPT or POLLING
  Can_Arc_ProcessType CanBusOffProcessing;

  // Defines if a CAN controller is used in the configuration.
  boolean CanControllerActivation;

  // Specifies the buadrate of the controller in kbps.
  uint32 CanControllerBaudRate;

  //  This parameter provides the controller ID which is unique in a given CAN
  //  Driver. The value for this parameter starts with 0 and continue without any
  //  gaps.
  CanControllerIdType CanControllerId;

  // Specifies propagation delay in time quantas.
  uint32 CanControllerPropSeg;

  // Specifies phase segment 1 in time quantas.
  uint32 CanControllerSeg1;

  // Specifies phase segment 2 in time quantas.
  uint32 CanControllerSeg2;

  //  Specifies the time quanta for the controller. The calculation of the resulting
  //  prescaler value depending on module clocking and time quanta shall be
  //  done offline Hardware specific.
  uint32 CanControllerTimeQuanta;

  //  Enables / disables API Can_MainFunction_Read() for handling PDU
  //  reception events in polling mode.
  Can_Arc_ProcessType CanRxProcessing;

  //  Enables / disables API Can_MainFunction_Write() for handling PDU
  //  transmission events in polling mode.
  Can_Arc_ProcessType CanTxProcessing;

  //  Enables / disables API Can_MainFunction_Wakeup() for handling wakeup
  //  events in polling mode.
  Can_Arc_ProcessType CanWakeupProcessing;

  //  Reference to the CPU clock configuration, which is set in the MCU driver
  //  configuration
  uint32 CanCpuClockRef;

  //  This parameter contains a reference to the Wakeup Source for this
  //  controller as defined in the ECU State Manager. Implementation Type:
  //  reference to EcuM_WakeupSourceType
  uint32 CanWakeupSourceRef;

  //
  // ArcCore stuff
  //

  // List of Hoh id's that belong to this controller
  const Can_HardwareObjectType  *Can_Arc_Hoh;

  boolean Can_Arc_Loopback;

  // Set this to use the fifo
  boolean Can_Arc_Fifo;

} Can_ControllerConfigType;



/*
 * CanConfigSet container
 */
typedef struct {
  const Can_ControllerConfigType *CanController;
  
  // Callbacks( Extension )
  const Can_CallbackType *CanCallbacks;
} Can_ConfigSetType;


typedef struct {
  // This is the multiple configuration set container for CAN Driver
  // Multiplicity 1..*
  const Can_ConfigSetType   *CanConfigSet;
  // This container contains the parameters related each CAN
  // Driver Unit.
  // Multiplicity 1..*
  const Can_GeneralType   *CanGeneral;


} Can_ConfigType;


extern const Can_ConfigType CanConfigData;
extern const Can_ControllerConfigType CanControllerConfigData[];
extern const Can_ConfigSetType Can_ConfigSet;


#endif /*CAN_CFG_H_*/
