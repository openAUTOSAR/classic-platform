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

#ifndef CANIF_CFG_H_
#define CANIF_CFG_H_

#include "Can.h"

// Identifiers for the elements in CanIfControllerConfig[]
// This is the ConfigurationIndex in CanIf_InitController()
typedef enum {
	CANIF_CHANNEL_0_CONFIG_0 = 0,

	CANIF_CHANNEL_1_CONFIG_0 = 1,

	CANIF_CHANNEL_CONFIGURATION_CNT
} CanIf_Arc_ConfigurationIndexType;

typedef enum {
	CANIF_CHANNEL_0,
	CANIF_CHANNEL_1,
	CANIF_CHANNEL_CNT,
} CanIf_Arc_ChannelIdType;

typedef enum {
	CANIF_SOFTFILTER_TYPE_BINARY = 0,  // Not supported
	CANIF_SOFTFILTER_TYPE_INDEX,  // Not supported
	CANIF_SOFTFILTER_TYPE_LINEAR, // Not supported
	CANIF_SOFTFILTER_TYPE_TABLE,  // Not supported
	CANIF_SOFTFILTER_TYPE_MASK,  // CanIfCanRxPduCanIdMask in RxPduConfig is used for filtering
} CanIf_SoftwareFilterTypeType;

typedef enum {
	CANIF_USER_TYPE_CAN_NM,
	CANIF_USER_TYPE_CAN_TP,
	CANIF_USER_TYPE_CAN_PDUR,
	CANIF_USER_TYPE_CAN_SPECIAL,
} CanIf_UserTypeType;



typedef void (*CanIf_FuncTypeCanSpecial)(PduIdType, const uint8 *, uint8, Can_IdType);

typedef enum {
  CANIF_PDU_TYPE_STATIC = 0,
  CANIF_PDU_TYPE_DYNAMIC       // Not supported
} CanIf_PduTypeType;

typedef enum {
  CANIF_CAN_ID_TYPE_29 = 0,
  CANIF_CAN_ID_TYPE_11
} CanIf_CanIdTypeType;

/*
 * Public container
 */
#define CANIF_VERSION_INFO_API            STD_ON
#define CANIF_DEV_ERROR_DETECT				    STD_ON
#define CANIF_DLC_CHECK                   STD_ON
#define CANIF_MULITPLE_DRIVER_SUPPORT     STD_OFF  // Not supported
#define CANIF_READRXPDU_DATA_API			    STD_OFF  // Not supported
#define CANIF_READRXPDU_NOTIFY_STATUS_API	STD_OFF  // Not supported
#define CANIF_READTXPDU_NOTIFY_STATUS_API	STD_OFF  // Not supported
#define CANIF_SETDYNAMICTXID_API          STD_OFF  // Not supported
#define CANIF_WAKEUP_EVENT_API				    STD_OFF  // Not supported
#define CANIF_TRANSCEIVER_API             STD_OFF  // Not supported
#define CANIF_TRANSMIT_CANCELLATION       STD_OFF  // Not supported

//-------------------------------------------------------------------

typedef struct {
	void (*CancelTxConfirmation)( void *);  // (const Can_PduType *);
	void (*RxIndication)(void *); //(const Can_PduType *);
	void (*ControllerBusOff)(uint8);
	void (*ControllerWakeup)(uint8);
	void (*Arc_Error)(uint8,uint32);
} CanIf_CallbackType;



//-------------------------------------------------------------------
/*
 * CanIfHrhRangeConfig container
 */

typedef struct {
	//	Lower CAN Identifier of a receive CAN L-PDU for identifier range
	//	definition, in which all CAN Ids shall pass the software filtering. Range: 11
	//	Bit for Standard CAN Identifier 29 Bit for Extended CAN Identifer
	uint32 CanIfRxPduLowerCanId;

	//	Upper CAN Identifier of a receive CAN L-PDU for identifier range
	//	definition, in which all CAN Ids shall pass the software filtering. Range: 11
	//	Bit for Standard CAN Identifier 29 Bit for Extended CAN Identifer
	uint32 CanIfRxPduUpperCanId;
} CanIf_HrhRangeConfigType;



//-------------------------------------------------------------------
/*
 * CanIfInitHrhConfig container
 */
