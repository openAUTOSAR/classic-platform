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

//lint -emacro(904,CANSM_VALIDATE_INIT,CANSM_VALIDATE_NETWORK,CANSM_VALIDATE_POINTER,CANSM_VALIDATE_MODE) //904 PC-Lint exception to MISRA 14.7 (validate macros).

/* Globally fulfilled requirements */
/** @req CANSM069 */

//#include "CanSM_Cfg.h"          /* @req CANSM254  @req CANSM025.configuration */
#include "ComStack_Types.h"
#include "CanSM.h"              /* @req CANSM013 */
#include "ComM.h"               /* @req CANSM174 */
#include "ComM_BusSm.h"         /* @req CANSM191 */
#include "Det.h"                /* @req CANSM015 */
#if defined(USE_DEM)
#include "Dem.h"                /* @req CANSM014 */
#endif
#include "CanIf.h"              /* @req CANSM017 */
#include "CanSM_Internal.h"
#include "CanSM_BswM.h"			/* @req CANSM348 */
#if defined(USE_BSWM)
#include "BswM_CanSM.h"
#endif

#if defined(USE_CANSM_EXTENSION)
#include "CanSM_Extension.h"
#endif

#define INVALID_CONTROLLER_ID 0xff

#if defined(USE_DEM)
	#define DEM_REPORT_ERROR_STATUS(_event,_eventstatus) Dem_ReportErrorStatus(_event,_eventstatus);
#endif

static CanSM_Internal_CtrlStatusType CanSM_InternalControllerStatus[NOF_CANSM_CANIF_CONTROLLERS];
static CanSM_Internal_NetworkType CanSM_InternalNetworks[CANSM_NETWORK_COUNT];

static CanSM_InternalType CanSM_Internal = {
		.InitStatus = CANSM_STATUS_UNINIT,
		.Networks = CanSM_InternalNetworks,
		.ControllerModeBuf = CanSM_InternalControllerStatus,
};

#ifdef HOST_TEST
void GetInternals(CanSM_InternalType **ptr){
	*ptr = &CanSM_Internal;
}
#endif

static const CanSM_ConfigType* CanSM_Config;


Std_ReturnType CanSM_Internal_GetCanSMControllerIndex(const uint8 controller, uint8* indexPtr)
{
	Std_ReturnType ret = E_NOT_OK;
	for(uint8 index = 0; index < NOF_CANSM_CANIF_CONTROLLERS; index++) {
		if( controller == CanSM_Internal.ControllerModeBuf[index].controllerId ) {
			*indexPtr = index;
			ret = E_OK;
			break;
		}
	}
	return ret;
}

/* @req CANSM023 */
void CanSM_Init( const CanSM_ConfigType* ConfigPtr ) {

	/* @req CANSM179 */
	if( NULL == ConfigPtr ) {
		CANSM_DET_REPORTERROR(CANSM_SERVICEID_INIT, CANSM_E_PARAM_POINTER);
		return;
	}

	CanSM_Config = ConfigPtr;

	for( uint8 ctrl = 0; ctrl < NOF_CANSM_CANIF_CONTROLLERS; ctrl++ ) {
		CanSM_Internal.ControllerModeBuf[ctrl].controllerId = INVALID_CONTROLLER_ID;
		CanSM_Internal.ControllerModeBuf[ctrl].indCtrlMode = CANIF_CS_STOPPED;
		CanSM_Internal.ControllerModeBuf[ctrl].hasPendingCtrlIndication = FALSE;
	}
	for (uint8 i = 0; i < CANSM_NETWORK_COUNT; ++i) {
		CanSM_Internal.Networks[i].requestedMode = COMM_NO_COMMUNICATION;
		CanSM_Internal.Networks[i].currentMode = COMM_NO_COMMUNICATION;
		CanSM_Internal.Networks[i].BsmState = CANSM_BSM_S_NOT_INITIALIZED;
		CanSM_Internal.Networks[i].FullCommState = CANSM_FULLCOMM_S_BUSOFF_CHECK;
		CanSM_Internal.Networks[i].PreFullCommState = CANSM_PREFULLCOMM_S_TRCV_NORMAL;
		CanSM_Internal.Networks[i].PreNoCommState = CANSM_PRENOCOMM_S_CC_STOPPED;

		CanSM_Internal.Networks[i].initialNoComRun = FALSE;
		CanSM_Internal.Networks[i].RepeatCounter = 0;

		for(uint8 ctrl = 0; ctrl < CanSM_Config->Networks[i].ControllerCount; ctrl++) {
			for(uint8 index = 0; index < NOF_CANSM_CANIF_CONTROLLERS; index++) {
				if( CanSM_Config->Networks[i].Controllers[ctrl].CanIfControllerId ==
					CanSM_Internal.ControllerModeBuf[index].controllerId ) {
					/* Controller already indexed */
					break;
				} else if( INVALID_CONTROLLER_ID == CanSM_Internal.ControllerModeBuf[index].controllerId ) {
					/* Empty slot, insert controller id */
					CanSM_Internal.ControllerModeBuf[index].controllerId = CanSM_Config->Networks[i].Controllers[ctrl].CanIfControllerId;
					break;
				}
			}
		}
	}

	CanSM_Internal.InitStatus = CANSM_STATUS_INIT;

}

