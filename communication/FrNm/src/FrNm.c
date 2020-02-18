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
/** @req FRNM102 */ /* FlexRay NM state machine shall contain states, transitions and triggers coordination algorithm from the point of view of one single node in the NM-cluster */
/** @req FRNM103 */ /* Transitions FlexRay NM state machine shall be triggered by calls interface functions or by the expiration of internal timers or counters.*/
/** @req FRNM177 */ /*FrNm module shall not use asynchronous NM-Task activation.*/
/** @req FRNM179 */ /*The AUTOSAR FlexRay NM algorithm shall support up to 64 nodes per NM-Cluster */
/** @req FRNM056 *//* if det enabled => parameter checking */
/** @req FRNM064 */ /** @req FRNM066 */
/** @req FRNM225 */ /* General requirement for NM coordination algorithm shall not rely on any processor specific hardware support*/
/** @req FRNM176*/ /* General requirement for FlexRay NM shall realize FlexRay NM functions for reception and for transmission of NM Messages */
/** @req FRNM196 */ /* This assumed integrator will configure FrNmRepetitionCycle will be an integer multiple of the NM Voting Cycle  */
/** @req FRNM194 */ /** @req FRNM193 */ /* assumed FrNmDataCycle and FrNmVotingCycle is not in the scope of FRNM it should be at global level
 * configured by integrator for FrIf or FrDrv, because FRNM has no information about segments and slots of transmission */
/** @req FRNM058 */ /* This is a general requirement as The FlexRay NM decisions shall be influenced by every received NM-Vote */
/** @req FRNM372 */ /* This is the global flexray cycle configuration requirement for cycle evaluation of FrNmMainAcrossFrCycle with respect to os scheduling  */

#include "ComStack_Types.h"
#include "ComM.h"
#include "NmStack_Types.h"
#include "Nm.h"
#include "Nm_Cbk.h"/** @req FRNM064 */ /** @req FRNM368 */
#include "FrNm.h" /* @req FRNM064 */ /* @req FRNM367 */ /* @req FRNM065 */
#include "FrNm_Cbk.h" /* @req FRNM065 */
#include "PduR_FrNm.h"
#include "SchM_FrNm.h"
#include "FrNm_Internal.h"
#include <string.h>

#if (FRNM_DEV_ERROR_DETECT == STD_ON)/* @req FRNM049 */
#include "Det.h"
#endif

#if defined(USE_DEM)
#include "Dem.h"
#endif

const FrNm_ConfigType* FrNm_ConfigPtr;

/* @req FRNM071 */ /* @req FRNM421 */
FrNm_InternalType FrNm_Internal = {
        .InitStatus = FRNM_STATUS_UNINIT,
};

/**
 *
 * @param frnmConfigPtr
 */
/* Initialize the complete FrNm module, i.e. all channels which are activated */
/* @req FRNM236 */
void FrNm_Init( const FrNm_ConfigType * const frnmConfigPtr ){
    const FrNm_ChannelInfoType* FrNmChannelsconf;
    FrNm_Internal_ChannelType* ChannelInternal;
#if (FRNM_PASSIVE_MODE_ENABLED == STD_OFF)
    uint8* destUserData;
    uint8 userDataLength;
    uint8 txPduCount;
#endif
    uint8 channel;
    uint8 pdu;
    uint8 rxPduCount;


    /* @req FRNM395 */
    FRNM_DET_REPORTERROR((NULL != frnmConfigPtr),FRNM_SERVICE_ID_INIT,FRNM_E_INVALID_POINTER);

    FrNm_ConfigPtr = frnmConfigPtr;  /* @req FRNM059 */

    for (channel=0; channel < FrNm_ConfigPtr->FrNmChnlCount; channel++){
        FrNmChannelsconf = &FrNm_ConfigPtr->FrNmChannels[channel];
        ChannelInternal = &FrNm_Internal.FrNmChannels[channel];
        /* @req FRNM030*/ /* @req FRNM307*/ /*if in any mode init is called will change to bus sleep*/
        ChannelInternal->Mode = NM_MODE_BUS_SLEEP;
        ChannelInternal->State = NM_STATE_BUS_SLEEP;
        ChannelInternal->CommunicationEnabled = TRUE;
        ChannelInternal->activeWakeup = FALSE;
        /* @req FRNM136*/
        ChannelInternal->FrNm_NetworkRequested = FALSE;
        ChannelInternal->FrNm_RepeatMessage  = FALSE;
        rxPduCount = FrNmChannelsconf->FrNmChannelIdentifiersConfig->FrNmRxPduCount;


#if (FRNM_PASSIVE_MODE_ENABLED == STD_OFF)
        txPduCount = FrNmChannelsconf->FrNmChannelIdentifiersConfig->FrNmTxPduCount;
        for (pdu = 0; pdu<txPduCount; pdu++){
           if(FrNmChannelsconf->FrNmChannelIdentifiersConfig->FrNmTxPduList[pdu].FrNmTxConatainsVote == STD_ON) {
               /* @req FRNM042 */
               ChannelInternal->FrNmTxVotePdu = 0x00;
           } else {
               /* @req FRNM042 */
               ChannelInternal->FrNmTxVotePdu = 0x00;
           }
           if(FrNmChannelsconf->FrNmChannelIdentifiersConfig->FrNmTxPduList[pdu].FrNmTxContainsData == STD_ON) {
               memset(ChannelInternal->FrNmTxDataPdu, 0x00, 8);
           }
        }
        memset(ChannelInternal->FrNmTxUserdataMessagePdu, 0x00, 8);

        destUserData = &ChannelInternal->FrNmTxDataPdu[FRNM_INTERNAL_GET_USER_DATA_OFFSET];
        userDataLength = FrNmChannelsconf->FrNmChannelIdentifiersConfig->FrNmTxPduLength - FRNM_INTERNAL_GET_USER_DATA_OFFSET; /* @req FRNM155 */

        /* @req FRNM045 */
        memset(destUserData, 0xFF, userDataLength);
#endif

        for (pdu = 0; pdu<rxPduCount; pdu++){

          if(FrNmChannelsconf->FrNmChannelIdentifiersConfig->FrNmRxPduList[pdu].FrNmRxContainsData == STD_ON) {
              memset(ChannelInternal->FrNmRxDataPdu, 0x00, 8);
          }
        }

        /* @req FRNM222 */ /* @req FRNM037 */
#if (FRNM_SOURCE_NODE_INDENTIFIER_ENABLED == STD_ON)
        ChannelInternal->FrNmTxDataPdu[FRNM_SOURCE_NODE_ID_POSITION] = FrNmChannelsconf->FrNmChannelIdentifiersConfig->FrNmNodeId;
#endif
        ChannelInternal->MessageTimeoutTimeLeft = 0;
        ChannelInternal->RepetitionCyclesLeft = (uint8)(FrNmChannelsconf->FrNmTimingConfig->FrNmRepetitionCycle);
        ChannelInternal->DataCyclesLeft = (uint8)(FrNmChannelsconf->FrNmTimingConfig->FrNmDataCycle);
        ChannelInternal->readySleepCntLeft = FrNmChannelsconf->FrNmTimingConfig->FrNmReadySleepCnt;
        ChannelInternal->setUserDataEnable = FALSE;
    }

    /* Initialize the PN reset timer */
#if ((FRNM_PNC_EIRA_CALC_ENABLED == STD_ON) && (FRNM_PNC_COUNT > 0))
    for (uint8 i = 0; i < FRNM_PNC_COUNT; i++) {
        FrNm_Internal.pnEIRATimers[i].timerRunning = FALSE;
        FrNm_Internal.pnEIRATimers[i].resetTimer = 0;
    }
    FrNm_Internal.pnEIRA.data = 0; /* reset EIRA */
#endif

    /* @req FRNM073 */
    FrNm_Internal.InitStatus = FRNM_STATUS_INIT;
    /* @req FRNM028 */
}

