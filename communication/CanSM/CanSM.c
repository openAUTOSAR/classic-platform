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

//lint -emacro(904,CANSM_VALIDATE_INIT,CANSM_VALIDATE_NETWORK,CANSM_VALIDATE_POINTER,CANSM_VALIDATE_MODE) //904 PC-Lint exception to MISRA 14.7 (validate macros).

/* Globally fulfilled requirements */
/** @req CANSM069 */
/** @req CANSM077 */
/** @req CANSM076 */
/** @req CANSM078 */
/** @req CANSM079 */
/** @req CANSM237 */
/** @req CANSM156.bswbuilder */

#include "ComStack_Types.h"
#include "CanSM.h"              /**< @req CANSM013 */
#include "Com.h"                /**< @req CANSM172 */
#include "ComM.h"               /**< @req CANSM174 */
#include "ComM_BusSm.h"         /**< @req CANSM191 */
#if defined(USE_DET)
#include "Det.h"                /**< @req CANSM015 */
#endif
#if defined(USE_DEM)
#include "Dem.h"                /**< @req CANSM014 */
#endif
#include "CanIf.h"              /**< @req CANSM017 */
#include "CanSM_Internal.h"

static CanSM_Internal_NetworkType CanSM_InternalNetworks[CANSM_NETWORK_COUNT];

static CanSM_InternalType CanSM_Internal = {
		.InitStatus = CANSM_STATUS_UNINIT,
		.Networks = CanSM_InternalNetworks,
};

#ifdef HOST_TEST
void GetInternals(CanSM_InternalType **ptr){
	*ptr = &CanSM_Internal;
}
#endif

static const CanSM_ConfigType* CanSM_Config;

/** @req CANSM217.exceptTranceiver */
void CanSM_Init( const CanSM_ConfigType* ConfigPtr ) {
	CANSM_VALIDATE_POINTER_NORV(ConfigPtr, CANSM_SERVICEID_INIT);  /**< @req CANSM179 */

	CanSM_Config = ConfigPtr;
	Std_ReturnType status = E_OK;
	Std_ReturnType totalStatus = E_OK;

	for (uint8 i = 0; i < CANSM_NETWORK_COUNT; ++i) {
		status = CanSM_Internal_RequestComMode(i, COMM_NO_COMMUNICATION, true);  /**< @req CANSM211 */
		if (status > totalStatus) {
			totalStatus = status;
		}
		CanSM_Internal.Networks[i].BusOffRecoveryState = CANSM_BOR_IDLE;
		CanSM_Internal.Networks[i].requestedMode = COMM_NO_COMMUNICATION;
	}

	if (totalStatus == E_OK) {
		CanSM_Internal.InitStatus = CANSM_STATUS_INIT;
	} else {
		// TODO report error?
	}
}

/** @req CANSM189 */
/** @req CANSM190 */
/** @req CANSM235 */
void CanSM_ControllerBusOff(uint8 Controller)
{
	CANSM_VALIDATE_INIT_NORV(CANSM_SERVICEID_CONTROLLERBUSOFF);
	Std_ReturnType status = E_NOT_OK;

	// Find which network has this controller
	for (uint8 i = 0; i < CANSM_NETWORK_COUNT; ++i) {
		const CanSM_NetworkType* Network = &CanSM_Config->Networks[i];
		for (uint8 j = 0; j < Network->ControllerCount; ++j) {
			const CanSM_ControllerType* ptrController = &Network->Controllers[j];
			if(ptrController->CanIfControllerId == Controller)
			{
				CanSM_Internal.Networks[i].busoffevent = TRUE;
				//CanSM_Internal.Networks[i].counter = 0;
				status = E_OK;
			}
		}
	}

	// Check if controller was valid
	if(status != E_OK){

	}
}

