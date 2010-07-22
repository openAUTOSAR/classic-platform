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


#if (CANNM_DEV_ERROR_DETECT == STD_ON)
#include "Det.h"
#endif
#include "ComStack_Types.h"
#include "CanNm.h"
#include "CanNm_Internal.h"
#include "CanNm_ConfigTypes.h"
#include "Nm_Cbk.h"
#include "NmStack_Types.h"
#include "MemMap.h"
#include "CanIf.h"
#include "Nm.h"

#include <string.h>

static const CanNm_ConfigType* CanNm_ConfigPtr;

CanNm_InternalType CanNm_Internal = {
		.InitStatus = CANNM_UNINIT,
};

/** Initialize the complete CanNm module, i.e. all channels which are activated */
void CanNm_Init( const CanNm_ConfigType * const cannmConfigPtr ){
	CANNM_VALIDATE_NOTNULL(cannmConfigPtr, CANNM_SERVICEID_INIT);

	CanNm_ConfigPtr = cannmConfigPtr;

	int channel;
	for (channel = 0; channel < CANNM_CHANNEL_COUNT; channel++) {
		const CanNm_ChannelType* ChannelConf = &CanNm_ConfigPtr->Channels[channel];
		CanNm_Internal_ChannelType* ChannelInternal = &CanNm_Internal.Channels[channel];
		if (ChannelConf->Active) {
			ChannelInternal->Mode = NM_MODE_BUS_SLEEP;
			ChannelInternal->State = NM_STATE_BUS_SLEEP;
			ChannelInternal->Requested = FALSE;

			memset(ChannelInternal->TxMessageSdu, 0x00, 8);
			memset(ChannelInternal->RxMessageSdu, 0x00, 8);

#if (CANNM_NODE_ID_ENABLED == STD_ON)
			if (ChannelConf->NidPosition != CANNM_PDU_OFF) {
				ChannelInternal->TxMessageSdu[ChannelConf->NidPosition] = ChannelConf->NodeId;
			}
#endif
		} else {
			ChannelInternal->State = NM_STATE_UNINIT;
		}
	}

	CanNm_Internal.InitStatus = CANNM_INIT;
}

/** Passive startup of the AUTOSAR CAN NM. It triggers the transition from Bus-
  * Sleep Mode to the Network Mode in Repeat Message State.
  * This service has no effect if the current state is not equal to Bus-Sleep Mode. In
  * that case NM_E_NOT_EXECUTED is returned. */
Nm_ReturnType CanNm_PassiveStartUp( const NetworkHandleType nmChannelHandle ){
	CANNM_VALIDATE_INIT(CANNM_SERVICEID_PASSIVESTARTUP, NM_E_NOT_OK);
	CANNM_VALIDATE_CHANNEL(nmChannelHandle, CANNM_SERVICEID_PASSIVESTARTUP, NM_E_NOT_OK);

	const CanNm_ChannelType* ChannelConf = &CanNm_ConfigPtr->Channels[nmChannelHandle];
	CanNm_Internal_ChannelType* ChannelInternal = &CanNm_Internal.Channels[nmChannelHandle];

	if (ChannelInternal->Mode == NM_MODE_BUS_SLEEP) {
		CanNm_Internal_BusSleep_to_RepeatMessage(ChannelConf, ChannelInternal);
		return NM_E_OK;
	} else {
		return NM_E_NOT_EXECUTED;
	}
}

/** Request the network, since ECU needs to communicate on the bus. Network
  * state shall be changed to ‘requested’ */
Nm_ReturnType CanNm_NetworkRequest( const NetworkHandleType nmChannelHandle ){
	CANNM_VALIDATE_INIT(CANNM_SERVICEID_NETWORKREQUEST, NM_E_NOT_OK);
	CANNM_VALIDATE_CHANNEL(nmChannelHandle, CANNM_SERVICEID_NETWORKREQUEST, NM_E_NOT_OK);

	const CanNm_ChannelType* ChannelConf = &CanNm_ConfigPtr->Channels[nmChannelHandle];
	CanNm_Internal_ChannelType* ChannelInternal = &CanNm_Internal.Channels[nmChannelHandle];

	ChannelInternal->Requested = TRUE;

	if (ChannelInternal->Mode == NM_MODE_BUS_SLEEP) {
		CanNm_Internal_BusSleep_to_RepeatMessage(ChannelConf, ChannelInternal);
	} else if (ChannelInternal->Mode == NM_MODE_PREPARE_BUS_SLEEP) {
		CanNm_Internal_PrepareBusSleep_to_RepeatMessage(ChannelConf, ChannelInternal);
	} else if (ChannelInternal->Mode == NM_MODE_NETWORK) {
		if (ChannelInternal->State == NM_STATE_READY_SLEEP) {
			CanNm_Internal_ReadySleep_to_NormalOperation(ChannelConf, ChannelInternal);
		}
	}
	return NM_E_OK;
}

