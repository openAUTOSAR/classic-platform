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


#ifndef CANNM_H_
#define CANNM_H_

// Functions called by NM Interface
// --------------------------------

/** Initialize the complete CanNm module, i.e. all channels which are activated */
void CanNm_Init( const CanNm_ConfigType * const cannmConfigPtr );

/** Passive startup of the AUTOSAR CAN NM. It triggers the transition from Bus-
  * Sleep Mode to the Network Mode in Repeat Message State.
  * This service has no effect if the current state is not equal to Bus-Sleep Mode. In
  * that case NM_E_NOT_EXECUTED is returned. */
Nm_ReturnType CanNm_PassiveStartUp( const NetworkHandleType nmChannelHandle );

/** Request the network, since ECU needs to communicate on the bus. Network
  * state shall be changed to ‘requested’ */
Nm_ReturnType CanNm_NetworkRequest( const NetworkHandleType nmChannelHandle );

/** Release the network, since ECU doesn’t have to communicate on the bus. Network
  * state shall be changed to ‘released’. */
Nm_ReturnType CanNm_NetworkRelease( const NetworkHandleType nmChannelHandle );

/** Disable the NM PDU transmission ability due to a ISO14229 Communication
  * Control (28hex) service */
Nm_ReturnType CanNm_DisableCommunication( const NetworkHandleType nmChannelHandle );

/** Enable the NM PDU transmission ability due to a ISO14229 Communication
  * Control (28hex) service */
Nm_ReturnType CanNm_EnableCommunication( const NetworkHandleType nmChannelHandle );

/** Set user data for NM messages transmitted next on the bus. */
Nm_ReturnType CanNm_SetUserData( const NetworkHandleType nmChannelHandle, const uint8* const nmUserDataPtr );

/** Get user data out of the most recently received NM message. */
Nm_ReturnType CanNm_GetUserData( const NetworkHandleType nmChannelHandle, uint8* const nmUserDataPtr );

/** Get node identifier out of the most recently received NM PDU. */
Nm_ReturnType CanNm_GetNodeIdentifier( const NetworkHandleType nmChannelHandle, uint8 * const nmNodeIdPtr );

/** Get node identifier configured for the local node. */
Nm_ReturnType CanNm_GetLocalNodeIdentifier( const NetworkHandleType nmChannelHandle, uint8 * const nmNodeIdPtr );

/** Set Repeat Message Request Bit for NM messages transmitted next on the bus. */
Nm_ReturnType CanNm_RepeatMessageRequest( const NetworkHandleType nmChannelHandle );

/** Get the whole PDU data out of the most recently received NM message. */
Nm_ReturnType CanNm_GetPduData( const NetworkHandleType nmChannelHandle, uint8 * const nmPduDataPtr );

/** Returns the state and the mode of the network management. */
Nm_ReturnType CanNm_GetState( const NetworkHandleType nmChannelHandle, Nm_StateType * const nmStatePtr, Nm_ModeType * const nmModePtr );

/** This service returns the version information of this module. */
void CanNm_GetVersionInfo( Std_VersionInfoType * versioninfo );

/** Request bus synchronization. */
Nm_ReturnType CanNm_RequestBusSynchronization( const NetworkHandleType mmChannelHandle );

/** Check if remote sleep indication takes place or not. */
Nm_ReturnType CanNm_CheckRemoteSleepIndication( const NetworkHandleType nmChannelHandle, boolean * const nmRemoteSleepIndPtr );


// Functions called by CAN Interface
// ---------------------------------

/** This service confirms a previous successfully processed CAN transmit request.
  * This callback service is called by the CanIf and implemented by the CanNm. */
void CanNm_TxConfirmation( PduIdType canNmTxPduId );

/** This service indicates a successful reception of a received NM message to the
  * CanNm after passing all filters and validation checks.
  * This callback service is called by the CAN Interface and implemented by the CanNm. */
void CanNm_RxIndication( PduIdType canNmRxPduId, const unit8 *canSduPtr );

/** Main function of the CanNm which processes the algorithm describes in that document.
  * Generated functions used: CanNm_MainFunction_<Instance Id>( void );
  * e.g.
  * CanNm_MainFunction_0() represents the CanNm instance for the CAN channel 0
  * CanNm_MainFunction_1() represents the CanNm instance for the CAN channel 1 */
void CanNm_Arc_MainFunction( uint8 instanceId );



#endif /* CANNM_H_ */