/**
 *
 * @param NetworkHandle
 * @return
 */
/* Initiates the Passive Startup of the FlexRay NM*/
/* @req FRNM237 */
Std_ReturnType FrNm_PassiveStartUp( const NetworkHandleType NetworkHandle ) {
    /* This function full implementation currently in Not Supported requirements */

    FrNm_Internal_ChannelType* ChannelInternal;
    Std_ReturnType status;
    uint8 channelIndex;

    /* @req FRNM050 *//* @req FRNM032 */
    FRNM_DET_REPORTERROR((FRNM_STATUS_INIT == FrNm_Internal.InitStatus),FRNM_SERVICE_ID_PASSIVESTARTUP,FRNM_E_UINIT, E_NOT_OK);
    /* @req FRNM051 */  /* @req FRNM021 */
    FRNM_VALIDATE_CHANNEL(NetworkHandle, FRNM_SERVICE_ID_PASSIVESTARTUP, E_NOT_OK);

    /* @req FRNM258 */
    status = E_OK;
    channelIndex = FrNm_ConfigPtr->FrNmChannelLookups[NetworkHandle];
    ChannelInternal = &FrNm_Internal.FrNmChannels[channelIndex];
    if (ChannelInternal->Mode == NM_MODE_BUS_SLEEP) {
        /* @req FRNM317*/
        /* @req FRNM138*/
        ChannelInternal->Mode = NM_MODE_SYNCHRONIZE;
        ChannelInternal->State = NM_STATE_SYNCHRONIZE;
    } else {
        status = E_NOT_OK; /* @req FRNM260*/
    }

    return status;
}

/**
 *
 * @param NetworkHandle
 * @return
 */
/* it for requesting the network communicate ECU on the bus. Update state to ‘requested’ */
/* @req FRNM238 */
#if(FRNM_PASSIVE_MODE_ENABLED == STD_OFF) /* @req FRNM261 */
Std_ReturnType FrNm_NetworkRequest( const NetworkHandleType NetworkHandle ) {
    uint8 channelIndex;
    const FrNm_ChannelInfoType* ChannelConf;
    FrNm_Internal_ChannelType* ChannelInternal;
    /* @req FRNM050 */ /* @req FRNM021 */ /* @req FRNM032 */
    FRNM_DET_REPORTERROR((FRNM_STATUS_INIT == FrNm_Internal.InitStatus),FRNM_SERVICE_ID_NETWORK_REQUEST,FRNM_E_UINIT, E_NOT_OK);
    /* @req FRNM051 */
    FRNM_VALIDATE_CHANNEL(NetworkHandle, FRNM_SERVICE_ID_NETWORK_REQUEST, E_NOT_OK);

    channelIndex = FrNm_ConfigPtr->FrNmChannelLookups[NetworkHandle];
    ChannelConf = &FrNm_ConfigPtr->FrNmChannels[channelIndex];
    ChannelInternal = &FrNm_Internal.FrNmChannels[channelIndex];

    /* @req FRNM139*/
    if (ChannelInternal->Mode == NM_MODE_BUS_SLEEP) {
        /* @req FRNM316*/
        ChannelInternal->Mode = NM_MODE_SYNCHRONIZE;
        ChannelInternal->State = NM_STATE_SYNCHRONIZE;
        ChannelInternal->FrNm_NetworkRequested = TRUE;
        ChannelInternal->activeWakeup = TRUE;
        ChannelInternal->RepetitionCyclesLeft = (uint8) ChannelConf->FrNmTimingConfig->FrNmRepetitionCycle;
        ChannelInternal->RepeatMessageTimeLeft = ChannelConf->FrNmTimingConfig->FrNmRepeatMessageTime;  /* @req FRNM117*/
    } else if (ChannelInternal->Mode == NM_MODE_SYNCHRONIZE){
        /* @req FRNM141*/
        ChannelInternal->FrNm_NetworkRequested = TRUE;
    } else if (ChannelInternal->Mode == NM_MODE_NETWORK){
        /* @req FRNM113*/
        ChannelInternal->FrNm_NetworkRequested = TRUE;
    } else{
        /*Nothing to be done */
    }

    return E_OK;
}
#endif

/**
 *
 * @param NetworkHandle
 * @return
 */
/* it for requesting the network does not communicate ECU on the bus. Update state to ‘released’ */
/* @req FRNM239 */
#if(FRNM_PASSIVE_MODE_ENABLED == STD_OFF) /* @req FRNM262 */
Std_ReturnType FrNm_NetworkRelease( const NetworkHandleType NetworkHandle )
{
    uint8 channelIndex;
    FrNm_Internal_ChannelType* ChannelInternal;
     /* @req FRNM050 */ /* @req FRNM032 */
    FRNM_DET_REPORTERROR((FRNM_STATUS_INIT == FrNm_Internal.InitStatus),FRNM_SERVICE_ID_NETWORK_RELEASE,FRNM_E_UINIT, E_NOT_OK);
    /* @req FRNM021 *//* @req FRNM051 */
    FRNM_VALIDATE_CHANNEL(NetworkHandle, FRNM_SERVICE_ID_NETWORK_RELEASE, E_NOT_OK);
    channelIndex = FrNm_ConfigPtr->FrNmChannelLookups[NetworkHandle];
    ChannelInternal = &FrNm_Internal.FrNmChannels[channelIndex];

    if((ChannelInternal->Mode == NM_MODE_SYNCHRONIZE)||(ChannelInternal->Mode == NM_MODE_NETWORK)){
        /* @req FRNM142*/  /* @req FRNM114*/
        ChannelInternal->FrNm_NetworkRequested = FALSE;
    }

    return E_OK;
}
#endif

/**
 *
 * @param NetworkHandle
 * @param nmUserDataPtr
 * @return
 */
/* This function sets user data for NM-Data transmitted next on the bus.*/
/* @req FRNM240 */
#if (FRNM_USER_DATA_ENABLED == STD_ON) /* @req FRNM263 */
Std_ReturnType FrNm_SetUserData( const NetworkHandleType NetworkHandle, const uint8* const nmUserDataPtr ) {

    uint8 channelIndex;
    const FrNm_ChannelInfoType* ChannelConf;
    FrNm_Internal_ChannelType* ChannelInternal;
    uint8* destUserData;
    uint8 userDataLength;
    /* @req FRNM050 */ /* @req FRNM032 */ /* @req FRNM021 */ /* @req FRNM319 */
    FRNM_DET_REPORTERROR((FRNM_STATUS_INIT == FrNm_Internal.InitStatus),FRNM_SERVICE_ID_SETUSERDATA,FRNM_E_UINIT, E_NOT_OK);
    /* @req FRNM051 */
    FRNM_VALIDATE_CHANNEL(NetworkHandle, FRNM_SERVICE_ID_SETUSERDATA, E_NOT_OK);
    FRNM_DET_REPORTERROR((NULL != nmUserDataPtr),FRNM_SERVICE_ID_SETUSERDATA,FRNM_E_INVALID_POINTER,E_NOT_OK);

    channelIndex = FrNm_ConfigPtr->FrNmChannelLookups[NetworkHandle];
    ChannelConf = &FrNm_ConfigPtr->FrNmChannels[channelIndex];
    ChannelInternal = &FrNm_Internal.FrNmChannels[channelIndex];
    /* @req FRNM043 */
    destUserData = &ChannelInternal->FrNmTxDataPdu[FRNM_INTERNAL_GET_USER_DATA_OFFSET];
    userDataLength = ChannelConf->FrNmChannelIdentifiersConfig->FrNmTxPduLength - FRNM_INTERNAL_GET_USER_DATA_OFFSET; /* @req FRNM155 */
    memcpy(destUserData, nmUserDataPtr, userDataLength);
    ChannelInternal->setUserDataEnable = TRUE;


    return E_OK;
}
#endif