/** @req CANSM181  @req CANSM183  @req CANSM182.partially  @req CANSM184 */
Std_ReturnType CanSM_RequestComMode( NetworkHandleType NetworkHandle, ComM_ModeType ComM_Mode ) {
	CANSM_VALIDATE_INIT(CANSM_SERVICEID_REQUESTCOMMODE, E_NOT_OK);
	CANSM_VALIDATE_NETWORK(NetworkHandle, CANSM_SERVICEID_REQUESTCOMMODE, E_NOT_OK);
	CANSM_VALIDATE_MODE(ComM_Mode, CANSM_SERVICEID_REQUESTCOMMODE, E_NOT_OK);

	return CanSM_Internal_RequestComMode(NetworkHandle, ComM_Mode, false );
}

/** @req CANSM032.partially  @req CANSM212  @req CANSM219.exceptTranceiver  @req CANSM218.exceptTranceiver
 *  @req CANSM231  @req CANSM232 */
Std_ReturnType CanSM_Internal_RequestComMode( NetworkHandleType NetworkHandle, ComM_ModeType ComM_Mode, boolean init ) {
	Std_ReturnType overallStatus = E_OK;
	Std_ReturnType status;
	status = CanSM_Internal_RequestCanIfMode(NetworkHandle, ComM_Mode);      /**< @req CANSM240 */
	if (status > overallStatus){
		overallStatus = status;
	}
	/* Follow figure 9-1 for init */
	if( !init ) {
		status = CanSM_Internal_RequestComGroupMode(NetworkHandle, ComM_Mode);   /**< @req CANSM241 */
		if (status > overallStatus) {
			overallStatus = status;
		}
	}
	status = CanSM_Internal_RequestCanIfPduMode(NetworkHandle, ComM_Mode);
	if (status > overallStatus) {
		overallStatus = status;
	}

	/* Follow figure 9-1 for init */
	if ( (overallStatus == E_OK) && !init ) {
		ComM_BusSM_ModeIndication(NetworkHandle, ComM_Mode);                 /**< @req CANSM089 */
		CanSM_Internal.Networks[NetworkHandle].requestedMode = ComM_Mode;
	}

	return overallStatus;
}

/** @req CANSM039  @req CANSM044 */
Std_ReturnType CanSM_Internal_RequestCanIfMode( NetworkHandleType NetworkHandle, ComM_ModeType ComM_Mode ) {
	const CanSM_NetworkType* Network = &CanSM_Config->Networks[NetworkHandle];
	CanIf_ControllerModeType CanIf_Mode = CANIF_CS_STARTED;
	Std_ReturnType totalStatus = E_OK;

	switch (ComM_Mode) {
		case COMM_NO_COMMUNICATION:
			CanIf_Mode = CANIF_CS_SLEEP;
			break;
		case COMM_FULL_COMMUNICATION:
			CanIf_Mode = CANIF_CS_STARTED;
			break;
		default:
			totalStatus = E_NOT_OK;
			break;
	}

	if (totalStatus == E_OK) {
		for (uint8 i = 0; i < Network->ControllerCount; ++i) {
			const CanSM_ControllerType* Controller = &Network->Controllers[i];
			Std_ReturnType status =
					CanIf_SetControllerMode(Controller->CanIfControllerId, CanIf_Mode);
			if (status > totalStatus) {
				totalStatus = status;
			}
		}
	}
	return totalStatus;
}