/** Release the network, since ECU doesn’t have to communicate on the bus. Network
  * state shall be changed to ‘released’. */
Nm_ReturnType CanNm_NetworkRelease( const NetworkHandleType nmChannelHandle ){
	CANNM_VALIDATE_INIT(CANNM_SERVICEID_NETWORKRELEASE, NM_E_NOT_OK);
	CANNM_VALIDATE_CHANNEL(nmChannelHandle, CANNM_SERVICEID_NETWORKRELEASE, NM_E_NOT_OK);
	const CanNm_ChannelType* ChannelConf = &CanNm_ConfigPtr->Channels[nmChannelHandle];
	CanNm_Internal_ChannelType* ChannelInternal = &CanNm_Internal.Channels[nmChannelHandle];

	ChannelInternal->Requested = FALSE;

	if (ChannelInternal->Mode == NM_MODE_NETWORK) {
		if (ChannelInternal->State == NM_STATE_NORMAL_OPERATION) {
			CanNm_Internal_NormalOperation_to_ReadySleep(ChannelConf, ChannelInternal);
		}
	}
	return NM_E_OK;
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

	const CanNm_ChannelType* ChannelConf = &CanNm_ConfigPtr->Channels[nmChannelHandle];
	CanNm_Internal_ChannelType* ChannelInternal = &CanNm_Internal.Channels[nmChannelHandle];
	uint8* destUserData = CanNm_Internal_GetUserDataPtr(ChannelConf, ChannelInternal->TxMessageSdu);
	uint8 userDataLength = CanNm_Internal_GetUserDataLength(ChannelConf);

	memcpy(destUserData, nmUserDataPtr, userDataLength);

	return NM_E_OK;
}

/** Get user data out of the most recently received NM message. */
Nm_ReturnType CanNm_GetUserData( const NetworkHandleType nmChannelHandle, uint8* const nmUserDataPtr ){
	CANNM_VALIDATE_INIT(CANNM_SERVICEID_GETUSERDATA, NM_E_NOT_OK);
	CANNM_VALIDATE_CHANNEL(nmChannelHandle, CANNM_SERVICEID_GETUSERDATA, NM_E_NOT_OK);

	const CanNm_ChannelType* ChannelConf = &CanNm_ConfigPtr->Channels[nmChannelHandle];
	CanNm_Internal_ChannelType* ChannelInternal = &CanNm_Internal.Channels[nmChannelHandle];
	uint8* sourceUserData = CanNm_Internal_GetUserDataPtr(ChannelConf, ChannelInternal->RxMessageSdu);
	uint8 userDataLength = CanNm_Internal_GetUserDataLength(ChannelConf);

	memcpy(nmUserDataPtr, sourceUserData, userDataLength);

	return NM_E_OK;
}

#if (CANNM_NODE_ID_ENABLED == STD_ON)
/** Get node identifier out of the most recently received NM PDU. */
Nm_ReturnType CanNm_GetNodeIdentifier( const NetworkHandleType nmChannelHandle, uint8 * const nmNodeIdPtr ){
	CANNM_VALIDATE_INIT(CANNM_SERVICEID_GETNODEIDENTIFIER, NM_E_NOT_OK);
	CANNM_VALIDATE_CHANNEL(nmChannelHandle, CANNM_SERVICEID_GETNODEIDENTIFIER, NM_E_NOT_OK);

	const CanNm_ChannelType* ChannelConf = &CanNm_ConfigPtr->Channels[nmChannelHandle];
	CanNm_Internal_ChannelType* ChannelInternal = &CanNm_Internal.Channels[nmChannelHandle];

	if (ChannelConf->NidPosition == CANNM_PDU_OFF) {
		return NM_E_NOT_EXECUTED;
	} else {
		*nmNodeIdPtr = ChannelInternal->RxMessageSdu[ChannelConf->NidPosition];
		return NM_E_OK;
	}
}

