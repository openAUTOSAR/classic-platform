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

//lint -emacro(904,CANNM_VALIDATE_INIT,CANNM_VALIDATE_CHANNEL,CANNM_VALIDATE_NOTNULL) //904 PC-Lint exception to MISRA 14.7 (validate macros).


/* Globally fulfilled requirements */
/** @req CANNM199 */ /* parameter checking */
/** @req CANNM196 */ /* equal to CANNM241 */
/** @req CANNM241 */ /* if det enabled => parameter checking */
/** @req CANNM292 */ /* check parameter pointers == null (all except init) */
/** @req CANNM210 */ /* mainfunction => instande id = 0 when deterror */
/** @req CANNM325 */ /* only describes optional API-functions */
/** @req CANNM287 */ /* each debuggable variable => globally in header => CanNm.h*/
/** @req CANNM288 */ /* lool CANNM287, this requirement specifies the headerfile */
/** @req CANNM289 */ /* for variables in header => size calcable with sizeof */
/** @req CANNM290 */ /* debuggable variables => in bsw description */
/** @req CANNM206 */
/** @req CANNM203 */
/** @req CANNM192 */ /* => CANNM_VALIDATE_CHANNEL */
/** @req CANNM191 */ /* => CANNM_VALIDATE_INIT */
/** @req CANNM093 */
/** @req CANNM089 */
/** @req CANNM039 */ /* when not init every functions reports deterror => CANNM_VALIDATE_INIT at startup from every function*/
/** @req CANNM020 */ /* report to dem */
/** @req CANNM088 */ /* 7.1 Coordination algorithm */
/** @req CANNM089 */ /* 7.1 Coordination algorithm */
/** @req CANNM093 */ /* 7.2 Operation modes */
/** @req CANNM146 */ /* cpu independent */
/** @req CANNM158 */ /* supporting user data can be statically turned on or off (config dependent) */
/** @req CANNM161 */ /* passive mode can be statically turned on or off (config dependent) */
/** @req CANNM162 */ /* passive mode is statically configured for all instances */
/** @req CANNM189 */ /* no deterrors returned by NM-API */
/** @req CANNM190 */ /* no demerrors returned by NM-API */
/** @req CANNM197 toolchain */ /* type checking at compile time */
/** @req CANNM198 toolchain */ /* value checking at config time */


/* CANNM081 => 299, 300, 301 */
/** @req CANNM299 */ /* CanNm_Cfg.c shall contain pre-compile time configuration parameters implemented as const */
/** @req CANNM300 */ /* CanNm_Lcfg.c shall contain link time configurable parameters */
/** @req CANNM301 */ /* CanNm_PBcfg.c shall contain post build time configurable parameters */

/* CANNM044 => 302, 303, 304 */
/** @req CANNM302 */ /* CanNm.h shall contain the declaration of provided interface functions */
/** @req CANNM303 */ /* CanNm_Cbk.h shall contain the declaration of provided call-back functions */
/** @req CANNM304 */ /* CanNm_Cfg.h shall contain pre-compile time configurable parameters */

/* CANNM001 => 237, 238 */
/** @req CANNM237 */ /* The CanNm module shall provide the periodic transmission mode. In this transmission mode the CanNm module shall send Network Management PDUs periodically */
/** @req CANNM238 */ /* The CanNm module may provide the periodic transmission mode with bus load reduction. In this transmission mode the CanNm module shall transmit Network Management PDUs due to a specific algorithm */

/* CANNM016 => 243, 244 */
/** @req CANNM243 */ /* parameter value check only in devmode */
/** @req CANNM244 */ /* reject service when invalid parameters are used and report det error */

#include "ComStack_Types.h" 	/** @req CANNM305 */
#include "CanNm.h"				/** @req CANNM306 */
#include "CanNm_Cfg.h"
#include "CanNm_Internal.h"
#include "Nm_Cbk.h"				/** @req CANNM307 */
#include "NmStack_Types.h"		/** @req CANNM309 */

#if (CANNM_COM_USER_DATA_SUPPORT == STD_ON || CANNM_USER_DATA_ENABLED == STD_ON)
#include "SchM_CanNm.h"			/** @req CANNM310 */
#endif

#include "MemMap.h"				/** @req CANNM311 */

#include "CanIf.h"				/** @req CANNM312 */
#include "Nm.h"					/** @req CANNM313 */ /* according to the spec it should be Nm_Cfg.h */

#if (CANNM_COM_USER_DATA_SUPPORT == STD_ON)	/** @req CANNM326 */
#include "PduR_CanNm.h"
#endif

#include <string.h>				/* what is string doing here?? */

#if (CANNM_DEV_ERROR_DETECT == STD_ON) /** @req CANNM188 */
#include "Det.h"				/** @req CANNM308 */
#endif
#if defined(USE_DEM)
#include "Dem.h"
#endif

static const CanNm_ConfigType* CanNm_ConfigPtr;

//lint -save -e785 //PC-Lint exception: Too few initializers for aggregate...
CanNm_InternalType CanNm_Internal = {
		.InitStatus = CANNM_UNINIT,
};
//lint -restore



#ifdef HOST_TEST
void GetChannelRunTimeData(uint32 channelId, uint32* messageCycleTimeLeft, uint32* timeoutTimeLeft) {

    CanNm_Internal_ChannelType* ChannelInternal = &CanNm_Internal.Channels[channelId];

    *messageCycleTimeLeft =  ChannelInternal->MessageCycleTimeLeft;
    *timeoutTimeLeft = ChannelInternal->TimeoutTimeLeft;
}
#endif



/** Initialize the complete CanNm module, i.e. all channels which are activated */
/** must be called directly after canif in order to fulfill CANNM253 */ /** @req CANNM253 */
void CanNm_Init( const CanNm_ConfigType * const cannmConfigPtr ){
	CANNM_VALIDATE_NOTNULL_INIT(cannmConfigPtr, CANNM_SERVICEID_INIT, 0);  //shall not be done for init

	CanNm_ConfigPtr = cannmConfigPtr;  /**< @req CANNM060 */

	uint8 channel;
	for (channel = 0; channel < CANNM_CHANNEL_COUNT; channel++) {
		const CanNm_ChannelType* ChannelConf = &CanNm_ConfigPtr->Channels[channel];
		CanNm_Internal_ChannelType* ChannelInternal = &CanNm_Internal.Channels[channel];

		ChannelInternal->Mode = NM_MODE_BUS_SLEEP;    /** @req CANNM144 */
		ChannelInternal->State = NM_STATE_BUS_SLEEP;  /** @req CANNM141 */
		ChannelInternal->Requested = FALSE;           /** @req CANNM143 */ /* reqeuested should be a state and not a flag */
		ChannelInternal->CommunicationEnabled = TRUE;
		ChannelInternal->immediateModeActive = FALSE;

#if (CANNM_COM_USER_DATA_SUPPORT == STD_ON)
		ChannelInternal->TransmissionStatus = CANNM_NO_TRANSMISSION;
#elif  (CANNM_USER_DATA_ENABLED == STD_ON) /* and com_user_data... == OFF */
		ChannelInternal->IsUserDataSet = FALSE;
#endif

		/** @req CANNM085 */
		memset(ChannelInternal->TxMessageSdu, 0x00, 8);
#if (CANNM_COM_USER_DATA_SUPPORT == STD_ON)
		memset(ChannelInternal->SpontaneousTxMessageSdu, 0x00, 8);
#endif
		memset(ChannelInternal->RxMessageSdu, 0x00, 8);

		/** @req CANNM025 */
		uint8* destUserData = CanNm_Internal_GetUserDataPtr(ChannelConf, ChannelInternal->TxMessageSdu);
		uint8 userDataLength = CanNm_Internal_GetUserDataLength(ChannelConf);
		memset(destUserData, 0xFF, userDataLength);

#if (CANNM_COM_USER_DATA_SUPPORT == STD_ON)
		destUserData = CanNm_Internal_GetUserDataPtr(ChannelConf, ChannelInternal->SpontaneousTxMessageSdu);
		userDataLength = CanNm_Internal_GetUserDataLength(ChannelConf);
		memset(destUserData, 0xFF, userDataLength);
#endif

		/** @req CANNM013 */
		if (ChannelConf->NidPosition != CANNM_PDU_OFF) {
			ChannelInternal->TxMessageSdu[ChannelConf->NidPosition] = ChannelConf->NodeId;
#if (CANNM_COM_USER_DATA_SUPPORT == STD_ON)
			ChannelInternal->SpontaneousTxMessageSdu[ChannelConf->NidPosition] = ChannelConf->NodeId;
#endif
		}

	    /** @req CANNM060 */
#if (CANNM_PASSIVE_MODE_ENABLED != STD_ON)
        if (ChannelConf->ImmediateNmTransmissions > 0)
        {
            ChannelInternal->MessageCycleTimeLeft = 0;
        }
        else
        {
            ChannelInternal->MessageCycleTimeLeft = 0;
        }
        ChannelInternal->MessageTimeoutTimeLeft = 0;
#endif
	}

	CanNm_Internal.InitStatus = CANNM_INIT;

	/** @req CANNM061 */
	/** @req CANNM033 */
}

