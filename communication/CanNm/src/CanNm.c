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
/** @req CANNM199 */ /* parameter checking */
/** @req CANNM196 */ /* equal to CANNM241 */
/** @req CANNM241 */ /* if det enabled => parameter checking */
/** @req CANNM292 */ /* check parameter pointers == null (all except init) */
/** @req CANNM210 */ /* mainfunction => instande id = 0 when deterror */
/** @req CANNM325 */ /* only describes optional API-functions */
/** @req CANNM206 */
/** @req CANNM192 */ /* => CANNM_VALIDATE_CHANNEL */
/** @req CANNM191 */ /* => CANNM_VALIDATE_INIT */
/** @req CANNM093 */
/** @req CANNM089 */
/** @req CANNM039 */ /* when not init every functions reports deterror => CANNM_VALIDATE_INIT at startup from every function*/
/** @req CANNM088 */ /* 7.1 Coordination algorithm */
/** @req CANNM089 */ /* 7.1 Coordination algorithm */
/** @req CANNM093 */ /* 7.2 Operation modes */
/** @req CANNM146 */ /* cpu independent */
/** @req CANNM158 */ /* supporting user data can be statically turned on or off (config dependent) */
/** @req CANNM161 */ /* passive mode can be statically turned on or off (config dependent) */
/** @req CANNM162 */ /* passive mode is statically configured for all instances */
/** @req CANNM189 */ /* no deterrors returned by NM-API */
/** @req CANNM197 toolchain */ /* type checking at compile time */
/** @req CANNM198 toolchain */ /* value checking at config time */


/* CANNM081 => 299, 300, 301 */
/** !req CANNM299 */ /* CanNm_Cfg.c does not contain any parameters*/
/** !req CANNM300 */ /* CanNm_Lcfg.c does not exist */
/** @req CANNM301 */ /* CanNm_PBcfg.c shall contain post build time configurable parameters */

/* CANNM044 => 302, 303, 304 */
/** @req CANNM302 */ /* CanNm.h shall contain the declaration of provided interface functions */
/** !req CANNM303 */ /* CanNm_Cbk.h shall contain the declaration of provided call-back functions */
/** @req CANNM304 */ /* CanNm_Cfg.h shall contain pre-compile time configurable parameters */

/* CANNM001 => 237, 238 */
/** @req CANNM237 */ /* The CanNm module shall provide the periodic transmission mode. In this transmission mode the CanNm module shall send Network Management PDUs periodically */
/** !req CANNM238 */ /* The CanNm module may provide the periodic transmission mode with bus load reduction. In this transmission mode the CanNm module shall transmit Network Management PDUs due to a specific algorithm */

/* CANNM016 => 243, 244 */
/** @req CANNM243 */ /* parameter value check only in devmode */
/** @req CANNM244 */ /* parameter invalidation */
/** @req CANNM416 *//* 1/0 in a each bit of PN info range represents whether PN is requested or not */
/** @req CANNM417 */ /* 1/0 in each bit of Filter mask byte configuration indicates whether required by ECU or not */
/** @req CANNM428 */ /* If PN supported EIRA reset timer for every PN request bit */
/** @req CANNM438 */ /* If PN supported ERA reset timer for every PN request bit for every channel */
#include "ComStack_Types.h" 	/** @req CANNM305 */
#include "CanNm.h"				/** @req CANNM306 */
#include "CanNm_Cfg.h"
#include "CanNm_Internal.h"
#include "Nm_Cbk.h"				/** @req CANNM307 */
#include "NmStack_Types.h"		/** @req CANNM309 */


#include "SchM_CanNm.h"			/** @req CANNM310 */


#include "MemMap.h"				/** @req CANNM311 */

#include "CanIf.h"				/** @req CANNM312 */
#include "Nm.h"					/** @req CANNM313 */ /* according to the spec it should be Nm_Cfg.h */

#if (CANNM_USER_DATA_ENABLED == STD_ON) && (CANNM_COM_USER_DATA_SUPPORT == STD_ON)	/** @req CANNM326 */
#include "PduR_CanNm.h"
#endif

#include <string.h>

#if (CANNM_DEV_ERROR_DETECT == STD_ON) /** @req CANNM188 */
#include "Det.h"				/** @req CANNM308 */
#endif
#if defined(USE_DEM)
#include "Dem.h"
#endif


static const CanNm_ConfigType* CanNm_ConfigPtr;

//lint -save -e785 //PC-Lint exception: Too few initializers for aggregate...
CanNm_InternalType CanNm_Internal = {
        .InitStatus = CANNM_STATUS_UNINIT,
};
//lint -restore


#ifdef HOST_TEST
void GetChannelRunTimeData(uint32 channelId, uint32* messageCycleTimeLeft, uint32* timeoutTimeLeft) {

    const CanNm_Internal_ChannelType* ChannelInternal = &CanNm_Internal.Channels[channelId];

    *messageCycleTimeLeft =  ChannelInternal->MessageCycleTimeLeft;
    *timeoutTimeLeft = ChannelInternal->TimeoutTimeLeft;
}
#if (CANNM_PNC_COUNT > 0)
Std_ReturnType VerifyAllEraBitsZero(void) {
    Std_ReturnType ret;
    ret = E_OK;
    for (uint8 chanIndex = 0; ((chanIndex < CANNM_CHANNEL_COUNT) && (ret == E_OK)); chanIndex++) {
        for (uint8 eraByteindex = 0; eraByteindex < CanNm_ConfigPtr->CanNmPnInfo->CanNmPnInfoLen; eraByteindex++) {
            if (CanNm_Internal.Channels[chanIndex].pnERA.bytes[eraByteindex] != 0) {
                ret = E_NOT_OK;
                break;
            }
        }
    }
    return ret;
}

boolean IsPNbitSet(uint8 chanIndex, uint8 eraByteindex, uint8 PNbitIndex) {
    boolean ret;
    ret = FALSE;
    if ((CanNm_Internal.Channels[chanIndex].pnERA.bytes[eraByteindex] & (1 << PNbitIndex)) == (1 << PNbitIndex)) {
        ret = TRUE;
    }

    return ret;
}

uint32 GetResetTimer(uint8 chanIdx, uint8 pnIdx) {
    uint8 timerIdx = CanNm_ConfigPtr->CanNmPnInfo->CanNmPnIndexToTimerMap[pnIdx];
    return CanNm_Internal.Channels[chanIdx].pnERATimers[timerIdx].resetTimer;
}
#endif /* #if (CANNM_PNC_COUNT > 0) */
#endif

/** Initialize the complete CanNm module, i.e. all channels which are activated */
/** must be called directly after canif in order to fulfill CANNM253 */ /** @req CANNM253 */
void CanNm_Init( const CanNm_ConfigType * const cannmConfigPtr ){
    CANNM_VALIDATE_NOTNULL_INIT(cannmConfigPtr, CANNM_SERVICEID_INIT, 0);  //shall not be done for init

    CanNm_ConfigPtr = cannmConfigPtr;  /**< @req CANNM060 */

    uint8 channel;

    for (channel = 0; channel < CANNM_CHANNEL_COUNT; channel++) {
        const CanNm_ChannelType* ChannelConf = &CanNm_ConfigPtr->Channels[channel];
        CanNm_Internal_ChannelType* ChannelInternal = &CanNm_Internal.Channels[channel];

        ChannelInternal->Mode = NM_MODE_BUS_SLEEP;    /** @req CANNM144 */
        ChannelInternal->State = NM_STATE_BUS_SLEEP;  /** @req CANNM141 */
        ChannelInternal->Requested = FALSE;           /** @req CANNM143 */ /* reqeuested should be a state and not a flag */
        ChannelInternal->CommunicationEnabled = TRUE;
        ChannelInternal->immediateModeActive = FALSE;
        ChannelInternal->MessageFilteringEnabled = FALSE;/** @req CANNM403 */

#if  (CANNM_USER_DATA_ENABLED == STD_ON)
#if (CANNM_COM_USER_DATA_SUPPORT == STD_ON)
        ChannelInternal->TransmissionStatus = CANNM_NO_TRANSMISSION;
#else
        ChannelInternal->IsUserDataSet = FALSE;
#endif
#endif

        /** @req CANNM085 */
        memset(ChannelInternal->TxMessageSdu, 0x00, 8);
#if (CANNM_USER_DATA_ENABLED == STD_ON) && (CANNM_COM_USER_DATA_SUPPORT == STD_ON)
        memset(ChannelInternal->SpontaneousTxMessageSdu, 0x00, 8);
#endif
        memset(ChannelInternal->RxMessageSdu, 0x00, 8);

        /** @req CANNM025 */
        uint8* destUserData = CanNm_Internal_GetUserDataPtr(ChannelConf, ChannelInternal->TxMessageSdu);
        PduLengthType userDataLength = CanNm_Internal_GetUserDataLength(ChannelConf);
        memset(destUserData, 0xFFu, (size_t)userDataLength);

#if (CANNM_USER_DATA_ENABLED == STD_ON) && (CANNM_COM_USER_DATA_SUPPORT == STD_ON)
        destUserData = CanNm_Internal_GetUserDataPtr(ChannelConf, ChannelInternal->SpontaneousTxMessageSdu);
        userDataLength = CanNm_Internal_GetUserDataLength(ChannelConf);
        memset(destUserData, 0xFF, (size_t)userDataLength);
#endif

        /** @req CANNM013 */
        if (ChannelConf->NidPosition != CANNM_PDU_OFF) {
            ChannelInternal->TxMessageSdu[ChannelConf->NidPosition] = ChannelConf->NodeId;
#if (CANNM_USER_DATA_ENABLED == STD_ON) && (CANNM_COM_USER_DATA_SUPPORT == STD_ON)
            ChannelInternal->SpontaneousTxMessageSdu[ChannelConf->NidPosition] = ChannelConf->NodeId;
#endif
        }

        /** @req CANNM045 */
        if (ChannelConf->CbvPosition != CANNM_PDU_OFF) {
            ChannelInternal->TxMessageSdu[ChannelConf->CbvPosition] = 0x00;
#if (CANNM_USER_DATA_ENABLED == STD_ON) && (CANNM_COM_USER_DATA_SUPPORT == STD_ON)
            ChannelInternal->SpontaneousTxMessageSdu[ChannelConf->CbvPosition] = 0x00;
#endif
        }
        /** @req CANNM060 */
#if (CANNM_PASSIVE_MODE_ENABLED != STD_ON)
        if (ChannelConf->ImmediateNmTransmissions > 0) {
            ChannelInternal->MessageCycleTimeLeft = 0;
        } else {
            ChannelInternal->MessageCycleTimeLeft = 0;
        }
        ChannelInternal->MessageTimeoutTimeLeft = 0;
#endif
    }

/* Initialize the PN reset timer */
#if ((CANNM_PNC_EIRA_CALC_ENABLED == STD_ON) && (CANNM_PNC_COUNT > 0))
    for (uint8 i = 0; i < CANNM_PNC_COUNT; i++) {
        CanNm_Internal.pnEIRATimers[i].timerRunning = FALSE;
        CanNm_Internal.pnEIRATimers[i].resetTimer = 0;
    }
    /* @req CANNM424 */
    CanNm_Internal.pnEIRA.data = 0; /* reset EIRA */
#endif
#if ((CANNM_PNC_ERA_CALC_ENABLED == STD_ON) && (CANNM_PNC_COUNT > 0))
    for (uint8 chanIndex = 0; chanIndex < CANNM_CHANNEL_COUNT; chanIndex++) {
        for (uint8 timerIndex = 0; timerIndex < CANNM_PNC_COUNT; timerIndex++) {
            CanNm_Internal.Channels[chanIndex].pnERATimers[timerIndex].timerRunning = FALSE;
            CanNm_Internal.Channels[chanIndex].pnERATimers[timerIndex].resetTimer = 0;
        }
        /* @req CANNM435 */
        CanNm_Internal.Channels[chanIndex].pnERA.data = 0;
    }
#endif
    CanNm_Internal.InitStatus = CANNM_STATUS_INIT;

    /** @req CANNM061 */
    /** @req CANNM033 */
}

