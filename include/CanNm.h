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


#ifndef CANNM_H
#define CANNM_H

/** @req CANNM021 */
#define CANNM_VENDOR_ID  VENDOR_ID_ARCCORE
#define CANNM_MODULE_ID  MODULE_ID_CANNM

#define CANNM_AR_RELEASE_MAJOR_VERSION	4
#define CANNM_AR_RELEASE_MINOR_VERSION	0
#define CANNM_AR_RELEASE_PATCH_VERSION	3

#define CANNM_AR_MAJOR_VERSION CANNM_AR_RELEASE_MAJOR_VERSION
#define CANNM_AR_MINOR_VERSION CANNM_AR_RELEASE_MINOR_VERSION
#define CANNM_AR_PATCH_VERSION CANNM_AR_RELEASE_PATCH_VERSION

#define CANNM_SW_MAJOR_VERSION	1
#define CANNM_SW_MINOR_VERSION	0
#define CANNM_SW_PATCH_VERSION	0

#include "CanNm_Cfg.h"

#include "ComStack_Types.h"	/** @req CANNM245 */
#include "NmStack_Types.h"	/** @req CANNM245 */

#include "CanNm_ConfigTypes.h"
#include "Modules.h"
#include "CanNm_Cbk.h"

/** @req CANNM240 */
#define CANNM_E_NO_INIT						0x01u /** @req CANNM316 */ /**< API service used */
#define CANNM_E_INVALID_CHANNEL				0x02u /** @req CANNM317 */ /**< API service called with wrong channel handle */
/** NM-Timeout Timer has abnormally expired outside of the Ready Sleep State;
it may happen: (1) because of Bus-Off state, (2) if some ECU requests bus communication or node detection shortly
before the NMTimeout Timer expires so that a NM message can not be transmitted in time;
this race condition applies to event-triggered systems */
#define CANNM_E_INVALID_PDUID 				0x03u /** @req CANNM318 */
#define CANNM_E_NET_START_IND				0x04u /** @req CANNM337 */
#define CANNM_E_INIT_FAILED					0x05u /** @req CANNM319 */
#define CANNM_E_NETWORK_TIMEOUT				0x11u /** @req CANNM321 */
#define NM_E_NULL_POINTER					0x12u /** @req CANNM322 */ /**< Null pointer has been passed as an argument (Does not apply to function CanNm_Init) */


#define CANNM_SERVICEID_INIT								0x00u
#define CANNM_SERVICEID_PASSIVESTARTUP						0x01u
#define CANNM_SERVICEID_NETWORKREQUEST						0x02u
#define CANNM_SERVICEID_NETWORKRELEASE						0x03u
#define CANNM_SERVICEID_DISABLECOMMUNICATION				0x0Cu
#define CANNM_SERVICEID_ENABLECOMMUNICATION					0x0Du
#define CANNM_SERVICEID_SETUSERDATA							0x04u
#define CANNM_SERVICEID_GETUSERDATA							0x05u
#define CANNM_SERVICEID_GETNODEIDENTIFIER					0x06u
#define CANNM_SERVICEID_GETLOCALNODEIDENTIFIER				0x07u
#define CANNM_SERVICEID_REPEATMESSAGEREQUEST				0x08u
#define CANNM_SERVICEID_GETPDUDATA							0x0Au
#define CANNM_SERVICEID_GETSTATE							0x0Bu
#define CANNM_SERVICEID_GETVERSIONINFO						0xF1u
#define CANNM_SERVICEID_REQUESTBUSSYNCHRONIZATION			0xC0u
#define CANNM_SERVICEID_CHECKREMOTESLEEPINDICATION			0xD0u
#define CANNM_SERVICEID_TXCONFIRMATION						0x0Fu
#define CANNM_SERVICEID_RXINDICATION						0x10u
#define CANNM_SERVICEID_ARC_MAINFUNCTION					0x13u
#define CANNM_SERVICEID_TRANSMIT							0x14u

#define CANNM_CBV_REPEAT_MESSAGE_REQUEST					0x01u

// Functions called by NM Interface
// --------------------------------

/** Initialize the complete CanNm module, i.e. all channels which are activated */
/** @req CANNM208 */
void CanNm_Init( const CanNm_ConfigType * const cannmConfigPtr );

/** Passive startup of the AUTOSAR CAN NM. It triggers the transition from Bus-
  * Sleep Mode to the Network Mode in Repeat Message State.
  * This service has no effect if the current state is not equal to Bus-Sleep Mode. In
  * that case NM_E_NOT_EXECUTED is returned. */
/** @req CANNM211 */
Std_ReturnType CanNm_PassiveStartUp( const NetworkHandleType nmChannelHandle );

/** Request the network, since ECU needs to communicate on the bus. Network
  * state shall be changed to �requested� */