typedef struct {
	//	Defines the HRH type i.e, whether its a BasicCan or FullCan. If BasicCan is
	//	configured, software filtering is enabled.
  Can_Arc_HohType CanIfHrhType;

	//	Selects the hardware receive objects by using the HRH range/list from
	//	CAN Driver configuration to define, for which HRH a software filtering has
	//	to be performed at during receive processing. True: Software filtering is
	//	enabled False: Software filtering is disabled
	boolean  CanIfSoftwareFilterHrh;

	//	Reference to controller Id to which the HRH belongs to. A controller can
	//	contain one or more HRHs.
	CanIf_Arc_ChannelIdType CanIfCanControllerHrhIdRef;

	//	The parameter refers to a particular HRH object in the CAN Driver Module
	//	configuration. The HRH id is unique in a given CAN Driver. The HRH Ids
	//	are defined in the CAN Driver Module and hence it is derived from CAN
	//	Driver Configuration.
	Can_Arc_HRHType CanIfHrhIdSymRef ;

	//	Defines the parameters required for configuraing multiple
	//	CANID ranges for a given same HRH.
	const CanIf_HrhRangeConfigType *CanIfHrhRangeConfig;

  // End Of List. Set to TRUE is this is the last object in the list.
  boolean CanIf_Arc_EOL;
} CanIf_HrhConfigType;

//-------------------------------------------------------------------
/*
 * CanIfInitHthConfig container
 */

typedef struct {
  //  Defines the HTH type i.e, whether its a BasicCan or FullCan.
  Can_Arc_HohType CanIfHthType;

  // Reference to controller Id to which the HTH belongs to. A controller
  // can contain one or more HTHs
  CanIf_Arc_ChannelIdType CanIfCanControllerIdRef;

  //  The parameter refers to a particular HTH object in the CAN Driver Module
  //  configuration. The HTH id is unique in a given CAN Driver. The HTH Ids
  //  are defined in the CAN Driver Module and hence it is derived from CAN
  //  Driver Configuration.
  Can_Arc_HTHType CanIfHthIdSymRef ;

  // End Of List. Set to TRUE is this is the last object in the list.
  boolean CanIf_Arc_EOL;
} CanIf_HthConfigType;

//-------------------------------------------------------------------
/*
 * CanIfInitHohConfig container
 */
typedef struct {
  //  Selects the CAN interface specific configuration setup. This type of external
  // data structure shall contain the post build initialization data for the
  // CAN interface for all underlying CAN Drivers.
  const Can_ConfigSetType   *CanConfigSet;

  // This container contains contiguration parameters for each hardware receive object.
  const CanIf_HrhConfigType *CanIfHrhConfig;

  // This container contains parameters releated to each HTH
  const CanIf_HthConfigType *CanIfHthConfig;

  // End Of List. Set to TRUE is this is the last object in the list.
  boolean CanIf_Arc_EOL;
} CanIf_InitHohConfigType;

//-------------------------------------------------------------------
/*
 * CanIfTxPduConfig container
 */

// This container contains the configuration (parameters) of each transmit
// CAN L-PDU. The SHORT-NAME of "CanIfTxPduConfig" container
// represents the symolic name of Transmit L-PDU.
typedef struct {
	//	ECU wide unique, symbolic handle for transmit CAN L-PDU. The
	//	CanIfCanTxPduId is configurable at pre-compile and post-built time.
	//	Range: 0..max. number of CantTxPduIds  	PduIdType 	CanTxPduId;
	PduIdType CanIfTxPduId;

	//	CAN Identifier of transmit CAN L-PDUs used by the CAN Driver for CAN L-
	//	PDU transmission. Range: 11 Bit For Standard CAN Identifier ... 29 Bit For
	//	Extended CAN identifier
	uint32  CanIfCanTxPduIdCanId;

	//	Data length code (in bytes) of transmit CAN L-PDUs used by the CAN
	//	Driver for CAN L-PDU transmission. The data area size of a CAN L-Pdu
	//	can have a range from 0 to 8 bytes.
	uint8 		CanIfCanTxPduIdDlc;

	// Defines the type of each transmit CAN L-PDU.
	// DYNAMIC  CAN ID is defined at runtime.
	// STATIC  CAN ID is defined at compile-time.
	CanIf_PduTypeType		CanIfCanTxPduType;

#if ( CANIF_READTXPDU_NOTIFY_STATUS_API == STD_ON )
	//	Enables and disables transmit confirmation for each transmit CAN L-PDU
	//	for reading its notification status. True: Enabled False: Disabled
	boolean		CanIfReadTxPduNotifyStatus;
#endif

	//	CAN Identifier of transmit CAN L-PDUs used by the CAN Driver for CAN L-
	//	PDU transmission.
	//  EXTENDED_CAN  The CANID is of type Extended (29 bits)
	//  STANDARD_CAN  The CANID is of type Standard (11 bits)
	CanIf_CanIdTypeType		CanIfTxPduIdCanIdType;

	//	Name of target confirmation services to target upper layers (PduR, CanNm
	//	and CanTp. If parameter is not configured then no call-out function is
	//	provided by the upper layer for this Tx L-PDU.
	void (*CanIfUserTxConfirmation)(PduIdType);   /* CANIF 109 */

	//	Handle, that defines the hardware object or the pool of hardware objects
	//	configured for transmission. The parameter refers HTH Id, to which the L-
	//	PDU belongs to.
	const CanIf_HthConfigType *CanIfCanTxPduHthRef;

	//	Reference to the "global" Pdu structure to allow harmonization of handle
	//	IDs in the COM-Stack.  ..
	void *PduIdRef;
} CanIf_TxPduConfigType;