/* @req CANSM064 */
void CanSM_ControllerBusOff(uint8 ControllerId)
{
	/* @req CANSM190 */
	if( CANSM_STATUS_UNINIT == CanSM_Internal.InitStatus ) {
		CANSM_DET_REPORTERROR(CANSM_SERVICEID_CONTROLLERBUSOFF, CANSM_E_UNINIT);
		return;
	}

	Std_ReturnType status = E_NOT_OK;

	// Find which network has this controller
	for (uint8 i = 0; i < CANSM_NETWORK_COUNT; ++i) {
		const CanSM_NetworkType* Network = &CanSM_Config->Networks[i];
		for (uint8 j = 0; j < Network->ControllerCount; ++j) {
			const CanSM_ControllerType* ptrController = &Network->Controllers[j];
			if(ptrController->CanIfControllerId == ControllerId)
			{
				if (CANSM_BSM_S_FULLCOM == CanSM_Internal.Networks[i].BsmState)
				{
					/* @req CANSM235 */
					CanSM_Internal.Networks[i].busoffevent = TRUE;
				}
                else {
                    CANSM_DET_REPORTERROR(CANSM_SERVICEID_CONTROLLERBUSOFF, CANSM_E_INVALID_INVALID_BUSOFF);
                }
				status = E_OK;
			}
		}
	}

	// Check if controller was valid
	if(status != E_OK){
		/* @req CANSM189 */
		CANSM_DET_REPORTERROR(CANSM_SERVICEID_CONTROLLERBUSOFF, CANSM_E_PARAM_CONTROLLER);
	}
}

/* @req CANSM062 */
Std_ReturnType CanSM_RequestComMode( NetworkHandleType NetworkHandle, ComM_ModeType ComM_Mode ) {

	/* @req CANSM182 */

	/* @req CANSM369 */
	/* @req CANSM370 */

	/* @req CANSM555 */
	if( CANSM_STATUS_UNINIT == CanSM_Internal.InitStatus ) {
		/* @req CANSM184 */
		CANSM_DET_REPORTERROR(CANSM_SERVICEID_REQUESTCOMMODE, CANSM_E_UNINIT);
		return E_NOT_OK;
	}

	if( NetworkHandle >= CANSM_NETWORK_COUNT) {
		/* @req CANSM183 */
		CANSM_DET_REPORTERROR(CANSM_SERVICEID_REQUESTCOMMODE, CANSM_E_INVALID_NETWORK_HANDLE);
		return E_NOT_OK;
	}
	/* Not a requirement but still.. */
	if( (COMM_NO_COMMUNICATION != ComM_Mode) && (COMM_SILENT_COMMUNICATION != ComM_Mode) && (COMM_FULL_COMMUNICATION != ComM_Mode) ) {
		CANSM_DET_REPORTERROR(CANSM_SERVICEID_REQUESTCOMMODE, CANSM_E_INVALID_NETWORK_MODE);
		return E_NOT_OK;
	}
	CanSM_Internal_NetworkType *Network = &CanSM_Internal.Networks[NetworkHandle];
	/* @req CANSM278 */
	if( CANSM_BSM_S_NOT_INITIALIZED == Network->BsmState ) {
		return E_NOT_OK;
	}

	if(E_OK != CanSM_Internal_GetNetworkControllerModeIndicated(NetworkHandle)) {
		/* @req CANSM395 */
		/* Pending indication from CanIf */
		CANSM_DET_REPORTERROR(CANSM_SERVICEID_REQUESTCOMMODE, CANSM_E_WAIT_MODE_INDICATION);
		return E_NOT_OK;
	}

	/* @req CANSM402 */
	/* Added to this requirement that state machine cannot be in CANSM_BSM_S_PRECOM
	 * if COMM_SILENT_COMMUNICATION is requested this to handle the case when
	 * CANSM_BSM_S_FULLCOM exits with T_REPEAT_MAX. */
	if( ((COMM_NO_COMMUNICATION == CanSM_Internal.Networks[NetworkHandle].currentMode) ||
			(CANSM_BSM_S_PRE_NOCOM == CanSM_Internal.Networks[NetworkHandle].BsmState)) &&
			(COMM_SILENT_COMMUNICATION == ComM_Mode)) {
		/* @req CANSM403 */
		CANSM_DET_REPORTERROR(CANSM_SERVICEID_REQUESTCOMMODE, CANSM_E_INVALID_COMM_REQUEST);
		return E_NOT_OK;
	}

	/* Translate below to busoff state */
	/* @req CANSM375 */
	/* @req CANSM376 */
	if( (CANSM_BSM_S_FULLCOM == CanSM_Internal.Networks[NetworkHandle].BsmState) &&
			(CANSM_FULLCOMM_S_BUSOFF_CHECK != CanSM_Internal.Networks[NetworkHandle].FullCommState) &&
			(CANSM_FULLCOMM_S_NO_BUSOFF != CanSM_Internal.Networks[NetworkHandle].FullCommState) ) {
		/* @req CANSM377 */
		CANSM_DET_REPORTERROR(CANSM_SERVICEID_REQUESTCOMMODE, CANSM_E_BUSOFF_RECOVERY_ACTIVE);
		return E_NOT_OK;
	}
	/* Deny request if there is a busoff event. Do not report error here since upper layer cannot
	 * know that there is a busoff event present. If busoff event has been handled in main function
	 * we will never end up here as if should be handled by check above. */
	if( CanSM_Internal.Networks[NetworkHandle].busoffevent ) {
		return E_NOT_OK;
	}
	/* Request accepted! */
	CanSM_Internal.Networks[NetworkHandle].requestedMode = ComM_Mode;

	return E_OK;
}

/* @req CANSM063 */
Std_ReturnType CanSM_GetCurrentComMode( NetworkHandleType NetworkHandle, ComM_ModeType* ComM_ModePtr ) {

	/* @req CANSM188 */
	if( CANSM_STATUS_UNINIT == CanSM_Internal.InitStatus ) {
		CANSM_DET_REPORTERROR(CANSM_SERVICEID_GETCURRENTCOMMODE, CANSM_E_UNINIT);
		return E_NOT_OK;
	}

	/* @req CANSM371 */
	/* @req CANSM372 */
	if( NetworkHandle > CANSM_NETWORK_COUNT) {
		/* @req CANSM187 */
		CANSM_DET_REPORTERROR(CANSM_SERVICEID_GETCURRENTCOMMODE, CANSM_E_INVALID_NETWORK_HANDLE);
		return E_NOT_OK;
	}

	if( NULL == ComM_ModePtr ) {
		/* @req CANSM360 */
		CANSM_DET_REPORTERROR(CANSM_SERVICEID_GETCURRENTCOMMODE, CANSM_E_PARAM_POINTER);
		return E_NOT_OK;
	}

	/* @req CANSM282 */
	if( FALSE == CanSM_Internal.Networks[NetworkHandle].initialNoComRun ) {
		return E_NOT_OK;
	}

	/* @req CANSM186 */
	*ComM_ModePtr = CanSM_Internal.Networks[NetworkHandle].currentMode;
	return E_OK;
}

