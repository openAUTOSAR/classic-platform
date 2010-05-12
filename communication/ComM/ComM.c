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



#include "ComM.h"
#if (COMM_DEV_ERROR_DETECT == STD_ON)
#include "Det.h"
#endif
#include "ComM_BusSm.h"
#include "ComM_Internal.h"

#include "CanSM.h"

static ComM_InternalType ComM_Internal = {
		.InitStatus = COMM_UNINIT,
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
	}

	for (int i = 0; i < COMM_USER_COUNT; ++i) {
		ComM_Internal.Users[i].RequestedMode = COMM_NO_COMMUNICATION;
	}

	ComM_Internal.InitStatus = COMM_INIT;

}

void ComM_DeInit(){
	COMM_VALIDATE_INIT(COMM_SERVICEID_DEINIT);

	ComM_Internal.InitStatus = COMM_UNINIT;

}

void ComM_GetVersionInfo( Std_VersionInfoType* versioninfo ){
	COMM_VALIDATE_INIT(COMM_SERVICEID_GETVERSIONINFO);
}


Std_ReturnType ComM_GetStatus( ComM_InitStatusType* Status ){
	COMM_VALIDATE_PARAMETER( (Status != NULL), COMM_SERVICEID_GETSTATUS, E_NOT_OK);

	*Status = ComM_Internal.InitStatus;
	return E_OK;
}

Std_ReturnType ComM_GetInhibitionStatus( NetworkHandleType Channel, ComM_InhibitionStatusType* Status ){
	COMM_VALIDATE_INIT(COMM_SERVICEID_GETINHIBITIONSTATUS, COMM_E_UNINIT);
	return E_OK;
}


Std_ReturnType ComM_RequestComMode( ComM_UserHandleType User, ComM_ModeType ComMode ){
	COMM_VALIDATE_INIT(COMM_SERVICEID_REQUESTCOMMODE, COMM_E_UNINIT);
	COMM_VALIDATE_USER(User, COMM_SERVICEID_REQUESTCOMMODE, E_NOT_OK);

	ComM_Internal_UserType* UserInternal = &ComM_Internal.Users[User];

	UserInternal->RequestedMode = ComMode;

	return ComM_Internal_DelegateRequestComMode(User, ComMode);
}

static Std_ReturnType ComM_Internal_DelegateRequestComMode( ComM_UserHandleType User, ComM_ModeType ComMode ){
	const ComM_UserType* UserConfig = &ComM_Config->Users[User];

	Std_ReturnType requestStatus = E_OK;

	/* Go through users channels. Relay to SMs. Collect overall success status */
	for (int i = 0; i < UserConfig->ChannelCount; ++i) {
		const ComM_ChannelType* Channel = UserConfig->ChannelList[i];
		Std_ReturnType status = E_OK;
		switch (Channel->BusType) {
			case COMM_BUS_TYPE_CAN:
				status = CanSM_RequestComMode(Channel->BusSMNetworkHandle, ComMode);
				break;
			default:
				status = E_NOT_OK;
				break;
		}
		if (status > requestStatus) {
			requestStatus = status;
		}
	}

	return requestStatus;
}

Std_ReturnType ComM_GetMaxComMode( ComM_UserHandleType User, ComM_ModeType* ComMode ){
	COMM_VALIDATE_INIT(COMM_SERVICEID_GETMAXCOMMODE, COMM_E_UNINIT);
	COMM_VALIDATE_USER(User, COMM_SERVICEID_GETMAXCOMMODE, E_NOT_OK);
	return E_OK;
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

	return ComM_Internal_DelegateGetCurrentComMode(User, ComMode);
}

static Std_ReturnType ComM_Internal_DelegateGetCurrentComMode( ComM_UserHandleType User, ComM_ModeType* ComMode ){
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
			/** @req ComM176 */
			if (mode < requestMode) {
				requestMode = mode;
			}
		} else {
			return status;
		}
	}

	*ComMode = requestMode;

	return E_OK;
}


Std_ReturnType ComM_PreventWakeUp( NetworkHandleType Channel, boolean Status ){
	COMM_VALIDATE_INIT(COMM_SERVICEID_PREVENTWAKEUP, COMM_E_UNINIT);
	COMM_VALIDATE_CHANNEL(Channel, COMM_SERVICEID_PREVENTWAKEUP, E_NOT_OK);
	return E_OK;
}

Std_ReturnType ComM_LimitChannelToNoComMode( NetworkHandleType Channel, boolean Status ){
	COMM_VALIDATE_INIT(COMM_SERVICEID_LIMITCHANNELTONOCOMMODE, COMM_E_UNINIT);
	COMM_VALIDATE_CHANNEL(Channel, COMM_SERVICEID_LIMITCHANNELTONOCOMMODE, E_NOT_OK);
	return E_OK;
}

Std_ReturnType ComM_LimitECUToNoComMode( boolean Status ){
	COMM_VALIDATE_INIT(COMM_SERVICEID_LIMITECUTONOCOMMODE, COMM_E_UNINIT);
	return E_OK;
}

Std_ReturnType ComM_ReadInhibitCounter( uint16* CounterValue ){
	COMM_VALIDATE_INIT(COMM_SERVICEID_READINHIBITCOUNTER, COMM_E_UNINIT);
	return E_OK;
}

Std_ReturnType ComM_ResetInhibitCounter(){
	COMM_VALIDATE_INIT(COMM_SERVICEID_RESETINHIBITCOUNTER, COMM_E_UNINIT);
	return E_OK;
}

Std_ReturnType ComM_SetECUGroupClassification( ComM_InhibitionStatusType Status ){
	COMM_VALIDATE_INIT(COMM_SERVICEID_SETECUGROUPCLASSIFICATION, COMM_E_UNINIT);
	return E_OK;
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