/** Passive startup of the AUTOSAR CAN NM. It triggers the transition from Bus-
  * Sleep Mode to the Network Mode in Repeat Message State.
  * This service has no effect if the current state is not equal to Bus-Sleep Mode. In
  * that case NM_E_NOT_EXECUTED is returned. */
Std_ReturnType CanNm_PassiveStartUp( const NetworkHandleType nmHandle){
	CANNM_VALIDATE_INIT(CANNM_SERVICEID_PASSIVESTARTUP);
	CANNM_VALIDATE_CHANNEL(nmHandle, CANNM_SERVICEID_PASSIVESTARTUP);

    uint8 channelIndex = CanNm_ConfigPtr->ChannelLookups[nmHandle];

	/** @req CANNM254 */
	const CanNm_ChannelType* ChannelConf = &CanNm_ConfigPtr->Channels[channelIndex];
	CanNm_Internal_ChannelType* ChannelInternal = &CanNm_Internal.Channels[channelIndex];
	Std_ReturnType status = E_OK;

	if (ChannelInternal->Mode == NM_MODE_BUS_SLEEP) {
		CanNm_Internal_BusSleep_to_RepeatMessage(ChannelConf, ChannelInternal,FALSE);  /**< @req CANNM128  @req CANNM314 */
		status = E_OK;
	} else {
		status = E_NOT_OK;  /** @req CANNM147 */ /** @req CANNM212 */
	}
	return status;
}

/** Request the network, since ECU needs to communicate on the bus. Network
  * state shall be changed to �requested� */
Std_ReturnType CanNm_NetworkRequest( const NetworkHandleType nmHandle ){

	CANNM_VALIDATE_INIT(CANNM_SERVICEID_NETWORKREQUEST);

    CANNM_VALIDATE_CHANNEL(nmHandle, CANNM_SERVICEID_NETWORKREQUEST);
    uint8 channelIndex = CanNm_ConfigPtr->ChannelLookups[nmHandle];

	/** @req CANNM256 */

	const CanNm_ChannelType* ChannelConf = &CanNm_ConfigPtr->Channels[channelIndex];
	CanNm_Internal_ChannelType* ChannelInternal = &CanNm_Internal.Channels[channelIndex];

	/** @req CANNM255 */

	ChannelInternal->Requested = TRUE;  /**< @req CANNM104 */ /* reqeuested should be a state and not a flag */

	if (ChannelInternal->Mode == NM_MODE_BUS_SLEEP) {
		CanNm_Internal_BusSleep_to_RepeatMessage(ChannelConf, ChannelInternal,TRUE);  /**< @req CANNM129  @req CANNM314 */
	} else if (ChannelInternal->Mode == NM_MODE_PREPARE_BUS_SLEEP) {
		CanNm_Internal_PrepareBusSleep_to_RepeatMessage(ChannelConf, ChannelInternal, TRUE);  /**< @req CANNM123  @req CANNM315 */
	} else if (ChannelInternal->Mode == NM_MODE_NETWORK) {
		if (ChannelInternal->State == NM_STATE_READY_SLEEP) {
			CanNm_Internal_ReadySleep_to_NormalOperation(ChannelConf, ChannelInternal);  /**< @req CANNM110 */
		}
	} else {
		//Nothing to be done
	}
	return E_OK;
}

/** Release the network, since ECU doesn�t have to communicate on the bus. Network
  * state shall be changed to �released�. */
Std_ReturnType CanNm_NetworkRelease( const NetworkHandleType nmHandle ){
	CANNM_VALIDATE_INIT(CANNM_SERVICEID_NETWORKRELEASE);
    CANNM_VALIDATE_CHANNEL(nmHandle, CANNM_SERVICEID_NETWORKRELEASE);
    uint8 channelIndex = CanNm_ConfigPtr->ChannelLookups[nmHandle];

	/** @req CANNM259 */

	const CanNm_ChannelType* ChannelConf = &CanNm_ConfigPtr->Channels[channelIndex];
	CanNm_Internal_ChannelType* ChannelInternal = &CanNm_Internal.Channels[channelIndex];

	if (!ChannelInternal->CommunicationEnabled) {
		return E_NOT_OK; /* @req CANNM294 */
	}

	/** @req CANNM258 */
	ChannelInternal->Requested = FALSE;  /**< @req CANNM105 */ /* released should be a state and not a flag */

	if (ChannelInternal->Mode == NM_MODE_NETWORK) {
		if (ChannelInternal->State == NM_STATE_NORMAL_OPERATION) {
			CanNm_Internal_NormalOperation_to_ReadySleep(ChannelConf, ChannelInternal);  /**< @req CANNM118 */
		}
	}
	return E_OK;
}

/** Disable the NM PDU transmission ability due to a ISO14229 Communication
  * Control (28hex) service */
#if (CANNM_COM_CONTROL_ENABLED == STD_ON) /* @req CANNM262 */
Std_ReturnType CanNm_DisableCommunication( const NetworkHandleType nmHandle ){

	CANNM_VALIDATE_INIT(CANNM_SERVICEID_DISABLECOMMUNICATION); /* @req CANNM261 */
#if (CANNM_PASSIVE_MODE_ENABLED == STD_ON) /** @req CANNM298 */
	return E_NOT_OK;
#endif

	CANNM_VALIDATE_CHANNEL(nmHandle, CANNM_SERVICEID_DISABLECOMMUNICATION);
	uint8 channelIndex = CanNm_ConfigPtr->ChannelLookups[nmHandle];

	CanNm_Internal_ChannelType* ChannelInternal = &CanNm_Internal.Channels[channelIndex];

	if (ChannelInternal->Mode != NM_MODE_NETWORK) {
		return E_NOT_OK; /* @req CANNM172 */
	}

	ChannelInternal->CommunicationEnabled = FALSE;

	return E_OK;
}
#endif


/** Enable the NM PDU transmission ability due to a ISO14229 Communication
  * Control (28hex) service */
