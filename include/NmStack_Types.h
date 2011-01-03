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


#ifndef NMSTACK_TYPES_H_
#define NMSTACK_TYPES_H_

/** Return type for NM functions. Derived from Std_ReturnType. */
typedef enum {
	NM_E_OK,
	NM_E_NOT_OK,
	NM_E_NOT_EXECUTED
} Nm_ReturnType;

/** Operational modes of the network management */
typedef enum {
	NM_MODE_BUS_SLEEP,
	NM_MODE_PREPARE_BUS_SLEEP,
	NM_MODE_SYNCHRONIZE,
	NM_MODE_NETWORK
} Nm_ModeType;

/** States of the network management state machine */
typedef enum {
	NM_STATE_UNINIT,
	NM_STATE_BUS_SLEEP,
	NM_STATE_PREPARE_BUS_SLEEP,
	NM_STATE_READY_SLEEP,
	NM_STATE_NORMAL_OPERATION,
	NM_STATE_REPEAT_MESSAGE,
	NM_STATE_SYNCHRONIZE
} Nm_StateType;

/** BusNm Type */
typedef enum {
	NM_BUSNM_CANNM	= 0,
	NM_BUSNM_FRNM	= 1,
	NM_BUSNM_LINNM	= 2,
	NM_BUSNM_UNDEF	= 0xFF
} Nm_BusNmType;


#endif /* NMSTACK_TYPES_H_ */