void CanSM_Internal_SetNetworkPduMode(NetworkHandleType NetworkHandle, CanIf_PduSetModeType pduMode)
{
	const CanSM_NetworkType *NetworkCfg = &CanSM_Config->Networks[NetworkHandle];

	for(uint8 ctrl = 0; ctrl < NetworkCfg->ControllerCount; ctrl++) {
		if( E_OK != CanIf_SetPduMode(NetworkCfg->Controllers[ctrl].CanIfControllerId, pduMode) ) {
			/* TODO: Report some kind of Det-error. */
//			CANSM_DET_REPORTERROR();
		}
	}
}

Std_ReturnType CanSM_Internal_SetNetworkControllerMode(NetworkHandleType NetworkHandle, CanIf_ControllerModeType controllerMode)
{
	Std_ReturnType totalStatus = E_OK;
	const CanSM_NetworkType *NetworkCfg = &CanSM_Config->Networks[NetworkHandle];
	for(uint8 ctrl = 0; ctrl < NetworkCfg->ControllerCount; ctrl++) {
		uint8 index;
		if( E_OK != CanSM_Internal_GetCanSMControllerIndex(NetworkCfg->Controllers[ctrl].CanIfControllerId, &index) ) {
			return E_NOT_OK;
		}

		CanSM_Internal.ControllerModeBuf[index].hasPendingCtrlIndication = TRUE;
		CanSM_Internal.ControllerModeBuf[index].pendingCtrlMode = controllerMode;
		if( E_OK != CanIf_SetControllerMode(NetworkCfg->Controllers[ctrl].CanIfControllerId, controllerMode)) {
			/* Not accepted. No mode indication pending */
			CanSM_Internal.ControllerModeBuf[index].hasPendingCtrlIndication = FALSE;
			totalStatus = E_NOT_OK;
		}
	}

	return totalStatus;
}

Std_ReturnType CanSM_Internal_GetNetworkControllerModeIndicated(NetworkHandleType NetworkHandle)
{
	Std_ReturnType totalStatus = E_OK;
	const CanSM_NetworkType *NetworkCfg = &CanSM_Config->Networks[NetworkHandle];

	for(uint8 ctrl = 0; ctrl < NetworkCfg->ControllerCount; ctrl++) {
		uint8 ctrlIndex;
		if( E_OK != CanSM_Internal_GetCanSMControllerIndex(NetworkCfg->Controllers[ctrl].CanIfControllerId, &ctrlIndex) ) {
			return E_NOT_OK;
		}

		if( CanSM_Internal.ControllerModeBuf[ctrlIndex].hasPendingCtrlIndication ) {
			totalStatus = E_NOT_OK;
		}

	}
	return totalStatus;
}

void CanSM_Internal_ComM_BusSM_ModeIndication( NetworkHandleType NetworkHandle, ComM_ModeType ComM_Mode )
{
	/* Update internals */
	CanSM_Internal.Networks[NetworkHandle].currentMode = ComM_Mode;
	/* Indicate to ComM */
	ComM_BusSM_ModeIndication(CanSM_Config->Networks[NetworkHandle].ComMNetworkHandle, &ComM_Mode);
}