/** Passive startup of the AUTOSAR CAN NM. It triggers the transition from Bus-
  * Sleep Mode to the Network Mode in Repeat Message State.
  * This service has no effect if the current state is not equal to Bus-Sleep Mode. In
  * that case NM_E_NOT_EXECUTED is returned. */
Std_ReturnType CanNm_PassiveStartUp( const NetworkHandleType nmHandle){
    CANNM_VALIDATE_INIT(CANNM_SERVICEID_PASSIVESTARTUP);
    CANNM_VALIDATE_CHANNEL(nmHandle, CANNM_SERVICEID_PASSIVESTARTUP);

    uint8 channelIndex = CanNm_ConfigPtr->ChannelLookups[nmHandle];

    /** @req CANNM254 */
    const CanNm_ChannelType* ChannelConf = &CanNm_ConfigPtr->Channels[channelIndex];
    CanNm_Internal_ChannelType* ChannelInternal = &CanNm_Internal.Channels[channelIndex];
    Std_ReturnType status = E_OK;

    if (ChannelInternal->Mode == NM_MODE_BUS_SLEEP) {
        CanNm_Internal_BusSleep_to_RepeatMessage(ChannelConf, ChannelInternal,FALSE);  /**< @req CANNM128  @req CANNM314 */
        status = E_OK;
    } else {
        status = E_NOT_OK;  /** @req CANNM147 */ /** @req CANNM212 */
    }
#if (CANNM_PNC_COUNT > 0)
    if (ChannelConf->CanNmPnEnable) {
        /* @req CANNM413 */
        /* @req CANNM414 */
        CanNm_Internal_SetPNICbv(ChannelConf,ChannelInternal);
    }
#endif
    return status;
}

/** Request the network, since ECU needs to communicate on the bus. Network
  * state shall be changed to �requested� */
Std_ReturnType CanNm_NetworkRequest( const NetworkHandleType nmHandle ){

    CANNM_VALIDATE_INIT(CANNM_SERVICEID_NETWORKREQUEST);

    CANNM_VALIDATE_CHANNEL(nmHandle, CANNM_SERVICEID_NETWORKREQUEST);
    uint8 channelIndex = CanNm_ConfigPtr->ChannelLookups[nmHandle];

    /** @req CANNM256 */

    const CanNm_ChannelType* ChannelConf = &CanNm_ConfigPtr->Channels[channelIndex];
    CanNm_Internal_ChannelType* ChannelInternal = &CanNm_Internal.Channels[channelIndex];

    /** @req CANNM255 */

    ChannelInternal->Requested = TRUE;  /**< @req CANNM104 */ /* reqeuested should be a state and not a flag */

    if (ChannelInternal->Mode == NM_MODE_BUS_SLEEP) {
        CanNm_Internal_BusSleep_to_RepeatMessage(ChannelConf, ChannelInternal,TRUE);  /**< @req CANNM129  @req CANNM314 */
    } else if (ChannelInternal->Mode == NM_MODE_PREPARE_BUS_SLEEP) {
        CanNm_Internal_PrepareBusSleep_to_RepeatMessage(ChannelConf, ChannelInternal, TRUE);  /**< @req CANNM123  @req CANNM315 */
    } else if (ChannelInternal->Mode == NM_MODE_NETWORK) {
        if (ChannelInternal->State == NM_STATE_READY_SLEEP) {
            CanNm_Internal_ReadySleep_to_NormalOperation(ChannelConf, ChannelInternal);  /**< @req CANNM110 */
        }
    } else {
        //Nothing to be done
    }
#if (CANNM_PNC_COUNT > 0)
        if (ChannelConf->CanNmPnEnable)
        {
            /* @req CANNM413 */
            /* @req CANNM414 */
            CanNm_Internal_SetPNICbv(ChannelConf,ChannelInternal);
        }
#endif
    return E_OK;
}

/** Release the network, since ECU doesn�t have to communicate on the bus. Network
  * state shall be changed to �released�. */
Std_ReturnType CanNm_NetworkRelease( const NetworkHandleType nmHandle ){
    Std_ReturnType status;
    status = E_OK;
    CANNM_VALIDATE_INIT(CANNM_SERVICEID_NETWORKRELEASE);
    CANNM_VALIDATE_CHANNEL(nmHandle, CANNM_SERVICEID_NETWORKRELEASE);
    uint8 channelIndex = CanNm_ConfigPtr->ChannelLookups[nmHandle];

    /** @req CANNM259 */

    const CanNm_ChannelType* ChannelConf = &CanNm_ConfigPtr->Channels[channelIndex];
    CanNm_Internal_ChannelType* ChannelInternal = &CanNm_Internal.Channels[channelIndex];

    if (!ChannelInternal->CommunicationEnabled) {
        status = E_NOT_OK; /* @req CANNM294 */
    }
    if (status == E_OK) {

        /** @req CANNM258 */
        ChannelInternal->Requested = FALSE;  /**< @req CANNM105 */ /* released should be a state and not a flag */

        if (ChannelInternal->Mode == NM_MODE_NETWORK) {
            if (ChannelInternal->State == NM_STATE_NORMAL_OPERATION) {
                CanNm_Internal_NormalOperation_to_ReadySleep(ChannelConf, ChannelInternal);  /**< @req CANNM118 */
            }
        }
    }
    return status;
}

/** Disable the NM PDU transmission ability due to a ISO14229 Communication
  * Control (28hex) service */
#if (CANNM_COM_CONTROL_ENABLED == STD_ON) /* @req CANNM262 */
Std_ReturnType CanNm_DisableCommunication( const NetworkHandleType nmHandle ){

    Std_ReturnType status;
    status = E_OK;
    CANNM_VALIDATE_INIT(CANNM_SERVICEID_DISABLECOMMUNICATION); /* @req CANNM261 */
#if (CANNM_PASSIVE_MODE_ENABLED == STD_ON) /** @req CANNM298 */
    status = E_NOT_OK;
#endif

    if(status == E_OK){
        CANNM_VALIDATE_CHANNEL(nmHandle, CANNM_SERVICEID_DISABLECOMMUNICATION);
        uint8 channelIndex = CanNm_ConfigPtr->ChannelLookups[nmHandle];

        CanNm_Internal_ChannelType* ChannelInternal = &CanNm_Internal.Channels[channelIndex];

        if (ChannelInternal->Mode != NM_MODE_NETWORK) {
            status = E_NOT_OK; /* @req CANNM172 */
        }
        if (status == E_OK){
            ChannelInternal->CommunicationEnabled = FALSE;
        }
    }
    return status;
}
#endif


/** Enable the NM PDU transmission ability due to a ISO14229 Communication
  * Control (28hex) service */
#if (CANNM_COM_CONTROL_ENABLED == STD_ON) /* @req CANNM264 */
Std_ReturnType CanNm_EnableCommunication( const NetworkHandleType nmHandle ){

    Std_ReturnType status;
    status = E_OK;
    CANNM_VALIDATE_INIT(CANNM_SERVICEID_ENABLECOMMUNICATION); /* @req CANNM263 */
#if (CANNM_PASSIVE_MODE_ENABLED == STD_ON) /** @req CANNM297 */
    status = E_NOT_OK;
#endif
    if (status == E_OK){
        CANNM_VALIDATE_CHANNEL(nmHandle, CANNM_SERVICEID_ENABLECOMMUNICATION);
        uint8 channelIndex = CanNm_ConfigPtr->ChannelLookups[nmHandle];

        CanNm_Internal_ChannelType* ChannelInternal = &CanNm_Internal.Channels[channelIndex];
        const CanNm_ChannelType* ChannelConf = &CanNm_ConfigPtr->Channels[channelIndex];

        if (ChannelInternal->Mode != NM_MODE_NETWORK) {/* @req CANNM295 */
            status = E_NOT_OK; /* @req CANNM295 */
        }
        if (status == E_OK){
            if (ChannelInternal->CommunicationEnabled) {
                status = E_NOT_OK; /* @req CANNM177*/
            }
            if (status == E_OK) {
                ChannelInternal->MessageCycleTimeLeft = ChannelConf->MessageCycleOffsetTime + ChannelConf->MainFunctionPeriod; /* @req CANNM178 */
                ChannelInternal->TimeoutTimeLeft = ChannelConf->TimeoutTime + ChannelConf->MainFunctionPeriod; /* @req CANNM179 */
                ChannelInternal->CommunicationEnabled = TRUE;  /* @req CANNM176 */
            }
        }
    }
    return status;

}
#endif

#if (CANNM_PASSIVE_MODE_ENABLED != STD_ON) /** @req CANNM266 */
#if (CANNM_USER_DATA_ENABLED == STD_ON)  /** @req CANNM158 */
#if (CANNM_COM_USER_DATA_SUPPORT == STD_OFF) /** @req CANNM327 */
/** Set user data for NM messages transmitted next on the bus. */
/** @req CANNM159 */
Std_ReturnType CanNm_SetUserData( const NetworkHandleType nmHandle, const uint8* const nmUserDataPtr ){
    CANNM_VALIDATE_INIT(CANNM_SERVICEID_SETUSERDATA);

    CANNM_VALIDATE_CHANNEL(nmHandle, CANNM_SERVICEID_SETUSERDATA);
    uint8 channelIndex = CanNm_ConfigPtr->ChannelLookups[nmHandle];
    CANNM_VALIDATE_NOTNULL(nmUserDataPtr, CANNM_SERVICEID_SETUSERDATA, nmHandle);

    /** @req CANNM265 */

    Std_ReturnType retVal = E_NOT_OK;
    const CanNm_ChannelType* ChannelConf = &CanNm_ConfigPtr->Channels[channelIndex];
    CanNm_Internal_ChannelType* ChannelInternal = &CanNm_Internal.Channels[channelIndex];

    /* can only be called once per cycle (per channel) */
    if (!ChannelInternal->IsUserDataSet) /* this flag avoids inconsistency */
    {
        SchM_Enter_CanNm_EA_0();

        ChannelInternal->IsUserDataSet = TRUE;

        SchM_Exit_CanNm_EA_0();

        {
            uint8* destUserData = CanNm_Internal_GetUserDataPtr(ChannelConf, ChannelInternal->TxMessageSdu);
            PduLengthType userDataLength = CanNm_Internal_GetUserDataLength(ChannelConf);

            memcpy(destUserData, nmUserDataPtr, (size_t)userDataLength);
        }

        ChannelInternal->IsUserDataSet = FALSE;

        retVal = E_OK;
    }

    return retVal;
}
#endif
#endif
#endif

#if (CANNM_USER_DATA_ENABLED == STD_ON)  /** @req CANNM158 */ /**@req CANNM268 */
/** Get user data out of the most recently received NM message. */
/** @req CANNM160 */
Std_ReturnType CanNm_GetUserData( const NetworkHandleType nmHandle, uint8* const nmUserDataPtr ){
    CANNM_VALIDATE_INIT(CANNM_SERVICEID_GETUSERDATA);

    CANNM_VALIDATE_CHANNEL(nmHandle, CANNM_SERVICEID_GETUSERDATA);
    uint8 channelIndex = CanNm_ConfigPtr->ChannelLookups[nmHandle];

    CANNM_VALIDATE_NOTNULL(nmUserDataPtr, CANNM_SERVICEID_GETUSERDATA, nmHandle);

    /** @req CANNM267 */

    const CanNm_ChannelType* ChannelConf = &CanNm_ConfigPtr->Channels[channelIndex];
    CanNm_Internal_ChannelType* ChannelInternal = &CanNm_Internal.Channels[channelIndex];
    const uint8* sourceUserData = CanNm_Internal_GetUserDataPtr(ChannelConf, ChannelInternal->RxMessageSdu);
    PduLengthType userDataLength = CanNm_Internal_GetUserDataLength(ChannelConf);

    memcpy(nmUserDataPtr, sourceUserData, (size_t)userDataLength);

    return E_OK;
}
#endif