Std_ReturnType CanSM_Internal_SetCanIfPduMode( NetworkHandleType NetworkHandle, CanIf_ChannelSetModeType ChannelMode ) {
	const CanSM_NetworkType* Network = &CanSM_Config->Networks[NetworkHandle];
	Std_ReturnType totalStatus = E_OK;

	for (uint8 i = 0; i < Network->ControllerCount; ++i) {
		const CanSM_ControllerType* Controller = &Network->Controllers[i];
		Std_ReturnType status =
				CanIf_SetPduMode(Controller->CanIfControllerId, ChannelMode);
		if (status > totalStatus) {
			totalStatus = status;
		}
	}

	return totalStatus;
}
/** @req CANSM083 */
Std_ReturnType CanSM_Internal_RequestCanIfPduMode( NetworkHandleType NetworkHandle, ComM_ModeType ComM_Mode ) {
	CanIf_ChannelSetModeType channelMode = CANIF_SET_OFFLINE;
	Std_ReturnType status = E_OK;
	boolean updatePduMode = FALSE;

	switch (ComM_Mode) {
		case COMM_NO_COMMUNICATION:
			/* No action */
			break;
		case COMM_SILENT_COMMUNICATION:
			/* Should actually only be done on transition from FULL_COMMUNICATION */
			channelMode = CANIF_SET_TX_OFFLINE;
			updatePduMode = TRUE;
			break;
		case COMM_FULL_COMMUNICATION:
			/* Should actually only be done on transition from SILENT_COMMUNICATION */
			channelMode = CANIF_SET_TX_ONLINE;
			updatePduMode = TRUE;
			break;
		default:
			status = E_NOT_OK;
			break;
	}
	if (updatePduMode) {
		status = CanSM_Internal_SetCanIfPduMode(NetworkHandle, channelMode);
	}
	return status;
}

/** @req CANSM173 */
Std_ReturnType CanSM_Internal_RequestComGroupMode( NetworkHandleType NetworkHandle, ComM_ModeType ComM_Mode ) {
	const CanSM_NetworkType* Network = &CanSM_Config->Networks[NetworkHandle];
	Std_ReturnType status = E_OK;

	switch (ComM_Mode) {
		case COMM_NO_COMMUNICATION:
			Com_IpduGroupStop(Network->ComRxPduGroupId);
			Com_IpduGroupStop(Network->ComTxPduGroupId);
			break;
		case COMM_SILENT_COMMUNICATION:
			Com_IpduGroupStart(Network->ComRxPduGroupId, FALSE);
			Com_IpduGroupStop(Network->ComTxPduGroupId);
			break;
		case COMM_FULL_COMMUNICATION:
			Com_IpduGroupStart(Network->ComRxPduGroupId, FALSE);
			Com_IpduGroupStart(Network->ComTxPduGroupId, FALSE);
			break;
		default:
			status = E_NOT_OK;
			break;
	}
	return status;
}

/** @req CANSM090  @req CANSM185  @req CANSM187  @req CANSM186  @req CANSM188 */
Std_ReturnType CanSM_GetCurrentComMode( NetworkHandleType NetworkHandle, ComM_ModeType* ComM_ModePtr ) {
	CANSM_VALIDATE_INIT(CANSM_SERVICEID_GETCURRENTCOMMODE, E_NOT_OK);
	CANSM_VALIDATE_NETWORK(NetworkHandle, CANSM_SERVICEID_GETCURRENTCOMMODE, E_NOT_OK);

	ComM_ModeType ComM_Mode = COMM_FULL_COMMUNICATION;

	const CanSM_NetworkType* Network = &CanSM_Config->Networks[NetworkHandle];
	Std_ReturnType totalStatus = E_OK;
	for (uint8 i = 0; i < Network->ControllerCount; ++i) {
		const CanSM_ControllerType* Controller = &Network->Controllers[i];
		CanIf_ControllerModeType CanIf_Mode;
		Std_ReturnType status =
				CanIf_GetControllerMode(Controller->CanIfControllerId, &CanIf_Mode);
		if (status > totalStatus) {
			totalStatus = status;
		}

		if (CanIf_Mode != CANIF_CS_STARTED) {
			ComM_Mode = COMM_NO_COMMUNICATION;
		}
	}

	*ComM_ModePtr = ComM_Mode;
	return totalStatus;
}

static void CanSM_Internal_CANSM_BOR_IDLE(NetworkHandleType NetworkHandle)
{
	CanSM_Internal_NetworkType *Network = &CanSM_Internal.Networks[NetworkHandle];

	Network->counter = 0;
	Network->timer = 0;
	Network->BusOffRecoveryState = CANSM_BOR_CHECK;
}

