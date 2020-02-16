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

//lint -emacro(904, UDPNM_VALIDATE_INIT, UDPNM_VALIDATE_CHANNEL, UDPNM_VALIDATE_NOTNULL) //904 PC-Lint exception to MISRA 14.7 (validate macros).


/* Globally fulfilled requirements */
/** @req UDPNM131 */
/** @req UDPNM081 */
/** @req UDPNM044 */
/** @req UDPNM082 */
/** @req UDPNM083 */
/** @req UDPNM089 */
/** @req UDPNM092 */
/** @req UDPNM093 */
/** @req UDPNM094 */
/** @req UDPNM146 */
/** @req UDPNM243 */
/** @req UDPNM244 */
/** @req UDPNM197 */
/** @req UDPNM198 */
/** @req UDPNM199 */

#include "ComStack_Types.h" 	/** @req CANNM082 */
#include "UdpNm.h"
#include "UdpNm_Internal.h"
#include "Nm_Cbk.h"
#include "NmStack_Types.h"
//#include SchM_UdpNm.h			//Not implemented.
#include "MemMap.h"


#include "SoAd.h"
#include "Nm.h"

#include <string.h>

/** @req UDPNM188 @req UDPNM241 @req UDPNM019 */
#if (UDPNM_DEV_ERROR_DETECT == STD_ON)
#include "Det.h"				/** @req CANNM082 */
#endif

/** @req UDPNM239 @req UDPNM239 @req UDPNM242 @req UDPNM207*/
#if defined(USE_DEM)
#include "Dem.h"				/** @req CANNM082 */
#endif

static const UdpNm_ConfigType* UdpNm_ConfigPtr;

//lint -save -e785 //PC-Lint exception: Too few initializers for aggregate...
UdpNm_InternalType UdpNm_Internal = {
		.InitStatus = UDPNM_UNINIT,
};


/** Initialize the complete UdpNm module, i.e. all channels which are activated */
/** @req UDPNM141 */
/** @req UDPNM143 */
/** @req UDPNM144 */
/** @req UDPNM145 */
/** @req UDPNM060 */
/** @req UDPNM061 */
/** @req UDPNM033 */
/** @req UDPNM039 */
/** @req UDPNM025 */
/** @req UDPNM085 */
/** @req UDPNM013 */
/** @req UDPNM208 */
void UdpNm_Init( const UdpNm_ConfigType* UdpNmConfigPtr ){
	UDPNM_VALIDATE_NOTNULL(UdpNmConfigPtr, UDPNM_SERVICEID_INIT);

	UdpNm_ConfigPtr = UdpNmConfigPtr;

	uint8 channel;
	for (channel = 0; channel < UDPNM_CHANNEL_COUNT; channel++) {
		const UdpNm_ChannelType* ChannelConf = &UdpNm_ConfigPtr->Channels[channel];
		UdpNm_Internal_ChannelType* ChannelInternal = &UdpNm_Internal.Channels[channel];

		ChannelInternal->Mode = NM_MODE_BUS_SLEEP;
		ChannelInternal->State = NM_STATE_BUS_SLEEP;
		ChannelInternal->Requested = FALSE;

		memset(ChannelInternal->TxMessageSdu, 0x00, 8);
		memset(ChannelInternal->RxMessageSdu, 0x00, 8);

		uint8* destUserData = UdpNm_Internal_GetUserDataPtr(ChannelConf, ChannelInternal->TxMessageSdu);
		uint8 userDataLength = UdpNm_Internal_GetUserDataLength(ChannelConf);
		memset(destUserData, 0xFF, userDataLength);

#if (UDPNM_NODE_ID_ENABLED == STD_ON)
		if (ChannelConf->UdpNmPduNidPosition != UDPNM_PDU_OFF) {
			ChannelInternal->TxMessageSdu[ChannelConf->UdpNmPduNidPosition] = ChannelConf->UdpNmNodeId;
		}
#endif
	}
	UdpNm_Internal.InitStatus = UDPNM_INIT;
}

/** Passive startup of the AUTOSAR Udp NM. It triggers the transition from Bus-
  * Sleep Mode to the Network Mode in Repeat Message State.
  * This service has no effect if the current state is not equal to Bus-Sleep Mode. In
  * that case NM_E_NOT_EXECUTED is returned. */
/** @req UDPNM095 */
/** @req UDPNM128 */
/** @req UDPNM147 */
/** @req UDPNM211 */
/** @req UDPNM212 */
Nm_ReturnType UdpNm_PassiveStartUp( const NetworkHandleType nmChannelHandle ){
	UDPNM_VALIDATE_INIT(UDPNM_SERVICEID_PASSIVESTARTUP, NM_E_NOT_OK);
	UDPNM_VALIDATE_CHANNEL(nmChannelHandle, UDPNM_SERVICEID_PASSIVESTARTUP, NM_E_NOT_OK);

	const UdpNm_ChannelType* ChannelConf = &UdpNm_ConfigPtr->Channels[nmChannelHandle];
	UdpNm_Internal_ChannelType* ChannelInternal = &UdpNm_Internal.Channels[nmChannelHandle];
	Nm_ReturnType status = NM_E_OK;

	if (ChannelInternal->Mode == NM_MODE_BUS_SLEEP) {
		UdpNm_Internal_BusSleep_to_RepeatMessage(ChannelConf, ChannelInternal);
		status = NM_E_OK;
	} else {
		status = NM_E_NOT_EXECUTED;
	}
	return status;
}

