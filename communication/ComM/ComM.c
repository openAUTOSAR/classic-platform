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

//lint -emacro(904,COMM_VALIDATE_INIT,COMM_VALIDATE_CHANNEL,COMM_VALIDATE_USER,COMM_VALIDATE_PARAMETER) //904 PC-Lint exception to MISRA 14.7 (validate macros).

/* Globally fulfilled requirements */
/** @req COMM43 */
/** @req COMM38 */
/** @req COMM463.partially */
/** @req COMM686 */
/** @req COMM51 */
/** @req COMM191 */
/** @req COMM301 */
/** @req COMM488 */
/** @req COMM599 */
/** @req COMM509 */
/** @req COMM269 */
/** @req COMM458 */
/** @req COMM640 */
/** @req COMM459 */
/** @req COMM462 */
/** @req COMM457.bswbuilder */
/** @req COMM419.bswbuilder */
/** @req COMM460.bswbuilder */
/** @req COMM549.bswbuilder */
/** @req COMM464 */

/* EcuM interface & wakeup */
/** @req COMM239 */
/** @req COMM660 */
/** @req COMM275 */
/** @req COMM814 */
/** @req COMM815 */
/** @req COMM406 */
/** @req COMM647 */
/** @req COMM648 */
/** @req COMM651 */
/** @req COMM316 */
/** @req COMM33 */
/** @req COMM317 */
/** @req COMM453 */
/** @req COMM454 */
/** @req COMM455 */
/** @req COMM344 */


#include <string.h>
#include "ComM.h"
#include "ComM_Dcm.h"
#include "ComM_EcuM.h"
#if (COMM_DEV_ERROR_DETECT == STD_ON)
#if defined(USE_DET)
#include "Det.h"
#endif
#endif

/** @req COMM507  @req COMM508 */
#if defined(USE_DEM)
#include "Dem.h"
#endif

#include "ComM_BusSm.h"
#include "ComM_Internal.h"

/** @req COMM506  @req COMM353 */
#if defined(USE_CANSM)
#include "CanSM.h"
#endif

#if defined(USE_LINSM)
#include "LinSM.h"
#endif

/** @req COMM347 */
#if defined(USE_NM) || defined(COMM_TESTS)
#include "Nm.h"
#endif

#if defined(USE_ECUM)
#include "EcuM.h"
#endif

static ComM_InternalType ComM_Internal = {
		.InitStatus = COMM_UNINIT,
		.InhibitCounter = 0,
#if (COMM_NO_COM == STD_ON)
		.NoCommunication = TRUE,
#else
		.NoCommunication = FALSE,
#endif
};

static const ComM_ConfigType * ComM_Config;


void ComM_Init(const ComM_ConfigType * Config ){
	COMM_VALIDATE_PARAMETER_NORV( (Config != NULL), COMM_SERVICEID_INIT);
	COMM_VALIDATE_PARAMETER_NORV( (Config->Channels != NULL), COMM_SERVICEID_INIT);
	COMM_VALIDATE_PARAMETER_NORV( (Config->Users != NULL), COMM_SERVICEID_INIT);

	ComM_Config = Config;

	for (uint8 i = 0; i < COMM_CHANNEL_COUNT; ++i) {
		ComM_Internal.Channels[i].Mode = COMM_NO_COMMUNICATION;  /**< @req COMM485 */
		ComM_Internal.Channels[i].SubMode = COMM_SUBMODE_NONE;
		ComM_Internal.Channels[i].UserRequestMask = 0;
		ComM_Internal.Channels[i].InhibitionStatus = COMM_INHIBITION_STATUS_NONE;
		ComM_Internal.Channels[i].NmIndicationMask = COMM_NM_INDICATION_NONE;
		ComM_Internal.Channels[i].RunModeIndication = FALSE;
		ComM_Internal.Channels[i].WakeUp = FALSE;
	}

	for (uint8 i = 0; i < COMM_USER_COUNT; ++i) {
		ComM_Internal.Users[i].RequestedMode = COMM_NO_COMMUNICATION;
	}

	ComM_Internal.InhibitCounter = 0;
	ComM_Internal.InitStatus = COMM_INIT;
	/** @req COMM313 */
}

void ComM_DeInit(){
	COMM_VALIDATE_INIT_NORV(COMM_SERVICEID_DEINIT);

	ComM_Internal.InitStatus = COMM_UNINIT;
}

Std_ReturnType ComM_GetStatus( ComM_InitStatusType* Status ){
	COMM_VALIDATE_PARAMETER( (Status != NULL), COMM_SERVICEID_GETSTATUS);

	*Status = ComM_Internal.InitStatus;
	return E_OK;
}

