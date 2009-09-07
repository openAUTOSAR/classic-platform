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


// Container that gets slamed into CanIf_InitController()
// Inits ALL controllers
// Multiplicity 1..*
const CanIf_ControllerConfigType CanIfControllerConfig[] =
{
  { // This is the ConfigurationIndex in CanIf_InitController()
    .WakeupSupport = CANIF_WAKEUP_SUPPORT_NO_WAKEUP,
    .CanIfControllerIdRef = CAN_CTRL_A,
    .CanIfDriverNameRef = "FLEXCAN",  // Not used
    .CanIfInitControllerRef = &CanControllerConfigData[0],
  },
  {
    .WakeupSupport = CANIF_WAKEUP_SUPPORT_NO_WAKEUP,
    .CanIfControllerIdRef = CAN_CTRL_C,
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
    .CanIfHthType = CAN_ECORE_HANDLE_TYPE_BASIC,
    .CanIfCanControllerIdRef = CAN_CTRL_A,
    .CanIfHthIdSymRef = CAN_HTH_A_1, // Ref to the HTH
    .CanIfEcoreEOL = 0,
  },
  {
    .CanIfHthType = CAN_ECORE_HANDLE_TYPE_BASIC,
    .CanIfCanControllerIdRef = CAN_CTRL_C,
    .CanIfHthIdSymRef = CAN_HTH_C_1, // Ref to the HTH
    .CanIfEcoreEOL = 1,
  },
};

//-------------------------------------------------------------------
const CanIf_HrhConfigType CanIfHrhConfigData[] =
{
  {
    .CanIfHrhType = CAN_ECORE_HANDLE_TYPE_BASIC,
    .CanIfSoftwareFilterHrh = TRUE,   // Disable software filtering
    .CanIfCanControllerHrhIdRef = CAN_CTRL_A,
    .CanIfHrhIdSymRef = CAN_HRH_A_1, // Ref to the HRH
    .CanIfEcoreEOL = 0,
  },
  {
    .CanIfHrhType = CAN_ECORE_HANDLE_TYPE_BASIC,
    .CanIfSoftwareFilterHrh = TRUE,   // Disable software filtering
    .CanIfCanControllerHrhIdRef = CAN_CTRL_C,
    .CanIfHrhIdSymRef = CAN_HRH_C_1, // Ref to the HRH
    .CanIfEcoreEOL = 1,
  },
};
//-------------------------------------------------------------------

/*
 *  TX PDUs
 */
const CanIf_TxPduConfigType CanIfTxPduConfigData[] =
{
};
//-------------------------------------------------------------------

/*
 *  RX PDUs
 */
const CanIf_RxPduConfigType CanIfRxPduConfigData[] =
{
  {
    .CanIfCanRxPduId = LedCommandRx,
    .CanIfCanRxPduCanId = 0x123, // CAN ID
    .CanIfCanRxPduDlc = 2, //DLC
#if ( CANIF_CANPDUID_READDATA_API == STD_ON )
    .CanIfReadRxPduData = FALSE, // no buffering
#endif
#if ( CANIF_READRXPDU_NOTIF_STATUS_API == STD_ON )
    .CanIfReadRxPduNotifyStatus = FALSE, // No indication
#endif
    .CanIfRxPduIdCanIdType = CANIF_CAN_ID_TYPE_29,
    .CanIfRxUserType = CANIF_USER_TYPE_CAN_PDUR,
    .CanIfUserRxIndication = PduR_CanIfRxIndication, // No indication
    .CanIfCanRxPduHrhRef = &CanIfHrhConfigData[0], // Received on controller A
    .PduIdRef = NULL, // Could be used by upper layers
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
    .CanIfEcoreEOL = 1,
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
};

