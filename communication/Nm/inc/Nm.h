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


#ifndef NM_H_
#define NM_H_

#include "Std_Types.h" /** @req NM124 */
#include "ComStack_Types.h"
#include "NmStack_Types.h" /** @req NM123 */
#include "Nm_ConfigTypes.h"
#include "ComM_Nm.h" /** @req NM124 */

#define NM_VENDOR_ID        60u
#define NM_MODULE_ID        29u

#define NM_AR_RELEASE_MAJOR_VERSION      4u
#define NM_AR_RELEASE_MINOR_VERSION      0u
#define NM_AR_RELEASE_PATCH_VERSION      3u

#define NM_AR_MAJOR_VERSION	NM_AR_RELEASE_MAJOR_VERSION
#define NM_AR_MINOR_VERSION	NM_AR_RELEASE_MINOR_VERSION
#define NM_AR_PATCH_VERSION	NM_AR_RELEASE_PATCH_VERSION

#define NM_SW_MAJOR_VERSION	3u
#define NM_SW_MINOR_VERSION	2u
#define NM_SW_PATCH_VERSION	1u

#include "Nm_Cfg.h" /** @req NM123 */

/** @req Nm232 */
/* Error codes*/
#define NM_E_UNINIT             0u
#define NM_E_HANDLE_UNDEF       1u
#define NM_E_PARAM_POINTER      2u

/* Api Service codes */
#define NM_SERVICEID_INIT                        0x00u
#define NM_SERVICEID_PASSIVESTARTUP              0x01u
#define NM_SERVICEID_NETWORKREQUEST              0x02u
#define NM_SERVICEID_NETWORKRELEASE              0x03u
#define NM_SERVICEID_DISABLECOMMUNICATION        0x04u
#define NM_SERVICEID_ENABLECOMMUNICATION         0x05u
#define NM_SERVICEID_SETUSERDATA                 0x06u
#define NM_SERVICEID_GETUSERDATA                 0x07u
#define NM_SERVICEID_GETPDUDATA                  0x08u
#define NM_SERVICEID_REPEATMESSAGEREQUEST        0x09u
#define NM_SERVICEID_GETNODEIDENTIFIER           0x0Au
#define NM_SERVICEID_GETLOCALNODEIDENTIFIER      0x0Bu
#define NM_SERVICEID_CHECKREMOTESLEEPINDICATION  0x0Du
#define NM_SERVICEID_GETSTATE                    0x0Eu
#define NM_SERVICEID_GETVERSIONINFO              0x0Fu
#define NM_SERVICEID_NETWORKSTARTINDICATION      0x11u
#define NM_SERVICEID_NETWORKMODE                 0x12u
#define NM_SERVICEID_BUSSLEEPMODE                0x14u
#define NM_SERVICEID_PREPAREBUSSLEEPMODE         0x13u
#define NM_SERVICEID_REMOTESLEEPINDICATION       0x17u
#define NM_SERVICEID_REMOTESLEEPCANCELLATION     0x18u
#define NM_SERVICEID_REMOTESYNCHRONISATIONPOINT  0x19u
#define NM_SERVICEID_PDURXINDICATION             0x15u
#define NM_SERVICEID_STATECHANGENOTIFICATION     0x16u
#define NM_SERVICEID_REPEATMESSAGEINDICATION     0x1Au
#define NM_SERVICEID_TXTIMEOUTEXCEPTION          0x1Bu
#define NM_SERVICEID_CARWAKEUPINDICATION         0x1Du
#define NM_SERVICEID_COORDREADYTOSLEEPINDICATION 0x1Eu
#define NM_SERVICEID_MAINFUNCTION                0x10u

extern const Nm_ConfigType Nm_Config ;


/** Initializes the NM Interface. */
/** @req NM030 */
void Nm_Init(void);

/** This service returns the version information of this module */
/** @req NM044 */
/* @req  Nm152 */
#if ( NM_VERSION_INFO_ENABLED == STD_ON )
#define Nm_GetVersionInfo(_vi) STD_GET_VERSION_INFO(_vi,NM)
#endif


/** This function calls the <BusNm>_PassiveStartUp function
  * (e.g. CanNm_PassiveStartUp function is called if channel is configured as CAN). */
/** @req NM031 */
Std_ReturnType Nm_PassiveStartUp( const NetworkHandleType NetworkHandle );

/** This function calls the <BusNm>_NetworkRequest
  * (e.g. CanNm_NetworkRequest function is called if channel is configured as CAN). */
/** @req NM032 */
Std_ReturnType Nm_NetworkRequest( const NetworkHandleType NetworkHandle );

/** This function calls the <BusNm>_NetworkRelease bus specific function
  * (e.g. CanNm_NetworkRelease function is called if channel is configured as CAN). */
/** @req NM046 */
Std_ReturnType Nm_NetworkRelease( const NetworkHandleType NetworkHandle );

#if (NM_COM_CONTROL_ENABLED == STD_ON) /* @req NM134  @req NM136 */
/** This function calls the CanNm_NetworkRelease bus specific function, to disable
  * the NM PDU transmission ability due to a ISO14229 Communication Control (28hex) service */
/** @req NM033 */
Std_ReturnType Nm_DisableCommunication( const NetworkHandleType NetworkHandle );

/** This function calls the CanNm_NetworkRequest bus specific function, to enable
  * the NM PDU transmission ability due to a ISO14229 Communication Control (28hex) service */
/** @req NM034 */
Std_ReturnType Nm_EnableCommunication( const NetworkHandleType NetworkHandle );
#endif

/** Set user data for NM messages transmitted next on the bus.
  * For that purpose <BusNm>_SetUserData shall be called
  * (e.g. CanNm_SetUserData function is called if channel is configured as CAN). */