CanSM_Internal_PreNoCommExitPointType CanSm_Internal_BSM_S_PRE_NOCOM( NetworkHandleType NetworkHandle )
{
	// Sub state machine CANSM_BSM_S_PRE_NOCOM requirements

	// Guard: CANSM_BSM_G_PN_NOT_SUPPORTED
	/* !req:CANTRCV CANSM436 */
	// Guard: CANSM_BSM_G_PN_SUPPORTED
	/* !req:CANTRCV CANSM437*/


	/* Comment: Partial networking not supported so the following requirements are not fulfilled

	*
	* Deinit with PN support
	* !req:PN CANSM438
	* !req:PN CANSM439
	* !req:PN CANSM440
	* !req:PN CANSM443
	* !req:PN CANSM441
	* !req:PN CANSM442
	* !req:PN CANSM444
	* !req:PN CANSM445
	* !req:PN CANSM446
	* !req:PN CANSM447
	* !req:PN CANSM448
	* !req:PN CANSM449
	* !req:PN CANSM450
	* !req:PN CANSM451
	* !req:PN CANSM452
	* !req:PN CANSM454
	* !req:PN CANSM453
	* !req:PN CANSM455
	* !req:PN CANSM456
	* !req:PN CANSM457
	* !req:PN CANSM458
	* !req:PN CANSM459
	* !req:PN CANSM460
	* !req:PN CANSM461
	* !req:PN CANSM462
	* !req:PN CANSM463
	*  */

	/* Requirements when partial networking not supported. */

	/* !req:CANTRCV CANSM472 */
	/* !req:CANTRCV CANSM473 */
	/* !req:CANTRCV CANSM474 */
	/* !req:CANTRCV CANSM556 */
	/* !req:CANTRCV CANSM475 */
	/* !req:CANTRCV CANSM476 */
	/* !req:CANTRCV CANSM477 */
	/* !req:CANTRCV CANSM478 */
	/* !req:CANTRCV CANSM557 */
	/* !req:CANTRCV CANSM479 */
	/* And CANSM471 */

	CanSM_Internal_PreNoCommExitPointType exitPoint = CANSM_PRENOCOMM_EXITPOINT_NONE;

	CanSM_Internal_NetworkType *Network = &CanSM_Internal.Networks[NetworkHandle];

	switch(Network->PreNoCommState)
	{
	case CANSM_PRENOCOMM_S_CC_STOPPED:
	{
		if( Network->RepeatCounter <= CanSM_Config->CanSMModeRequestRepetitionMax ) {
			Network->RepeatCounter++;
			/* @req CANSM464 */
			/* @req CANSM465 */
			if( E_OK == CanSM_Internal_SetNetworkControllerMode(NetworkHandle, CANIF_CS_STOPPED) ) {
				/* At least ok to enter CC_STOPPED_WAIT */
				if( E_OK == CanSM_Internal_GetNetworkControllerModeIndicated(NetworkHandle) ) {
					/*@req CANSM466 */
					Network->PreNoCommState = CANSM_PRENOCOMM_S_CC_SLEEP;
					Network->RepeatCounter = 0;
				} else {
					Network->PreNoCommState = CANSM_PRENOCOMM_S_CC_STOPPED_WAIT;
					Network->subStateTimer = 0;
				}
			}
		} else {
			/* @req CANSM480 */
			/* @req CANSM385 */
			CANSM_DET_REPORTERROR(CANSM_SERVICEID_MAINFUNCTION, CANSM_E_MODE_REQUEST_TIMEOUT);
			Network->RepeatCounter = 0;
			Network->subStateTimer = 0;
			Network->PreNoCommState = CANSM_PRENOCOMM_S_CC_STOPPED;
		}
		break;
	}
	case CANSM_PRENOCOMM_S_CC_STOPPED_WAIT:
	{
		Network->subStateTimer++;
		if( E_OK == CanSM_Internal_GetNetworkControllerModeIndicated(NetworkHandle)) {
			/* @req CANSM466 */
			Network->PreNoCommState = CANSM_PRENOCOMM_S_CC_SLEEP;
			Network->RepeatCounter = 0;

		} else if( Network->subStateTimer >= CanSM_Config->CanSMModeRequestRepetitionTime ) {
			/* @req CANSM467 */
			Network->PreNoCommState = CANSM_PRENOCOMM_S_CC_STOPPED;
			break;
		} else
			break;

	}
	//lint -fallthrough
	case CANSM_PRENOCOMM_S_CC_SLEEP:
	{
		if( Network->RepeatCounter <= CanSM_Config->CanSMModeRequestRepetitionMax ) {
			Network->RepeatCounter++;
			/* @req CANSM468 */
			/* @req CANSM469 */
			if( E_OK == CanSM_Internal_SetNetworkControllerMode(NetworkHandle, CANIF_CS_SLEEP) ) {
				/* At least ok to enter CC_SLEEP_WAIT */
				if( E_OK == CanSM_Internal_GetNetworkControllerModeIndicated(NetworkHandle) ) {
					/*@req CANSM470 */
					Network->PreNoCommState = CANSM_PRENOCOMM_S_TRCV_NORMAL;
					Network->RepeatCounter = 0;
				} else {
					Network->PreNoCommState = CANSM_PRENOCOMM_S_CC_SLEEP_WAIT;
					Network->subStateTimer = 0;
				}
			}
		} else {
			/* @req CANSM480 */
			/* @req CANSM385 */
			CANSM_DET_REPORTERROR(CANSM_SERVICEID_MAINFUNCTION, CANSM_E_MODE_REQUEST_TIMEOUT);
			Network->RepeatCounter = 0;
			Network->PreNoCommState = CANSM_PRENOCOMM_S_CC_STOPPED;
		}
		break;
	}
	case CANSM_PRENOCOMM_S_CC_SLEEP_WAIT:
	{
		Network->subStateTimer++;
		if( E_OK == CanSM_Internal_GetNetworkControllerModeIndicated(NetworkHandle)) {
			/* @req CANSM466 */
			Network->PreNoCommState = CANSM_PRENOCOMM_S_TRCV_NORMAL;
			Network->RepeatCounter = 0;
		} else if( Network->subStateTimer >= CanSM_Config->CanSMModeRequestRepetitionTime ) {
			/* @req CANSM471 */
			Network->PreNoCommState = CANSM_PRENOCOMM_S_CC_SLEEP;
			break;
		} else {
			break;
		}

	}
	case CANSM_PRENOCOMM_S_TRCV_NORMAL:
	case CANSM_PRENOCOMM_S_TRCV_NORMAL_WAIT:
	case CANSM_PRENOCOMM_S_TRCV_STANDBY:
	case CANSM_PRENOCOMM_S_TRCV_STANDBY_WAIT:
		/* No support for CanTrcv so just flow through and exit to NOCOMM.
		 * In some way @req CANSM560 */
		exitPoint = CANSM_PRENOCOMM_EXITPOINT_NOCOM;
		break;

	default:
		break;
	}

	return exitPoint;
}

