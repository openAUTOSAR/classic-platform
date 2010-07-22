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









extern Com_ConfigType ComConfig_TEST;

extern const PduR_PBConfigType PduRConfigData_TEST[];


PduR_StdRTableType PduRConfigData_TEST = {
	.TargetPduId = 0,
	.NRoutingPaths = 6,
	.RoutingTable = {
		{
			.FctPtrs = &PduR_StdLinFctPtrs,
			.DestPduId = 0,
			.SduLength = 2,
			.DataProvision = PDUR_DIRECT,
			.BufferDepth = 3,
		},
		{
			.FctPtrs = &PduR_StdLinFctPtrs,
			.DestPduId = 1,
			.SduLength = 8,
			.DataProvision = PDUR_NO_PROVISION,
		},
		{
			.FctPtrs = &PduR_StdLinFctPtrs,
			.DestPduId = 2,
			.SduLength = 8,
			.DataProvision = PDUR_TRIGGER_TRANSMIT,
			.BufferDepth = 3,
		},
		{
			.FctPtrs = &PduR_StdLinFctPtrs,
			.DestPduId = 3,
			.SduLength = 8,
			.DataProvision = PDUR_DIRECT,
			.PduR_GatewayMode = 1,
			.BufferDepth = 4,
		},

		/* Gateway mode between lin interfaces but without buffers inbetween */
		{
			.FctPtrs = &PduR_StdLinFctPtrs,
			.DestPduId = 4,
			.SduLength = 8,
			.DataProvision = PDUR_NO_PROVISION,
			.PduR_GatewayMode = 1,
		},

		/* Gateway mode between lin interfaces using trigger transmit data provision */
		{
			.FctPtrs = &PduR_StdLinFctPtrs,
			.DestPduId = 5,
			.SduLength = 8,
			.DataProvision = PDUR_TRIGGER_TRANSMIT,
			.PduR_GatewayMode = 1,
			.BufferDepth = 4,
		},
	}
};