/** Get node identifier configured for the local node. */
Nm_ReturnType CanNm_GetLocalNodeIdentifier( const NetworkHandleType nmChannelHandle, uint8 * const nmNodeIdPtr ){
	CANNM_VALIDATE_INIT(CANNM_SERVICEID_GETLOCALNODEIDENTIFIER, NM_E_NOT_OK);
	CANNM_VALIDATE_CHANNEL(nmChannelHandle, CANNM_SERVICEID_GETLOCALNODEIDENTIFIER, NM_E_NOT_OK);

	const CanNm_ChannelType* ChannelConf = &CanNm_ConfigPtr->Channels[nmChannelHandle];
	*nmNodeIdPtr = ChannelConf->NodeId;

	return NM_E_OK;
}
#endif

#if (CANNM_NODE_DETECTION_ENABLED == STD_ON)
/** Set Repeat Message Request Bit for NM messages transmitted next on the bus. */
Nm_ReturnType CanNm_RepeatMessageRequest( const NetworkHandleType nmChannelHandle ){
	CANNM_VALIDATE_INIT(CANNM_SERVICEID_REPEATMESSAGEREQUEST, NM_E_NOT_OK);
	CANNM_VALIDATE_CHANNEL(nmChannelHandle, CANNM_SERVICEID_REPEATMESSAGEREQUEST, NM_E_NOT_OK);

	const CanNm_ChannelType* ChannelConf = &CanNm_ConfigPtr->Channels[nmChannelHandle];
	CanNm_Internal_ChannelType* ChannelInternal = &CanNm_Internal.Channels[nmChannelHandle];

	if (ChannelConf->CbvPosition != CANNM_PDU_OFF) {
		if (ChannelInternal->State == NM_STATE_READY_SLEEP) {
			ChannelInternal->TxMessageSdu[ChannelConf->CbvPosition] = CANNM_CBV_REPEAT_MESSAGE_REQUEST;
			CanNm_Internal_ReadySleep_to_RepeatMessage(ChannelConf, ChannelInternal);
		} else if (ChannelInternal->State == NM_STATE_NORMAL_OPERATION) {
			ChannelInternal->TxMessageSdu[ChannelConf->CbvPosition] = CANNM_CBV_REPEAT_MESSAGE_REQUEST;
			CanNm_Internal_NormalOperation_to_RepeatMessage(ChannelConf, ChannelInternal);
		}
		return NM_E_OK;
	} else {
		return NM_E_NOT_EXECUTED;
	}
}
#endif

#if ((CANNM_NODE_ID_ENABLED == STD_ON) || (CANNM_NODE_DETECTION_ENABLED == STD_ON) || (CANNM_USER_DATA_ENABLED == STD_ON))
/** Get the whole PDU data out of the most recently received NM message. */
Nm_ReturnType CanNm_GetPduData( const NetworkHandleType nmChannelHandle, uint8 * const nmPduDataPtr ){
	CANNM_VALIDATE_INIT(CANNM_SERVICEID_GETPDUDATA, NM_E_NOT_OK);
	CANNM_VALIDATE_CHANNEL(nmChannelHandle, CANNM_SERVICEID_GETPDUDATA, NM_E_NOT_OK);

	const CanNm_ChannelType* ChannelConf = &CanNm_ConfigPtr->Channels[nmChannelHandle];
	CanNm_Internal_ChannelType* ChannelInternal = &CanNm_Internal.Channels[nmChannelHandle];

	memcpy(nmPduDataPtr, ChannelInternal->RxMessageSdu, ChannelConf->PduLength);

	return NM_E_OK;
}
#endif

/** Returns the state and the mode of the network management. */
Nm_ReturnType CanNm_GetState( const NetworkHandleType nmChannelHandle, Nm_StateType * const nmStatePtr, Nm_ModeType * const nmModePtr ){
	CANNM_VALIDATE_INIT(CANNM_SERVICEID_GETSTATE, NM_E_NOT_OK);
	CANNM_VALIDATE_CHANNEL(nmChannelHandle, CANNM_SERVICEID_GETSTATE, NM_E_NOT_OK);

	CanNm_Internal_ChannelType* ChannelInternal = &CanNm_Internal.Channels[nmChannelHandle];
	*nmStatePtr = ChannelInternal->State;
	*nmModePtr = ChannelInternal->Mode;

	return NM_E_OK;
}

