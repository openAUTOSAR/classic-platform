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

#include <stdlib.h>
#include "Can.h"
#include "CanIf_Cbk.h"

/* CONFIGURATION TEMPLATE
 * ------------------------------------------------------------------
 *
 * The following template configures:
 * - 2 CAN controllers, CAN_CTRL_A and CAN_CNTR_C
 * - Callbacks are configured to call standard CanIf callbacks
 *
 * CAN_CTRL_A
 *   - 125K baudrate
 *   - Extended 11-bit ID's
 *   - Rx/Tx are interrupt based
 *   - Both Rx(MB 16->24) and Tx(MB 24->31) Hoh's are of type BASIC
 *
 * CAN_CTRL_C
 *   - 125K baudrate
 *   - Standard 29-bit ID's
 *   - Rx/Tx are interrupt based
 *   - Both Rx(MB 16->24) and Tx(MB 24->31) Hoh's are of type BASIC
 * *
 */

// Accept everything = 0
Can_FilterMaskType Can_FilterMaskConfigData = 0;

// HOH:s for CAN_CTRL_A
const Can_HardwareObjectType CanHardwareObjectConfig_CTRL_A[] =
{
  {
    .CanHandleType =    CAN_ARC_HANDLE_TYPE_BASIC,
    .CanIdType =        CAN_ID_TYPE_EXTENDED,
    .CanObjectType =    CAN_OBJECT_TYPE_RECEIVE,
    .CanObjectId =      CAN_HRH_A_1,
    .CanFilterMaskRef = &Can_FilterMaskConfigData,
    // ArcCore
    .Can_Arc_MbMask = 0x00ff0000,
    .Can_Arc_EOL = 0,
  },
  {
    .CanHandleType =    CAN_ARC_HANDLE_TYPE_BASIC,
    .CanIdType =        CAN_ID_TYPE_EXTENDED,
    .CanObjectType =    CAN_OBJECT_TYPE_TRANSMIT,
    .CanObjectId =      CAN_HTH_A_1,
    .CanFilterMaskRef = &Can_FilterMaskConfigData,
    // ArcCore
    .Can_Arc_MbMask =   0xff000000,
    .Can_Arc_EOL = 1,
  },
};

// HOH:s for CAN_CTRL_C
const Can_HardwareObjectType CanHardwareObjectConfig_CTRL_C[] =
{
  {
    .CanHandleType =    CAN_ARC_HANDLE_TYPE_BASIC,
    .CanIdType =        CAN_ID_TYPE_STANDARD,
    .CanObjectType =    CAN_OBJECT_TYPE_RECEIVE,
    .CanObjectId =      CAN_HRH_C_1,
    .CanFilterMaskRef = &Can_FilterMaskConfigData,
    // ArcCore
    .Can_Arc_MbMask =   0x00ff0000,
    .Can_Arc_EOL = 0,
  },
  {
    .CanHandleType =    CAN_ARC_HANDLE_TYPE_BASIC,
    .CanIdType =        CAN_ID_TYPE_STANDARD,
    .CanObjectType =    CAN_OBJECT_TYPE_TRANSMIT,
    .CanObjectId =      CAN_HTH_C_1,
    .CanFilterMaskRef = &Can_FilterMaskConfigData,
    // ArcCore
    .Can_Arc_MbMask =   0xff000000,
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
    .CanControllerId = CAN_CTRL_A,
    .CanControllerPropSeg = 4,
    .CanControllerSeg1 =    4,
    .CanControllerSeg2 =    4,
    .CanBusOffProcessing =  CAN_ARC_PROCESS_TYPE_INTERRUPT,
    .CanRxProcessing =      CAN_ARC_PROCESS_TYPE_INTERRUPT,
    .CanTxProcessing =      CAN_ARC_PROCESS_TYPE_INTERRUPT,
    .CanWakeupProcessing =  CAN_ARC_PROCESS_TYPE_INTERRUPT,
    .CanCpuClockRef = PERIPHERAL_CLOCK_FLEXCAN_A,
    // ArcCore
    .Can_Arc_Hoh = &CanHardwareObjectConfig_CTRL_A[0],
    .Can_Arc_Loopback = 0,
    .Can_Arc_Fifo = 0,

  },{
    .CanControllerActivation = TRUE,
    .CanControllerBaudRate = 125,
    .CanControllerId = CAN_CTRL_C,
    .CanControllerPropSeg = 4,
    .CanControllerSeg1 =    4,
    .CanControllerSeg2 =    4,
    .CanBusOffProcessing =  CAN_ARC_PROCESS_TYPE_INTERRUPT,
    .CanRxProcessing =      CAN_ARC_PROCESS_TYPE_INTERRUPT,
    .CanTxProcessing =      CAN_ARC_PROCESS_TYPE_INTERRUPT,
    .CanWakeupProcessing =  CAN_ARC_PROCESS_TYPE_INTERRUPT,
    .CanCpuClockRef = PERIPHERAL_CLOCK_FLEXCAN_C,
    // ArcCore
    .Can_Arc_Hoh = &CanHardwareObjectConfig_CTRL_C[0],
    .Can_Arc_Loopback = 1,
    .Can_Arc_Fifo = 0,
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



