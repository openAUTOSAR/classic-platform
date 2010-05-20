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

#include "Com.h"

typedef struct {
	const uint8						CanIfControllerId;
} CanSM_ControllerType;

typedef struct {
	const CanSM_ControllerType*		Controllers;
	const uint8						ControllerCount;
	const uint8						ComMNetworkHandle;
	const Com_PduGroupIdType		ComRxPduGroupId;
	const Com_PduGroupIdType		ComTxPduGroupId;
} CanSM_NetworkType;

typedef struct {
	const CanSM_NetworkType*		Networks;
} CanSM_ConfigType;

#endif /* CANSM_CONFIGTYPES_H_ */