/**
 *
 * @param NetworkHandle
 * @param nmUserDataPtr
 * @return
 */
/* This function gets user data form the last successfully received NM message .*/
/* @req FRNM241 */
#if (FRNM_USER_DATA_ENABLED == STD_ON) /* @req FRNM264 */
Std_ReturnType FrNm_GetUserData(const NetworkHandleType NetworkHandle, uint8* const nmUserDataPtr ) {
    uint8 channelIndex;
    const FrNm_ChannelInfoType* ChannelConf;
    const FrNm_Internal_ChannelType* ChannelInternal;
    const uint8* sourceUserData;
    uint8 userDataLength;
    /* @req FRNM050 */ /* @req FRNM032 */ /* @req FRNM021 */
    FRNM_DET_REPORTERROR((FRNM_STATUS_INIT == FrNm_Internal.InitStatus),FRNM_SERVICE_ID_GETUSERDATA,FRNM_E_UINIT, E_NOT_OK);
    /* @req FRNM051 */
    FRNM_VALIDATE_CHANNEL(NetworkHandle, FRNM_SERVICE_ID_GETUSERDATA, E_NOT_OK);
    FRNM_DET_REPORTERROR((NULL != nmUserDataPtr),FRNM_SERVICE_ID_GETUSERDATA,FRNM_E_INVALID_POINTER,E_NOT_OK);

    channelIndex = FrNm_ConfigPtr->FrNmChannelLookups[NetworkHandle];
    ChannelConf = &FrNm_ConfigPtr->FrNmChannels[channelIndex];
    ChannelInternal = &FrNm_Internal.FrNmChannels[channelIndex];
    sourceUserData = &ChannelInternal->FrNmRxDataPdu[FRNM_INTERNAL_GET_USER_DATA_OFFSET];
    userDataLength = ChannelConf->FrNmChannelIdentifiersConfig->FrNmRxPduLength - FRNM_INTERNAL_GET_USER_DATA_OFFSET; /* @req FRNM155 */
    /* @req FRNM044 */
    memcpy(nmUserDataPtr, sourceUserData, userDataLength);

    return E_OK;
}
#endif

/**
 *
 * @param NetworkHandle
 * @param nmPduData
 * @return
 */
/* This function gets Gets PDU data form the last successfully received NM message .*/
/* @req FRNM242 */
#if(FRNM_CONTROL_BIT_VECTOR_ENABLED == STD_ON)||(FRNM_SOURCE_NODE_INDENTIFIER_ENABLED==STD_ON)||(FRNM_USER_DATA_ENABLED == STD_ON)/* @req FRNM266 */
Std_ReturnType FrNm_GetPduData( const NetworkHandleType NetworkHandle, uint8* const nmPduData ) {
    uint8 channelIndex;
    const FrNm_ChannelInfoType* ChannelConf;
    const FrNm_Internal_ChannelType* ChannelInternal;
    /* @req FRNM050 */ /* @req FRNM021 */
    FRNM_DET_REPORTERROR((FRNM_STATUS_INIT == FrNm_Internal.InitStatus),FRNM_SERVICE_ID_GETPDUDATA,FRNM_E_UINIT, E_NOT_OK);
    /* @req FRNM051 */
    FRNM_VALIDATE_CHANNEL(NetworkHandle, FRNM_SERVICE_ID_GETPDUDATA, E_NOT_OK);
    FRNM_DET_REPORTERROR((NULL != nmPduData),FRNM_SERVICE_ID_GETPDUDATA,FRNM_E_INVALID_POINTER,E_NOT_OK);

    channelIndex = FrNm_ConfigPtr->FrNmChannelLookups[NetworkHandle];
    ChannelConf = &FrNm_ConfigPtr->FrNmChannels[channelIndex];
    ChannelInternal = &FrNm_Internal.FrNmChannels[channelIndex];
    /* @req FRNM265 */
    memcpy(nmPduData, ChannelInternal->FrNmRxDataPdu, ChannelConf->FrNmChannelIdentifiersConfig->FrNmRxPduLength);

    return E_OK;
}
#endif

/**
 *
 * @param NetworkHandle
 * @return
 */
/*This function causes a Repeat Message Request to be transmitted next on the bus*/
/* @req FRNM243 */
#if (FRNM_NODE_DETECTION_ENABLED == STD_ON) /* @req FRNM228 */
Std_ReturnType FrNm_RepeatMessageRequest( const NetworkHandleType NetworkHandle ){
    uint8 channelIndex;
    FrNm_Internal_ChannelType* ChannelInternal;
      /* @req FRNM050 */ /* @req FRNM021 */
    FRNM_DET_REPORTERROR((FRNM_STATUS_INIT == FrNm_Internal.InitStatus),FRNM_SERVICE_ID_REPEATMSG_REQUEST,FRNM_E_UINIT, E_NOT_OK);
    /* @req FRNM051 */
    FRNM_VALIDATE_CHANNEL(NetworkHandle, FRNM_SERVICE_ID_REPEATMSG_REQUEST, E_NOT_OK);
    channelIndex = FrNm_ConfigPtr->FrNmChannelLookups[NetworkHandle];
       ChannelInternal = &FrNm_Internal.FrNmChannels[channelIndex];
    /* @req FRNM172*/  /* @req FRNM111*/
    if (ChannelInternal->Mode == NM_MODE_NETWORK){

        if ((ChannelInternal->State == NM_STATE_READY_SLEEP)||(ChannelInternal->State == NM_STATE_NORMAL_OPERATION)){
#if (FRNM_CONTROL_BIT_VECTOR_ENABLED == STD_OFF)/* @req FRNM324 */
            ChannelInternal->FrNmTxDataPdu[CBV_INDEX] = 0x00; /* @req FRNM3829 */
#else
#if(FRNM_REPEAT_MESSAGE_BIT_ENABLED == STD_ON)
            ChannelInternal->FrNmTxDataPdu[CBV_INDEX] |= FRNM_CBV_REPEAT_MESSAGE_REQUEST;
#endif
#endif
        }
        /* @req FRNM226*/
        ChannelInternal->FrNm_RepeatMessage = TRUE;
    }

    return E_OK;
}
#endif

/**
 *
 * @param NetworkHandle
 * @param nmNodeIdPtr
 * @return
 */