/** Get node identifier out of the most recently received NM PDU. */
/** @req CANNM132 */
Std_ReturnType CanNm_GetNodeIdentifier( const NetworkHandleType nmHandle, uint8 * const nmNodeIdPtr ){
    CANNM_VALIDATE_INIT(CANNM_SERVICEID_GETNODEIDENTIFIER);
    CANNM_VALIDATE_CHANNEL(nmHandle, CANNM_SERVICEID_GETNODEIDENTIFIER);

    uint8 channelIndex = CanNm_ConfigPtr->ChannelLookups[nmHandle];
    CANNM_VALIDATE_NOTNULL(nmNodeIdPtr, CANNM_SERVICEID_GETNODEIDENTIFIER, nmHandle);

    /** @req CANNM269 */
    const CanNm_ChannelType* ChannelConf = &CanNm_ConfigPtr->Channels[channelIndex];
    const CanNm_Internal_ChannelType* ChannelInternal = &CanNm_Internal.Channels[channelIndex];
    Std_ReturnType status = E_NOT_OK;

    if (ChannelConf->NidPosition == CANNM_PDU_OFF) { /** @req CANNM270 */
        /* status = NM_E_NOT_EXECUTED; */
    } else {
        *nmNodeIdPtr = ChannelInternal->RxMessageSdu[ChannelConf->NidPosition];
        status = E_OK;
    }
    return status;
}

/** Get node identifier configured for the local node. */
/** @req CANNM133 */
Std_ReturnType CanNm_GetLocalNodeIdentifier( const NetworkHandleType nmHandle, uint8 * const nmNodeIdPtr ){
    CANNM_VALIDATE_INIT(CANNM_SERVICEID_GETLOCALNODEIDENTIFIER);

    CANNM_VALIDATE_CHANNEL(nmHandle, CANNM_SERVICEID_GETLOCALNODEIDENTIFIER);
    uint8 channelIndex = CanNm_ConfigPtr->ChannelLookups[nmHandle];

    CANNM_VALIDATE_NOTNULL(nmNodeIdPtr, CANNM_SERVICEID_GETLOCALNODEIDENTIFIER, nmHandle);

    /** @req CANNM271 */

    const CanNm_ChannelType* ChannelConf = &CanNm_ConfigPtr->Channels[channelIndex];
    *nmNodeIdPtr = ChannelConf->NodeId;

    return E_OK;
}


#if (CANNM_NODE_DETECTION_ENABLED == STD_ON)
/** Set Repeat Message Request Bit for NM messages transmitted next on the bus. */
/** @req CANNM135  @req CANNM274 */
Std_ReturnType CanNm_RepeatMessageRequest( const NetworkHandleType nmHandle ){
    CANNM_VALIDATE_INIT(CANNM_SERVICEID_REPEATMESSAGEREQUEST);
    CANNM_VALIDATE_CHANNEL(nmHandle, CANNM_SERVICEID_REPEATMESSAGEREQUEST);

    uint8 channelIndex = CanNm_ConfigPtr->ChannelLookups[nmHandle];

    /** @req CANNM273 */
    const CanNm_ChannelType* ChannelConf = &CanNm_ConfigPtr->Channels[channelIndex];
    CanNm_Internal_ChannelType* ChannelInternal = &CanNm_Internal.Channels[channelIndex];

    Std_ReturnType status = E_NOT_OK;  /**< @req CANNM137 */

    if (ChannelConf->CbvPosition != CANNM_PDU_OFF) {
        if (ChannelInternal->State == NM_STATE_READY_SLEEP) {

            ChannelInternal->TxMessageSdu[ChannelConf->CbvPosition] |= CANNM_CBV_REPEAT_MESSAGE_REQUEST;  /**< @req CANNM113 */
#if (CANNM_USER_DATA_ENABLED == STD_ON) && (CANNM_COM_USER_DATA_SUPPORT == STD_ON)
            ChannelInternal->SpontaneousTxMessageSdu[ChannelConf->CbvPosition] |= CANNM_CBV_REPEAT_MESSAGE_REQUEST;
#endif
            CanNm_Internal_ReadySleep_to_RepeatMessage(ChannelConf, ChannelInternal);  /**< @req CANNM112 */
            status = E_OK;
        } else if (ChannelInternal->State == NM_STATE_NORMAL_OPERATION) {
            ChannelInternal->TxMessageSdu[ChannelConf->CbvPosition] |=  CANNM_CBV_REPEAT_MESSAGE_REQUEST;  /**< @req CANNM121 */
#if (CANNM_USER_DATA_ENABLED == STD_ON) && (CANNM_COM_USER_DATA_SUPPORT == STD_ON)
            ChannelInternal->SpontaneousTxMessageSdu[ChannelConf->CbvPosition] |= CANNM_CBV_REPEAT_MESSAGE_REQUEST;
#endif
            CanNm_Internal_NormalOperation_to_RepeatMessage(ChannelConf, ChannelInternal);  /**< @req CANNM120 */
            status = E_OK;
        } else {
            //Nothing to be done
        }
    }
    return status;
}
#endif

/** Get the whole PDU data out of the most recently received NM message. */
/** @req CANNM138  @req CANNM276 */
Std_ReturnType CanNm_GetPduData( const NetworkHandleType nmHandle, uint8 * const nmPduDataPtr ){
    CANNM_VALIDATE_INIT(CANNM_SERVICEID_GETPDUDATA);
    CANNM_VALIDATE_CHANNEL(nmHandle, CANNM_SERVICEID_GETPDUDATA);
    CANNM_VALIDATE_NOTNULL(nmPduDataPtr, CANNM_SERVICEID_GETPDUDATA, nmHandle);

    uint8 channelIndex = CanNm_ConfigPtr->ChannelLookups[nmHandle];

    /** @req CANNM275 */

    const CanNm_ChannelType* ChannelConf = &CanNm_ConfigPtr->Channels[channelIndex];
    const CanNm_Internal_ChannelType* ChannelInternal = &CanNm_Internal.Channels[channelIndex];

    memcpy(nmPduDataPtr, ChannelInternal->RxMessageSdu, (size_t)ChannelConf->PduLength);

    return E_OK;
}

/** Returns the state and the mode of the network management. */
Std_ReturnType CanNm_GetState( const NetworkHandleType nmHandle, Nm_StateType * const nmStatePtr, Nm_ModeType * const nmModePtr ){
    CANNM_VALIDATE_INIT(CANNM_SERVICEID_GETSTATE);

    CANNM_VALIDATE_CHANNEL(nmHandle, CANNM_SERVICEID_GETSTATE);
    uint8 channelIndex = CanNm_ConfigPtr->ChannelLookups[nmHandle];

    CANNM_VALIDATE_NOTNULL(nmStatePtr, CANNM_SERVICEID_GETSTATE, nmHandle);
    CANNM_VALIDATE_NOTNULL(nmModePtr, CANNM_SERVICEID_GETSTATE, nmHandle);

    /** @req CANNM277 */

    const CanNm_Internal_ChannelType* ChannelInternal = &CanNm_Internal.Channels[channelIndex];
    *nmStatePtr = ChannelInternal->State;
    *nmModePtr = ChannelInternal->Mode;

    return E_OK;
}

/** Request bus synchronization. */
Std_ReturnType CanNm_RequestBusSynchronization( const NetworkHandleType nmHandle ){

    /* @req CANNM279 */
    CANNM_VALIDATE_INIT(CANNM_SERVICEID_REQUESTBUSSYNCHRONIZATION);
    CANNM_VALIDATE_CHANNEL(nmHandle, CANNM_SERVICEID_REQUESTBUSSYNCHRONIZATION);
    // Not implemented
    (void)nmHandle;
    return E_NOT_OK;
}

/** Check if remote sleep indication takes place or not. */
Std_ReturnType CanNm_CheckRemoteSleepIndication( const NetworkHandleType nmHandle, boolean * const nmRemoteSleepIndPtr ){
    /* @req CANNM281 */
    CANNM_VALIDATE_INIT(CANNM_SERVICEID_CHECKREMOTESLEEPINDICATION);

    CANNM_VALIDATE_CHANNEL(nmHandle, CANNM_SERVICEID_CHECKREMOTESLEEPINDICATION);
    CANNM_VALIDATE_NOTNULL(nmRemoteSleepIndPtr, CANNM_SERVICEID_CHECKREMOTESLEEPINDICATION, nmHandle);

    //lint -estring(920,pointer)  /* cast to void */
    (void)nmRemoteSleepIndPtr;
    (void)nmHandle;
    //lint +estring(920,pointer)  /* cast to void */
    // Not implemented
    return E_NOT_OK;
}


// Functions called by CAN Interface
// ---------------------------------

/** This service confirms a previous successfully processed CAN transmit request.
  * This callback service is called by the CanIf and implemented by the CanNm. */


/* modified CanNm_TxConfirmation & CanNm_Internal_TransmitMessage:
    -when CanNm_Transmit is called by the PduR due to @req CANNM329 PduR_CanNmTxConfirmation must be called (when message was sent successfully)
     => so CanNm_TxConfirmation was modified
    -due to the fact that destinction between cyclic messages and spontaneous massages is now a mondatory feature (spontaneous transmission => CanNm_Transmit)
     CanNm_TxConfirmation has to decide what kind of message was lastly sent
     => this is done by a new internal flag in "CanNm_Internal_ChannelType" => TransmissionStatus
     => the flag is set by CanNm_Transmit (spontaneous message = CANNM_SPONTANEOUS_TRANSMISSION) & CanNm_internal_TransmitMessage (cyclic message = CANNM_ONGOING_TRANSMISSION)
     => cyclic messages or spontaneous messages can only be sent when the flag has "CANNM_NO_TRANSMISSION" status
     => when confirmation arrives (CanNm_TxConfirmation) the flag is set back to the lastly mentioned status
    -because CanNm_Transmit & CanNm_internal_TransmitMessage both can set the mentioned flag an exclusive area (EXCLUSIVE_AREA_0) is used (SchM_CanNm is now mandatory)
    -when CanNm_Transmit is enabled (CANNM_COM_USER_DATA_SUPPORT = STD_ON) CanNm_Internal_TransmitMessage needs to fetch the user Data from PduR by calling PduR_CanNmTriggerTransmit (@req CANNM328) */

void CanNm_TxConfirmation( PduIdType CanNmTxPduId ){
    CANNM_VALIDATE_INIT_NORV(CANNM_SERVICEID_TXCONFIRMATION, CanNmTxPduId); /** @req CANNM229 */
    CANNM_VALIDATE_PDUID_NORV(CanNmTxPduId, CANNM_SERVICEID_TXCONFIRMATION); /** @req CANNM229 */

    /** @req CANNM283 */

    const CanNm_ChannelType* ChannelConf = &CanNm_ConfigPtr->Channels[CanNmTxPduId];
    CanNm_Internal_ChannelType* ChannelInternal = &CanNm_Internal.Channels[CanNmTxPduId];

#if (CANNM_USER_DATA_ENABLED == STD_ON) && (CANNM_COM_USER_DATA_SUPPORT == STD_ON)
    /* is there an ongoing transmit */
    if (ChannelInternal->TransmissionStatus == CANNM_SPONTANEOUS_TRANSMISSION)
    {
        /* transmit was ok */
        PduR_CanNmTxConfirmation(ChannelConf->CanNmUserDataTxPduId); /** @req CANNM329 */
        ChannelInternal->TransmissionStatus = CANNM_NO_TRANSMISSION;
    }
    else
#endif

    {
        if (ChannelInternal->Mode == NM_MODE_NETWORK) {
            CanNm_Internal_NetworkMode_to_NetworkMode(ChannelConf, ChannelInternal);  /**< @req CANNM099 */
        }
    }

    ChannelInternal->MessageTimeoutTimeLeft = 0; /** @req CANNM065 **/
}

