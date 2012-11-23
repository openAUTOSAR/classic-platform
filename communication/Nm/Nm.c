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


#if defined(USE_CANNM)
#include "CanNm.h"
#endif
#include "Nm.h"

/* Globally fulfilled requirements */
/** @req NM006 */
/** @req NM026 */

const Nm_ConfigType* Nm_ConfigPtr;

/** Initializes the NM Interface. */
void Nm_Init( const Nm_ConfigType * const nmConfigPtr ){
	Nm_ConfigPtr = nmConfigPtr;
}

/** This service returns the version information of this module */
void Nm_GetVersionInfo( Std_VersionInfoType* nmVerInfoPtr ){}

/** This function calls the <BusNm>_PassiveStartUp function
  * (e.g. CanNm_PassiveStartUp function is called if channel is configured as CAN). */
Nm_ReturnType Nm_PassiveStartUp( const NetworkHandleType NetworkHandle ){
	const Nm_ChannelType* ChannelConf = &Nm_ConfigPtr->Channels[NetworkHandle];

	switch(ChannelConf->BusType) {
#if defined(USE_CANNM)
		case NM_BUSNM_CANNM:
			return CanNm_PassiveStartUp(ChannelConf->BusNmNetworkHandle);
#endif
#if defined(USE_FRNM)
		case NM_BUSNM_FRNM:							\
			return FrNm_PassiveStartUp(Channelconf->BusNmNetworkHandle);
#endif
#if defined(USE_LINNM)
		case NM_BUSNM_LINNM:							\
			return LinNm_PassiveStartUp(Channelconf->BusNmNetworkHandle);
#endif
		default: return NM_E_NOT_OK;
	}
}

/** This function calls the <BusNm>_NetworkRequest
  * (e.g. CanNm_NetworkRequest function is called if channel is configured as CAN). */
Nm_ReturnType Nm_NetworkRequest( const NetworkHandleType NetworkHandle ){
	const Nm_ChannelType* ChannelConf = &Nm_ConfigPtr->Channels[NetworkHandle];

	switch(ChannelConf->BusType) {
#if defined(USE_CANNM)
		case NM_BUSNM_CANNM:
			return CanNm_NetworkRequest(ChannelConf->BusNmNetworkHandle);
#endif
#if defined(USE_FRNM)
		case NM_BUSNM_FRNM:
			return FrNm_NetworkRequest(Channelconf->BusNmNetworkHandle);
#endif
#if defined(USE_LINNM)
		case NM_BUSNM_LINNM:
			return LinNm_NetworkRequest(Channelconf->BusNmNetworkHandle);
#endif
		default: return NM_E_NOT_OK;
	}
}

/** This function calls the <BusNm>_NetworkRelease bus specific function
  * (e.g. CanNm_NetworkRelease function is called if channel is configured as CAN). */
Nm_ReturnType Nm_NetworkRelease( const NetworkHandleType NetworkHandle ){
	const Nm_ChannelType* ChannelConf = &Nm_ConfigPtr->Channels[NetworkHandle];

	switch(ChannelConf->BusType) {
#if defined(USE_CANNM)
		case NM_BUSNM_CANNM:
			return CanNm_NetworkRelease(ChannelConf->BusNmNetworkHandle);
#endif
#if defined(USE_FRNM)
		case NM_BUSNM_FRNM:
			return FrNm_NetworkRelease(Channelconf->BusNmNetworkHandle);
#endif
#if defined(USE_LINNM)
		case NM_BUSNM_LINNM:
			return LinNm_NetworkRelease(Channelconf->BusNmNetworkHandle);
#endif
		default: return NM_E_NOT_OK;
	}
}

/** This function calls the CanNm_NetworkRelease bus specific function, to disable
  * the NM PDU transmission ability due to a ISO14229 Communication Control (28hex) service */
Nm_ReturnType Nm_DisableCommunication( const NetworkHandleType NetworkHandle ){
	const Nm_ChannelType* ChannelConf = &Nm_ConfigPtr->Channels[NetworkHandle];

	switch(ChannelConf->BusType) {
#if defined(USE_CANNM)
		case NM_BUSNM_CANNM:
			return CanNm_DisableCommunication(ChannelConf->BusNmNetworkHandle);
#endif
#if defined(USE_FRNM)
		case NM_BUSNM_FRNM:
			return FrNm_DisableCommunication(Channelconf->BusNmNetworkHandle);
#endif
#if defined(USE_LINNM)
		case NM_BUSNM_LINNM:
			return LinNm_DisableCommunication(Channelconf->BusNmNetworkHandle);
#endif
		default: return NM_E_NOT_OK;
	}
}