/*This function gets the node identifier from the last successfully received NM-message*/
/* @req FRNM244 */
#if(FRNM_SOURCE_NODE_INDENTIFIER_ENABLED == STD_ON)/* @req FRNM267 */
Std_ReturnType FrNm_GetNodeIdentifier( const NetworkHandleType NetworkHandle, uint8* const nmNodeIdPtr){
    uint8 channelIndex;
    const FrNm_Internal_ChannelType* ChannelInternal;
    /* @req FRNM050 */ /* @req FRNM021 */
    FRNM_DET_REPORTERROR((FRNM_STATUS_INIT == FrNm_Internal.InitStatus),FRNM_SERVICE_ID_GETNODE_ID,FRNM_E_UINIT, E_NOT_OK);
    /* @req FRNM051 */
    FRNM_VALIDATE_CHANNEL(NetworkHandle, FRNM_SERVICE_ID_GETNODE_ID, E_NOT_OK);
    FRNM_DET_REPORTERROR((NULL != nmNodeIdPtr),FRNM_SERVICE_ID_GETNODE_ID,FRNM_E_INVALID_POINTER,E_NOT_OK);
    channelIndex = FrNm_ConfigPtr->FrNmChannelLookups[NetworkHandle];
    ChannelInternal = &FrNm_Internal.FrNmChannels[channelIndex];
    /* @req FRNM047 */
    *nmNodeIdPtr = ChannelInternal->FrNmRxDataPdu[FRNM_SOURCE_NODE_ID_POSITION];

    return E_OK;
}
#endif

/**
 *
 * @param NetworkHandle
 * @param nmNodeIdPtr
 * @return
 */
/*This function gets the node identifier configured for the local node.*/
/* @req FRNM245 */
#if(FRNM_SOURCE_NODE_INDENTIFIER_ENABLED == STD_ON)/* @req FRNM268 */
Std_ReturnType FrNm_GetLocalNodeIdentifier( const NetworkHandleType NetworkHandle, uint8* const nmNodeIdPtr ){
    uint8 channelIndex;
    const FrNm_ChannelInfoType* ChannelConf;
    /* @req FRNM050 */ /* @req FRNM021 */
    FRNM_DET_REPORTERROR((FRNM_STATUS_INIT == FrNm_Internal.InitStatus),FRNM_SERVICE_ID_GETLOCAL_NODEID,FRNM_E_UINIT, E_NOT_OK);
    /* @req FRNM051 */
    FRNM_VALIDATE_CHANNEL(NetworkHandle, FRNM_SERVICE_ID_GETLOCAL_NODEID, E_NOT_OK);
    FRNM_DET_REPORTERROR((NULL != nmNodeIdPtr),FRNM_SERVICE_ID_GETLOCAL_NODEID,FRNM_E_INVALID_POINTER,E_NOT_OK);
    channelIndex = FrNm_ConfigPtr->FrNmChannelLookups[NetworkHandle];
    ChannelConf = &FrNm_ConfigPtr->FrNmChannels[channelIndex];
    /* @req FRNM046 */
    *nmNodeIdPtr =  ChannelConf->FrNmChannelIdentifiersConfig->FrNmNodeId;

    return E_OK;
}
#endif

/**
 *
 * @param NetworkHandle
 * @return
 */
/*the service is provided only to be compatible to future extensions and to be compatible to the CAN-NM interface.*/
#if (FRNM_BUS_SYNCHRONIZATION_ENABLED == STD_ON) /* @req FRNM269 */
/* @req FRNM246 */
Std_ReturnType FrNm_RequestBusSynchronization( const NetworkHandleType NetworkHandle ){

    /* @req FRNM050 */ /* @req FRNM021 */
    FRNM_DET_REPORTERROR((FRNM_STATUS_INIT == FrNm_Internal.InitStatus),FRNM_SERVICE_ID_REQBUS_SYNC,FRNM_E_UINIT, E_NOT_OK);
    /* @req FRNM051 */
    FRNM_VALIDATE_CHANNEL(NetworkHandle, FRNM_SERVICE_ID_REQBUS_SYNC, E_NOT_OK);

    /* @req FRNM174 */
    return E_OK;
}
#endif

/**
 *
 * @param NetworkHandle
 * @param nmRemoteSleepIndPtr
 * @return
 */
/*This function checks if remote sleep indication has taken place or not.*/
/* req FRNM247 */
/*lint -e{818} Pointer parameter 'nmRemoteSleepIndPtr' (line 454) could be declared as pointing to const */
Std_ReturnType FrNm_CheckRemoteSleepIndication( const NetworkHandleType NetworkHandle, boolean* const nmRemoteSleepIndPtr ){

    /* @req FRNM050 */ /* @req FRNM021 */
    FRNM_DET_REPORTERROR((FRNM_STATUS_INIT == FrNm_Internal.InitStatus),FRNM_SERVICE_ID_CHKREMOTE_SLEEPIND,FRNM_E_UINIT, E_NOT_OK);
    /* @req FRNM051 */
    FRNM_VALIDATE_CHANNEL(NetworkHandle, FRNM_SERVICE_ID_CHKREMOTE_SLEEPIND, E_NOT_OK);
    FRNM_DET_REPORTERROR((NULL != nmRemoteSleepIndPtr),FRNM_SERVICE_ID_CHKREMOTE_SLEEPIND,FRNM_E_INVALID_POINTER,E_NOT_OK);

    return E_OK;
}

/**
 *
 * @param NetworkHandle
 * @param nmStatePtr
 * @param nmModePtr
 * @return
 */
/*This function returns the state and the mode of the network management*/
/* @req FRNM248 */
Std_ReturnType FrNm_GetState( const NetworkHandleType NetworkHandle, Nm_StateType* const nmStatePtr, Nm_ModeType* const nmModePtr ){
    uint8 channelIndex;
    const FrNm_Internal_ChannelType* ChannelInternal;
    /* @req FRNM050 */ /* @req FRNM021 */
    FRNM_DET_REPORTERROR((FRNM_STATUS_INIT == FrNm_Internal.InitStatus),FRNM_SERVICE_ID_GETSTATE,FRNM_E_UINIT, E_NOT_OK);
    /* @req FRNM051 */
    FRNM_VALIDATE_CHANNEL(NetworkHandle, FRNM_SERVICE_ID_GETSTATE, E_NOT_OK);
    FRNM_DET_REPORTERROR((NULL != nmStatePtr),FRNM_SERVICE_ID_GETSTATE,FRNM_E_INVALID_POINTER,E_NOT_OK);
    FRNM_DET_REPORTERROR((NULL != nmModePtr),FRNM_SERVICE_ID_GETSTATE,FRNM_E_INVALID_POINTER,E_NOT_OK);
    channelIndex = FrNm_ConfigPtr->FrNmChannelLookups[NetworkHandle];
    ChannelInternal = &FrNm_Internal.FrNmChannels[channelIndex];
    /* @req FRNM104 */
    *nmStatePtr = ChannelInternal->State;
    *nmModePtr = ChannelInternal->Mode;

    return E_OK;
}

/**
 *
 * @param NetworkHandle
 */
