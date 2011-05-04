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

#include "LinIf_Cfg.h"
#include "Lin_Cfg.h"

extern const Lin_ChannelConfigType LinChannelConfigData[];

// Frames config
const LinIf_FrameType LinIfFrameCfg[] = {
		{
			.LinIfChecksumType = ENHANCED,
			.LinIfFramePriority = 0,//not used
			.LinIfFrameType = UNCONDITIONAL,
			.LinIfLength = 2,
			.LinIfPid = 0xC1,
			.LinIfTxTargetPduId = PDU_MSG_LIN_TX_1,
			.LinIfFixedFrameSdu = 0,
			.LinIfPduDirection = LinIfTxPdu,
		},
		{
			.LinIfChecksumType = ENHANCED,
			.LinIfFramePriority = 0,//not used
			.LinIfFrameType = UNCONDITIONAL,
			.LinIfLength = 3,
			.LinIfPid = 0x42,
			.LinIfTxTargetPduId = PDU_MSG_LIN_RX_1,
			.LinIfFixedFrameSdu = 0,
			.LinIfPduDirection = LinIfRxPdu,
		},
		{
			.LinIfChecksumType = ENHANCED,
			.LinIfFramePriority = 0,//not used
			.LinIfFrameType = UNCONDITIONAL,
			.LinIfLength = 6,
			.LinIfPid = 0x03,
			.LinIfTxTargetPduId = PDU_MSG_LIN_RX_2,
			.LinIfFixedFrameSdu = 0,
			.LinIfPduDirection = LinIfRxPdu,
		},
};

//  Schedule entry config.
// Multiplicity 1..*
const LinIfEntryType LinIfEntryCfg1[] = {
	{
		.LinIfDelay = 8,
		.LinIfEntryIndex = 0,
		.LinIfCollisionResolvingRef = 0,
		.LinIfFrameRef = 0,
	},
	{
		.LinIfDelay = 8,
		.LinIfEntryIndex = 1,
		.LinIfCollisionResolvingRef = 0,
		.LinIfFrameRef = 1,
	},
	{
		.LinIfDelay = 8,
		.LinIfEntryIndex = 2,
		.LinIfCollisionResolvingRef = 0,
		.LinIfFrameRef = 2,
	},
	{
		.LinIfDelay = 8,
		.LinIfEntryIndex = 3,
		.LinIfCollisionResolvingRef = 0,
		.LinIfFrameRef = 1,
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
		.LinIfNofEntries = 4,
	},
};

// Channel config.
// Multiplicity 1..*
const LinIf_ChannelType LinIfChannelCfg[] =
{
  {
    .LinIfChannelId = LIN_CTRL_B, // Should map against driver
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
	.LinIfTimeBase = 8,
	.LinIfChannel = &LinIfChannelCfg[0],
};