//-------------------------------------------------------------------
/*
 * CanIfRxPduConfig container
 */


// This container contains the configuration (parameters) of each receive
// CAN L-PDU. The SHORT-NAME of "CanIfRxPduConfig" container itself
// represents the symolic name of Receive L-PDU.

typedef struct {
  //  ECU wide unique, symbolic handle for receive CAN L-PDU. The
  //  CanRxPduId is configurable at pre-compile and post-built time. It shall fulfill
  //  ANSI/AUTOSAR definitions for constant defines. Range: 0..max. number
  //  of defined CanRxPduIds
  PduIdType CanIfCanRxPduId;

  //	CAN Identifier of Receive CAN L-PDUs used by the CAN Interface. Exa:
	//	Software Filtering. Range: 11 Bit For Standard CAN Identifier ... 29 Bit For
	//	Extended CAN identifier
	uint32 		CanIfCanRxPduCanId;

	//	Data Length code of received CAN L-PDUs used by the CAN Interface.
	//	Exa: DLC check. The data area size of a CAN L-PDU can have a range
	//	from 0 to 8 bytes.  	uint8 		CanIfCanRxPduDlc;
	uint8		CanIfCanRxPduDlc;

#if ( CANIF_CANPDUID_READDATA_API == STD_ON )
	//	Enables and disables the Rx buffering for reading of received L-PDU data.
	//	True: Enabled False: Disabled
	boolean		CanIfReadRxPduData;
#endif

#if ( CANIF_READRXPDU_NOTIF_STATUS_API == STD_ON )
	//	CanIfReadRxPduNotifyStatus {CANIF_READRXPDU_NOTIFY_STATUS}
	//	Enables and disables receive indication for each receive CAN L-PDU for
	//	reading its' notification status. True: Enabled False: Disabled
	boolean		CanIfReadRxPduNotifyStatus;
#endif

  //  CAN Identifier of receive CAN L-PDUs used by the CAN Driver for CAN L-
  //  PDU transmission.
  //  EXTENDED_CAN  The CANID is of type Extended (29 bits)
  //  STANDARD_CAN  The CANID is of type Standard (11 bits)
	CanIf_CanIdTypeType	CanIfRxPduIdCanIdType;

	//	This parameter defines the type of the receive indication call-outs called to
	//	the corresponding upper layer the used TargetRxPduId belongs to.
	CanIf_UserTypeType  CanIfRxUserType;

	//	Name of target indication services to target upper layers (PduRouter,
	//	CanNm, CanTp and ComplexDeviceDrivers). If parameter is 0 no call-out
	//	function is configured.
	void *CanIfUserRxIndication;

	//	The HRH to which Rx L-PDU belongs to, is referred through this
	//	parameter.
	const CanIf_HrhConfigType *CanIfCanRxPduHrhRef;

	//	Reference to the "global" Pdu structure to allow harmonization of handle
	//	IDs in the COM-Stack.
	void *PduIdRef;

	// Defines the type of software filtering that should be used
	// for this receive object.
	CanIf_SoftwareFilterTypeType CanIfSoftwareFilterType;

	// Acceptance filters, 1 - care, 0 - don't care.
	// Is enabled by the CanIfSoftwareFilterMask in CanIf_HrhConfigType
	// ArcCore exension
	uint32 CanIfCanRxPduCanIdMask;

} CanIf_RxPduConfigType;

//-------------------------------------------------------------------

/*
 * CanIfControllerConfig container
 */