/* FrSM when synchronization of the FlexRay cluster could not be achieved*/
/* @req FRNM393 */
void FrNm_StartupError( const NetworkHandleType NetworkHandle )
{
    uint8 channelIndex;
    const FrNm_ChannelInfoType* FrNmChannelsconf;
    FrNm_Internal_ChannelType* ChannelInternal;
    /* @req FRNM050 */ /* @req FRNM021 */
    FRNM_DET_REPORTERROR((FRNM_STATUS_INIT == FrNm_Internal.InitStatus),FRNM_SERVICE_ID_STARTUPERROR,FRNM_E_UINIT);
    /* @req FRNM051 */
    FRNM_VALIDATE_CHANNEL(NetworkHandle, FRNM_SERVICE_ID_STARTUPERROR);

    channelIndex = FrNm_ConfigPtr->FrNmChannelLookups[NetworkHandle];
    FrNmChannelsconf = &FrNm_ConfigPtr->FrNmChannels[channelIndex];
    ChannelInternal = &FrNm_Internal.FrNmChannels[channelIndex];
    /* @req FRNM336 */ /* Go to bus sleep even when the FrNM Main function is no longer executing */
    if(ChannelInternal->Mode == NM_MODE_SYNCHRONIZE ) {
      if(ChannelInternal->FrNm_NetworkRequested == FALSE) {
          /* @req FRNM376 */
          ChannelInternal->State = NM_STATE_BUS_SLEEP;
          ChannelInternal->Mode = NM_MODE_BUS_SLEEP;
          FrNm_Internal_Resetvotedata(ChannelInternal);
          /* @req FRNM134 */
          #if ( FRNM_STATE_CHANGE_INDICATION_ENABLED == STD_ON) /* @req FRNM106 */
          Nm_StateChangeNotification( FrNmChannelsconf->FrNmChannelIdentifiersConfig->FrNmComMNetworkHandleRef,NM_STATE_SYNCHRONIZE,NM_STATE_BUS_SLEEP);
          #endif
          Nm_BusSleepMode( FrNmChannelsconf->FrNmChannelIdentifiersConfig->FrNmComMNetworkHandleRef); /* @req FRNM135 */
          ChannelInternal->FrNm_RepeatMessage = FALSE; /* @req FRNM320 */
      } else {
          /* @req FRNM340 */
          /*No Changes remains in SYNCHRONIZE state */
      }
    } else if(ChannelInternal->Mode == NM_MODE_NETWORK) {

        switch (ChannelInternal->State) {

            case NM_STATE_REPEAT_MESSAGE:
                /* @req FRNM383 */

#if (FRNM_CYCLE_COUNTER_EMULATION == STD_OFF)
                ChannelInternal->Mode = NM_MODE_SYNCHRONIZE;
                ChannelInternal->State = NM_STATE_SYNCHRONIZE;
                FrNm_Internal_Resetvotedata(ChannelInternal);
#if ( FRNM_STATE_CHANGE_INDICATION_ENABLED == STD_ON) /* @req FRNM106 */
                Nm_StateChangeNotification( FrNmChannelsconf->FrNmChannelIdentifiersConfig->FrNmComMNetworkHandleRef,NM_STATE_REPEAT_MESSAGE,NM_STATE_SYNCHRONIZE);
#endif
#else
                /* @req FRNM385 */
                ChannelInternal->Mode = NM_MODE_BUS_SLEEP;
                ChannelInternal->State = NM_STATE_BUS_SLEEP;
                FrNm_Internal_Resetvotedata(ChannelInternal);
                /* @req FRNM134 */
#if ( FRNM_STATE_CHANGE_INDICATION_ENABLED == STD_ON) /* @req FRNM106 */
                Nm_StateChangeNotification( FrNmChannelsconf->FrNmChannelIdentifiersConfig->FrNmComMNetworkHandleRef,NM_STATE_REPEAT_MESSAGE,NM_STATE_BUS_SLEEP);
#endif
                Nm_BusSleepMode( FrNmChannelsconf->FrNmChannelIdentifiersConfig->FrNmComMNetworkHandleRef); /* @req FRNM135 */
                ChannelInternal->FrNm_RepeatMessage = FALSE; /* @req FRNM320 */
#endif

            break;
#if (FRNM_PASSIVE_MODE_ENABLED == STD_OFF)
            case NM_STATE_NORMAL_OPERATION:
                /* @req FRNM342*/
                ChannelInternal->Mode = NM_MODE_SYNCHRONIZE;
                ChannelInternal->State = NM_STATE_SYNCHRONIZE;
                FrNm_Internal_Resetvotedata(ChannelInternal);
#if (FRNM_STATE_CHANGE_INDICATION_ENABLED == STD_ON) /* @req FRNM106 */
                Nm_StateChangeNotification(FrNmChannelsconf->FrNmChannelIdentifiersConfig->FrNmComMNetworkHandleRef,NM_STATE_NORMAL_OPERATION, NM_STATE_SYNCHRONIZE);
#endif

            break;
#endif
            case NM_STATE_READY_SLEEP:
                /* @req FRNM338 */
#if (FRNM_CYCLE_COUNTER_EMULATION == STD_OFF)
                ChannelInternal->Mode = NM_MODE_SYNCHRONIZE;
                ChannelInternal->State = NM_STATE_SYNCHRONIZE;
                FrNm_Internal_Resetvotedata(ChannelInternal);
#if (FRNM_STATE_CHANGE_INDICATION_ENABLED == STD_ON)/* @req FRNM106 */
                Nm_StateChangeNotification(FrNmChannelsconf->FrNmChannelIdentifiersConfig->FrNmComMNetworkHandleRef,NM_STATE_READY_SLEEP, NM_STATE_SYNCHRONIZE);
#endif
#else
                ChannelInternal->Mode = NM_MODE_BUS_SLEEP;
                ChannelInternal->State = NM_STATE_BUS_SLEEP;
                FrNm_Internal_Resetvotedata(ChannelInternal);
                /* @req FRNM134 */
#if ( FRNM_STATE_CHANGE_INDICATION_ENABLED == STD_ON) /* @req FRNM106 */
                Nm_StateChangeNotification( FrNmChannelsconf->FrNmChannelIdentifiersConfig->FrNmComMNetworkHandleRef,NM_STATE_READY_SLEEP,NM_STATE_BUS_SLEEP);
#endif
                Nm_BusSleepMode( FrNmChannelsconf->FrNmChannelIdentifiersConfig->FrNmComMNetworkHandleRef); /* @req FRNM135 */
                ChannelInternal->FrNm_RepeatMessage = FALSE; /* @req FRNM320 */
#endif
            break;

            default:
                /*Nothing to do */
            break;

            };
        } else {
                /*Nothing to do*/
              }
}

/**
 *
 * @param FrNmTxPduId
 * @param PduInfoPtr
 * @return
 */
/*This function is used by the PduR to trigger a spontaneous transmission of an NM message with the provided NM User Data
 * This function is only a dummy to avoid linker errors of the PduR */
/* @req FRNM359 */
Std_ReturnType FrNm_Transmit( PduIdType FrNmTxPduId, const PduInfoType* PduInfoPtr ){

    (void)FrNmTxPduId;
    (void)PduInfoPtr; /*lint !e920 pointer not used */
    /* @req FRNM366 */
    return E_OK;
}

/**
 *
 * @param nmChannelHandle
 * @return
 */
