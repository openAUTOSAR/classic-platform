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


#ifndef ETHSM_INTERNAL_H
#define ETHSM_INTERNAL_H

#include "EthSM.h"

/** @req   @req   @req  */
#if (ETHSM_DEV_ERROR_DETECT == STD_ON)
#define ETHSM_DET_REPORTERROR(serviceId, errorId)			\
	Det_ReportError(MODULE_ID_ETHSM, 0, serviceId, errorId)

#define ETHSM_VALIDATE(expression, serviceId, errorId, ...)	\
	if (!(expression)) {									\
		ETHSM_DET_REPORTERROR(serviceId, errorId);			\
		return __VA_ARGS__;									\
	}

#else
#define ETHSM_DET_REPORTERROR(...)
#define ETHSM_VALIDATE(...)
#endif

#define ETHSM_VALIDATE_INIT(serviceID, ...)					\
		ETHSM_VALIDATE((EthSM_Internal.InitStatus == ETHSM_STATUS_INIT), serviceID, ETHSM_E_UNINIT, __VA_ARGS__)

#define ETHSM_VALIDATE_POINTER(pointer, serviceID, ...)					\
		ETHSM_VALIDATE( (pointer != NULL), serviceID, ETHSM_E_PARAM_POINTER, __VA_ARGS__)

#define ETHSM_VALIDATE_NETWORK(net, serviceID, ...)					\
		ETHSM_VALIDATE( (net < ETHSM_NETWORK_COUNT), serviceID, ETHSM_E_INVALID_NETWORK_HANDLE, __VA_ARGS__)

#define ETHSM_VALIDATE_MODE(mode, serviceID, ...)					\
		ETHSM_VALIDATE( (mode <= COMM_FULL_COMMUNICATION) && (mode != COMM_SILENT_COMMUNICATION), serviceID, ETHSM_E_INVALID_NETWORK_MODE, __VA_ARGS__)

typedef enum {
	ETHSM_STATUS_UNINIT,
	ETHSM_STATUS_INIT
} EthSM_Internal_InitStatusType;

typedef struct {
	ComM_ModeType requestedMode;
} EthSM_Internal_NetworkType;

typedef struct {
	EthSM_Internal_InitStatusType 		InitStatus;
	EthSM_Internal_NetworkType*		Networks;
} EthSM_InternalType;

Std_ReturnType EthSM_Internal_RequestComMode( NetworkHandleType NetworkHandle, ComM_ModeType ComM_Mode );
Std_ReturnType EthSM_Internal_RequestEthIfMode( NetworkHandleType NetworkHandle, ComM_ModeType ComM_Mode );
Std_ReturnType EthSM_Internal_RequestComGroupMode( NetworkHandleType NetworkHandle, ComM_ModeType ComM_Mode );

#endif /* ETHSM_INTERNAL_H */
