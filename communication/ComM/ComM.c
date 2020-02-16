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

//lint -emacro(904,COMM_VALIDATE_INIT,COMM_VALIDATE_CHANNEL,COMM_VALIDATE_USER,COMM_VALIDATE_PARAMETER) //904 PC-Lint exception to MISRA 14.7 (validate macros).

/* Globally fulfilled requirements */
/** @req COMM863 */
/** @req COMM051 */
/** @req COMM845 */
/** @req COMM846 */
/** @req COMM881 */
/** @req COMM880 */
/** @req COMM686 */
/** @req COMM51.partially */
/** @req COMM191 */
/** @req COMM301 */
/** @req COMM488 */
/** @req COMM599.partially */
/** @req COMM509 */
/** @req COMM459 */
/** @req COMM462 */
/** @req COMM457.bswbuilder */
/** @req COMM419.bswbuilder */
/** @req COMM327.bswbuilder */
/** @req COMM159.bswbuilder */
/** @req COMM322.bswbuilder */
/** @req COMM460.bswbuilder */
/** @req COMM549.bswbuilder */
/** @req COMM464 */

#include <string.h>
#include "ComM.h" /** @req COMM463.partially */
#include "ComM_Dcm.h" /** @req COMM463.partially */
#include "ComM_EcuM.h"
#if (COMM_DEV_ERROR_DETECT == STD_ON)
#include "Det.h"
#endif

#if defined(USE_DEM)
#include "Dem.h"
#endif

#include "ComM_BusSm.h" /** @req COMM463.partially */
#include "ComM_Internal.h"

#if defined(USE_CANSM) || defined(COMM_TESTS)
#include "CanSM.h" /** @req COMM506.partially */
#endif

#if defined(USE_LINSM)
#include "LinSM.h" /** @req COMM506.partially */
#endif

#if defined(USE_NM) || defined(COMM_TESTS)
#include "Nm.h" /** @req COMM506.partially @req COMM828 */
#endif

#if defined(USE_BSWM) /** @req COMM506.partially */
#include "BswM.h"
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
		ComM_Internal.Channels[i].SubMode = COMM_NO_COM_NO_PENDING_REQUEST; /** @req COMM898 */
		ComM_Internal.Channels[i].RequestedSubMode = COMM_NO_COM_NO_PENDING_REQUEST;
		ComM_Internal.Channels[i].UserRequestMask = 0;
		ComM_Internal.Channels[i].DCM_Requested = FALSE;
		ComM_Internal.Channels[i].InhibitionStatus = COMM_INHIBITION_STATUS_NONE;
		ComM_Internal.Channels[i].NmIndicationMask = COMM_NM_INDICATION_NONE;
		ComM_Internal.Channels[i].CommunicationAllowed = FALSE; /** @req COMM884 */
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

Std_ReturnType ComM_GetState(NetworkHandleType Channel, ComM_StateType *State)
{
	COMM_VALIDATE_INIT(COMM_SERVICEID_GETSTATE);

	ComM_Internal_ChannelType* ChannelInternal = &ComM_Internal.Channels[Channel];
	*State = ChannelInternal->SubMode;

	return E_OK;
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

	/** @req COMM500 */
	return ComM_Internal_RequestComMode(User, ComMode);
}