Std_ReturnType ComM_GetInhibitionStatus( NetworkHandleType Channel, ComM_InhibitionStatusType* Status ){
	COMM_VALIDATE_INIT(COMM_SERVICEID_GETINHIBITIONSTATUS);

	ComM_Internal_ChannelType* ChannelInternal = &ComM_Internal.Channels[Channel];
	*Status = ChannelInternal->InhibitionStatus;
	return E_OK;
}


Std_ReturnType ComM_RequestComMode( ComM_UserHandleType User, ComM_ModeType ComMode ){
	COMM_VALIDATE_INIT(COMM_SERVICEID_REQUESTCOMMODE);
	COMM_VALIDATE_USER(User, COMM_SERVICEID_REQUESTCOMMODE);
	/** @req COMM151 */
	COMM_VALIDATE_PARAMETER((ComMode != COMM_SILENT_COMMUNICATION), COMM_SERVICEID_REQUESTCOMMODE);

	return ComM_Internal_RequestComMode(User, ComMode);
}

static Std_ReturnType ComM_Internal_RequestComMode(
				ComM_UserHandleType User, ComM_ModeType ComMode ){

	const ComM_UserType* UserConfig = &ComM_Config->Users[User];
	ComM_Internal_UserType* UserInternal = &ComM_Internal.Users[User];

	UserInternal->RequestedMode = ComMode;  /**< @req COMM471  @req COMM500  @req COMM92 */
	uint32 userMask = (1LU << User);

	Std_ReturnType requestStatus = E_OK;

	/* Go through users channels. Relay to SMs. Collect overall success status */
	for (uint8 i = 0; i < UserConfig->ChannelCount; ++i) {
		const ComM_ChannelType* Channel = UserConfig->ChannelList[i];
		ComM_Internal_ChannelType* ChannelInternal = &ComM_Internal.Channels[Channel->Number];

		/** @req COMM784.1  @req COMM304  @req COMM625 */
		// Put user request into mask
		if (ComMode == COMM_NO_COMMUNICATION) {
			ChannelInternal->UserRequestMask &= ~(userMask);
#if defined(USE_ECUM) || defined(COMM_TESTS)
			ComM_Internal_ReleaseRUN(Channel->Number);
#endif
		} else if (ComMode == COMM_FULL_COMMUNICATION) {
			ChannelInternal->UserRequestMask |= userMask;
#if defined(USE_ECUM) || defined(COMM_TESTS)
			ComM_Internal_RequestRUN(Channel->Number);
#endif
		} else {
			//Nothing to be done.
		}
	}

	return requestStatus;
}

Std_ReturnType ComM_GetMaxComMode( ComM_UserHandleType User, ComM_ModeType* ComMode ){
	COMM_VALIDATE_INIT(COMM_SERVICEID_GETMAXCOMMODE);
	COMM_VALIDATE_USER(User, COMM_SERVICEID_GETMAXCOMMODE);
	// Not implemented
	(void)ComMode;
	return E_NOT_OK;
}

/** @req COMM80 */
Std_ReturnType ComM_GetRequestedComMode( ComM_UserHandleType User, ComM_ModeType* ComMode ){
	COMM_VALIDATE_INIT(COMM_SERVICEID_GETREQUESTEDCOMMODE);
	COMM_VALIDATE_USER(User, COMM_SERVICEID_GETREQUESTEDCOMMODE);

	ComM_Internal_UserType* UserInternal = &ComM_Internal.Users[User];
	*ComMode = UserInternal->RequestedMode;
	return E_OK;
}

/** @req COMM84 */
Std_ReturnType ComM_GetCurrentComMode( ComM_UserHandleType User, ComM_ModeType* ComMode ){
	COMM_VALIDATE_INIT(COMM_SERVICEID_GETCURRENTCOMMODE);
	COMM_VALIDATE_USER(User, COMM_SERVICEID_GETCURRENTCOMMODE);

	return ComM_Internal_PropagateGetCurrentComMode(User, ComMode);
}