#if (CANNM_COM_CONTROL_ENABLED == STD_ON) /* @req CANNM264 */
Std_ReturnType CanNm_EnableCommunication( const NetworkHandleType nmHandle ){

	CANNM_VALIDATE_INIT(CANNM_SERVICEID_ENABLECOMMUNICATION); /* @req CANNM263 */
#if (CANNM_PASSIVE_MODE_ENABLED == STD_ON) /** @req CANNM297 */
	return E_NOT_OK;
#endif

	CANNM_VALIDATE_CHANNEL(nmHandle, CANNM_SERVICEID_ENABLECOMMUNICATION);
	uint8 channelIndex = CanNm_ConfigPtr->ChannelLookups[nmHandle];

	CanNm_Internal_ChannelType* ChannelInternal = &CanNm_Internal.Channels[channelIndex];
	const CanNm_ChannelType* ChannelConf = &CanNm_ConfigPtr->Channels[channelIndex];

	if (ChannelInternal->Mode != NM_MODE_NETWORK) {/* @req CANNM295 */
		return E_NOT_OK; /* @req CANNM295 */
	}

	if (ChannelInternal->CommunicationEnabled) {
		return E_NOT_OK; /* @req CANNM177*/
	}

	ChannelInternal->MessageCycleTimeLeft = ChannelConf->MessageCycleOffsetTime + ChannelConf->MainFunctionPeriod; /* @req CANNM178 */
	ChannelInternal->TimeoutTimeLeft = ChannelConf->TimeoutTime + ChannelConf->MainFunctionPeriod; /* @req CANNM179 */
	ChannelInternal->CommunicationEnabled = TRUE;  /* @req CANNM176 */

	return E_OK;

}
#endif

#if (CANNM_PASSIVE_MODE_ENABLED != STD_ON) /** @req CANNM266 */
#if (CANNM_USER_DATA_ENABLED == STD_ON)  /** @req CANNM158 */
#if (CANNM_COM_USER_DATA_SUPPORT == STD_OFF) /** @req CANNM327 */
/** Set user data for NM messages transmitted next on the bus. */
/** @req CANNM159 */
Std_ReturnType CanNm_SetUserData( const NetworkHandleType nmHandle, const uint8* const nmUserDataPtr ){
	CANNM_VALIDATE_INIT(CANNM_SERVICEID_SETUSERDATA);

    CANNM_VALIDATE_CHANNEL(nmHandle, CANNM_SERVICEID_SETUSERDATA);
    uint8 channelIndex = CanNm_ConfigPtr->ChannelLookups[nmHandle];
	CANNM_VALIDATE_NOTNULL(nmUserDataPtr, CANNM_SERVICEID_SETUSERDATA, nmHandle);

	/** @req CANNM265 */

	Std_ReturnType retVal = E_NOT_OK;
	const CanNm_ChannelType* ChannelConf = &CanNm_ConfigPtr->Channels[channelIndex];
	CanNm_Internal_ChannelType* ChannelInternal = &CanNm_Internal.Channels[channelIndex];

	/* can only be called once per cycle (per channel) */
	if (!ChannelInternal->IsUserDataSet) /* this flag avoids inconsistency */
	{
		SchM_Enter_CanNm(EXCLUSIVE_AREA_0);

		ChannelInternal->IsUserDataSet = TRUE;

		SchM_Exit_CanNm(EXCLUSIVE_AREA_0);

		{
			uint8* destUserData = CanNm_Internal_GetUserDataPtr(ChannelConf, ChannelInternal->TxMessageSdu);
			uint8 userDataLength = CanNm_Internal_GetUserDataLength(ChannelConf);

			memcpy(destUserData, nmUserDataPtr, userDataLength);
		}

		ChannelInternal->IsUserDataSet = FALSE;

		retVal = E_OK;
	}

	return retVal;
}
#endif
#endif
#endif

#if (CANNM_USER_DATA_ENABLED == STD_ON)  /** @req CANNM158 */ /**@req CANNM268 */
/** Get user data out of the most recently received NM message. */
/** @req CANNM160 */
Std_ReturnType CanNm_GetUserData( const NetworkHandleType nmHandle, uint8* const nmUserDataPtr ){
	CANNM_VALIDATE_INIT(CANNM_SERVICEID_GETUSERDATA);

    CANNM_VALIDATE_CHANNEL(nmHandle, CANNM_SERVICEID_GETUSERDATA);
    uint8 channelIndex = CanNm_ConfigPtr->ChannelLookups[nmHandle];

	CANNM_VALIDATE_NOTNULL(nmUserDataPtr, CANNM_SERVICEID_GETUSERDATA, nmHandle);

	/** @req CANNM267 */

	const CanNm_ChannelType* ChannelConf = &CanNm_ConfigPtr->Channels[channelIndex];
	CanNm_Internal_ChannelType* ChannelInternal = &CanNm_Internal.Channels[channelIndex];
	uint8* sourceUserData = CanNm_Internal_GetUserDataPtr(ChannelConf, ChannelInternal->RxMessageSdu);
	uint8 userDataLength = CanNm_Internal_GetUserDataLength(ChannelConf);

	memcpy(nmUserDataPtr, sourceUserData, userDataLength);

	return E_OK;
}
#endif


/** Get node identifier out of the most recently received NM PDU. */
/** @req CANNM132 */
Std_ReturnType CanNm_GetNodeIdentifier( const NetworkHandleType nmHandle, uint8 * const nmNodeIdPtr ){
	CANNM_VALIDATE_INIT(CANNM_SERVICEID_GETNODEIDENTIFIER);
    CANNM_VALIDATE_CHANNEL(nmHandle, CANNM_SERVICEID_GETNODEIDENTIFIER);

    uint8 channelIndex = CanNm_ConfigPtr->ChannelLookups[nmHandle];
	CANNM_VALIDATE_NOTNULL(nmNodeIdPtr, CANNM_SERVICEID_GETNODEIDENTIFIER, nmHandle);

	/** @req CANNM269 */
	const CanNm_ChannelType* ChannelConf = &CanNm_ConfigPtr->Channels[channelIndex];
	CanNm_Internal_ChannelType* ChannelInternal = &CanNm_Internal.Channels[channelIndex];
	Std_ReturnType status = E_NOT_OK;

	if (ChannelConf->NidPosition == CANNM_PDU_OFF) { /** @req CANNM270 */
		/* status = NM_E_NOT_EXECUTED; */
	} else {
		*nmNodeIdPtr = ChannelInternal->RxMessageSdu[ChannelConf->NidPosition];
		status = E_OK;
	}
	return status;
}

/** Get node identifier configured for the local node. */
/** @req CANNM133 */
Std_ReturnType CanNm_GetLocalNodeIdentifier( const NetworkHandleType nmHandle, uint8 * const nmNodeIdPtr ){
	CANNM_VALIDATE_INIT(CANNM_SERVICEID_GETLOCALNODEIDENTIFIER);

    CANNM_VALIDATE_CHANNEL(nmHandle, CANNM_SERVICEID_GETLOCALNODEIDENTIFIER);
    uint8 channelIndex = CanNm_ConfigPtr->ChannelLookups[nmHandle];

	CANNM_VALIDATE_NOTNULL(nmNodeIdPtr, CANNM_SERVICEID_GETLOCALNODEIDENTIFIER, nmHandle);

	/** @req CANNM271 */

	const CanNm_ChannelType* ChannelConf = &CanNm_ConfigPtr->Channels[channelIndex];
	*nmNodeIdPtr = ChannelConf->NodeId;

	return E_OK;
}