static void CanSM_Internal_CANSM_BOR_CHECK(NetworkHandleType NetworkHandle)
{
	CanSM_Internal_NetworkType *Network = &CanSM_Internal.Networks[NetworkHandle];

	Network->timer++;
	if(Network->busoffevent){
		Network->busoffevent = FALSE;
		Network->timer = 0;
		Network->BusOffRecoveryState = CANSM_BOR_TXOFF_L1;

		// Restart CAN
		CanSM_Internal_RequestCanIfMode(NetworkHandle, COMM_FULL_COMMUNICATION);
		// Tx offline
		CanSM_Internal_SetCanIfPduMode(NetworkHandle, CANIF_SET_TX_OFFLINE);
	}
	else if(Network->timer >= CanSM_Config->Networks[NetworkHandle].CanSMBorTimeTxEnsured){
#if defined(USE_DEM)
		Dem_ReportErrorStatus(CanSM_Config->Networks[NetworkHandle].CanSMBusOffDemEvent, DEM_EVENT_STATUS_PASSED);
#endif
		Network->BusOffRecoveryState = CANSM_BOR_NO_BUS_OFF;
	}
}

static void CanSM_Internal_CANSM_BOR_NO_BUS_OFF(NetworkHandleType NetworkHandle)
{
	CanSM_Internal_NetworkType *Network = &CanSM_Internal.Networks[NetworkHandle];

	if(Network->busoffevent){
		Network->busoffevent = FALSE;
		Network->timer = 0;
		Network->BusOffRecoveryState = CANSM_BOR_TXOFF_L1;

		// Restart CAN
		CanSM_Internal_RequestCanIfMode(NetworkHandle, COMM_FULL_COMMUNICATION);
		// Tx offline
		CanSM_Internal_SetCanIfPduMode(NetworkHandle, CANIF_SET_TX_OFFLINE);
	}
}

static void CanSM_Internal_CANSM_BOR_TXOFF_L1(NetworkHandleType NetworkHandle)
{
	CanSM_Internal_NetworkType *Network = &CanSM_Internal.Networks[NetworkHandle];

	Network->timer++;
    if(Network->timer >= CanSM_Config->Networks[NetworkHandle].CanSMBorTimeL1){
		Network->timer = 0;
		Network->BusOffRecoveryState = CANSM_BOR_CHECK_L1;
		// inc busoff counter
		Network->counter++;
    	// Try starting Tx again
		CanSM_Internal_SetCanIfPduMode(NetworkHandle, CANIF_SET_TX_ONLINE);
	}
}

static void CanSM_Internal_CANSM_BOR_CHECK_L1(NetworkHandleType NetworkHandle)
{
	CanSM_Internal_NetworkType *Network = &CanSM_Internal.Networks[NetworkHandle];

	Network->timer++;
	if(Network->busoffevent){
		Network->busoffevent = FALSE;
		Network->timer = 0;
		if(Network->counter >= CanSM_Config->Networks[NetworkHandle].CanSMBorCounterL1ToL2){
			Network->BusOffRecoveryState = CANSM_BOR_TXOFF_L2;
		}else{
			Network->BusOffRecoveryState = CANSM_BOR_TXOFF_L1;
		}

		// Restart CAN
		CanSM_Internal_RequestCanIfMode(NetworkHandle, COMM_FULL_COMMUNICATION);
		// Tx offline
		CanSM_Internal_SetCanIfPduMode(NetworkHandle, CANIF_SET_TX_OFFLINE);
	}
	else if(Network->timer >= CanSM_Config->Networks[NetworkHandle].CanSMBorTimeTxEnsured){
		// clear busoff counter
		Network->counter = 0;

#if defined(USE_DEM)
		Dem_ReportErrorStatus(CanSM_Config->Networks[NetworkHandle].CanSMBusOffDemEvent,  DEM_EVENT_STATUS_PASSED);
#endif
		Network->BusOffRecoveryState = CANSM_BOR_NO_BUS_OFF;
	}
}

