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

#include "LinSM_Cfg.h"
#include "LinIf_Cfg.h"
#include "Lin_Cfg.h"

const LinSM_ScheduleType LinSMScheduleType =
{
	.LinSMRxPduGroupRef = 0, // TODO ref till COM
	.LinSMTxPduGroupRef = 0, // TODO ref till COM
	.LinSMScheduleIndexRef = &LinIfScheduleTableCfg[0],
};

const LinSM_ChannelType LinSMChannelType[] =
{
	{
		.LinSMConfirmationTimeout = 20.0,
		.LinSMSleepSupport = TRUE,
		.LinSMChannelIndex = LIN_CTRL_B,
		.LinSMSchedule = &LinSMScheduleType,
	},
};