/** This service indicates a successful reception of a received NM message to the
  * CanNm after passing all filters and validation checks.
  * This callback service is called by the CAN Interface and implemented by the CanNm. */
/* @req CANNM135 */
/*lint -efunc(818,CanNm_RxIndication) Cannot declare pionter as pointing to const as API is defined by AUTOSAR */
void CanNm_RxIndication( PduIdType CanNmRxPduId, PduInfoType *PduInfoPtr ) {

    /* @req CANNM285 */
    boolean status;
    status = TRUE;
    CANNM_VALIDATE_INIT_NORV(CANNM_SERVICEID_RXINDICATION, CanNmRxPduId); /** @req CANNM232 */ /** @req CANNM233 */
    CANNM_VALIDATE_NOTNULL_NORV(PduInfoPtr, CANNM_SERVICEID_RXINDICATION, CanNmRxPduId); /** @req CANNM232 */ /** @req CANNM233 */
    CANNM_VALIDATE_NOTNULL_NORV(PduInfoPtr->SduDataPtr, CANNM_SERVICEID_RXINDICATION, CanNmRxPduId); /** @req CANNM232 */ /** @req CANNM233 */
    CANNM_VALIDATE_PDUID_NORV(CanNmRxPduId, CANNM_SERVICEID_RXINDICATION); /** @req CANNM232 */ /** @req CANNM233 */

    const CanNm_ChannelType* ChannelConf = &CanNm_ConfigPtr->Channels[CanNmRxPduId];
    CanNm_Internal_ChannelType* ChannelInternal = &CanNm_Internal.Channels[CanNmRxPduId];

    memcpy(ChannelInternal->RxMessageSdu, PduInfoPtr->SduDataPtr, (size_t)ChannelConf->PduLength);  /**< @req CANNM035 */
#if (CANNM_NODE_DETECTION_ENABLED == STD_ON)
    boolean repeatMessageBitIndication = FALSE;
#endif /* (CANNM_NODE_DETECTION_ENABLED == STD_ON) */
#if (CANNM_NODE_DETECTION_ENABLED == STD_ON) || (CANNM_PNC_COUNT > 0)
    if (ChannelConf->CbvPosition != CANNM_PDU_OFF) {
        uint8 cbv = ChannelInternal->RxMessageSdu[ChannelConf->CbvPosition];
#if (CANNM_NODE_DETECTION_ENABLED == STD_ON)
        repeatMessageBitIndication = ((cbv & CANNM_CBV_REPEAT_MESSAGE_REQUEST) != 0u) ? TRUE: FALSE;
#endif /* (CANNM_NODE_DETECTION_ENABLED == STD_ON) */
#if  (CANNM_PNC_COUNT > 0)
        Std_ReturnType ret;
        boolean pni = ((cbv & CANNM_CBV_PNI) != 0) ? TRUE: FALSE;
        ret = CanNm_Internal_RxProcess(ChannelConf,ChannelInternal,pni);

        if (E_NOT_OK == ret){
            status = FALSE;
        }
#endif
    }
#endif /* (CANNM_NODE_DETECTION_ENABLED == STD_ON) || (CANNM_PNC_COUNT > 0) */
    if (status == TRUE) {
        if (ChannelInternal->Mode == NM_MODE_BUS_SLEEP) {

#if (CANNM_DEV_ERROR_DETECT == STD_ON)
            /* @req CANNM019 */
            CANNM_DET_REPORTERROR(CANNM_SERVICEID_RXINDICATION, CANNM_E_NET_START_IND, CanNmRxPduId); /** @req CANNM336 */
#endif
            CanNm_Internal_BusSleep_to_BusSleep(ChannelConf, ChannelInternal);  /**< @req CANNM127 */
        } else if (ChannelInternal->Mode == NM_MODE_PREPARE_BUS_SLEEP) {
            CanNm_Internal_PrepareBusSleep_to_RepeatMessage(ChannelConf, ChannelInternal,FALSE);  /**< @req CANNM124  @req CANNM315 */
        } else if (ChannelInternal->Mode == NM_MODE_NETWORK) {
            CanNm_Internal_NetworkMode_to_NetworkMode(ChannelConf, ChannelInternal);  /**< @req CANNM098 */
#if (CANNM_NODE_DETECTION_ENABLED == STD_ON)
            if (repeatMessageBitIndication) {
                if (ChannelInternal->State == NM_STATE_READY_SLEEP) {
                    CanNm_Internal_ReadySleep_to_RepeatMessage(ChannelConf, ChannelInternal);  /**< @req CANNM111 */
                } else if (ChannelInternal->State == NM_STATE_NORMAL_OPERATION) {
                    CanNm_Internal_NormalOperation_to_RepeatMessage(ChannelConf, ChannelInternal);  /**< @req CANNM119 */
                } else {
                    //Nothing to be done
                }
            }
#endif /* (CANNM_NODE_DETECTION_ENABLED == STD_ON) */
        } else {
            //Nothing to be done
        }
#if (CANNM_PDU_RX_INDICATION_ENABLED == STD_ON)
        // IMPROVEMENT: call NM rx indication
#endif
    }
}


/**
 * This function is used by the PduR to trigger a spontaneous transmission of an NM message
 * with the provided NM User Data
 */

/* when transmit is called trough PduR the given CanNmUserDataTxPduId must be matched to the internal Channel
    => therefore each channel now needs the information which PduID matches to its internal configuration
    => added CanNmUserDataTxPduId to "CanNm_ChannelType"
    => this information must be filled at configuration-time
    the transmit-function seeks all channels for the given pdu and goes on when its found => E_NOT_OK when not */

Std_ReturnType CanNm_Transmit( PduIdType CanNmUserDataTxPduId, const PduInfoType *PduInfoPtr) {
    (void)CanNmUserDataTxPduId; /* Avoid compiler warning - used depedning on config */
    //lint -estring(920,pointer)  /* cast to void */
    (void)PduInfoPtr; /* Avoid compiler warning - used depedning on config */
    //lint +estring(920,pointer)  /* cast to void */

    Std_ReturnType retVal;
    retVal = E_NOT_OK;
#if (CANNM_USER_DATA_ENABLED == STD_ON) && (CANNM_COM_USER_DATA_SUPPORT == STD_ON) /** @req CANNM330 */

    /* only transmit when in
     * repeat message state or normal operation state
     * otherwise return E_NOT_OK;
     */

    boolean found = FALSE;

    /* read which channel is affected trough this CanNmUserDataTxPduId */
    uint8 affectedChannel;


    for (affectedChannel = 0; affectedChannel < CANNM_CHANNEL_COUNT; affectedChannel++) {
        if (CanNmUserDataTxPduId == CanNm_ConfigPtr->Channels[affectedChannel].CanNmUserDataTxPduId) {
            found = TRUE;
            break;
        }
    }
    /* If not found this pdu is invalid */
    CANNM_VALIDATE(found, CANNM_SERVICEID_TRANSMIT, CANNM_E_INVALID_PDUID, 0, E_NOT_OK);
    /* is channel configured */
    CANNM_VALIDATE_CHANNEL(affectedChannel, CANNM_SERVICEID_TRANSMIT);
    /* is pdu ptr null? */
    CANNM_VALIDATE_NOTNULL(PduInfoPtr, CANNM_SERVICEID_TRANSMIT, affectedChannel); /* no requirement */
    /* is data ptr null */
    CANNM_VALIDATE_NOTNULL(PduInfoPtr->SduDataPtr, CANNM_SERVICEID_TRANSMIT, affectedChannel); /* no requirement */

    if ((found == TRUE) && (affectedChannel < CANNM_CHANNEL_COUNT)) {
        const CanNm_ChannelType* 	ChannelConf = &CanNm_ConfigPtr->Channels[affectedChannel];
        CanNm_Internal_ChannelType* ChannelInternal = &CanNm_Internal.Channels[affectedChannel];

        /* @req CANNM170*/
        if (((ChannelInternal->State == NM_STATE_REPEAT_MESSAGE) || (ChannelInternal->State == NM_STATE_NORMAL_OPERATION)) &&
             (ChannelInternal->CommunicationEnabled)) {
            SchM_Enter_CanNm_EA_0();

            /* check if there is an ongoing uncommited transmission */
            if (ChannelInternal->TransmissionStatus == CANNM_NO_TRANSMISSION) {
                uint8* destUserData = CanNm_Internal_GetUserDataPtr(ChannelConf, ChannelInternal->SpontaneousTxMessageSdu);
                PduLengthType userDataLength = CanNm_Internal_GetUserDataLength(ChannelConf);

                ChannelInternal->TransmissionStatus = CANNM_SPONTANEOUS_TRANSMISSION;

                SchM_Exit_CanNm_EA_0();

                memcpy(destUserData, PduInfoPtr->SduDataPtr, (size_t)userDataLength);

                {
                    PduInfoType pdu = {
                            .SduDataPtr = ChannelInternal->SpontaneousTxMessageSdu,
                            .SduLength = ChannelConf->PduLength,
                    };

                    retVal = CanIf_Transmit(ChannelConf->CanIfPduId, &pdu);

                    /* in order to avoid locking the possibility to send we have to reset the flag when CanIf_Tansmit fails */
                    if (retVal != E_OK) {
                        ChannelInternal->TransmissionStatus = CANNM_NO_TRANSMISSION;
                    }
#if ((CANNM_PNC_EIRA_CALC_ENABLED == STD_ON) && (CANNM_PNC_COUNT > 0))
                    if (ChannelConf->CanNmPnEnable) {
                        CanNm_Internal_ProcessTxPdu(pdu.SduDataPtr+CanNm_ConfigPtr->CanNmPnInfo->CanNmPnInfoOffset);
                    }
#endif
                }
            } else {
                SchM_Exit_CanNm_EA_0();
            }
        }
    }
#else
    retVal = E_OK; /** @req CANNM333 */
#endif
    return retVal;
}

/**
 * Set the NM Coordinator Sleep Ready bit in the Control Bit Vector
 * CURRENTLY UNSUPPORTED
 */
Std_ReturnType CanNm_SetSleepReadyBit( const NetworkHandleType nmHandle, const boolean nmSleepReadyBit) {
    /* not supported */
    (void)nmHandle;
    (void)nmSleepReadyBit;
    return E_NOT_OK;
}