CanSM_Internal_PreFullCommExitPointType CanSm_Internal_BSM_S_PRE_FULLCOM( NetworkHandleType NetworkHandle )
{
	/* Sub state machine to prepare full communication requirements
	 *
	 * !req:CANTRCV CANSM483
	 * !req:CANTRCV CANSM484
	 * !req:CANTRCV CANSM485
	 * !req:CANTRCV CANSM558
	 * !req:CANTRCV CANSM486
	 *
	 * */

	/* TODO: Some states are very similar. Fix this!
		 *  */
	CanSM_Internal_PreFullCommExitPointType exitPoint = CANSM_PREFULLCOMM_EXITPOINT_NONE;

	CanSM_Internal_NetworkType *Network = &CanSM_Internal.Networks[NetworkHandle];

	switch(Network->PreFullCommState)
	{
	case CANSM_PREFULLCOMM_S_TRCV_NORMAL:
	case CANSM_PREFULLCOMM_S_TRCV_NORMAL_WAIT:
		/* No support for CanTrcv so just flow through and enter S_CC_STOPPED.
		 * In some way @req CANSM560. */
		Network->PreFullCommState = CANSM_PREFULLCOMM_S_CC_STOPPED;
		break;
	case CANSM_PREFULLCOMM_S_CC_STOPPED:
	{
		if( Network->RepeatCounter <= CanSM_Config->CanSMModeRequestRepetitionMax ) {
			Network->RepeatCounter++;
			/* @req CANSM487 */
			/* @req CANSM488 */
			if( E_OK == CanSM_Internal_SetNetworkControllerMode(NetworkHandle, CANIF_CS_STOPPED) ) {
				/* At least ok to enter CC_STOPPED_WAIT */
				if( E_OK == CanSM_Internal_GetNetworkControllerModeIndicated(NetworkHandle) ) {
					/*@req CANSM489 */
					Network->PreFullCommState = CANSM_PREFULLCOMM_S_CC_STARTED;
					Network->RepeatCounter = 0;
				} else {
					Network->PreFullCommState = CANSM_PREFULLCOMM_S_CC_STOPPED_WAIT;
					Network->subStateTimer = 0;
				}
			}
		} else {
			/* @req CANSM495 */
			/* @req CANSM385 */
			CANSM_DET_REPORTERROR(CANSM_SERVICEID_MAINFUNCTION, CANSM_E_MODE_REQUEST_TIMEOUT);
			Network->RepeatCounter = 0;
			exitPoint = CANSM_PREFULLCOMM_EXITPOINT_REPEAT_MAX;
		}
		break;
	}
	case CANSM_PREFULLCOMM_S_CC_STOPPED_WAIT:
	{
		Network->subStateTimer++;
		if( E_OK == CanSM_Internal_GetNetworkControllerModeIndicated(NetworkHandle)) {
			/* @req CANSM489 */
			Network->PreFullCommState = CANSM_PREFULLCOMM_S_CC_STARTED;
			Network->RepeatCounter = 0;
		} else if( Network->subStateTimer >= CanSM_Config->CanSMModeRequestRepetitionTime ) {
			/* @req CANSM490 */
			Network->PreFullCommState = CANSM_PREFULLCOMM_S_CC_STOPPED;
			break;
		} else
		break;
	}
	//lint -fallthrough
	case CANSM_PREFULLCOMM_S_CC_STARTED:
	{
		if( Network->RepeatCounter <= CanSM_Config->CanSMModeRequestRepetitionMax ) {
			Network->RepeatCounter++;
			/* @req CANSM491 */
			/* @req CANSM492 */
			if( E_OK == CanSM_Internal_SetNetworkControllerMode(NetworkHandle, CANIF_CS_STARTED) ) {
				/* At least ok to enter CC_STARTED_WAIT */
				if( E_OK == CanSM_Internal_GetNetworkControllerModeIndicated(NetworkHandle) ) {
					/*@req CANSM493 */
					Network->RepeatCounter = 0;
					exitPoint = CANSM_PREFULLCOMM_EXITPOINT_FULLCOMM;
				} else {
					Network->PreFullCommState = CANSM_PREFULLCOMM_S_CC_STARTED_WAIT;
					Network->subStateTimer = 0;
				}
			}
		} else {
			/* @req CANSM495 */
			/* @req CANSM385 */
			CANSM_DET_REPORTERROR(CANSM_SERVICEID_MAINFUNCTION, CANSM_E_MODE_REQUEST_TIMEOUT);
			Network->RepeatCounter = 0;
			exitPoint = CANSM_PREFULLCOMM_EXITPOINT_REPEAT_MAX;
		}
		break;
	}
	case CANSM_PREFULLCOMM_S_CC_STARTED_WAIT:
	{
		Network->subStateTimer++;
		if( E_OK == CanSM_Internal_GetNetworkControllerModeIndicated(NetworkHandle)) {
			/* @req CANSM493 */
			Network->RepeatCounter = 0;
			exitPoint = CANSM_PREFULLCOMM_EXITPOINT_FULLCOMM;
		} else if( Network->subStateTimer >= CanSM_Config->CanSMModeRequestRepetitionTime ) {
			/* @req CANSM494 */
			Network->PreFullCommState = CANSM_PREFULLCOMM_S_CC_STARTED;
		}
		break;
	}
	default:
		break;
	}
	return exitPoint;
}