static Std_ReturnType ComM_Internal_RequestComMode(
				ComM_UserHandleType User, ComM_ModeType ComMode ){

	const ComM_UserType* UserConfig = &ComM_Config->Users[User]; /** @req COMM795 */
	ComM_Internal_UserType* UserInternal = &ComM_Internal.Users[User];

	UserInternal->RequestedMode = ComMode;  /**<  @req COMM500  @req COMM92 */
	uint32 userMask = (1LU << User);

	Std_ReturnType requestStatus = E_OK;

	/* Go through users channels. Relay to SMs. Collect overall success status */
	for (uint8 i = 0; i < UserConfig->ChannelCount; ++i) {
		const ComM_ChannelType* Channel = UserConfig->ChannelList[i];
		ComM_Internal_ChannelType* ChannelInternal = &ComM_Internal.Channels[Channel->Number];

		/** @req COMM625 */
		// Put user request into mask
		if (ComMode == COMM_NO_COMMUNICATION) {
			ChannelInternal->UserRequestMask &= ~(userMask);
		} else if (ComMode == COMM_FULL_COMMUNICATION) {
			ChannelInternal->UserRequestMask |= userMask; /** @req COMM839 */
		} else {
			//Nothing to be done.
		}

		// take request -> new state
		Std_ReturnType status = ComM_Internal_UpdateChannelState(Channel, TRUE);
		if (status > requestStatus){
			requestStatus = status;
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

Std_ReturnType ComM_GetRequestedComMode( ComM_UserHandleType User, ComM_ModeType* ComMode ){
	COMM_VALIDATE_INIT(COMM_SERVICEID_GETREQUESTEDCOMMODE);
	COMM_VALIDATE_USER(User, COMM_SERVICEID_GETREQUESTEDCOMMODE);

	/** @req COMM797 */
	ComM_Internal_UserType* UserInternal = &ComM_Internal.Users[User];
	*ComMode = UserInternal->RequestedMode;
	return E_OK;
}

/** @req COMM084 */
Std_ReturnType ComM_GetCurrentComMode( ComM_UserHandleType User, ComM_ModeType* ComMode ){
	COMM_VALIDATE_INIT(COMM_SERVICEID_GETCURRENTCOMMODE);
	COMM_VALIDATE_USER(User, COMM_SERVICEID_GETCURRENTCOMMODE);

	return ComM_Internal_PropagateGetCurrentComMode(User, ComMode);
}

Std_ReturnType ComM_PreventWakeUp( NetworkHandleType Channel, boolean Status ){
	COMM_VALIDATE_INIT(COMM_SERVICEID_PREVENTWAKEUP);
	COMM_VALIDATE_CHANNEL(Channel, COMM_SERVICEID_PREVENTWAKEUP);
#if (COMM_MODE_LIMITATION_ENABLED == STD_ON) /** @req COMM799 */
	ComM_Internal_ChannelType* ChannelInternal = &ComM_Internal.Channels[Channel];
	if (Status) {
		ChannelInternal->InhibitionStatus |=  (COMM_INHIBITION_STATUS_WAKE_UP);
	} else {
		ChannelInternal->InhibitionStatus &= ~(COMM_INHIBITION_STATUS_WAKE_UP);
	}
	return E_OK;
#else
	(void)Status; /* Avoid compiler warning  */
	return E_NOT_OK;
#endif
}

/** @req COMM800.partially */
Std_ReturnType ComM_LimitChannelToNoComMode( NetworkHandleType Channel, boolean Status ){
	COMM_VALIDATE_INIT(COMM_SERVICEID_LIMITCHANNELTONOCOMMODE);
	COMM_VALIDATE_CHANNEL(Channel, COMM_SERVICEID_LIMITCHANNELTONOCOMMODE);
#if (COMM_MODE_LIMITATION_ENABLED == STD_ON)
	const ComM_ChannelType* ChannelConf = &ComM_Config->Channels[Channel];
	ComM_Internal_ChannelType* ChannelInternal = &ComM_Internal.Channels[Channel];
	if (Status) {
		ChannelInternal->InhibitionStatus |=  (COMM_INHIBITION_STATUS_NO_COMMUNICATION);
	} else {
		ChannelInternal->InhibitionStatus &= ~(COMM_INHIBITION_STATUS_NO_COMMUNICATION);
	}
	return ComM_Internal_UpdateChannelState(ChannelConf, FALSE);
#else
	(void)Status; /* Avoid compiler warning */
	return E_NOT_OK;
#endif
}

/** @req COMM801.partially */
Std_ReturnType ComM_LimitECUToNoComMode( boolean Status ){
	COMM_VALIDATE_INIT(COMM_SERVICEID_LIMITECUTONOCOMMODE);
#if (COMM_MODE_LIMITATION_ENABLED == STD_ON)
	ComM_Internal.NoCommunication = Status;
	uint8 Channel;
	Std_ReturnType totalStatus = E_OK;
	for (Channel = 0; Channel < COMM_CHANNEL_COUNT; Channel++) {
		const ComM_ChannelType* ChannelConf = &ComM_Config->Channels[Channel];
		Std_ReturnType status = ComM_Internal_UpdateChannelState(ChannelConf, FALSE);
		if (status > totalStatus) {
			totalStatus = status;
		}
	}
	return totalStatus;
#else
	(void)Status;  /* Avoid compiler warning */
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
	(void)(CounterValue);
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
/** @req COMM383 */
void ComM_Nm_NetworkStartIndication( NetworkHandleType Channel ){
	COMM_VALIDATE_INIT_NORV(COMM_SERVICEID_NM_NETWORKSTARTINDICATION); /** @req COMM805 */
	COMM_VALIDATE_CHANNEL_NORV(Channel, COMM_SERVICEID_NM_NETWORKSTARTINDICATION);
	const ComM_ChannelType* ChannelConf = &ComM_Config->Channels[Channel];
	ComM_Internal_ChannelType* ChannelInternal = &ComM_Internal.Channels[Channel];

	// Used to simulate Wake-up
	ChannelInternal->NmIndicationMask |= COMM_NM_INDICATION_RESTART;
	Std_ReturnType status = ComM_Internal_UpdateChannelState(ChannelConf, FALSE);
	if (status != E_OK) {
		// TODO: report error?
	}
}

/** @req COMM390 */
void ComM_Nm_NetworkMode( NetworkHandleType Channel ){
	COMM_VALIDATE_INIT_NORV(COMM_SERVICEID_NM_NETWORKMODE); /** @req COMM806 */
	COMM_VALIDATE_CHANNEL_NORV(Channel, COMM_SERVICEID_NM_NETWORKMODE);
	const ComM_ChannelType* ChannelConf = &ComM_Config->Channels[Channel];
	ComM_Internal_ChannelType* ChannelInternal = &ComM_Internal.Channels[Channel];

	ChannelInternal->NmIndicationMask |= COMM_NM_INDICATION_NETWORK_MODE;
	Std_ReturnType status = ComM_Internal_UpdateChannelState(ChannelConf, FALSE);
	if (status != E_OK) {
		// TODO: report error?
	}
}

/** @req COMM391 */
void ComM_Nm_PrepareBusSleepMode( NetworkHandleType Channel ){
	COMM_VALIDATE_INIT_NORV(COMM_SERVICEID_NM_PREPAREBUSSLEEPMODE); /** @req COMM808 */
	COMM_VALIDATE_CHANNEL_NORV(Channel, COMM_SERVICEID_NM_PREPAREBUSSLEEPMODE);
	const ComM_ChannelType* ChannelConf = &ComM_Config->Channels[Channel];
	ComM_Internal_ChannelType* ChannelInternal = &ComM_Internal.Channels[Channel];

	ChannelInternal->NmIndicationMask |= COMM_NM_INDICATION_PREPARE_BUS_SLEEP;
	Std_ReturnType status = ComM_Internal_UpdateChannelState(ChannelConf, FALSE);
	if (status != E_OK) {
		// TODO: report error?
	}
}

/** @req COMM392 */
void ComM_Nm_BusSleepMode( NetworkHandleType Channel ){
	COMM_VALIDATE_INIT_NORV(COMM_SERVICEID_NM_BUSSLEEPMODE); /** @req COMM810 */
	COMM_VALIDATE_CHANNEL_NORV(Channel, COMM_SERVICEID_NM_BUSSLEEPMODE);
	const ComM_ChannelType* ChannelConf = &ComM_Config->Channels[Channel];
	ComM_Internal_ChannelType* ChannelInternal = &ComM_Internal.Channels[Channel];

	ChannelInternal->NmIndicationMask |= COMM_NM_INDICATION_BUS_SLEEP;
	Std_ReturnType status = ComM_Internal_UpdateChannelState(ChannelConf, FALSE);
	if (status != E_OK) {
		// TODO: report error?
	}
}

/** @req COMM792 */
void ComM_Nm_RestartIndication( NetworkHandleType Channel ){
	COMM_VALIDATE_INIT_NORV(COMM_SERVICEID_NM_RESTARTINDICATION); /** @req COMM812 */
	COMM_VALIDATE_CHANNEL_NORV(Channel, COMM_SERVICEID_NM_RESTARTINDICATION);
	const ComM_ChannelType* ChannelConf = &ComM_Config->Channels[Channel];
	ComM_Internal_ChannelType* ChannelInternal = &ComM_Internal.Channels[Channel];

	ChannelInternal->NmIndicationMask |= COMM_NM_INDICATION_RESTART;
	Std_ReturnType status = ComM_Internal_UpdateChannelState(ChannelConf, TRUE);
	if (status != E_OK) {
		// TODO: report error?
	}

}


// ECU State Manager Callbacks
// ---------------------------
void ComM_EcuM_WakeUpIndication( NetworkHandleType Channel ){
	COMM_VALIDATE_INIT_NORV(COMM_SERVICEID_ECUM_WAKEUPINDICATION); /** @req COMM814 */
	COMM_VALIDATE_CHANNEL_NORV(Channel, COMM_SERVICEID_ECUM_WAKEUPINDICATION);
}


// Diagnostic Communication Manager Callbacks
// ------------------------------------------

void ComM_DCM_ActiveDiagnostic(NetworkHandleType Channel){
	COMM_VALIDATE_INIT_NORV(COMM_SERVICEID_DCM_ACTIVEDIAGNOSTIC);
	COMM_VALIDATE_CHANNEL_NORV(Channel, COMM_SERVICEID_DCM_ACTIVEDIAGNOSTIC);

	const ComM_ChannelType* ChannelConf = &ComM_Config->Channels[Channel];
	ComM_Internal.Channels[Channel].DCM_Requested = TRUE;

	/* TODO: What to do if return error */
	(void)ComM_Internal_UpdateChannelState(ChannelConf, FALSE); /** @req COMM866 */
}

void ComM_DCM_InactiveDiagnostic(NetworkHandleType Channel){
	COMM_VALIDATE_INIT_NORV(COMM_SERVICEID_DCM_INACTIVEDIAGNOSTIC);
	COMM_VALIDATE_CHANNEL_NORV(Channel, COMM_SERVICEID_DCM_INACTIVEDIAGNOSTIC);

	const ComM_ChannelType* ChannelConf = &ComM_Config->Channels[Channel];
	ComM_Internal.Channels[Channel].DCM_Requested = FALSE;

	/* TODO: What to do if return error */
	(void)ComM_Internal_UpdateChannelState(ChannelConf, FALSE);
}


// Bus State Manager Callbacks
// ---------------------------

void ComM_BusSM_ModeIndication( NetworkHandleType Channel, ComM_ModeType *Mode ){
	COMM_VALIDATE_INIT_NORV(COMM_SERVICEID_BUSSM_MODEINDICATION); /** @req COMM816 */
	COMM_VALIDATE_CHANNEL_NORV(Channel, COMM_SERVICEID_BUSSM_MODEINDICATION);
	ComM_ModeType ComMode;
	ComMode = *Mode;
	ComM_Internal_ChannelType* ChannelInternal = &ComM_Internal.Channels[Channel];
	const ComM_ChannelType* ChannelConf = &ComM_Config->Channels[Channel];

	ChannelInternal->Mode = ComMode;
	ChannelInternal->SubMode = ChannelInternal->RequestedSubMode; /** @req COMM898 */ /** @req COMM899 */ /** @req COMM883 */

	switch(ComMode)
	{
		case COMM_NO_COMMUNICATION:
			break;
		case COMM_SILENT_COMMUNICATION:
			break;
		case COMM_FULL_COMMUNICATION:
		    /* TODO: What to do if return error */
			(void)ComM_Internal_NotifyNm(ChannelConf);
			break;
	}

#if defined (USE_BSWM)
    BswM_ComM_CurrentMode(ChannelConf->BusSMNetworkHandle, ComMode); /* @req COMM976 */
#endif

}


// Scheduled main function
// -----------------------

// Prototype right here because this function should not be exposed
void ComM_MainFunction(NetworkHandleType Channel);

/** @req COMM429 */
void ComM_MainFunction(NetworkHandleType Channel) {
	const ComM_ChannelType* ChannelConf = &ComM_Config->Channels[Channel];
	ComM_Internal_ChannelType* ChannelInternal = &ComM_Internal.Channels[Channel];

	if ((ChannelConf->NmVariant == COMM_NM_VARIANT_NONE) ||
		(ChannelConf->NmVariant == COMM_NM_VARIANT_LIGHT)) {
		Std_ReturnType status = ComM_Internal_TickFullComMinTime(ChannelConf, ChannelInternal);
		if (status != E_OK) {
			// TODO: Report error?
		}
	}
	if (ChannelConf->NmVariant == COMM_NM_VARIANT_LIGHT) {
		Std_ReturnType status = ComM_Internal_TickLightTime(ChannelConf, ChannelInternal);
		if (status != E_OK) {
			// TODO: Report error?
		}
	}

	if (ChannelInternal->Mode == COMM_NO_COMMUNICATION &&
		ChannelInternal->SubMode == COMM_NO_COM_REQUEST_PENDING )
	{
		Std_ReturnType status = ComM_Internal_UpdateChannelState(ChannelConf, FALSE);
		if (status != E_OK) {
			// TODO: Report error?
		}
	}
}


void ComM_CommunicationAllowed( NetworkHandleType Channel, boolean Allowed) {
	COMM_VALIDATE_INIT_NORV(COMM_SERVICEID_COMMUNICATIONALLOWED);
	COMM_VALIDATE_CHANNEL_NORV(Channel, COMM_SERVICEID_COMMUNICATIONALLOWED);

	ComM_Internal_ChannelType* ChannelInternal = &ComM_Internal.Channels[Channel];
	ChannelInternal->CommunicationAllowed = Allowed; /** @req COMM885 */
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
		(ChannelInternal->SubMode == COMM_FULL_COM_READY_SLEEP)) {
		if (ChannelConf->MainFunctionPeriod >= ChannelInternal->LightTimeoutTimeLeft) {
			ChannelInternal->LightTimeoutTimeLeft = 0;
			status = ComM_Internal_UpdateChannelState(ChannelConf, FALSE);
		} else {
			ChannelInternal->LightTimeoutTimeLeft -= ChannelConf->MainFunctionPeriod;
		}
	}
	return status;
}

static Std_ReturnType ComM_Internal_PropagateGetCurrentComMode( ComM_UserHandleType User, ComM_ModeType* ComMode ){
	const ComM_UserType* UserConfig = &ComM_Config->Users[User];

	ComM_ModeType requestMode = COMM_FULL_COMMUNICATION;
	Std_ReturnType totalStatus = E_OK;
	/* Go through users channels. Relay to SMs. Collect overall mode and success status */
	for (uint8 i = 0; i < UserConfig->ChannelCount; ++i) {
		const ComM_ChannelType* Channel = UserConfig->ChannelList[i]; /** @req COMM798 */
		Std_ReturnType status = E_OK;
		ComM_ModeType mode = COMM_FULL_COMMUNICATION;
		switch (Channel->BusType) {
#if defined(USE_CANSM) || defined(COMM_TESTS)
			case COMM_BUS_TYPE_CAN:
				status = CanSM_GetCurrentComMode(Channel->BusSMNetworkHandle, &mode); /** @req COMM855 */
				break;
#endif
#if defined(USE_LINSM)
		case COMM_BUS_TYPE_LIN:
			status = LinSM_GetCurrentComMode(Channel->BusSMNetworkHandle, &mode); /** @req COMM857 */
			break;
#endif
			default:
				status = E_NOT_OK;
				break;
		}
		if (status == E_OK) {
			if (mode < requestMode) {	/** @req COMM176 */
				requestMode = mode;
			}
		} else {
			totalStatus = status;
		}
	}
	*ComMode = requestMode;
	return totalStatus;
}

/** @req COMM073  @req COMM071
 *  @req COMM069  @req COMM402 */
static Std_ReturnType ComM_Internal_PropagateComMode( const ComM_ChannelType* ChannelConf, ComM_ModeType ComMode ){

	Std_ReturnType busSMStatus = E_OK;
	switch (ChannelConf->BusType) {
#if defined(USE_CANSM) || defined(COMM_TESTS)
		case COMM_BUS_TYPE_CAN:
			busSMStatus = CanSM_RequestComMode(ChannelConf->BusSMNetworkHandle, ComMode); /** @req COMM854 */
			break;
#endif
#if defined(USE_LINSM)
		case COMM_BUS_TYPE_LIN:
			busSMStatus = LinSM_RequestComMode(ChannelConf->BusSMNetworkHandle, ComMode); /** @req COMM856 */
			break;
#endif
		default:
			busSMStatus = E_NOT_OK;
			break;
	}

	return busSMStatus;
}

/** @req COMM472  @req COMM602  @req COMM261 */
static Std_ReturnType ComM_Internal_NotifyNm( const ComM_ChannelType* ChannelConf){
	ComM_Internal_ChannelType* ChannelInternal = &ComM_Internal.Channels[ChannelConf->Number];

	Std_ReturnType status = E_OK;

	if (ChannelConf->NmVariant == COMM_NM_VARIANT_FULL)
	{
#if defined(USE_NM) || defined(COMM_TESTS)
		if (ChannelInternal->Mode == COMM_FULL_COMMUNICATION)
		{
			if (ChannelInternal->SubMode == COMM_FULL_COM_NETWORK_REQUESTED)
			{
				status = Nm_NetworkRequest(ChannelConf->NmChannelHandle);  /** @req COMM869.partially */ /** @req COMM870 */
			}
			else if (ChannelInternal->SubMode == COMM_FULL_COM_READY_SLEEP)
			{
				status = Nm_NetworkRelease(ChannelConf->NmChannelHandle);  /**< @req COMM133 */
			}
		}
#else
		status = E_NOT_OK;
#endif
	}

	if (ChannelConf->NmVariant == COMM_NM_VARIANT_NONE || ChannelConf->NmVariant == COMM_NM_VARIANT_LIGHT)
	{
		if (ChannelInternal->Mode == COMM_FULL_COMMUNICATION)
		{
			if (ChannelInternal->SubMode == COMM_FULL_COM_NETWORK_REQUESTED)
			{
				/* start timer */ /* cancelling timer by restarting it */
				ChannelInternal->FullComMinDurationTimeLeft = COMM_T_MIN_FULL_COM_MODE_DURATION; /** @req COMM886 */ /** @req COMM887 */
			}
			else if (ChannelInternal->SubMode == COMM_FULL_COM_READY_SLEEP)
			{
				ChannelInternal->LightTimeoutTimeLeft = ChannelConf->LightTimeout; /** @req COMM891 */
			}
		}
	}

	return status;
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
	if (ChannelInternal->NmIndicationMask & COMM_NM_INDICATION_RESTART) {
		// "restart" indication
		status = ComM_Internal_Enter_NetworkRequested(ChannelConf, ChannelInternal);  /**< @req COMM583 */
		ChannelInternal->NmIndicationMask &= ~(COMM_NM_INDICATION_RESTART);
	} else {
		if (((ChannelInternal->InhibitionStatus & COMM_INHIBITION_STATUS_NO_COMMUNICATION) ||
			(ChannelInternal->InhibitionStatus & COMM_INHIBITION_STATUS_WAKE_UP) ||
			(ComM_Internal.NoCommunication == TRUE)) && ChannelInternal->DCM_Requested == FALSE) { /** @req COMM182 */
			// Inhibition is active
			/** @req COMM302  @req COMM218  @req COMM219  @req COMM215.3 */
			if (isRequest){
				/* @req COMM142 */
				if (ComM_Internal.InhibitCounter < 65535)
				{
					ComM_Internal.InhibitCounter++;
				}
			}
		} else {
			if (ChannelInternal->UserRequestMask != 0 || ChannelInternal->DCM_Requested == TRUE) {
				// Channel is requested
				status = ComM_Internal_Enter_NetworkRequested(ChannelConf, ChannelInternal);
			} else {
				// Channel is not requested
				ChannelInternal->SubMode = COMM_NO_COM_NO_PENDING_REQUEST; /** @req COMM897 */
			}
		}
	}
	return status;
}

static inline Std_ReturnType ComM_Internal_UpdateFromSilentCom(const ComM_ChannelType* ChannelConf,
					ComM_Internal_ChannelType* ChannelInternal,	boolean isRequest) {
	Std_ReturnType status = E_OK;
	if (ChannelInternal->NmIndicationMask & COMM_NM_INDICATION_RESTART) {
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
		if (((ChannelInternal->InhibitionStatus & COMM_INHIBITION_STATUS_NO_COMMUNICATION) ||
			(ComM_Internal.NoCommunication == TRUE)) && ChannelInternal->DCM_Requested == FALSE) { /** @req COMM182 */
			// Inhibition is active
			/** @req COMM215.2 */
			if (isRequest) {
				/* @req COMM142 */
				if (ComM_Internal.InhibitCounter < 65535)
				{
					ComM_Internal.InhibitCounter++;
				}
			}
		} else {
			if (ChannelInternal->UserRequestMask != 0 || ChannelInternal->DCM_Requested == TRUE) { /** @req COMM878 */
				// Channel is requested
				status = ComM_Internal_Enter_NetworkRequested(ChannelConf, ChannelInternal);
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
				((ChannelInternal->SubMode == COMM_FULL_COM_NETWORK_REQUESTED && ChannelConf->NmVariant == COMM_NM_VARIANT_PASSIVE) || ChannelInternal->SubMode == COMM_FULL_COM_READY_SLEEP)) {
		// "prepare bus sleep" indication
		status = ComM_Internal_Enter_SilentCom(ChannelConf, ChannelInternal);  /**< @req COMM299.partially */ /** @req COMM900 */
		ChannelInternal->NmIndicationMask &= ~(COMM_NM_INDICATION_PREPARE_BUS_SLEEP);
	} else {
		if (((ChannelInternal->InhibitionStatus & COMM_INHIBITION_STATUS_NO_COMMUNICATION) ||
			(ComM_Internal.NoCommunication == TRUE)) && ChannelInternal->DCM_Requested == FALSE) { /** @req COMM182 */
			// Inhibition is active
			if (ComM_Internal_FullComMinTime_AllowsExit(ChannelConf, ChannelInternal)) {
				if (ChannelInternal->SubMode == COMM_FULL_COM_READY_SLEEP) {
					if ((ChannelConf->NmVariant == COMM_NM_VARIANT_LIGHT) &&
						(ChannelInternal->LightTimeoutTimeLeft == 0)) {
						status = ComM_Internal_Enter_NoCom(ChannelConf, ChannelInternal);  /**< @req COMM610 */
					}
				} else {
					/** @req COMM303  @req COMM215.1 */
					status = ComM_Internal_Enter_ReadySleep(ChannelConf, ChannelInternal);
				}
			}
			if (isRequest){
				/* @req COMM142 */
				if (ComM_Internal.InhibitCounter < 65535)
				{
					ComM_Internal.InhibitCounter++;
				}
			}
		} else {
			if (ChannelInternal->UserRequestMask == 0 && ChannelInternal->DCM_Requested == FALSE) {
				// Channel no longer requested
				if (ComM_Internal_FullComMinTime_AllowsExit(ChannelConf, ChannelInternal)) {
					if (ChannelInternal->SubMode == COMM_FULL_COM_READY_SLEEP) {
						if ((ChannelConf->NmVariant == COMM_NM_VARIANT_LIGHT) &&
							(ChannelInternal->LightTimeoutTimeLeft == 0)) {
							status = ComM_Internal_Enter_NoCom(ChannelConf, ChannelInternal);  /**< @req COMM610 */
						}
					} else {
						status = ComM_Internal_Enter_ReadySleep(ChannelConf, ChannelInternal); /** @req COMM889 */ /** @req COMM888 */ /** @req COMM890 */
					}
				}
			} else {
				// Channel is requested
				if (ChannelInternal->SubMode != COMM_FULL_COM_NETWORK_REQUESTED) { /** @req COMM842 */
					status = ComM_Internal_Enter_NetworkRequested(ChannelConf, ChannelInternal);  /** @reqCOMM882 @reqCOMM883*/
				}
			}
		}
	}
	return status;
}

static inline Std_ReturnType ComM_Internal_Enter_NoCom(const ComM_ChannelType* ChannelConf,
													ComM_Internal_ChannelType* ChannelInternal) {
	ChannelInternal->RequestedSubMode = COMM_NO_COM_NO_PENDING_REQUEST;
	return ComM_Internal_PropagateComMode(ChannelConf, COMM_NO_COMMUNICATION);
}

static inline Std_ReturnType ComM_Internal_Enter_SilentCom(const ComM_ChannelType* ChannelConf,
														ComM_Internal_ChannelType* ChannelInternal) {
	ChannelInternal->RequestedSubMode = COMM_SILENT_COM;
	return ComM_Internal_PropagateComMode(ChannelConf, COMM_SILENT_COMMUNICATION);
}

static inline Std_ReturnType ComM_Internal_Enter_NetworkRequested(const ComM_ChannelType* ChannelConf,
																ComM_Internal_ChannelType* ChannelInternal) {
	Std_ReturnType status = E_OK;

	if (ChannelInternal->SubMode == COMM_NO_COM_NO_PENDING_REQUEST)
	{
		ChannelInternal->SubMode = COMM_NO_COM_REQUEST_PENDING; /** @req COMM875 */ /** @req COMM894 */ /** @req COMM876 */
	}

	if (ChannelInternal->CommunicationAllowed == TRUE)
	{
		ChannelInternal->RequestedSubMode = COMM_FULL_COM_NETWORK_REQUESTED;
		status = ComM_Internal_PropagateComMode(ChannelConf, COMM_FULL_COMMUNICATION);
	}

	return status;
}

static inline Std_ReturnType ComM_Internal_Enter_ReadySleep(const ComM_ChannelType* ChannelConf,
															ComM_Internal_ChannelType* ChannelInternal) {
	Std_ReturnType status = E_OK;

	if (ChannelInternal->Mode != COMM_FULL_COMMUNICATION)
	{
		ChannelInternal->RequestedSubMode = COMM_FULL_COM_READY_SLEEP;
		status = ComM_Internal_PropagateComMode(ChannelConf, COMM_FULL_COMMUNICATION);
	}
	else
	{
		ChannelInternal->SubMode = COMM_FULL_COM_READY_SLEEP;
		status = ComM_Internal_NotifyNm(ChannelConf);
	}

	return status;
}
