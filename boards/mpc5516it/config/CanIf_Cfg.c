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








#include "CanIf.h"
#include <stdlib.h>

// Imported structs from Can_Lcfg.c
extern const Can_ControllerConfigType CanControllerConfigData[];
extern const Can_ConfigSetType CanConfigSetData;

// Contains the mapping from CanIf-specific Channels to Can Controllers
const CanControllerIdType CanIf_Arc_ChannelToControllerMap[CANIF_CHANNEL_CNT] = {
		CAN_CTRL_A, // CANIF_CHANNEL_0
		CAN_CTRL_C, // CANIF_CHANNEL_1
};

// Container that gets slamed into CanIf_InitController()
// Inits ALL controllers
// Multiplicity 1..*
const CanIf_ControllerConfigType CanIfControllerConfig[] =
{
  { // CANIF_CHANNEL_0_CONFIG_0
    .WakeupSupport = CANIF_WAKEUP_SUPPORT_NO_WAKEUP,
    .CanIfControllerIdRef = CANIF_CHANNEL_0,
    .CanIfDriverNameRef = "FLEXCAN",  // Not used
    .CanIfInitControllerRef = &CanControllerConfigData[0],
  },
  { // CANIF_CHANNEL_1_CONFIG_0
    .WakeupSupport = CANIF_WAKEUP_SUPPORT_NO_WAKEUP,
    .CanIfControllerIdRef = CANIF_CHANNEL_1,
    .CanIfDriverNameRef = "FLEXCAN", // Not used
    .CanIfInitControllerRef = &CanControllerConfigData[1],
  }
};

// Function callbacks for higher layers
const CanIf_DispatchConfigType CanIfDispatchConfig =
{
  .CanIfBusOffNotification = NULL,
  .CanIfWakeUpNotification = NULL,        // Not used
  .CanIfWakeupValidNotification = NULL,   // Not used
  .CanIfErrorNotificaton = NULL,
};

//-------------------------------------------------------------------
const CanIf_HthConfigType CanIfHthConfigData[] =
{
  {
    .CanIfHthType = CAN_ARC_HANDLE_TYPE_BASIC,
    .CanIfCanControllerIdRef = CANIF_CHANNEL_0,
    .CanIfHthIdSymRef = CAN_HTH_A_1, // Ref to the HTH
    .CanIf_Arc_EOL = 0,
  },
  {
    .CanIfHthType = CAN_ARC_HANDLE_TYPE_BASIC,
    .CanIfCanControllerIdRef = CANIF_CHANNEL_1,
    .CanIfHthIdSymRef = CAN_HTH_C_1, // Ref to the HTH
    .CanIf_Arc_EOL = 1,
  },
};

//-------------------------------------------------------------------
const CanIf_HrhConfigType CanIfHrhConfigData[] =
{
  {
    .CanIfHrhType = CAN_ARC_HANDLE_TYPE_BASIC,
    .CanIfSoftwareFilterHrh = TRUE,   // Disable software filtering
    .CanIfCanControllerHrhIdRef = CANIF_CHANNEL_0,
    .CanIfHrhIdSymRef = CAN_HRH_A_1, // Ref to the HRH
    .CanIf_Arc_EOL = 0,
  },
  {
    .CanIfHrhType = CAN_ARC_HANDLE_TYPE_BASIC,
    .CanIfSoftwareFilterHrh = TRUE,   // Disable software filtering
    .CanIfCanControllerHrhIdRef = CANIF_CHANNEL_1,
    .CanIfHrhIdSymRef = CAN_HRH_C_1, // Ref to the HRH
    .CanIf_Arc_EOL = 1,
  },
};
//-------------------------------------------------------------------

/*
 *  TX PDUs
 */
const CanIf_TxPduConfigType CanIfTxPduConfigData[] =
{
  {
    .CanIfTxPduId = PDU_MSG_HARDWARE_TEST_ENGINE_STATUS, //PDU_MSG_TX789,
    .CanIfCanTxPduIdCanId = 0x0000000,
    .CanIfCanTxPduIdDlc = 8,
    .CanIfCanTxPduType = CANIF_PDU_TYPE_STATIC,
#if ( CANIF_READTXPDU_NOTIFY_STATUS_API == STD_ON )
    .CanIfReadTxPduNotifyStatus = FALSE,
#endif
    .CanIfTxPduIdCanIdType = CANIF_CAN_ID_TYPE_29,
    .CanIfUserTxConfirmation = PduR_CanIfTxConfirmation, //NULL,
    .CanIfCanTxPduHthRef = &CanIfHthConfigData[0], // Send on channel 0,
    .PduIdRef = NULL,
  },
  {
    .CanIfTxPduId = PDU_MSG_TX987,
    .CanIfCanTxPduIdCanId = 0x0000100,
    .CanIfCanTxPduIdDlc = 8,
    .CanIfCanTxPduType = CANIF_PDU_TYPE_DYNAMIC,
#if ( CANIF_READTXPDU_NOTIFY_STATUS_API == STD_ON )
    .CanIfReadTxPduNotifyStatus = FALSE,
#endif
    .CanIfTxPduIdCanIdType = CANIF_CAN_ID_TYPE_29,
    .CanIfUserTxConfirmation = NULL,
    .CanIfCanTxPduHthRef = &CanIfHthConfigData[1], // Send on channel 1,
    .PduIdRef = NULL,
  },
  //Added by mattias
	{
	  .CanIfTxPduId = 2, //PDU_MSG_TX789,
	  .CanIfCanTxPduIdCanId = 0x0000200,
	  .CanIfCanTxPduIdDlc = 8,
	  .CanIfCanTxPduType = CANIF_PDU_TYPE_STATIC,
	#if ( CANIF_READTXPDU_NOTIFY_STATUS_API == STD_ON )
	  .CanIfReadTxPduNotifyStatus = FALSE,
	#endif
	  .CanIfTxPduIdCanIdType = CANIF_CAN_ID_TYPE_29,
	  .CanIfUserTxConfirmation = PduR_CanIfTxConfirmation, // NULL
	  .CanIfCanTxPduHthRef = &CanIfHthConfigData[0], // Send on channel 0,
	  .PduIdRef = NULL,
	},
};
//-------------------------------------------------------------------