/*Enable the NM PDU transmission ability due to a ISO14229 Communication Control (28hex) service*/
/*  @req FRNM387*/
Std_ReturnType FrNm_EnableCommunication( const NetworkHandleType nmChannelHandle ) {

    Std_ReturnType status;
    /* @req FRNM050 *//* @req FRNM021 */
    FRNM_DET_REPORTERROR((FRNM_STATUS_INIT == FrNm_Internal.InitStatus),FRNM_SERVICE_ID_ENABLECOMM,FRNM_E_UINIT, E_NOT_OK);
#if (FRNM_PASSIVE_MODE_ENABLED == STD_ON) /** @req FRNM389 */
    (void)nmChannelHandle;
    status = E_NOT_OK;
#else
    status = E_OK;
    uint8 channelIndex;
    FrNm_Internal_ChannelType* ChannelInternal;
    /* @req FRNM051 */
    FRNM_VALIDATE_CHANNEL(nmChannelHandle, FRNM_SERVICE_ID_ENABLECOMM,E_NOT_OK);
    channelIndex = FrNm_ConfigPtr->FrNmChannelLookups[nmChannelHandle];
    ChannelInternal = &FrNm_Internal.FrNmChannels[channelIndex];

    if(ChannelInternal->Mode != NM_MODE_NETWORK ) { /* @req FRNM388 */
        status = E_NOT_OK ;
    } else {
        ChannelInternal->CommunicationEnabled = TRUE;
        status = E_OK;
    }
#endif
    return status;
}

/**
 *
 * @param nmChannelHandle
 * @return
 */
/*Disable the NM PDU transmission ability due to a ISO14229 Communication Control (28hex) service*/
/*  @req FRNM390*/
Std_ReturnType FrNm_DisableCommunication( const NetworkHandleType nmChannelHandle ) {

    /* @req FRNM050 */ /* @req FRNM021 */
    Std_ReturnType status;
    FRNM_DET_REPORTERROR((FRNM_STATUS_INIT == FrNm_Internal.InitStatus),FRNM_SERVICE_ID_DISABLECOMM,FRNM_E_UINIT, E_NOT_OK);
#if (FRNM_PASSIVE_MODE_ENABLED == STD_ON) /** @req FRNM392 */
    (void)nmChannelHandle;
    status = E_NOT_OK;
#else
    status = E_OK;
    uint8 channelIndex;
    FrNm_Internal_ChannelType* ChannelInternal;
    /* @req FRNM051 */
    FRNM_VALIDATE_CHANNEL(nmChannelHandle, FRNM_SERVICE_ID_DISABLECOMM,E_NOT_OK);
    channelIndex = FrNm_ConfigPtr->FrNmChannelLookups[nmChannelHandle];
    ChannelInternal = &FrNm_Internal.FrNmChannels[channelIndex];

    if(ChannelInternal->Mode != NM_MODE_NETWORK ) { /* @req FRNM391 */
        status = E_NOT_OK;
    } else {
        ChannelInternal->CommunicationEnabled = FALSE;
        status = E_OK;
    }
#endif
    return status;
}

/**
 *
 * @param nmChannelHandle
 * @param nmSleepReadyBit
 * @return
 */
/*
 * Set the NM Coordinator Sleep Ready bit in the Control Bit Vector
 * CURRENTLY UNSUPPORTED
 */
Std_ReturnType FrNm_SetSleepReadyBit( const NetworkHandleType nmChannelHandle, const boolean nmSleepReadyBit ){

    /* @req FRNM050 */ /* @req FRNM021 */
    FRNM_DET_REPORTERROR((FRNM_STATUS_INIT == FrNm_Internal.InitStatus),FRNM_SERVICE_ID_SETSLEEP_READYBIT,FRNM_E_UINIT, E_NOT_OK);
    /* not supported */
    (void)nmChannelHandle;
    (void)nmSleepReadyBit;

    return E_OK;
}



/** Functions called by Flexray Interface call back functions  **/
/**
 *
 * @param RxPduId
 * @param PduInfoPtr
 */
/*Indication of a received I-PDU from a lower layer communication module*/
/*  @req FRNM251*/
/*lint -e{818} Pointer parameter 'PduInfoPtr' (line 717) could be declared as pointing to const */
void FrNm_RxIndication( PduIdType RxPduId, PduInfoType* PduInfoPtr ){
    /*  @req FRNM176*/ /*  @req FRNM276*/
    const FrNm_ChannelInfoType* FrNmChannelsconf;
    FrNm_Internal_ChannelType* ChannelInternal;
    uint8 pduIndex;
    uint8 channelIndex;
    uint8 cbv;
    boolean repeatMessageBitIndication;

    /* @req FRNM050 */ /* @req FRNM021 */ /* req FRNM361 */
    FRNM_DET_REPORTERROR((FRNM_STATUS_INIT == FrNm_Internal.InitStatus),FRNM_SERVICE_ID_RX_INDICATION,FRNM_E_UINIT);
    FRNM_DET_REPORTERROR((RxPduId < FRNM_RX_PDU_COUNT),FRNM_SERVICE_ID_RX_INDICATION,FRNM_E_PDU_ID_INVALID);
    FRNM_DET_REPORTERROR((NULL != PduInfoPtr),FRNM_SERVICE_ID_RX_INDICATION,FRNM_E_INVALID_POINTER);
    FRNM_DET_REPORTERROR((NULL != PduInfoPtr->SduDataPtr),FRNM_SERVICE_ID_RX_INDICATION,FRNM_E_INVALID_POINTER);

    channelIndex = FrNm_ConfigPtr->FrNmChnlRxpduMaps[RxPduId];
    FrNmChannelsconf = &FrNm_ConfigPtr->FrNmChannels[channelIndex];
    ChannelInternal = &FrNm_Internal.FrNmChannels[channelIndex];

    /* @req FRNM013 */
    memcpy(ChannelInternal->FrNmRxDataPdu, PduInfoPtr->SduDataPtr, PduInfoPtr->SduLength);

    cbv = ChannelInternal->FrNmRxDataPdu[CBV_INDEX];
    repeatMessageBitIndication = cbv & FRNM_CBV_REPEAT_MESSAGE_REQUEST;

#if ((FRNM_PNC_EIRA_CALC_ENABLED == STD_ON) && (FRNM_PNC_COUNT > 0))
    /* @req FRNM405 */ /* @req FRNM404 */ /* @req FRNM406 */ /* @req FRNM407 */ /* @req FRNM413 */
    if (((cbv & FRNM_CBV_PNI) != 0) && (FrNmChannelsconf->FrNmChannelIdentifiersConfig->FrNmPnEnabled == STD_ON)) {
        //Proces PN only if PNI information is available and PN is enabled
        FrNm_Internal_RxProcess(ChannelInternal);
    }
#endif

    if (ChannelInternal->Mode == NM_MODE_BUS_SLEEP) {
       /* Notify 'Network Start' */
        /* @req FRNM175 */
       Nm_NetworkStartIndication(FrNmChannelsconf->FrNmChannelIdentifiersConfig->FrNmComMNetworkHandleRef);
    }else if(ChannelInternal->Mode == NM_MODE_NETWORK){
        if(ChannelInternal->State == NM_STATE_NORMAL_OPERATION){
#if (FRNM_PASSIVE_MODE_ENABLED == STD_OFF)
            if(TRUE == repeatMessageBitIndication){
                FrNm_Internal_NormalOperation_to_RepeatMessage(FrNmChannelsconf, ChannelInternal);
             }
#endif
        }else if(ChannelInternal->State == NM_STATE_READY_SLEEP) {
            for(pduIndex=0;pduIndex<FrNmChannelsconf->FrNmChannelIdentifiersConfig->FrNmRxPduCount;pduIndex++) {
                if(RxPduId == FrNmChannelsconf->FrNmChannelIdentifiersConfig->FrNmRxPduList[pduIndex].FrNmRxPduId) {
                    if(FrNmChannelsconf->FrNmChannelIdentifiersConfig->FrNmRxPduList[pduIndex].FrNmRxConatainsVote == STD_ON) {
                      if((ChannelInternal->FrNmRxDataPdu[VOTE_INDEX] & VOTE_VALUE) == VOTE_VALUE) {
                              /* @req FRNM128 */ /* @req FRNM314 */
                              ChannelInternal->readySleepCntLeft = FrNmChannelsconf->FrNmTimingConfig->FrNmReadySleepCnt;
            #if (FRNM_CYCLE_COUNTER_EMULATION ==STD_ON) /* @req FRNM378 */
                              ChannelInternal->syncLossTimerLeft = FrNmChannelsconf->FrNmTimingConfig->FrNmSyncLossTimer;
            #endif

                      }
                    }
                }
            }
#if (FRNM_PASSIVE_MODE_ENABLED == STD_OFF)
            if(TRUE == repeatMessageBitIndication){
                FrNm_Internal_ReadySleep_to_RepeatMessage(FrNmChannelsconf, ChannelInternal);
            }
#else
            (void)repeatMessageBitIndication;
#endif

        }else{
            /*Nothing to do */
        }
    }else{
        /*Nothing to do */
    }

}

