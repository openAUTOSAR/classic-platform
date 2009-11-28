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








#include "Com_PbCfg.h"
#include "Com_RunTest.h"

#include "stdlib.h"


/*
 * PCAN Configuration
 */
ComSignal_type PCAN_ComSignal[] = {
	// Signals for PCAN_RX_FRAME_1
	{
		.ComBitPosition = 0,
		.ComBitSize = 8,
		//.ComDataInvalidAction (NOT IMPLEMENTED)
		.ComErrorNotification = NULL,
		.ComFirstTimeoutFactor = 0,
		.ComHandleId = 0,
		//.ComInvalidNotification (NOT IMPLEMENTED)
		.ComNotification = NULL,
		//.ComRxDataTimeoutAction (NOT IMPLEMENTED)
		//.ComSignalDataInvalidValue (NOT IMPLEMENTED)
		.ComSignalEndianess = BIG_ENDIAN,
		.ComSignalInitValue = 0x0,
		.ComSignalLength = 0,
		.ComSignalType = UINT8,
		.ComTimeoutFactor = 0,
		.ComTimeoutNotification = NULL,
		.ComTransferProperty = TRIGGERED,
		.ComUpdateBitPosition = 0,
		.ComSignalArcUseUpdateBit = 0,
		.ComFilter = {
			.ComFilterAlgorithm = ALWAYS,
			.ComFilterMask = 0,
			.ComFilterMax = 0,
			.ComFilterMin = 0,
			.ComFilterOffset = 0,
			.ComFilterPeriodFactor = 0,
			.ComFilterX = 0
		}
	},
	{
		.ComBitPosition = 8,
		.ComBitSize = 8,
		//.ComDataInvalidAction (NOT IMPLEMENTED)
		.ComErrorNotification = NULL,
		.ComFirstTimeoutFactor = 0,
		.ComHandleId = 1,
		//.ComInvalidNotification (NOT IMPLEMENTED)
		.ComNotification = NULL,
		//.ComRxDataTimeoutAction (NOT IMPLEMENTED)
		//.ComSignalDataInvalidValue (NOT IMPLEMENTED)
		.ComSignalEndianess = BIG_ENDIAN,
		.ComSignalInitValue = 0x00,
		.ComSignalLength = 0,
		.ComSignalType = UINT8,
		.ComTimeoutFactor = 0,
		.ComTimeoutNotification = NULL,
		.ComTransferProperty = TRIGGERED,
		.ComUpdateBitPosition = 0,
		.ComSignalArcUseUpdateBit = 0,
		.ComFilter = {
			.ComFilterAlgorithm = ALWAYS,
			.ComFilterMask = 0,
			.ComFilterMax = 0,
			.ComFilterMin = 0,
			.ComFilterOffset = 0,
			.ComFilterPeriodFactor = 0,
			.ComFilterX = 0
		}
	},
	{
		.ComBitPosition = 16,
		.ComBitSize = 1,
		//.ComDataInvalidAction (NOT IMPLEMENTED)
		.ComErrorNotification = NULL,
		.ComFirstTimeoutFactor = 0,
		.ComHandleId = 2,
		//.ComInvalidNotification (NOT IMPLEMENTED)
		.ComNotification = NULL,
		//.ComRxDataTimeoutAction (NOT IMPLEMENTED)
		//.ComSignalDataInvalidValue (NOT IMPLEMENTED)
		.ComSignalEndianess = BIG_ENDIAN,
		.ComSignalInitValue = 0x00,
		.ComSignalLength = 0,
		.ComSignalType = UINT8,
		.ComTimeoutFactor = 0,
		.ComTimeoutNotification = NULL,
		.ComTransferProperty = TRIGGERED,
		.ComUpdateBitPosition = 0,
		.ComSignalArcUseUpdateBit = 0,
		.ComFilter = {
			.ComFilterAlgorithm = ALWAYS,
			.ComFilterMask = 0,
			.ComFilterMax = 0,
			.ComFilterMin = 0,
			.ComFilterOffset = 0,
			.ComFilterPeriodFactor = 0,
			.ComFilterX = 0
		}
	},
	{
		.ComBitPosition = 17,
		.ComBitSize = 2,
		//.ComDataInvalidAction (NOT IMPLEMENTED)
		.ComErrorNotification = NULL,
		.ComFirstTimeoutFactor = 0,
		.ComHandleId = 3,
		//.ComInvalidNotification (NOT IMPLEMENTED)
		.ComNotification = NULL,
		//.ComRxDataTimeoutAction (NOT IMPLEMENTED)
		//.ComSignalDataInvalidValue (NOT IMPLEMENTED)
		.ComSignalEndianess = BIG_ENDIAN,
		.ComSignalInitValue = 0x00,
		.ComSignalLength = 0,
		.ComSignalType = UINT8,
		.ComTimeoutFactor = 0,
		.ComTimeoutNotification = NULL,
		.ComTransferProperty = TRIGGERED,
		.ComUpdateBitPosition = 0,
		.ComSignalArcUseUpdateBit = 0,
		.ComFilter = {
			.ComFilterAlgorithm = ALWAYS,
			.ComFilterMask = 0,
			.ComFilterMax = 0,
			.ComFilterMin = 0,
			.ComFilterOffset = 0,
			.ComFilterPeriodFactor = 0,
			.ComFilterX = 0
		}
	},



	// Signals for PCAN_RX_FRAME_2
	{
		.ComBitPosition = 0,
		.ComBitSize = 8,
		//.ComDataInvalidAction (NOT IMPLEMENTED)
		.ComErrorNotification = NULL,
		.ComFirstTimeoutFactor = 0,
		.ComHandleId = 4,
		//.ComInvalidNotification (NOT IMPLEMENTED)
		.ComNotification = NULL,
		//.ComRxDataTimeoutAction (NOT IMPLEMENTED)
		//.ComSignalDataInvalidValue (NOT IMPLEMENTED)
		.ComSignalEndianess = BIG_ENDIAN,
		.ComSignalInitValue = 0x0,
		.ComSignalLength = 0,
		.ComSignalType = UINT8,
		.ComTimeoutFactor = 0,
		.ComTimeoutNotification = NULL,
		.ComTransferProperty = TRIGGERED,
		.ComUpdateBitPosition = 0,
		.ComSignalArcUseUpdateBit = 0,
		.ComFilter = {
			.ComFilterAlgorithm = ALWAYS,
			.ComFilterMask = 0,
			.ComFilterMax = 0,
			.ComFilterMin = 0,
			.ComFilterOffset = 0,
			.ComFilterPeriodFactor = 0,
			.ComFilterX = 0
		}
	},
	{
		.ComBitPosition = 8,
		.ComBitSize = 8,
		//.ComDataInvalidAction (NOT IMPLEMENTED)
		.ComErrorNotification = NULL,
		.ComFirstTimeoutFactor = 0,
		.ComHandleId = 5,
		//.ComInvalidNotification (NOT IMPLEMENTED)
		.ComNotification = NULL,
		//.ComRxDataTimeoutAction (NOT IMPLEMENTED)
		//.ComSignalDataInvalidValue (NOT IMPLEMENTED)
		.ComSignalEndianess = BIG_ENDIAN,
		.ComSignalInitValue = 0x00,
		.ComSignalLength = 0,
		.ComSignalType = UINT8,
		.ComTimeoutFactor = 0,
		.ComTimeoutNotification = NULL,
		.ComTransferProperty = TRIGGERED,
		.ComUpdateBitPosition = 0,
		.ComSignalArcUseUpdateBit = 0,
		.ComFilter = {
			.ComFilterAlgorithm = ALWAYS,
			.ComFilterMask = 0,
			.ComFilterMax = 0,
			.ComFilterMin = 0,
			.ComFilterOffset = 0,
			.ComFilterPeriodFactor = 0,
			.ComFilterX = 0
		}
	},
	{
		.ComBitPosition = 16,
		.ComBitSize = 1,
		//.ComDataInvalidAction (NOT IMPLEMENTED)
		.ComErrorNotification = NULL,
		.ComFirstTimeoutFactor = 0,
		.ComHandleId = 6,
		//.ComInvalidNotification (NOT IMPLEMENTED)
		.ComNotification = NULL,
		//.ComRxDataTimeoutAction (NOT IMPLEMENTED)
		//.ComSignalDataInvalidValue (NOT IMPLEMENTED)
		.ComSignalEndianess = BIG_ENDIAN,
		.ComSignalInitValue = 0x00,
		.ComSignalLength = 0,
		.ComSignalType = UINT8,
		.ComTimeoutFactor = 0,
		.ComTimeoutNotification = NULL,
		.ComTransferProperty = TRIGGERED,
		.ComUpdateBitPosition = 0,
		.ComSignalArcUseUpdateBit = 0,
		.ComFilter = {
			.ComFilterAlgorithm = ALWAYS,
			.ComFilterMask = 0,
			.ComFilterMax = 0,
			.ComFilterMin = 0,
			.ComFilterOffset = 0,
			.ComFilterPeriodFactor = 0,
			.ComFilterX = 0
		}
	},
	{
		.ComBitPosition = 17,
		.ComBitSize = 2,
		//.ComDataInvalidAction (NOT IMPLEMENTED)
		.ComErrorNotification = NULL,
		.ComFirstTimeoutFactor = 0,
		.ComHandleId = 7,
		//.ComInvalidNotification (NOT IMPLEMENTED)
		.ComNotification = NULL,
		//.ComRxDataTimeoutAction (NOT IMPLEMENTED)
		//.ComSignalDataInvalidValue (NOT IMPLEMENTED)
		.ComSignalEndianess = BIG_ENDIAN,
		.ComSignalInitValue = 0x00,
		.ComSignalLength = 0,
		.ComSignalType = UINT8,
		.ComTimeoutFactor = 0,
		.ComTimeoutNotification = NULL,
		.ComTransferProperty = TRIGGERED,
		.ComUpdateBitPosition = 0,
		.ComSignalArcUseUpdateBit = 0,
		.ComFilter = {
			.ComFilterAlgorithm = ALWAYS,
			.ComFilterMask = 0,
			.ComFilterMax = 0,
			.ComFilterMin = 0,
			.ComFilterOffset = 0,
			.ComFilterPeriodFactor = 0,
			.ComFilterX = 0
		}
	},

	// Signals for PCAN_TX_FRAME_1
	{
		.ComBitPosition = 0,
		.ComBitSize = 8,
		//.ComDataInvalidAction (NOT IMPLEMENTED)
		.ComErrorNotification = NULL,
		.ComFirstTimeoutFactor = 0,
		.ComHandleId = 8,
		//.ComInvalidNotification (NOT IMPLEMENTED)
		.ComNotification = NULL,
		//.ComRxDataTimeoutAction (NOT IMPLEMENTED)
		//.ComSignalDataInvalidValue (NOT IMPLEMENTED)
		.ComSignalEndianess = BIG_ENDIAN,
		.ComSignalInitValue = 0x0,
		.ComSignalLength = 0,
		.ComSignalType = UINT8,
		.ComTimeoutFactor = 0,
		.ComTimeoutNotification = NULL,
		.ComTransferProperty = TRIGGERED,
		.ComUpdateBitPosition = 0,
		.ComSignalArcUseUpdateBit = 0,
		.ComFilter = {
			.ComFilterAlgorithm = ALWAYS,
			.ComFilterMask = 0,
			.ComFilterMax = 0,
			.ComFilterMin = 0,
			.ComFilterOffset = 0,
			.ComFilterPeriodFactor = 0,
			.ComFilterX = 0
		}
	},
	{
		.ComBitPosition = 8,
		.ComBitSize = 8,
		//.ComDataInvalidAction (NOT IMPLEMENTED)
		.ComErrorNotification = NULL,
		.ComFirstTimeoutFactor = 0,
		.ComHandleId = 9,
		//.ComInvalidNotification (NOT IMPLEMENTED)
		.ComNotification = NULL,
		//.ComRxDataTimeoutAction (NOT IMPLEMENTED)
		//.ComSignalDataInvalidValue (NOT IMPLEMENTED)
		.ComSignalEndianess = BIG_ENDIAN,
		.ComSignalInitValue = 0x00,
		.ComSignalLength = 0,
		.ComSignalType = UINT8,
		.ComTimeoutFactor = 0,
		.ComTimeoutNotification = NULL,
		.ComTransferProperty = TRIGGERED,
		.ComUpdateBitPosition = 0,
		.ComSignalArcUseUpdateBit = 0,
		.ComFilter = {
			.ComFilterAlgorithm = ALWAYS,
			.ComFilterMask = 0,
			.ComFilterMax = 0,
			.ComFilterMin = 0,
			.ComFilterOffset = 0,
			.ComFilterPeriodFactor = 0,
			.ComFilterX = 0
		}
	},
	{
		.ComBitPosition = 16,
		.ComBitSize = 1,
		//.ComDataInvalidAction (NOT IMPLEMENTED)
		.ComErrorNotification = NULL,
		.ComFirstTimeoutFactor = 0,
		.ComHandleId = 10,
		//.ComInvalidNotification (NOT IMPLEMENTED)
		.ComNotification = NULL,
		//.ComRxDataTimeoutAction (NOT IMPLEMENTED)
		//.ComSignalDataInvalidValue (NOT IMPLEMENTED)
		.ComSignalEndianess = BIG_ENDIAN,
		.ComSignalInitValue = 0x00,
		.ComSignalLength = 0,
		.ComSignalType = UINT8,
		.ComTimeoutFactor = 0,
		.ComTimeoutNotification = NULL,
		.ComTransferProperty = TRIGGERED,
		.ComUpdateBitPosition = 0,
		.ComSignalArcUseUpdateBit = 0,
		.ComFilter = {
			.ComFilterAlgorithm = ALWAYS,
			.ComFilterMask = 0,
			.ComFilterMax = 0,
			.ComFilterMin = 0,
			.ComFilterOffset = 0,
			.ComFilterPeriodFactor = 0,
			.ComFilterX = 0
		}
	},
	{
		.ComBitPosition = 17,
		.ComBitSize = 2,
		//.ComDataInvalidAction (NOT IMPLEMENTED)
		.ComErrorNotification = NULL,
		.ComFirstTimeoutFactor = 0,
		.ComHandleId = 11,
		//.ComInvalidNotification (NOT IMPLEMENTED)
		.ComNotification = NULL,
		//.ComRxDataTimeoutAction (NOT IMPLEMENTED)
		//.ComSignalDataInvalidValue (NOT IMPLEMENTED)
		.ComSignalEndianess = BIG_ENDIAN,
		.ComSignalInitValue = 0x00,
		.ComSignalLength = 0,
		.ComSignalType = UINT8,
		.ComTimeoutFactor = 0,
		.ComTimeoutNotification = NULL,
		.ComTransferProperty = TRIGGERED,
		.ComUpdateBitPosition = 0,
		.ComSignalArcUseUpdateBit = 0,
		.ComFilter = {
			.ComFilterAlgorithm = ALWAYS,
			.ComFilterMask = 0,
			.ComFilterMax = 0,
			.ComFilterMin = 0,
			.ComFilterOffset = 0,
			.ComFilterPeriodFactor = 0,
			.ComFilterX = 0
		}
	},

	// Signals for PCAN_TX_FRAME_2
	{
		.ComBitPosition = 0,
		.ComBitSize = 8,
		//.ComDataInvalidAction (NOT IMPLEMENTED)
		.ComErrorNotification = NULL,
		.ComFirstTimeoutFactor = 0,
		.ComHandleId = 12,
		//.ComInvalidNotification (NOT IMPLEMENTED)
		.ComNotification = NULL,
		//.ComRxDataTimeoutAction (NOT IMPLEMENTED)
		//.ComSignalDataInvalidValue (NOT IMPLEMENTED)
		.ComSignalEndianess = BIG_ENDIAN,
		.ComSignalInitValue = 0x0,
		.ComSignalLength = 0,
		.ComSignalType = UINT8,
		.ComTimeoutFactor = 0,
		.ComTimeoutNotification = NULL,
		.ComTransferProperty = TRIGGERED,
		.ComUpdateBitPosition = 0,
		.ComSignalArcUseUpdateBit = 0,
		.ComFilter = {
			.ComFilterAlgorithm = ALWAYS,
			.ComFilterMask = 0,
			.ComFilterMax = 0,
			.ComFilterMin = 0,
			.ComFilterOffset = 0,
			.ComFilterPeriodFactor = 0,
			.ComFilterX = 0
		}
	},
	{
		.ComBitPosition = 8,
		.ComBitSize = 8,
		//.ComDataInvalidAction (NOT IMPLEMENTED)
		.ComErrorNotification = NULL,
		.ComFirstTimeoutFactor = 0,
		.ComHandleId = 13,
		//.ComInvalidNotification (NOT IMPLEMENTED)
		.ComNotification = NULL,
		//.ComRxDataTimeoutAction (NOT IMPLEMENTED)
		//.ComSignalDataInvalidValue (NOT IMPLEMENTED)
		.ComSignalEndianess = BIG_ENDIAN,
		.ComSignalInitValue = 0x00,
		.ComSignalLength = 0,
		.ComSignalType = UINT8,
		.ComTimeoutFactor = 0,
		.ComTimeoutNotification = NULL,
		.ComTransferProperty = TRIGGERED,
		.ComUpdateBitPosition = 0,
		.ComSignalArcUseUpdateBit = 0,
		.ComFilter = {
			.ComFilterAlgorithm = ALWAYS,
			.ComFilterMask = 0,
			.ComFilterMax = 0,
			.ComFilterMin = 0,
			.ComFilterOffset = 0,
			.ComFilterPeriodFactor = 0,
			.ComFilterX = 0
		}
	},
	{
		.ComBitPosition = 16,
		.ComBitSize = 1,
		//.ComDataInvalidAction (NOT IMPLEMENTED)
		.ComErrorNotification = NULL,
		.ComFirstTimeoutFactor = 0,
		.ComHandleId = 14,
		//.ComInvalidNotification (NOT IMPLEMENTED)
		.ComNotification = NULL,
		//.ComRxDataTimeoutAction (NOT IMPLEMENTED)
		//.ComSignalDataInvalidValue (NOT IMPLEMENTED)
		.ComSignalEndianess = BIG_ENDIAN,
		.ComSignalInitValue = 0x00,
		.ComSignalLength = 0,
		.ComSignalType = UINT8,
		.ComTimeoutFactor = 0,
		.ComTimeoutNotification = NULL,
		.ComTransferProperty = TRIGGERED,
		.ComUpdateBitPosition = 0,
		.ComSignalArcUseUpdateBit = 0,
		.ComFilter = {
			.ComFilterAlgorithm = ALWAYS,
			.ComFilterMask = 0,
			.ComFilterMax = 0,
			.ComFilterMin = 0,
			.ComFilterOffset = 0,
			.ComFilterPeriodFactor = 0,
			.ComFilterX = 0
		}
	},
	{
		.ComBitPosition = 17,
		.ComBitSize = 2,
		//.ComDataInvalidAction (NOT IMPLEMENTED)
		.ComErrorNotification = NULL,
		.ComFirstTimeoutFactor = 0,
		.ComHandleId = 15,
		//.ComInvalidNotification (NOT IMPLEMENTED)
		.ComNotification = NULL,
		//.ComRxDataTimeoutAction (NOT IMPLEMENTED)
		//.ComSignalDataInvalidValue (NOT IMPLEMENTED)
		.ComSignalEndianess = BIG_ENDIAN,
		.ComSignalInitValue = 0x00,
		.ComSignalLength = 0,
		.ComSignalType = UINT8,
		.ComTimeoutFactor = 0,
		.ComTimeoutNotification = NULL,
		.ComTransferProperty = TRIGGERED,
		.ComUpdateBitPosition = 0,
		.ComSignalArcUseUpdateBit = 0,
		.ComFilter = {
			.ComFilterAlgorithm = ALWAYS,
			.ComFilterMask = 0,
			.ComFilterMax = 0,
			.ComFilterMin = 0,
			.ComFilterOffset = 0,
			.ComFilterPeriodFactor = 0,
			.ComFilterX = 0
		}
	},
	{
		.Com_Arc_EOL  = 1
	}
};