/*
 *  RX PDUs
 */
const CanIf_RxPduConfigType CanIfRxPduConfigData[] =
{
  {
    .CanIfCanRxPduId = PDU_MSG_HARDWARE_TEST_ENGINE_CONTROL, //PDU_MSG_RX234,
    .CanIfCanRxPduCanId = 1, // CAN ID
    .CanIfCanRxPduDlc = 8, //DLC
#if ( CANIF_CANPDUID_READDATA_API == STD_ON )
    .CanIfReadRxPduData = FALSE, // no buffering
#endif
#if ( CANIF_READRXPDU_NOTIF_STATUS_API == STD_ON )
    .CanIfReadRxPduNotifyStatus = FALSE, // No indication
#endif
    .CanIfRxPduIdCanIdType = CANIF_CAN_ID_TYPE_29,
    .CanIfRxUserType = CANIF_USER_TYPE_CAN_PDUR, // CANIF_USER_TYPE_CAN_SPECIAL, // Changed by Mattias to test PDU router and Com layer.
    .CanIfUserRxIndication = NULL, // No indication
    .CanIfCanRxPduHrhRef = &CanIfHrhConfigData[0], // Received on channel 0,
    .PduIdRef = NULL, // Could be used by upper layers
    .CanIfSoftwareFilterType = CANIF_SOFTFILTER_TYPE_MASK, // Not enabled in HRH
    .CanIfCanRxPduCanIdMask = 0xFFF,
  },
  {
    .CanIfCanRxPduId = PDU_MSG_HARDWARE_TEST_ROUTED_MSG, //PDU_MSG_RX123,
    .CanIfCanRxPduCanId = 0x02, // CAN ID
    .CanIfCanRxPduDlc = 8, //DLC
#if ( CANIF_CANPDUID_READDATA_API == STD_ON )
    .CanIfReadRxPduData = FALSE, // no buffering
#endif
#if ( CANIF_READRXPDU_NOTIF_STATUS_API == STD_ON )
    .CanIfReadRxPduNotifyStatus = FALSE, // No indication
#endif
    .CanIfRxPduIdCanIdType = CANIF_CAN_ID_TYPE_29, //
    .CanIfRxUserType = CANIF_USER_TYPE_CAN_PDUR, // CANIF_USER_TYPE_CAN_SPECIAL,
    .CanIfUserRxIndication = NULL, // No indication
    .CanIfCanRxPduHrhRef = &CanIfHrhConfigData[0], //&CanIfHrhConfigData[1], // Received on channel 0,
    .PduIdRef = NULL, //
    .CanIfSoftwareFilterType = CANIF_SOFTFILTER_TYPE_MASK, // Not enabled in HRH
    .CanIfCanRxPduCanIdMask = 0xFFF,
  },

};

//-------------------------------------------------------------------
const CanIf_InitHohConfigType CanIfHohConfigData[] =
{
  {
    .CanConfigSet = &CanConfigSetData,
    .CanIfHrhConfig = CanIfHrhConfigData,
    .CanIfHthConfig = CanIfHthConfigData,
    .CanIf_Arc_EOL = 1,
  },
};

// This container contains the init parameters of the CAN
// Multiplicity 1..*
const CanIf_InitConfigType CanIfInitConfig =
{
  .CanIfConfigSet = 0, // Not used
  .CanIfNumberOfCanRxPduIds = sizeof(CanIfRxPduConfigData)/sizeof(CanIf_RxPduConfigType),
  .CanIfNumberOfCanTXPduIds = sizeof(CanIfTxPduConfigData)/sizeof(CanIf_TxPduConfigType),
  .CanIfNumberOfDynamicCanTXPduIds = 0, // Not used

  // Containers
  .CanIfHohConfigPtr = CanIfHohConfigData,
  .CanIfRxPduConfigPtr = CanIfRxPduConfigData,
  .CanIfTxPduConfigPtr = CanIfTxPduConfigData,
};

// This container includes all necessary configuration sub-containers
// according the CAN Interface configuration structure.
CanIf_ConfigType CanIf_Config =
{
  .ControllerConfig = CanIfControllerConfig,
  .DispatchConfig = &CanIfDispatchConfig,
  .InitConfig = &CanIfInitConfig,
  .TransceiverConfig = NULL, // Not used
  .Arc_ChannelToControllerMap = CanIf_Arc_ChannelToControllerMap,
};