#if ( UDPNM_PASSIVE_MODE_ENABLED == STD_OFF )
/** Request the network, since ECU needs to communicate on the bus. Network
  * state shall be changed to */
/** @req UDPNM110 */
/** @req UDPNM123 */
/** @req UDPNM129 */
/** @req UDPNM104 */
/** @req UDPNM213 */
/** @req UDPNM255 */
Nm_ReturnType UdpNm_NetworkRequest( const NetworkHandleType nmChannelHandle ){
	UDPNM_VALIDATE_INIT(UDPNM_SERVICEID_NETWORKREQUEST, NM_E_NOT_OK);
	UDPNM_VALIDATE_CHANNEL(nmChannelHandle, UDPNM_SERVICEID_NETWORKREQUEST, NM_E_NOT_OK);

	const UdpNm_ChannelType* ChannelConf = &UdpNm_ConfigPtr->Channels[nmChannelHandle];
	UdpNm_Internal_ChannelType* ChannelInternal = &UdpNm_Internal.Channels[nmChannelHandle];

	ChannelInternal->Requested = TRUE;

	if (ChannelInternal->Mode == NM_MODE_BUS_SLEEP) {
		UdpNm_Internal_BusSleep_to_RepeatMessage(ChannelConf, ChannelInternal);
	} else if (ChannelInternal->Mode == NM_MODE_PREPARE_BUS_SLEEP) {
		UdpNm_Internal_PrepareBusSleep_to_RepeatMessage(ChannelConf, ChannelInternal);
	} else if (ChannelInternal->Mode == NM_MODE_NETWORK) {
		if (ChannelInternal->State == NM_STATE_READY_SLEEP) {
			UdpNm_Internal_ReadySleep_to_NormalOperation(ChannelConf, ChannelInternal);
		}
	} else {
		//Nothing to be done
	}
	return NM_E_OK;
}

/** Release the network, since ECU doesn't have to communicate on the bus. Network
  * state shall be changed to */
/** @req UDPNM105 */
/** @req UDPNM214 */
/** @req UDPNM258 */
Nm_ReturnType UdpNm_NetworkRelease( const NetworkHandleType nmChannelHandle ){
	UDPNM_VALIDATE_INIT(UDPNM_SERVICEID_NETWORKRELEASE, NM_E_NOT_OK);
	UDPNM_VALIDATE_CHANNEL(nmChannelHandle, UDPNM_SERVICEID_NETWORKRELEASE, NM_E_NOT_OK);
	const UdpNm_ChannelType* ChannelConf = &UdpNm_ConfigPtr->Channels[nmChannelHandle];
	UdpNm_Internal_ChannelType* ChannelInternal = &UdpNm_Internal.Channels[nmChannelHandle];

	ChannelInternal->Requested = FALSE;

	if (ChannelInternal->Mode == NM_MODE_NETWORK) {
		if (ChannelInternal->State == NM_STATE_NORMAL_OPERATION) {
			UdpNm_Internal_NormalOperation_to_ReadySleep(ChannelConf, ChannelInternal);
		}
	}
	return NM_E_OK;
}
#endif

/** Disable the NM PDU transmission ability due to a ISO14229 Communication
  * Control (28hex) service */
Nm_ReturnType UdpNm_DisableCommunication( const NetworkHandleType nmChannelHandle ){
	UDPNM_VALIDATE_INIT(UDPNM_SERVICEID_DISABLECOMMUNICATION, NM_E_NOT_OK);
	UDPNM_VALIDATE_CHANNEL(nmChannelHandle, UDPNM_SERVICEID_DISABLECOMMUNICATION, NM_E_NOT_OK);
	// Not implemented
	return NM_E_NOT_OK;
}

/** Enable the NM PDU transmission ability due to a ISO14229 Communication
  * Control (28hex) service */
Nm_ReturnType UdpNm_EnableCommunication( const NetworkHandleType nmChannelHandle ){
	UDPNM_VALIDATE_INIT(UDPNM_SERVICEID_ENABLECOMMUNICATION, NM_E_NOT_OK);
	UDPNM_VALIDATE_CHANNEL(nmChannelHandle, UDPNM_SERVICEID_ENABLECOMMUNICATION, NM_E_NOT_OK);
	// Not implemented
	return NM_E_NOT_OK;
}

#if ((UDPNM_USER_DATA_ENABLED == STD_ON) && ( UDPNM_COM_USER_DATA_SUPPORT == STD_OFF))
/** Set user data for NM messages transmitted next on the bus. */
/** @req UDPNM217 */
Nm_ReturnType UdpNm_SetUserData( const NetworkHandleType nmChannelHandle, const uint8* nmUserDataPtr ){
	UDPNM_VALIDATE_INIT(UDPNM_SERVICEID_SETUSERDATA, NM_E_NOT_OK);
	UDPNM_VALIDATE_CHANNEL(nmChannelHandle, UDPNM_SERVICEID_SETUSERDATA, NM_E_NOT_OK);

	const UdpNm_ChannelType* ChannelConf = &UdpNm_ConfigPtr->Channels[nmChannelHandle];
	UdpNm_Internal_ChannelType* ChannelInternal = &UdpNm_Internal.Channels[nmChannelHandle];
	uint8* destUserData = UdpNm_Internal_GetUserDataPtr(ChannelConf, ChannelInternal->TxMessageSdu);
	uint8 userDataLength = UdpNm_Internal_GetUserDataLength(ChannelConf);

	memcpy(destUserData, nmUserDataPtr, userDataLength);

	return NM_E_OK;
}
#endif