#if (CANNM_NODE_DETECTION_ENABLED == STD_ON)
/** Set Repeat Message Request Bit for NM messages transmitted next on the bus. */
/** @req CANNM135  @req CANNM274 */
Std_ReturnType CanNm_RepeatMessageRequest( const NetworkHandleType nmHandle ){
	CANNM_VALIDATE_INIT(CANNM_SERVICEID_REPEATMESSAGEREQUEST);
    CANNM_VALIDATE_CHANNEL(nmHandle, CANNM_SERVICEID_REPEATMESSAGEREQUEST);

    uint8 channelIndex = CanNm_ConfigPtr->ChannelLookups[nmHandle];

	/** @req CANNM273 */
	const CanNm_ChannelType* ChannelConf = &CanNm_ConfigPtr->Channels[channelIndex];
	CanNm_Internal_ChannelType* ChannelInternal = &CanNm_Internal.Channels[channelIndex];



	Std_ReturnType status = E_NOT_OK;  /**< @req CANNM137 */

	if (ChannelConf->CbvPosition != CANNM_PDU_OFF) {
		if (ChannelInternal->State == NM_STATE_READY_SLEEP) {
			ChannelInternal->TxMessageSdu[ChannelConf->CbvPosition] = CANNM_CBV_REPEAT_MESSAGE_REQUEST;  /**< @req CANNM113 */
#if (CANNM_COM_USER_DATA_SUPPORT == STD_ON)
			ChannelInternal->SpontaneousTxMessageSdu[ChannelConf->CbvPosition] = CANNM_CBV_REPEAT_MESSAGE_REQUEST;
#endif
			CanNm_Internal_ReadySleep_to_RepeatMessage(ChannelConf, ChannelInternal);  /**< @req CANNM112 */
			status = E_OK;
		} else if (ChannelInternal->State == NM_STATE_NORMAL_OPERATION) {
			ChannelInternal->TxMessageSdu[ChannelConf->CbvPosition] = CANNM_CBV_REPEAT_MESSAGE_REQUEST;  /**< @req CANNM121 */
#if (CANNM_COM_USER_DATA_SUPPORT == STD_ON)
			ChannelInternal->SpontaneousTxMessageSdu[ChannelConf->CbvPosition] = CANNM_CBV_REPEAT_MESSAGE_REQUEST;
#endif
			CanNm_Internal_NormalOperation_to_RepeatMessage(ChannelConf, ChannelInternal);  /**< @req CANNM120 */
			status = E_OK;
		} else {
			//Nothing to be done
		}
	}
	return status;
}
#endif

/** Get the whole PDU data out of the most recently received NM message. */
/** @req CANNM138  @req CANNM276 */
Std_ReturnType CanNm_GetPduData( const NetworkHandleType nmHandle, uint8 * const nmPduDataPtr ){
	CANNM_VALIDATE_INIT(CANNM_SERVICEID_GETPDUDATA);
    CANNM_VALIDATE_CHANNEL(nmHandle, CANNM_SERVICEID_GETPDUDATA);
	CANNM_VALIDATE_NOTNULL(nmPduDataPtr, CANNM_SERVICEID_GETPDUDATA, nmHandle);

	uint8 channelIndex = CanNm_ConfigPtr->ChannelLookups[nmHandle];

	/** @req CANNM275 */

	const CanNm_ChannelType* ChannelConf = &CanNm_ConfigPtr->Channels[channelIndex];
	CanNm_Internal_ChannelType* ChannelInternal = &CanNm_Internal.Channels[channelIndex];

	memcpy(nmPduDataPtr, ChannelInternal->RxMessageSdu, ChannelConf->PduLength);

	return E_OK;
}

/** Returns the state and the mode of the network management. */
Std_ReturnType CanNm_GetState( const NetworkHandleType nmHandle, Nm_StateType * const nmStatePtr, Nm_ModeType * const nmModePtr ){
	CANNM_VALIDATE_INIT(CANNM_SERVICEID_GETSTATE);

    CANNM_VALIDATE_CHANNEL(nmHandle, CANNM_SERVICEID_GETSTATE);
    uint8 channelIndex = CanNm_ConfigPtr->ChannelLookups[nmHandle];

	CANNM_VALIDATE_NOTNULL(nmStatePtr, CANNM_SERVICEID_GETSTATE, nmHandle);
	CANNM_VALIDATE_NOTNULL(nmModePtr, CANNM_SERVICEID_GETSTATE, nmHandle);

	/** @req CANNM277 */

	CanNm_Internal_ChannelType* ChannelInternal = &CanNm_Internal.Channels[channelIndex];
	*nmStatePtr = ChannelInternal->State;
	*nmModePtr = ChannelInternal->Mode;

	return E_OK;
}

/** Request bus synchronization. */
Std_ReturnType CanNm_RequestBusSynchronization( const NetworkHandleType nmHandle ){

	/* @req CANNM279 */
	CANNM_VALIDATE_INIT(CANNM_SERVICEID_REQUESTBUSSYNCHRONIZATION);
    CANNM_VALIDATE_CHANNEL(nmHandle, CANNM_SERVICEID_REQUESTBUSSYNCHRONIZATION);
	// Not implemented
	(void)nmHandle;
	return E_NOT_OK;
}

/** Check if remote sleep indication takes place or not. */
Std_ReturnType CanNm_CheckRemoteSleepIndication( const NetworkHandleType nmHandle, boolean * const nmRemoteSleepIndPtr ){
	/* CANNM281 */
	CANNM_VALIDATE_INIT(CANNM_SERVICEID_CHECKREMOTESLEEPINDICATION);

    CANNM_VALIDATE_CHANNEL(nmHandle, CANNM_SERVICEID_CHECKREMOTESLEEPINDICATION);
	CANNM_VALIDATE_NOTNULL(nmRemoteSleepIndPtr, CANNM_SERVICEID_CHECKREMOTESLEEPINDICATION, nmHandle);

	(void)nmRemoteSleepIndPtr;
	(void)nmHandle;
	// Not implemented
	return E_NOT_OK;
}


// Functions called by CAN Interface
// ---------------------------------

/** This service confirms a previous successfully processed CAN transmit request.
  * This callback service is called by the CanIf and implemented by the CanNm. */


/* modified CanNm_TxConfirmation & CanNm_Internal_TransmitMessage:
	-when CanNm_Transmit is called by the PduR due to @req CANNM329 PduR_CanNmTxConfirmation must be called (when message was sent successfully)
	 => so CanNm_TxConfirmation was modified
	-due to the fact that destinction between cyclic messages and spontaneous massages is now a mondatory feature (spontaneous transmission => CanNm_Transmit)
	 CanNm_TxConfirmation has to decide what kind of message was lastly sent
	 => this is done by a new internal flag in "CanNm_Internal_ChannelType" => TransmissionStatus
	 => the flag is set by CanNm_Transmit (spontaneous message = CANNM_SPONTANEOUS_TRANSMISSION) & CanNm_internal_TransmitMessage (cyclic message = CANNM_ONGOING_TRANSMISSION)
	 => cyclic messages or spontaneous messages can only be sent when the flag has "CANNM_NO_TRANSMISSION" status
	 => when confirmation arrives (CanNm_TxConfirmation) the flag is set back to the lastly mentioned status
	-because CanNm_Transmit & CanNm_internal_TransmitMessage both can set the mentioned flag an exclusive area (EXCLUSIVE_AREA_0) is used (SchM_CanNm is now mandatory)
	-when CanNm_Transmit is enabled (CANNM_COM_USER_DATA_SUPPORT = STD_ON) CanNm_Internal_TransmitMessage needs to fetch the user Data from PduR by calling PduR_CanNmTriggerTransmit (@req CANNM328) */

void CanNm_TxConfirmation( PduIdType canNmTxPduId ){
	CANNM_VALIDATE_INIT_NORV(CANNM_SERVICEID_TXCONFIRMATION, canNmTxPduId); /** @req CANNM229 */ /** @req CANNM230 */
	CANNM_VALIDATE_PDUID_NORV(canNmTxPduId, CANNM_SERVICEID_TXCONFIRMATION); /** @req CANNM229 */ /** @req CANNM230 */

	/** @req CANNM283 */

	const CanNm_ChannelType* ChannelConf = &CanNm_ConfigPtr->Channels[canNmTxPduId];
	CanNm_Internal_ChannelType* ChannelInternal = &CanNm_Internal.Channels[canNmTxPduId];

#if (CANNM_COM_USER_DATA_SUPPORT == STD_ON)
	/* is there an ongoing transmit */
	if (ChannelInternal->TransmissionStatus == CANNM_SPONTANEOUS_TRANSMISSION)
	{
		/* transmit was ok */
		PduR_CanNmTxConfirmation(ChannelConf->CanNmTxPduId); /** @req CANNM329 */
		ChannelInternal->TransmissionStatus = CANNM_NO_TRANSMISSION;
	}
	else
#endif

	{
		if (ChannelInternal->Mode == NM_MODE_NETWORK) {
			CanNm_Internal_NetworkMode_to_NetworkMode(ChannelConf, ChannelInternal);  /**< @req CANNM099 */
		}
	}

    ChannelInternal->MessageTimeoutTimeLeft = 0; /** @req CANNM065 **/
}

