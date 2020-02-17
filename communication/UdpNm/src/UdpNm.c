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

/* Globally fulfilled requirements */
/** @req SWS_UdpNm_00131 - The AUTOSAR UdpNm algorithm shall support up to 250 nodes per NM-Cluster by default.*/
/** @req SWS_UdpNm_00081 - The code file structure shall include UdpNm_Lcfg.c and UdpNm_PBcfg.c. */
/** @req SWS_UdpNm_00044 - Header files UdpNm shall provide. */
/** @req SWS_UdpNm_00082 - Header files UdpNm shall include. *//** Memory mapping is done in MemMap.h instead of UdpNm_MemMap.h */
/** @req SWS_UdpNm_00083 - Header files containing data that UdpNm shall include. *//**SoAd.h and Nm.h are included, which includes SoAd_Cfg.h and Nm_Cfg.h */
/** @req SWS_UdpNm_00089 - The UdpNm state machine. */
/** @req SWS_UdpNm_00092 - UdpNm operational modes. */
/** @req SWS_UdpNm_00093 - Changes of the AUTOSAR UdpNm operational modes shall be signaled to the upper
 * layer by means of call-back functions. */
/** @req SWS_UdpNm_00094 - UdpNm operational states.  */
/** @req SWS_UdpNm_00146 - The UdpNm coordination algorithm shall be processor independent. */
/** @req SWS_UdpNm_00244 - The UdpNm module shall reject the execution of a service
called with an invalid parameter and shall inform the DET. */
/** @req SWS_UdpNm_00197 - Parameter type checking shall be performed at compile time. */
/** @req SWS_UdpNm_00198 - Parameter value check shall be performed at configuration time. */
/** @req SWS_UdpNm_00199 - Parameter value check shall be performed at execution time. */
/** @req SWS_UdpNm_00158 - Support of NM user data shall be statically configurable. */
/** @req SWS_UdpNm_00086 - Checked during configuration. */
/** @req SWS_UdpNm_00314 - Checked during configuration.  */

#include "Platform_Types.h"
#include "ComStack_Types.h"
#include "UdpNm.h"
#include "UdpNm_Internal.h"
#include "Nm_Cbk.h"
#include "NmStack_Types.h"
#include "SchM_UdpNm.h"
#include "MemMap.h"
#include "SoAd.h"
#include "Nm.h"
#include <string.h>

/** @req SWS_UdpNm_00311 */
#if (UDPNM_COM_USER_DATA_SUPPORT == STD_ON)
#include "PduR_UdpNm.h"
#endif

#if (UDPNM_DEV_ERROR_DETECT == STD_ON)
#include "Det.h"
#endif

#if defined(USE_DEM)
#include "Dem.h"
#endif

static const UdpNm_ConfigType* UdpNm_ConfigPtr;

void UdpNm_MainFunction(NetworkHandleType nmChannelHandle);

/*lint -save -e785 Too few initializers for aggregate UdpNm_Internal. OK, the rest are set in UdpNm_Init.
 * This is the only one that needs to be set before. */
UdpNm_InternalType UdpNm_Internal = {
        .InitStatus = UDPNM_UNINIT,
};
/*lint -restore -e785 */