#if (UDPNM_USER_DATA_ENABLED == STD_ON)
/** Get user data out of the most recently received NM message. */
/** @req UDPNM138 */
/** @req UDPNM139 */
/** @req UDPNM218 */
Nm_ReturnType UdpNm_GetUserData( const NetworkHandleType nmChannelHandle, uint8* const nmUserDataPtr ){
	UDPNM_VALIDATE_INIT(UDPNM_SERVICEID_GETUSERDATA, NM_E_NOT_OK);
	UDPNM_VALIDATE_CHANNEL(nmChannelHandle, UDPNM_SERVICEID_GETUSERDATA, NM_E_NOT_OK);

	const UdpNm_ChannelType* ChannelConf = &UdpNm_ConfigPtr->Channels[nmChannelHandle];
	UdpNm_Internal_ChannelType* ChannelInternal = &UdpNm_Internal.Channels[nmChannelHandle];
	uint8* sourceUserData = UdpNm_Internal_GetUserDataPtr(ChannelConf, ChannelInternal->RxMessageSdu);
	uint8 userDataLength = UdpNm_Internal_GetUserDataLength(ChannelConf);

	memcpy(nmUserDataPtr, sourceUserData, userDataLength);

	return NM_E_OK;
}
#endif

#if (UDPNM_NODE_ID_ENABLED == STD_ON)
/** Get node identifier out of the most recently received NM PDU. */
/** @req UDPNM219 */
/** @req UDPNM132 */
Nm_ReturnType UdpNm_GetNodeIdentifier( const NetworkHandleType nmChannelHandle, uint8* const nmNodeIdPtr ){
	UDPNM_VALIDATE_INIT(UDPNM_SERVICEID_GETNODEIDENTIFIER, NM_E_NOT_OK);
	UDPNM_VALIDATE_CHANNEL(nmChannelHandle, UDPNM_SERVICEID_GETNODEIDENTIFIER, NM_E_NOT_OK);

	const UdpNm_ChannelType* ChannelConf = &UdpNm_ConfigPtr->Channels[nmChannelHandle];
	UdpNm_Internal_ChannelType* ChannelInternal = &UdpNm_Internal.Channels[nmChannelHandle];
	Nm_ReturnType status = NM_E_OK;

	if (ChannelConf->UdpNmPduNidPosition == UDPNM_PDU_OFF) {
		status = NM_E_NOT_EXECUTED;
	} else {
		*nmNodeIdPtr = ChannelInternal->RxMessageSdu[ChannelConf->UdpNmPduNidPosition];
		status = NM_E_OK;
	}
	return status;
}

/** Get node identifier configured for the local node. */
/** @req UDPNM220 */
/** @req UDPNM133 */
Nm_ReturnType UdpNm_GetLocalNodeIdentifier( const NetworkHandleType nmChannelHandle, uint8* const nmNodeIdPtr ){
	UDPNM_VALIDATE_INIT(UDPNM_SERVICEID_GETLOCALNODEIDENTIFIER, NM_E_NOT_OK);
	UDPNM_VALIDATE_CHANNEL(nmChannelHandle, UDPNM_SERVICEID_GETLOCALNODEIDENTIFIER, NM_E_NOT_OK);

	const UdpNm_ChannelType* ChannelConf = &UdpNm_ConfigPtr->Channels[nmChannelHandle];
	*nmNodeIdPtr = ChannelConf->UdpNmNodeId;

	return NM_E_OK;
}
#endif

#if (UDPNM_NODE_DETECTION_ENABLED == STD_ON)
/** Set Repeat Message Request Bit for NM messages transmitted next on the bus. */
/** @req UDPNM137 */
/** @req UDPNM120 */
/** @req UDPNM121 */
/** @req UDPNM112 */
/** @req UDPNM113 */
/** @req UDPNM135 */
/** @req UDPNM221 */
Nm_ReturnType UdpNm_RepeatMessageRequest( const NetworkHandleType nmChannelHandle ){
	UDPNM_VALIDATE_INIT(UDPNM_SERVICEID_REPEATMESSAGEREQUEST, NM_E_NOT_OK);
	UDPNM_VALIDATE_CHANNEL(nmChannelHandle, UDPNM_SERVICEID_REPEATMESSAGEREQUEST, NM_E_NOT_OK);

	const UdpNm_ChannelType* ChannelConf = &UdpNm_ConfigPtr->Channels[nmChannelHandle];
	UdpNm_Internal_ChannelType* ChannelInternal = &UdpNm_Internal.Channels[nmChannelHandle];
	Nm_ReturnType status = NM_E_NOT_EXECUTED;

	if (ChannelConf->UdpNmPduCbvPosition != UDPNM_PDU_OFF) {
		if (ChannelInternal->State == NM_STATE_READY_SLEEP) {
			ChannelInternal->TxMessageSdu[ChannelConf->UdpNmPduCbvPosition] |= UDPNM_CBV_REPEAT_MESSAGE_REQUEST;
			UdpNm_Internal_ReadySleep_to_RepeatMessage(ChannelConf, ChannelInternal);
			status = NM_E_OK;
		} else if (ChannelInternal->State == NM_STATE_NORMAL_OPERATION) {
			ChannelInternal->TxMessageSdu[ChannelConf->UdpNmPduCbvPosition] |= UDPNM_CBV_REPEAT_MESSAGE_REQUEST;
			UdpNm_Internal_NormalOperation_to_RepeatMessage(ChannelConf, ChannelInternal);
			status = NM_E_OK;
		} else {
			//Nothing to be done
		}
	}
	return status;
}
#endif