/** This service returns the version information of this module. */
void CanNm_GetVersionInfo( Std_VersionInfoType * versioninfo ){
	CANNM_VALIDATE_INIT(CANNM_SERVICEID_GETVERSIONINFO);
}

/** Request bus synchronization. */
Nm_ReturnType CanNm_RequestBusSynchronization( const NetworkHandleType nmChannelHandle ){
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
	CANNM_VALIDATE_CHANNEL(canNmTxPduId, CANNM_SERVICEID_TXCONFIRMATION);

	const CanNm_ChannelType* ChannelConf = &CanNm_ConfigPtr->Channels[canNmTxPduId];
	CanNm_Internal_ChannelType* ChannelInternal = &CanNm_Internal.Channels[canNmTxPduId];

	if (ChannelInternal->Mode == NM_MODE_NETWORK) {
		CanNm_Internal_NetworkMode_to_NetworkMode(ChannelConf, ChannelInternal);
	}
}

/** This service indicates a successful reception of a received NM message to the
  * CanNm after passing all filters and validation checks.
  * This callback service is called by the CAN Interface and implemented by the CanNm. */
void CanNm_RxIndication( PduIdType canNmRxPduId, const uint8 *canSduPtr ){
	CANNM_VALIDATE_INIT(CANNM_SERVICEID_RXINDICATION);
	CANNM_VALIDATE_CHANNEL(canNmRxPduId, CANNM_SERVICEID_RXINDICATION);

	const CanNm_ChannelType* ChannelConf = &CanNm_ConfigPtr->Channels[canNmRxPduId];
	CanNm_Internal_ChannelType* ChannelInternal = &CanNm_Internal.Channels[canNmRxPduId];

	memcpy(ChannelInternal->RxMessageSdu, canSduPtr, ChannelConf->PduLength);

	boolean repeatMessageBitIndication = FALSE;
	if (ChannelConf->CbvPosition != CANNM_PDU_OFF) {
		uint8 cbv = ChannelInternal->RxMessageSdu[ChannelConf->CbvPosition];
		repeatMessageBitIndication = cbv & CANNM_CBV_REPEAT_MESSAGE_REQUEST;
	}

	if (ChannelInternal->Mode == NM_MODE_BUS_SLEEP) {
		CanNm_Internal_BusSleep_to_BusSleep(ChannelConf, ChannelInternal);
	} else if (ChannelInternal->Mode == NM_MODE_PREPARE_BUS_SLEEP) {
		CanNm_Internal_PrepareBusSleep_to_RepeatMessage(ChannelConf, ChannelInternal);
	} else if (ChannelInternal->Mode == NM_MODE_NETWORK) {
		CanNm_Internal_NetworkMode_to_NetworkMode(ChannelConf, ChannelInternal);
		if (repeatMessageBitIndication) {
			if (ChannelInternal->State == NM_STATE_READY_SLEEP) {
				CanNm_Internal_ReadySleep_to_RepeatMessage(ChannelConf, ChannelInternal);
			} else if (ChannelInternal->State == NM_STATE_NORMAL_OPERATION) {
				CanNm_Internal_NormalOperation_to_RepeatMessage(ChannelConf, ChannelInternal);
			}
		}
	}
#if (CANNM_PDU_RX_INDICATION_ENABLED == STD_ON)
	// TODO: call NM rx indication
#endif
}




// ----------------------------------------------------------------------------
// Internal functions
// ----------------------------------------------------------------------------

void CanNm_MainFunction( NetworkHandleType nmChannelHandle ) {
	CANNM_VALIDATE_INIT(CANNM_SERVICEID_ARC_MAINFUNCTION);

	const CanNm_ChannelType* ChannelConf = &CanNm_ConfigPtr->Channels[nmChannelHandle];
	CanNm_Internal_ChannelType* ChannelInternal = &CanNm_Internal.Channels[nmChannelHandle];
	if (ChannelInternal->Mode == NM_MODE_NETWORK) {
		CanNm_Internal_TickTimeoutTime(ChannelConf, ChannelInternal);
#if (CANNM_PASSIVE_MODE_ENABLED != STD_ON)
		if ((ChannelInternal->State == NM_STATE_REPEAT_MESSAGE) ||
			(ChannelInternal->State == NM_STATE_NORMAL_OPERATION)) {
			CanNm_Internal_TickMessageCycleTime(ChannelConf, ChannelInternal);
		}
#endif
		if (ChannelInternal->State == NM_STATE_REPEAT_MESSAGE) {
			CanNm_Internal_TickRepeatMessageTime(ChannelConf, ChannelInternal);
		}
	} else if (ChannelInternal->Mode == NM_MODE_PREPARE_BUS_SLEEP) {
		CanNm_Internal_TickWaitBusSleepTime(ChannelConf, ChannelInternal);
	}
}