CanSM_Internal_FullCommExitPointType CanSm_Internal_BSM_S_FULLCOM( NetworkHandleType NetworkHandle )
{
	/* Sub state machine for requested full communication requirements
	 *

	 * !req:CHANGEBAUDRATE CANSM507
	 * !req:CHANGEBAUDRATE CANSM528
	 *  */
	CanSM_Internal_FullCommExitPointType exitPoint = CANSM_FULLCOMM_EXITPOINT_NONE;
	CanSM_Internal_NetworkType *Network = &CanSM_Internal.Networks[NetworkHandle];
	const CanSM_NetworkType *NetworkCfg = &CanSM_Config->Networks[NetworkHandle];

	switch(Network->FullCommState)
	{
	case CANSM_FULLCOMM_S_BUSOFF_CHECK:
		Network->subStateTimer++;
		if( Network->busoffevent ) {
			Network->busoffevent = FALSE;
			if( Network->busoffCounter < NetworkCfg->CanSMBorCounterL1ToL2 ) {
				Network->busoffCounter++;
			}
			/* @req CANSM500 */
#if defined(USE_BSWM)
			/* @req CANSM508*/
			BswM_CanSM_CurrentState(NetworkCfg->ComMNetworkHandle, CANSM_BSWM_BUS_OFF);
#endif
			/* @req CANSM521 */
			CanSM_Internal_ComM_BusSM_ModeIndication(NetworkHandle, COMM_SILENT_COMMUNICATION);

#if defined(USE_DEM)
			/* @req CANSM522 */
			if( DEM_EVENT_ID_NULL != NetworkCfg->CanSMDemEventId ) {
				DEM_REPORT_ERROR_STATUS(NetworkCfg->CanSMDemEventId, DEM_EVENT_STATUS_PREFAILED);
			}
#endif

			Network->RepeatCounter = 0;
			Network->FullCommState = CANSM_FULLCOMM_S_RESTART_CC;
			break;
		} else if( Network->subStateTimer >= NetworkCfg->CanSMBorTimeTxEnsured ) {
			/* @req CANSM496 */
			/* !req:TXCONFIRMPOLLING CANSM497 */

#if defined(USE_DEM)
			/* @req CANSM498 */
			if( DEM_EVENT_ID_NULL != NetworkCfg->CanSMDemEventId ) {
				DEM_REPORT_ERROR_STATUS(NetworkCfg->CanSMDemEventId, DEM_EVENT_STATUS_PASSED);
			}
#endif
			Network->busoffCounter = 0;
			Network->FullCommState = CANSM_FULLCOMM_S_NO_BUSOFF;
		} else
			break;
	//lint -fallthrough
	case CANSM_FULLCOMM_S_NO_BUSOFF:
		if( Network->busoffevent ) {
			Network->busoffevent = FALSE;
			if( Network->busoffCounter < NetworkCfg->CanSMBorCounterL1ToL2 ) {
				Network->busoffCounter++;
			}

			/* @req CANSM500 */
#if defined(USE_BSWM)
			/* @req CANSM508 */
			BswM_CanSM_CurrentState(NetworkCfg->ComMNetworkHandle, CANSM_BSWM_BUS_OFF);
#endif
            /* @req CANSM521 */
			CanSM_Internal_ComM_BusSM_ModeIndication(NetworkHandle, COMM_SILENT_COMMUNICATION);
#if defined(USE_DEM)
			/* @req CANSM522 */
			if( DEM_EVENT_ID_NULL != NetworkCfg->CanSMDemEventId ) {
				Dem_ReportErrorStatus(NetworkCfg->CanSMDemEventId, DEM_EVENT_STATUS_PREFAILED);
			}
#endif

			Network->RepeatCounter = 0;
			Network->FullCommState = CANSM_FULLCOMM_S_RESTART_CC;
		} else if( (COMM_SILENT_COMMUNICATION == Network->requestedMode) ||
				( COMM_NO_COMMUNICATION == Network->requestedMode ) ) {
			/* @req CANSM499 */
			/* @req CANSM554 */
			exitPoint = CANSM_FULLCOMM_EXITPOINT_SILENTCOMM;
		}
		break;
	case CANSM_FULLCOMM_S_RESTART_CC:
	{
		if( Network->RepeatCounter <= CanSM_Config->CanSMModeRequestRepetitionMax ) {
			Network->RepeatCounter++;
			/* @req CANSM509 */
			if( E_OK == CanSM_Internal_SetNetworkControllerMode(NetworkHandle, CANIF_CS_STARTED) ) {
				if( E_OK == CanSM_Internal_GetNetworkControllerModeIndicated(NetworkHandle) ) {
					/* @req CANSM511 */
					/* @req CANSM513 */
					CanSM_Internal_SetNetworkPduMode(NetworkHandle, CANIF_SET_TX_OFFLINE);
					Network->FullCommState = CANSM_FULLCOMM_S_TX_OFF;
				} else {
					/* @req CANSM510 */
					Network->FullCommState = CANSM_FULLCOMM_S_RESTART_CC_WAIT;
				}
				Network->subStateTimer = 0;
			}
		} else {
			/* @req CANSM523 */
			/* @req CANSM385 */
			CANSM_DET_REPORTERROR(CANSM_SERVICEID_MAINFUNCTION, CANSM_E_MODE_REQUEST_TIMEOUT);
			exitPoint = CANSM_FULLCOMM_EXITPOINT_PRENOCOMM;
		}
		break;
	}
	case CANSM_FULLCOMM_S_RESTART_CC_WAIT:
		Network->subStateTimer++;
		if( E_OK == CanSM_Internal_GetNetworkControllerModeIndicated(NetworkHandle)) {
			/* @req CANSM511 */
			/* @req CANSM513 */
			CanSM_Internal_SetNetworkPduMode(NetworkHandle, CANIF_SET_TX_OFFLINE);
			Network->subStateTimer = 0;
			Network->FullCommState = CANSM_FULLCOMM_S_TX_OFF;
		} else if( Network->subStateTimer >= CanSM_Config->CanSMModeRequestRepetitionTime ) {
			/* @req CANSM512 */
			Network->FullCommState = CANSM_FULLCOMM_S_RESTART_CC;
		}
		break;
	case CANSM_FULLCOMM_S_TX_OFF:
		/* @req CANSM514 */
		/* @req CANSM515 */
		Network->subStateTimer++;
		if( ((Network->subStateTimer >= NetworkCfg->CanSMBorTimeL1) &&
				(Network->busoffCounter < NetworkCfg->CanSMBorCounterL1ToL2)) ||
				((Network->subStateTimer >= NetworkCfg->CanSMBorTimeL2) &&
				(Network->busoffCounter >= NetworkCfg->CanSMBorCounterL1ToL2))) {
#if defined(USE_CANSM_EXTENSION)
		    CanSM_Extension_WriteState(NetworkHandle, Network->busoffCounter, Network->subStateTimer, NetworkCfg);
#endif
			/* @req CANSM516 */
			CanSM_Internal_SetNetworkPduMode(NetworkHandle, CANIF_SET_ONLINE);
#if defined(USE_BSWM)
			/* @req CANSM517*/
			BswM_CanSM_CurrentState(NetworkCfg->ComMNetworkHandle, CANSM_BSWM_FULL_COMMUNICATION);
#endif
			/* @req CANSM518 */
			CanSM_Internal_ComM_BusSM_ModeIndication(NetworkHandle, COMM_FULL_COMMUNICATION);
			Network->FullCommState = CANSM_FULLCOMM_S_BUSOFF_CHECK;
			Network->subStateTimer = 0;
		}
		break;
	default:
		break;
	}

	return exitPoint;
}