// Definitions of IPDU groups.
ComIPduGroup_type PCAN_ComIPduGroup[] = {
	{
		.ComIPduGroupHandleId = 0
	},
	{
		.Com_Arc_EOL  = 1
	}
};

ComIPdu_type PCAN_ComIPdu[] = {
	// This CAN-message is used to update properties of the engine.
	{
		.ComIPduCallout = NULL,
		.ComIPduRxHandleId = PCAN_RX_FRAME_1,
		.ComIPduSignalProcessing = IMMEDIATE,
		.ComIPduSize = 8,
		.ComIPduDirection = RECEIVE,
		.ComIPduGroupRef = &PCAN_ComIPduGroup[0],
		//.ComIPduSignalGroupRef (NOT IMPLEMENTED)
		//./PduIdRef (NOT IMPLEMENTED)
		.ComTxIPdu = {NULL},
		.ComIPduSignalRef = {
			&PCAN_ComSignal[0],
			&PCAN_ComSignal[1],
			&PCAN_ComSignal[2],
			&PCAN_ComSignal[3],
			NULL,
		},
	},
	// This CAN-message is used to update properties of the engine.
	{
		.ComIPduCallout = NULL,
		.ComIPduRxHandleId = PCAN_RX_FRAME_2,
		.ComIPduSignalProcessing = IMMEDIATE,
		.ComIPduSize = 8,
		.ComIPduDirection = RECEIVE,
		.ComIPduGroupRef = &PCAN_ComIPduGroup[0],
		//.ComIPduSignalGroupRef (NOT IMPLEMENTED)
		//./PduIdRef (NOT IMPLEMENTED)
		.ComTxIPdu = {NULL},
		.ComIPduSignalRef = {
			&PCAN_ComSignal[4],
			&PCAN_ComSignal[5],
			&PCAN_ComSignal[6],
			&PCAN_ComSignal[7],
			NULL,
		},
	},
	{
		.ComIPduCallout = NULL,
		.ComIPduRxHandleId = PCAN_TX_FRAME_1,
		.ComIPduSignalProcessing = IMMEDIATE,
		.ComIPduSize = 8,
		.ComIPduDirection = SEND,
		.ComIPduGroupRef = &PCAN_ComIPduGroup[0],
		//.ComIPduSignalGroupRef (NOT IMPLEMENTED)
		//./PduIdRef (NOT IMPLEMENTED)
		.ComTxIPdu = {
			.ComTxIPduMinimumDelayFactor = 0,
			.ComTxIPduUnusedAreasDefault = 0x00,
			.ComTxModeTrue = {
				.ComTxModeMode = DIRECT,
				.ComTxModeNumberOfRepetitions = 0,
				.ComTxModeRepetitionPeriodFactor = 0,
				.ComTxModeTimeOffsetFactor = 0,
				.ComTxModeTimePeriodFactor = 0,
			},
			//.ComTxModeFalse (NOT IMPLEMENTED)
		},
		.ComIPduSignalRef = {
			&PCAN_ComSignal[8],
			&PCAN_ComSignal[9],
			&PCAN_ComSignal[10],
			&PCAN_ComSignal[11],
			NULL,
		},
	},
	{
		.ComIPduCallout = NULL,
		.ComIPduRxHandleId = PCAN_TX_FRAME_2,
		.ComIPduSignalProcessing = IMMEDIATE,
		.ComIPduSize = 8,
		.ComIPduDirection = SEND,
		.ComIPduGroupRef = &PCAN_ComIPduGroup[0],
		//.ComIPduSignalGroupRef (NOT IMPLEMENTED)
		//./PduIdRef (NOT IMPLEMENTED)
		.ComTxIPdu = {
			.ComTxIPduMinimumDelayFactor = 0,
			.ComTxIPduUnusedAreasDefault = 0x00,
			.ComTxModeTrue = {
				.ComTxModeMode = DIRECT,
				.ComTxModeNumberOfRepetitions = 0,
				.ComTxModeRepetitionPeriodFactor = 0,
				.ComTxModeTimeOffsetFactor = 0,
				.ComTxModeTimePeriodFactor = 0,
			},
			//.ComTxModeFalse (NOT IMPLEMENTED)
		},
		.ComIPduSignalRef = {
			&PCAN_ComSignal[12],
			&PCAN_ComSignal[13],
			&PCAN_ComSignal[14],
			&PCAN_ComSignal[15],
			NULL,
		},
	},

	{
		.Com_Arc_EOL  = 1
	}
};