/** @req CANNM234 */
void CanNm_MainFunction( void ) {

    CANNM_VALIDATE_INIT_NORV(CANNM_SERVICEID_MAINFUNCTION,0); /** @req CANNM235 */ /**@req CANNM236 */

    uint8 channel;
    for (channel = 0; channel < CANNM_CHANNEL_COUNT; channel++) {

        /** @req CANNM108 */
        const CanNm_ChannelType* ChannelConf = &CanNm_ConfigPtr->Channels[channel];
        CanNm_Internal_ChannelType* ChannelInternal = &CanNm_Internal.Channels[channel];
        if (ChannelInternal->Mode == NM_MODE_NETWORK) {

            if (ChannelInternal->State == NM_STATE_REPEAT_MESSAGE) {
                CanNm_Internal_TickRepeatMessageTime(ChannelConf, ChannelInternal);  /**< @req CANNM102 */
            }
#if (CANNM_PASSIVE_MODE_ENABLED != STD_ON)
            CanNm_Internal_TickTxTimeout(ChannelConf, ChannelInternal);
#endif
            /* @req CANNM174 */
            if (ChannelInternal->CommunicationEnabled) {
                CanNm_Internal_TickTimeoutTime(ChannelConf, ChannelInternal);
            }

#if (CANNM_PASSIVE_MODE_ENABLED != STD_ON)

            /** @req CANNM161 */ /** @req CANNM162 */ /** @req CANNM072 */
            if (((ChannelInternal->State == NM_STATE_REPEAT_MESSAGE) || (ChannelInternal->State == NM_STATE_NORMAL_OPERATION)) &&
                (ChannelInternal->CommunicationEnabled)) {
                /** @req CANNM051 @req CANNM032  @req CANNM087  @req CANNM100 */
                /** @req CANNM173 @req CANNM170 */

                CanNm_Internal_TickMessageCycleTime(ChannelConf, ChannelInternal);
            }
#endif
        } else if (ChannelInternal->Mode == NM_MODE_PREPARE_BUS_SLEEP) {
            CanNm_Internal_TickWaitBusSleepTime(ChannelConf, ChannelInternal);  /**< @req CANNM115 */
        } else {
            //Nothing to be done
        }

    }
#if ((CANNM_PNC_EIRA_CALC_ENABLED == STD_ON) && (CANNM_PNC_COUNT > 0))
    CanNm_Internal_TickPnEIRAResetTime();
#endif
#if ((CANNM_PNC_ERA_CALC_ENABLED == STD_ON) && (CANNM_PNC_COUNT > 0))
    CanNm_Internal_TickPnEraResetTime();
#endif
}

/* @req CANNM344 */
void CanNm_ConfirmPnAvailability( const NetworkHandleType nmChannelHandle )
{
    /* @req CANNM345 */
    CANNM_VALIDATE_INIT_NORV(CANNM_SERVICEID_CONFIRMPNAVAILABILITY,0);


    CANNM_VALIDATE_NORV((nmChannelHandle< CanNm_ConfigPtr->ChannelLookupsSize), CANNM_SERVICEID_CONFIRMPNAVAILABILITY,CANNM_E_INVALID_CHANNEL,0);
    uint8 channelIndex = CanNm_ConfigPtr->ChannelLookups[nmChannelHandle];

    CanNm_Internal_ChannelType* ChannelInternal = &CanNm_Internal.Channels[channelIndex];
    const CanNm_ChannelType* ChannelConf = &CanNm_ConfigPtr->Channels[channelIndex];

    /* @req CANNM346 */
    /* @req CANNM404 */
    ChannelInternal->MessageFilteringEnabled = ChannelConf->CanNmPnEnable;
}

// Timer helpers
// -------------
#if ((CANNM_PNC_EIRA_CALC_ENABLED == STD_ON) && (CANNM_PNC_COUNT > 0))
/**
 * @brief reset EIRA bits in case of timeout
 * @param pnIndex - Indices of PNC which have timed out
 * @param indexCount - No of PNC that have timed out
 * @return void
 */
void CanNm_Internal_resetEIRAPNbits(uint8 *pnIndex, uint8 indexCount)
{
    uint8 byteNo;
    uint8 bit;
    uint8 idx;

    for (idx=0;idx<indexCount;idx++)
    {
        byteNo 	= (*(pnIndex+idx))/8;
        bit		= (*(pnIndex+idx))%8;
        /* @req CANNM431 */
        CanNm_Internal.pnEIRA.bytes[byteNo] &=  ~(1u<<bit); /* Reset PN bit */
    }
    PduInfoType pdu =
    {
            .SduDataPtr = &CanNm_Internal.pnEIRA.bytes[0],
            .SduLength = CanNm_ConfigPtr->CanNmPnInfo->CanNmPnInfoLen,
    };
    /* @req CANNM432 */
    PduR_CanNmRxIndication(CanNm_ConfigPtr->CanNmPnInfo->CanNmEIRARxNSduId,&pdu);

}

/**
 * @brief Run PN reset timers every main function cycle
 * @param void
 * @return void
 */
static inline void CanNm_Internal_TickPnEIRAResetTime(void)
{
    uint8 pncIndexReset[CANNM_PNC_COUNT]= {0};
    uint8 len;
    len = 0;
    uint8 timerIdx;
    SchM_Enter_CanNm_EA_0();

    for (timerIdx=0; timerIdx<CANNM_PNC_COUNT;timerIdx++) {
        if (CanNm_Internal.pnEIRATimers[timerIdx].timerRunning)
        {
            if (CANNM_MAIN_FUNCTION_PERIOD >= CanNm_Internal.pnEIRATimers[timerIdx].resetTimer)
            {
                CanNm_Internal.pnEIRATimers[timerIdx].timerRunning = FALSE;
                pncIndexReset[len] = CanNm_ConfigPtr->CanNmPnInfo->CanNmTimerIndexToPnMap[timerIdx];
                len++;
            } else {
                CanNm_Internal.pnEIRATimers[timerIdx].resetTimer -= CANNM_MAIN_FUNCTION_PERIOD;
            }
        } else {
            /* Do nothing */
        }
    }
    if (len> 0)
    {
        CanNm_Internal_resetEIRAPNbits(pncIndexReset,len);
    } else{
        /* Do nothing */
    }
    SchM_Exit_CanNm_EA_0();
}
#endif
#if ((CANNM_PNC_ERA_CALC_ENABLED == STD_ON) && (CANNM_PNC_COUNT > 0))
/**
 * @brief reset ERA bits in case of timeout
 * @param pnIndex - Indices of PNC which have timed out
 * @param indexCount - No of PNC that have timed out
 * @return void
 */
void CanNm_Internal_ResetEraPnBits(uint8 chanIndex, uint8 *pnIndex, uint8 indexCount)
{
    uint8 byteNo;
    uint8 bit;
    uint8 idx;

    for (idx = 0; idx < indexCount; idx++)
    {
        byteNo 	= (*(pnIndex + idx)) / 8;
        bit		= (*(pnIndex + idx)) % 8;
        /* @req CANNM442 */
        CanNm_Internal.Channels[chanIndex].pnERA.bytes[byteNo] &=  ~(1u << bit); /* Reset PN bit */
    }

    PduInfoType pdu =
    {
            .SduDataPtr = &CanNm_Internal.Channels[chanIndex].pnERA.bytes[0],
            .SduLength = CanNm_ConfigPtr->CanNmPnInfo->CanNmPnInfoLen,
    };

    /* @req CANNM443 */
    PduR_CanNmRxIndication(CanNm_ConfigPtr->Channels[chanIndex].CanNmERARxNSduId, &pdu);
}

/**
 * @brief Run PN reset timers every main function cycle
 * @param void
 * @return void
 */
static inline void CanNm_Internal_TickPnEraResetTime(void) {

    uint8 pncIndexReset[CANNM_PNC_COUNT]= {0};
    uint8 len;
    len = 0;
    uint8 chanIdx;
    uint8 timerIdx;

    SchM_Enter_CanNm_EA_0();

    for (chanIdx = 0; chanIdx < CANNM_CHANNEL_COUNT; chanIdx++) {

        if (CanNm_ConfigPtr->Channels[chanIdx].CanNmPnEraCalcEnabled) {

            for (timerIdx = 0; timerIdx < CANNM_PNC_COUNT; timerIdx++) {

                if (CanNm_Internal.Channels[chanIdx].pnERATimers[timerIdx].timerRunning) {

                    if (CANNM_MAIN_FUNCTION_PERIOD >= CanNm_Internal.Channels[chanIdx].pnERATimers[timerIdx].resetTimer) {
                        CanNm_Internal.Channels[chanIdx].pnERATimers[timerIdx].timerRunning = FALSE;
                        pncIndexReset[len] = CanNm_ConfigPtr->CanNmPnInfo->CanNmTimerIndexToPnMap[timerIdx];
                        len++;
                    } else {
                        CanNm_Internal.Channels[chanIdx].pnERATimers[timerIdx].resetTimer -= CANNM_MAIN_FUNCTION_PERIOD;
                    }
                }
            }

            if (len > 0) {

                CanNm_Internal_ResetEraPnBits(chanIdx, pncIndexReset, len);

                memset(pncIndexReset, 0, (size_t)CANNM_PNC_COUNT);
                len = 0;
            }
        }
    }

    SchM_Exit_CanNm_EA_0();
}
#endif

/**
 * @brief tick NM timeout time every main function cycle
 * @param ChannelConf channel configuration
 * @param ChannelInternal channel internal runtime data
 * @return void
 */
static inline void CanNm_Internal_TickTimeoutTime( const CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal ) {

    if (ChannelConf->MainFunctionPeriod >= ChannelInternal->TimeoutTimeLeft) {
        ChannelInternal->TimeoutTimeLeft = 0;
        if (ChannelInternal->State == NM_STATE_REPEAT_MESSAGE) {

#if (CANNM_DEV_ERROR_DETECT == STD_ON) /** @req CANNM193 */
            /* @req CANNM019 */
            CANNM_DET_REPORTERROR(CANNM_SERVICEID_MAINFUNCTION, CANNM_E_NETWORK_TIMEOUT, 0); /* invalid due to requirement CANNM236 => main function should report instance id = channelId BUT we don't have the information here*/
#endif

#if (CANNM_USER_DATA_ENABLED == STD_ON) && (CANNM_COM_USER_DATA_SUPPORT == STD_ON)
            /* reset confirmation flag in order for beeing able to send again*/
            ChannelInternal->TransmissionStatus = CANNM_NO_TRANSMISSION;
#endif

            CanNm_Internal_RepeatMessage_to_RepeatMessage(ChannelConf, ChannelInternal);  /**< @req CANNM101 */
        } else if (ChannelInternal->State == NM_STATE_NORMAL_OPERATION) {
#if (CANNM_DEV_ERROR_DETECT == STD_ON) /** @req CANNM194 */
            /* @req CANNM019 */
            CANNM_DET_REPORTERROR(CANNM_SERVICEID_MAINFUNCTION, CANNM_E_NETWORK_TIMEOUT, 0); /* invalid due to requirement CANNM236 => main function should report instance id = channelId BUT we don't have the information here*/
#endif

#if (CANNM_USER_DATA_ENABLED == STD_ON) && (CANNM_COM_USER_DATA_SUPPORT == STD_ON)
            /* reset confirmation flag in order for beeing able to send again*/
            ChannelInternal->TransmissionStatus = CANNM_NO_TRANSMISSION;
#endif

            CanNm_Internal_NormalOperation_to_NormalOperation(ChannelConf, ChannelInternal);  /**< @req CANNM117 */
        } else if (ChannelInternal->State == NM_STATE_READY_SLEEP) {
            CanNm_Internal_ReadySleep_to_PrepareBusSleep(ChannelConf, ChannelInternal);  /**< @req CANNM109 */
        } else {
            //Nothing to be done
        }
    } else {
        ChannelInternal->TimeoutTimeLeft -= ChannelConf->MainFunctionPeriod;
    }
}

/**
 * @brief tick repeat message time duration
 * @param ChannelConf channel configuration
 * @param ChannelInternal channel internal runtime data
 * @return void
 */