/* @req CANSM065 */
void CanSM_MainFunction() {

	/* @req CANSM167 */
	/* @req CANSM266 */
	/* @req CANSM284 */
	/* @req CANSM428 */
	/* !req:PN CANSM422 */

	/* Main state machine requirements


	 * Guard: G_FULL_COM_MODE_REQUESTED
	 * !req:CHANGEBAUDRATE CANSM427
	 * Guard: G_SILENT_COM_MODE_REQUESTED
	 * !req:CHANGEBAUDRATE CANSM429

	 * Effect: E_BR_END_FULL_COM
	 * !req:CHANGEBAUDRATE CANSM432
	 * Effect: E_BR_END_SILENT_COM
	 * !req:CHANGEBAUDRATE CANSM433
	 * */

	/* Sub state machine to operate a requested baud rate change
	 *
	 * !req:CHANGEBAUDRATE CANSM524
	 * !req:CHANGEBAUDRATE CANSM525
	 * !req:CHANGEBAUDRATE CANSM526
	 * !req:CHANGEBAUDRATE CANSM527
	 * !req:CHANGEBAUDRATE CANSM529
	 * !req:CHANGEBAUDRATE CANSM531
	 * !req:CHANGEBAUDRATE CANSM532
	 * !req:CHANGEBAUDRATE CANSM533
	 * !req:CHANGEBAUDRATE CANSM534
	 * !req:CHANGEBAUDRATE CANSM535
	 * !req:CHANGEBAUDRATE CANSM536
	 * !req:CHANGEBAUDRATE CANSM542
	 * !req:CHANGEBAUDRATE CANSM543
	 *  */


	if( CANSM_STATUS_UNINIT == CanSM_Internal.InitStatus ) {
		return;
	}
	for (uint8 networkHandle = 0; networkHandle < CANSM_NETWORK_COUNT; networkHandle++) {
		CanSM_Internal_NetworkType *Network = &CanSM_Internal.Networks[networkHandle];
#if defined(USE_BSWM)
		const CanSM_NetworkType *NetworkCfg = &CanSM_Config->Networks[networkHandle];
#endif
		CanSM_Internal_BsmStateType prevState = Network->BsmState;
		switch(Network->BsmState)
		{
		case CANSM_BSM_S_NOT_INITIALIZED:
			/* @req CANSM423 */
			if( CANSM_STATUS_INIT == CanSM_Internal.InitStatus ) {
#if defined(USE_BSWM)
				/* @req CANSM431 */
				BswM_CanSM_CurrentState(NetworkCfg->ComMNetworkHandle, CANSM_BSWM_NO_COMMUNICATION);
#endif
				Network->BsmState = CANSM_BSM_S_PRE_NOCOM;
			}
			break;
		case CANSM_BSM_S_PRE_NOCOM:
			if( CANSM_PRENOCOMM_EXITPOINT_NOCOM == CanSm_Internal_BSM_S_PRE_NOCOM(networkHandle) ) {
				/* @req CANSM430 */
				CanSM_Internal_ComM_BusSM_ModeIndication(networkHandle, COMM_NO_COMMUNICATION);
				Network->initialNoComRun = TRUE;
				Network->BsmState = CANSM_BSM_S_NOCOM;
			}
			break;
		case CANSM_BSM_S_NOCOM:
			 /* @req CANSM425 */
			if( COMM_FULL_COMMUNICATION == Network->requestedMode ) {
				Network->BsmState = CANSM_BSM_S_PRE_FULLCOM;
			}
			break;
		case CANSM_BSM_S_PRE_FULLCOM:
		{
			CanSM_Internal_PreFullCommExitPointType preFullCommExit = CanSm_Internal_BSM_S_PRE_FULLCOM(networkHandle);
			if( CANSM_PREFULLCOMM_EXITPOINT_FULLCOMM == preFullCommExit ) {
				Network->BsmState = CANSM_BSM_S_FULLCOM;
#if defined(USE_BSWM)
				/* @req CANSM435 */
				BswM_CanSM_CurrentState(NetworkCfg->ComMNetworkHandle, CANSM_BSWM_FULL_COMMUNICATION);
#endif
				/* @req CANSM539 */
				CanSM_Internal_SetNetworkPduMode(networkHandle, CANIF_SET_ONLINE);
				/* @req CANSM540 */
				CanSM_Internal_ComM_BusSM_ModeIndication(networkHandle, COMM_FULL_COMMUNICATION);
			} else if( CANSM_PREFULLCOMM_EXITPOINT_REPEAT_MAX == preFullCommExit ) {
				Network->BsmState = CANSM_BSM_S_PRE_NOCOM;
			}
			break;
		}
		case CANSM_BSM_S_FULLCOM:
		{
			CanSM_Internal_FullCommExitPointType fullCommExit = CanSm_Internal_BSM_S_FULLCOM(networkHandle);
			if(CANSM_FULLCOMM_EXITPOINT_PRENOCOMM == fullCommExit) {
				Network->BsmState = CANSM_BSM_S_PRE_NOCOM;
#if defined(USE_BSWM)
				/* @req CANSM431 */
				BswM_CanSM_CurrentState(NetworkCfg->ComMNetworkHandle, CANSM_BSWM_NO_COMMUNICATION);
#endif
			} else if( CANSM_FULLCOMM_EXITPOINT_SILENTCOMM == fullCommExit ) {
				Network->BsmState = CANSM_BSM_S_SILENTCOM;
#if defined(USE_BSWM)
				/* @req CANSM434 */
				BswM_CanSM_CurrentState(NetworkCfg->ComMNetworkHandle, CANSM_BSWM_SILENT_COMMUNICATION);
#endif
				/* @req CANSM541 */
				CanSM_Internal_SetNetworkPduMode(networkHandle, CANIF_SET_ONLINE);
				/* @req CANSM537 */
				CanSM_Internal_SetNetworkPduMode(networkHandle, CANIF_SET_TX_OFFLINE);
				/* @req CANSM538 */
				CanSM_Internal_ComM_BusSM_ModeIndication(networkHandle, COMM_SILENT_COMMUNICATION);

			}
			break;
		}
		case CANSM_BSM_S_SILENTCOM:

			if( COMM_NO_COMMUNICATION == Network->requestedMode ) {
				 // Trigger: T_NO_COM_REQUEST
#if defined(USE_BSWM)
				/* @req CANSM431 */
				BswM_CanSM_CurrentState(NetworkCfg->ComMNetworkHandle, CANSM_BSWM_NO_COMMUNICATION);
#endif
				 /* @req CANSM426 */
				Network->BsmState = CANSM_BSM_S_PRE_NOCOM;
			} else if( COMM_FULL_COMMUNICATION == Network->requestedMode ) {
				// Effect: E_SILENT_TO_FULL_COM
				/* Should be same as E_FULL_COMM. Spec. refers to CANSM435 but only obeying that
				 * requirement would not be the same as E_FULL_COMM. */
				/* @req CANSM550 */
				Network->BsmState = CANSM_BSM_S_FULLCOM;
#if defined(USE_BSWM)
				/* @req CANSM435 CANSM550 */
				BswM_CanSM_CurrentState(NetworkCfg->ComMNetworkHandle, CANSM_BSWM_FULL_COMMUNICATION);
#endif
				CanSM_Internal_SetNetworkPduMode(networkHandle, CANIF_SET_ONLINE);
				CanSM_Internal_ComM_BusSM_ModeIndication(networkHandle, COMM_FULL_COMMUNICATION);

			}
			break;
		default:
			break;
		}

		if( (CANSM_BSM_S_PRE_NOCOM == Network->BsmState) &&
				(CANSM_BSM_S_PRE_NOCOM != prevState)) {
			/* Entered CANSM_BSM_S_PRE_NOCOM, reset pre no comm substate */
			Network->PreNoCommState = CANSM_PRENOCOMM_S_CC_STOPPED;
			Network->RepeatCounter = 0;
			Network->subStateTimer = 0;
		} else if( (CANSM_BSM_S_PRE_FULLCOM == Network->BsmState) &&
				(CANSM_BSM_S_PRE_FULLCOM != prevState)) {
			/* Entered CANSM_BSM_S_PRE_NOCOM, reset pre full comm substate */
			Network->PreFullCommState = CANSM_PREFULLCOMM_S_TRCV_NORMAL;
			Network->RepeatCounter = 0;
			Network->subStateTimer = 0;
		} else if( (CANSM_BSM_S_FULLCOM == Network->BsmState) &&
				(CANSM_BSM_S_FULLCOM != prevState)) {
			/* Entered CANSM_BSM_S_FULLCOM, reset full comm substate */
			Network->FullCommState = CANSM_FULLCOMM_S_BUSOFF_CHECK;
			Network->RepeatCounter = 0;
			Network->subStateTimer = 0;
		}

	}

}