typedef enum {
	CANIF_WAKEUP_SUPPORT_CONTROLLER,
	CANIF_WAKEUP_SUPPORT_NO_WAKEUP,
	CANIF_WAKEUP_SUPPORT_TRANSCEIVER,
} CanIf_WakeupSupportType;


// This is the type supplied to CanIf_InitController()
typedef struct {
	CanIf_WakeupSupportType WakeupSupport;  // Not used

	// CanIf-specific id of the controller
	CanIf_Arc_ChannelIdType CanIfControllerIdRef;

	const char CanIfDriverNameRef[8]; // Not used

	const Can_ControllerConfigType *CanIfInitControllerRef;
} CanIf_ControllerConfigType;

//-------------------------------------------------------------------
/*
 * CanIfTransceiverDrvConfig container
 */

typedef struct {
	boolean	TrcvWakeupNotification;
	uint8	TrcvIdRef;
} CanIf_TransceiverDrvConfigType;


typedef struct {
	uint32 todo;
} CanIf_TransceiverConfigType;

// Callout functions with respect to the upper layers. This callout functions
// defined in this container are common to all configured underlying CAN
// Drivers / CAN Transceiver Drivers.
typedef struct {
	//	Name of target BusOff notification services to target upper layers
	//	(PduRouter, CanNm, CanTp and ComplexDeviceDrivers).
	//  Multiplicity: 1
	void (*CanIfBusOffNotification)(uint8 Controller);

	//	Name of target wakeup notification services to target upper layers
	//	e.g Ecu_StateManager. If parameter is 0
	//	no call-out function is configured.
	//  Multiplicity: 0..1
	void (*CanIfWakeUpNotification)();

	//	Name of target wakeup validation notification services to target upper
	//	layers (ECU State Manager). If parameter is 0 no call-out function is
	//	configured.
	//  Multiplicity: 0..1
	void (*CanIfWakeupValidNotification)();

	// ArcCore ext.
	void (*CanIfErrorNotificaton)(uint8,Can_Arc_ErrorType);

} CanIf_DispatchConfigType;

// This container contains the references to the configuration setup of each
// underlying CAN driver.

typedef struct {
	//	Selects the CAN Interface specific configuration setup. This type of the
	//	external data structure shall contain the post build initialization data for the
	//	CAN Interface for all underlying CAN Dirvers. constant to CanIf_ConfigType
	uint32 CanIfConfigSet;

	uint32 CanIfNumberOfCanRxPduIds;
	uint32 CanIfNumberOfCanTXPduIds;
	uint32 CanIfNumberOfDynamicCanTXPduIds;

	//
	// Containers
	//

  //  This container contains the reference to the configuration
	//  setup of each underlying CAN driver.
  //  Multiplicity: 0..*
	const CanIf_InitHohConfigType *CanIfHohConfigPtr;

	//	This container contains the configuration (parameters) of each
	//	receive CAN L-PDU. The SHORT-NAME of
	//	"CanIfRxPduConfig" container itself represents the symolic
	//	name of Receive L-PDU.
	// Multiplicity: 0..*
	const CanIf_RxPduConfigType *CanIfRxPduConfigPtr;

	//	This container contains the configuration (parameters) of each
	//	transmit CAN L-PDU. The SHORT-NAME of
	//	"CanIfTxPduConfig" container represents the symolic name of
	//  Transmit L-PDU.
	// Multiplicity: 0..*
	const CanIf_TxPduConfigType *CanIfTxPduConfigPtr;

} CanIf_InitConfigType;


typedef struct {
	//	This container contains the configuration (parameters) of all
	//	addressed CAN controllers by each underlying CAN driver.
	//  Multiplicity: 1..*
	const CanIf_ControllerConfigType 	*ControllerConfig;

	//	Callout functions with respect to the upper layers. This callout
	//	functions defined in this container are common to all
	//	configured underlying CAN Drivers / CAN Transceiver Drivers
	const CanIf_DispatchConfigType 	*DispatchConfig;

	//	This container contains the init parameters of the CAN
	//	Interface.
	//  Multiplicity: 1..*
	const CanIf_InitConfigType 		*InitConfig;

	//	This container contains the configuration (parameters) of all
	//	addressed CAN transceivers by each underlying CAN
	//	Transceiver Driver.
	//  Multiplicity: 1..*
	const CanIf_TransceiverConfigType *TransceiverConfig;

	// ArcCore: Contains the mapping from CanIf-specific Channels to Can Controllers
	const CanControllerIdType			*Arc_ChannelToControllerMap;
} CanIf_ConfigType;


extern CanIf_ConfigType CanIf_Config;

#endif