/** This function calls the CanNm_NetworkRequest bus specific function, to enable
  * the NM PDU transmission ability due to a ISO14229 Communication Control (28hex) service */
Nm_ReturnType Nm_EnableCommunication( const NetworkHandleType NetworkHandle ){
	const Nm_ChannelType* ChannelConf = &Nm_ConfigPtr->Channels[NetworkHandle];

	switch(ChannelConf->BusType) {
#if defined(USE_CANNM)
		case NM_BUSNM_CANNM:
			return CanNm_EnableCommunication(ChannelConf->BusNmNetworkHandle);
#endif
#if defined(USE_FRNM)
		case NM_BUSNM_FRNM:
			return FrNm_EnableCommunication(Channelconf->BusNmNetworkHandle);
#endif
#if defined(USE_LINNM)
		case NM_BUSNM_LINNM:
			return LinNm_EnableCommunication(Channelconf->BusNmNetworkHandle);
#endif
		default: return NM_E_NOT_OK;
	}
}

/** Set user data for NM messages transmitted next on the bus.
  * For that purpose <BusNm>_SetUserData shall be called
  * (e.g. CanNm_SetUserData function is called if channel is configured as CAN). */
#if (NM_USER_DATA_ENABLED == STD_ON)
Nm_ReturnType Nm_SetUserData( const NetworkHandleType NetworkHandle, const uint8 * const nmUserDataPtr ){
	const Nm_ChannelType* ChannelConf = &Nm_ConfigPtr->Channels[NetworkHandle];

	switch(ChannelConf->BusType) {
#if defined(USE_CANNM)
		case NM_BUSNM_CANNM:
			return CanNm_SetUserData(ChannelConf->BusNmNetworkHandle, nmUserDataPtr);
#endif
#if defined(USE_FRNM)
		case NM_BUSNM_FRNM:
			return FrNm_SetUserData(Channelconf->BusNmNetworkHandle, nmUserDataPtr);
#endif
#if defined(USE_LINNM)
		case NM_BUSNM_LINNM:
			return LinNm_SetUserData(Channelconf->BusNmNetworkHandle, nmUserDataPtr);
#endif
		default: return NM_E_NOT_OK;
	}
}
#endif

/** Get user data out of the last successfully received NM message.
  * For that purpose <BusNm>_GetUserData shall be called
  * (e.g. CanNm_GetUserData function is called if channel is configured as CAN). */
#if (NM_USER_DATA_ENABLED == STD_ON)
Nm_ReturnType Nm_GetUserData( const NetworkHandleType NetworkHandle, uint8 * const nmUserDataPtr, uint8 * const nmNodeIdPtr ){
	/* For some reason the signature of this service differs from its busNm equivalents... */
	Nm_ReturnType userDataRet = NM_E_NOT_OK;
	Nm_ReturnType nodeIdRet = NM_E_NOT_OK;
	const Nm_ChannelType* ChannelConf = &Nm_ConfigPtr->Channels[NetworkHandle];
	switch(ChannelConf->BusType) {
#if defined(USE_CANNM)
	case NM_BUSNM_CANNM:
		userDataRet = CanNm_GetUserData(ChannelConf->BusNmNetworkHandle, nmUserDataPtr);
		nodeIdRet = CanNm_GetNodeIdentifier(ChannelConf->BusNmNetworkHandle, nmNodeIdPtr);
		break;
#endif
#if defined(USE_FRNM)
	case NM_BUSNM_FRNM:
		userDataRet = FrNm_GetUserData(ChannelConf->BusNmNetworkHandle, nmUserDataPtr);
		nodeIdRet = FrNm_GetNodeIdentifier(ChannelConf->BusNmNetworkHandle, nmNodeIdPtr);
		break;
#endif
#if defined(USE_LINNM)
	case NM_BUSNM_LINNM:
		userDataRet = LinNm_GetUserData(ChannelConf->BusNmNetworkHandle, nmUserDataPtr);
		nodeIdRet = LinNm_GetNodeIdentifier(ChannelConf->BusNmNetworkHandle, nmNodeIdPtr);
		break;
#endif
	default:
		break;
	}

	if( (NM_E_OK != userDataRet) || ( NM_E_OK != nodeIdRet )) {
		return NM_E_NOT_OK;
	} else {
		return NM_E_OK;
	}
}
#endif

