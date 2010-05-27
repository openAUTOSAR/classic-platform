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


#include <string.h>
#include "ComM.h"
#if (COMM_DEV_ERROR_DETECT == STD_ON)
#include "Det.h"
#endif
#include "ComM_BusSm.h"
#include "ComM_Internal.h"
#include "CanSM.h"

static ComM_InternalType ComM_Internal = {
		.InitStatus = COMM_UNINIT,
		.InhibitCounter = 0,
#if (COMM_NO_COM == STD_ON)
		.NoCommunication = TRUE,
#else
		.NoCommunication = FALSE,
#endif
};

static ComM_ConfigType * ComM_Config;


void ComM_Init( ComM_ConfigType * Config ){
	COMM_VALIDATE_PARAMETER( (Config != NULL), COMM_SERVICEID_INIT);
	COMM_VALIDATE_PARAMETER( (Config->Channels != NULL), COMM_SERVICEID_INIT);
	COMM_VALIDATE_PARAMETER( (Config->Users != NULL), COMM_SERVICEID_INIT);

	ComM_Config = Config;

	for (int i = 0; i < COMM_CHANNEL_COUNT; ++i) {
		/** @req ComM485 */
		ComM_Internal.Channels[i].Mode = COMM_NO_COMMUNICATION;
		ComM_Internal.Channels[i].SubMode = COMM_SUBMODE_NONE;
		ComM_Internal.Channels[i].UserRequestMask = 0;
		ComM_Internal.Channels[i].InhibitionStatus = COMM_INHIBITION_STATUS_NONE;
	}

	for (int i = 0; i < COMM_USER_COUNT; ++i) {
		ComM_Internal.Users[i].RequestedMode = COMM_NO_COMMUNICATION;
	}

	ComM_Internal.InhibitCounter = 0;
	ComM_Internal.InitStatus = COMM_INIT;
}

void ComM_DeInit(){
	COMM_VALIDATE_INIT(COMM_SERVICEID_DEINIT);

	ComM_Internal.InitStatus = COMM_UNINIT;
}

#if (COMM_VERSION_INFO_API == STD_ON)
static Std_VersionInfoType ComM_Internal_VersionInfo = {
  .vendorID   = (uint16)1,
  .moduleID   = (uint16)1,
  .instanceID = (uint8)1,
  .sw_major_version = (uint8)COMM_SW_MAJOR_VERSION,
  .sw_minor_version = (uint8)COMM_SW_MINOR_VERSION,
  .sw_patch_version = (uint8)COMM_SW_PATCH_VERSION,
  .ar_major_version = (uint8)COMM_AR_MAJOR_VERSION,
  .ar_minor_version = (uint8)COMM_AR_MINOR_VERSION,
  .ar_patch_version = (uint8)COMM_AR_PATCH_VERSION,
};

void ComM_GetVersionInfo( Std_VersionInfoType* VersionInfo ){
	COMM_VALIDATE_INIT(COMM_SERVICEID_GETVERSIONINFO);
	memcpy(VersionInfo, &ComM_Internal_VersionInfo, sizeof(Std_VersionInfoType));
}
#endif


Std_ReturnType ComM_GetStatus( ComM_InitStatusType* Status ){
	COMM_VALIDATE_PARAMETER( (Status != NULL), COMM_SERVICEID_GETSTATUS, E_NOT_OK);

	*Status = ComM_Internal.InitStatus;
	return E_OK;
}

Std_ReturnType ComM_GetInhibitionStatus( NetworkHandleType Channel, ComM_InhibitionStatusType* Status ){
	COMM_VALIDATE_INIT(COMM_SERVICEID_GETINHIBITIONSTATUS, COMM_E_UNINIT);

	ComM_Internal_ChannelType* ChannelInternal = &ComM_Internal.Channels[Channel];
	*Status = ChannelInternal->InhibitionStatus;
	return E_OK;
}


Std_ReturnType ComM_RequestComMode( ComM_UserHandleType User, ComM_ModeType ComMode ){
	COMM_VALIDATE_INIT(COMM_SERVICEID_REQUESTCOMMODE, COMM_E_UNINIT);
	COMM_VALIDATE_USER(User, COMM_SERVICEID_REQUESTCOMMODE, E_NOT_OK);

	return ComM_Internal_RequestComMode(User, ComMode);
}

