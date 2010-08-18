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

#ifndef DCM_CFG_H_
#define DCM_CFG_H_
/*
 * DCM General
 */
#define DCM_VERSION_INFO_API    		STD_ON		// Activate/Deactivate ver info API.
#define DCM_DEV_ERROR_DETECT			STD_ON		// Activate/Deactivate Dev Error Detection and Notification.
#define DCM_REQUEST_INDICATION_ENABLED	STD_ON		// Activate/Deactivate indication request mechanism.
#define DCM_RESPOND_ALL_REQUEST			STD_ON		// Activate/Deactivate response on SID 0x40-0x7f and 0xc0-0xff.
#define DCM_TASK_TIME					TBD			// Time for periodic task (in ms).
#define DCM_PAGEDBUFFER_ENABLED			STD_OFF		// Enable/disable page buffer mechanism (currently only disabled supported)

#endif /*DCM_CFG_H_*/