// Timer helpers
// -------------

static inline void CanNm_Internal_TickTimeoutTime( const CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal ) {
	if (ChannelConf->MainFunctionPeriod >= ChannelInternal->TimeoutTimeLeft) {
		ChannelInternal->TimeoutTimeLeft = 0;
		if (ChannelInternal->State == NM_STATE_REPEAT_MESSAGE) {
			CanNm_Internal_RepeatMessage_to_RepeatMessage(ChannelConf, ChannelInternal);
		} else if (ChannelInternal->State == NM_STATE_NORMAL_OPERATION) {
			CanNm_Internal_NormalOperation_to_NormalOperation(ChannelConf, ChannelInternal);
		} else if (ChannelInternal->State == NM_STATE_READY_SLEEP) {
			CanNm_Internal_ReadySleep_to_PrepareBusSleep(ChannelConf, ChannelInternal);
		}
	} else {
		ChannelInternal->TimeoutTimeLeft -= ChannelConf->MainFunctionPeriod;
	}
}

static inline void CanNm_Internal_TickRepeatMessageTime( const CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal ) {
	if (ChannelConf->MainFunctionPeriod >= ChannelInternal->RepeatMessageTimeLeft) {
		ChannelInternal->RepeatMessageTimeLeft = 0;
		if (ChannelInternal->State == NM_STATE_REPEAT_MESSAGE) {
			if (ChannelInternal->Requested) {
				CanNm_Internal_RepeatMessage_to_NormalOperation(ChannelConf, ChannelInternal);
			} else {
				CanNm_Internal_RepeatMessage_to_ReadySleep(ChannelConf, ChannelInternal);
			}
		}
	} else {
		ChannelInternal->RepeatMessageTimeLeft -= ChannelConf->MainFunctionPeriod;
	}
}

static inline void CanNm_Internal_TickWaitBusSleepTime( const CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal ) {
	if (ChannelConf->MainFunctionPeriod >= ChannelInternal->WaitBusSleepTimeLeft) {
		ChannelInternal->WaitBusSleepTimeLeft = 0;
		if (ChannelInternal->Mode == NM_MODE_PREPARE_BUS_SLEEP) {
			CanNm_Internal_PrepareBusSleep_to_BusSleep(ChannelConf, ChannelInternal);
		}
	} else {
		ChannelInternal->WaitBusSleepTimeLeft -= ChannelConf->MainFunctionPeriod;
	}
}

static inline void CanNm_Internal_TickMessageCycleTime( const CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal ) {
	if (ChannelConf->MainFunctionPeriod >= ChannelInternal->MessageCycleTimeLeft) {
		ChannelInternal->MessageCycleTimeLeft = 0;
		if ((ChannelInternal->State == NM_STATE_REPEAT_MESSAGE) ||
			(ChannelInternal->State == NM_STATE_NORMAL_OPERATION)) {
			ChannelInternal->MessageCycleTimeLeft = ChannelConf->MessageCycleTime;
			CanNm_Internal_TransmitMessage(ChannelConf, ChannelInternal);
		}
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
	CanIf_Transmit(ChannelConf->CanIfPduId, &pdu);
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
	}
}

// Transition helpers
// ------------------

static inline void CanNm_Internal_PrepareBusSleep_to_RepeatMessage( const CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal ) {
	ChannelInternal->Mode = NM_MODE_NETWORK;
	ChannelInternal->State = NM_STATE_REPEAT_MESSAGE;
	ChannelInternal->TimeoutTimeLeft = ChannelConf->TimeoutTime;
	ChannelInternal->RepeatMessageTimeLeft = ChannelConf->RepeatMessageTime;
	ChannelInternal->MessageCycleTimeLeft = ChannelConf->MessageCycleOffsetTime;
	// Notify 'Network Mode'
	Nm_NetworkMode(ChannelConf->NmNetworkHandle);
}
static inline void CanNm_Internal_PrepareBusSleep_to_BusSleep( const CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal ) {
	ChannelInternal->Mode = NM_MODE_BUS_SLEEP;
	ChannelInternal->State = NM_STATE_BUS_SLEEP;
	// Notify 'Bus-Sleep Mode'
	Nm_BusSleepMode(ChannelConf->NmNetworkHandle);
}