Com_ConfigType PCAN_ComConfig = {
	.ComConfigurationId = 2,
	.ComIPdu = PCAN_ComIPdu,
	.ComIPduGroup = PCAN_ComIPduGroup,
	.ComSignal = PCAN_ComSignal,
};






/*
 * Target test configuration
 */
ComGroupSignal_type HardwareTest_ComGroupSignal[] = {
	{
		.ComBitPosition = 32,
		.ComBitSize	= 8,
		.ComHandleId = 0,
		.ComSignalType = UINT8,
	},
	{
		.ComBitPosition = 40,
		.ComBitSize	= 2,
		.ComHandleId = 1,
		.ComSignalType = UINT8,
	}
};

ComSignal_type HardwareTest_ComSignal[] = {
	{
		.ComBitPosition = 0,
		.ComBitSize = 16,
		//.ComDataInvalidAction (NOT IMPLEMENTED)
		.ComErrorNotification = NULL,
		.ComFirstTimeoutFactor = 0,
		.ComHandleId = 0,
		//.ComInvalidNotification (NOT IMPLEMENTED)
		.ComNotification = NULL,
		//.ComRxDataTimeoutAction (NOT IMPLEMENTED)
		//.ComSignalDataInvalidValue (NOT IMPLEMENTED)
		.ComSignalEndianess = BIG_ENDIAN,
		.ComSignalInitValue = 0xFF,
		.ComSignalLength = 0,
		.ComSignalType = UINT16,
		.ComTimeoutFactor = 0,
		.ComTimeoutNotification = NULL,
		.ComTransferProperty = TRIGGERED,
		.ComUpdateBitPosition = 16,
		.ComSignalArcUseUpdateBit = 1,
		.ComFilter = {
			.ComFilterAlgorithm = ALWAYS,
			.ComFilterMask = 0,
			.ComFilterMax = 0,
			.ComFilterMin = 0,
			.ComFilterOffset = 0,
			.ComFilterPeriodFactor = 0,
			.ComFilterX = 0
		}
	},
	{
		.ComBitPosition = 0,
		.ComBitSize = 16,
		//.ComDataInvalidAction (NOT IMPLEMENTED)
		.ComErrorNotification = NULL,
		.ComFirstTimeoutFactor = 0,
		.ComHandleId = 1,
		//.ComInvalidNotification (NOT IMPLEMENTED)
		.ComNotification = RTE_EngineChangeSpeed,
		//.ComRxDataTimeoutAction (NOT IMPLEMENTED)
		//.ComSignalDataInvalidValue (NOT IMPLEMENTED)
		.ComSignalEndianess = BIG_ENDIAN,
		.ComSignalInitValue = 0x00,
		.ComSignalLength = 0,
		.ComSignalType = UINT16,
		.ComTimeoutFactor = 0,
		.ComTimeoutNotification = NULL,
		.ComTransferProperty = TRIGGERED,
		.ComUpdateBitPosition = 16,
		.ComSignalArcUseUpdateBit = 1,
		.ComFilter = {
			.ComFilterAlgorithm = ALWAYS,
			.ComFilterMask = 0,
			.ComFilterMax = 0,
			.ComFilterMin = 0,
			.ComFilterOffset = 0,
			.ComFilterPeriodFactor = 0,
			.ComFilterX = 0
		}
	},

	// This is a signal group
	{
		//.ComBitPosition = 0,
		//.ComBitSize = 16,
		//.ComDataInvalidAction (NOT IMPLEMENTED)
		//.ComErrorNotification = NULL,
		//.ComFirstTimeoutFactor = 0,
		.ComHandleId = 2,
		//.ComInvalidNotification (NOT IMPLEMENTED)
		.ComNotification = RTE_SIL2MESSAGE,
		//.ComRxDataTimeoutAction (NOT IMPLEMENTED)
		//.ComSignalDataInvalidValue (NOT IMPLEMENTED)
		//.ComSignalEndianess = BIG_ENDIAN,
		//.ComSignalInitValue = 0xFF,
		//.ComSignalLength = 0,
		//.ComSignalType = UINT16,
		//.ComTimeoutFactor = 0,
		//.ComTimeoutNotification = NULL,
		//.ComTransferProperty = TRIGGERED,
		.ComUpdateBitPosition = 42,
		.ComSignalArcUseUpdateBit = 1,
		.Com_Arc_IsSignalGroup = 1,
		.ComGroupSignal = {
			&HardwareTest_ComGroupSignal[0],
			&HardwareTest_ComGroupSignal[1],
		},
		/*.ComFilter = {
			.ComFilterAlgorithm = ALWAYS,
			.ComFilterMask = 0,
			.ComFilterMax = 0,
			.ComFilterMin = 0,
			.ComFilterOffset = 0,
			.ComFilterPeriodFactor = 0,
			.ComFilterX = 0
		}*/
	},
	{
		.Com_Arc_EOL  = 1
	}
};