/** This service indicates a successful reception of a received NM message to the
  * CanNm after passing all filters and validation checks.
  * This callback service is called by the CAN Interface and implemented by the CanNm. */
void CanNm_RxIndication( PduIdType RxPduId, PduInfoType *PduInfoPtr ){
	CANNM_VALIDATE_INIT_NORV(CANNM_SERVICEID_RXINDICATION, RxPduId); /** @req CANNM232 */ /** @req CANNM233 */
	CANNM_VALIDATE_NOTNULL_NORV(PduInfoPtr, CANNM_SERVICEID_RXINDICATION, RxPduId); /** @req CANNM232 */ /** @req CANNM233 */
	CANNM_VALIDATE_NOTNULL_NORV(PduInfoPtr->SduDataPtr, CANNM_SERVICEID_RXINDICATION, RxPduId); /** @req CANNM232 */ /** @req CANNM233 */
	CANNM_VALIDATE_PDUID_NORV(RxPduId, CANNM_SERVICEID_RXINDICATION); /** @req CANNM232 */ /** @req CANNM233 */

	const CanNm_ChannelType* ChannelConf = &CanNm_ConfigPtr->Channels[RxPduId];
	CanNm_Internal_ChannelType* ChannelInternal = &CanNm_Internal.Channels[RxPduId];

	memcpy(ChannelInternal->RxMessageSdu, PduInfoPtr->SduDataPtr, ChannelConf->PduLength);  /**< @req CANNM035 */

	boolean repeatMessageBitIndication = FALSE;
	if (ChannelConf->CbvPosition != CANNM_PDU_OFF) {
		uint8 cbv = ChannelInternal->RxMessageSdu[ChannelConf->CbvPosition];
		repeatMessageBitIndication = cbv & CANNM_CBV_REPEAT_MESSAGE_REQUEST;
	}

	if (ChannelInternal->Mode == NM_MODE_BUS_SLEEP) {
#if (CANNM_DEV_ERROR_DETECT == STD_ON)
		/* @req CANNM019 */
		CANNM_DET_REPORTERROR(CANNM_SERVICEID_RXINDICATION, CANNM_E_NET_START_IND, RxPduId); /** @req CANNM336 */
#endif
		CanNm_Internal_BusSleep_to_BusSleep(ChannelConf, ChannelInternal);  /**< @req CANNM127 */
	} else if (ChannelInternal->Mode == NM_MODE_PREPARE_BUS_SLEEP) {
		CanNm_Internal_PrepareBusSleep_to_RepeatMessage(ChannelConf, ChannelInternal,FALSE);  /**< @req CANNM124  @req CANNM315 */
	} else if (ChannelInternal->Mode == NM_MODE_NETWORK) {
		CanNm_Internal_NetworkMode_to_NetworkMode(ChannelConf, ChannelInternal);  /**< @req CANNM098 */
		if (repeatMessageBitIndication) {
			if (ChannelInternal->State == NM_STATE_READY_SLEEP) {
				CanNm_Internal_ReadySleep_to_RepeatMessage(ChannelConf, ChannelInternal);  /**< @req CANNM111 */
			} else if (ChannelInternal->State == NM_STATE_NORMAL_OPERATION) {
				CanNm_Internal_NormalOperation_to_RepeatMessage(ChannelConf, ChannelInternal);  /**< @req CANNM119 */
			} else {
				//Nothing to be done
			}
		}
	} else {
		//Nothing to be done
	}
#if (CANNM_PDU_RX_INDICATION_ENABLED == STD_ON)
	// TODO: call NM rx indication
#endif
}


/**
 * This function is used by the PduR to trigger a spontaneous transmission of an NM message
 * with the provided NM User Data
 */

/* when transmit is called trough PduR the given CanNmTxPduId must be matched to the internal Channel
	=> therefore each channel now needs the information which PduID matches to its internal configuration
	=> added CanNmTxPduId to "CanNm_ChannelType"
	=> this information must be filled at configuration-time
	the transmit-function seeks all channels for the given pdu and goes on when its found => E_NOT_OK when not */

Std_ReturnType CanNm_Transmit( PduIdType CanNmTxPduId, const PduInfoType *PduInfoPtr)
{
	(void)CanNmTxPduId; /* Avoid compiler warning - used depedning on config */
	(void)PduInfoPtr; /* Avoid compiler warning - used depedning on config */

#if (CANNM_COM_USER_DATA_SUPPORT == STD_ON) /** @req CANNM330 */

	/* only transmit when in
	 * repeat message state or normal operation state
	 * otherwise return E_NOT_OK;
	 */

	boolean found = FALSE;
	Std_ReturnType retVal = E_NOT_OK;

	/* read which channel is affected trough this CanNmTxPduId */
	uint8 affectedChannel;


	for (affectedChannel = 0; affectedChannel < CANNM_CHANNEL_COUNT; affectedChannel++)
	{
		if (CanNmTxPduId == CanNm_ConfigPtr->Channels[affectedChannel].CanNmTxPduId)
		{
			found = TRUE;
			break;
		}
	}

	/* is channel configured */
	CANNM_VALIDATE_CHANNEL(affectedChannel, CANNM_SERVICEID_TRANSMIT);
	/* is pdu ptr null? */
	CANNM_VALIDATE_NOTNULL(PduInfoPtr, CANNM_SERVICEID_TRANSMIT, affectedChannel); /* no requirement */
	/* is data ptr null */
	CANNM_VALIDATE_NOTNULL(PduInfoPtr->SduDataPtr, CANNM_SERVICEID_TRANSMIT, affectedChannel); /* no requirement */

	if (found == TRUE)
	{
		const CanNm_ChannelType* 	ChannelConf = &CanNm_ConfigPtr->Channels[affectedChannel];
		CanNm_Internal_ChannelType* ChannelInternal = &CanNm_Internal.Channels[affectedChannel];

		/* @req CANNM170*/
		if (((ChannelInternal->State == NM_STATE_REPEAT_MESSAGE) || (ChannelInternal->State == NM_STATE_NORMAL_OPERATION)) &&
		     (ChannelInternal->CommunicationEnabled))
		{
			SchM_Enter_CanNm(EXCLUSIVE_AREA_0);

			/* check if there is an ongoing uncommited transmission */
			if (ChannelInternal->TransmissionStatus == CANNM_NO_TRANSMISSION)
			{
				uint8* destUserData = CanNm_Internal_GetUserDataPtr(ChannelConf, ChannelInternal->SpontaneousTxMessageSdu);
				uint8 userDataLength = CanNm_Internal_GetUserDataLength(ChannelConf);

				ChannelInternal->TransmissionStatus = CANNM_SPONTANEOUS_TRANSMISSION;

				SchM_Exit_CanNm(EXCLUSIVE_AREA_0);

				memcpy(destUserData, PduInfoPtr->SduDataPtr, userDataLength);

				{
					PduInfoType pdu = {
							.SduDataPtr = ChannelInternal->SpontaneousTxMessageSdu,
							.SduLength = ChannelConf->PduLength,
					};

					retVal = CanIf_Transmit(ChannelConf->CanIfPduId, &pdu);

					if (retVal != E_OK) /* in order to avoid locking the possibility to send we have to reset the flag when CanIf_Tansmit fails */
					{
						ChannelInternal->TransmissionStatus = CANNM_NO_TRANSMISSION;
					}
				}
			}
			else
			{
				SchM_Exit_CanNm(EXCLUSIVE_AREA_0);
			}
		}
	}
	else
	{
		//TODO: no valid pdu - no requirement
	}

	return retVal;

#else

	return E_OK; /** @req CANNM333 */

#endif
}

/**
 * Set the NM Coordinator Sleep Ready bit in the Control Bit Vector
 * CURRENTLY UNSUPPORTED
 */
