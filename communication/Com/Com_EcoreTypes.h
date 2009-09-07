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
	uint32 ComFilterEcoreN;
	uint32 ComFilterEcoreNewValue;
	uint32 ComFilterEcoreOldValue;
} ComEcoreFilter_type;

typedef struct {

	ComEcoreFilter_type ComFilter;

	uint32 ComEcoreDeadlineCounter;
	uint32 ComTimeoutFactor;
	void *ComIPduDataPtr;

	uint8 ComIPduHandleId;
	uint8 ComSignalUpdated;
	//uint8 ComEcoreEOL;
	//uint8 ComEcoreIsSignalGroup;

	/* For signal groups */
	void *ComEcoreShadowBuffer;

} ComEcoreSignal_type;


typedef struct {
	void *ComEcoreShadowBuffer;
	//uint8 ComIPduHandleId;

	uint8 ComSignalUpdated;
	uint8 ComEcoreEOL;
} ComEcoreGroupSignal_type;


/*
typedef struct {
	void *ComEcoreShadowBuffer;
	void *ComEcoreIPduDataPtr;
	uint8 ComEcoreEOL;
} ComEcoreSignalGroup_type;
*/

typedef struct {
	uint8  ComTxIPduNumberOfRepetitionsLeft;
	uint32 ComTxModeRepetitionPeriodTimer;
	uint32 ComTxIPduMinimumDelayTimer;
	uint32 ComTxModeTimePeriodTimer;
} ComEcoreTxIPduTimer_type;

typedef struct {

	ComEcoreTxIPduTimer_type ComEcoreTxIPduTimers;
	void *ComIPduDataPtr;

	uint8 ComEcoreNIPduSignalGroupRef;

	uint8 NComIPduSignalRef;

	uint8 ComEcoreIpduStarted;

} ComEcoreIPdu_type;

typedef struct {
	uint16 ComNIPdu;
	ComEcoreIPdu_type *ComIPdu; // Only used in PduIdCheck()
	//ComEcoreIPduGroup_type *ComIPduGroup;
	ComEcoreSignal_type *ComSignal;
	//ComEcoreSignalGroup_type *ComSignalGroup;
	ComEcoreGroupSignal_type *ComGroupSignal;
	PduInfoType OutgoingPdu;
} ComEcoreConfig_type;
