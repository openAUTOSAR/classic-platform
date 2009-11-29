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








/*
* Module vendor:  Autocore
* Module version: 1.0.0
* Specification: Autosar v3.0.1, Final
*
*/


#include "Com_PbCfg.h"
#include "stdlib.h"


/*
 * Group signal definitions
 */
ComGroupSignal_type ComGroupSignal[] = {
	{
		.Com_Arc_EOL = 1
	}
};


/*
 * Signal definitions
 */
ComSignal_type ComSignal[] = {
	{
		.ComHandleId = SetLedLevelRx,
		.ComFirstTimeoutFactor = 0,
		.ComNotification = NULL,
		.ComTimeoutFactor = 0,
		.ComTimeoutNotification = NULL,
		.ComTransferProperty = TRIGGERED,


		.ComSignalInitValue = 0,
		.ComBitPosition = 0,
		.ComBitSize = 16,
		.ComSignalEndianess = BIG_ENDIAN,
		.ComSignalType = UINT16,
		.Com_Arc_IsSignalGroup = 0,

	},
	{
		.Com_Arc_EOL = 1
	}
};


/*
 * I-PDU group definitions
 */
ComIPduGroup_type ComIPduGroup[] = {
	{
		.ComIPduGroupHandleId = RxGroup
	},

	{
		.Com_Arc_EOL  = 1
	}
};


/*
 * I-PDU definitions
 */
ComIPdu_type ComIPdu[] = {

	{
		.ComIPduRxHandleId = LedCommandRx,
		.ComIPduCallout = NULL,
		.ComIPduSignalProcessing =  IMMEDIATE,
		.ComIPduSize =  2,
		.ComIPduDirection = RECEIVE,
		.ComIPduGroupRef = RxGroup,

		.ComIPduSignalRef = {

			&ComSignal[ SetLedLevelRx ],

			NULL,
		},
	},

	{
		.Com_Arc_EOL = 1
	}
};

const Com_ConfigType ComConfiguration = {
	.ComConfigurationId = 1,
	.ComIPdu = ComIPdu,
	.ComIPduGroup = ComIPduGroup,
	.ComSignal = ComSignal,
	.ComGroupSignal = ComGroupSignal
};


