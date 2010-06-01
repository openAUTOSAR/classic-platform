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


#include "ComStack_Types.h"
#include "CanNm.h"
#include "CanNm_Internal.h"
#include "Nm_Cbk.h"
#include "NmStack_Types.h"
#include "MemMap.h"

CanNm_InternalType CanNm_Internal = {
		.InitStatus = CANNM_UNINIT,
};

/** Initialize the complete CanNm module, i.e. all channels which are activated */
void CanNm_Init( const CanNm_ConfigType * const cannmConfigPtr ){}

/** Passive startup of the AUTOSAR CAN NM. It triggers the transition from Bus-
  * Sleep Mode to the Network Mode in Repeat Message State.
  * This service has no effect if the current state is not equal to Bus-Sleep Mode. In
  * that case NM_E_NOT_EXECUTED is returned. */
Nm_ReturnType CanNm_PassiveStartUp( const NetworkHandleType nmChannelHandle ){
	CANNM_VALIDATE_INIT(CANNM_SERVICEID_PASSIVESTARTUP, NM_E_NOT_OK);
	CANNM_VALIDATE_CHANNEL(nmChannelHandle, CANNM_SERVICEID_PASSIVESTARTUP, NM_E_NOT_OK);
	return NM_E_NOT_OK;
}

/** Request the network, since ECU needs to communicate on the bus. Network
  * state shall be changed to ‘requested’ */
Nm_ReturnType CanNm_NetworkRequest( const NetworkHandleType nmChannelHandle ){
	CANNM_VALIDATE_INIT(CANNM_SERVICEID_NETWORKREQUEST, NM_E_NOT_OK);
	CANNM_VALIDATE_CHANNEL(nmChannelHandle, CANNM_SERVICEID_NETWORKREQUEST, NM_E_NOT_OK);
	return NM_E_NOT_OK;
}

/** Release the network, since ECU doesn’t have to communicate on the bus. Network
  * state shall be changed to ‘released’. */
Nm_ReturnType CanNm_NetworkRelease( const NetworkHandleType nmChannelHandle ){
	CANNM_VALIDATE_INIT(CANNM_SERVICEID_NETWORKRELEASE, NM_E_NOT_OK);
	CANNM_VALIDATE_CHANNEL(nmChannelHandle, CANNM_SERVICEID_NETWORKRELEASE, NM_E_NOT_OK);
	return NM_E_NOT_OK;
}

/** Disable the NM PDU transmission ability due to a ISO14229 Communication
  * Control (28hex) service */
Nm_ReturnType CanNm_DisableCommunication( const NetworkHandleType nmChannelHandle ){
	CANNM_VALIDATE_INIT(CANNM_SERVICEID_DISABLECOMMUNICATION, NM_E_NOT_OK);
	CANNM_VALIDATE_CHANNEL(nmChannelHandle, CANNM_SERVICEID_DISABLECOMMUNICATION, NM_E_NOT_OK);
	return NM_E_NOT_OK;
}

/** Enable the NM PDU transmission ability due to a ISO14229 Communication
  * Control (28hex) service */
Nm_ReturnType CanNm_EnableCommunication( const NetworkHandleType nmChannelHandle ){
	CANNM_VALIDATE_INIT(CANNM_SERVICEID_ENABLECOMMUNICATION, NM_E_NOT_OK);
	CANNM_VALIDATE_CHANNEL(nmChannelHandle, CANNM_SERVICEID_ENABLECOMMUNICATION, NM_E_NOT_OK);
	return NM_E_NOT_OK;
}

/** Set user data for NM messages transmitted next on the bus. */
Nm_ReturnType CanNm_SetUserData( const NetworkHandleType nmChannelHandle, const uint8* const nmUserDataPtr ){
	CANNM_VALIDATE_INIT(CANNM_SERVICEID_SETUSERDATA, NM_E_NOT_OK);
	CANNM_VALIDATE_CHANNEL(nmChannelHandle, CANNM_SERVICEID_SETUSERDATA, NM_E_NOT_OK);
	return NM_E_NOT_OK;
}

/** Get user data out of the most recently received NM message. */
Nm_ReturnType CanNm_GetUserData( const NetworkHandleType nmChannelHandle, uint8* const nmUserDataPtr ){
	CANNM_VALIDATE_INIT(CANNM_SERVICEID_GETUSERDATA, NM_E_NOT_OK);
	CANNM_VALIDATE_CHANNEL(nmChannelHandle, CANNM_SERVICEID_GETUSERDATA, NM_E_NOT_OK);
	return NM_E_NOT_OK;
}

/** Get node identifier out of the most recently received NM PDU. */
Nm_ReturnType CanNm_GetNodeIdentifier( const NetworkHandleType nmChannelHandle, uint8 * const nmNodeIdPtr ){
	CANNM_VALIDATE_INIT(CANNM_SERVICEID_GETNODEIDENTIFIER, NM_E_NOT_OK);
	CANNM_VALIDATE_CHANNEL(nmChannelHandle, CANNM_SERVICEID_GETNODEIDENTIFIER, NM_E_NOT_OK);
	return NM_E_NOT_OK;
}

