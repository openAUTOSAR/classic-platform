/*-------------------------------- Arctic Core ------------------------------
 * Copyright (C) 2013, ArcCore AB, Sweden, www.arccore.com.
 * Contact: <contact@arccore.com>
 * 
 * You may ONLY use this file:
 * 1)if you have a valid commercial ArcCore license and then in accordance with  
 * the terms contained in the written license agreement between you and ArcCore, 
 * or alternatively
 * 2)if you follow the terms found in GNU General Public License version 2 as 
 * published by the Free Software Foundation and appearing in the file 
 * LICENSE.GPL included in the packaging of this file or here 
 * <http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt>
 *-------------------------------- Arctic Core -----------------------------*/








#ifndef LINIF_TYPES_H_
#define LINIF_TYPES_H_

#include "Std_Types.h"
#include "Lin_ConfigTypes.h"
/*===========[Type definitions]===============================================*/

/* Checksum type */
typedef enum
{
    CLASSIC,
    ENHANCED
} LinIf_ChecksumType;

/* Type of a Lin frame */
typedef enum
{
    ASSIGN,
    ASSIGN_NAD,
    CONDITIONAL,
    EVENT_TRIGGERED,
    FREE,
    MRF,
    SPORADIC,
    SRF,
    UNASSIGN,
    UNCONDITIONAL
} LinIf_FrameTypeType;

/* Direction of a Lin PDU */
typedef enum
{
    LinIfInternalPdu,
    LinIfRxPdu,
    LinIfSlaveToSlavePdu,
    LinIfTxPdu
} LinIf_PduDirectionType;

typedef enum {
    LINIF_UNINIT,
    LINIF_INIT,
    LINIF_CHANNEL_UNINIT,
    LINIF_CHANNEL_OPERATIONAL,
    LINIF_CHANNEL_SLEEP_TRANS,
    LINIF_CHANNEL_SLEEP,
}LinIf_StatusType;

/* Resuming modes of schedule table after having been
 * interrupted by another RUN_ONCE schedule table */
typedef enum
{
    CONTINUE_AT_IT_POINT,
    START_FROM_BEGINNING
} LinIf_ResumePositionType;

/* Run modes of a schedule table */
typedef enum
{
    RUN_CONTINUOUS,
    RUN_ONCE
} LinIfRunModeType;

/* Lin frame */
typedef struct
{
    LinIf_ChecksumType LinIfChecksumType;
    char* LinIfFrameName;
    uint8 LinIfFramePriority;
    LinIf_FrameTypeType LinIfFrameType;
    uint8 LinIfLength;
    uint8 LinIfPid;
    uint8 LinIfTxTargetPduId;
    uint8 *LinIfFixedFrameSdu;
    LinIf_PduDirectionType LinIfPduDirection;
} LinIf_FrameType;

/* Master node type */
typedef struct
{
    uint32 LinIfJitter;
} LinIf_MasterType;

/* Slave node type */
typedef struct
{
    uint32 LinIfConfiguredNad;
    uint32 LinIfFunctionId;
    char* LinIfProtocolVersion;
    uint32 LinIfResponseErrorBitPos;
    uint32 LinIfSupplierId;
    uint32 LinIfVariant;
    uint32 LinIfResponseErrorEventRef;
    uint16 LinIfResponseErrorFrameRef;
} LinIf_SlaveType;


/* Entry type */
typedef struct
{
    uint16 LinIfDelay;
    uint16 LinIfEntryIndex;
    uint16 LinIfCollisionResolvingRef;
    uint16 LinIfFrameRef; /* Index in the LinIfFrameCfg array */
} LinIfEntryType;

/* Lin schedule table */
typedef struct
{
    LinIf_ResumePositionType LinIfResumePosition;
    LinIfRunModeType LinIfRunMode;
    uint8 LinIfSchedulePriority;
    uint16 LinIfScheduleTableIndex;
    char* LinIfScheduleTableName;
    const LinIfEntryType *LinIfEntry;
    uint16 LinIfNofEntries;
} LinIf_ScheduleTableType;

typedef uint8 LinIf_WakeUpSourceType;

typedef char* LinIf_NodeComposition;



/* General settings as structure. */
typedef struct
{
    /* Switches the Development Error Detection and Notification ON or OFF. */
    boolean LinIfDevErrorDetect;

    /* States if multiple drivers are included in the LIN Interface or not.
     * The reason for this parameter is to reduce the size of LIN Interface if
     * multiple drivers are not used. */
    boolean LinIfMultipleDriversSupported;

    /* States if the node configuration commands Assign NAD and Conditional
     * Change NAD are supported. */

    boolean LinIfNcOptionalRequestSupported;
    /* States if the TP is included in the LIN Interface or not. The reason for
     * this parameter is to reduce the size of LIN Interface if the TP is not
     * used. */

    boolean LinIfTpSupported;
    /* Switches the LinIf_GetVersionInfo function ON or OFF. */
    boolean LinIfVersionInfoApi;
} LinIf_GeneralType;

typedef uint8 LinIf_SchHandleType;
/* Lin channel */
typedef struct
{
    /* Internal ID for the channel on LIN Interface level. This parameter shall
     * map the NetworkHandleType to the physical LIN channel.
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
    /* Startup state */
    LinIf_StatusType LinIfStartupState;
} LinIf_ChannelType;

/* Global configuration */
typedef struct
{
    uint16                   LinIfTimeBase;
    const LinIf_ChannelType *LinIfChannel;
    const LinIf_FrameType *LinIfFrameConfig;
} LinIf_ConfigType;

/* Lin configuration */
typedef struct
{
    const LinIf_GeneralType      *LinIfGeneral;
    const LinIf_ConfigType *LinIfGlobalConfig;
} LinIf_Type;

#endif