#if ((UDPNM_NODE_ID_ENABLED == STD_ON) || (UDPNM_NODE_DETECTION_ENABLED == STD_ON) || (UDPNM_USER_DATA_ENABLED == STD_ON))
/** Get the whole PDU data out of the most recently received NM message. */
/** @req UDPNM309 */
Nm_ReturnType UdpNm_GetPduData( const NetworkHandleType nmChannelHandle, uint8* const nmPduDataPtr ){
	UDPNM_VALIDATE_INIT(UDPNM_SERVICEID_GETPDUDATA, NM_E_NOT_OK);
	UDPNM_VALIDATE_CHANNEL(nmChannelHandle, UDPNM_SERVICEID_GETPDUDATA, NM_E_NOT_OK);

	const UdpNm_ChannelType* ChannelConf = &UdpNm_ConfigPtr->Channels[nmChannelHandle];
	UdpNm_Internal_ChannelType* ChannelInternal = &UdpNm_Internal.Channels[nmChannelHandle];

	memcpy(nmPduDataPtr, ChannelInternal->RxMessageSdu, ChannelConf->UdpNmPduLength);

	return NM_E_OK;
}
#endif

/** Returns the state and the mode of the network management. */
/** @req UDPNM310 */
Nm_ReturnType UdpNm_GetState( const NetworkHandleType nmChannelHandle, Nm_StateType* const nmStatePtr, Nm_ModeType* const nmModePtr ){
	UDPNM_VALIDATE_INIT(UDPNM_SERVICEID_GETSTATE, NM_E_NOT_OK);
	UDPNM_VALIDATE_CHANNEL(nmChannelHandle, UDPNM_SERVICEID_GETSTATE, NM_E_NOT_OK);

	UdpNm_Internal_ChannelType* ChannelInternal = &UdpNm_Internal.Channels[nmChannelHandle];
	*nmStatePtr = ChannelInternal->State;
	*nmModePtr = ChannelInternal->Mode;

	return NM_E_OK;
}

/** Request bus synchronization. */
Nm_ReturnType UdpNm_RequestBusSynchronization( const NetworkHandleType nmChannelHandle ){
	UDPNM_VALIDATE_INIT(UDPNM_SERVICEID_REQUESTBUSSYNCHRONIZATION, NM_E_NOT_OK);
	UDPNM_VALIDATE_CHANNEL(nmChannelHandle, UDPNM_SERVICEID_REQUESTBUSSYNCHRONIZATION, NM_E_NOT_OK);
	// Not implemented
	return NM_E_NOT_OK;
}

/** Check if remote sleep indication takes place or not. */
Nm_ReturnType UdpNm_CheckRemoteSleepIndication( const NetworkHandleType nmChannelHandle, boolean* const nmRemoteSleepIndPtr ){
	UDPNM_VALIDATE_INIT(UDPNM_SERVICEID_CHECKREMOTESLEEPINDICATION, NM_E_NOT_OK);
	UDPNM_VALIDATE_CHANNEL(nmChannelHandle, UDPNM_SERVICEID_CHECKREMOTESLEEPINDICATION, NM_E_NOT_OK);
	(void)nmRemoteSleepIndPtr;
	// Not implemented
	return NM_E_NOT_OK;
}

/** Sets the NM coordinator ID in the control bit vector of each NM message. */
Nm_ReturnType UdpNm_SetCoordBits( const NetworkHandleType nmChannelHandle, const uint8 nmCoordBits ){
	UDPNM_VALIDATE_INIT(UDPNM_SERVICEID_CHECKREMOTESLEEPINDICATION, NM_E_NOT_OK);
	UDPNM_VALIDATE_CHANNEL(nmChannelHandle, UDPNM_SERVICEID_CHECKREMOTESLEEPINDICATION, NM_E_NOT_OK);
	// Not implemented
	return NM_E_NOT_OK;
}

// Functions called by SoAd
// ---------------------------------

/** This service confirms a previous successfully processed transmit request.
  * This callback service is called by the SoAd and implemented by the UdpNm. */
