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








#ifndef LINSM_CFG_H_
#define LINSM_CFG_H_

#include "Std_Types.h"
#include "LinIf_Cfg.h"

// Own timeout configs in ticks
#define LINSM_SCHEDULE_REQUEST_TIMEOUT 1
#define LINSM_GOTO_SLEEP_TIMEOUT 2
#define LINSM_WAKEUP_TIMEOUT 1

/* Switches the Development Error Detection and Notification
ON or OFF. */
#define LINSM_DEV_ERROR_DETECT STD_ON

/* Switches the LINSM_GetVersionInfo function ON or OFF. */
#define LINSM_VERSION_INFO_API STD_ON

typedef struct {
	uint32 LinSMRxPduGroupRef;//ComIPduGroup
	uint32 LinSMTxPduGroupRef;
	const LinIf_ScheduleTableType *LinSMScheduleIndexRef;
}LinSM_ScheduleType;

typedef struct {
	float LinSMConfirmationTimeout;
	boolean LinSMSleepSupport;
	uint16 LinSMChannelIndex;
	const LinSM_ScheduleType *LinSMSchedule;
}LinSM_ChannelType;

extern const LinSM_ChannelType LinSMChannelType[];

#endif /*LINSM_CFG_H_*/