/** @req CANNM213 */
Std_ReturnType CanNm_NetworkRequest( const NetworkHandleType nmChannelHandle );

/** Release the network, since ECU doesn�t have to communicate on the bus. Network
  * state shall be changed to �released�. */
/** @req CANNM214 */
Std_ReturnType CanNm_NetworkRelease( const NetworkHandleType nmChannelHandle );

/** Disable the NM PDU transmission ability due to a ISO14229 Communication
  * Control (28hex) service */
/** @req CANNM215 */
Std_ReturnType CanNm_DisableCommunication( const NetworkHandleType nmChannelHandle );

/** Enable the NM PDU transmission ability due to a ISO14229 Communication
  * Control (28hex) service */
/** @req CANNM216 */
Std_ReturnType CanNm_EnableCommunication( const NetworkHandleType nmChannelHandle );

/** Set user data for NM messages transmitted next on the bus. */
/** @req CANNM217 */
Std_ReturnType CanNm_SetUserData( const NetworkHandleType nmChannelHandle, const uint8* const nmUserDataPtr );

/** Get user data out of the most recently received NM message. */
/** @req CANNM218 */
Std_ReturnType CanNm_GetUserData( const NetworkHandleType nmChannelHandle, uint8* const nmUserDataPtr );

/** Get node identifier out of the most recently received NM PDU. */
/** @req CANNM219 */
Std_ReturnType CanNm_GetNodeIdentifier( const NetworkHandleType nmChannelHandle, uint8 * const nmNodeIdPtr );

/** Get node identifier configured for the local node. */
/** @req CANNM220 */
Std_ReturnType CanNm_GetLocalNodeIdentifier( const NetworkHandleType nmChannelHandle, uint8 * const nmNodeIdPtr );

/** Set Repeat Message Request Bit for NM messages transmitted next on the bus. */
/** @req CANNM221 */
Std_ReturnType CanNm_RepeatMessageRequest( const NetworkHandleType nmChannelHandle );

/** Get the whole PDU data out of the most recently received NM message. */
/** @req CANNM222 */
Std_ReturnType CanNm_GetPduData( const NetworkHandleType nmChannelHandle, uint8 * const nmPduDataPtr );

/** Returns the state and the mode of the network management. */
/** @req CANNM223 */
Std_ReturnType CanNm_GetState( const NetworkHandleType nmChannelHandle, Nm_StateType * const nmStatePtr, Nm_ModeType * const nmModePtr );


/** Request bus synchronization. */
/** @req CANNM226 */
Std_ReturnType CanNm_RequestBusSynchronization( const NetworkHandleType nmChannelHandle );

/** Check if remote sleep indication takes place or not. */
/** @req CANNM227 */
Std_ReturnType CanNm_CheckRemoteSleepIndication( const NetworkHandleType nmChannelHandle, boolean * const nmRemoteSleepIndPtr );


/** This service returns the version information of this module. */
/** @req CANNM224 */
#if ( CANNM_VERSION_INFO_API == STD_ON ) /** @req CANNM278 */
#define CanNm_GetVersionInfo(_vi) STD_GET_VERSION_INFO(_vi,CANNM) /** @req CANNM225 */
#endif /* CANNM_VERSION_INFO_API */

// Functions called by CAN Interface
// ---------------------------------

/** This service confirms a previous successfully processed CAN transmit request.
  * This callback service is called by the CanIf and implemented by the CanNm. */
/** @req CANNM228 */
void CanNm_TxConfirmation( PduIdType canNmTxPduId );

/** This service indicates a successful reception of a received NM message to the
  * CanNm after passing all filters and validation checks.
  * This callback service is called by the CAN Interface and implemented by the CanNm. */


/** @req CANNM231 */
void CanNm_RxIndication( PduIdType RxPduId, PduInfoType *PduInfoPtr );

/**
 * This function is used by the PduR to trigger a spontaneous transmission of an NM message
 * with the provided NM User Data
 */
/* @req CANNM331 */
Std_ReturnType CanNm_Transmit( PduIdType CanNmTxPduId, const PduInfoType *PduInfoPtr);

/**
 * Set the NM Coordinator Sleep Ready bit in the Control Bit Vector
 * CURRENTLY UNSUPPORTED
 */
Std_ReturnType CanNm_SetSleepReadyBit( const NetworkHandleType nmChannelHandle, const boolean nmSleepReadyBit);


/** @req 234 **/
void CanNm_MainFunction(void);
#ifdef HOST_TEST
extern void ReportErrorStatus();
void GetChannelRunTimeData(uint32 channelId, uint32* messageCycleTimeLeft, uint32* timeoutTimeLeft);
#endif

#endif /* CANNM_H */