/** Get the whole PDU data out of the most recently received NM message.
  * For that purpose CanNm_GetPduData shall be called. */
#if ((NM_NODE_ID_ENABLED == STD_ON) || (NM_NODE_DETECTION_ENABLED == STD_ON) || (NM_USER_DATA_ENABLED == STD_ON))
Nm_ReturnType Nm_GetPduData( const NetworkHandleType NetworkHandle, uint8 * const nmPduData ){
	const Nm_ChannelType* ChannelConf = &Nm_ConfigPtr->Channels[NetworkHandle];

	switch(ChannelConf->BusType) {
#if defined(USE_CANNM)
		case NM_BUSNM_CANNM:
			return CanNm_GetPduData(ChannelConf->BusNmNetworkHandle, nmPduData);
#endif
#if defined(USE_FRNM)
		case NM_BUSNM_FRNM:
			return FrNm_GetPduData(Channelconf->BusNmNetworkHandle, nmPduData);
#endif
#if defined(USE_LINNM)
		case NM_BUSNM_LINNM:
			return LinNm_GetPduData(Channelconf->BusNmNetworkHandle, nmPduData);
#endif
		default: return NM_E_NOT_OK;
	}
}
#endif

/** Set Repeat Message Request Bit for NM messages transmitted next on the bus.
  * For that purpose <BusNm>_RepeatMessageRequest shall be called
  * (e.g. CanNm_RepeatMessageRequest function is called if channel is configured as CAN) */
#if (NM_NODE_DETECTION_ENABLED == STD_ON)
Nm_ReturnType Nm_RepeatMessageRequest( const NetworkHandleType NetworkHandle ){
	const Nm_ChannelType* ChannelConf = &Nm_ConfigPtr->Channels[NetworkHandle];

	switch(ChannelConf->BusType) {
#if defined(USE_CANNM)
		case NM_BUSNM_CANNM:
			return CanNm_RepeatMessageRequest(ChannelConf->BusNmNetworkHandle);
#endif
#if defined(USE_FRNM)
		case NM_BUSNM_FRNM:
			return FrNm_RepeatMessageRequest(Channelconf->BusNmNetworkHandle);
#endif
#if defined(USE_LINNM)
		case NM_BUSNM_LINNM:
			return LinNm_RepeatMessageRequest(Channelconf->BusNmNetworkHandle);
#endif
		default: return NM_E_NOT_OK;
	}
}
#endif

/** Get node identifier out of the last successfully received NM-message.
  * The function <BusNm>_GetNodeIdentifier shall be called. */
#if (NM_NODE_ID_ENABLED == STD_ON)
Nm_ReturnType Nm_GetNodeIdentifier( const NetworkHandleType NetworkHandle, uint8 * const nmNodeIdPtr ){
	const Nm_ChannelType* ChannelConf = &Nm_ConfigPtr->Channels[NetworkHandle];

	switch(ChannelConf->BusType) {
#if defined(USE_CANNM)
		case NM_BUSNM_CANNM:
			return CanNm_GetNodeIdentifier(ChannelConf->BusNmNetworkHandle, nmNodeIdPtr);
#endif
#if defined(USE_FRNM)
		case NM_BUSNM_FRNM:
			return FrNm_GetNodeIdentifier(Channelconf->BusNmNetworkHandle, nmNodeIdPtr);
#endif
#if defined(USE_LINNM)
		case NM_BUSNM_LINNM:
			return LinNm_GetNodeIdentifier(Channelconf->BusNmNetworkHandle, nmNodeIdPtr);
#endif
		default: return NM_E_NOT_OK;
	}
}

/** Get node identifier configured for the local node.
  * For that purpose <BusNm>_GetLocalNodeIdentifier shall be called
  * (e.g. CanNm_GetLocalNodeIdentifier function is called if channel is configured as CAN). */
Nm_ReturnType Nm_GetLocalNodeIdentifier( const NetworkHandleType NetworkHandle, uint8 * const nmNodeIdPtr ){
	const Nm_ChannelType* ChannelConf = &Nm_ConfigPtr->Channels[NetworkHandle];
 
	switch(ChannelConf->BusType) {
#if defined(USE_CANNM)
		case NM_BUSNM_CANNM:
			return CanNm_GetLocalNodeIdentifier(ChannelConf->BusNmNetworkHandle, nmNodeIdPtr);
#endif
#if defined(USE_FRNM)
		case NM_BUSNM_FRNM:
			return FrNm_GetLocalNodeIdentifier(Channelconf->BusNmNetworkHandle, nmNodeIdPtr);
#endif
#if defined(USE_LINNM)
		case NM_BUSNM_LINNM:
			return LinNm_GetLocalNodeIdentifier(Channelconf->BusNmNetworkHandle, nmNodeIdPtr);
#endif
		default: return NM_E_NOT_OK;
	}
}
#endif

