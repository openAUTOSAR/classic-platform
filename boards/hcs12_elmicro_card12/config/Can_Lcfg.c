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








#include <stdlib.h>
#include "Can.h"
#include "CanIf_Cbk.h"

/* CONFIGURATION TEMPLATE
 * ------------------------------------------------------------------
 *
 * The following template configures:
 * - 2 CAN controllers, CAN_CTRL_0 and CAN_CTRL_4
 * - Callbacks are configured to call standard CanIf callbacks
 *
 * CAN_CTRL_0
 *   - 125K baudrate
 *   - Extended 29-bit ID's
 *   - Rx/Tx are interrupt based
 *   - Hoh's are of type BASIC
 *
 * CAN_CTRL_4
 *   - 125K baudrate
 *   - Extended 29-bit ID's
 *   - Rx/Tx are interrupt based
 *   - Hoh's are of type BASIC
 * *
 */

// Accept everything = 1
Can_FilterMaskType Can_FilterMaskConfigData[] = {
		{
			{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff},
			{0,0,0,0,0,0,0,0},
			CAN_ARC_IDAM_2_32BIT
		},
		{
			{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff},
			{0,0,0,0,0,0,0,0},
			CAN_ARC_IDAM_2_32BIT
		},
};

// HOH:s for CAN_CTRL_0
const Can_HardwareObjectType CanHardwareObjectConfig_CTRL_0[] =
{
  {
    .CanHandleType =    CAN_ARC_HANDLE_TYPE_BASIC,
    .CanIdType =        CAN_ID_TYPE_EXTENDED,
    .CanObjectType =    CAN_OBJECT_TYPE_RECEIVE,
    .CanObjectId =      CAN_HRH_0_1,
    .CanFilterMaskRef = &Can_FilterMaskConfigData[0],
    // ArcCore
    .Can_Arc_EOL = 0,
  },
  {
    .CanHandleType =    CAN_ARC_HANDLE_TYPE_BASIC,
    .CanIdType =        CAN_ID_TYPE_EXTENDED,
    .CanObjectType =    CAN_OBJECT_TYPE_TRANSMIT,
    .CanObjectId =      CAN_HTH_0_1,
    .CanFilterMaskRef = &Can_FilterMaskConfigData[0],
    // ArcCore
    .Can_Arc_EOL = 1,
  },
};

// HOH:s for CAN_CTRL_4
const Can_HardwareObjectType CanHardwareObjectConfig_CTRL_4[] =
{
  {
    .CanHandleType =    CAN_ARC_HANDLE_TYPE_BASIC,
    .CanIdType =        CAN_ID_TYPE_STANDARD,
    .CanObjectType =    CAN_OBJECT_TYPE_RECEIVE,
    .CanObjectId =      CAN_HRH_4_1,
    .CanFilterMaskRef = &Can_FilterMaskConfigData[1],
    // ArcCore
    .Can_Arc_EOL = 0,
  },
  {
    .CanHandleType =    CAN_ARC_HANDLE_TYPE_BASIC,
    .CanIdType =        CAN_ID_TYPE_STANDARD,
    .CanObjectType =    CAN_OBJECT_TYPE_TRANSMIT,
    .CanObjectId =      CAN_HTH_4_1,
    .CanFilterMaskRef = &Can_FilterMaskConfigData[1],
    // ArcCore
    .Can_Arc_EOL = 1,
  },
};

/* CAN controller data for ALL controllers that are to be configured
 * See Autosar release 3.0 specification for more info
 */
const Can_ControllerConfigType CanControllerConfigData[] =
{
  {
    .CanControllerActivation = TRUE,
    .CanControllerBaudRate = 125,
    .CanControllerId = CAN_CTRL_0,
    .CanControllerPropSeg = 0,
    .CanControllerSeg1 =    12,
    .CanControllerSeg2 =    1,
    .CanBusOffProcessing =  CAN_ARC_PROCESS_TYPE_INTERRUPT,
    .CanRxProcessing =      CAN_ARC_PROCESS_TYPE_INTERRUPT,
    .CanTxProcessing =      CAN_ARC_PROCESS_TYPE_INTERRUPT,
    .CanWakeupProcessing =  CAN_ARC_PROCESS_TYPE_INTERRUPT,
    // ArcCore
    .Can_Arc_Hoh = &CanHardwareObjectConfig_CTRL_0[0],
    .Can_Arc_Loopback = 0,
  },{
    .CanControllerActivation = TRUE,
    .CanControllerBaudRate = 125,
    .CanControllerId = CAN_CTRL_4,
    .CanControllerPropSeg = 0,
    .CanControllerSeg1 =    12,
    .CanControllerSeg2 =    1,
    .CanBusOffProcessing =  CAN_ARC_PROCESS_TYPE_INTERRUPT,
    .CanRxProcessing =      CAN_ARC_PROCESS_TYPE_INTERRUPT,
    .CanTxProcessing =      CAN_ARC_PROCESS_TYPE_INTERRUPT,
    .CanWakeupProcessing =  CAN_ARC_PROCESS_TYPE_INTERRUPT,
    // ArcCore
    .Can_Arc_Hoh = &CanHardwareObjectConfig_CTRL_4[0],
    .Can_Arc_Loopback = 1,
  }
};

/* Callbacks for the can drivers
 * See Autosar release 3.0 specification for more info
 */
const Can_CallbackType CanCallbackConfigData = {
	    NULL, //CanIf_CancelTxConfirmation,
	    CanIf_RxIndication,
	    CanIf_ControllerBusOff,
	    CanIf_TxConfirmation,
	    NULL, //CanIf_ControllerWakeup,
	    CanIf_Arc_Error,
};

/* Configset configuration information
 * See Autosar release 3.0 specification for more info
 */
const Can_ConfigSetType CanConfigSetData =
{
  .CanController = CanControllerConfigData,
  .CanCallbacks =	&CanCallbackConfigData,
};

/* Top config struct passed to Can_Init() */
const Can_ConfigType CanConfigData = {
  .CanConfigSet = &CanConfigSetData,
};