/** @req UDPNM099 */
/** @req UDPNM228 */
/** @req UDPNM229 */
/** @req UDPNM230 */
void UdpNm_SoAdIfTxConfirmation( PduIdType UdpNmTxPduId ){
	UDPNM_VALIDATE_INIT(UDPNM_SERVICEID_SOADIFTXCONFIRMATION);
	UDPNM_VALIDATE_CHANNEL(UdpNmTxPduId, UDPNM_SERVICEID_SOADIFTXCONFIRMATION);

	const UdpNm_ChannelType* ChannelConf = &UdpNm_ConfigPtr->Channels[UdpNmTxPduId];
	UdpNm_Internal_ChannelType* ChannelInternal = &UdpNm_Internal.Channels[UdpNmTxPduId];

	if (ChannelInternal->Mode == NM_MODE_NETWORK) {
		UdpNm_Internal_NetworkMode_to_NetworkMode(ChannelConf, ChannelInternal);
	}

	/* Message Timeout */
	ChannelInternal->MessageTimeoutTimeLeft = ChannelConf->UdpNmMsgTimeoutTime;
}

/** This service indicates a successful reception of a received NM message to the
  * UdpNm after passing all filters and validation checks.
  * This callback service is called by the SoAd and implemented by the UdpNm. */
/** @req UDPNM098 */
/** @req UDPNM119 */
/** @req UDPNM111 */
/** @req UDPNM124 */
/** @req UDPNM035 */
/** @req UDPNM037 */
/** @req UDPNM164 */
/** @req UDPNM165 */
/** @req UDPNM231 */
/** @req UDPNM232 partial */
/** @req UDPNM233 */
void UdpNm_SoAdIfRxIndication( PduIdType udpNmRxPduId, const uint8* udpSduPtr ){
	UDPNM_VALIDATE_INIT(UDPNM_SERVICEID_SOADIFRXINDICATION);
	UDPNM_VALIDATE_CHANNEL(udpNmRxPduId, UDPNM_SERVICEID_SOADIFRXINDICATION);

	const UdpNm_ChannelType* ChannelConf = &UdpNm_ConfigPtr->Channels[udpNmRxPduId];
	UdpNm_Internal_ChannelType* ChannelInternal = &UdpNm_Internal.Channels[udpNmRxPduId];

	memcpy(ChannelInternal->RxMessageSdu, udpSduPtr, ChannelConf->UdpNmPduLength);

	boolean repeatMessageBitIndication = FALSE;
	if (ChannelConf->UdpNmPduCbvPosition != UDPNM_PDU_OFF) {
		uint8 cbv = ChannelInternal->RxMessageSdu[ChannelConf->UdpNmPduCbvPosition];
		repeatMessageBitIndication = cbv & UDPNM_CBV_REPEAT_MESSAGE_REQUEST;
	}

	if (ChannelInternal->Mode == NM_MODE_BUS_SLEEP) {
		UdpNm_Internal_BusSleep_to_BusSleep(ChannelConf, ChannelInternal);
	} else if (ChannelInternal->Mode == NM_MODE_PREPARE_BUS_SLEEP) {
		UdpNm_Internal_PrepareBusSleep_to_RepeatMessage(ChannelConf, ChannelInternal);
	} else if (ChannelInternal->Mode == NM_MODE_NETWORK) {
		UdpNm_Internal_NetworkMode_to_NetworkMode(ChannelConf, ChannelInternal);
		if (repeatMessageBitIndication) {
			if (ChannelInternal->State == NM_STATE_READY_SLEEP) {
				UdpNm_Internal_ReadySleep_to_RepeatMessage(ChannelConf, ChannelInternal);
			} else if (ChannelInternal->State == NM_STATE_NORMAL_OPERATION) {
				UdpNm_Internal_NormalOperation_to_RepeatMessage(ChannelConf, ChannelInternal);
			} else {
				//Nothing to be done
			}
		}
	} else {
		//Nothing to be done
	}
#if (UDPNM_PDU_RX_INDICATION_ENABLED == STD_ON)
	// TODO: call NM rx indication
#endif
	/* Message Timeout */
	ChannelInternal->MessageTimeoutTimeLeft = ChannelConf->UdpNmMsgTimeoutTime;
}

/** UdpNm_Transmit is implemented as an empty function and shall always return E_OK. */
/** @req UDPNM313 */
/** @req UDPNM315 */
Std_ReturnType UdpNm_Transmit( PduIdType UdpNmSrcPduId, const PduInfoType* UdpNmSrcPduInfoPtr ){
	return E_NOT_OK;
}

// ----------------------------------------------------------------------------
// Internal functions
// ----------------------------------------------------------------------------