/** Initialize the complete UdpNm module, i.e. all channels which are activated */
/** @req SWS_UdpNm_00141 */
/** @req SWS_UdpNm_00143 */
/** @req SWS_UdpNm_00144 */
/** @req SWS_UdpNm_00145 */
/** @req SWS_UdpNm_00060 */
/** @req SWS_UdpNm_00061 */
/** @req SWS_UdpNm_00033 */
/** @req SWS_UdpNm_00039 */
/** @req SWS_UdpNm_00025 */
/** @req SWS_UdpNm_00085 */
/** @req SWS_UdpNm_00013 */
/** @req SWS_UdpNm_00208 */
void UdpNm_Init( const UdpNm_ConfigType* UdpNmConfigPtr ){
    UDPNM_VALIDATE_NOTNULL(UdpNmConfigPtr, UDPNM_SERVICEID_INIT);

    UdpNm_ConfigPtr = UdpNmConfigPtr;

    for (uint8 channel = 0; channel < UDPNM_CHANNEL_COUNT; channel++) {
        const UdpNm_ChannelType* ChannelConf = &UdpNm_ConfigPtr->Channels[channel];
        UdpNm_Internal_ChannelType* ChannelInternal = &UdpNm_Internal.Channels[channel];

        ChannelInternal->Mode = NM_MODE_BUS_SLEEP;
        ChannelInternal->State = NM_STATE_BUS_SLEEP;
        ChannelInternal->Requested = FALSE;

        ChannelInternal->TimeoutTimeLeft = 0;
        ChannelInternal->RepeatMessageTimeLeft = 0;
        ChannelInternal->WaitBusSleepTimeLeft = 0;
        ChannelInternal->MessageCycleTimeLeft = 0;
        ChannelInternal->MessageTimeoutTimeLeft = 0;

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

#if ((UDPNM_PNC_EIRA_CALC_ENABLED == STD_ON) && (UDPNM_PNC_COUNT > 0))
    /* Initialize the PN reset timer */
    for (uint8 i = 0; i < UDPNM_PNC_COUNT; i++) {
        UdpNm_Internal.pnEIRATimers[i].timerRunning = FALSE;
        UdpNm_Internal.pnEIRATimers[i].resetTimer = 0;
    }

    /** @req SWS_UdpNm_00344 */
    memset(UdpNm_Internal.pnEIRABytes, 0, UDPNM_PNC_INFO_LEN); /* reset EIRA */
#endif
#if ((UDPNM_PNC_ERA_CALC_ENABLED == STD_ON) && (UDPNM_PNC_COUNT > 0))
    for (uint8 chanIndex = 0; chanIndex < UDPNM_CHANNEL_COUNT; chanIndex++) {
        for (uint8 timerIndex = 0; timerIndex < UDPNM_PNC_COUNT; timerIndex++) {
            UdpNm_Internal.Channels[chanIndex].pnERATimers[timerIndex].timerRunning = FALSE;
            UdpNm_Internal.Channels[chanIndex].pnERATimers[timerIndex].resetTimer = 0;
        }

        /** @req SWS_UdpNm_00355 */
        memset(UdpNm_Internal.Channels[chanIndex].pnERABytes, 0, UDPNM_PNC_INFO_LEN);
    }
#endif

    UdpNm_Internal.InitStatus = UDPNM_INIT;
}

/** Passive startup of the AUTOSAR Udp NM. It triggers the transition from Bus-
  * Sleep Mode to the Network Mode in Repeat Message State.
  * This service has no effect if the current state is equal to Network Mode. In
  * that case E_NOT_OK is returned. */
/** @req SWS_UdpNm_00095 */
/** @req SWS_UdpNm_00128 */
/** @req SWS_UdpNm_00147 */
/** @req SWS_UdpNm_00211 */
Std_ReturnType UdpNm_PassiveStartUp( const NetworkHandleType nmChannelHandle ){
    UDPNM_VALIDATE_INIT(UDPNM_SERVICEID_PASSIVESTARTUP, E_NOT_OK);
    UDPNM_VALIDATE_CHANNEL(nmChannelHandle, UDPNM_SERVICEID_PASSIVESTARTUP, E_NOT_OK);

    const UdpNm_ChannelType* ChannelConf = &UdpNm_ConfigPtr->Channels[UdpNm_ConfigPtr->ChannelMap[nmChannelHandle]];
    UdpNm_Internal_ChannelType* ChannelInternal = &UdpNm_Internal.Channels[UdpNm_ConfigPtr->ChannelMap[nmChannelHandle]];
    Std_ReturnType status = E_OK;

    if (ChannelInternal->Mode == NM_MODE_NETWORK) {
        status = E_NOT_OK;
    } else {
        UdpNm_Internal_BusSleep_to_RepeatMessage(ChannelConf, ChannelInternal);
        status = E_OK;
    }

#if (UDPNM_PNC_COUNT > 0)
    if (TRUE == ChannelConf->UdpNmPnEnable) {

        /** @req SWS_UdpNm_00332 */
        /** @req SWS_UdpNm_00333 */
        UdpNm_Internal_SetPniCbv(ChannelConf, ChannelInternal);
    }
#endif

    return status;
}

#if (UDPNM_PASSIVE_MODE_ENABLED == STD_OFF)
/** Request the network, since ECU needs to communicate on the bus. Network
  * state shall be changed to */
/** @req SWS_UdpNm_00110 */
/** @req SWS_UdpNm_00123 */
/** @req SWS_UdpNm_00129 */
/** @req SWS_UdpNm_00104 */
/** @req SWS_UdpNm_00213 */
Std_ReturnType UdpNm_NetworkRequest( const NetworkHandleType nmChannelHandle ){
    UDPNM_VALIDATE_INIT(UDPNM_SERVICEID_NETWORKREQUEST, E_NOT_OK);
    UDPNM_VALIDATE_CHANNEL(nmChannelHandle, UDPNM_SERVICEID_NETWORKREQUEST, E_NOT_OK);

    const UdpNm_ChannelType* ChannelConf = &UdpNm_ConfigPtr->Channels[UdpNm_ConfigPtr->ChannelMap[nmChannelHandle]];
    UdpNm_Internal_ChannelType* ChannelInternal = &UdpNm_Internal.Channels[UdpNm_ConfigPtr->ChannelMap[nmChannelHandle]];

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

#if (UDPNM_PNC_COUNT > 0)
    if (TRUE == ChannelConf->UdpNmPnEnable) {

        /** @req SWS_UdpNm_00332 */
        /** @req SWS_UdpNm_00333 */
        UdpNm_Internal_SetPniCbv(ChannelConf, ChannelInternal);
    }
#endif

    return E_OK;
}

/** Release the network, since ECU doesn't have to communicate on the bus. Network
  * state shall be changed to */
/** @req SWS_UdpNm_00105 */
/** @req SWS_UdpNm_00214 */
Std_ReturnType UdpNm_NetworkRelease( const NetworkHandleType nmChannelHandle ){
    UDPNM_VALIDATE_INIT(UDPNM_SERVICEID_NETWORKRELEASE, E_NOT_OK);
    UDPNM_VALIDATE_CHANNEL(nmChannelHandle, UDPNM_SERVICEID_NETWORKRELEASE, E_NOT_OK);

    UdpNm_Internal_ChannelType* ChannelInternal = &UdpNm_Internal.Channels[UdpNm_ConfigPtr->ChannelMap[nmChannelHandle]];

    ChannelInternal->Requested = FALSE;

    if (ChannelInternal->Mode == NM_MODE_NETWORK) {
        if (ChannelInternal->State == NM_STATE_NORMAL_OPERATION) {
            UdpNm_Internal_NormalOperation_to_ReadySleep(ChannelInternal);
        }
    }
    return E_OK;
}
#endif

/** Disable the NM PDU transmission ability due to a ISO14229 Communication
  * Control (28hex) service */
Std_ReturnType UdpNm_DisableCommunication( const NetworkHandleType nmChannelHandle ){
    UDPNM_VALIDATE_INIT(UDPNM_SERVICEID_DISABLECOMMUNICATION, E_NOT_OK);
    UDPNM_VALIDATE_CHANNEL(nmChannelHandle, UDPNM_SERVICEID_DISABLECOMMUNICATION, E_NOT_OK);
    // Not implemented
    return E_NOT_OK;
}

/** Enable the NM PDU transmission ability due to a ISO14229 Communication
  * Control (28hex) service */
Std_ReturnType UdpNm_EnableCommunication( const NetworkHandleType nmChannelHandle ){
    UDPNM_VALIDATE_INIT(UDPNM_SERVICEID_ENABLECOMMUNICATION, E_NOT_OK);
    UDPNM_VALIDATE_CHANNEL(nmChannelHandle, UDPNM_SERVICEID_ENABLECOMMUNICATION, E_NOT_OK);
    // Not implemented
    return E_NOT_OK;
}

/** @req SWS_UdpNm_00312 */
#if ((UDPNM_USER_DATA_ENABLED == STD_ON) && ( UDPNM_COM_USER_DATA_SUPPORT == STD_OFF))
/** Set user data for NM messages transmitted next on the bus. */
/** @req SWS_UdpNm_00217 */
/** @req SWS_UdpNm_00159 */
Std_ReturnType UdpNm_SetUserData( const NetworkHandleType nmChannelHandle, const uint8* nmUserDataPtr ){
    UDPNM_VALIDATE_INIT(UDPNM_SERVICEID_SETUSERDATA, E_NOT_OK);
    UDPNM_VALIDATE_CHANNEL(nmChannelHandle, UDPNM_SERVICEID_SETUSERDATA, E_NOT_OK);

    const UdpNm_ChannelType* ChannelConf = &UdpNm_ConfigPtr->Channels[UdpNm_ConfigPtr->ChannelMap[nmChannelHandle]];
    UdpNm_Internal_ChannelType* ChannelInternal = &UdpNm_Internal.Channels[UdpNm_ConfigPtr->ChannelMap[nmChannelHandle]];
    uint8* destUserData = UdpNm_Internal_GetUserDataPtr(ChannelConf, ChannelInternal->TxMessageSdu);
    uint8 userDataLength = UdpNm_Internal_GetUserDataLength(ChannelConf);

    memcpy(destUserData, nmUserDataPtr, userDataLength);

    return E_OK;
}
#endif

#if (UDPNM_USER_DATA_ENABLED == STD_ON)
/** Get user data out of the most recently received NM message. */
/** @req SWS_UdpNm_00218 */
/** @req SWS_UdpNm_00160 */
Std_ReturnType UdpNm_GetUserData( const NetworkHandleType nmChannelHandle, uint8* const nmUserDataPtr ){
    UDPNM_VALIDATE_INIT(UDPNM_SERVICEID_GETUSERDATA, E_NOT_OK);
    UDPNM_VALIDATE_CHANNEL(nmChannelHandle, UDPNM_SERVICEID_GETUSERDATA, E_NOT_OK);

    const UdpNm_ChannelType* ChannelConf = &UdpNm_ConfigPtr->Channels[UdpNm_ConfigPtr->ChannelMap[nmChannelHandle]];
    UdpNm_Internal_ChannelType* ChannelInternal = &UdpNm_Internal.Channels[UdpNm_ConfigPtr->ChannelMap[nmChannelHandle]];
    const uint8* sourceUserData = UdpNm_Internal_GetUserDataPtr(ChannelConf, ChannelInternal->RxMessageSdu);
    uint8 userDataLength = UdpNm_Internal_GetUserDataLength(ChannelConf);

    memcpy(nmUserDataPtr, sourceUserData, userDataLength);

    return E_OK;
}
#endif

#if (UDPNM_NODE_ID_ENABLED == STD_ON)
/** Get node identifier out of the most recently received NM PDU. */
/** @req SWS_UdpNm_00219 */
/** @req SWS_UdpNm_00132 */
Std_ReturnType UdpNm_GetNodeIdentifier( const NetworkHandleType nmChannelHandle, uint8* const nmNodeIdPtr ){
    UDPNM_VALIDATE_INIT(UDPNM_SERVICEID_GETNODEIDENTIFIER, E_NOT_OK);
    UDPNM_VALIDATE_CHANNEL(nmChannelHandle, UDPNM_SERVICEID_GETNODEIDENTIFIER, E_NOT_OK);

    const UdpNm_ChannelType* ChannelConf = &UdpNm_ConfigPtr->Channels[UdpNm_ConfigPtr->ChannelMap[nmChannelHandle]];
    const UdpNm_Internal_ChannelType* ChannelInternal = &UdpNm_Internal.Channels[UdpNm_ConfigPtr->ChannelMap[nmChannelHandle]];
    Std_ReturnType status = E_OK;

    if (ChannelConf->UdpNmPduNidPosition == UDPNM_PDU_OFF) {
        status = E_NOT_OK;
    } else {
        *nmNodeIdPtr = ChannelInternal->RxMessageSdu[ChannelConf->UdpNmPduNidPosition];
    }

    return status;
}

/** Get node identifier configured for the local node. */
/** @req SWS_UdpNm_00220 */
/** @req SWS_UdpNm_00133 */
Std_ReturnType UdpNm_GetLocalNodeIdentifier( const NetworkHandleType nmChannelHandle, uint8* const nmNodeIdPtr ){
    UDPNM_VALIDATE_INIT(UDPNM_SERVICEID_GETLOCALNODEIDENTIFIER, E_NOT_OK);
    UDPNM_VALIDATE_CHANNEL(nmChannelHandle, UDPNM_SERVICEID_GETLOCALNODEIDENTIFIER, E_NOT_OK);

    const UdpNm_ChannelType* ChannelConf = &UdpNm_ConfigPtr->Channels[UdpNm_ConfigPtr->ChannelMap[nmChannelHandle]];
    *nmNodeIdPtr = ChannelConf->UdpNmNodeId;

    return E_OK;
}
#endif

#if (UDPNM_NODE_DETECTION_ENABLED == STD_ON)
/** Set Repeat Message Request Bit for NM messages transmitted next on the bus. */
/** @req SWS_UdpNm_00137 */
/** @req SWS_UdpNm_00120 */
/** @req SWS_UdpNm_00121 */
/** @req SWS_UdpNm_00112 */
/** @req SWS_UdpNm_00113 */
/** @req SWS_UdpNm_00135 */
/** @req SWS_UdpNm_00221 */
Std_ReturnType UdpNm_RepeatMessageRequest( const NetworkHandleType nmChannelHandle ){
    UDPNM_VALIDATE_INIT(UDPNM_SERVICEID_REPEATMESSAGEREQUEST, E_NOT_OK);
    UDPNM_VALIDATE_CHANNEL(nmChannelHandle, UDPNM_SERVICEID_REPEATMESSAGEREQUEST, E_NOT_OK);

    const UdpNm_ChannelType* ChannelConf = &UdpNm_ConfigPtr->Channels[UdpNm_ConfigPtr->ChannelMap[nmChannelHandle]];
    UdpNm_Internal_ChannelType* ChannelInternal = &UdpNm_Internal.Channels[UdpNm_ConfigPtr->ChannelMap[nmChannelHandle]];
    Std_ReturnType status = E_NOT_OK;

    if (ChannelConf->UdpNmPduCbvPosition != UDPNM_PDU_OFF) {
        if (ChannelInternal->State == NM_STATE_READY_SLEEP) {
            ChannelInternal->TxMessageSdu[ChannelConf->UdpNmPduCbvPosition] |= UDPNM_CBV_REPEAT_MESSAGE_REQUEST;
            UdpNm_Internal_ReadySleep_to_RepeatMessage(ChannelConf, ChannelInternal);
            status = E_OK;
        } else if (ChannelInternal->State == NM_STATE_NORMAL_OPERATION) {
            ChannelInternal->TxMessageSdu[ChannelConf->UdpNmPduCbvPosition] |= UDPNM_CBV_REPEAT_MESSAGE_REQUEST;
            UdpNm_Internal_NormalOperation_to_RepeatMessage(ChannelConf, ChannelInternal);
            status = E_OK;
        } else {
            //Nothing to be done
        }
    }

    return status;
}
#endif

#if ((UDPNM_NODE_ID_ENABLED == STD_ON) || (UDPNM_NODE_DETECTION_ENABLED == STD_ON) || (UDPNM_USER_DATA_ENABLED == STD_ON))
/** Get the whole PDU data out of the most recently received NM message. */
/** @req SWS_UdpNm_00309 */
/** @req SWS_UdpNm_00138 */
/** @req SWS_UdpNm_00139 */
Std_ReturnType UdpNm_GetPduData( const NetworkHandleType nmChannelHandle, uint8* const nmPduDataPtr ){
    UDPNM_VALIDATE_INIT(UDPNM_SERVICEID_GETPDUDATA, E_NOT_OK);
    UDPNM_VALIDATE_CHANNEL(nmChannelHandle, UDPNM_SERVICEID_GETPDUDATA, E_NOT_OK);

    const UdpNm_ChannelType* ChannelConf = &UdpNm_ConfigPtr->Channels[UdpNm_ConfigPtr->ChannelMap[nmChannelHandle]];
    const UdpNm_Internal_ChannelType* ChannelInternal = &UdpNm_Internal.Channels[UdpNm_ConfigPtr->ChannelMap[nmChannelHandle]];

    memcpy(nmPduDataPtr, ChannelInternal->RxMessageSdu, ChannelConf->UdpNmPduLength);

    return E_OK;
}
#endif

/** Returns the state and the mode of the network management. */
/** @req SWS_UdpNm_00310 */
Std_ReturnType UdpNm_GetState( const NetworkHandleType nmChannelHandle, Nm_StateType* const nmStatePtr, Nm_ModeType* const nmModePtr ){
    UDPNM_VALIDATE_INIT(UDPNM_SERVICEID_GETSTATE, E_NOT_OK);
    UDPNM_VALIDATE_CHANNEL(nmChannelHandle, UDPNM_SERVICEID_GETSTATE, E_NOT_OK);

    const UdpNm_Internal_ChannelType* ChannelInternal = &UdpNm_Internal.Channels[UdpNm_ConfigPtr->ChannelMap[nmChannelHandle]];
    *nmStatePtr = ChannelInternal->State;
    *nmModePtr = ChannelInternal->Mode;

    return E_OK;
}

/** Request bus synchronization. */
Std_ReturnType UdpNm_RequestBusSynchronization( const NetworkHandleType nmChannelHandle ){
    UDPNM_VALIDATE_INIT(UDPNM_SERVICEID_REQUESTBUSSYNCHRONIZATION, E_NOT_OK);
    UDPNM_VALIDATE_CHANNEL(nmChannelHandle, UDPNM_SERVICEID_REQUESTBUSSYNCHRONIZATION, E_NOT_OK);
    // Not implemented
    return E_NOT_OK;
}

/** Check if remote sleep indication takes place or not. */
Std_ReturnType UdpNm_CheckRemoteSleepIndication( const NetworkHandleType nmChannelHandle, boolean* const nmRemoteSleepIndPtr ){
    UDPNM_VALIDATE_INIT(UDPNM_SERVICEID_CHECKREMOTESLEEPINDICATION, E_NOT_OK);
    UDPNM_VALIDATE_CHANNEL(nmChannelHandle, UDPNM_SERVICEID_CHECKREMOTESLEEPINDICATION, E_NOT_OK);
    //lint -estring(920,pointer)  /* cast to void */
    (void)nmRemoteSleepIndPtr;
    //lint +estring(920,pointer)  /* cast to void */
    // Not implemented
    return E_NOT_OK;
}

/** Sets the NM coordinator ID in the control bit vector of each NM message. */
Std_ReturnType UdpNm_SetCoordBits( const NetworkHandleType nmChannelHandle, const uint8 nmCoordBits ){
    UDPNM_VALIDATE_INIT(UDPNM_SERVICEID_CHECKREMOTESLEEPINDICATION, E_NOT_OK);
    UDPNM_VALIDATE_CHANNEL(nmChannelHandle, UDPNM_SERVICEID_CHECKREMOTESLEEPINDICATION, E_NOT_OK);

    // Not implemented
    (void) nmCoordBits;

    return E_NOT_OK;
}

// Functions called by SoAd
// ---------------------------------

/** This service confirms a previous successfully processed transmit request.
  * This callback service is called by the SoAd and implemented by the UdpNm. */
/** @req SWS_UdpNm_00099 */
/** @req SWS_UdpNm_00228 */
/** @req SWS_UdpNm_00229 */
/** @req SWS_UdpNm_00230 */
/** @req SWS_UdpNm_00316 */
void UdpNm_SoAdIfTxConfirmation( PduIdType UdpNmTxPduId ){
    UDPNM_VALIDATE_INIT(UDPNM_SERVICEID_SOADIFTXCONFIRMATION);
    UDPNM_VALIDATE_PDUID(UdpNmTxPduId, UDPNM_SERVICEID_SOADIFTXCONFIRMATION);

    const UdpNm_ChannelType* ChannelConf = &UdpNm_ConfigPtr->Channels[UdpNmTxPduId];
    UdpNm_Internal_ChannelType* ChannelInternal = &UdpNm_Internal.Channels[UdpNmTxPduId];

    if (ChannelInternal->Mode == NM_MODE_NETWORK) {
        UdpNm_Internal_NetworkMode_to_NetworkMode(ChannelConf, ChannelInternal);
    }

    /* Message Timeout */
    ChannelInternal->MessageTimeoutTimeLeft = ChannelConf->UdpNmMsgTimeoutTime;

#if (UDPNM_COM_USER_DATA_SUPPORT == STD_ON)
    PduR_UdpNmTxConfirmation(UdpNmTxPduId);
#endif /* UDPNM_COM_USER_DATA_SUPPORT == STD_ON */
}

/** This service indicates a successful reception of a received NM message to the
  * UdpNm after passing all filters and validation checks.
  * This callback service is called by the SoAd and implemented by the UdpNm. */
/** @req SWS_UdpNm_00098 */
/** @req SWS_UdpNm_00119 */
/** @req SWS_UdpNm_00111 */
/** @req SWS_UdpNm_00124 */
/** @req SWS_UdpNm_00035 */
/** @req SWS_UdpNm_00037 */
/** @req SWS_UdpNm_00164 */
/** @req SWS_UdpNm_00165 */
/** @req SWS_UdpNm_00231 */
/** @req SWS_UdpNm_00232 partial */
/** @req SWS_UdpNm_00233 */
/*lint -e{818} Pointer parameter 'PduInfoPtr' could be declared as pointing to const */
void UdpNm_SoAdIfRxIndication( PduIdType RxPduId, const PduInfoType* PduInfoPtr ){
    UDPNM_VALIDATE_INIT(UDPNM_SERVICEID_SOADIFRXINDICATION);
    UDPNM_VALIDATE_NOTNULL(PduInfoPtr, UDPNM_SERVICEID_SOADIFRXINDICATION);
    UDPNM_VALIDATE_PDUID(RxPduId, UDPNM_SERVICEID_SOADIFRXINDICATION);

    const UdpNm_ChannelType* ChannelConf= &UdpNm_ConfigPtr->Channels[RxPduId];
    UdpNm_Internal_ChannelType* ChannelInternal= &UdpNm_Internal.Channels[RxPduId];
    Std_ReturnType ret;
    ret = E_OK;

    memcpy(ChannelInternal->RxMessageSdu, PduInfoPtr->SduDataPtr, ChannelConf->UdpNmPduLength);

    boolean repeatMessageBitIndication = FALSE;
    if (ChannelConf->UdpNmPduCbvPosition != UDPNM_PDU_OFF) {
        uint8 cbv = ChannelInternal->RxMessageSdu[ChannelConf->UdpNmPduCbvPosition];
        repeatMessageBitIndication = cbv & UDPNM_CBV_REPEAT_MESSAGE_REQUEST;
#if (UDPNM_PNC_COUNT > 0)
        ret = UdpNm_Internal_RxProcess(ChannelConf, ChannelInternal, (boolean) ((cbv & UDPNM_CBV_PNI) > 0));
#endif
    }

    if (E_OK == ret) { /*lint !e774 ret evaluates E_OK always when UDPNM_PNC_COUNT = 0 */
        if (ChannelInternal->Mode == NM_MODE_BUS_SLEEP) {
            UdpNm_Internal_BusSleep_to_BusSleep(ChannelConf);
        } else if (ChannelInternal->Mode == NM_MODE_PREPARE_BUS_SLEEP) {
            UdpNm_Internal_PrepareBusSleep_to_RepeatMessage(ChannelConf, ChannelInternal);
        } else if (ChannelInternal->Mode == NM_MODE_NETWORK) {
            UdpNm_Internal_NetworkMode_to_NetworkMode(ChannelConf, ChannelInternal);
            if (TRUE == repeatMessageBitIndication) {
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
        // IMPROVEMENT: Add support for NM rx indication
    #endif
        /* Message Timeout */
        ChannelInternal->MessageTimeoutTimeLeft = ChannelConf->UdpNmMsgTimeoutTime;
    }
}

/** UdpNm_Transmit is implemented as an empty function and shall always return E_OK. */
/** @req SWS_UdpNm_00313 */
/** @req SWS_UdpNm_00315 */
#if (UDPNM_COM_USER_DATA_SUPPORT == STD_ON)
Std_ReturnType UdpNm_Transmit( PduIdType UdpNmSrcPduId, const PduInfoType* UdpNmSrcPduInfoPtr ){
    UDPNM_VALIDATE_NOTNULL(UdpNmSrcPduInfoPtr, UDPNM_SERVICEID_TRANSMIT, E_OK);

    (void) UdpNmSrcPduId;

    return E_OK;
}
#endif

/** @req SWS_UdpNm_00224 */
/** @req SWS_UdpNm_00318 */
#if ( UDPNM_VERSION_INFO_API == STD_ON )
void UdpNm_GetVersionInfo(Std_VersionInfoType* versioninfo) {

    UDPNM_VALIDATE_NOTNULL(versioninfo, UDPNM_SERVICEID_GETVERSIONINFO);

    STD_GET_VERSION_INFO(versioninfo, UDPNM);
}
#endif /* UDPNM_VERSION_INFO_API */

// ----------------------------------------------------------------------------
// Internal functions
// ----------------------------------------------------------------------------

// Accessed through UdpNm_MainFunction_<channel>
/** @req SWS_UdpNm_00193 */
/** @req SWS_UdpNm_00194 */
/** @req SWS_UdpNm_00087 */
/** @req SWS_UdpNm_00088 */
/** @req SWS_UdpNm_00116 */
/** @req SWS_UdpNm_00108 */
/** @req SWS_UdpNm_00072 */
/** @req SWS_UdpNm_00237 */
/** @req SWS_UdpNm_00051 */
/** @req SWS_UdpNm_00161 */
/** @req SWS_UdpNm_00162 */
/** @req SWS_UdpNm_00234 */
void UdpNm_MainFunction(NetworkHandleType nmChannelHandle) {
    UDPNM_VALIDATE_INIT(UDPNM_SERVICEID_ARC_MAINFUNCTION);

    const NetworkHandleType chanIndex = UdpNm_ConfigPtr->ChannelMap[nmChannelHandle];
    const UdpNm_ChannelType* ChannelConf = &UdpNm_ConfigPtr->Channels[chanIndex];
    UdpNm_Internal_ChannelType* ChannelInternal = &UdpNm_Internal.Channels[chanIndex];
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

#if ((UDPNM_PNC_EIRA_CALC_ENABLED == STD_ON) && (UDPNM_PNC_COUNT > 0))
    if (chanIndex == UDPNM_CHANNEL_INDEX_SHORTEST_PERIOD) {
        UdpNm_Internal_TickPnEiraResetTime(ChannelConf->UdpNmMainFunctionPeriod);
    }
#endif
#if ((UDPNM_PNC_ERA_CALC_ENABLED == STD_ON) && (UDPNM_PNC_COUNT > 0))
    if (TRUE == ChannelConf->UdpNmPnEraCalcEnabled) {
        UdpNm_Internal_TickPnEraResetTime(chanIndex);
    }
#endif
}

// Timer helpers
// -------------

/** @req SWS_UdpNm_00206 */
/** @req SWS_UdpNm_00117 */
static inline void UdpNm_Internal_TickTimeoutTime( const UdpNm_ChannelType* ChannelConf, UdpNm_Internal_ChannelType* ChannelInternal ){
    if (ChannelConf->UdpNmMainFunctionPeriod >= ChannelInternal->TimeoutTimeLeft) {
        ChannelInternal->TimeoutTimeLeft = 0;
        if (ChannelInternal->State == NM_STATE_REPEAT_MESSAGE) {

#if defined(USE_DEM)
            if (UdpNm_ConfigPtr->DemReferences->UdpNmNetworkTimeoutDemEventId != DEM_EVENT_ID_NULL) {
                Dem_ReportErrorStatus(UdpNm_ConfigPtr->DemReferences->UdpNmNetworkTimeoutDemEventId, DEM_EVENT_STATUS_FAILED);
            }
#endif

            UdpNm_Internal_RepeatMessage_to_RepeatMessage(ChannelConf, ChannelInternal);
#if (UDPNM_DEV_ERROR_DETECT == STD_ON)
            UDPNM_DET_REPORTERROR(UDPNM_SERVICEID_ARC_MAINFUNCTION, UDPNM_E_NETWORK_TIMEOUT, 0);
#endif
        } else if (ChannelInternal->State == NM_STATE_NORMAL_OPERATION) {

#if defined(USE_DEM)
            if (UdpNm_ConfigPtr->DemReferences->UdpNmNetworkTimeoutDemEventId != DEM_EVENT_ID_NULL) {
                Dem_ReportErrorStatus(UdpNm_ConfigPtr->DemReferences->UdpNmNetworkTimeoutDemEventId, DEM_EVENT_STATUS_FAILED);
            }
#endif
            UdpNm_Internal_NormalOperation_to_NormalOperation(ChannelConf, ChannelInternal);
#if (UDPNM_DEV_ERROR_DETECT == STD_ON)
            UDPNM_DET_REPORTERROR(UDPNM_SERVICEID_ARC_MAINFUNCTION, UDPNM_E_NETWORK_TIMEOUT, 0);
#endif
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
        //Nm_TxTimeoutException(ChannelConf->UdpNmComMChannelIdRef);    //This NM API is not supported yet
    } else {
        /* Message Timeout */
        ChannelInternal->MessageTimeoutTimeLeft -= ChannelConf->UdpNmMainFunctionPeriod;
    }
}

/** @req SWS_UdpNm_00102 */
/** @req SWS_UdpNm_00103 */
/** @req SWS_UdpNm_00106 */
/** @req SWS_UdpNm_00109 */
static inline void UdpNm_Internal_TickRepeatMessageTime( const UdpNm_ChannelType* ChannelConf, UdpNm_Internal_ChannelType* ChannelInternal ){
    if (ChannelConf->UdpNmMainFunctionPeriod >= ChannelInternal->RepeatMessageTimeLeft) {
        ChannelInternal->RepeatMessageTimeLeft = 0;
        if (ChannelInternal->State == NM_STATE_REPEAT_MESSAGE) {
            if (TRUE == ChannelInternal->Requested) {
                UdpNm_Internal_RepeatMessage_to_NormalOperation(ChannelConf, ChannelInternal);
            } else {
                UdpNm_Internal_RepeatMessage_to_ReadySleep(ChannelConf, ChannelInternal);
            }
        }
    } else {
        ChannelInternal->RepeatMessageTimeLeft -= ChannelConf->UdpNmMainFunctionPeriod;
    }
}

/** @req SWS_UdpNm_00115 */
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

/** @req SWS_UdpNm_00101 */
/** @req SWS_UdpNm_00005 */
/** @req SWS_UdpNm_00040 */
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
/**
 * @brief Handle periodic NM message transmission
 * @param ChannelConf channel configuration
 * @param ChannelInternal channel internal runtime data
 * @return void
 */
/** @req SWS_UdpNm_00032 */
static inline void UdpNm_Internal_TransmitMessage( const UdpNm_ChannelType* ChannelConf,
        UdpNm_Internal_ChannelType* ChannelInternal ) {

    PduInfoType pdu = {
            .SduDataPtr = ChannelInternal->TxMessageSdu,
            .SduLength = ChannelConf->UdpNmPduLength,
    };

#if ((UDPNM_USER_DATA_ENABLED == STD_ON) && (UDPNM_COM_USER_DATA_SUPPORT == STD_ON))
    PduInfoType userData;

    userData.SduDataPtr = UdpNm_Internal_GetUserDataPtr(ChannelConf, ChannelInternal->TxMessageSdu);
    userData.SduLength = UdpNm_Internal_GetUserDataLength(ChannelConf);

    /* IMPROVEMENT: Add Det error when transmit is failing */
    /** @req SWS_UdpNm_00317 */
    (void) PduR_UdpNmTriggerTransmit(ChannelConf->UdpNmUserDataTxPduId, &userData);

#if ((UDPNM_PNC_EIRA_CALC_ENABLED == STD_ON) && (UDPNM_PNC_COUNT > 0))
    if (TRUE == ChannelConf->UdpNmPnEnable) {
        UdpNm_Internal_ProcessTxPdu(&pdu.SduDataPtr[UdpNm_ConfigPtr->UdpNmPnInfo->UdpNmPnInfoOffset],
                ChannelConf,
                ChannelInternal
                );
    } else {
        /* Do nothing */
    }
#endif /* ((UDPNM_PNC_EIRA_CALC_ENABLED == STD_ON) && (UDPNM_PNC_COUNT > 0)) */
#endif /* ((UDPNM_USER_DATA_ENABLED == STD_ON) && (UDPNM_COM_USER_DATA_SUPPORT == STD_ON)) */

    if (SoAd_IfTransmit(ChannelConf->UdpNmTxPduId, &pdu) != E_OK) {
#if defined(USE_DEM)
        if (UdpNm_ConfigPtr->DemReferences->UdpNmNetworkTimeoutDemEventId != DEM_EVENT_ID_NULL) {
            Dem_ReportErrorStatus(UdpNm_ConfigPtr->DemReferences->UdpNmTcpIpTransmitErrorDemEventId, DEM_EVENT_STATUS_FAILED);
        }
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

/** @req SWS_UdpNm_00096 */
/** @req SWS_UdpNm_00097 */
/** @req SWS_UdpNm_00100 */
static inline void UdpNm_Internal_PrepareBusSleep_to_RepeatMessage( const UdpNm_ChannelType* ChannelConf, UdpNm_Internal_ChannelType* ChannelInternal ) {
    ChannelInternal->Mode = NM_MODE_NETWORK;
    ChannelInternal->State = NM_STATE_REPEAT_MESSAGE;
    ChannelInternal->TimeoutTimeLeft = ChannelConf->UdpNmTimeoutTime + ChannelConf->UdpNmMainFunctionPeriod;
    ChannelInternal->RepeatMessageTimeLeft = ChannelConf->UdpNmRepeatMessageTime + ChannelConf->UdpNmMainFunctionPeriod;
    ChannelInternal->MessageCycleTimeLeft = ChannelConf->UdpNmMsgCycleOffset + ChannelConf->UdpNmMainFunctionPeriod;
    // Notify 'Network Mode'
    Nm_NetworkMode(ChannelConf->UdpNmComMChannelIdRef);
}

/** @req SWS_UdpNm_00126 */
static inline void UdpNm_Internal_PrepareBusSleep_to_BusSleep( const UdpNm_ChannelType* ChannelConf, UdpNm_Internal_ChannelType* ChannelInternal ){
    ChannelInternal->Mode = NM_MODE_BUS_SLEEP;
    ChannelInternal->State = NM_STATE_BUS_SLEEP;
    // Notify 'Bus-Sleep Mode'
    Nm_BusSleepMode(ChannelConf->UdpNmComMChannelIdRef);
}

static inline void UdpNm_Internal_BusSleep_to_RepeatMessage( const UdpNm_ChannelType* ChannelConf, UdpNm_Internal_ChannelType* ChannelInternal ) {
    ChannelInternal->Mode = NM_MODE_NETWORK;
    ChannelInternal->State = NM_STATE_REPEAT_MESSAGE;
    ChannelInternal->TimeoutTimeLeft = ChannelConf->UdpNmTimeoutTime + ChannelConf->UdpNmMainFunctionPeriod;
    ChannelInternal->RepeatMessageTimeLeft = ChannelConf->UdpNmRepeatMessageTime + ChannelConf->UdpNmMainFunctionPeriod;
    ChannelInternal->MessageCycleTimeLeft = ChannelConf->UdpNmMsgCycleOffset + ChannelConf->UdpNmMainFunctionPeriod;
    // Notify 'Network Mode'
    Nm_NetworkMode(ChannelConf->UdpNmComMChannelIdRef);
}

/** @req SWS_UdpNm_00127 */
static inline void UdpNm_Internal_BusSleep_to_BusSleep( const UdpNm_ChannelType* ChannelConf ){
#if  (UDPNM_PNC_COUNT == 0)
    Nm_NetworkStartIndication(ChannelConf->UdpNmComMChannelIdRef);
#else
    /*lint -estring(920,pointer) Cast to void. OK, used depending on the config. */
    (void) ChannelConf;
#endif
}

static inline void UdpNm_Internal_RepeatMessage_to_RepeatMessage( const UdpNm_ChannelType* ChannelConf, UdpNm_Internal_ChannelType* ChannelInternal ){
    ChannelInternal->TimeoutTimeLeft = ChannelConf->UdpNmTimeoutTime;
}

/*lint -save -e715  Symbol ChannelConf not referenced. OK, depends on the define. */
/** @req SWS_UdpNm_00107 */
static inline void UdpNm_Internal_RepeatMessage_to_ReadySleep( const UdpNm_ChannelType* ChannelConf, UdpNm_Internal_ChannelType* ChannelInternal ){
    ChannelInternal->Mode = NM_MODE_NETWORK;
    ChannelInternal->State = NM_STATE_READY_SLEEP;
#if (UDPNM_NODE_DETECTION_ENABLED == STD_ON)
    UdpNm_Internal_ClearCbv(ChannelConf, ChannelInternal);
#endif
}
/*lint -restore -e715 */

/*lint -save -e715  Symbol ChannelConf not referenced. OK, depends on the define. */
static inline void UdpNm_Internal_RepeatMessage_to_NormalOperation( const UdpNm_ChannelType* ChannelConf, UdpNm_Internal_ChannelType* ChannelInternal ){
    ChannelInternal->Mode = NM_MODE_NETWORK;
    ChannelInternal->State = NM_STATE_NORMAL_OPERATION;
#if (UDPNM_NODE_DETECTION_ENABLED == STD_ON)
    UdpNm_Internal_ClearCbv(ChannelConf, ChannelInternal);
#endif
}
/*lint -restore -e715 */

static inline void UdpNm_Internal_NormalOperation_to_RepeatMessage( const UdpNm_ChannelType* ChannelConf, UdpNm_Internal_ChannelType* ChannelInternal ){
    ChannelInternal->Mode = NM_MODE_NETWORK;
    ChannelInternal->State = NM_STATE_REPEAT_MESSAGE;
    ChannelInternal->RepeatMessageTimeLeft = ChannelConf->UdpNmRepeatMessageTime + ChannelConf->UdpNmMainFunctionPeriod;
    ChannelInternal->MessageCycleTimeLeft = ChannelConf->UdpNmMsgCycleOffset + ChannelConf->UdpNmMainFunctionPeriod;
}

/** @req SWS_UdpNm_00118 */
static inline void UdpNm_Internal_NormalOperation_to_ReadySleep( UdpNm_Internal_ChannelType* ChannelInternal ){
    ChannelInternal->Mode = NM_MODE_NETWORK;
    ChannelInternal->State = NM_STATE_READY_SLEEP;
}

static inline void UdpNm_Internal_NormalOperation_to_NormalOperation( const UdpNm_ChannelType* ChannelConf, UdpNm_Internal_ChannelType* ChannelInternal ){
    ChannelInternal->TimeoutTimeLeft = ChannelConf->UdpNmTimeoutTime;
}

static inline void UdpNm_Internal_ReadySleep_to_NormalOperation( const UdpNm_ChannelType* ChannelConf, UdpNm_Internal_ChannelType* ChannelInternal ){
    ChannelInternal->Mode = NM_MODE_NETWORK;
    ChannelInternal->State = NM_STATE_NORMAL_OPERATION;
    ChannelInternal->MessageCycleTimeLeft = ChannelConf->UdpNmMsgCycleOffset + ChannelConf->UdpNmMainFunctionPeriod;
}

/** @req SWS_UdpNm_00114 */
static inline void UdpNm_Internal_ReadySleep_to_PrepareBusSleep( const UdpNm_ChannelType* ChannelConf, UdpNm_Internal_ChannelType* ChannelInternal ){
    ChannelInternal->Mode = NM_MODE_PREPARE_BUS_SLEEP;
    ChannelInternal->State = NM_STATE_PREPARE_BUS_SLEEP;
    ChannelInternal->WaitBusSleepTimeLeft = ChannelConf->UdpNmWaitBusSleepTime;
    // Notify 'Prepare Bus-Sleep Mode'
    Nm_PrepareBusSleepMode(ChannelConf->UdpNmComMChannelIdRef);
}

static inline void UdpNm_Internal_ReadySleep_to_RepeatMessage( const UdpNm_ChannelType* ChannelConf, UdpNm_Internal_ChannelType* ChannelInternal ){
    ChannelInternal->Mode = NM_MODE_NETWORK;
    ChannelInternal->State = NM_STATE_REPEAT_MESSAGE;
    ChannelInternal->RepeatMessageTimeLeft = ChannelConf->UdpNmRepeatMessageTime + ChannelConf->UdpNmMainFunctionPeriod;
    ChannelInternal->MessageCycleTimeLeft = ChannelConf->UdpNmMsgCycleOffset + ChannelConf->UdpNmMainFunctionPeriod;
}

static inline void UdpNm_Internal_NetworkMode_to_NetworkMode( const UdpNm_ChannelType* ChannelConf, UdpNm_Internal_ChannelType* ChannelInternal ){
    ChannelInternal->TimeoutTimeLeft = ChannelConf->UdpNmTimeoutTime + ChannelConf->UdpNmMainFunctionPeriod;
}

#if (UDPNM_PNC_COUNT > 0)
#if (UDPNM_PNC_EIRA_CALC_ENABLED == STD_ON)
/**
 * @brief UdpNm_Internal_ResetEiraPnBits reset EIRA bits in case of timeout
 * @param pnIndex - Indices of PNC which have timed out
 * @param indexCount - No of PNC that have timed out
 * @return void
 */
static void UdpNm_Internal_ResetEiraPnBits(const uint8 *pnIndex, uint8 indexCount) {

    uint8 byteNo;
    uint8 bit;
    uint8 idx;

    for (idx = 0; idx < indexCount; idx++) {

        byteNo     = pnIndex[idx] / 8;
        bit        = pnIndex[idx] % 8;
        /** @req SWS_UdpNm_00351 */
        UdpNm_Internal.pnEIRABytes[byteNo] &= ~(1u << bit); /* Reset PN bit */
    }

#if (UDPNM_PDU_RX_INDICATION_ENABLED == STD_ON)
    PduInfoType pdu = {
        .SduDataPtr = &UdpNm_Internal.pnEIRABytes[0],
        .SduLength = UdpNm_ConfigPtr->UdpNmPnInfo->UdpNmPnInfoLen,
    };

    /** @req SWS_UdpNm_00352 */
    PduR_UdpNmRxIndication(UdpNm_ConfigPtr->UdpNmPnInfo->UdpNmEIRARxNSduId, &pdu);
#endif
}

static void UdpNm_Internal_TickPnEiraResetTime(uint32 period) {

    uint8 pncIndexReset[UDPNM_PNC_COUNT] = {0};
    uint8 len;
    len = 0;
    uint8 timerIdx;
    SchM_Enter_UdpNm_EA_0();

    for (timerIdx = 0; timerIdx < UDPNM_PNC_COUNT; timerIdx++) {

        if (TRUE == UdpNm_Internal.pnEIRATimers[timerIdx].timerRunning) {

            if (period >= UdpNm_Internal.pnEIRATimers[timerIdx].resetTimer) {
                UdpNm_Internal.pnEIRATimers[timerIdx].timerRunning = FALSE;
                pncIndexReset[len] = UdpNm_ConfigPtr->UdpNmPnInfo->UdpNmTimerIndexToPnMap[timerIdx];
                len++;
            } else {
                UdpNm_Internal.pnEIRATimers[timerIdx].resetTimer -= period;
            }
        } else {
            /* Do nothing */
        }
    }

    if (len > 0) {
        UdpNm_Internal_ResetEiraPnBits(pncIndexReset, len);
    } else {
        /* Do nothing */
    }

    SchM_Exit_UdpNm_EA_0();
}

/**
 * @brief restartPNTimer Restart PN timers
 * @param pnIndex - Index of PNC
 * @return void
 */
static void restartPNTimer(uint8 pnIndex) {

    uint8 timerIndex;

    timerIndex = UdpNm_ConfigPtr->UdpNmPnInfo->UdpNmPnIndexToTimerMap[pnIndex];

    UdpNm_Internal.pnEIRATimers[timerIndex].timerRunning = TRUE;
    UdpNm_Internal.pnEIRATimers[timerIndex].resetTimer = UDPNM_PNC_RESET_TIME;
}

/**
 * @brief restartTimerForInternalExternalRequests Identify the PN timer that needs to be restarted
 * @param calcEIRA - EIRA of the new received/transmitted and filtered PDU
 * @return void
 */
static void restartTimerForInternalExternalRequests(const uint8 *calcEIRA) {

    uint8 byteNo;
    uint8 bit;
    uint8 byteEIRA;

    for (byteNo = 0; byteNo < UDPNM_PNC_INFO_LEN; byteNo++) {

        byteEIRA = calcEIRA[byteNo];

        if (byteEIRA > 0) {

            for (bit = 0; bit < 8; bit++) {

                if (((byteEIRA >> bit) & UDPNM_LSBIT_MASK) == UDPNM_LSBIT_MASK) {
                    restartPNTimer((byteNo * 8) + bit); /*lint !e734 Loss of precision. OK, the result will not be bigger than what can fit in 8 bits. */
                } else {
                    /* Do nothing */
                }
            }
        } else {
            /* Do nothing */
        }
    }
}

static void UdpNm_Internal_ProcessTxPdu(const uint8 * const pnInfo,
        const UdpNm_ChannelType* ChannelConf,
        UdpNm_Internal_ChannelType* ChannelInternal) {

    uint8 calculatedEIRA[UDPNM_PNC_INFO_LEN] = {0};
    boolean eiraIsZero = TRUE;

    /* Using the main function service ID since that is where it is called from. */
    UDPNM_VALIDATE_NOTNULL(pnInfo, UDPNM_SERVICEID_ARC_MAINFUNCTION);

    for (uint8 byteNo = 0; byteNo < UDPNM_PNC_INFO_LEN; byteNo++) {
        /* Accumulate internal requests */
        calculatedEIRA[byteNo] = pnInfo[byteNo] & UdpNm_ConfigPtr->UdpNmPnInfo->UdpNmPnInfoMask[byteNo];

        if (calculatedEIRA[byteNo] != 0) {
            eiraIsZero = FALSE;
        }
    }

    if (FALSE == eiraIsZero) {

        /** @req SWS_UdpNm_00348 */
        /** @req SWS_UdpNm_00349 */
        /** @req SWS_UdpNm_00352 */
        UdpNm_Internal_UpdateRequestArray(TRUE, calculatedEIRA, ChannelConf, ChannelInternal);
    } else {
        /* Do nothing */
    }
}
#endif /* (UDPNM_PNC_EIRA_CALC_ENABLED == STD_ON) */

#if (UDPNM_PNC_ERA_CALC_ENABLED == STD_ON)
/**
 * @brief restartPNERATimer restart PN timers
 * @param pnIndex - Index of PNC
 * @return void
 */
static void restartPNERATimer(UdpNm_Internal_ChannelType* ChannelInternal, uint8 pnIndex) {

    uint8 timerIndex = UdpNm_ConfigPtr->UdpNmPnInfo->UdpNmPnIndexToTimerMap[pnIndex];

    ChannelInternal->pnERATimers[timerIndex].timerRunning = TRUE;
    ChannelInternal->pnERATimers[timerIndex].resetTimer = UDPNM_PNC_RESET_TIME;
}

/**
 * @brief restartTimerForExternalRequests Identify the PN timer that needs to be restarted
 * @param calcERA - ERA of the new received/transmitted and filtered PDU
 * @return void
 */
static void restartTimerForExternalRequests(UdpNm_Internal_ChannelType* ChannelInternal,
        const uint8 *calcERA) {

    uint8 byteNo;
    uint8 bit;
    uint8 byteERA;

    for (byteNo = 0; byteNo < UDPNM_PNC_INFO_LEN; byteNo++) {

        byteERA = calcERA[byteNo];

        if (byteERA > 0) {

            for (bit = 0; bit < 8; bit++) {

                if (((byteERA >> bit) & UDPNM_LSBIT_MASK) == UDPNM_LSBIT_MASK) {
                    restartPNERATimer(ChannelInternal, (byteNo * 8) + bit); /*lint !e734 Loss of precision. OK, the result will not be bigger than what can fit in 8 bits. */
                } else {
                    /* Do nothing */
                }
            }
        } else {
            /* Do nothing */
        }
    }
}

/**
 * @brief UdpNm_Internal_ResetEraPnbits Reset ERA bits in case of timeout
 * @param pnIndex - Indices of PNC which have timed out
 * @param indexCount - No of PNC that have timed out
 * @return void
 */
static void UdpNm_Internal_ResetEraPnbits(uint8 chanIndex, const uint8 *pnIndex, uint8 indexCount) {

    uint8 byteNo;
    uint8 bit;
    uint8 idx;

    for (idx = 0; idx < indexCount; idx++) {
        byteNo     = pnIndex[idx] / 8;
        bit        = pnIndex[idx] % 8;
        /** @req SWS_UdpNm_00360 */
        UdpNm_Internal.Channels[chanIndex].pnERABytes[byteNo] &= ~(1u << bit); /* Reset PN bit */
    }

    PduInfoType pdu = {
        .SduDataPtr = &UdpNm_Internal.Channels[chanIndex].pnERABytes[0],
        .SduLength = UdpNm_ConfigPtr->UdpNmPnInfo->UdpNmPnInfoLen,
    };

#if (UDPNM_PDU_RX_INDICATION_ENABLED == STD_ON)
    /** @req SWS_UdpNm_00361 */
    PduR_UdpNmRxIndication(UdpNm_ConfigPtr->Channels[chanIndex].UdpNmERARxNSduId, &pdu);
#endif
}

static void UdpNm_Internal_TickPnEraResetTime(const uint8 chanIdx) {

    uint8 pncIndexReset[UDPNM_PNC_COUNT] = {0};
    uint8 len = 0;
    uint8 timerIdx;

    SchM_Enter_UdpNm_EA_0();

    for (timerIdx = 0; timerIdx < UDPNM_PNC_COUNT; timerIdx++) {
        if (TRUE == UdpNm_Internal.Channels[chanIdx].pnERATimers[timerIdx].timerRunning) {
            if (UdpNm_ConfigPtr->Channels[chanIdx].UdpNmMainFunctionPeriod >= UdpNm_Internal.Channels[chanIdx].pnERATimers[timerIdx].resetTimer) {
                UdpNm_Internal.Channels[chanIdx].pnERATimers[timerIdx].timerRunning = FALSE;
                pncIndexReset[len] = UdpNm_ConfigPtr->UdpNmPnInfo->UdpNmTimerIndexToPnMap[timerIdx];
                len++;
            } else {
                UdpNm_Internal.Channels[chanIdx].pnERATimers[timerIdx].resetTimer -= UdpNm_ConfigPtr->Channels[chanIdx].UdpNmMainFunctionPeriod;
            }
        }
    }

    if (len > 0) {
        UdpNm_Internal_ResetEraPnbits(chanIdx, pncIndexReset, len);
    }

    SchM_Exit_UdpNm_EA_0();
}
#endif /* UDPNM_PNC_ERA_CALC_ENABLED */

static void UdpNm_Internal_UpdateRequestArray(boolean updateEira,
        const uint8 *calculatedRa,
        const UdpNm_ChannelType *ChannelConf,
        UdpNm_Internal_ChannelType *ChannelInternal) {

    uint8 oldRequestArray[UDPNM_PNC_INFO_LEN] = {0};
    boolean changed = FALSE;
    uint8 *requestAggregate = NULL;
    PduIdType sduId = 0;

    SchM_Enter_UdpNm_EA_0();

#if (UDPNM_PNC_EIRA_CALC_ENABLED == STD_ON)
    if (TRUE == updateEira) {
        restartTimerForInternalExternalRequests(calculatedRa);
        memcpy(&oldRequestArray[0], &UdpNm_Internal.pnEIRABytes[0], UDPNM_PNC_INFO_LEN);
        requestAggregate = &UdpNm_Internal.pnEIRABytes[0];
        sduId = UdpNm_ConfigPtr->UdpNmPnInfo->UdpNmEIRARxNSduId;
    }
#endif
#if ((UDPNM_PNC_EIRA_CALC_ENABLED == STD_OFF) && (UDPNM_PNC_ERA_CALC_ENABLED == STD_ON))
    if (!updateEira) {
#elif ((UDPNM_PNC_EIRA_CALC_ENABLED == STD_ON) && (UDPNM_PNC_ERA_CALC_ENABLED == STD_ON))
    else {
#endif
#if (UDPNM_PNC_ERA_CALC_ENABLED == STD_ON)
        restartTimerForExternalRequests(ChannelInternal, calculatedRa);
        memcpy(&oldRequestArray[0], &ChannelInternal->pnERABytes[0], UDPNM_PNC_INFO_LEN);
        requestAggregate = &ChannelInternal->pnERABytes[0];
        sduId = ChannelConf->UdpNmERARxNSduId;
    }
#endif
#if (((UDPNM_PNC_EIRA_CALC_ENABLED == STD_OFF) && (UDPNM_PNC_ERA_CALC_ENABLED == STD_ON)) || ((UDPNM_PNC_EIRA_CALC_ENABLED == STD_ON) && (UDPNM_PNC_ERA_CALC_ENABLED == STD_OFF)))
    else {
        /* We only end up here if only EIRA or ERA is enabled and the updateEira
         * parameter does not match this which means a programming error.
         * Improvement: Add Det error. */
        return;
    }
#endif

    for (uint8 i = 0; i < UDPNM_PNC_INFO_LEN; i++) {
        requestAggregate[i] |= calculatedRa[i];

        if (requestAggregate[i] != oldRequestArray[i]) {
            changed = TRUE;
        }
    }

    SchM_Exit_UdpNm_EA_0();

#if (UDPNM_PDU_RX_INDICATION_ENABLED == STD_ON)
    if (TRUE == changed) {
        PduInfoType pdu = {
            .SduDataPtr = requestAggregate,
            .SduLength = UdpNm_ConfigPtr->UdpNmPnInfo->UdpNmPnInfoLen,
        };
        PduR_UdpNmRxIndication(sduId, &pdu);
    } else {
        /* Do nothing */
    }
#else /* (UDPNM_PDU_RX_INDICATION_ENABLED == STD_ON) */
    (void) changed;
#endif /* (UDPNM_PDU_RX_INDICATION_ENABLED == STD_OFF) */
}

static Std_ReturnType UdpNm_Internal_RxProcess(
        const UdpNm_ChannelType* ChannelConf,
        UdpNm_Internal_ChannelType* ChannelInternal,
        boolean pni) {

    uint8 calculatedRequestArray[UDPNM_PNC_INFO_LEN] = {0};
    uint8 i;
    Std_ReturnType ret;
    boolean requestArrayIsZero;

    requestArrayIsZero = TRUE;
    /** @req SWS_UdpNm_00328 *//* No pn normal reception */
    /** @req SWS_UdpNm_00329 *//* No pni in cbv -  normal reception */
    ret = E_OK;

    if ((TRUE == ChannelConf->UdpNmPnEnable)
            && (TRUE == pni)){

        /** @req SWS_UdpNm_00331 */
        /* NM PDU Filter Algorithm */
        /** @req SWS_UdpNm_00335 */
        /** @req SWS_UdpNm_00336 */
        /** @req SWS_UdpNm_00337 */
        /** @req SWS_UdpNm_00338 */
        /** @req SWS_UdpNm_00339 */
        for (i = 0; i < UDPNM_PNC_INFO_LEN; i++) {

            calculatedRequestArray[i] = ChannelInternal->RxMessageSdu[UdpNm_ConfigPtr->UdpNmPnInfo->UdpNmPnInfoOffset + i] &
                    UdpNm_ConfigPtr->UdpNmPnInfo->UdpNmPnInfoMask[i]; /* Accumulate external requests */

            if (calculatedRequestArray[i] != 0) {
                requestArrayIsZero = FALSE;
            }
        }
        if (FALSE == requestArrayIsZero) {

#if (UDPNM_PNC_EIRA_CALC_ENABLED == STD_ON)
            /** @req SWS_UdpNm_00347 */
            /** @req SWS_UdpNm_00349 */
            /** @req SWS_UdpNm_00352 */
            UdpNm_Internal_UpdateRequestArray(TRUE, calculatedRequestArray, ChannelConf, ChannelInternal);
#endif /* (UDPNM_PNC_EIRA_CALC_ENABLED == STD_ON) */
#if (UDPNM_PNC_ERA_CALC_ENABLED == STD_ON)
            if (TRUE == ChannelConf->UdpNmPnEraCalcEnabled) {
                /** @req SWS_UdpNm_00357 */
                /** @req SWS_UdpNm_00359 */
                /** @req SWS_UdpNm_00361 */
                UdpNm_Internal_UpdateRequestArray(FALSE, calculatedRequestArray, ChannelConf, ChannelInternal);
            }
#endif /* (UDPNM_PNC_ERA_CALC_ENABLED == STD_ON) */
        }
    }
    return ret;
}

static void UdpNm_Internal_SetPniCbv(
        const UdpNm_ChannelType* ChannelConf,
        UdpNm_Internal_ChannelType* ChannelInternal) {

    if (ChannelConf->UdpNmPduCbvPosition != UDPNM_PDU_OFF) {
        ChannelInternal->TxMessageSdu[ChannelConf->UdpNmPduCbvPosition] |= UDPNM_CBV_PNI;
    }
}
#endif /* (UDPNM_PNC_COUNT > 0) */

#ifdef HOST_TEST
void UdpNm_Test_GetChannelRunTimeData(uint32 channelId, uint32* messageCycleTimeLeft, uint32* timeoutTimeLeft) {

    const UdpNm_Internal_ChannelType* ChannelInternal = &UdpNm_Internal.Channels[channelId];

    *messageCycleTimeLeft =  ChannelInternal->MessageCycleTimeLeft;
    *timeoutTimeLeft = ChannelInternal->TimeoutTimeLeft;
}

void UdpNm_Test_GetCbvPosition(uint32 channelId, UdpNm_PduPositionType* UdpCbvPosition) {

    const UdpNm_Internal_ChannelType* ChannelInternal = &UdpNm_Internal.Channels[channelId];
    const NetworkHandleType chanIndex = UdpNm_ConfigPtr->ChannelMap[channelId];
    const UdpNm_ChannelType* ChannelConf = &UdpNm_ConfigPtr->Channels[chanIndex];

    *UdpCbvPosition = (UdpNm_PduPositionType)ChannelInternal->TxMessageSdu[ChannelConf->UdpNmPduCbvPosition];
}

#if (UDPNM_PNC_COUNT > 0)
Std_ReturnType UdpNm_Test_VerifyAllEiraBitsZero(void) {

    Std_ReturnType ret;

    ret = E_OK;
    for (uint8 i = 0; i < UDPNM_PNC_INFO_LEN; i++) {

        if (UdpNm_Internal.pnEIRABytes[i] != 0) {
            ret = E_NOT_OK;
            break;
        }
    }

    return ret;
}

Std_ReturnType UdpNm_Test_VerifyAllEraBitsZero(void) {
    Std_ReturnType ret;

    ret = E_OK;

    for (uint8 chanIndex = 0; chanIndex < UDPNM_CHANNEL_COUNT; chanIndex++) {
        for (uint8 eraByteindex = 0; eraByteindex < UDPNM_PNC_INFO_LEN; eraByteindex++) {
            if (UdpNm_Internal.Channels[chanIndex].pnERABytes[eraByteindex] != 0) {
                ret= E_NOT_OK;
                break;
            }
        }
    }

    return ret;
}

boolean UdpNm_Test_IsPnEiraBitSet(uint8 eiraByteIndex, uint8 pnBitIndex) {
    return (UdpNm_Internal.pnEIRABytes[eiraByteIndex] & (1u << pnBitIndex)) == (1u << pnBitIndex);
}

boolean UdpNm_Test_IsPnEraBitSet(uint8 chanIndex, uint8 eraByteIndex, uint8 pnBitIndex) {
    boolean ret;

    ret = FALSE;
    if ((UdpNm_Internal.Channels[chanIndex].pnERABytes[eraByteIndex] & (1u << pnBitIndex)) == (1u << pnBitIndex)) {
        ret = TRUE;
    }

    return ret;
}

uint32 UdpNm_Test_GetEiraResetTimer(uint8 pnIdx) {
    return UdpNm_Internal.pnEIRATimers[UdpNm_ConfigPtr->UdpNmPnInfo->UdpNmPnIndexToTimerMap[pnIdx]].resetTimer;
}

uint32 UdpNm_Test_GetEraResetTimer(uint8 chanIdx, uint8 pnIdx) {
    uint8 timerIdx = UdpNm_ConfigPtr->UdpNmPnInfo->UdpNmPnIndexToTimerMap[pnIdx];
    return UdpNm_Internal.Channels[chanIdx].pnERATimers[timerIdx].resetTimer;
}
#endif /* (UDPNM_PNC_COUNT > 0) */
#endif /* HOST_TEST */