static inline void CanNm_Internal_BusSleep_to_RepeatMessage( const CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal ) {
	ChannelInternal->Mode = NM_MODE_NETWORK;
	ChannelInternal->State = NM_STATE_REPEAT_MESSAGE;
	ChannelInternal->TimeoutTimeLeft = ChannelConf->TimeoutTime;
	ChannelInternal->RepeatMessageTimeLeft = ChannelConf->RepeatMessageTime;
	ChannelInternal->MessageCycleTimeLeft = ChannelConf->MessageCycleOffsetTime;
	// Notify 'Network Mode'
	Nm_NetworkMode(ChannelConf->NmNetworkHandle);
}
static inline void CanNm_Internal_BusSleep_to_BusSleep( const CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal ) {
	// Notify 'Network Start'
	Nm_NetworkStartIndication(ChannelConf->NmNetworkHandle);
}

static inline void CanNm_Internal_RepeatMessage_to_RepeatMessage( const CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal ) {
	ChannelInternal->TimeoutTimeLeft = ChannelConf->TimeoutTime;
	// TODO: Notify 'Network Timeout'?
}
static inline void CanNm_Internal_RepeatMessage_to_ReadySleep( const CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal ) {
	ChannelInternal->Mode = NM_MODE_NETWORK;
	ChannelInternal->State = NM_STATE_READY_SLEEP;
	CanNm_Internal_ClearCbv(ChannelConf, ChannelInternal);
}
static inline void CanNm_Internal_RepeatMessage_to_NormalOperation( const CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal ) {
	ChannelInternal->Mode = NM_MODE_NETWORK;
	ChannelInternal->State = NM_STATE_NORMAL_OPERATION;
	CanNm_Internal_ClearCbv(ChannelConf, ChannelInternal);
}

static inline void CanNm_Internal_NormalOperation_to_RepeatMessage( const CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal ) {
	ChannelInternal->Mode = NM_MODE_NETWORK;
	ChannelInternal->State = NM_STATE_REPEAT_MESSAGE;
	ChannelInternal->RepeatMessageTimeLeft = ChannelConf->RepeatMessageTime;
	ChannelInternal->MessageCycleTimeLeft = ChannelConf->MessageCycleOffsetTime;
}
static inline void CanNm_Internal_NormalOperation_to_ReadySleep( const CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal ) {
	ChannelInternal->Mode = NM_MODE_NETWORK;
	ChannelInternal->State = NM_STATE_READY_SLEEP;
}
static inline void CanNm_Internal_NormalOperation_to_NormalOperation( const CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal ) {
	ChannelInternal->TimeoutTimeLeft = ChannelConf->TimeoutTime;
}

static inline void CanNm_Internal_ReadySleep_to_NormalOperation( const CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal ) {
	ChannelInternal->Mode = NM_MODE_NETWORK;
	ChannelInternal->State = NM_STATE_NORMAL_OPERATION;
	ChannelInternal->MessageCycleTimeLeft = ChannelConf->MessageCycleOffsetTime;
}
static inline void CanNm_Internal_ReadySleep_to_PrepareBusSleep( const CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal ) {
	ChannelInternal->Mode = NM_MODE_PREPARE_BUS_SLEEP;
	ChannelInternal->State = NM_STATE_PREPARE_BUS_SLEEP;
	ChannelInternal->WaitBusSleepTimeLeft = ChannelConf->WaitBusSleepTime;
	// Notify 'Prepare Bus-Sleep Mode'
	Nm_PrepareBusSleepMode(ChannelConf->NmNetworkHandle);
}
static inline void CanNm_Internal_ReadySleep_to_RepeatMessage( const CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal ) {
	ChannelInternal->Mode = NM_MODE_NETWORK;
	ChannelInternal->State = NM_STATE_REPEAT_MESSAGE;
	ChannelInternal->RepeatMessageTimeLeft = ChannelConf->RepeatMessageTime;
	ChannelInternal->MessageCycleTimeLeft = ChannelConf->MessageCycleOffsetTime;
}

static inline void CanNm_Internal_NetworkMode_to_NetworkMode( const CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal ) {
	ChannelInternal->TimeoutTimeLeft = ChannelConf->TimeoutTime;
}