// Accessed through UdpNm_MainFunction_<channel>
/** @req UDPNM193 */
/** @req UDPNM194 */
/** @req UDPNM087 */
/** @req UDPNM088 */
/** @req UDPNM116 */
/** @req UDPNM108 */
/** @req UDPNM072 */
/** @req UDPNM237 */
/** @req UDPNM051 */
/** @req UDPNM161 */
/** @req UDPNM162 */
/** @req UDPNM077 */
/** @req UDPNM234 */
void UdpNm_MainFunction( NetworkHandleType nmChannelHandle ) {
	UDPNM_VALIDATE_INIT(UDPNM_SERVICEID_ARC_MAINFUNCTION);

	const UdpNm_ChannelType* ChannelConf = &UdpNm_ConfigPtr->Channels[nmChannelHandle];
	UdpNm_Internal_ChannelType* ChannelInternal = &UdpNm_Internal.Channels[nmChannelHandle];
	if (ChannelInternal->Mode == NM_MODE_NETWORK) {
		UdpNm_Internal_TickTimeoutTime(ChannelConf, ChannelInternal);
		UdpNm_Internal_TickMessageTimeoutTime(ChannelConf, ChannelInternal);
#if (UDPNM_PASSIVE_MODE_ENABLED != STD_ON)
		if ((ChannelInternal->State == NM_STATE_REPEAT_MESSAGE) ||
			(ChannelInternal->State == NM_STATE_NORMAL_OPERATION)) {

			UdpNm_Internal_TickMessageCycleTime(ChannelConf, ChannelInternal);
		}
#endif
		if (ChannelInternal->State == NM_STATE_REPEAT_MESSAGE) {
			UdpNm_Internal_TickRepeatMessageTime(ChannelConf, ChannelInternal);
		}
	} else if (ChannelInternal->Mode == NM_MODE_PREPARE_BUS_SLEEP) {
			UdpNm_Internal_TickWaitBusSleepTime(ChannelConf, ChannelInternal);
	} else {
		//Nothing to be done
	}
}

// Timer helpers
// -------------

/** @req UDPNM206 */
/** @req UDPNM117 */
static inline void UdpNm_Internal_TickTimeoutTime( const UdpNm_ChannelType* ChannelConf, UdpNm_Internal_ChannelType* ChannelInternal ){
	if (ChannelConf->UdpNmMainFunctionPeriod >= ChannelInternal->TimeoutTimeLeft) {
		ChannelInternal->TimeoutTimeLeft = 0;
		if (ChannelInternal->State == NM_STATE_REPEAT_MESSAGE) {
#if defined(USE_DEM)
			Dem_ReportErrorStatus(UDPNM_E_NETWORK_TIMEOUT, DEM_EVENT_STATUS_FAILED);
#endif
			UdpNm_Internal_RepeatMessage_to_RepeatMessage(ChannelConf, ChannelInternal);
		} else if (ChannelInternal->State == NM_STATE_NORMAL_OPERATION) {
#if defined(USE_DEM)
			Dem_ReportErrorStatus(UDPNM_E_NETWORK_TIMEOUT, DEM_EVENT_STATUS_FAILED);
#endif
			UdpNm_Internal_NormalOperation_to_NormalOperation(ChannelConf, ChannelInternal);
		} else if (ChannelInternal->State == NM_STATE_READY_SLEEP) {
			UdpNm_Internal_ReadySleep_to_PrepareBusSleep(ChannelConf, ChannelInternal);
		} else {
			//Nothing to be done
		}
	} else {
		ChannelInternal->TimeoutTimeLeft -= ChannelConf->UdpNmMainFunctionPeriod;
	}
}

static inline void UdpNm_Internal_TickMessageTimeoutTime( const UdpNm_ChannelType* ChannelConf, UdpNm_Internal_ChannelType* ChannelInternal ){
	if (ChannelConf->UdpNmMainFunctionPeriod >= ChannelInternal->MessageTimeoutTimeLeft) {
		//Nm_TxTimeoutException(ChannelConf->UdpNmChannelIdRef);	//This NM API is not supported yet
	} else {
		/* Message Timeout */
		ChannelInternal->MessageTimeoutTimeLeft -= ChannelConf->UdpNmMainFunctionPeriod;
	}
}

/** @req UDPNM102 */
/** @req UDPNM103 */
/** @req UDPNM106 */
/** @req UDPNM109 */
static inline void UdpNm_Internal_TickRepeatMessageTime( const UdpNm_ChannelType* ChannelConf, UdpNm_Internal_ChannelType* ChannelInternal ){
	if (ChannelConf->UdpNmMainFunctionPeriod >= ChannelInternal->RepeatMessageTimeLeft) {
		ChannelInternal->RepeatMessageTimeLeft = 0;
		if (ChannelInternal->State == NM_STATE_REPEAT_MESSAGE) {
			if (ChannelInternal->Requested) {
				UdpNm_Internal_RepeatMessage_to_NormalOperation(ChannelConf, ChannelInternal);
			} else {
				UdpNm_Internal_RepeatMessage_to_ReadySleep(ChannelConf, ChannelInternal);
			}
		}
	} else {
		ChannelInternal->RepeatMessageTimeLeft -= ChannelConf->UdpNmMainFunctionPeriod;
	}
}

/** @req UDPNM115 */
static inline void UdpNm_Internal_TickWaitBusSleepTime( const UdpNm_ChannelType* ChannelConf, UdpNm_Internal_ChannelType* ChannelInternal ){
	if (ChannelConf->UdpNmMainFunctionPeriod >= ChannelInternal->WaitBusSleepTimeLeft) {
		ChannelInternal->WaitBusSleepTimeLeft = 0;
		if (ChannelInternal->Mode == NM_MODE_PREPARE_BUS_SLEEP) {
			UdpNm_Internal_PrepareBusSleep_to_BusSleep(ChannelConf, ChannelInternal);
		}
	} else {
		ChannelInternal->WaitBusSleepTimeLeft -= ChannelConf->UdpNmMainFunctionPeriod;
	}
}

