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


#ifndef NM_H_
#define NM_H_

/** @req NM117.partially */
#include "Std_Types.h"
#include "ComStack_Types.h"
#include "NmStack_Types.h"
#include "Nm_ConfigTypes.h"
#include "ComM_Nm.h"

#define NM_AR_MAJOR_VERSION	1
#define NM_AR_MINOR_VERSION	0
#define NM_AR_PATCH_VERSION	1

#define NM_SW_MAJOR_VERSION	1
#define NM_SW_MINOR_VERSION	0
#define NM_SW_PATCH_VERSION	0

#include "Nm_Cfg.h"

/** Initializes the NM Interface. */
/** @req NM030 */
void Nm_Init( const Nm_ConfigType * const nmConfigPtr );

/** This service returns the version information of this module */
/** @req NM044 */
void Nm_GetVersionInfo( Std_VersionInfoType* nmVerInfoPtr );

/** This function calls the <BusNm>_PassiveStartUp function
  * (e.g. CanNm_PassiveStartUp function is called if channel is configured as CAN). */
/** @req NM031 */
Nm_ReturnType Nm_PassiveStartUp( const NetworkHandleType NetworkHandle );

/** This function calls the <BusNm>_NetworkRequest
  * (e.g. CanNm_NetworkRequest function is called if channel is configured as CAN). */
/** @req NM032 */
Nm_ReturnType Nm_NetworkRequest( const NetworkHandleType NetworkHandle );

/** This function calls the <BusNm>_NetworkRelease bus specific function
  * (e.g. CanNm_NetworkRelease function is called if channel is configured as CAN). */
/** @req NM046 */
Nm_ReturnType Nm_NetworkRelease( const NetworkHandleType NetworkHandle );

/** This function calls the CanNm_NetworkRelease bus specific function, to disable
  * the NM PDU transmission ability due to a ISO14229 Communication Control (28hex) service */
/** @req NM033 */
Nm_ReturnType Nm_DisableCommunication( const NetworkHandleType NetworkHandle );

/** This function calls the CanNm_NetworkRequest bus specific function, to enable
  * the NM PDU transmission ability due to a ISO14229 Communication Control (28hex) service */
/** @req NM034 */
Nm_ReturnType Nm_EnableCommunication( const NetworkHandleType NetworkHandle );

/** Set user data for NM messages transmitted next on the bus.
  * For that purpose <BusNm>_SetUserData shall be called
  * (e.g. CanNm_SetUserData function is called if channel is configured as CAN). */
/** @req NM035 */
Nm_ReturnType Nm_SetUserData( const NetworkHandleType NetworkHandle, const uint8 * const nmUserDataPtr );

/** Get user data out of the last successfully received NM message.
  * For that purpose <BusNm>_GetUserData shall be called
  * (e.g. CanNm_GetUserData function is called if channel is configured as CAN). */
/** @req NM036 */
Nm_ReturnType Nm_GetUserData( const NetworkHandleType NetworkHandle, uint8 * const nmUserDataPtr, uint8 * const nmNodeIdPtr );

/** Get the whole PDU data out of the most recently received NM message.
  * For that purpose CanNm_GetPduData shall be called. */
/** @req NM037 */
Nm_ReturnType Nm_GetPduData( const NetworkHandleType NetworkHandle, uint8 * const nmPduData );

/** Set Repeat Message Request Bit for NM messages transmitted next on the bus.
  * For that purpose <BusNm>_RepeatMessageRequest shall be called
  * (e.g. CanNm_RepeatMessageRequest function is called if channel is configured as CAN) */
/** @req NM038 */
Nm_ReturnType Nm_RepeatMessageRequest( const NetworkHandleType NetworkHandle );

/** Get node identifier out of the last successfully received NM-message.
  * The function <BusNm>_GetNodeIdentifier shall be called. */
/** @req NM039 */
Nm_ReturnType Nm_GetNodeIdentifier( const NetworkHandleType NetworkHandle, uint8 * const nmNodeIdPtr );

/** Get node identifier configured for the local node.
  * For that purpose <BusNm>_GetLocalNodeIdentifier shall be called
  * (e.g. CanNm_GetLocalNodeIdentifier function is called if channel is configured as CAN). */
/** @req NM040 */
Nm_ReturnType Nm_GetLocalNodeIdentifier( const NetworkHandleType NetworkHandle, uint8 * const nmNodeIdPtr );

/** Check if remote sleep indication takes place or not. This in turn calls the
  * <BusNm>_CheckRemoteSleepIndication for the bus specific NM layer
  * (e.g. CanNm_CheckRemoteSleepIndication function is called if channel is configured as CAN). */
/** @req NM042 */
Nm_ReturnType Nm_CheckRemoteSleepIndication( const NetworkHandleType nmNetworkHandle, boolean * const nmRemoteSleepIndPtr );

/** Returns the state of the network management. This function in turn calls the
  * <BusNm>_GetState function (e.g. CanNm_GetState function is called if channel is configured as CAN). */
/** @req NM043 */
Nm_ReturnType Nm_GetState( const NetworkHandleType nmNetworkHandle, Nm_StateType* const nmStatePtr, Nm_ModeType* const nmModePtr );

/** Notification that a NM-message has been received in the Bus-Sleep Mode, what
  * indicates that some nodes in the network have already entered the Network Mode.
  * The callback function shall start the network management state machine. */
/** @req NM104 */
void Nm_NetworkStartIndication( const NetworkHandleType nmNetworkHandle );

/** Notification that the network management has entered Network Mode. The
  * callback function shall enable transmission of application messages. */
/** @req NM105 */
void Nm_NetworkMode( const NetworkHandleType nmNetworkHandle );

/** Notification that the network management has entered Prepare Bus-Sleep Mode.
  * The callback function shall disable transmission of application messages. */
/** @req NM106 */
void Nm_PrepareBusSleepMode( const NetworkHandleType nmNetworkHandle );

/** Notification that the network management has entered Bus-Sleep Mode. */
/** @req NM107 */
void Nm_BusSleepMode( const NetworkHandleType nmNetworkHandle );

/** Notification that a NM message has been received. */
/** @req NM112 */
void Nm_PduRxIndication( const NetworkHandleType nmNetworkHandle );

/** Notification that the CAN Generic NM state has changed. */
/** @req NM114 */
void Nm_StateChangeNotification(
		const NetworkHandleType nmNetworkHandle,
		const Nm_StateType nmPreviousState,
		const Nm_StateType nmCurrentState );

/** This function implements the processes of the NM Interface, which need a fix
  * cyclic scheduling. This function is supplied for the NM coordinator functionality
  * (Nm020). However, specific implementation may not need it (Nm093) */
/** @req NM118 */
void Nm_MainFunction(void);


#endif /* NM_H_ */
