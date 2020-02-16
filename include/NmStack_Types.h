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