static Std_ReturnType ComM_Internal_RequestComMode(
				ComM_UserHandleType User, ComM_ModeType ComMode ){

	const ComM_UserType* UserConfig = &ComM_Config->Users[User];
	ComM_Internal_UserType* UserInternal = &ComM_Internal.Users[User];

	UserInternal->RequestedMode = ComMode;
	uint32 userMask = 1 << User;

	Std_ReturnType requestStatus = E_OK;

	/* Go through users channels. Relay to SMs. Collect overall success status */
	for (int i = 0; i < UserConfig->ChannelCount; ++i) {
		const ComM_ChannelType* Channel = UserConfig->ChannelList[i];
		ComM_Internal_ChannelType* ChannelInternal = &ComM_Internal.Channels[Channel->Number];

		// Put user request into mask
		if (ComMode == COMM_NO_COMMUNICATION) {
			ChannelInternal->UserRequestMask &= ~(userMask);
		} else if (ComMode == COMM_FULL_COMMUNICATION) {
			ChannelInternal->UserRequestMask |= userMask;
		}

		// take request -> new state
		Std_ReturnType status = ComM_Internal_UpdateChannelState(Channel, TRUE);
		if (status > requestStatus) requestStatus = status;

	}

	return requestStatus;
}

Std_ReturnType ComM_GetMaxComMode( ComM_UserHandleType User, ComM_ModeType* ComMode ){
	COMM_VALIDATE_INIT(COMM_SERVICEID_GETMAXCOMMODE, COMM_E_UNINIT);
	COMM_VALIDATE_USER(User, COMM_SERVICEID_GETMAXCOMMODE, E_NOT_OK);
	// Not implemented
	return E_NOT_OK;
}

Std_ReturnType ComM_GetRequestedComMode( ComM_UserHandleType User, ComM_ModeType* ComMode ){
	COMM_VALIDATE_INIT(COMM_SERVICEID_GETREQUESTEDCOMMODE, COMM_E_UNINIT);
	COMM_VALIDATE_USER(User, COMM_SERVICEID_GETREQUESTEDCOMMODE, E_NOT_OK);

	ComM_Internal_UserType* UserInternal = &ComM_Internal.Users[User];
	*ComMode = UserInternal->RequestedMode;
	return E_OK;
}

Std_ReturnType ComM_GetCurrentComMode( ComM_UserHandleType User, ComM_ModeType* ComMode ){
	COMM_VALIDATE_INIT(COMM_SERVICEID_GETCURRENTCOMMODE, COMM_E_UNINIT);
	COMM_VALIDATE_USER(User, COMM_SERVICEID_GETCURRENTCOMMODE, E_NOT_OK);

	return ComM_Internal_PropagateGetCurrentComMode(User, ComMode);
}


Std_ReturnType ComM_PreventWakeUp( NetworkHandleType Channel, boolean Status ){
	COMM_VALIDATE_INIT(COMM_SERVICEID_PREVENTWAKEUP, COMM_E_UNINIT);
	COMM_VALIDATE_CHANNEL(Channel, COMM_SERVICEID_PREVENTWAKEUP, E_NOT_OK);
#if (COMM_MODE_LIMITATION_ENABLED == STD_ON)
	ComM_ChannelType* ChannelConf = &ComM_Config->Channels[Channel];
	ComM_Internal_ChannelType* ChannelInternal = &ComM_Internal.Channels[Channel];
	if (Status) {
		ChannelInternal->InhibitionStatus |=  (COMM_INHIBITION_STATUS_WAKE_UP);
	} else {
		ChannelInternal->InhibitionStatus &= ~(COMM_INHIBITION_STATUS_WAKE_UP);
	}
	return E_OK;
#else
	return E_NOT_OK;
#endif
}

Std_ReturnType ComM_LimitChannelToNoComMode( NetworkHandleType Channel, boolean Status ){
	COMM_VALIDATE_INIT(COMM_SERVICEID_LIMITCHANNELTONOCOMMODE, COMM_E_UNINIT);
	COMM_VALIDATE_CHANNEL(Channel, COMM_SERVICEID_LIMITCHANNELTONOCOMMODE, E_NOT_OK);
#if (COMM_MODE_LIMITATION_ENABLED == STD_ON)
	ComM_ChannelType* ChannelConf = &ComM_Config->Channels[Channel];
	ComM_Internal_ChannelType* ChannelInternal = &ComM_Internal.Channels[Channel];
	if (Status) {
		ChannelInternal->InhibitionStatus |=  (COMM_INHIBITION_STATUS_NO_COMMUNICATION);
	} else {
		ChannelInternal->InhibitionStatus &= ~(COMM_INHIBITION_STATUS_NO_COMMUNICATION);
	}
	return ComM_Internal_UpdateChannelState(ChannelConf, FALSE);
#else
	return E_NOT_OK;
#endif
}

Std_ReturnType ComM_LimitECUToNoComMode( boolean Status ){
	COMM_VALIDATE_INIT(COMM_SERVICEID_LIMITECUTONOCOMMODE, COMM_E_UNINIT);
#if (COMM_MODE_LIMITATION_ENABLED == STD_ON)
	ComM_Internal.NoCommunication = Status;
	int Channel;
	for (Channel = 0; Channel < COMM_CHANNEL_COUNT; Channel++) {
		ComM_ChannelType* ChannelConf = &ComM_Config->Channels[Channel];
		ComM_Internal_UpdateChannelState(ChannelConf, FALSE);
	}
	return E_OK;
#else
	return E_NOT_OK;
#endif
}