/** @req CANNM102 */
static inline void CanNm_Internal_TickRepeatMessageTime( const CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal ) {
    if (ChannelConf->MainFunctionPeriod >= ChannelInternal->RepeatMessageTimeLeft) {
        ChannelInternal->RepeatMessageTimeLeft = 0;
        if (ChannelInternal->State == NM_STATE_REPEAT_MESSAGE) {
            if (ChannelInternal->Requested) {
                CanNm_Internal_RepeatMessage_to_NormalOperation(ChannelConf, ChannelInternal);  /** @req CANNM103 */
            } else {
                CanNm_Internal_RepeatMessage_to_ReadySleep(ChannelConf, ChannelInternal); /** @req CANNM106 */
            }
        }
    } else {
        ChannelInternal->RepeatMessageTimeLeft -= ChannelConf->MainFunctionPeriod;
    }
}

/**
 * @brief tick wait bus sleep time duration
 * @param ChannelConf channel configuration
 * @param ChannelInternal channel internal runtime data
 * @return void
 */
/** @req CANNM115 */
static inline void CanNm_Internal_TickWaitBusSleepTime( const CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal ) {
    if (ChannelConf->MainFunctionPeriod >= ChannelInternal->WaitBusSleepTimeLeft) {
        ChannelInternal->WaitBusSleepTimeLeft = 0;
        if (ChannelInternal->Mode == NM_MODE_PREPARE_BUS_SLEEP) {
            CanNm_Internal_PrepareBusSleep_to_BusSleep(ChannelConf, ChannelInternal); /** @req CANNM088 */ /** @req CANNM115 */
        }
    } else {
        ChannelInternal->WaitBusSleepTimeLeft -= ChannelConf->MainFunctionPeriod;
    }
}

/**
 * @brief tick timeout for Tx confirmation
 * @param ChannelConf channel configuration
 * @param ChannelInternal channel internal runtime data
 * @return void
 */
/* TxTimeout Processing */
static inline void CanNm_Internal_TickTxTimeout( const CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal ) {

    if (0 != ChannelInternal->MessageTimeoutTimeLeft ) {
        if (ChannelConf->MainFunctionPeriod >= ChannelInternal->MessageTimeoutTimeLeft ) {
            /** @req CANNM066 **/
            Nm_TxTimeoutException(ChannelConf->NmNetworkHandle);
            ChannelInternal->MessageTimeoutTimeLeft = 0;
        } else {
            ChannelInternal->MessageTimeoutTimeLeft -= ChannelConf->MainFunctionPeriod;
        }
    }

}

/**
 * @brief tick message cycle time
 * @param ChannelConf channel configuration
 * @param ChannelInternal channel internal runtime data
 * @return void
 */