/* @req CANSM396 */
void CanSM_ControllerModeIndication( uint8 ControllerId, CanIf_ControllerModeType ControllerMode )
{

	if( CANSM_STATUS_UNINIT == CanSM_Internal.InitStatus ) {
		/* @req CANSM398 */
		CANSM_DET_REPORTERROR(CANSM_SERVICEID_CONTROLLERMODEINDICATION, CANSM_E_UNINIT);
		return;
	}

	uint8 controllerIndex;
	if( E_OK == CanSM_Internal_GetCanSMControllerIndex(ControllerId, &controllerIndex) ) {
		/* Controller was valid */

		if( CanSM_Internal.ControllerModeBuf[controllerIndex].hasPendingCtrlIndication &&
				(ControllerMode == CanSM_Internal.ControllerModeBuf[controllerIndex].pendingCtrlMode) ) {
			CanSM_Internal.ControllerModeBuf[controllerIndex].hasPendingCtrlIndication = FALSE;
		}

		CanSM_Internal.ControllerModeBuf[controllerIndex].indCtrlMode = ControllerMode;
	} else {
		/* @req CANSM397 */
		CANSM_DET_REPORTERROR(CANSM_SERVICEID_CONTROLLERMODEINDICATION, CANSM_E_PARAM_CONTROLLER);
	}

}

#if 0
Std_ReturnType CanSm_CheckBaudrate(NetworkHandleType network, const uint16 Baudrate) {
	/* !req:CHANGEBAUDRATE CANSM564 */
	/* !req:CHANGEBAUDRATE CANSM565 */
	/* !req:CHANGEBAUDRATE CANSM562 */
	/* !req:CHANGEBAUDRATE CANSM571 */
	/* !req:CHANGEBAUDRATE CANSM563 */
	/* !req:CHANGEBAUDRATE CANSM566 */
	/* !req:CHANGEBAUDRATE CANSM567 */
	/* !req:CHANGEBAUDRATE CANSM568 */
	/* !req:CHANGEBAUDRATE CANSM572 */
	return E_NOT_OK;
}

Std_ReturnType CanSm_ChangeBaudrate(NetworkHandleType network, const uint16 Baudrate) {
	/* !req:CHANGEBAUDRATE CANSM569 */
	/* !req:CHANGEBAUDRATE CANSM570 */
	/* !req:CHANGEBAUDRATE CANSM502 */
	/* !req:CHANGEBAUDRATE CANSM504 */
	/* !req:CHANGEBAUDRATE CANSM505 */
	/* !req:CHANGEBAUDRATE CANSM530 */
	/* !req:CHANGEBAUDRATE CANSM506 */
	/* !req:CHANGEBAUDRATE CANSM573 */
	/* !req:CHANGEBAUDRATE CANSM574 */
	/* !req:CHANGEBAUDRATE CANSM503 */

	return E_NOT_OK;
}

void CanSM_TransceiverModeIndication( uint8 TransceiverId, CanTrcv_TrcvModeType TransceiverMode )
{
	/* !req:CANTRCV CANSM400 */
	/* !req:CANTRCV CANSM401 */
}

void CanSM_TxTimeoutException( NetworkHandleType Channel )
{
	/* !req:PN CANSM411 */
	/* !req:PN CANSM412 */
}

void CanSM_ClearTrcvWufFlagIndication( uint8 Transceiver )
{
	/* !req:CANTRCV CANSM414 */
	/* !req:CANTRCV CANSM415 */
}

void CanSM_CheckTransceiverWakeFlagIndication( uint8 Transceiver )
{
	/* !req:CANTRCV CANSM417 */
	/* !req:CANTRCV CANSM418 */
}

void CanSM_ConfirmPnAvailability( uint8 Transceiver )
{
	/* !req:PN CANSM546 */
	/* !req:PN CANSM420 */
	/* !req:PN CANSM421 */
}
#endif