/** @req UDPNM101 */
/** @req UDPNM005 */
/** @req UDPNM040 */
static inline void UdpNm_Internal_TickMessageCycleTime( const UdpNm_ChannelType* ChannelConf, UdpNm_Internal_ChannelType* ChannelInternal ){
	if (ChannelConf->UdpNmMainFunctionPeriod >= ChannelInternal->MessageCycleTimeLeft) {
		ChannelInternal->MessageCycleTimeLeft = 0;
		if ((ChannelInternal->State == NM_STATE_REPEAT_MESSAGE) ||
			(ChannelInternal->State == NM_STATE_NORMAL_OPERATION)) {
			UdpNm_Internal_TransmitMessage(ChannelConf, ChannelInternal);
			ChannelInternal->MessageCycleTimeLeft = ChannelConf->UdpNmMsgCycleTime;
			ChannelInternal->MessageTimeoutTimeLeft = ChannelConf->UdpNmMsgTimeoutTime;
		}
	} else {
		ChannelInternal->MessageCycleTimeLeft -= ChannelConf->UdpNmMainFunctionPeriod;
	}
}

// Message helpers
// ---------------

/** @req UDPNM032 */
static inline void UdpNm_Internal_TransmitMessage( const UdpNm_ChannelType* ChannelConf, UdpNm_Internal_ChannelType* ChannelInternal ){
	PduInfoType pdu = {
			.SduDataPtr = ChannelInternal->TxMessageSdu,
			.SduLength = ChannelConf->UdpNmPduLength,
	};
	Std_ReturnType status = SoAdIf_Transmit(ChannelConf->UdpNmTxPduId, &pdu);

	if (status != E_OK){
#if defined(USE_DEM)
		Dem_ReportErrorStatus(UDPNM_E_TCPIP_TRANSMIT_ERROR, DEM_EVENT_STATUS_FAILED);
#endif
	}
}

static inline uint8 UdpNm_Internal_GetUserDataOffset( const UdpNm_ChannelType* ChannelConf ) {
	uint8 userDataPos = 0;
	userDataPos += (ChannelConf->UdpNmPduNidPosition == UDPNM_PDU_OFF) ? 0 : 1;
	userDataPos += (ChannelConf->UdpNmPduCbvPosition == UDPNM_PDU_OFF) ? 0 : 1;
	return userDataPos;
}

static inline uint8* UdpNm_Internal_GetUserDataPtr( const UdpNm_ChannelType* ChannelConf, uint8* MessageSduPtr ) {
	uint8 userDataOffset = UdpNm_Internal_GetUserDataOffset(ChannelConf);
	return &MessageSduPtr[userDataOffset];
}

static inline uint8 UdpNm_Internal_GetUserDataLength( const UdpNm_ChannelType* ChannelConf ) {
	uint8 userDataOffset = UdpNm_Internal_GetUserDataOffset(ChannelConf);
	return ChannelConf->UdpNmPduLength - userDataOffset;
}

static inline void UdpNm_Internal_ClearCbv( const UdpNm_ChannelType* ChannelConf, UdpNm_Internal_ChannelType* ChannelInternal ){
	if (ChannelConf->UdpNmPduCbvPosition != UDPNM_PDU_OFF) {
		ChannelInternal->TxMessageSdu[ChannelConf->UdpNmPduCbvPosition] = 0x00;
	}
}

// Transition helpers
// ------------------

/** @req UDPNM096 */
/** @req UDPNM097 */
/** @req UDPNM100 */
static inline void UdpNm_Internal_PrepareBusSleep_to_RepeatMessage( const UdpNm_ChannelType* ChannelConf, UdpNm_Internal_ChannelType* ChannelInternal ) {
	ChannelInternal->Mode = NM_MODE_NETWORK;
	ChannelInternal->State = NM_STATE_REPEAT_MESSAGE;
	ChannelInternal->TimeoutTimeLeft = ChannelConf->UdpNmTimeoutTime;
	ChannelInternal->RepeatMessageTimeLeft = ChannelConf->UdpNmRepeatMessageTime;
	ChannelInternal->MessageCycleTimeLeft = ChannelConf->UdpNmMsgCycleOffset;
	// Notify 'Network Mode'
	Nm_NetworkMode(ChannelConf->UdpNmChannelIdRef);
}

/** @req UDPNM126 */
static inline void UdpNm_Internal_PrepareBusSleep_to_BusSleep( const UdpNm_ChannelType* ChannelConf, UdpNm_Internal_ChannelType* ChannelInternal ){
	ChannelInternal->Mode = NM_MODE_BUS_SLEEP;
	ChannelInternal->State = NM_STATE_BUS_SLEEP;
	// Notify 'Bus-Sleep Mode'
	Nm_BusSleepMode(ChannelConf->UdpNmChannelIdRef);
}

static inline void UdpNm_Internal_BusSleep_to_RepeatMessage( const UdpNm_ChannelType* ChannelConf, UdpNm_Internal_ChannelType* ChannelInternal ) {
	ChannelInternal->Mode = NM_MODE_NETWORK;
	ChannelInternal->State = NM_STATE_REPEAT_MESSAGE;
	ChannelInternal->TimeoutTimeLeft = ChannelConf->UdpNmTimeoutTime;
	ChannelInternal->RepeatMessageTimeLeft = ChannelConf->UdpNmRepeatMessageTime;
	ChannelInternal->MessageCycleTimeLeft = ChannelConf->UdpNmMsgCycleOffset;
	// Notify 'Network Mode'
	Nm_NetworkMode(ChannelConf->UdpNmChannelIdRef);
}

