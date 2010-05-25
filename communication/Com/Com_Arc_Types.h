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








typedef struct {
	uint32 ComFilterArcN;
	uint32 ComFilterArcNewValue;
	uint32 ComFilterArcOldValue;
} Com_Arc_Filter_type;

typedef struct {

	Com_Arc_Filter_type ComFilter;

	uint32 Com_Arc_DeadlineCounter;
	uint32 ComTimeoutFactor;
	void *ComIPduDataPtr;

	uint8 ComIPduHandleId;
	uint8 ComSignalUpdated;
	//uint8 Com_Arc_EOL;
	//uint8 Com_Arc_IsSignalGroup;

	/* For signal groups */
	void *Com_Arc_ShadowBuffer;

} Com_Arc_Signal_type;


typedef struct {
	void *Com_Arc_ShadowBuffer;
	//uint8 ComIPduHandleId;

	uint8 ComSignalUpdated;
	uint8 Com_Arc_EOL;
} Com_Arc_GroupSignal_type;


/*
typedef struct {
	void *Com_Arc_ShadowBuffer;
	void *Com_Arc_IPduDataPtr;
	uint8 Com_Arc_EOL;
} Com_Arc_SignalGroup_type;
*/

typedef struct {
	uint8  ComTxIPduNumberOfRepetitionsLeft;
	uint32 ComTxModeRepetitionPeriodTimer;
	uint32 ComTxIPduMinimumDelayTimer;
	uint32 ComTxModeTimePeriodTimer;
} Com_Arc_TxIPduTimer_type;

typedef struct {

	Com_Arc_TxIPduTimer_type Com_Arc_TxIPduTimers;
	void *ComIPduDataPtr;

	uint8 Com_Arc_NIPduSignalGroupRef;

	uint8 NComIPduSignalRef;

	uint8 Com_Arc_IpduStarted;

} Com_Arc_IPdu_type;

typedef struct {
	uint16 ComNIPdu;
	Com_Arc_IPdu_type *ComIPdu; // Only used in PduIdCheck()
	//Com_Arc_IPduGroup_type *ComIPduGroup;
	Com_Arc_Signal_type *ComSignal;
	//Com_Arc_SignalGroup_type *ComSignalGroup;
	Com_Arc_GroupSignal_type *ComGroupSignal;
	PduInfoType OutgoingPdu;
} Com_Arc_Config_type;
