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

#include "CanNm.h"
#include "Nm.h"
#include "Nm_Internal.h"

const Nm_ConfigType* Nm_ConfigPtr;

/** Initializes the NM Interface. */
void Nm_Init( Nm_ConfigType * const nmConfigPtr ){
	Nm_ConfigPtr = nmConfigPtr;
}

/** This service returns the version information of this module */
void Nm_GetVersionInfo( Std_VersionInfoType* nmVerInfoPtr ){}

/** This function calls the <BusNm>_PassiveStartUp function
  * (e.g. CanNm_PassiveStartUp function is called if channel is configured as CAN). */
Nm_ReturnType Nm_PassiveStartUp( const NetworkHandleType NetworkHandle ){
	const Nm_ChannelType* ChannelConf = &Nm_ConfigPtr->Channels[NetworkHandle];
	NM_CALL_BUSNM(PassiveStartUp, ChannelConf);
}

/** This function calls the <BusNm>_NetworkRequest
  * (e.g. CanNm_NetworkRequest function is called if channel is configured as CAN). */
Nm_ReturnType Nm_NetworkRequest( const NetworkHandleType NetworkHandle ){
	const Nm_ChannelType* ChannelConf = &Nm_ConfigPtr->Channels[NetworkHandle];
	NM_CALL_BUSNM(NetworkRequest, ChannelConf)
}

/** This function calls the <BusNm>_NetworkRelease bus specific function
  * (e.g. CanNm_NetworkRelease function is called if channel is configured as CAN). */
Nm_ReturnType Nm_NetworkRelease( const NetworkHandleType NetworkHandle ){
	const Nm_ChannelType* ChannelConf = &Nm_ConfigPtr->Channels[NetworkHandle];
	NM_CALL_BUSNM(NetworkRelease, ChannelConf)
}

/** This function calls the CanNm_NetworkRelease bus specific function, to disable
  * the NM PDU transmission ability due to a ISO14229 Communication Control (28hex) service */
Nm_ReturnType Nm_DisableCommunication( const NetworkHandleType NetworkHandle ){
	const Nm_ChannelType* ChannelConf = &Nm_ConfigPtr->Channels[NetworkHandle];
	NM_CALL_BUSNM(DisableCommunication, ChannelConf)
}

/** This function calls the CanNm_NetworkRequest bus specific function, to enable
  * the NM PDU transmission ability due to a ISO14229 Communication Control (28hex) service */
Nm_ReturnType Nm_EnableCommunication( const NetworkHandleType NetworkHandle ){
	const Nm_ChannelType* ChannelConf = &Nm_ConfigPtr->Channels[NetworkHandle];
	NM_CALL_BUSNM(EnableCommunication, ChannelConf)
}

/** Set user data for NM messages transmitted next on the bus.
  * For that purpose <BusNm>_SetUserData shall be called
  * (e.g. CanNm_SetUserData function is called if channel is configured as CAN). */
Nm_ReturnType Nm_SetUserData( const NetworkHandleType NetworkHandle, const uint8 * const nmUserDataPtr ){
	const Nm_ChannelType* ChannelConf = &Nm_ConfigPtr->Channels[NetworkHandle];
	NM_CALL_BUSNM(SetUserData, ChannelConf, nmUserDataPtr)
}

/** Get user data out of the last successfully received NM message.
  * For that purpose <BusNm>_GetUserData shall be called
  * (e.g. CanNm_GetUserData function is called if channel is configured as CAN). */
Nm_ReturnType Nm_GetUserData( const NetworkHandleType NetworkHandle, uint8 * const nmUserDataPtr, uint8 * const nmNodeIdPtr ){
	/* For some reason the signature of this service differs from its busNm equivalents... */
	const Nm_ChannelType* ChannelConf = &Nm_ConfigPtr->Channels[NetworkHandle];
	switch(ChannelConf->BusType) {
#if defined(USE_CANNM)
	case NM_BUSNM_CANNM:
		NM_BUSNM_GET_USERDATA_AND_NODEID( CanNm, ChannelConf, nmUserDataPtr, nmNodeIdPtr )
#endif
#if defined(USE_FRNM)
	case NM_BUSNM_FRNM:
		NM_BUSNM_GET_USERDATA_AND_NODEID( FrNm, ChannelConf, nmUserDataPtr, nmNodeIdPtr )
#endif
#if defined(USE_LINNM)
	case NM_BUSNM_LINNM:
		NM_BUSNM_GET_USERDATA_AND_NODEID( LinNm, ChannelConf, nmUserDataPtr, nmNodeIdPtr )
#endif
	default: return NM_E_NOT_OK;
	}
}