/**
 *
 * @param TxPduId
 * @param PduInfoPtr
 * @return
 */
/*The lower layer communication module requests the buffer of the SDU for transmission from the upper layer module*/
/*  @req FRNM252*/
Std_ReturnType FrNm_TriggerTransmit( PduIdType TxPduId, PduInfoType* PduInfoPtr ){

#if (FRNM_PASSIVE_MODE_ENABLED == STD_ON)
    (void)TxPduId;
    (void)PduInfoPtr; /*lint !e920 Pointer unused */
    return E_NOT_OK;
#else
    uint8 pduIndex;
    Std_ReturnType Status;

    uint8 channelIndex;
    const FrNm_ChannelInfoType* FrNmChannelsconf;
    FrNm_Internal_ChannelType* ChannelInternal;
#if( FRNM_USER_DATA_ENABLED == STD_ON)
    PduInfoType userData;
#endif
    /* @req FRNM050 */ /* @req FRNM021 */
    FRNM_DET_REPORTERROR((FRNM_STATUS_INIT == FrNm_Internal.InitStatus),FRNM_SERVICE_ID_TRIGGER_TRANSMIT,FRNM_E_UINIT,E_NOT_OK);
    FRNM_DET_REPORTERROR((TxPduId < FRNM_TX_PDU_COUNT),FRNM_SERVICE_ID_TRIGGER_TRANSMIT,FRNM_E_PDU_ID_INVALID,E_NOT_OK);
    FRNM_DET_REPORTERROR((NULL != PduInfoPtr),FRNM_SERVICE_ID_TRIGGER_TRANSMIT,FRNM_E_INVALID_POINTER,E_NOT_OK);
    FRNM_DET_REPORTERROR((NULL != PduInfoPtr->SduDataPtr),FRNM_SERVICE_ID_TRIGGER_TRANSMIT,FRNM_E_INVALID_POINTER,E_NOT_OK);

    if(FrNm_ConfigPtr->FrNmChnlTxpduMaps != NULL_PTR){
        Status = E_OK;
        /* @req FRNM277 */
        channelIndex = FrNm_ConfigPtr->FrNmChnlTxpduMaps[TxPduId];
        FrNmChannelsconf = &FrNm_ConfigPtr->FrNmChannels[channelIndex];
        ChannelInternal = &FrNm_Internal.FrNmChannels[channelIndex];

        for(pduIndex=0;pduIndex<FrNmChannelsconf->FrNmChannelIdentifiersConfig->FrNmTxPduCount;pduIndex++) {
            if(FrNmChannelsconf->FrNmChannelIdentifiersConfig->FrNmTxPduList[pduIndex].FrNmTxConfPduId == TxPduId) {
               if(((FrNmChannelsconf->FrNmChannelIdentifiersConfig->FrNmTxPduList[pduIndex].FrNmTxContainsData == STD_ON)
                 &&(FrNmChannelsconf->FrNmChannelIdentifiersConfig->FrNmTxPduList[pduIndex].FrNmTxConatainsVote == STD_ON))
                  ||((FrNmChannelsconf->FrNmChannelIdentifiersConfig->FrNmTxPduList[pduIndex].FrNmTxContainsData == STD_ON)
                  &&(FrNmChannelsconf->FrNmChannelIdentifiersConfig->FrNmTxPduList[pduIndex].FrNmTxConatainsVote == STD_OFF))){

#if( FRNM_USER_DATA_ENABLED == STD_ON) /* @req FRNM362*/
                   if(NULL!=FrNmChannelsconf->FrNmChannelIdentifiersConfig->FrNmUserDataConfig){
                        userData.SduDataPtr = &ChannelInternal->FrNmTxDataPdu[FRNM_INTERNAL_GET_USER_DATA_OFFSET];
                        userData.SduLength = (PduLengthType)FrNmChannelsconf->FrNmChannelIdentifiersConfig->FrNmTxPduLength - FRNM_INTERNAL_GET_USER_DATA_OFFSET;
                        /* @req FRNM394 */ /* @req FRNM364 */
                        Status = PduR_FrNmTriggerTransmit(FrNmChannelsconf->FrNmChannelIdentifiersConfig->FrNmUserDataConfig->FrNmUserDataTxPduId,&userData);
                        if (Status == E_OK)
                        {
                        /* @req FRNM280 */
                          memcpy(PduInfoPtr->SduDataPtr, ChannelInternal->FrNmTxDataPdu, FrNmChannelsconf->FrNmChannelIdentifiersConfig->FrNmTxPduList[pduIndex].FrNmTxPduLength);
                          PduInfoPtr->SduLength = FrNmChannelsconf->FrNmChannelIdentifiersConfig->FrNmTxPduList[pduIndex].FrNmTxPduLength;
                        }
                   }
#else
                    memcpy(PduInfoPtr->SduDataPtr, ChannelInternal->FrNmTxDataPdu, FrNmChannelsconf->FrNmChannelIdentifiersConfig->FrNmTxPduList[pduIndex].FrNmTxPduLength);
                    PduInfoPtr->SduLength = FrNmChannelsconf->FrNmChannelIdentifiersConfig->FrNmTxPduList[pduIndex].FrNmTxPduLength;
#endif

                 }else if((FrNmChannelsconf->FrNmChannelIdentifiersConfig->FrNmTxPduList[pduIndex].FrNmTxContainsData == STD_OFF)
                  &&(FrNmChannelsconf->FrNmChannelIdentifiersConfig->FrNmTxPduList[pduIndex].FrNmTxConatainsVote == STD_ON)){
                     memcpy(PduInfoPtr->SduDataPtr, &ChannelInternal->FrNmTxVotePdu, FrNmChannelsconf->FrNmChannelIdentifiersConfig->FrNmTxPduList[pduIndex].FrNmTxPduLength);
                     PduInfoPtr->SduLength = FrNmChannelsconf->FrNmChannelIdentifiersConfig->FrNmTxPduList[pduIndex].FrNmTxPduLength;
                 }else{
                     /*Nothing to do */
                 }

            }

         }
    }else{
        Status = E_NOT_OK;
    }

    return Status;
#endif
}

/**
 *
 * @param TxPduId
 */