static inline void CanNm_Internal_TickMessageCycleTime( const CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal ) {

    if (ChannelConf->MainFunctionPeriod >= ChannelInternal->MessageCycleTimeLeft) {

            if (ChannelInternal->immediateModeActive == TRUE) {
                ChannelInternal->MessageCycleTimeLeft = ChannelConf->ImmediateNmCycleTime; /** @req CANNM334 */
            } else {
                ChannelInternal->MessageCycleTimeLeft = ChannelConf->MessageCycleTime;  /** @req CANNM040 */
            }

            /** @req CANNM087  @req CANNM100 */
            CanNm_Internal_TransmitMessage(ChannelConf, ChannelInternal); /** CANNM032 */ /* should transmit independently from state?! (CANNM032) */

    } else {
        ChannelInternal->MessageCycleTimeLeft -= ChannelConf->MainFunctionPeriod;
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
static inline void CanNm_Internal_TransmitMessage( const CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal ) {

    boolean internal_status;
    internal_status = TRUE;
    PduInfoType pdu = {
            .SduDataPtr = ChannelInternal->TxMessageSdu,
            .SduLength = ChannelConf->PduLength,
    };

    if (!ChannelInternal->CommunicationEnabled) {
        internal_status = FALSE;
    }
    if (internal_status == TRUE){
#if (CANNM_USER_DATA_ENABLED == STD_ON)
#if (CANNM_COM_USER_DATA_SUPPORT == STD_ON)

        SchM_Enter_CanNm_EA_0();

        if (ChannelInternal->TransmissionStatus == CANNM_NO_TRANSMISSION) {
            PduInfoType userData;
            ChannelInternal->TransmissionStatus = CANNM_ONGOING_TRANSMISSION;

            SchM_Exit_CanNm_EA_0();

            userData.SduDataPtr = CanNm_Internal_GetUserDataPtr(ChannelConf, ChannelInternal->TxMessageSdu);
            userData.SduLength = CanNm_Internal_GetUserDataLength(ChannelConf);
            /* IMPROVMENT: Add Det error when transmit is failing */
            (void)PduR_CanNmTriggerTransmit(ChannelConf->CanNmUserDataTxPduId, &userData); /** @req CANNM328 */

#if ((CANNM_PNC_EIRA_CALC_ENABLED == STD_ON) && (CANNM_PNC_COUNT > 0))
            if (ChannelConf->CanNmPnEnable) {
                CanNm_Internal_ProcessTxPdu(pdu.SduDataPtr+CanNm_ConfigPtr->CanNmPnInfo->CanNmPnInfoOffset);
            }
#endif
#else
        SchM_Enter_CanNm_EA_0();

        if (!ChannelInternal->IsUserDataSet) {

            ChannelInternal->IsUserDataSet = TRUE;

            SchM_Exit_CanNm_EA_0();
#endif
#endif
            ChannelInternal->MessageTimeoutTimeLeft = ChannelConf->MessageTimeoutTime; /** @req CANNM064 */

            // NOTE: what to do if Transmit fails?
            Std_ReturnType status = CanIf_Transmit(ChannelConf->CanIfPduId, &pdu);

            if (status != E_OK) {
#ifdef HOST_TEST
                ReportErrorStatus();
#endif
            }

            /* check if we are sending immediateMessages */
            if (ChannelInternal->immediateModeActive == TRUE) {
                /* increment the immediateNmTransmissionsSent counter */
                ChannelInternal->immediateNmTransmissionsSent++;

                /* check if we already have reached the amount of messages which shall be send via immediateTransmit */
                if (ChannelInternal->immediateNmTransmissionsSent == ChannelConf->ImmediateNmTransmissions) {
                    /* if so then deactivate the immediate mode again */
                    ChannelInternal->immediateModeActive = FALSE;

                    /* and wait offset before sending next regular (not immediate) pdu */
                    if (0 != ChannelConf->MessageCycleOffsetTime) {
                        ChannelInternal->MessageCycleTimeLeft = ChannelConf->MessageCycleOffsetTime; /** @req CANNM335 */
                    } else {
                        /* Offset is zero, wait message cycle time. */
                        ChannelInternal->MessageCycleTimeLeft = ChannelConf->MessageCycleTime;
                    }
                }
            }
#if (CANNM_USER_DATA_ENABLED == STD_ON)
#if (CANNM_COM_USER_DATA_SUPPORT == STD_ON)

            /* set back the flag directly => this won't change the previous behaviour */
            /* flag is only needed to decide whether the last successfully sent message was a spontaneous transmit or a cyclic message */
            /* needed in function TxConfirmation */

            ChannelInternal->TransmissionStatus = CANNM_NO_TRANSMISSION; /* no need to lock */
        } else {
            SchM_Exit_CanNm_EA_0();
        }
#else
            ChannelInternal->IsUserDataSet = FALSE;
        } else {
            SchM_Exit_CanNm_EA_0();
        }
#endif
#endif
    }
}

/**
 * @brief Get user data offset byte index in NM Pdu
 * @param ChannelConf channel configuration
 * @return User starting data byte index in PDU
 */
static inline PduLengthType CanNm_Internal_GetUserDataOffset( const CanNm_ChannelType* ChannelConf ) {
    PduLengthType userDataPos = 0;
    userDataPos += (ChannelConf->NidPosition == CANNM_PDU_OFF) ? 0 : 1;
    userDataPos += (ChannelConf->CbvPosition == CANNM_PDU_OFF) ? 0 : 1;
    return userDataPos;
}

/**
 * @brief get the ptr to user data location in Nm Pdu
 * @param ChannelConf
 * @param MessageSduPtr
 * @return pointer to user data bytes in PDU
 */
static inline uint8* CanNm_Internal_GetUserDataPtr( const CanNm_ChannelType* ChannelConf, uint8* MessageSduPtr ) {
    PduLengthType userDataOffset = CanNm_Internal_GetUserDataOffset(ChannelConf);
    return &MessageSduPtr[userDataOffset];
}

/**
 * @brief get user data length
 * @param ChannelConf
 * @return user data length
 */
static inline PduLengthType CanNm_Internal_GetUserDataLength( const CanNm_ChannelType* ChannelConf ) {
    PduLengthType userDataOffset = CanNm_Internal_GetUserDataOffset(ChannelConf);
    return ChannelConf->PduLength - userDataOffset;
}

/**
 * @brief clear repeat message request indication in CBV byte of Nm Pdu
 * @param ChannelConf
 * @param ChannelInternal
 * @return void
 */
static inline void CanNm_Internal_ClearRptMsgRqstCbv( const CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal ) {
    if (ChannelConf->CbvPosition != CANNM_PDU_OFF) {
        ChannelInternal->TxMessageSdu[ChannelConf->CbvPosition] &= ~(CANNM_CBV_REPEAT_MESSAGE_REQUEST);
#if (CANNM_USER_DATA_ENABLED == STD_ON) && (CANNM_COM_USER_DATA_SUPPORT == STD_ON)
        ChannelInternal->SpontaneousTxMessageSdu[ChannelConf->CbvPosition] &= ~(CANNM_CBV_REPEAT_MESSAGE_REQUEST);
#endif
    }
}

/**
 * @brief set the PNI bit in CBV byte of Nm Pdu
 * @param ChannelConf - channel configuration
 * @param ChannelInternal - channel internal runtime data
 * @return void
 *
 */
#if (CANNM_PNC_COUNT > 0)
static inline void CanNm_Internal_SetPNICbv( const CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal ) {

    if (ChannelConf->CbvPosition != CANNM_PDU_OFF) {

        ChannelInternal->TxMessageSdu[ChannelConf->CbvPosition] |= CANNM_CBV_PNI;
#if (CANNM_USER_DATA_ENABLED == STD_ON) && (CANNM_COM_USER_DATA_SUPPORT == STD_ON)
        ChannelInternal->SpontaneousTxMessageSdu[ChannelConf->CbvPosition] |= CANNM_CBV_PNI;
#endif
    }
}
#endif
// Transition helpers
// ------------------
/**
 * @brief transit from prepare bus sleep to repeat message state
 * @param ChannelConf
 * @param ChannelInternal
 * @param isNwReq
 * @return void
 */
static inline void CanNm_Internal_PrepareBusSleep_to_RepeatMessage( const CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal ,boolean isNwReq ) {
    ChannelInternal->Mode = NM_MODE_NETWORK;
    ChannelInternal->State = NM_STATE_REPEAT_MESSAGE;

    if ((ChannelConf->ImmediateNmTransmissions > 0) && isNwReq) /** @req CANNM005 CANNM334 */
    {
        ChannelInternal->MessageCycleTimeLeft = 0; /* begin transmission immediately */ /** @req CANNM334 */
        ChannelInternal->immediateNmTransmissionsSent = 0; /* reset counter of sent immediate messages */
        ChannelInternal->immediateModeActive = TRUE; /* activate immediate-transmission-mode */
    }
    else {
        ChannelInternal->MessageCycleTimeLeft = ChannelConf->MessageCycleOffsetTime + ChannelConf->MainFunctionPeriod;  /**< @req CANNM100 */
    }

    ChannelInternal->RepeatMessageTimeLeft = ChannelConf->RepeatMessageTime + ChannelConf->MainFunctionPeriod;
    ChannelInternal->TimeoutTimeLeft = ChannelConf->TimeoutTime + ChannelConf->MainFunctionPeriod;  /**< @req CANNM096 */

    // Notify 'Network Mode'
    Nm_NetworkMode(ChannelConf->NmNetworkHandle);  /**< @req CANNM097 */
    /**< @req CANNM166 */
#if (CANNM_STATE_CHANGE_IND_ENABLED == STD_ON)
    Nm_StateChangeNotification(ChannelConf->NmNetworkHandle, NM_STATE_PREPARE_BUS_SLEEP, NM_STATE_REPEAT_MESSAGE);
#endif

}

/**
 * @brief transit from prepare bus sleep to bus sleep state
 * @param ChannelConf
 * @param ChannelInternal
 * @return void
 */
static inline void CanNm_Internal_PrepareBusSleep_to_BusSleep( const CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal ) {
	SchM_Enter_CanNm_EA_0();
    ChannelInternal->Mode = NM_MODE_BUS_SLEEP;
    ChannelInternal->State = NM_STATE_BUS_SLEEP;
    // Notify 'Bus-Sleep Mode'
    Nm_BusSleepMode(ChannelConf->NmNetworkHandle);  /**< @req CANNM126 */ /** @req CANNM324 */
    /**< @req CANNM166 */
#if (CANNM_STATE_CHANGE_IND_ENABLED == STD_ON)
    Nm_StateChangeNotification(ChannelConf->NmNetworkHandle, NM_STATE_PREPARE_BUS_SLEEP, NM_STATE_BUS_SLEEP);
#endif
	SchM_Exit_CanNm_EA_0();
}

/**
 * @brief transit from bus sleep to repeat message
 * @param ChannelConf
 * @param ChannelInternal
 * @param isNwReq
 * @return void
 */
static inline void CanNm_Internal_BusSleep_to_RepeatMessage( const CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal,boolean isNwReq ) {
    ChannelInternal->Mode = NM_MODE_NETWORK;
    ChannelInternal->State = NM_STATE_REPEAT_MESSAGE;

    if ((ChannelConf->ImmediateNmTransmissions > 0) && isNwReq) /** @req CANNM005 */
    {
        ChannelInternal->MessageCycleTimeLeft = 0; /* begin transmission immediately */ /** @req CANNM334 */
        ChannelInternal->immediateNmTransmissionsSent = 0; /* reset counter of sent immediate messages */
        ChannelInternal->immediateModeActive = TRUE; /* activate immediate-transmission-mode */
    } else {
        ChannelInternal->MessageCycleTimeLeft = ChannelConf->MessageCycleOffsetTime + ChannelConf->MainFunctionPeriod;  /**< @req CANNM100 */
    }

    ChannelInternal->TimeoutTimeLeft = ChannelConf->TimeoutTime + ChannelConf->MainFunctionPeriod ;  /**< @req CANNM096 */
    ChannelInternal->RepeatMessageTimeLeft = ChannelConf->RepeatMessageTime + ChannelConf->MainFunctionPeriod;

    // Notify 'Network Mode'
    Nm_NetworkMode(ChannelConf->NmNetworkHandle);  /**< @req CANNM097 */ /** @req CANNM324 */
    /**< @req CANNM166 */
#if (CANNM_STATE_CHANGE_IND_ENABLED == STD_ON)
    Nm_StateChangeNotification(ChannelConf->NmNetworkHandle, NM_STATE_BUS_SLEEP, NM_STATE_REPEAT_MESSAGE);
#endif

}
/**
 * @brief transit from bus sleep to bus sleep
 * @param ChannelConf
 * @param ChannelInternal
 * @return void
 */
static inline void CanNm_Internal_BusSleep_to_BusSleep( const CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal ) {
    // Notify 'Network Start'
#if  (CANNM_PNC_COUNT == 0)
    Nm_NetworkStartIndication(ChannelConf->NmNetworkHandle);  /**< @req CANNM127 */ /** @req CANNM324 */
#else
    //lint -estring(920,pointer)  /* cast to void */
    (void) ChannelConf; //Just to avoid 715 PC-Lint warning about not used.
    //lint +estring(920,pointer)  /* cast to void */
#endif
    //lint -estring(920,pointer)  /* cast to void */
    (void) ChannelInternal; //Just to avoid 715 PC-Lint warning about not used.
    //lint +estring(920,pointer)  /* cast to void */
}

/**
 * @brief transit from repeat message to repeat message
 * @param ChannelConf
 * @param ChannelInternal
 * @return void
 */
static inline void CanNm_Internal_RepeatMessage_to_RepeatMessage( const CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal ) {
    ChannelInternal->TimeoutTimeLeft = ChannelConf->TimeoutTime;  /**< @req CANNM101 */
}

/**
 * @brief transit from repeat message state to ready sleep
 * @param ChannelConf
 * @param ChannelInternal
 * @return void
 */
static inline void CanNm_Internal_RepeatMessage_to_ReadySleep( const CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal ) {
    ChannelInternal->Mode = NM_MODE_NETWORK;
    ChannelInternal->State = NM_STATE_READY_SLEEP;
#if (CANNM_NODE_DETECTION_ENABLED == STD_ON)
    /** @req CANNM107 */
    CanNm_Internal_ClearRptMsgRqstCbv(ChannelConf, ChannelInternal);
#endif
    /**< @req CANNM166 */
#if (CANNM_STATE_CHANGE_IND_ENABLED == STD_ON)
    Nm_StateChangeNotification(ChannelConf->NmNetworkHandle, NM_STATE_REPEAT_MESSAGE, NM_STATE_READY_SLEEP);
#endif
}

/**
 * @brief transit from repeat message to normal operation state
 * @param ChannelConf
 * @param ChannelInternal
 * @return void
 */
static inline void CanNm_Internal_RepeatMessage_to_NormalOperation( const CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal ) {
    ChannelInternal->Mode = NM_MODE_NETWORK;
    ChannelInternal->State = NM_STATE_NORMAL_OPERATION;
#if (CANNM_NODE_DETECTION_ENABLED == STD_ON)
    /** @req CANNM107 */
    CanNm_Internal_ClearRptMsgRqstCbv(ChannelConf, ChannelInternal);
#endif
    /**< @req CANNM166 */
#if (CANNM_STATE_CHANGE_IND_ENABLED == STD_ON)
    Nm_StateChangeNotification(ChannelConf->NmNetworkHandle, NM_STATE_REPEAT_MESSAGE, NM_STATE_NORMAL_OPERATION);
#endif
}

/**
 * @brief transit from normal operation to repeat message state
 * @param ChannelConf
 * @param ChannelInternal
 * @return void
 */
static inline void CanNm_Internal_NormalOperation_to_RepeatMessage( const CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal ) {
    ChannelInternal->Mode = NM_MODE_NETWORK;
    ChannelInternal->State = NM_STATE_REPEAT_MESSAGE;
    ChannelInternal->RepeatMessageTimeLeft = ChannelConf->RepeatMessageTime + ChannelConf->MainFunctionPeriod;
    ChannelInternal->MessageCycleTimeLeft = ChannelConf->MessageCycleOffsetTime + ChannelConf->MainFunctionPeriod;  /**< @req CANNM100 */
    /**< @req CANNM166 */
#if (CANNM_STATE_CHANGE_IND_ENABLED == STD_ON)
    Nm_StateChangeNotification(ChannelConf->NmNetworkHandle, NM_STATE_NORMAL_OPERATION, NM_STATE_REPEAT_MESSAGE);
#endif
}

/**
 * @brief transition from normal operation to ready sleep
 * @param ChannelConf
 * @param ChannelInternal
 * @return void
 */
static inline void CanNm_Internal_NormalOperation_to_ReadySleep( const CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal ) {
    ChannelInternal->Mode = NM_MODE_NETWORK;
    ChannelInternal->State = NM_STATE_READY_SLEEP;
    //lint -estring(920,pointer)  /* cast to void */
    (void) ChannelConf; //Just to avoid 715 PC-Lint warning about not used.
    //lint +estring(920,pointer)  /* cast to void */
    /**< @req CANNM166 */
#if (CANNM_STATE_CHANGE_IND_ENABLED == STD_ON)
    Nm_StateChangeNotification(ChannelConf->NmNetworkHandle, NM_STATE_NORMAL_OPERATION, NM_STATE_READY_SLEEP);
#endif
}

/**
 * @brief transition from normal operation to normal operation
 * @param ChannelConf
 * @param ChannelInternal
 * @return void
 */
static inline void CanNm_Internal_NormalOperation_to_NormalOperation( const CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal ) {
    ChannelInternal->TimeoutTimeLeft = ChannelConf->TimeoutTime;  /**< @req CANNM117 */
}

/**
 * @brief transition from ready sleep to normal operation
 * @param ChannelConf
 * @param ChannelInternal
 * @return void
 */
static inline void CanNm_Internal_ReadySleep_to_NormalOperation( const CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal ) {
    ChannelInternal->Mode = NM_MODE_NETWORK;
    ChannelInternal->State = NM_STATE_NORMAL_OPERATION;
    ChannelInternal->MessageCycleTimeLeft = ChannelConf->MessageCycleOffsetTime + ChannelConf->MainFunctionPeriod;  /**< @req CANNM116 */
    /**< @req CANNM166 */
#if (CANNM_STATE_CHANGE_IND_ENABLED == STD_ON)
    Nm_StateChangeNotification(ChannelConf->NmNetworkHandle, NM_STATE_READY_SLEEP, NM_STATE_NORMAL_OPERATION);
#endif
}
/**
 * @brief transition from ready sleep to bus prepare bus sleep state
 * @param ChannelConf
 * @param ChannelInternal
 * @return void
 */
static inline void CanNm_Internal_ReadySleep_to_PrepareBusSleep( const CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal ) {
	SchM_Enter_CanNm_EA_0();
    ChannelInternal->Mode = NM_MODE_PREPARE_BUS_SLEEP;
    ChannelInternal->State = NM_STATE_PREPARE_BUS_SLEEP;
    ChannelInternal->WaitBusSleepTimeLeft = ChannelConf->WaitBusSleepTime;
    ChannelInternal->MessageTimeoutTimeLeft = 0;/* Disable tx timeout timer */
    // Notify 'Prepare Bus-Sleep Mode'
    Nm_PrepareBusSleepMode(ChannelConf->NmNetworkHandle);  /**< @req CANNM114 */ /** @req CANNM324 */
    /**< @req CANNM166 */
#if (CANNM_STATE_CHANGE_IND_ENABLED == STD_ON)
    Nm_StateChangeNotification(ChannelConf->NmNetworkHandle, NM_STATE_READY_SLEEP, NM_STATE_PREPARE_BUS_SLEEP);
#endif
	SchM_Exit_CanNm_EA_0();
}

/**
 * @brief transition from ready sleep to repeat message state
 * @param ChannelConf
 * @param ChannelInternal
 * @return void
 */
static inline void CanNm_Internal_ReadySleep_to_RepeatMessage( const CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal ) {
    ChannelInternal->Mode = NM_MODE_NETWORK;
    ChannelInternal->State = NM_STATE_REPEAT_MESSAGE;
    ChannelInternal->RepeatMessageTimeLeft = ChannelConf->RepeatMessageTime + ChannelConf->MainFunctionPeriod;
    ChannelInternal->MessageCycleTimeLeft = ChannelConf->MessageCycleOffsetTime + ChannelConf->MainFunctionPeriod;  /**< @req CANNM100 */
    /**< @req CANNM166 */
#if (CANNM_STATE_CHANGE_IND_ENABLED == STD_ON)
    Nm_StateChangeNotification(ChannelConf->NmNetworkHandle, NM_STATE_READY_SLEEP,	NM_STATE_REPEAT_MESSAGE);
#endif
}

/**
 * @brief transition from Nw mode to Nw mode
 * @param ChannelConf
 * @param ChannelInternal
 * @return void
 */
static inline void CanNm_Internal_NetworkMode_to_NetworkMode( const CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal ) {
    ChannelInternal->TimeoutTimeLeft = ChannelConf->TimeoutTime + ChannelConf->MainFunctionPeriod;  /**< @req CANNM098 @req CANNM099 */

}

#if ((CANNM_PNC_EIRA_CALC_ENABLED == STD_ON) && (CANNM_PNC_COUNT > 0))
static inline void restartPnEiraTimer(uint8 pnIndex);
static void restartTimerForInternalExternalRequests(CanNm_Internal_RAType *calcEIRA);
/**
 * @brief restart PN timers
 * @param pnIndex - Index of PNC
 * @return void
 */
static inline void restartPnEiraTimer(uint8 pnIndex) {

    uint8 timerIndex = CanNm_ConfigPtr->CanNmPnInfo->CanNmPnIndexToTimerMap[pnIndex];

    CanNm_Internal.pnEIRATimers[timerIndex].timerRunning = TRUE;
    CanNm_Internal.pnEIRATimers[timerIndex].resetTimer = CANNM_PNC_RESET_TIME;

}

/**
 * @brief Identify the PN timer that needs to be restarted
 * @param calcEIRA - EIRA of the new received/transmitted and filtered PDU
 * @return void
 */
static void restartTimerForInternalExternalRequests(CanNm_Internal_RAType *calcEIRA) {

    uint8 byteNo;
    uint8 bit;
    uint8 byteEIRA;

    for (byteNo = 0; byteNo < CanNm_ConfigPtr->CanNmPnInfo->CanNmPnInfoLen; byteNo++) {

        byteEIRA = calcEIRA->bytes[byteNo];

        if ( byteEIRA > 0) {

            for (bit = 0; bit < 8; bit++) {

                if ((byteEIRA >> bit) & CANNM_LSBIT_MASK) {

                    restartPnEiraTimer((byteNo * 8) + bit);
                } else {
                    /* Do nothing */
                }
            }
        } else {
            /* Do nothing */
        }
    }
}
#endif
#if ((CANNM_PNC_ERA_CALC_ENABLED == STD_ON) && (CANNM_PNC_COUNT > 0))
static inline void restartPnEraTimer(CanNm_Internal_ChannelType* ChannelInternal, uint8 pnIndex);
void restartTimerForExternalRequests(CanNm_Internal_ChannelType* ChannelInternal,
        CanNm_Internal_RAType *calcEIRA);
/**
 * @brief restart PN timers
 * @param pnIndex - Index of PNC
 * @return void
 */
static inline void restartPnEraTimer(CanNm_Internal_ChannelType* ChannelInternal, uint8 pnIndex) {

    uint8 timerIndex = CanNm_ConfigPtr->CanNmPnInfo->CanNmPnIndexToTimerMap[pnIndex];

    ChannelInternal->pnERATimers[timerIndex].timerRunning = TRUE;
    ChannelInternal->pnERATimers[timerIndex].resetTimer = CANNM_PNC_RESET_TIME;
}

/**
 * @brief Identify the PN timer that needs to be restarted
 * @param calcERA - ERA of the new received/transmitted and filtered PDU
 * @return void
 */
void restartTimerForExternalRequests(CanNm_Internal_ChannelType* ChannelInternal,
        CanNm_Internal_RAType *calcERA) {

    uint8 byteNo;
    uint8 bit;
    uint8 byteERA;

    for (byteNo = 0; byteNo < CanNm_ConfigPtr->CanNmPnInfo->CanNmPnInfoLen; byteNo++) {

        byteERA = calcERA->bytes[byteNo];

        if ( byteERA > 0) {

            for(bit = 0; bit < 8; bit++) {

                if ((byteERA >> bit) & CANNM_LSBIT_MASK) {
                    restartPnEraTimer(ChannelInternal, (byteNo * 8) + bit);
                } else {
                    /* Do nothing */
                }
            }
        } else {
            /* Do nothing */
        }
    }
}
#endif

/**
 * @brief NM filtering process done for each reception
 * @param ChannelConf - Channel configuration
 * @param ChannelInternal - Channel internal runtime data
 * @param pni - PNI bit set in CBV?
 * @return reception valid or not
 *
 */
#if (CANNM_PNC_COUNT > 0)
static inline Std_ReturnType CanNm_Internal_RxProcess(const CanNm_ChannelType* ChannelConf, CanNm_Internal_ChannelType* ChannelInternal, boolean pni ) {

    Std_ReturnType ret;
    ret = E_OK;
    uint8 i;
    uint8 offset;
    CanNm_Internal_RAType calculatedEIRA = {0};
#if (CANNM_PNC_ERA_CALC_ENABLED == STD_ON)
    CanNm_Internal_RAType calculatedERA = {0};
#endif /* (CANNM_PNC_ERA_CALC_ENABLED == STD_ON) */

    boolean isUpdated;
    isUpdated = TRUE;
    /* @req CANNM409 */
    if (!ChannelConf->CanNmPnEnable) {
        ret = E_OK; /* No pn normal reception */

    }
    /* @req CANNM410 */
    else if ((ChannelConf->CanNmAllNmMsgKeepAwake) && (!pni)) {
        ret = E_OK; /* No pni in cbv -  normal reception */
    }
    /* @req CANNM411 */
    else if ((!ChannelConf->CanNmAllNmMsgKeepAwake) && (!pni)) {
        ret = E_NOT_OK; /* No pni in cbv - discard reception */
    } else if (!ChannelInternal->MessageFilteringEnabled) {
        ret = E_NOT_OK; /* pni set in cbv, but filter is disabled - discard reception */
    } else {
        /* @req CANNM412 */ /* Do nothing - NM filtering done below*/
        isUpdated = FALSE;
    }
    if (!isUpdated) {
        /* @req CANNM415 */
        /* @req CANNM418 */
        offset = CanNm_ConfigPtr->CanNmPnInfo->CanNmPnInfoOffset;
        for (i = 0; i < CanNm_ConfigPtr->CanNmPnInfo->CanNmPnInfoLen; i++) {
            /* Accumulate external requests */
            calculatedEIRA.bytes[i] = ChannelInternal->RxMessageSdu[i + offset] & CanNm_ConfigPtr->CanNmPnInfo->CanNmPnInfoMask[i];
        }
#if (CANNM_PNC_ERA_CALC_ENABLED == STD_ON)
        calculatedERA.data = calculatedEIRA.data;
#endif /* (CANNM_PNC_ERA_CALC_ENABLED == STD_ON) */
        /* EIRA == ERA here so only need to check one. */
        if (calculatedEIRA.data == 0) {
            if (!ChannelConf->CanNmAllNmMsgKeepAwake) {
                ret = E_NOT_OK;     /* @req CANNM420 */
                isUpdated = TRUE;
            } else {
                ret = E_OK; /* @req CANNM421 */
                isUpdated = TRUE;
            }
        } else {
            /* @req CANNM419 */ /* Do nothing */ /* Process as below */
        }
        if (!isUpdated) {
            /* @req CANNM422 */
#if (CANNM_PNC_EIRA_CALC_ENABLED == STD_ON)
            CanNm_Internal_RAType EIRAOld = {0};
            boolean changed;

            SchM_Enter_CanNm_EA_0();
            /* @req CANNM429 */
            restartTimerForInternalExternalRequests(&calculatedEIRA);
            /* @req CANNM423 */
            /* @req CANNM425 */
            /* @req CANNM426 */
            EIRAOld.data = CanNm_Internal.pnEIRA.data;
            CanNm_Internal.pnEIRA.data |= calculatedEIRA.data;
            changed = (EIRAOld.data != CanNm_Internal.pnEIRA.data) ? TRUE: FALSE;

            SchM_Exit_CanNm_EA_0();

            if (changed) {
                PduInfoType pdu = {
                .SduDataPtr = &CanNm_Internal.pnEIRA.bytes[0],
                .SduLength = CanNm_ConfigPtr->CanNmPnInfo->CanNmPnInfoLen,
                };
                /* @req CANNM432 */
                PduR_CanNmRxIndication(CanNm_ConfigPtr->CanNmPnInfo->CanNmEIRARxNSduId,&pdu);
            } else {
                /* Do nothing */
            }

#endif /* (CANNM_PNC_EIRA_CALC_ENABLED == STD_ON) */
            /* @req CANNM433 */
#if (CANNM_PNC_ERA_CALC_ENABLED == STD_ON)
            if (ChannelConf->CanNmPnEraCalcEnabled) {
                CanNm_Internal_RAType ERAOld = {0};
                boolean changedERA;

                SchM_Enter_CanNm_EA_0();
                /* @req CANNM439 */
                restartTimerForExternalRequests(ChannelInternal, &calculatedERA);
                /* @req CANNM434 */
                /* @req CANNM436 */
                /* @req CANNM437 */
                ERAOld.data = ChannelInternal->pnERA.data;
                ChannelInternal->pnERA.data |= calculatedERA.data;
                changedERA = (ERAOld.data != ChannelInternal->pnERA.data) ? TRUE: FALSE;

                SchM_Exit_CanNm_EA_0();

                if (changedERA) {
                    PduInfoType pdu = {
                    .SduDataPtr = &ChannelInternal->pnERA.bytes[0],
                    .SduLength = CanNm_ConfigPtr->CanNmPnInfo->CanNmPnInfoLen,
                    };
                    /* @req CANNM443 */
                    PduR_CanNmRxIndication(ChannelConf->CanNmERARxNSduId, &pdu);
                } else {
                    /* Do nothing */
                }
            }
#endif /* (CANNM_PNC_ERA_CALC_ENABLED == STD_ON) */
        }
    }
    return ret;
}
#endif

/**
 * @brief Determine internal requests from ComM and identify which PN timer has to be started
 * @param pnInfo - Pn Info range of the transmitted PDU
 * @return void
 */
#if ((CANNM_PNC_EIRA_CALC_ENABLED == STD_ON) && (CANNM_PNC_COUNT > 0))
void CanNm_Internal_ProcessTxPdu(uint8 *pnInfo) {

    uint8 byteNo;
    CanNm_Internal_RAType calculatedEIRA = {0};
    CanNm_Internal_RAType EIRAOld = {0};
    boolean changed;

    SchM_Enter_CanNm_EA_0();

    for (byteNo = 0; byteNo < CanNm_ConfigPtr->CanNmPnInfo->CanNmPnInfoLen; byteNo++) {
        calculatedEIRA.bytes[byteNo] = *(pnInfo + byteNo) & CanNm_ConfigPtr->CanNmPnInfo->CanNmPnInfoMask[byteNo]; /* Accumulate internal requests */
    }

    if (calculatedEIRA.data != 0) {

        /* @req CANNM430 */
        restartTimerForInternalExternalRequests(&calculatedEIRA);
        EIRAOld.data = CanNm_Internal.pnEIRA.data;
        CanNm_Internal.pnEIRA.data |= calculatedEIRA.data; /* @req CANNM427 */
        changed = (EIRAOld.data != CanNm_Internal.pnEIRA.data) ? TRUE: FALSE;

        if (changed) {

            PduInfoType pdu = {
                .SduDataPtr = &CanNm_Internal.pnEIRA.bytes[0],
                .SduLength = CanNm_ConfigPtr->CanNmPnInfo->CanNmPnInfoLen,
            };
            /* @req CANNM432 */
            PduR_CanNmRxIndication(CanNm_ConfigPtr->CanNmPnInfo->CanNmEIRARxNSduId,&pdu);
        } else {
            /* Do nothing */
        }
    } else {
        /* Do nothing */
    }
    SchM_Exit_CanNm_EA_0();
}

#endif
