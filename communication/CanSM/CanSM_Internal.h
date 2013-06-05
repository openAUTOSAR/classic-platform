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


#ifndef CANSM_INTERNAL_H
#define CANSM_INTERNAL_H

#include "CanSM.h"

/** @req CANSM027  @req CANSM028  @req CANSM071 */
#if (CANSM_DEV_ERROR_DETECT == STD_ON)
#define CANSM_DET_REPORTERROR(serviceId, errorId)			\
	Det_ReportError(MODULE_ID_CANSM, 0, serviceId, errorId)

#define CANSM_VALIDATE(expression, serviceId, errorId, ...)	\
	if (!(expression)) {									\
		CANSM_DET_REPORTERROR(serviceId, errorId);			\
		return __VA_ARGS__;									\
	}

#define CANSM_VALIDATE_NORV(expression, serviceId, errorId)	\
	if (!(expression)) {									\
		CANSM_DET_REPORTERROR(serviceId, errorId);			\
		return;									\
	}

#else
#define CANSM_DET_REPORTERROR(...)
#define CANSM_VALIDATE(...)
#define CANSM_VALIDATE_NORV(...)
#endif

#define CANSM_VALIDATE_INIT(serviceID, ...)					\
		CANSM_VALIDATE((CanSM_Internal.InitStatus == CANSM_STATUS_INIT), serviceID, CANSM_E_UNINIT, __VA_ARGS__)

#define CANSM_VALIDATE_INIT_NORV(serviceID)					\
		CANSM_VALIDATE_NORV((CanSM_Internal.InitStatus == CANSM_STATUS_INIT), serviceID, CANSM_E_UNINIT)

#define CANSM_VALIDATE_POINTER_NORV(pointer, serviceID)					\
		CANSM_VALIDATE_NORV( (pointer != NULL), serviceID, CANSM_E_PARAM_POINTER)

#define CANSM_VALIDATE_NETWORK(net, serviceID, ...)					\
		CANSM_VALIDATE( (net < CANSM_NETWORK_COUNT), serviceID, CANSM_E_INVALID_NETWORK_HANDLE, __VA_ARGS__)

#define CANSM_VALIDATE_MODE(mode, serviceID, ...)					\
		CANSM_VALIDATE( (mode <= COMM_FULL_COMMUNICATION) && (mode != COMM_SILENT_COMMUNICATION), serviceID, CANSM_E_INVALID_NETWORK_MODE, __VA_ARGS__)

typedef enum {
	CANSM_STATUS_UNINIT,
	CANSM_STATUS_INIT
} CanSM_Internal_InitStatusType;

typedef struct {
	uint16 timer;
	uint8 counter;
	boolean busoffevent;
	ComM_ModeType requestedMode;
	CanSM_BusOffRecoveryStateType       BusOffRecoveryState;
} CanSM_Internal_NetworkType;

typedef struct {
	CanSM_Internal_InitStatusType 		InitStatus;
	CanSM_Internal_NetworkType*		Networks;
} CanSM_InternalType;

Std_ReturnType CanSM_Internal_RequestComMode( NetworkHandleType NetworkHandle, ComM_ModeType ComM_Mode, boolean init);
Std_ReturnType CanSM_Internal_RequestCanIfMode( NetworkHandleType NetworkHandle, ComM_ModeType ComM_Mode );
Std_ReturnType CanSM_Internal_RequestComGroupMode( NetworkHandleType NetworkHandle, ComM_ModeType ComM_Mode );
Std_ReturnType CanSM_Internal_RequestCanIfPduMode( NetworkHandleType NetworkHandle, ComM_ModeType ComM_Mode );

#endif /* CANSM_INTERNAL_H */