/** @req NM035 */
Std_ReturnType Nm_SetUserData( const NetworkHandleType NetworkHandle, const uint8 * const nmUserDataPtr );

/** Get user data out of the last successfully received NM message.
  * For that purpose <BusNm>_GetUserData shall be called
  * (e.g. CanNm_GetUserData function is called if channel is configured as CAN). */
/** @req NM036 */
Std_ReturnType Nm_GetUserData( const NetworkHandleType NetworkHandle, uint8 * const nmUserDataPtr );

/** Get the whole PDU data out of the most recently received NM message.
  * For that purpose CanNm_GetPduData shall be called. */
/** @req NM037 */
Std_ReturnType Nm_GetPduData( const NetworkHandleType NetworkHandle, uint8 * const nmPduData );

/** Set Repeat Message Request Bit for NM messages transmitted next on the bus.
  * For that purpose <BusNm>_RepeatMessageRequest shall be called
  * (e.g. CanNm_RepeatMessageRequest function is called if channel is configured as CAN) */
/** @req NM038 */
Std_ReturnType Nm_RepeatMessageRequest( const NetworkHandleType NetworkHandle );

/** Get node identifier out of the last successfully received NM-message.
  * The function <BusNm>_GetNodeIdentifier shall be called. */
/** @req NM039 */
Std_ReturnType Nm_GetNodeIdentifier( const NetworkHandleType NetworkHandle, uint8 * const nmNodeIdPtr );

/** Get node identifier configured for the local node.
  * For that purpose <BusNm>_GetLocalNodeIdentifier shall be called
  * (e.g. CanNm_GetLocalNodeIdentifier function is called if channel is configured as CAN). */
/** @req NM040 */
Std_ReturnType Nm_GetLocalNodeIdentifier( const NetworkHandleType NetworkHandle, uint8 * const nmNodeIdPtr );

/** Check if remote sleep indication takes place or not. This in turn calls the
  * <BusNm>_CheckRemoteSleepIndication for the bus specific NM layer
  * (e.g. CanNm_CheckRemoteSleepIndication function is called if channel is configured as CAN). */
/** @req NM042 */
Std_ReturnType Nm_CheckRemoteSleepIndication( const NetworkHandleType nmNetworkHandle, boolean * const nmRemoteSleepIndPtr );

/** Returns the state of the network management. This function in turn calls the
  * <BusNm>_GetState function (e.g. CanNm_GetState function is called if channel is configured as CAN). */
/** @req NM043 */
Std_ReturnType Nm_GetState( const NetworkHandleType nmNetworkHandle, Nm_StateType* const nmStatePtr, Nm_ModeType* const nmModePtr );

/** Notification that a NM-message has been received in the Bus-Sleep Mode, what
  * indicates that some nodes in the network have already entered the Network Mode.
  * The callback function shall start the network management state machine. */
/** @req NM154 */
void Nm_NetworkStartIndication( const NetworkHandleType nmNetworkHandle );

/** Notification that the network management has entered Network Mode. The
  * callback function shall enable transmission of application messages. */
/** @req NM156 */
void Nm_NetworkMode( const NetworkHandleType nmNetworkHandle );

/** Notification that the network management has entered Prepare Bus-Sleep Mode.
  * The callback function shall disable transmission of application messages. */
/** @req NM159 */
void Nm_PrepareBusSleepMode( const NetworkHandleType nmNetworkHandle );

/** Notification that the network management has entered Bus-Sleep Mode. */
/** @req NM162 */
void Nm_BusSleepMode( const NetworkHandleType nmNetworkHandle );

/** Notification that the network management has detected that all other nodes on the network are ready to enter Bus-Sleep Mode */
/** @req NM192 */
void Nm_RemoteSleepIndication( const NetworkHandleType nmNetworkHandle );

/**Notification that the network management has detected that not all other nodes on the network are longer ready to enter Bus-Sleep Mode */
/** @req NM193 */
void Nm_RemoteSleepCancellation( const NetworkHandleType nmNetworkHandle );

/** Notification to the NM Coordinator functionality that this is a suitable point in time to initiate the coordination algorithm on */
/** @req NM194 */
void Nm_SynchronizationPoint( const NetworkHandleType nmNetworkHandle );

/** Notification that a NM message has been received. */
/** @req NM112 */
void Nm_PduRxIndication( const NetworkHandleType nmNetworkHandle );

/** Notification that the CAN Generic NM state has changed. */
/** @req NM114 */
void Nm_StateChangeNotification(
        const NetworkHandleType nmNetworkHandle,
        const Nm_StateType nmPreviousState,
        const Nm_StateType nmCurrentState );

/** Service to indicate that an NM message with set Repeat Message Request Bit has been received */
/** @req NM230 */
void Nm_RepeatMessageIndication( const NetworkHandleType nmNetworkHandle );

/** Service to indicate that an attempt to send an NM message failed */
/** @req NM234 */
void Nm_TxTimeoutException( const NetworkHandleType nmNetworkHandle );

/** This function is called by a <Bus>Nm to indicate reception of a CWU request */
/** @req NM250 */
void Nm_CarWakeUpIndication( const NetworkHandleType nmNetworkHandle );

/** Seta an indication, when the NM coordinator Sleep Ready bit in the Control Bit Vector is set */
/** @req NM254 */
void Nm_CoordReadyToSleepIndication( const NetworkHandleType nmNetworkHandle );

/** This function implements the processes of the NM Interface, which need a fix
  * cyclic scheduling. This function is supplied for the NM coordinator functionality
  * (Nm020). However, specific implementation may not need it (Nm093) */
/** @req NM118 */
void Nm_MainFunction(void);


#endif /* NM_H_ */
