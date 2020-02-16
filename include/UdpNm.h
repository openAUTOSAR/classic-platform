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


#ifndef UDPNM_H
#define UDPNM_H

/** @req UDPNM021 */

#include "ComStack_Types.h"
#include "NmStack_Types.h"
#include "UdpNm_ConfigTypes.h"
#include "Modules.h"
#include "UdpNm_Cbk.h"

#define UDPNM_VENDOR_ID  1
#define UDPNM_MODULE_ID  MODULE_ID_UDPNM
/** @req UDPNM200 */
#define UDPNM_AR_MAJOR_VERSION	3
#define UDPNM_AR_MINOR_VERSION	1
#define UDPNM_AR_PATCH_VERSION	5

#define UDPNM_SW_MAJOR_VERSION	1
#define UDPNM_SW_MINOR_VERSION	0
#define UDPNM_SW_PATCH_VERSION	0

#include "UdpNm_Cfg.h"


/** @req UDPNM018  @req UDPNM240 */
#define UDPNM_E_NO_INIT						0x01u
#define UDPNM_E_INVALID_CHANNEL				0x02u

/** NM-Timeout Timer has abnormally expired outside of the Ready Sleep State;
  * it may happen: (1) because of Bus-Off state, (2) if some ECU requests bus communication or node detection shortly
  * before the NMTimeout Timer expires so that a NM message can not be transmitted in time;
  * this race condition applies to event-triggered systems */
#define UDP_E_INVALID_PDUID			        0x03u
#define UDPNM_E_NULL_POINTER				0x12u


#define UDPNM_SERVICEID_INIT								0x01u
#define UDPNM_SERVICEID_PASSIVESTARTUP						0x0Eu
#define UDPNM_SERVICEID_NETWORKREQUEST						0x02u
#define UDPNM_SERVICEID_NETWORKRELEASE						0x03u
#define UDPNM_SERVICEID_DISABLECOMMUNICATION				0x0Cu
#define UDPNM_SERVICEID_ENABLECOMMUNICATION					0x0Du
#define UDPNM_SERVICEID_SETUSERDATA							0x04u
#define UDPNM_SERVICEID_GETUSERDATA							0x05u
#define UDPNM_SERVICEID_GETNODEIDENTIFIER					0x06u
#define UDPNM_SERVICEID_GETLOCALNODEIDENTIFIER				0x07u
#define UDPNM_SERVICEID_REPEATMESSAGEREQUEST				0x08u
#define UDPNM_SERVICEID_GETPDUDATA							0x0Au
#define UDPNM_SERVICEID_GETSTATE							0x0Bu
#define UDPNM_SERVICEID_GETVERSIONINFO						0x09u
#define UDPNM_SERVICEID_REQUESTBUSSYNCHRONIZATION			0x14u
#define UDPNM_SERVICEID_CHECKREMOTESLEEPINDICATION			0x11u
#define UDPNM_SERVICEID_SETCOORDBIT                         0x12u
#define UDPNM_SERVICEID_SOADIFTXCONFIRMATION				0x0Fu
#define UDPNM_SERVICEID_SOADIFRXINDICATION					0x10u
#define UDPNM_SERVICEID_TRANSMIT                            0x15u
#define UDPNM_SERVICEID_ARC_MAINFUNCTION					0x13u

/** @req UDPNM045 partial */
#define UDPNM_CBV_REPEAT_MESSAGE_REQUEST					0x01u

// Functions called by NM Interface
// --------------------------------

/** Initialize the complete UdpNm module, i.e. all channels which are activated */
void UdpNm_Init( const UdpNm_ConfigType* UdpNmConfigPtr );

/** Passive startup of the AUTOSAR Udp NM. It triggers the transition from Bus-
  * Sleep Mode to the Network Mode in Repeat Message State.
  * This service has no effect if the current state is not equal to Bus-Sleep Mode. In
  * that case NM_E_NOT_EXECUTED is returned. */
Nm_ReturnType UdpNm_PassiveStartUp( const NetworkHandleType nmChannelHandle );