/** Get node identifier configured for the local node. */
Nm_ReturnType CanNm_GetLocalNodeIdentifier( const NetworkHandleType nmChannelHandle, uint8 * const nmNodeIdPtr ){
	CANNM_VALIDATE_INIT(CANNM_SERVICEID_GETLOCALNODEIDENTIFIER, NM_E_NOT_OK);
	CANNM_VALIDATE_CHANNEL(nmChannelHandle, CANNM_SERVICEID_GETLOCALNODEIDENTIFIER, NM_E_NOT_OK);
	return NM_E_NOT_OK;
}

/** Set Repeat Message Request Bit for NM messages transmitted next on the bus. */
Nm_ReturnType CanNm_RepeatMessageRequest( const NetworkHandleType nmChannelHandle ){
	CANNM_VALIDATE_INIT(CANNM_SERVICEID_REPEATMESSAGEREQUEST, NM_E_NOT_OK);
	CANNM_VALIDATE_CHANNEL(nmChannelHandle, CANNM_SERVICEID_REPEATMESSAGEREQUEST, NM_E_NOT_OK);
	return NM_E_NOT_OK;
}

/** Get the whole PDU data out of the most recently received NM message. */
Nm_ReturnType CanNm_GetPduData( const NetworkHandleType nmChannelHandle, uint8 * const nmPduDataPtr ){
	CANNM_VALIDATE_INIT(CANNM_SERVICEID_GETPDUDATA, NM_E_NOT_OK);
	CANNM_VALIDATE_CHANNEL(nmChannelHandle, CANNM_SERVICEID_GETPDUDATA, NM_E_NOT_OK);
	return NM_E_NOT_OK;
}

/** Returns the state and the mode of the network management. */
Nm_ReturnType CanNm_GetState( const NetworkHandleType nmChannelHandle, Nm_StateType * const nmStatePtr, Nm_ModeType * const nmModePtr ){
	CANNM_VALIDATE_INIT(CANNM_SERVICEID_GETSTATE, NM_E_NOT_OK);
	CANNM_VALIDATE_CHANNEL(nmChannelHandle, CANNM_SERVICEID_GETSTATE, NM_E_NOT_OK);
	return NM_E_NOT_OK;
}

/** This service returns the version information of this module. */
void CanNm_GetVersionInfo( Std_VersionInfoType * versioninfo ){
	CANNM_VALIDATE_INIT(CANNM_SERVICEID_GETVERSIONINFO);
}

/** Request bus synchronization. */
Nm_ReturnType CanNm_RequestBusSynchronization( const NetworkHandleType mmChannelHandle ){
	CANNM_VALIDATE_INIT(CANNM_SERVICEID_REQUESTBUSSYNCHRONIZATION, NM_E_NOT_OK);
	CANNM_VALIDATE_CHANNEL(nmChannelHandle, CANNM_SERVICEID_REQUESTBUSSYNCHRONIZATION, NM_E_NOT_OK);
	return NM_E_NOT_OK;
}

/** Check if remote sleep indication takes place or not. */
Nm_ReturnType CanNm_CheckRemoteSleepIndication( const NetworkHandleType nmChannelHandle, boolean * const nmRemoteSleepIndPtr ){
	CANNM_VALIDATE_INIT(CANNM_SERVICEID_CHECKREMOTESLEEPINDICATION, NM_E_NOT_OK);
	CANNM_VALIDATE_CHANNEL(nmChannelHandle, CANNM_SERVICEID_CHECKREMOTESLEEPINDICATION, NM_E_NOT_OK);
	return NM_E_NOT_OK;
}


// Functions called by CAN Interface
// ---------------------------------

/** This service confirms a previous successfully processed CAN transmit request.
  * This callback service is called by the CanIf and implemented by the CanNm. */
void CanNm_TxConfirmation( PduIdType canNmTxPduId ){
	CANNM_VALIDATE_INIT(CANNM_SERVICEID_TXCONFIRMATION);
}

/** This service indicates a successful reception of a received NM message to the
  * CanNm after passing all filters and validation checks.
  * This callback service is called by the CAN Interface and implemented by the CanNm. */
void CanNm_RxIndication( PduIdType canNmRxPduId, const uint8 *canSduPtr ){
	CANNM_VALIDATE_INIT(CANNM_SERVICEID_RXINDICATION);
}

/** Main function of the CanNm which processes the algorithm describes in that document.
  * Generated functions used: CanNm_MainFunction_<Instance Id>( void ){}
  * e.g.
  * CanNm_MainFunction_0() represents the CanNm instance for the CAN channel 0
  * CanNm_MainFunction_1() represents the CanNm instance for the CAN channel 1 */
void CanNm_Arc_MainFunction( uint8 instanceId ){
	CANNM_VALIDATE_INIT(CANNM_SERVICEID_ARC_MAINFUNCTION);
}


