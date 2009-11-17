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
void ExtendedDataRecord1GetFunc(uint8 *Data) {
	*Data = 11;
}

void ExtendedDataRecord2GetFunc(uint8 *Data) {
	Data[0] = 21;
	Data[1] = 22;
}

void ExtendedDataRecord3GetFunc(uint8 *Data) {
	Data[0] = 31;
	Data[1] = 32;
	Data[2] = 33;
}

/*
 * Classes of extended data record
 */
Dem_ExtendedDataRecordClassType ExtendedDataRecordClassList[] = {
		{
			.RecordNumber = 1,	// Unique!
			.DataSize = 1,
			.CallbackGetExtDataRecord = ExtendedDataRecord1GetFunc
		},
		{
			.RecordNumber = 2,
			.DataSize = 3,
			.CallbackGetExtDataRecord = ExtendedDataRecord3GetFunc
		},
		{
			.RecordNumber = 3,
			.DataSize = 2,
			.CallbackGetExtDataRecord = ExtendedDataRecord2GetFunc
		}
};

/*
 * Classes of extended data
 */
Dem_ExtendedDataClassType ExtendedDataClass1 = {
		.ExtendedDataRecordClassRef = {
			&ExtendedDataRecordClassList[0],
			NULL
		}
};

Dem_ExtendedDataClassType ExtendedDataClass2 = {
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
Dem_FreezeFrameClassType FreezeFrameClassList[] = {
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
 * Classes of event
 */
Dem_EventClassType EventClass1 = {
		.ConsiderPtoStatus = FALSE,
		.EventDestination = {
				NULL
		},
		.FFPrestorageSupported = FALSE,
		.HealingAllowed = FALSE
};

Dem_EventClassType EventClass2 = {
		.ConsiderPtoStatus = FALSE,
		.EventDestination = {
				DEM_DTC_ORIGIN_PRIMARY_MEMORY,
				NULL
		},
		.FFPrestorageSupported = FALSE,
		.HealingAllowed = FALSE
};

Dem_EventParameterType EventParameter[] = {
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
				.EcoreEOL  = 1
		}

};

Dem_ConfigSetType DEM_ConfigSet = {
		.EventParameter = EventParameter,
//		.DTCClassType = NULL,
//		.GroupOfDtc = NULL,
//		.OemIdClass = NULL
};


Dem_ConfigType DEM_Config = {
	.ConfigSet = &DEM_ConfigSet,
};