/** Request the network, since ECU needs to communicate on the bus. Network
  * state shall be changed to*/
Nm_ReturnType UdpNm_NetworkRequest( const NetworkHandleType nmChannelHandle );

/** Release the network, since ECU doesn't have to communicate on the bus. Network
  * state shall be changed to */
Nm_ReturnType UdpNm_NetworkRelease( const NetworkHandleType nmChannelHandle );

/** Disable the NM PDU transmission ability due to a ISO14229 Communication
  * Control (28hex) service */
Nm_ReturnType UdpNm_DisableCommunication( const NetworkHandleType nmChannelHandle );

/** Enable the NM PDU transmission ability due to a ISO14229 Communication
  * Control (28hex) service */
Nm_ReturnType UdpNm_EnableCommunication( const NetworkHandleType nmChannelHandle );

/** Set user data for NM messages transmitted next on the bus. */
Nm_ReturnType UdpNm_SetUserData( const NetworkHandleType nmChannelHandle, const uint8* nmUserDataPtr );

/** Get user data out of the most recently received NM message. */
Nm_ReturnType UdpNm_GetUserData( const NetworkHandleType nmChannelHandle, uint8* const nmUserDataPtr );

/** Get node identifier out of the most recently received NM PDU. */
Nm_ReturnType UdpNm_GetNodeIdentifier( const NetworkHandleType nmChannelHandle, uint8* const nmNodeIdPtr );

/** Get node identifier configured for the local node. */
Nm_ReturnType UdpNm_GetLocalNodeIdentifier( const NetworkHandleType nmChannelHandle, uint8* const nmNodeIdPtr );

/** Set Repeat Message Request Bit for NM messages transmitted next on the bus. */
Nm_ReturnType UdpNm_RepeatMessageRequest( const NetworkHandleType nmChannelHandle );

/** Get the whole PDU data out of the most recently received NM message. */
Nm_ReturnType UdpNm_GetPduData( const NetworkHandleType nmChannelHandle, uint8* const nmPduDataPtr );

/** Returns the state and the mode of the network management. */
Nm_ReturnType UdpNm_GetState( const NetworkHandleType nmChannelHandle, Nm_StateType* const nmStatePtr, Nm_ModeType* const nmModePtr );

/** This service returns the version information of this module. */
/**< @req UDPNM224 */
#if ( UDPNM_VERSION_INFO_API == STD_ON )
#define UdpNm_GetVersionInfo(_vi) STD_GET_VERSION_INFO(_vi,UDPNM)
#endif /* UDPNM_VERSION_INFO_API */

/** Request bus synchronization. */
Nm_ReturnType UdpNm_RequestBusSynchronization( const NetworkHandleType nmChannelHandle );

/** Check if remote sleep indication takes place or not. */
Nm_ReturnType UdpNm_CheckRemoteSleepIndication( const NetworkHandleType nmChannelHandle, boolean* const NmRemoteSleepIndPtr );

/** Sets the NM coordinator ID in the control bit vector of each NM message. */
Nm_ReturnType UdpNm_SetCoordBits( const NetworkHandleType nmChannelHandle, const uint8 nmCoordBits );

// Functions called by SoAd Interface
// ---------------------------------

/** This service confirms a previous successfully processed transmit request.
  * This callback service is called by the SoAd and implemented by the UdpNm. */
void UdpNm_SoAdIfTxConfirmation( PduIdType UdpNmTxPduId );

/** This service indicates a successful reception of a received NM message to the
  * UdpNm after passing all filters and validation checks.
  * This callback service is called by the SoAd and implemented by the UdpNm. */
void UdpNm_SoAdIfRxIndication( PduIdType udpNmRxPduId, const uint8* udpSduPtr );

// UdpNm functions called by the PDU-Router
// ----------------------------------------

/** UdpNm_Transmit is implemented as an empty function and shall always return E_OK. */
Std_ReturnType UdpNm_Transmit( PduIdType UdpNmSrcPduId, const PduInfoType* UdpNmSrcPduInfoPtr );

#endif /* UDPNM_H */