Std_ReturnType CanNm_SetSleepReadyBit( const NetworkHandleType nmHandle, const boolean nmSleepReadyBit)
{
	/* not supported */
	(void)nmHandle;
	(void)nmSleepReadyBit;
	return E_NOT_OK;
}


/** @req CANNM234 */
void CanNm_MainFunction( void ) {

	CANNM_VALIDATE_INIT_NORV(CANNM_SERVICEID_ARC_MAINFUNCTION,0); /** @req CANNM235 */ /**@req CANNM236 */

	uint8 channel;
	for (channel = 0; channel < CANNM_CHANNEL_COUNT; channel++) {

		/** @req CANNM108 */
		const CanNm_ChannelType* ChannelConf = &CanNm_ConfigPtr->Channels[channel];
		CanNm_Internal_ChannelType* ChannelInternal = &CanNm_Internal.Channels[channel];
		if (ChannelInternal->Mode == NM_MODE_NETWORK) {

		    if (ChannelInternal->State == NM_STATE_REPEAT_MESSAGE) {
		        CanNm_Internal_TickRepeatMessageTime(ChannelConf, ChannelInternal);  /**< @req CANNM102 */
		    }

		    /* @req CANNM174 */
		    if (ChannelInternal->CommunicationEnabled) {
			    CanNm_Internal_TickTimeoutTime(ChannelConf, ChannelInternal);
		    }

#if (CANNM_PASSIVE_MODE_ENABLED != STD_ON)

		    CanNm_Internal_TickTxTimeout(ChannelConf, ChannelInternal);
            /** @req CANNM161 */ /** @req CANNM162 */ /** @req CANNM072 */
			if (((ChannelInternal->State == NM_STATE_REPEAT_MESSAGE) || (ChannelInternal->State == NM_STATE_NORMAL_OPERATION)) &&
				 (ChannelInternal->CommunicationEnabled)) {
				/** @req CANNM051 @req CANNM032  @req CANNM087  @req CANNM100 */
				/** @req CANNM173 @req CANNM170 */

				CanNm_Internal_TickMessageCycleTime(ChannelConf, ChannelInternal);
			}
	#endif

		} else if (ChannelInternal->Mode == NM_MODE_PREPARE_BUS_SLEEP) {
			CanNm_Internal_TickWaitBusSleepTime(ChannelConf, ChannelInternal);  /**< @req CANNM115 */
		} else {
			//Nothing to be done
		}
	}
}

// Timer helpers
// -------------

static inline void CanNm_Internal_TickTimeoutTime( const CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal ) {

	if (ChannelConf->MainFunctionPeriod >= ChannelInternal->TimeoutTimeLeft) {
		ChannelInternal->TimeoutTimeLeft = 0;
		if (ChannelInternal->State == NM_STATE_REPEAT_MESSAGE) {

#if (CANNM_DEV_ERROR_DETECT == STD_ON) /** @req CANNM193 */
			/* @req CANNM019 */
			CANNM_DET_REPORTERROR(CANNM_SERVICEID_ARC_MAINFUNCTION, CANNM_E_NETWORK_TIMEOUT, 0); /* invalid due to requirement CANNM236 => main function should report instance id = channelId BUT we don't have the information here*/
#endif

#if (CANNM_COM_USER_DATA_SUPPORT == STD_ON)
			/* reset confirmation flag in order for beeing able to send again*/
			ChannelInternal->TransmissionStatus = CANNM_NO_TRANSMISSION;
#endif

			CanNm_Internal_RepeatMessage_to_RepeatMessage(ChannelConf, ChannelInternal);  /**< @req CANNM101 */
		} else if (ChannelInternal->State == NM_STATE_NORMAL_OPERATION) {
#if (CANNM_DEV_ERROR_DETECT == STD_ON) /** @req CANNM194 */
			/* @req CANNM019 */
			CANNM_DET_REPORTERROR(CANNM_SERVICEID_ARC_MAINFUNCTION, CANNM_E_NETWORK_TIMEOUT, 0); /* invalid due to requirement CANNM236 => main function should report instance id = channelId BUT we don't have the information here*/
#endif

#if (CANNM_COM_USER_DATA_SUPPORT == STD_ON)
			/* reset confirmation flag in order for beeing able to send again*/
			ChannelInternal->TransmissionStatus = CANNM_NO_TRANSMISSION;
#endif

			CanNm_Internal_NormalOperation_to_NormalOperation(ChannelConf, ChannelInternal);  /**< @req CANNM117 */
		} else if (ChannelInternal->State == NM_STATE_READY_SLEEP) {
			CanNm_Internal_ReadySleep_to_PrepareBusSleep(ChannelConf, ChannelInternal);  /**< @req CANNM109 */
		} else {
			//Nothing to be done
		}
	} else {
		ChannelInternal->TimeoutTimeLeft -= ChannelConf->MainFunctionPeriod;
	}
}

/** @req CANNM102 */
static inline void CanNm_Internal_TickRepeatMessageTime( const CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal ) {
	if (ChannelConf->MainFunctionPeriod >= ChannelInternal->RepeatMessageTimeLeft) {
		ChannelInternal->RepeatMessageTimeLeft = 0;
		if (ChannelInternal->State == NM_STATE_REPEAT_MESSAGE) {
			if (ChannelInternal->Requested) {
				CanNm_Internal_RepeatMessage_to_NormalOperation(ChannelConf, ChannelInternal);  /** @req CANNM103 */
			} else {
				CanNm_Internal_RepeatMessage_to_ReadySleep(ChannelConf, ChannelInternal); /** @req CANNM106 */
			}
		}
	} else {
		ChannelInternal->RepeatMessageTimeLeft -= ChannelConf->MainFunctionPeriod;
	}
}

/** @req CANNM115 */
static inline void CanNm_Internal_TickWaitBusSleepTime( const CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal ) {
	if (ChannelConf->MainFunctionPeriod >= ChannelInternal->WaitBusSleepTimeLeft) {
		ChannelInternal->WaitBusSleepTimeLeft = 0;
		if (ChannelInternal->Mode == NM_MODE_PREPARE_BUS_SLEEP) {
			CanNm_Internal_PrepareBusSleep_to_BusSleep(ChannelConf, ChannelInternal); /** @req CANNM088 */ /** @req CANNM115 */
		}
	} else {
		ChannelInternal->WaitBusSleepTimeLeft -= ChannelConf->MainFunctionPeriod;
	}
}
/* TxTimeout Processing */
static inline void CanNm_Internal_TickTxTimeout( const CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal ) {

    if (0 != ChannelInternal->MessageTimeoutTimeLeft ) {
        if (ChannelConf->MainFunctionPeriod >= ChannelInternal->MessageTimeoutTimeLeft ) {
            /** @req CANNM066 **/
            Nm_TxTimeoutException(ChannelConf->NmNetworkHandle);
            ChannelInternal->MessageTimeoutTimeLeft = 0;
        } else {
            ChannelInternal->MessageTimeoutTimeLeft -= ChannelConf->MainFunctionPeriod;
        }
    }

}

static inline void CanNm_Internal_TickMessageCycleTime( const CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal ) {

	if (ChannelConf->MainFunctionPeriod >= ChannelInternal->MessageCycleTimeLeft) {

			if (ChannelInternal->immediateModeActive == TRUE)
			{
				ChannelInternal->MessageCycleTimeLeft = ChannelConf->ImmediateNmCycleTime; /** @req CANNM334 */
			}
			else
			{
				ChannelInternal->MessageCycleTimeLeft = ChannelConf->MessageCycleTime;  /** @req CANNM040 */
			}

			/** @req CANNM087  @req CANNM100 */
			CanNm_Internal_TransmitMessage(ChannelConf, ChannelInternal); /** CANNM032 */ /* should transmit independently from state?! (CANNM032) */

	} else {
		ChannelInternal->MessageCycleTimeLeft -= ChannelConf->MainFunctionPeriod;
	}
}