Std_ReturnType ComM_ReadInhibitCounter( uint16* CounterValue ){
	COMM_VALIDATE_INIT(COMM_SERVICEID_READINHIBITCOUNTER, COMM_E_UNINIT);
#if (COMM_MODE_LIMITATION_ENABLED == STD_ON)
	*CounterValue = ComM_Internal.InhibitCounter;
	return E_OK;
#else
	return E_NOT_OK;
#endif
}

Std_ReturnType ComM_ResetInhibitCounter(){
	COMM_VALIDATE_INIT(COMM_SERVICEID_RESETINHIBITCOUNTER, COMM_E_UNINIT);
#if (COMM_MODE_LIMITATION_ENABLED == STD_ON)
	ComM_Internal.InhibitCounter = 0;
	return E_OK;
#else
	return E_NOT_OK;
#endif
}

Std_ReturnType ComM_SetECUGroupClassification( ComM_InhibitionStatusType Status ){
	COMM_VALIDATE_INIT(COMM_SERVICEID_SETECUGROUPCLASSIFICATION, COMM_E_UNINIT);
	// Not implemented
	return E_NOT_OK;
}


// Network Management Interface Callbacks
// --------------------------------------

void ComM_Nm_NetworkStartIndication( NetworkHandleType Channel ){
	COMM_VALIDATE_INIT(COMM_SERVICEID_NM_NETWORKSTARTINDICATION);
	COMM_VALIDATE_CHANNEL(Channel, COMM_SERVICEID_NM_NETWORKSTARTINDICATION);
}

void ComM_Nm_NetworkMode( NetworkHandleType Channel ){
	COMM_VALIDATE_INIT(COMM_SERVICEID_NM_NETWORKMODE);
	COMM_VALIDATE_CHANNEL(Channel, COMM_SERVICEID_NM_NETWORKMODE);
}

void ComM_Nm_PrepareBusSleepMode( NetworkHandleType Channel ){
	COMM_VALIDATE_INIT(COMM_SERVICEID_NM_PREPAREBUSSLEEPMODE);
	COMM_VALIDATE_CHANNEL(Channel, COMM_SERVICEID_NM_PREPAREBUSSLEEPMODE);
}

void ComM_Nm_BusSleepMode( NetworkHandleType Channel ){
	COMM_VALIDATE_INIT(COMM_SERVICEID_NM_BUSSLEEPMODE);
	COMM_VALIDATE_CHANNEL(Channel, COMM_SERVICEID_NM_BUSSLEEPMODE);
}

void ComM_Nm_RestartIndication( NetworkHandleType Channel ){
	COMM_VALIDATE_INIT(COMM_SERVICEID_NM_RESTARTINDICATION);
	COMM_VALIDATE_CHANNEL(Channel, COMM_SERVICEID_NM_RESTARTINDICATION);
}


// ECU State Manager Callbacks
// ---------------------------

void ComM_EcuM_RunModeIndication( NetworkHandleType Channel ){
	COMM_VALIDATE_INIT(COMM_SERVICEID_ECUM_RUNMODEINDICATION);
	COMM_VALIDATE_CHANNEL(Channel, COMM_SERVICEID_ECUM_RUNMODEINDICATION);
}

void ComM_EcuM_WakeUpIndication( NetworkHandleType Channel ){
	COMM_VALIDATE_INIT(COMM_SERVICEID_ECUM_WAKEUPINDICATION);
	COMM_VALIDATE_CHANNEL(Channel, COMM_SERVICEID_ECUM_WAKEUPINDICATION);
}


// Diagnostic Communication Manager Callbacks
// ------------------------------------------

void ComM_DCM_ActiveDiagnostic(){
	COMM_VALIDATE_INIT(COMM_SERVICEID_DCM_ACTIVEDIAGNOSTIC);
}

void ComM_DCM_InactiveDiagnostic(){
	COMM_VALIDATE_INIT(COMM_SERVICEID_DCM_INACTIVEDIAGNOSTIC);
}


// Bus State Manager Callbacks
// ---------------------------

void ComM_BusSM_ModeIndication( NetworkHandleType Channel, ComM_ModeType ComMode ){
	COMM_VALIDATE_INIT(COMM_SERVICEID_BUSSM_MODEINDICATION);
	COMM_VALIDATE_CHANNEL(Channel, COMM_SERVICEID_BUSSM_MODEINDICATION);
}


// ----------------------------------------------------------------------------
// Internal functions
// ----------------------------------------------------------------------------