/** Get the whole PDU data out of the most recently received NM message.
  * For that purpose CanNm_GetPduData shall be called. */
Nm_ReturnType Nm_GetPduData( const NetworkHandleType NetworkHandle, uint8 * const nmPduData ){
	const Nm_ChannelType* ChannelConf = &Nm_ConfigPtr->Channels[NetworkHandle];
	NM_CALL_BUSNM(GetPduData, ChannelConf, nmPduData)
}

/** Set Repeat Message Request Bit for NM messages transmitted next on the bus.
  * For that purpose <BusNm>_RepeatMessageRequest shall be called
  * (e.g. CanNm_RepeatMessageRequest function is called if channel is configured as CAN) */
Nm_ReturnType Nm_RepeatMessageRequest( const NetworkHandleType NetworkHandle ){
	const Nm_ChannelType* ChannelConf = &Nm_ConfigPtr->Channels[NetworkHandle];
	NM_CALL_BUSNM(RepeatMessageRequest, ChannelConf)
}

/** Get node identifier out of the last successfully received NM-message.
  * The function <BusNm>_GetNodeIdentifier shall be called. */
Nm_ReturnType Nm_GetNodeIdentifier( const NetworkHandleType NetworkHandle, uint8 * const nmNodeIdPtr ){
	const Nm_ChannelType* ChannelConf = &Nm_ConfigPtr->Channels[NetworkHandle];
	NM_CALL_BUSNM(GetNodeIdentifier, ChannelConf, nmNodeIdPtr)
}

/** Get node identifier configured for the local node.
  * For that purpose <BusNm>_GetLocalNodeIdentifier shall be called
  * (e.g. CanNm_GetLocalNodeIdentifier function is called if channel is configured as CAN). */
Nm_ReturnType Nm_GetLocalNodeIdentifier( const NetworkHandleType NetworkHandle, uint8 * const nmNodeIdPtr ){
	const Nm_ChannelType* ChannelConf = &Nm_ConfigPtr->Channels[NetworkHandle];
	NM_CALL_BUSNM(GetLocalNodeIdentifier, ChannelConf, nmNodeIdPtr)
}

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
	NM_CALL_BUSNM(GetState, ChannelConf, nmStatePtr, nmModePtr)
}

/** Notification that a NM-message has been received in the Bus-Sleep Mode, what
  * indicates that some nodes in the network have already entered the Network Mode.
  * The callback function shall start the network management state machine. */
void Nm_NetworkStartIndication( const NetworkHandleType NetworkHandle ){
	const Nm_ChannelType* ChannelConf = &Nm_ConfigPtr->Channels[NetworkHandle];
	ComM_Nm_NetworkStartIndication(ChannelConf->ComMNetworkHandle);
}

/** Notification that the network management has entered Network Mode. The
  * callback function shall enable transmission of application messages. */
void Nm_NetworkMode( const NetworkHandleType NetworkHandle ){
	const Nm_ChannelType* ChannelConf = &Nm_ConfigPtr->Channels[NetworkHandle];
	ComM_Nm_NetworkMode(ChannelConf->ComMNetworkHandle);
}

/** Notification that the network management has entered Prepare Bus-Sleep Mode.
  * The callback function shall disable transmission of application messages. */
void Nm_PrepareBusSleepMode( const NetworkHandleType NetworkHandle ){
	const Nm_ChannelType* ChannelConf = &Nm_ConfigPtr->Channels[NetworkHandle];
	ComM_Nm_PrepareBusSleepMode(ChannelConf->ComMNetworkHandle);
}

/** Notification that the network management has entered Bus-Sleep Mode. */
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
void Nm_MainFunction(){}

