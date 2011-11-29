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


#ifndef COM_ARC_TYPES_H_
#define COM_ARC_TYPES_H_

#include "Std_Types.h"
#include "Com_Types.h"


typedef struct {
	uint32 ComFilterArcN;
	uint32 ComFilterArcNewValue;
	uint32 ComFilterArcOldValue;
} Com_Arc_Filter_type;

typedef struct {

	uint32 Com_Arc_DeadlineCounter;
	uint8 ComSignalUpdated;
} Com_Arc_Signal_type;


typedef struct {
	void *Com_Arc_ShadowBuffer;
	uint8 ComSignalUpdated;
	uint8 Com_Arc_EOL;
} Com_Arc_GroupSignal_type;

typedef struct {
	uint8  ComTxIPduNumberOfRepetitionsLeft;
	uint32 ComTxModeRepetitionPeriodTimer;
	uint32 ComTxIPduMinimumDelayTimer;
	uint32 ComTxModeTimePeriodTimer;
} Com_Arc_TxIPduTimer_type;

typedef struct {

	Com_Arc_TxIPduTimer_type Com_Arc_TxIPduTimers;
	uint8 Com_Arc_IpduStarted;
	uint16 Com_Arc_DynSignalLength;
} Com_Arc_IPdu_type;

typedef struct {
	Com_Arc_IPdu_type *ComIPdu; // Only used in PduIdCheck()
	Com_Arc_Signal_type *ComSignal;
	Com_Arc_GroupSignal_type *ComGroupSignal;
} Com_Arc_Config_type;

#endif