// Message helpers
// ---------------

static inline void CanNm_Internal_TransmitMessage( const CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal ) {

	PduInfoType pdu = {
			.SduDataPtr = ChannelInternal->TxMessageSdu,
			.SduLength = ChannelConf->PduLength,
	};

	if (!ChannelInternal->CommunicationEnabled) {
		return;
	}

#if (CANNM_COM_USER_DATA_SUPPORT == STD_ON)

	SchM_Enter_CanNm(EXCLUSIVE_AREA_0);

	if (ChannelInternal->TransmissionStatus == CANNM_NO_TRANSMISSION)
	{
		PduInfoType userData;

		ChannelInternal->TransmissionStatus = CANNM_ONGOING_TRANSMISSION;

		SchM_Exit_CanNm(EXCLUSIVE_AREA_0);

		userData.SduDataPtr = CanNm_Internal_GetUserDataPtr(ChannelConf, ChannelInternal->TxMessageSdu);
		userData.SduLength = CanNm_Internal_GetUserDataLength(ChannelConf);

		/* TODO: What if returning not ok? */
		(void)PduR_CanNmTriggerTransmit(ChannelConf->CanNmTxPduId, &userData); /** @req CANNM328 */
#elif (CANNM_USER_DATA_ENABLED == STD_ON) /* & com_user... == OFF */

	SchM_Enter_CanNm(EXCLUSIVE_AREA_0);

	if (!ChannelInternal->IsUserDataSet)
	{

		ChannelInternal->IsUserDataSet = TRUE;

		SchM_Exit_CanNm(EXCLUSIVE_AREA_0);
#endif
		ChannelInternal->MessageTimeoutTimeLeft = ChannelConf->MessageTimeoutTime; /** @req CANNM064 */

		// TODO: what to do if Transmit fails?
		Std_ReturnType status = CanIf_Transmit(ChannelConf->CanIfPduId, &pdu);

		if (status != E_OK) {
			ChannelInternal->MessageTimeoutTimeLeft =0;
#ifdef HOST_TEST
			ReportErrorStatus();
#endif
		}
		/* check if we are sending immediateMessages */
		if (ChannelInternal->immediateModeActive == TRUE)
		{
			/* increment the immediateNmTransmissionsSent counter */
			ChannelInternal->immediateNmTransmissionsSent++;

			/* check if we already have reached the amount of messages which shall be send via immediateTransmit */
			if (ChannelInternal->immediateNmTransmissionsSent == ChannelConf->ImmediateNmTransmissions)
			{
				/* if so then deactivate the immediate mode again */
				ChannelInternal->immediateModeActive = FALSE;

				/* and wait offset before sending next regular (not immediate) pdu */
				ChannelInternal->MessageCycleTimeLeft = ChannelConf->MessageCycleOffsetTime; /** @req CANNM335 */
			}
		}

#if (CANNM_COM_USER_DATA_SUPPORT == STD_ON)

		/* set back the flag directly => this won't change the previous behaviour */
		/* flag is only needed to decide whether the last successfully sent message was a spontaneous transmit or a cyclic message */
		/* needed in function TxConfirmation */

		ChannelInternal->TransmissionStatus = CANNM_NO_TRANSMISSION; /* no need to lock */
	}
	else
	{
		SchM_Exit_CanNm(EXCLUSIVE_AREA_0);
	}
#elif (CANNM_USER_DATA_ENABLED == STD_ON)  /* and com_user_... == OFF */

		ChannelInternal->IsUserDataSet = FALSE;
	}
	else
	{
		SchM_Exit_CanNm(EXCLUSIVE_AREA_0);
	}
#endif

}

static inline uint8 CanNm_Internal_GetUserDataOffset( const CanNm_ChannelType* ChannelConf ) {
	uint8 userDataPos = 0;
	userDataPos += (ChannelConf->NidPosition == CANNM_PDU_OFF) ? 0 : 1;
	userDataPos += (ChannelConf->CbvPosition == CANNM_PDU_OFF) ? 0 : 1;
	return userDataPos;
}

static inline uint8* CanNm_Internal_GetUserDataPtr( const CanNm_ChannelType* ChannelConf, uint8* MessageSduPtr ) {
	uint8 userDataOffset = CanNm_Internal_GetUserDataOffset(ChannelConf);
	return &MessageSduPtr[userDataOffset];
}

static inline uint8 CanNm_Internal_GetUserDataLength( const CanNm_ChannelType* ChannelConf ) {
	uint8 userDataOffset = CanNm_Internal_GetUserDataOffset(ChannelConf);
	return ChannelConf->PduLength - userDataOffset;
}

static inline void CanNm_Internal_ClearCbv( const CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal ) {
	if (ChannelConf->CbvPosition != CANNM_PDU_OFF) {
		ChannelInternal->TxMessageSdu[ChannelConf->CbvPosition] = 0x00;
#if (CANNM_COM_USER_DATA_SUPPORT == STD_ON)
		ChannelInternal->SpontaneousTxMessageSdu[ChannelConf->CbvPosition] = 0x00;
#endif
	}
}

// Transition helpers
// ------------------

static inline void CanNm_Internal_PrepareBusSleep_to_RepeatMessage( const CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal ,boolean isNwReq ) {
	ChannelInternal->Mode = NM_MODE_NETWORK;
	ChannelInternal->State = NM_STATE_REPEAT_MESSAGE;

	if (ChannelConf->ImmediateNmTransmissions > 0 && isNwReq) /** @req CANNM005 CANNM334 */
	{
		ChannelInternal->MessageCycleTimeLeft = 0; /* begin transmission immediately */ /** @req CANNM334 */
		ChannelInternal->immediateNmTransmissionsSent = 0; /* reset counter of sent immediate messages */
		ChannelInternal->immediateModeActive = TRUE; /* activate immediate-transmission-mode */
	}
	else
	{
		ChannelInternal->MessageCycleTimeLeft = ChannelConf->MessageCycleOffsetTime + ChannelConf->MainFunctionPeriod;  /**< @req CANNM100 */
	}

    ChannelInternal->RepeatMessageTimeLeft = ChannelConf->RepeatMessageTime + ChannelConf->MainFunctionPeriod;
	ChannelInternal->TimeoutTimeLeft = ChannelConf->TimeoutTime + ChannelConf->MainFunctionPeriod;  /**< @req CANNM096 */

	// Notify 'Network Mode'
	Nm_NetworkMode(ChannelConf->NmNetworkHandle);  /**< @req CANNM097 */
	/**< @req CANNM166 */
#if (CANNM_STATE_CHANGE_IND_ENABLED == STD_ON)
	Nm_StateChangeNotification(ChannelConf->NmNetworkHandle, NM_STATE_PREPARE_BUS_SLEEP, NM_STATE_REPEAT_MESSAGE);
#endif

}

static inline void CanNm_Internal_PrepareBusSleep_to_BusSleep( const CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal ) {
	ChannelInternal->Mode = NM_MODE_BUS_SLEEP;
	ChannelInternal->State = NM_STATE_BUS_SLEEP;
	// Notify 'Bus-Sleep Mode'
	Nm_BusSleepMode(ChannelConf->NmNetworkHandle);  /**< @req CANNM126 */ /** @req CANNM324 */
	/**< @req CANNM166 */
#if (CANNM_STATE_CHANGE_IND_ENABLED == STD_ON)
	Nm_StateChangeNotification(ChannelConf->NmNetworkHandle, NM_STATE_PREPARE_BUS_SLEEP, NM_STATE_BUS_SLEEP);
#endif
}

