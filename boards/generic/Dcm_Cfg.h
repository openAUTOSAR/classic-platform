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