/** Check if remote sleep indication takes place or not. This in turn calls the
  * <BusNm>_CheckRemoteSleepIndication for the bus specific NM layer
  * (e.g. CanNm_CheckRemoteSleepIndication function is called if channel is configured as CAN). */
Nm_ReturnType Nm_CheckRemoteSleepIndication( const NetworkHandleType NetworkHandle, boolean * const nmRemoteSleepIndPtr ){
	return E_NOT_OK;
}

/** Returns the state of the network management. This function in turn calls the
  * <BusNm>_GetState function (e.g. CanNm_GetState function is called if channel is configured as CAN). */
Nm_ReturnType Nm_GetState( const NetworkHandleType NetworkHandle, Nm_StateType* const nmStatePtr, Nm_ModeType* const nmModePtr ){
	const Nm_ChannelType* ChannelConf = &Nm_ConfigPtr->Channels[NetworkHandle];

	switch(ChannelConf->BusType) {
#if defined(USE_CANNM)
		case NM_BUSNM_CANNM:
			return CanNm_GetState(ChannelConf->BusNmNetworkHandle, nmStatePtr, nmModePtr);
#endif
#if defined(USE_FRNM)
		case NM_BUSNM_FRNM:
			return FrNm_GetState(Channelconf->BusNmNetworkHandle, nmStatePtr, nmModePtr);
#endif
#if defined(USE_LINNM)
		case NM_BUSNM_LINNM:
			return LinNm_GetState(Channelconf->BusNmNetworkHandle, nmStatePtr, nmModePtr);
#endif
		default: return NM_E_NOT_OK;
	}
}

/** Notification that a NM-message has been received in the Bus-Sleep Mode, what
  * indicates that some nodes in the network have already entered the Network Mode.
  * The callback function shall start the network management state machine. */
/** @req NM012.partially.1 */
void Nm_NetworkStartIndication( const NetworkHandleType NetworkHandle ){
	const Nm_ChannelType* ChannelConf = &Nm_ConfigPtr->Channels[NetworkHandle];
	ComM_Nm_NetworkStartIndication(ChannelConf->ComMNetworkHandle);
}

/** Notification that the network management has entered Network Mode. The
  * callback function shall enable transmission of application messages. */
/** @req NM012.partially.2 */
void Nm_NetworkMode( const NetworkHandleType NetworkHandle ){
	const Nm_ChannelType* ChannelConf = &Nm_ConfigPtr->Channels[NetworkHandle];
	ComM_Nm_NetworkMode(ChannelConf->ComMNetworkHandle);
}

/** Notification that the network management has entered Prepare Bus-Sleep Mode.
  * The callback function shall disable transmission of application messages. */
/** @req NM012.partially.3 */
void Nm_PrepareBusSleepMode( const NetworkHandleType NetworkHandle ){
	const Nm_ChannelType* ChannelConf = &Nm_ConfigPtr->Channels[NetworkHandle];
	ComM_Nm_PrepareBusSleepMode(ChannelConf->ComMNetworkHandle);
}

/** Notification that the network management has entered Bus-Sleep Mode. */
/** @req NM012.partially.4 */
void Nm_BusSleepMode( const NetworkHandleType NetworkHandle ){
	const Nm_ChannelType* ChannelConf = &Nm_ConfigPtr->Channels[NetworkHandle];
	ComM_Nm_BusSleepMode(ChannelConf->ComMNetworkHandle);
}

/** Notification that a NM message has been received. */
void Nm_PduRxIndication( const NetworkHandleType NetworkHandle ){}

/** Notification that the CAN Generic NM state has changed. */
void Nm_StateChangeNotification(
		const NetworkHandleType nmNetworkHandle,
		const Nm_StateType nmPreviousState,
		const Nm_StateType nmCurrentState ){}

/** This function implements the processes of the NM Interface, which need a fix
  * cyclic scheduling. This function is supplied for the NM coordinator functionality
  * (Nm020). However, specific implementation may not need it (Nm093) */
/** @req 121 */
void Nm_MainFunction(){}

