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


#ifndef CANNM_INTERNAL_H_
#define CANNM_INTERNAL_H_

#if (CANNM_DEV_ERROR_DETECT == STD_ON)
#define CANNM_DET_REPORTERROR(serviceId, errorId)			\
	Det_ReportError(MODULE_ID_CANNM, 0, serviceId, errorId)

#define CANNM_VALIDATE(expression, serviceId, errorId, ...)	\
	if (!(expression)) {									\
		CANNM_DET_REPORTERROR(serviceId, errorId);			\
		return __VA_ARGS__;									\
	}

#else
#define CANNM_DET_REPORTERROR(...)
#define CANNM_VALIDATE(...)
#endif

#define CANNM_VALIDATE_INIT(serviceID, ...)					\
		CANNM_VALIDATE((CanNm_Internal.InitStatus == CANNM_INIT), serviceID, CANNM_E_NO_INIT, __VA_ARGS__)

#define CANNM_VALIDATE_CHANNEL(channel, serviceID, ...)					\
		CANNM_VALIDATE( (channel < CANNM_CHANNEL_COUNT), serviceID, CANNM_E_INVALID_CHANNEL, __VA_ARGS__)


typedef enum {
	CANNM_INIT,
	CANNM_UNINIT,
} CanNm_InitStatusType;

typedef struct {
	CanNm_InitStatusType 		InitStatus;
} CanNm_InternalType;

#endif /* CANNM_INTERNAL_H_ */