Std_ReturnType ComM_PreventWakeUp( NetworkHandleType Channel, boolean Status ){
	COMM_VALIDATE_INIT(COMM_SERVICEID_PREVENTWAKEUP);
	COMM_VALIDATE_CHANNEL(Channel, COMM_SERVICEID_PREVENTWAKEUP);
#if (COMM_MODE_LIMITATION_ENABLED == STD_ON)
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

/** @req COMM361  @req COMM105.1  @req COMM800 */
Std_ReturnType ComM_LimitChannelToNoComMode( NetworkHandleType Channel, boolean Status ){
	COMM_VALIDATE_INIT(COMM_SERVICEID_LIMITCHANNELTONOCOMMODE);
	COMM_VALIDATE_CHANNEL(Channel, COMM_SERVICEID_LIMITCHANNELTONOCOMMODE);
#if (COMM_MODE_LIMITATION_ENABLED == STD_ON)
	ComM_Internal_ChannelType* ChannelInternal = &ComM_Internal.Channels[Channel];
	if (Status) {
		ChannelInternal->InhibitionStatus |=  (COMM_INHIBITION_STATUS_NO_COMMUNICATION);
	} else {
		ChannelInternal->InhibitionStatus &= ~(COMM_INHIBITION_STATUS_NO_COMMUNICATION);
	}
	return E_OK;
#else
	return E_NOT_OK;
#endif
}

/** @req COMM105.2  @req COMM801.partially */
Std_ReturnType ComM_LimitECUToNoComMode( boolean Status ){
	COMM_VALIDATE_INIT(COMM_SERVICEID_LIMITECUTONOCOMMODE);
#if (COMM_MODE_LIMITATION_ENABLED == STD_ON)
	ComM_Internal.NoCommunication = Status;
	return E_OK;
#else
	return E_NOT_OK;
#endif
}

/** @req COMM143  @req COMM802 */
Std_ReturnType ComM_ReadInhibitCounter( uint16* CounterValue ){
	COMM_VALIDATE_INIT(COMM_SERVICEID_READINHIBITCOUNTER);
#if (COMM_MODE_LIMITATION_ENABLED == STD_ON)
	*CounterValue = ComM_Internal.InhibitCounter;
	return E_OK;
#else
	return E_NOT_OK;
#endif
}

/** @req COMM803 */
Std_ReturnType ComM_ResetInhibitCounter(){
	COMM_VALIDATE_INIT(COMM_SERVICEID_RESETINHIBITCOUNTER);
#if (COMM_MODE_LIMITATION_ENABLED == STD_ON)
	ComM_Internal.InhibitCounter = 0;
	return E_OK;
#else
	return E_NOT_OK;
#endif
}

Std_ReturnType ComM_SetECUGroupClassification( ComM_InhibitionStatusType Status ){
	COMM_VALIDATE_INIT(COMM_SERVICEID_SETECUGROUPCLASSIFICATION);
	// Not implemented
	(void)Status;
	return E_NOT_OK;
}


// Network Management Interface Callbacks
// --------------------------------------
/** @req COMM804 */
void ComM_Nm_NetworkStartIndication( NetworkHandleType Channel ){
	COMM_VALIDATE_INIT_NORV(COMM_SERVICEID_NM_NETWORKSTARTINDICATION);
	COMM_VALIDATE_CHANNEL_NORV(Channel, COMM_SERVICEID_NM_NETWORKSTARTINDICATION);
	ComM_Internal_ChannelType* ChannelInternal = &ComM_Internal.Channels[Channel];

	// Used to simulate Wake-up
	ChannelInternal->NmIndicationMask |= COMM_NM_INDICATION_RESTART;
}

/** @req COMM807 */
void ComM_Nm_NetworkMode( NetworkHandleType Channel ){
	COMM_VALIDATE_INIT_NORV(COMM_SERVICEID_NM_NETWORKMODE);
	COMM_VALIDATE_CHANNEL_NORV(Channel, COMM_SERVICEID_NM_NETWORKMODE);
	ComM_Internal_ChannelType* ChannelInternal = &ComM_Internal.Channels[Channel];

	ChannelInternal->NmIndicationMask |= COMM_NM_INDICATION_NETWORK_MODE;
}

/** @req COMM809 */
void ComM_Nm_PrepareBusSleepMode( NetworkHandleType Channel ){
	COMM_VALIDATE_INIT_NORV(COMM_SERVICEID_NM_PREPAREBUSSLEEPMODE);
	COMM_VALIDATE_CHANNEL_NORV(Channel, COMM_SERVICEID_NM_PREPAREBUSSLEEPMODE);
	ComM_Internal_ChannelType* ChannelInternal = &ComM_Internal.Channels[Channel];

	ChannelInternal->NmIndicationMask |= COMM_NM_INDICATION_PREPARE_BUS_SLEEP;
}

/** @req COMM811 */
void ComM_Nm_BusSleepMode( NetworkHandleType Channel ){
	COMM_VALIDATE_INIT_NORV(COMM_SERVICEID_NM_BUSSLEEPMODE);
	COMM_VALIDATE_CHANNEL_NORV(Channel, COMM_SERVICEID_NM_BUSSLEEPMODE);
	ComM_Internal_ChannelType* ChannelInternal = &ComM_Internal.Channels[Channel];

	ChannelInternal->NmIndicationMask |= COMM_NM_INDICATION_BUS_SLEEP;
}

/** @req COMM813 */
void ComM_Nm_RestartIndication( NetworkHandleType Channel ){
	COMM_VALIDATE_INIT_NORV(COMM_SERVICEID_NM_RESTARTINDICATION);
	COMM_VALIDATE_CHANNEL_NORV(Channel, COMM_SERVICEID_NM_RESTARTINDICATION);
	ComM_Internal_ChannelType* ChannelInternal = &ComM_Internal.Channels[Channel];

	ChannelInternal->NmIndicationMask |= COMM_NM_INDICATION_RESTART;
}

#if defined(USE_ECUM) || defined(COMM_TESTS)
static void ComM_Internal_ReleaseRUN(NetworkHandleType Channel)
{
	if(TRUE == EcuM_ComM_HasRequestedRUN(Channel))
	{
		/* Release run request */
		EcuM_ComM_ReleaseRUN(Channel);
	}
	/* Set internal RunModeIndication to false since no callback on release, only on RunIndication */
	ComM_Internal.Channels[Channel].RunModeIndication = FALSE;
}

static void ComM_Internal_RequestRUN(NetworkHandleType Channel)
{
	if(FALSE == EcuM_ComM_HasRequestedRUN(Channel))
	{
		/* Request run and wait for RunModeIndication */
		EcuM_ComM_RequestRUN(Channel);
	}
}
#endif
static boolean ComM_Internal_RunModeIndication( NetworkHandleType Channel )
{
	return ComM_Internal.Channels[Channel].RunModeIndication;
}

// ECU State Manager Callbacks
// ---------------------------

void ComM_EcuM_RunModeIndication( NetworkHandleType Channel ){
	COMM_VALIDATE_INIT_NORV(COMM_SERVICEID_ECUM_RUNMODEINDICATION);
	COMM_VALIDATE_CHANNEL_NORV(Channel, COMM_SERVICEID_ECUM_RUNMODEINDICATION);

	ComM_Internal.Channels[Channel].RunModeIndication = TRUE;
}

void ComM_EcuM_WakeUpIndication( NetworkHandleType Channel ){
	COMM_VALIDATE_INIT_NORV(COMM_SERVICEID_ECUM_WAKEUPINDICATION);
	COMM_VALIDATE_CHANNEL_NORV(Channel, COMM_SERVICEID_ECUM_WAKEUPINDICATION);

	/* Activate wake-up i.e. start RequestRun and go to full communication */
#if defined(USE_ECUM) || defined(COMM_TESTS)
	ComM_Internal_RequestRUN(Channel);
#endif
	ComM_Internal.Channels[Channel].WakeUp = TRUE;
}


// Diagnostic Communication Manager Callbacks
// ------------------------------------------

void ComM_DCM_ActiveDiagnostic(void){
	COMM_VALIDATE_INIT_NORV(COMM_SERVICEID_DCM_ACTIVEDIAGNOSTIC);

	/** !req ComM182 : The communication inhibition shall get temporarily inactive during an active diagnostic session */

	// TODO Activate channels,  we must have a DCM user with channels
}

void ComM_DCM_InactiveDiagnostic(void){
	COMM_VALIDATE_INIT_NORV(COMM_SERVICEID_DCM_INACTIVEDIAGNOSTIC);

	/** !req ComM182 : The communication inhibition shall get temporarily inactive during an active diagnostic session */

	// TODO DeActivate channels,  we must have a DCM user with channels
}


// Bus State Manager Callbacks
// ---------------------------

void ComM_BusSM_ModeIndication( NetworkHandleType Channel, ComM_ModeType ComMode ){
	COMM_VALIDATE_INIT_NORV(COMM_SERVICEID_BUSSM_MODEINDICATION);
	COMM_VALIDATE_CHANNEL_NORV(Channel, COMM_SERVICEID_BUSSM_MODEINDICATION);
	// Not implemented
	(void)ComMode;
}


// Scheduled main function
// -----------------------

// Prototype right here because this function should not be exposed
void ComM_MainFunction(NetworkHandleType Channel);

/** @req COMM429 */
void ComM_MainFunction(NetworkHandleType Channel) {
	const ComM_ChannelType* ChannelConf = &ComM_Config->Channels[Channel];
	ComM_Internal_ChannelType* ChannelInternal = &ComM_Internal.Channels[Channel];

	Std_ReturnType status = ComM_Internal_UpdateChannelState(ChannelConf, TRUE);

	if ((ChannelConf->NmVariant == COMM_NM_VARIANT_NONE) ||
		(ChannelConf->NmVariant == COMM_NM_VARIANT_LIGHT)) {
		status = ComM_Internal_TickFullComMinTime(ChannelConf, ChannelInternal);
	}
	if (ChannelConf->NmVariant == COMM_NM_VARIANT_LIGHT) {
		status = ComM_Internal_TickLightTime(ChannelConf, ChannelInternal);
	}

	if (status != E_OK) {
		// Report error?
	}
}

void ComM_MainFunction_All_Channels() {
	for (uint8 i = 0; i < COMM_CHANNEL_COUNT; i++) {
		ComM_MainFunction(i);
	}
}


// ----------------------------------------------------------------------------
// Internal functions
// ----------------------------------------------------------------------------

static inline Std_ReturnType ComM_Internal_TickFullComMinTime(const ComM_ChannelType* ChannelConf, ComM_Internal_ChannelType* ChannelInternal) {
	Std_ReturnType status = E_OK;
	if (ChannelInternal->Mode == COMM_FULL_COMMUNICATION) {
		if (ChannelConf->MainFunctionPeriod >= ChannelInternal->FullComMinDurationTimeLeft) {
			ChannelInternal->FullComMinDurationTimeLeft = 0;
			status = ComM_Internal_UpdateChannelState(ChannelConf, FALSE);
		} else {
			ChannelInternal->FullComMinDurationTimeLeft -= ChannelConf->MainFunctionPeriod;
		}
	}
	return status;
}

static inline boolean ComM_Internal_FullComMinTime_AllowsExit(const ComM_ChannelType* ChannelConf, const ComM_Internal_ChannelType* ChannelInternal) {
	boolean rv;
	/** @req COMM311 */
	if ((ChannelConf->NmVariant == COMM_NM_VARIANT_LIGHT) ||
		(ChannelConf->NmVariant == COMM_NM_VARIANT_NONE)){
		rv = (ChannelInternal->FullComMinDurationTimeLeft == 0);
	} else {
		rv = TRUE;
	}
	return rv;
}

static inline Std_ReturnType ComM_Internal_TickLightTime(const ComM_ChannelType* ChannelConf, ComM_Internal_ChannelType* ChannelInternal) {
	Std_ReturnType status = E_OK;
	if ((ChannelInternal->Mode == COMM_FULL_COMMUNICATION) &&
		(ChannelInternal->SubMode == COMM_SUBMODE_READY_SLEEP)) {
		if (ChannelConf->MainFunctionPeriod >= ChannelInternal->LightTimeoutTimeLeft) {
			ChannelInternal->LightTimeoutTimeLeft = 0;
			status = ComM_Internal_UpdateChannelState(ChannelConf, FALSE);
		} else {
			ChannelInternal->LightTimeoutTimeLeft -= ChannelConf->MainFunctionPeriod;
		}
	}
	return status;
}

/** @req COMM678.2 */
static Std_ReturnType ComM_Internal_PropagateGetCurrentComMode( ComM_UserHandleType User, ComM_ModeType* ComMode ){
	const ComM_UserType* UserConfig = &ComM_Config->Users[User];

	ComM_ModeType requestMode = COMM_FULL_COMMUNICATION;
	Std_ReturnType totalStatus = E_OK;
	/* Go through users channels. Relay to SMs. Collect overall mode and success status */
	for (uint8 i = 0; i < UserConfig->ChannelCount; ++i) {
		const ComM_ChannelType* Channel = UserConfig->ChannelList[i];
		Std_ReturnType status = E_OK;
		ComM_ModeType mode = COMM_FULL_COMMUNICATION;
		switch (Channel->BusType) {
#if defined(USE_CANSM) || defined(COMM_TESTS)
			case COMM_BUS_TYPE_CAN:
				status = CanSM_GetCurrentComMode(Channel->BusSMNetworkHandle, &mode);
				break;
#endif
#if defined(USE_LINSM)
		case COMM_BUS_TYPE_LIN:
			status = LinSM_GetCurrentComMode(Channel->BusSMNetworkHandle, &mode);
			break;
#endif
			default:
				status = E_NOT_OK;
				break;
		}
		if (status == E_OK) {
			if (mode < requestMode) {	/** @req ComM176 */
				requestMode = mode;
			}
		} else {
			totalStatus = status;
		}
	}
	*ComMode = requestMode;
	return totalStatus;
}

/** @req COMM281.partially  @req COMM70  @req COMM73  @req COMM71  @req COMM72
 *  @req COMM69  @req COMM402  @req COMM434  @req COMM678.1  @req COMM168  @req COMM676.partially */
static Std_ReturnType ComM_Internal_PropagateComMode( const ComM_ChannelType* ChannelConf ){
	ComM_Internal_ChannelType* ChannelInternal = &ComM_Internal.Channels[ChannelConf->Number];
	ComM_ModeType ComMode = ChannelInternal->Mode;

	Std_ReturnType busSMStatus = E_OK;
	switch (ChannelConf->BusType) {
#if defined(USE_CANSM) || defined(COMM_TESTS)
		case COMM_BUS_TYPE_CAN:
			busSMStatus = CanSM_RequestComMode(ChannelConf->BusSMNetworkHandle, ComMode);
			break;
#endif
#if defined(USE_LINSM)
		case COMM_BUS_TYPE_LIN:
			busSMStatus = LinSM_RequestComMode(ChannelConf->BusSMNetworkHandle, ComMode);
			break;
#endif
		default:
			busSMStatus = E_NOT_OK;
	}

	return busSMStatus;
}

/** @req COMM472  @req COMM602  @req COMM261 */
static Std_ReturnType ComM_Internal_NotifyNm( const ComM_ChannelType* ChannelConf){
	ComM_Internal_ChannelType* ChannelInternal = &ComM_Internal.Channels[ChannelConf->Number];

	Std_ReturnType globalStatus = E_OK;

	if ((ChannelConf->NmVariant == COMM_NM_VARIANT_FULL) ||
		(ChannelConf->NmVariant == COMM_NM_VARIANT_PASSIVE)) {
#if defined(USE_NM) || defined(COMM_TESTS)
		Nm_ReturnType nmStatus = NM_E_OK;
		if (ChannelInternal->Mode == COMM_FULL_COMMUNICATION) {
			if (ChannelInternal->SubMode == COMM_SUBMODE_NETWORK_REQUESTED) {
				nmStatus = Nm_NetworkRequest(ChannelConf->NmChannelHandle);  /**< @req COMM129.1 */
			} else if (ChannelInternal->SubMode == COMM_SUBMODE_READY_SLEEP) {
				nmStatus = Nm_NetworkRelease(ChannelConf->NmChannelHandle);  /**< @req COMM133.1 */
			} else {
				//Nothing to be done.
			}
		}
		if (nmStatus != NM_E_OK) {
			globalStatus = E_NOT_OK;
		}
#else
		globalStatus = E_NOT_OK;
#endif
	}

	return globalStatus;
}

/* Processes all requests etc. and makes state machine transitions accordingly */
static Std_ReturnType ComM_Internal_UpdateChannelState( const ComM_ChannelType* ChannelConf, boolean isRequest ) {
	ComM_Internal_ChannelType* ChannelInternal = &ComM_Internal.Channels[ChannelConf->Number];
	Std_ReturnType status = E_OK;
	switch (ChannelInternal->Mode) {
		case COMM_NO_COMMUNICATION:
			status = ComM_Internal_UpdateFromNoCom(ChannelConf, ChannelInternal, isRequest);
			break;
		case COMM_SILENT_COMMUNICATION:
			status = ComM_Internal_UpdateFromSilentCom(ChannelConf, ChannelInternal, isRequest);
			break;
		case COMM_FULL_COMMUNICATION:
			status = ComM_Internal_UpdateFromFullCom(ChannelConf, ChannelInternal, isRequest);
			break;
		default:
			status = E_NOT_OK;
			break;
	}
	return status;
}

static inline Std_ReturnType ComM_Internal_UpdateFromNoCom(const ComM_ChannelType* ChannelConf,
					ComM_Internal_ChannelType* ChannelInternal, boolean isRequest) {
	Std_ReturnType status = E_OK;
	if (ChannelInternal->NmIndicationMask & COMM_NM_INDICATION_RESTART) {  /**< @req COMM207.partial */
		// "restart" indication
		status = ComM_Internal_Enter_NetworkRequested(ChannelConf, ChannelInternal);  /**< @req COMM583 */
		ChannelInternal->NmIndicationMask &= ~(COMM_NM_INDICATION_RESTART);
	} else {
		if ((ChannelInternal->InhibitionStatus & COMM_INHIBITION_STATUS_NO_COMMUNICATION) ||
			(ChannelInternal->InhibitionStatus & COMM_INHIBITION_STATUS_WAKE_UP) ||
			(ComM_Internal.NoCommunication == TRUE)) {
			// Inhibition is active
			/** @req COMM302  @req COMM218  @req COMM219  @req COMM215.3  @req COMM216.3 */
			if (isRequest){
				ComM_Internal.InhibitCounter++;
			}
		} else {
			if ((ChannelInternal->UserRequestMask != 0) || (TRUE == ChannelInternal->WakeUp)) {
				if(TRUE == ComM_Internal_RunModeIndication(ChannelConf->Number))
				{
					// Channel is requested
					ChannelInternal->WakeUp = FALSE;
					status = ComM_Internal_Enter_NetworkRequested(ChannelConf, ChannelInternal);  /**< @req COMM784.2 */
				}
			} else {
				// Channel is not requested
			}
		}
	}
	return status;
}

static inline Std_ReturnType ComM_Internal_UpdateFromSilentCom(const ComM_ChannelType* ChannelConf,
					ComM_Internal_ChannelType* ChannelInternal,	boolean isRequest) {
	Std_ReturnType status = E_OK;
	if (ChannelInternal->NmIndicationMask & COMM_NM_INDICATION_RESTART) {  /**< @req COMM207.partial */
		// "restart" indication
		status = ComM_Internal_Enter_ReadySleep(ChannelConf, ChannelInternal);  /**< @req COMM296.1 */
		ChannelInternal->NmIndicationMask &= ~(COMM_NM_INDICATION_RESTART);
	} else if (ChannelInternal->NmIndicationMask & COMM_NM_INDICATION_BUS_SLEEP) {
		// "bus sleep" indication
		status = ComM_Internal_Enter_NoCom(ChannelConf, ChannelInternal);  /**< @req COMM295 */
		ChannelInternal->NmIndicationMask &= ~(COMM_NM_INDICATION_BUS_SLEEP);
	} else if (ChannelInternal->NmIndicationMask & COMM_NM_INDICATION_NETWORK_MODE) {
		// "network mode" indication
		status = ComM_Internal_Enter_ReadySleep(ChannelConf, ChannelInternal);  /**< @req COMM296.2 */
		ChannelInternal->NmIndicationMask &= ~(COMM_NM_INDICATION_NETWORK_MODE);
	} else {
		if ((ChannelInternal->InhibitionStatus & COMM_INHIBITION_STATUS_NO_COMMUNICATION) ||
			(ComM_Internal.NoCommunication == TRUE)) {
			// Inhibition is active
			/** @req COMM215.2  @req COMM216.2 */
			if (isRequest) {
				ComM_Internal.InhibitCounter++;
			}
		} else {
			if (ChannelInternal->UserRequestMask != 0) {
				// Channel is requested
				status = ComM_Internal_Enter_NetworkRequested(ChannelConf, ChannelInternal);  /**< @req COMM785 */
			} else {
				// Stay in SILENT
			}
		}
	}
	return status;
}

static inline Std_ReturnType ComM_Internal_UpdateFromFullCom(const ComM_ChannelType* ChannelConf,
					ComM_Internal_ChannelType* ChannelInternal, boolean isRequest) {
	Std_ReturnType status = E_OK;
	if (ChannelInternal->NmIndicationMask & COMM_NM_INDICATION_BUS_SLEEP) {
		// "bus sleep" indication
		status = ComM_Internal_Enter_NoCom(ChannelConf, ChannelInternal);  /**< @req COMM637 */
		ChannelInternal->NmIndicationMask &= ~(COMM_NM_INDICATION_BUS_SLEEP);
	} else if ((ChannelInternal->NmIndicationMask & COMM_NM_INDICATION_PREPARE_BUS_SLEEP) &&
				(ChannelInternal->SubMode == COMM_SUBMODE_READY_SLEEP)) {
		// "prepare bus sleep" indication
		status = ComM_Internal_Enter_SilentCom(ChannelConf, ChannelInternal);  /**< @req COMM299 */
		ChannelInternal->NmIndicationMask &= ~(COMM_NM_INDICATION_PREPARE_BUS_SLEEP);
	} else {
		if ((ChannelInternal->InhibitionStatus & COMM_INHIBITION_STATUS_NO_COMMUNICATION) ||
			(ComM_Internal.NoCommunication == TRUE)) {
			// Inhibition is active
			if (ComM_Internal_FullComMinTime_AllowsExit(ChannelConf, ChannelInternal)) {  /**< @req COMM205.1 */
				if (ChannelInternal->SubMode == COMM_SUBMODE_READY_SLEEP) {
					if ((ChannelConf->NmVariant == COMM_NM_VARIANT_LIGHT) &&
						(ChannelInternal->LightTimeoutTimeLeft == 0)) {
						status = ComM_Internal_Enter_NoCom(ChannelConf, ChannelInternal);  /**< @req COMM610.1 */
					}
				} else {
					/** @req COMM478.seeAlsoCOMM52  @req COMM303  @req COMM215.1  @req COMM216.1 */
					status = ComM_Internal_Enter_ReadySleep(ChannelConf, ChannelInternal);
				}
			}
			if (isRequest){
				ComM_Internal.InhibitCounter++;
			}
		} else {
			if (ChannelInternal->UserRequestMask == 0) {
				// Channel no longer requested
				if (ComM_Internal_FullComMinTime_AllowsExit(ChannelConf, ChannelInternal)) {  /**< @req COMM205.1 */
					if (ChannelInternal->SubMode == COMM_SUBMODE_READY_SLEEP) {
						if ((ChannelConf->NmVariant == COMM_NM_VARIANT_LIGHT) &&
							(ChannelInternal->LightTimeoutTimeLeft == 0)) {
							status = ComM_Internal_Enter_NoCom(ChannelConf, ChannelInternal);  /**< @req COMM610.2 */
						}
					} else {
						status = ComM_Internal_Enter_ReadySleep(ChannelConf, ChannelInternal);
					}
				}
			} else {
				// Channel is requested
				if (ChannelInternal->SubMode != COMM_SUBMODE_NETWORK_REQUESTED) {
					status = ComM_Internal_Enter_NetworkRequested(ChannelConf, ChannelInternal);  /**< @req COMM479 */
				}
			}
		}
	}
	return status;
}

static inline Std_ReturnType ComM_Internal_Enter_NoCom(const ComM_ChannelType* ChannelConf,
													ComM_Internal_ChannelType* ChannelInternal) {
	ChannelInternal->Mode = COMM_NO_COMMUNICATION;
	return ComM_Internal_PropagateComMode(ChannelConf);
}

static inline Std_ReturnType ComM_Internal_Enter_SilentCom(const ComM_ChannelType* ChannelConf,
														ComM_Internal_ChannelType* ChannelInternal) {
	ChannelInternal->Mode = COMM_SILENT_COMMUNICATION;
	return ComM_Internal_PropagateComMode(ChannelConf);
}

static inline Std_ReturnType ComM_Internal_Enter_NetworkRequested(const ComM_ChannelType* ChannelConf,
																ComM_Internal_ChannelType* ChannelInternal) {
	boolean propagateToBusSM = (ChannelInternal->Mode != COMM_FULL_COMMUNICATION);
	ChannelInternal->FullComMinDurationTimeLeft = COMM_T_MIN_FULL_COM_MODE_DURATION;
	ChannelInternal->Mode = COMM_FULL_COMMUNICATION;
	ChannelInternal->SubMode = COMM_SUBMODE_NETWORK_REQUESTED;

	Std_ReturnType status, globalStatus = E_OK;
	status = ComM_Internal_NotifyNm(ChannelConf);  /**< @req COMM129.2 */
	if (status > globalStatus){
		globalStatus = status;
	}
	if (propagateToBusSM) {
		status = ComM_Internal_PropagateComMode(ChannelConf);
		if (status > globalStatus){
			globalStatus = status;
		}
	}
	return globalStatus;
}

static inline Std_ReturnType ComM_Internal_Enter_ReadySleep(const ComM_ChannelType* ChannelConf,
															ComM_Internal_ChannelType* ChannelInternal) {
	boolean propagateToBusSM = (ChannelInternal->Mode != COMM_FULL_COMMUNICATION);
	ChannelInternal->LightTimeoutTimeLeft = ChannelConf->LightTimeout;
	ChannelInternal->Mode = COMM_FULL_COMMUNICATION;
	ChannelInternal->SubMode = COMM_SUBMODE_READY_SLEEP;

	Std_ReturnType status, globalStatus = E_OK;
	status = ComM_Internal_NotifyNm(ChannelConf);  /**< @req COMM133.1 */
	if (status > globalStatus){
		globalStatus = status;
	}
	if (propagateToBusSM) {
		status = ComM_Internal_PropagateComMode(ChannelConf);
		if (status > globalStatus){
			globalStatus = status;
		}
	}
	return globalStatus;
}