/*The lower layer communication module confirms the transmission of an I-PDU*/
/*  req FRNM460 in 4.2 spec*/
void FrNm_TxConfirmation( PduIdType TxPduId ){

#if (FRNM_PASSIVE_MODE_ENABLED == STD_ON)
    (void)TxPduId;
    return;
#else
    uint8 channelIndex;
    FrNm_Internal_ChannelType* ChannelInternal;
    const FrNm_ChannelInfoType* FrNmChannelsconf;
    /* @req FRNM050 */ /* @req FRNM021 */
    FRNM_DET_REPORTERROR((FRNM_STATUS_INIT == FrNm_Internal.InitStatus),FRNM_SERVICE_ID_TX_CONFIRMATION,FRNM_E_UINIT);
    FRNM_DET_REPORTERROR((TxPduId < FRNM_TX_PDU_COUNT),FRNM_SERVICE_ID_TX_CONFIRMATION,FRNM_E_PDU_ID_INVALID);

    if(FrNm_ConfigPtr->FrNmChnlTxpduMaps != NULL_PTR){
        channelIndex = FrNm_ConfigPtr->FrNmChnlTxpduMaps[TxPduId];
        ChannelInternal = &FrNm_Internal.FrNmChannels[channelIndex];
        FrNmChannelsconf = &FrNm_ConfigPtr->FrNmChannels[channelIndex];

#if( FRNM_USER_DATA_ENABLED == STD_ON)
        if(NULL!=FrNmChannelsconf->FrNmChannelIdentifiersConfig->FrNmUserDataConfig){
            /* @req FRNM363 */ /* @req FRNM365 */
            PduR_FrNmTxConfirmation(FrNmChannelsconf->FrNmChannelIdentifiersConfig->FrNmUserDataConfig->FrNmUserDataTxPduId);
        }
#endif

        ChannelInternal->MessageTimeoutTimeLeft = FrNmChannelsconf->FrNmTimingConfig->FrNmMsgTimeoutTime;
    }
#endif
}

/* Scheduled main function */
/* ----------------------- */

/* Prototype right here because this function should not be exposed */
void FrNm_MainFunction(NetworkHandleType Channel);

/**
 *
 * @param Channel
 */
/*Main function of FlexRay NM*/
/* @req FRNM283 */
void FrNm_MainFunction(NetworkHandleType Channel){
    /* @req FRNM356 */  /* @req FRNM168*/
    uint8 channelIndex;
    const FrNm_ChannelInfoType* FrNmChannelsconf;
    FrNm_Internal_ChannelType* ChannelInternal;
    uint8 FrIfCurrentCycle;
    uint16 FrIfCurrentMacroTick;
    uint8 FrIfCtrlID;
    FrIfCurrentCycle =0;
    FrIfCurrentMacroTick =0;

    /* @req FRNM050 */ /* @req FRNM021 */  /* @req FRNM344*/
    FRNM_DET_REPORTERROR((FRNM_STATUS_INIT == FrNm_Internal.InitStatus),(FRNM_SERVICE_ID_MAIN_FUNCTION+Channel),FRNM_E_UINIT);
    /* @req FRNM051 */
    FRNM_VALIDATE_CHANNEL(Channel, (FRNM_SERVICE_ID_MAIN_FUNCTION+Channel));
    channelIndex = FrNm_ConfigPtr->FrNmChannelLookups[Channel];
    FrNmChannelsconf = &FrNm_ConfigPtr->FrNmChannels[channelIndex];
    ChannelInternal = &FrNm_Internal.FrNmChannels[channelIndex];
    FrIfCtrlID = FrNm_ConfigPtr->FrNmChnlFrIfCtrlIdMap[channelIndex];
    if(ChannelInternal->Mode != NM_MODE_BUS_SLEEP){
    /* @req FRNM375 */
     if (E_NOT_OK == FrIf_GetGlobalTime(FrIfCtrlID,&FrIfCurrentCycle,&FrIfCurrentMacroTick) )  {
         FrNm_Internal_Globaltime_Error_Handling(FrNmChannelsconf, ChannelInternal);
     }else{


            if(ChannelInternal->Mode != NM_MODE_BUS_SLEEP){
                FrNm_internal_CheckRepetitionCycles(FrNmChannelsconf, ChannelInternal, FrIfCurrentCycle);
            }

#if (FRNM_HW_VOTE_ENABLE == STD_ON) /* @req FRNM169 */
            uint8 frnmVectorDataPdu[8]= {0};
            if(E_OK == FrIf_GetNmVector(FrIfCtrlID,frnmVectorDataPdu)) {
                /* @req FRNM335 */
                    FrNm_Internal_Hardware_VectorData_Handling(frnmVectorDataPdu,FrNmChannelsconf, ChannelInternal);
                }
#endif

            if (ChannelInternal->Mode == NM_MODE_SYNCHRONIZE) {
                if(ChannelInternal->repetitionCycleCompleted == TRUE) {

                    FrNm_Internal_Synchronize_to_RepeatMessage(FrNmChannelsconf, ChannelInternal);
                }
            }
            if (ChannelInternal->Mode == NM_MODE_NETWORK) {

              FrNm_Internal_TickRepeatMessageTime(FrNmChannelsconf, ChannelInternal );
#if (FRNM_PASSIVE_MODE_ENABLED == STD_OFF) /* Active mode handling */
              FrNm_Internal_TickTxTimeout(FrNmChannelsconf, ChannelInternal);

                if(ChannelInternal->repetitionCycleCompleted == TRUE)  {  /* @req FRNM115*/

                    if (ChannelInternal->State == NM_STATE_NORMAL_OPERATION){
                        if(ChannelInternal->FrNm_RepeatMessage == TRUE){
                           /* @req FRNM124*/
                           FrNm_Internal_NormalOperation_to_RepeatMessage(FrNmChannelsconf, ChannelInternal);
                        }else{
                           if(ChannelInternal->FrNm_NetworkRequested == FALSE)
                           {
                               /* @req FRNM125*/
                               FrNm_Internal_NormalOperation_to_ReadySleep(FrNmChannelsconf, ChannelInternal);
                           }
                        }
                    }else if (ChannelInternal->State == NM_STATE_READY_SLEEP){
                        FrNm_Internal_ReadySleepState_Handling(FrNmChannelsconf, ChannelInternal);
                    }else{
                         /* do nothing */
                    }
                }
                if ((ChannelInternal->State == NM_STATE_REPEAT_MESSAGE)||(ChannelInternal->State == NM_STATE_NORMAL_OPERATION)){
                      /* @req FRNM116*/ /* @req FRNM123*/
                    if (E_NOT_OK == FrNm_Internal_TransmitMessage(FrNmChannelsconf, ChannelInternal)){
                        /* do nothing added  for lint warning */
                    }
                }
#else /* Passive mode */
                if((ChannelInternal->repetitionCycleCompleted == TRUE) &&
                        (ChannelInternal->State == NM_STATE_READY_SLEEP)){  /* @req FRNM115*/
                    FrNm_Internal_ReadySleepState_Handling(FrNmChannelsconf, ChannelInternal);

                }
#endif
            }
       }
#if ((FRNM_PNC_EIRA_CALC_ENABLED == STD_ON) && (FRNM_PNC_COUNT > 0))
     if (channelIndex == 0) { //Timer monitoring is done for the first channel
         FrNm_Internal_TickPnEIRAResetTime(FrNmChannelsconf->FrNmTimingConfig->FrNmMainFunctionPeriod);
     }
#endif
    }
}