// Definitions of IPDU groups.
ComIPduGroup_type HardwareTest_ComIPduGroup[] = {
	{
		.ComIPduGroupHandleId = 0
	},
	{
		.ComIPduGroupHandleId = 1
	},
	{
		.ComIPduGroupHandleId = 2
	},
	{
		.Com_Arc_EOL  = 1
	}
};

ComIPdu_type HardwareTest_ComIPdu[] = {
	{
		.ComIPduCallout = NULL,
		.ComIPduRxHandleId = PDU_MSG_HARDWARE_TEST_ENGINE_STATUS,
		.ComIPduSignalProcessing = IMMEDIATE,
		.ComIPduSize = 8,
		.ComIPduDirection = SEND,
		.ComIPduGroupRef = &HardwareTest_ComIPduGroup[0],
		//.ComIPduSignalGroupRef (NOT IMPLEMENTED)
		//./PduIdRef (NOT IMPLEMENTED)
		.ComTxIPdu = {
			.ComTxIPduMinimumDelayFactor = 500,
			.ComTxIPduUnusedAreasDefault = 0x00,
			.ComTxModeTrue = {
				.ComTxModeMode = MIXED,
				.ComTxModeNumberOfRepetitions = 0,
				.ComTxModeRepetitionPeriodFactor = 0,
				.ComTxModeTimeOffsetFactor = 400,
				.ComTxModeTimePeriodFactor = 1000,
			},
			//.ComTxModeFalse (NOT IMPLEMENTED)
		},
		.ComIPduSignalRef = {
			&HardwareTest_ComSignal[0],
			NULL,
		},
	},

	// This CAN-message is used to update properties of the engine.
	{
		.ComIPduCallout = NULL,
		.ComIPduRxHandleId = PDU_MSG_HARDWARE_TEST_ENGINE_CONTROL,
		.ComIPduSignalProcessing = IMMEDIATE,
		.ComIPduSize = 8,
		.ComIPduDirection = RECEIVE,
		.ComIPduGroupRef = &HardwareTest_ComIPduGroup[0],
		//.ComIPduSignalGroupRef (NOT IMPLEMENTED)
		//./PduIdRef (NOT IMPLEMENTED)
		.ComTxIPdu = {NULL},
		.ComIPduSignalRef = {
			&HardwareTest_ComSignal[1],
			&HardwareTest_ComSignal[2],
			NULL,
		},
	},


	// This is a gateway mode message on the PDU router level.
	{
		.ComIPduCallout = NULL,
		.ComIPduRxHandleId = PDU_MSG_HARDWARE_TEST_ROUTED_MSG,
		.ComIPduSize = 0,
		.ComIPduDirection = RECEIVE,
		.ComIPduGroupRef = &HardwareTest_ComIPduGroup[0],
		//.ComIPduSignalGroupRef (NOT IMPLEMENTED)
		//./PduIdRef (NOT IMPLEMENTED)
		.ComTxIPdu = {NULL},
		.ComIPduSignalRef = {
			NULL
		},
	},
	{
		.Com_Arc_EOL  = 1
	}
};

Com_ConfigType HardwareTest_ComConfig = {
	.ComConfigurationId = 2,
	.ComIPdu = HardwareTest_ComIPdu,
	.ComIPduGroup = HardwareTest_ComIPduGroup,
	.ComSignal = HardwareTest_ComSignal,
	.ComGroupSignal = HardwareTest_ComGroupSignal
};



// TEST CONFIGURATION


