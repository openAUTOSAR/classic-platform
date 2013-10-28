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








#ifndef LINIF_CFG_H_
#define LINIF_CFG_H_

#include "Std_Types.h"
#include "Lin_Cfg.h"

// PDU definitions
enum {
	LINIF_PDU_ID_ENG_TX = 0, // Changed by Mattias original value 101
	LINIF_PDU_ID_LAMPS_RX,
	LINIF_PDU_ID_TCM_RX
};

/* Switches the Development Error Detection and Notification
ON or OFF. */
#define LINIF_DEV_ERROR_DETECT STD_ON

/* States if multiple drivers are included in the LIN Interface or not. The
 * reason for this parameter is to reduce the size of LIN Interface if multiple
 * drivers are not used. */
#define LINIF_MULTIPLE_DRIVER_SUPPORT STD_OFF

/* States if the node configuration commands Assign NAD and Conditional
 * Change NAD are supported. */
#define LINIF_OPTIONAL_REQUEST_SUPPORTED STD_OFF

/* States if the TP is included in the LIN Interface or not. The reason for this
 * parameter is to reduce the size of LIN Interface if the TP is not used. */
#define LINIF_TP_SUPPORTED STD_OFF

/* Switches the LinIf_GetVersionInfo function ON or OFF. */
#define LINIF_VERSION_INFO_API STD_ON

typedef struct {
	/* Switches the Development Error Detection and Notification
	ON or OFF. */
	boolean LinIfDevErrorDetect;
	/* States if multiple drivers are included in the LIN Interface or not. The
	 * reason for this parameter is to reduce the size of LIN Interface if multiple
	 * drivers are not used. */
	boolean LinIfMultipleDriversSupported;
	/* States if the node configuration commands Assign NAD and Conditional
	 * Change NAD are supported. */
	boolean LinIfNcOptionalRequestSupported;
	/* States if the TP is included in the LIN Interface or not. The reason for this
	 * parameter is to reduce the size of LIN Interface if the TP is not used. */
	boolean LinIfTpSupported;
	/* Switches the LinIf_GetVersionInfo function ON or OFF. */
	boolean LinIfVersionInfoApi;
}LinIf_GeneralType;

typedef enum {
	CLASSIC,
	ENHANCED,
}LinIf_ChecksumType;

typedef enum {
	ASSIGN,
	ASSIGN_NAD,
	CONDITIONAL,
	EVENT_TRIGGERED,
	FREE,
	MRF,
	SPORADIC,
	SRF,
	UNASSIGN,
	UNCONDITIONAL,
}LinIf_FrameTypeType;

typedef enum
{
	LinIfInternalPdu,
	LinIfRxPdu,
	LinIfSlaveToSlavePdu,
	LinIfTxPdu,
}LinIf_PduDirectionType;

typedef struct {
	LinIf_ChecksumType LinIfChecksumType;
	char* LinIfFrameName;
	uint8 LinIfFramePriority;
	LinIf_FrameTypeType LinIfFrameType;
	uint8 LinIfLength;
	uint8 LinIfPid;
	uint8 LinIfTxTargetPduId;
	uint8 *LinIfFixedFrameSdu;
	LinIf_PduDirectionType LinIfPduDirection;
	//LinIf_SubstitutionFramesType *LinIfSubstitutionFrames;
} LinIf_FrameType;

typedef struct {
	uint32 LinIfJitter;
}LinIf_MasterType;

typedef struct {
	uint32 LinIfConfiguredNad;
	uint32 LinIfFunctionId;
	char* LinIfProtocolVersion;
	uint32 LinIfResponseErrorBitPos;
	uint32 LinIfSupplierId;
	uint32 LinIfVariant;
	uint32 LinIfResponseErrorEventRef;
	uint16 LinIfResponseErrorFrameRef;
}LinIf_SlaveType;

typedef enum {
	CONTINUE_AT_IT_POINT,
	START_FROM_BEGINNING,
}LinIf_ResumePositionType;

typedef enum {
	RUN_CONTINUOUS,
	RUN_ONCE,
}LinIfRunModeType;

typedef struct {
	uint16 LinIfDelay;
	uint16 LinIfEntryIndex;
	uint16 LinIfCollisionResolvingRef;
	uint16 LinIfFrameRef;
}LinIfEntryType;

typedef struct {
	LinIf_ResumePositionType LinIfResumePosition;
	LinIfRunModeType LinIfRunMode;
	uint8 LinIfSchedulePriority;
	uint16 LinIfScheduleTableIndex;
	char* LinIfScheduleTableName;
	const LinIfEntryType *LinIfEntry;
	uint16 LinIfNofEntries;
}LinIf_ScheduleTableType;

typedef uint8 LinIf_WakeUpSourceType;

typedef char* LinIf_NodeComposition;

typedef struct {
	/* Internal ID for the channel on LIN Interface level. This parameter shall map
	 * the NetworkHandleType to the physical LIN channel.
	 * Implementation Type: NetworkHandleType */
	uint8 LinIfChannelId;
	/* Number of schedule requests the schedule table manager can handle for
	 * this channel. */
	uint8 LinIfScheduleRequestQueueLength;
	/* Reference to the used channel in Lin. Replaces LINIF_CHANNEL_INDEX */
	const Lin_ChannelConfigType *LinIfChannelRef;

	/* Generic container for all types of LIN frames. */
	const LinIf_FrameType *LinIfFrame;
	/* Each Master can only be connected to one physical channel.
	 * This could be compared to the Node parameter in a LDF file. */
	LinIf_MasterType LinIfMaster;
	/* Describes a schedule table. Each LinIfChannel may have several schedule tables.
	 * Each schedule table can only be connected to one channel. */
	const LinIf_ScheduleTableType *LinIfScheduleTable;
	/* The Node attributes of the Slaves are provided with these parameter. */
	const LinIf_SlaveType *LinIfSlave;
	/* This container contains the configuration (parameters) needed
	to configure a wakeup capable channel */
	const LinIf_WakeUpSourceType *LinIfWakeUpSource;
} LinIf_ChannelType;

typedef struct {
	uint16 LinIfTimeBase;
	const LinIf_ChannelType *LinIfChannel;
}LinIf_GlobalConfigType;

typedef struct {
	const LinIf_GeneralType *LinIfGeneral;
	const LinIf_GlobalConfigType *LinIfGlobalConfig;
} LinIf_Type;

extern const LinIfEntryType LinIfEntryCfg1[];
extern const LinIf_ScheduleTableType LinIfScheduleTableCfg[];
extern const LinIf_ChannelType LinIfChannelCfg[];
extern const LinIf_GlobalConfigType LinIfGlobalConfig;

// TODO not in Lin if spec
extern const LinIf_FrameType LinIfFrameCfg[];

#define LINIF_CONTROLLER_CNT 1
#define LINIF_SCH_CNT 2

#endif /*LINIF_CFG_H_*/
