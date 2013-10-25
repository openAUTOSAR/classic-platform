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








#include "LinIf_Cfg.h"
#include "Lin_Cfg.h"
#include "PduR.h"

extern const Lin_ChannelConfigType LinChannelConfigData[];

// Frames config
const LinIf_FrameType LinIfFrameCfg[] = {
		{
			.LinIfChecksumType = ENHANCED,
			.LinIfFramePriority = 0,//not used
			.LinIfFrameType = UNCONDITIONAL,
			.LinIfLength = 8,
			.LinIfPid = 0xb,
			.LinIfTxTargetPduId = PDUR_PDU_ID_ENG_TX,
			.LinIfFixedFrameSdu = 0,
			.LinIfPduDirection = LinIfTxPdu,
		},
		{
			.LinIfChecksumType = ENHANCED,
			.LinIfFramePriority = 0,//not used
			.LinIfFrameType = UNCONDITIONAL,
			.LinIfLength = 4,
			.LinIfPid = 0xe,
			.LinIfTxTargetPduId = PDUR_PDU_ID_LAMPS_RX,
			.LinIfFixedFrameSdu = 0,
			.LinIfPduDirection = LinIfRxPdu,
		},
		{
			.LinIfChecksumType = ENHANCED,
			.LinIfFramePriority = 0,//not used
			.LinIfFrameType = UNCONDITIONAL,
			.LinIfLength = 8,
			.LinIfPid = 0x12,
			.LinIfTxTargetPduId = PDUR_PDU_ID_TCM_RX,
			.LinIfFixedFrameSdu = 0,
			.LinIfPduDirection = LinIfRxPdu,
		},
};

//  Schedule entry config.
// Multiplicity 1..*
const LinIfEntryType LinIfEntryCfg1[] = {
	{
		.LinIfDelay = 10,
		.LinIfEntryIndex = 0,
		.LinIfCollisionResolvingRef = 0,
		.LinIfFrameRef = 0,
	},
	{
		.LinIfDelay = 15,
		.LinIfEntryIndex = 1,
		.LinIfCollisionResolvingRef = 0,
		.LinIfFrameRef = 1,
	},
	{
		.LinIfDelay = 25,
		.LinIfEntryIndex = 2,
		.LinIfCollisionResolvingRef = 0,
		.LinIfFrameRef = 2,
	},
};

// Schedule table config.
// Multiplicity 1..*
const LinIf_ScheduleTableType LinIfScheduleTableCfg[] =
{
	{
		.LinIfResumePosition = START_FROM_BEGINNING,
		.LinIfRunMode = RUN_CONTINUOUS,
		.LinIfSchedulePriority = 1,
		.LinIfScheduleTableIndex = 0,
		.LinIfScheduleTableName = "NULL_SCHEDULE", //Not used
		.LinIfEntry = 0, // Null schedule
		.LinIfNofEntries = 0,
	},
	{
		.LinIfResumePosition = START_FROM_BEGINNING,
		.LinIfRunMode = RUN_CONTINUOUS,
		.LinIfSchedulePriority = 1,
		.LinIfScheduleTableIndex = 1,
		.LinIfScheduleTableName = "NORMAL", //Not used
		.LinIfEntry = &LinIfEntryCfg1[0],
		.LinIfNofEntries = 3,
	},
};

// Channel config.
// Multiplicity 1..*
const LinIf_ChannelType LinIfChannelCfg[] =
{
  {
    .LinIfChannelId = LIN_CTRL_A, // Should map against driver
    .LinIfScheduleRequestQueueLength = 1,
    .LinIfChannelRef = &LinChannelConfigData[0],
    .LinIfFrame = 0, // TODO ???????
    .LinIfMaster.LinIfJitter = 1,
    .LinIfScheduleTable = &LinIfScheduleTableCfg[0],
    .LinIfSlave = 0, // Master only
    .LinIfWakeUpSource = 0, // Not needed
   },
};

// Global config.
const LinIf_GlobalConfigType LinIfGlobalConfig =
{
	.LinIfTimeBase = 5,
	.LinIfChannel = &LinIfChannelCfg[0],
};
