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


#ifndef CANSM_CONFIGTYPES_H_
#define CANSM_CONFIGTYPES_H_

/** @req CANSM010.bswbuilder */

#include "Com.h"

#if defined(USE_DEM)
#include "Dem.h"
#endif

typedef struct {
	const uint8						CanIfControllerId;
} CanSM_ControllerType;

typedef struct {
	const CanSM_ControllerType*		Controllers;
	const uint8						ControllerCount;
	const uint8						ComMNetworkHandle;
	const Com_PduGroupIdType		ComRxPduGroupId;     /**< @req CANSM091 */
	const Com_PduGroupIdType		ComTxPduGroupId;
	const uint32                    CanSMBorTimeTxEnsured;
	const uint32                    CanSMBorTimeL1;
	const uint32                    CanSMBorTimeL2;
	const uint8                     CanSMBorCounterL1ToL2;
	const uint8                     CanSMBorCounterL2Err;
	const boolean                   CanSMBorTxConfirmationPolling;
	const boolean                   CanSMBorDisableRxDlMonitoring;
#if defined(USE_DEM)
	const Dem_EventIdType			CanSMBusOffDemEvent;
#endif
} CanSM_NetworkType;

typedef struct {
	const CanSM_NetworkType*		Networks;
} CanSM_ConfigType;                                       /**< @req CANSM061 */

#endif /* CANSM_CONFIGTYPES_H_ */