static inline void CanNm_Internal_BusSleep_to_RepeatMessage( const CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal,boolean isNwReq ) {
	ChannelInternal->Mode = NM_MODE_NETWORK;
	ChannelInternal->State = NM_STATE_REPEAT_MESSAGE;

	if (ChannelConf->ImmediateNmTransmissions > 0 && isNwReq) /** @req CANNM005 */
	{
		ChannelInternal->MessageCycleTimeLeft = 0; /* begin transmission immediately */ /** @req CANNM334 */
		ChannelInternal->immediateNmTransmissionsSent = 0; /* reset counter of sent immediate messages */
		ChannelInternal->immediateModeActive = TRUE; /* activate immediate-transmission-mode */
	}
	else
	{
		ChannelInternal->MessageCycleTimeLeft = ChannelConf->MessageCycleOffsetTime + ChannelConf->MainFunctionPeriod;  /**< @req CANNM100 */
	}

    ChannelInternal->TimeoutTimeLeft = ChannelConf->TimeoutTime + ChannelConf->MainFunctionPeriod ;  /**< @req CANNM096 */
    ChannelInternal->RepeatMessageTimeLeft = ChannelConf->RepeatMessageTime + ChannelConf->MainFunctionPeriod;
	// Notify 'Network Mode'
	Nm_NetworkMode(ChannelConf->NmNetworkHandle);  /**< @req CANNM097 */ /** @req CANNM324 */
	/**< @req CANNM166 */
#if (CANNM_STATE_CHANGE_IND_ENABLED == STD_ON)
	Nm_StateChangeNotification(ChannelConf->NmNetworkHandle, NM_STATE_BUS_SLEEP, NM_STATE_REPEAT_MESSAGE);
#endif

}
static inline void CanNm_Internal_BusSleep_to_BusSleep( const CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal ) {
	// Notify 'Network Start'
	Nm_NetworkStartIndication(ChannelConf->NmNetworkHandle);  /**< @req CANNM127 */ /** @req CANNM324 */
	(void) ChannelInternal; //Just to avoid 715 PC-Lint warning about not used.
}

static inline void CanNm_Internal_RepeatMessage_to_RepeatMessage( const CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal ) {
	ChannelInternal->TimeoutTimeLeft = ChannelConf->TimeoutTime;  /**< @req CANNM101 */
}
static inline void CanNm_Internal_RepeatMessage_to_ReadySleep( const CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal ) {
	ChannelInternal->Mode = NM_MODE_NETWORK;
	ChannelInternal->State = NM_STATE_READY_SLEEP;
#if (CANNM_NODE_DETECTION_ENABLED == STD_ON)
	/** @req CANNM107 */
	CanNm_Internal_ClearCbv(ChannelConf, ChannelInternal);
#endif
	/**< @req CANNM166 */
#if (CANNM_STATE_CHANGE_IND_ENABLED == STD_ON)
	Nm_StateChangeNotification(ChannelConf->NmNetworkHandle, NM_STATE_REPEAT_MESSAGE, NM_STATE_READY_SLEEP);
#endif
}
static inline void CanNm_Internal_RepeatMessage_to_NormalOperation( const CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal ) {
	ChannelInternal->Mode = NM_MODE_NETWORK;
	ChannelInternal->State = NM_STATE_NORMAL_OPERATION;
#if (CANNM_NODE_DETECTION_ENABLED == STD_ON)
	/** @req CANNM107 */
	CanNm_Internal_ClearCbv(ChannelConf, ChannelInternal);
#endif
	/**< @req CANNM166 */
#if (CANNM_STATE_CHANGE_IND_ENABLED == STD_ON)
	Nm_StateChangeNotification(ChannelConf->NmNetworkHandle, NM_STATE_REPEAT_MESSAGE, NM_STATE_NORMAL_OPERATION);
#endif
}

static inline void CanNm_Internal_NormalOperation_to_RepeatMessage( const CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal ) {
	ChannelInternal->Mode = NM_MODE_NETWORK;
	ChannelInternal->State = NM_STATE_REPEAT_MESSAGE;
	ChannelInternal->RepeatMessageTimeLeft = ChannelConf->RepeatMessageTime + ChannelConf->MainFunctionPeriod;
	ChannelInternal->MessageCycleTimeLeft = ChannelConf->MessageCycleOffsetTime + ChannelConf->MainFunctionPeriod;  /**< @req CANNM100 */
	/**< @req CANNM166 */
#if (CANNM_STATE_CHANGE_IND_ENABLED == STD_ON)
	Nm_StateChangeNotification(ChannelConf->NmNetworkHandle, NM_STATE_NORMAL_OPERATION, NM_STATE_REPEAT_MESSAGE);
#endif
}
static inline void CanNm_Internal_NormalOperation_to_ReadySleep( const CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal ) {
	ChannelInternal->Mode = NM_MODE_NETWORK;
	ChannelInternal->State = NM_STATE_READY_SLEEP;
	(void) ChannelConf; //Just to avoid 715 PC-Lint warning about not used.
	/**< @req CANNM166 */
#if (CANNM_STATE_CHANGE_IND_ENABLED == STD_ON)
	Nm_StateChangeNotification(ChannelConf->NmNetworkHandle, NM_STATE_NORMAL_OPERATION, NM_STATE_READY_SLEEP);
#endif
}
static inline void CanNm_Internal_NormalOperation_to_NormalOperation( const CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal ) {
	ChannelInternal->TimeoutTimeLeft = ChannelConf->TimeoutTime;  /**< @req CANNM117 */
}

static inline void CanNm_Internal_ReadySleep_to_NormalOperation( const CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal ) {
	ChannelInternal->Mode = NM_MODE_NETWORK;
	ChannelInternal->State = NM_STATE_NORMAL_OPERATION;
	ChannelInternal->MessageCycleTimeLeft = ChannelConf->MessageCycleOffsetTime + ChannelConf->MainFunctionPeriod;  /**< @req CANNM116 */
	/**< @req CANNM166 */
#if (CANNM_STATE_CHANGE_IND_ENABLED == STD_ON)
	Nm_StateChangeNotification(ChannelConf->NmNetworkHandle, NM_STATE_READY_SLEEP, NM_STATE_NORMAL_OPERATION);
#endif
}
static inline void CanNm_Internal_ReadySleep_to_PrepareBusSleep( const CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal ) {
	ChannelInternal->Mode = NM_MODE_PREPARE_BUS_SLEEP;
	ChannelInternal->State = NM_STATE_PREPARE_BUS_SLEEP;
	ChannelInternal->WaitBusSleepTimeLeft = ChannelConf->WaitBusSleepTime;
	// Notify 'Prepare Bus-Sleep Mode'
	Nm_PrepareBusSleepMode(ChannelConf->NmNetworkHandle);  /**< @req CANNM114 */ /** @req CANNM324 */
	/**< @req CANNM166 */
#if (CANNM_STATE_CHANGE_IND_ENABLED == STD_ON)
	Nm_StateChangeNotification(ChannelConf->NmNetworkHandle, NM_STATE_READY_SLEEP, NM_STATE_PREPARE_BUS_SLEEP);
#endif
}
static inline void CanNm_Internal_ReadySleep_to_RepeatMessage( const CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal ) {
	ChannelInternal->Mode = NM_MODE_NETWORK;
	ChannelInternal->State = NM_STATE_REPEAT_MESSAGE;
	ChannelInternal->RepeatMessageTimeLeft = ChannelConf->RepeatMessageTime + ChannelConf->MainFunctionPeriod;
	ChannelInternal->MessageCycleTimeLeft = ChannelConf->MessageCycleOffsetTime + ChannelConf->MainFunctionPeriod;  /**< @req CANNM100 */
	/**< @req CANNM166 */
#if (CANNM_STATE_CHANGE_IND_ENABLED == STD_ON)
	Nm_StateChangeNotification(ChannelConf->NmNetworkHandle, NM_STATE_READY_SLEEP,	NM_STATE_REPEAT_MESSAGE);
#endif
}

static inline void CanNm_Internal_NetworkMode_to_NetworkMode( const CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal ) {
	ChannelInternal->TimeoutTimeLeft = ChannelConf->TimeoutTime + ChannelConf->MainFunctionPeriod;  /**< @req CANNM098 @req CANNM099 */

}