/** @req UDPNM127 */
static inline void UdpNm_Internal_BusSleep_to_BusSleep( const UdpNm_ChannelType* ChannelConf, UdpNm_Internal_ChannelType* ChannelInternal ){
    Nm_NetworkStartIndication(ChannelConf->UdpNmChannelIdRef);
}

static inline void UdpNm_Internal_RepeatMessage_to_RepeatMessage( const UdpNm_ChannelType* ChannelConf, UdpNm_Internal_ChannelType* ChannelInternal ){
	ChannelInternal->TimeoutTimeLeft = ChannelConf->UdpNmTimeoutTime;
}

/** @req UDPNM107 */
static inline void UdpNm_Internal_RepeatMessage_to_ReadySleep( const UdpNm_ChannelType* ChannelConf, UdpNm_Internal_ChannelType* ChannelInternal ){
	ChannelInternal->Mode = NM_MODE_NETWORK;
	ChannelInternal->State = NM_STATE_READY_SLEEP;
#if (UDPNM_NODE_DETECTION_ENABLED == STD_ON)
	UdpNm_Internal_ClearCbv(ChannelConf, ChannelInternal);
#endif
}

static inline void UdpNm_Internal_RepeatMessage_to_NormalOperation( const UdpNm_ChannelType* ChannelConf, UdpNm_Internal_ChannelType* ChannelInternal ){
	ChannelInternal->Mode = NM_MODE_NETWORK;
	ChannelInternal->State = NM_STATE_NORMAL_OPERATION;
#if (UDPNM_NODE_DETECTION_ENABLED == STD_ON)
	UdpNm_Internal_ClearCbv(ChannelConf, ChannelInternal);
#endif
}

static inline void UdpNm_Internal_NormalOperation_to_RepeatMessage( const UdpNm_ChannelType* ChannelConf, UdpNm_Internal_ChannelType* ChannelInternal ){
	ChannelInternal->Mode = NM_MODE_NETWORK;
	ChannelInternal->State = NM_STATE_REPEAT_MESSAGE;
	ChannelInternal->RepeatMessageTimeLeft = ChannelConf->UdpNmRepeatMessageTime;
	ChannelInternal->MessageCycleTimeLeft = ChannelConf->UdpNmMsgCycleOffset;
}

/** @req UDPNM118 */
static inline void UdpNm_Internal_NormalOperation_to_ReadySleep( const UdpNm_ChannelType* ChannelConf, UdpNm_Internal_ChannelType* ChannelInternal ){
	ChannelInternal->Mode = NM_MODE_NETWORK;
	ChannelInternal->State = NM_STATE_READY_SLEEP;
}

static inline void UdpNm_Internal_NormalOperation_to_NormalOperation( const UdpNm_ChannelType* ChannelConf, UdpNm_Internal_ChannelType* ChannelInternal ){
	ChannelInternal->TimeoutTimeLeft = ChannelConf->UdpNmTimeoutTime;
}

static inline void UdpNm_Internal_ReadySleep_to_NormalOperation( const UdpNm_ChannelType* ChannelConf, UdpNm_Internal_ChannelType* ChannelInternal ){
	ChannelInternal->Mode = NM_MODE_NETWORK;
	ChannelInternal->State = NM_STATE_NORMAL_OPERATION;
	ChannelInternal->MessageCycleTimeLeft = ChannelConf->UdpNmMsgCycleOffset;
}

/** @req UDPNM114 */
static inline void UdpNm_Internal_ReadySleep_to_PrepareBusSleep( const UdpNm_ChannelType* ChannelConf, UdpNm_Internal_ChannelType* ChannelInternal ){
	ChannelInternal->Mode = NM_MODE_PREPARE_BUS_SLEEP;
	ChannelInternal->State = NM_STATE_PREPARE_BUS_SLEEP;
	ChannelInternal->WaitBusSleepTimeLeft = ChannelConf->UdpNmWaitBusSleepTime;
	// Notify 'Prepare Bus-Sleep Mode'
	Nm_PrepareBusSleepMode(ChannelConf->UdpNmChannelIdRef);
}

static inline void UdpNm_Internal_ReadySleep_to_RepeatMessage( const UdpNm_ChannelType* ChannelConf, UdpNm_Internal_ChannelType* ChannelInternal ){
	ChannelInternal->Mode = NM_MODE_NETWORK;
	ChannelInternal->State = NM_STATE_REPEAT_MESSAGE;
	ChannelInternal->RepeatMessageTimeLeft = ChannelConf->UdpNmRepeatMessageTime;
	ChannelInternal->MessageCycleTimeLeft = ChannelConf->UdpNmMsgCycleOffset;
}

static inline void UdpNm_Internal_NetworkMode_to_NetworkMode( const UdpNm_ChannelType* ChannelConf, UdpNm_Internal_ChannelType* ChannelInternal ){
	ChannelInternal->TimeoutTimeLeft = ChannelConf->UdpNmTimeoutTime;
}