static void CanSM_Internal_CANSM_BOR_TXOFF_L2(NetworkHandleType NetworkHandle)
{
	CanSM_Internal_NetworkType *Network = &CanSM_Internal.Networks[NetworkHandle];

	Network->timer++;
    if(Network->timer >= CanSM_Config->Networks[NetworkHandle].CanSMBorTimeL2){
		Network->timer = 0;
		Network->BusOffRecoveryState = CANSM_BOR_CHECK_L2;
		// inc busoff counter
		Network->counter++;
    	// Try starting Tx again
		CanSM_Internal_SetCanIfPduMode(NetworkHandle, CANIF_SET_TX_ONLINE);
	}
}

static void CanSM_Internal_CANSM_BOR_CHECK_L2(NetworkHandleType NetworkHandle)
{
	CanSM_Internal_NetworkType *Network = &CanSM_Internal.Networks[NetworkHandle];

	Network->timer++;
	if(Network->busoffevent){
		Network->busoffevent = FALSE;
		Network->timer = 0;
		if(Network->counter >= CanSM_Config->Networks[NetworkHandle].CanSMBorCounterL2Err){
			// TBD DEM error
#if defined(USE_DEM)
			Dem_ReportErrorStatus(CanSM_Config->Networks[NetworkHandle].CanSMBusOffDemEvent,  DEM_EVENT_STATUS_FAILED);
#endif
			Network->BusOffRecoveryState = CANSM_BOR_TXOFF_L2;
		}else{
			/* TODO: Should we really go to CANSM_BOR_TXOFF_L1 here? */
			Network->BusOffRecoveryState = CANSM_BOR_TXOFF_L1;
		}

		// Restart CAN
		CanSM_Internal_RequestCanIfMode(NetworkHandle, COMM_FULL_COMMUNICATION);
		// Tx offline
		CanSM_Internal_SetCanIfPduMode(NetworkHandle, CANIF_SET_TX_OFFLINE);
	}
	else if(Network->timer >= CanSM_Config->Networks[NetworkHandle].CanSMBorTimeTxEnsured){
		// clear busoff counter
		Network->counter = 0;
		// TBD DEM & deadline monitoring
#if defined(USE_DEM)
		Dem_ReportErrorStatus(CanSM_Config->Networks[NetworkHandle].CanSMBusOffDemEvent,  DEM_EVENT_STATUS_PASSED);
#endif
		Network->BusOffRecoveryState = CANSM_BOR_NO_BUS_OFF;
	}
}

void CanSM_MainFunction() {
	CANSM_VALIDATE_INIT_NORV(CANSM_SERVICEID_MAINFUNCTION);

	for (uint8 i = 0; i < CANSM_NETWORK_COUNT; ++i) {
		CanSM_Internal_NetworkType *Network = &CanSM_Internal.Networks[i];
		// Always goto IDLE if no com ordered
		if(Network->requestedMode == COMM_NO_COMMUNICATION){
			Network->BusOffRecoveryState = CANSM_BOR_IDLE;
		}
		else
		{
			// Handle bus off recovery state machine
			switch(Network->BusOffRecoveryState)
			{
			case CANSM_BOR_IDLE:
				CanSM_Internal_CANSM_BOR_IDLE(i);
				break;
			case CANSM_BOR_CHECK:
				CanSM_Internal_CANSM_BOR_CHECK(i);
				break;
			case CANSM_BOR_NO_BUS_OFF:
				CanSM_Internal_CANSM_BOR_NO_BUS_OFF(i);
				break;
			case CANSM_BOR_TXOFF_L1:
				CanSM_Internal_CANSM_BOR_TXOFF_L1(i);
				break;
			case CANSM_BOR_CHECK_L1:
				CanSM_Internal_CANSM_BOR_CHECK_L1(i);
				break;
			case CANSM_BOR_TXOFF_L2:
				CanSM_Internal_CANSM_BOR_TXOFF_L2(i);
				break;
			case CANSM_BOR_CHECK_L2:
				CanSM_Internal_CANSM_BOR_CHECK_L2(i);
				break;
			default:
				break;
			}
		}
	}
}

