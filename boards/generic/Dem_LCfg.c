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








#include "Dem.h"

/*
 * DEM Configuration
 */

// Read extended data callback function declarations
Std_ReturnType GetExtendedDataRecord_0x01(uint8 *Data) {
	*Data = 11;

	return E_OK;
}

Std_ReturnType GetExtendedDataRecord_0x02(uint8 *Data) {
	Data[0] = 21;
	Data[1] = 22;

	return E_OK;
}

Std_ReturnType GetExtendedDataRecord_0x03(uint8 *Data) {
	Data[0] = 31;
	Data[1] = 32;
	Data[2] = 33;

	return E_OK;
}

// Read fault detection counter callback function declarations
Std_ReturnType GetFaultDetectionCounter1(sint8 *EventIdFaultDetectionCounter)
{
	*EventIdFaultDetectionCounter = -11;

	return E_OK;
}

/*
 * Classes of extended data record
 */
const Dem_ExtendedDataRecordClassType ExtendedDataRecordClassList[] = {
		{
			.RecordNumber = 1,	// Unique!
			.DataSize = 1,
			.CallbackGetExtDataRecord = GetExtendedDataRecord_0x01
		},
		{
			.RecordNumber = 2,
			.DataSize = 3,
			.CallbackGetExtDataRecord = GetExtendedDataRecord_0x03
		},
		{
			.RecordNumber = 3,
			.DataSize = 2,
			.CallbackGetExtDataRecord = GetExtendedDataRecord_0x02
		}
};

/*
 * Classes of extended data
 */
const Dem_ExtendedDataClassType ExtendedDataClass1 = {
		.ExtendedDataRecordClassRef = {
			&ExtendedDataRecordClassList[0],
			NULL	// End of list mark
		}
};

const Dem_ExtendedDataClassType ExtendedDataClass2 = {
		.ExtendedDataRecordClassRef = {
			&ExtendedDataRecordClassList[0],
			&ExtendedDataRecordClassList[1],
			&ExtendedDataRecordClassList[2],
			NULL	// End of list mark
		}
};

/*
 * Classes of freeze frames
 */
const Dem_FreezeFrameClassType FreezeFrameClassList[] = {
		{
				.FFKind = DEM_FREEZE_FRAME_NON_OBD,
//				.FFRecordNum;	// Optional
//				.FFIdClassRef; // Optional
		},
		{
				.FFKind = DEM_FREEZE_FRAME_NON_OBD,
//				.FFRecordNum;	// Optional
//				.FFIdClassRef; // Optional
		}
};

/*
 * Classes of PreDebounce algorithms
 */
const Dem_PreDebounceMonitorInternalType PreDebounceMonitorInternal1 = {
		.CallbackGetFDCnt = &GetFaultDetectionCounter1
};

const Dem_PreDebounceAlgorithmClassType PreDebounceAlgorithmClass1 = {
		.PreDebounceName = DEM_NO_PRE_DEBOUNCE,
		.PreDebounceAlgorithm = {
				.PreDebounceMonitorInternal = &PreDebounceMonitorInternal1
		}
};


/*
 * Classes of event
 */
const Dem_EventClassType EventClass1 = {
		.ConsiderPtoStatus = FALSE,
		.EventDestination = {
				NULL
		},
		.FFPrestorageSupported = FALSE,
		.HealingAllowed = FALSE,
		.OperationCycleRef = DEM_IGNITION,
		.PreDebounceAlgorithmClass = &PreDebounceAlgorithmClass1
};

const Dem_EventClassType EventClass2 = {
		.ConsiderPtoStatus = FALSE,
		.EventDestination = {
				DEM_DTC_ORIGIN_PRIMARY_MEMORY,
				NULL
		},
		.FFPrestorageSupported = FALSE,
		.HealingAllowed = FALSE,
		.OperationCycleRef = DEM_POWER,
		.PreDebounceAlgorithmClass = NULL
};

const Dem_EventParameterType EventParameter[] = {
		{
				.EventID = DEM_EVENT_ID_TEMP_HIGH,
				.EventKind = DEM_EVENT_KIND_SWC,
				.EventClass = &EventClass2,
				.ExtendedDataClassRef = NULL,
				.FreezeFrameClassRef = NULL,
				.CallbackInitMforE = NULL,
				.CallbackEventStatusChanged = NULL,
				.DTCClass = NULL
		},
		{
				.EventID = DEM_EVENT_ID_TEMP_LOW,
				.EventKind = DEM_EVENT_KIND_SWC,
				.EventClass = &EventClass1,
				.ExtendedDataClassRef = &ExtendedDataClass1,
				.FreezeFrameClassRef = NULL,
				.CallbackInitMforE = NULL,
				.CallbackEventStatusChanged = NULL,
				.DTCClass = NULL
		},
		{
				.EventID = DEM_EVENT_ID_BLOW,
				.EventKind = DEM_EVENT_KIND_BSW,
				.EventClass = &EventClass2,
				.ExtendedDataClassRef = &ExtendedDataClass2,
				.FreezeFrameClassRef = NULL,
				.CallbackInitMforE = NULL,
				.CallbackEventStatusChanged = NULL,
				.DTCClass = NULL
		},
		{
				.Arc_EOL  = TRUE
		}

};

const Dem_ConfigSetType DEM_ConfigSet = {
		.EventParameter = EventParameter,
//		.DTCClassType = NULL,		TODO: Add later
//		.GroupOfDtc = NULL,			TODO: Add later
//		.OemIdClass = NULL			TODO: Add later
};


const Dem_ConfigType DEM_Config = {
	.ConfigSet = &DEM_ConfigSet,
};
