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






#ifndef WDGM_H_
#define WDGM_H_

#define WDGM_SW_MAJOR_VERSION	1
#define WDGM_SW_MINOR_VERSION	0
#define WDGM_SW_PATCH_VERSION	0

#include "Std_Types.h"

/* "forward" declare types due to circular dependency chain */
#ifndef CFG_WDGM_USE_SERVICE_COMPONENT
typedef uint8 WdgM_SupervisedEntityIdType;
#endif
typedef uint8 WdgM_ModeType;

#include "WdgM_Cfg.h"

// API Service ID's
#define WDGM_INIT_ID                          0x00
#define WDGM_SETMODE_ID                       0x03
#define WDGM_UPDATEALIVECOUNTER_ID			  0x04
#define WDGM_ACTIVATEALIVESUPERVISION_ID      0x05
#define WDGM_DEACTIVATEALIVESUPERVISION_ID    0x06
#define WDGM_MAINFUNCTION_TRIGGER_ID          0x06
#define WDGM_MAINFUNCTION_ALIVESUPERVISION_ID 0x08
#define WDGM_GETMODE_ID                       0x0b
#define WDGM_GETALIVESUPERVISIONSTATUS_ID     0x0c
#define WDGM_GETGLOBALSTATUS_ID               0x0d


/** @req WDGM004 **/
#define WDGM_E_NO_INIT 		                0x10
#define WDGM_E_PARAM_CONFIG 		        0x11
#define WDGM_E_PARAM_MODE 		            0x12
#define WDGM_E_PARAM_SEID 		            0x13
#define WDGM_E_NULL_POINTER		            0x14
#define WDGM_E_DISABLE_NOT_ALLOWED          0x15
#define WDGM_E_DEACTIVATE_NOT_ALLOWED       0x16
//#define WDGM_E_ALIVE_SUPERVISION          DEM assigned
//#define WDGM_E_SET_MODE                   DEM assigned

Std_ReturnType WdgM_UpdateAliveCounter (WdgM_SupervisedEntityIdType SEid);
Std_ReturnType WdgM_ActivateAliveSupervision (WdgM_SupervisedEntityIdType SEid);
Std_ReturnType WdgM_DeactivateAliveSupervision (WdgM_SupervisedEntityIdType SEid);
Std_ReturnType WdgM_GetGlobalStatus (WdgM_AliveSupervisionStatusType *Status);
Std_ReturnType WdgM_GetAliveSupervisionStatus (WdgM_SupervisedEntityIdType SEid, WdgM_AliveSupervisionStatusType *Status);
void WdgM_Init(const WdgM_ConfigType* ConfigPtr);
void WdgM_DeInit(void);
Std_ReturnType WdgM_SetMode(WdgM_ModeType Mode);
Std_ReturnType WdgM_GetMode(WdgM_ModeType *Mode);
void WdgM_MainFunction_AliveSupervision (void);
void WdgM_MainFunction_Trigger (void);

#endif /*WDGM_H_*/