static Std_ReturnType ComM_Internal_PropagateGetCurrentComMode( ComM_UserHandleType User, ComM_ModeType* ComMode ){
	const ComM_UserType* UserConfig = &ComM_Config->Users[User];

	ComM_ModeType requestMode = COMM_FULL_COMMUNICATION;
	/* Go through users channels. Relay to SMs. Collect overall mode and success status */
	for (int i = 0; i < UserConfig->ChannelCount; ++i) {
		const ComM_ChannelType* Channel = UserConfig->ChannelList[i];
		Std_ReturnType status = E_OK;
		ComM_ModeType mode;
		switch (Channel->BusType) {
			case COMM_BUS_TYPE_CAN:
				status = CanSM_GetCurrentComMode(Channel->BusSMNetworkHandle, &mode);
				break;
			default:
				status = E_NOT_OK;
				break;
		}
		if (status == E_OK) {
			if (mode < requestMode) {	/** @req ComM176 */
				requestMode = mode;
			}
		} else {
			return status;
		}
	}
	*ComMode = requestMode;
	return E_OK;
}

static Std_ReturnType ComM_Internal_PropagateComMode( const ComM_ChannelType* Channel ){
	ComM_Internal_ChannelType* ChannelInternal = &ComM_Internal.Channels[Channel->Number];
	ComM_ModeType ComMode = ChannelInternal->Mode;

	switch (Channel->BusType) {
#if defined(USE_CANSM)
		case COMM_BUS_TYPE_CAN:
			return CanSM_RequestComMode(Channel->BusSMNetworkHandle, ComMode);
#endif
#if defined(USE_LINSM)
		case COMM_BUS_TYPE_LIN:
			return LinSM_RequestComMode(Channel->BusSMNetworkHandle, ComMode);
#endif
		default:
			return E_NOT_OK;
	}
}

/* Processes all requests etc. and makes state machine transitions accordingly */
static Std_ReturnType ComM_Internal_UpdateChannelState( const ComM_ChannelType* Channel, boolean isRequest ) {
	ComM_Internal_ChannelType* ChannelInternal = &ComM_Internal.Channels[Channel->Number];

	switch (ChannelInternal->Mode) {
		case COMM_NO_COMMUNICATION:
			if ((ChannelInternal->InhibitionStatus & COMM_INHIBITION_STATUS_NO_COMMUNICATION) ||
				(ChannelInternal->InhibitionStatus & COMM_INHIBITION_STATUS_WAKE_UP) ||
				(ComM_Internal.NoCommunication == TRUE)) {
				// Stay in NO
				if (isRequest) {
					ComM_Internal.InhibitCounter++;
				}
			} else {
				if (ChannelInternal->UserRequestMask != 0) {
					// NO -> FULL
					ChannelInternal->Mode = COMM_FULL_COMMUNICATION;
					ChannelInternal->SubMode = COMM_SUBMODE_NETWORK_REQUESTED;
					return ComM_Internal_PropagateComMode(Channel);
				} else {
					// Stay in NO
				}
			}
			break;
		case COMM_SILENT_COMMUNICATION:
			if ((ChannelInternal->InhibitionStatus & COMM_INHIBITION_STATUS_NO_COMMUNICATION) ||
				(ComM_Internal.NoCommunication == TRUE)) {
				// Stay in SILENT
				if (isRequest) {
					ComM_Internal.InhibitCounter++;
				}
			} else {
				if (ChannelInternal->UserRequestMask != 0) {
					// SILENT -> FULL/NETWORK_REQUESTED
					ChannelInternal->Mode = COMM_FULL_COMMUNICATION;
					ChannelInternal->SubMode = COMM_SUBMODE_NETWORK_REQUESTED;
					return ComM_Internal_PropagateComMode(Channel);
				} else {
					// Stay in SILENT
				}
			}
			break;
		case COMM_FULL_COMMUNICATION:
			if ((ChannelInternal->InhibitionStatus & COMM_INHIBITION_STATUS_NO_COMMUNICATION) ||
				(ComM_Internal.NoCommunication == TRUE)) {
				// FULL/* -> FULL/READY_SLEEP
				ChannelInternal->SubMode = COMM_SUBMODE_READY_SLEEP;
				if (isRequest) {
					ComM_Internal.InhibitCounter++;
				}
			} else {
				if (ChannelInternal->UserRequestMask == 0) {
					// FULL/* -> FULL/READY_SLEEP
					ChannelInternal->SubMode = COMM_SUBMODE_READY_SLEEP;
				} else {
					// FULL/* -> FULL/NETWORK_REQUESTED
					ChannelInternal->SubMode = COMM_SUBMODE_NETWORK_REQUESTED;
				}
			}
			break;
		default:
			break;
	}

	return E_OK;
}
