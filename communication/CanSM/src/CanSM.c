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


//#include "CanSM_Cfg.h"          /* @req CANSM254 *//* CanSM.h includes CanSM_Cfg.h */
#include "ComStack_Types.h"
#include "CanSM.h"              /* @req CANSM013 */
#include "ComM.h"               /* @req CANSM174 */
#include "ComM_BusSM.h"         /* @req CANSM191 */
#include "Det.h"                /* @req CANSM015 */
#if defined(USE_DEM)
#include "Dem.h"                /* @req CANSM014 */
#endif
#include "CanIf.h"              /* @req CANSM017 */
#include "CanSM_Internal.h"
#include "CanSM_BswM.h"			/* @req CANSM348 */
#include "CanSM_Cbk.h"
#if defined(USE_BSWM)
#include "BswM_CanSM.h"
#endif

#if defined(USE_CANSM_EXTENSION)
#include "CanSM_Extension.h"
#endif

#define INVALID_CONTROLLER_ID 0xff

#define CANSM_START_SEC_VAR_CLEARED_UNSPECIFIED
#include "CanSM_MemMap.h"  /*lint !e9019 OTHER [MISRA 2012 Rule 20.1, advisory] OTHER AUTOSAR specified way of using MemMap*/
static CanSM_Internal_CtrlStatusType CanSM_InternalControllerStatus[NOF_CANSM_CANIF_CONTROLLERS];
#define CANSM_STOP_SEC_VAR_CLEARED_UNSPECIFIED
#include "CanSM_MemMap.h"  /*lint !e9019 OTHER [MISRA 2012 Rule 20.1, advisory] OTHER AUTOSAR specified way of using MemMap*/

#define CANSM_START_SEC_VAR_CLEARED_UNSPECIFIED
#include "CanSM_MemMap.h"  /*lint !e9019 OTHER [MISRA 2012 Rule 20.1, advisory] OTHER AUTOSAR specified way of using MemMap*/
static CanSM_Internal_NetworkType CanSM_InternalNetworks[CANSM_NETWORK_COUNT];
#define CANSM_STOP_SEC_VAR_CLEARED_UNSPECIFIED
#include "CanSM_MemMap.h"  /*lint !e9019 OTHER [MISRA 2012 Rule 20.1, advisory] OTHER AUTOSAR specified way of using MemMap*/


#define CANSM_START_SEC_VAR_INIT_UNSPECIFIED
#include "CanSM_MemMap.h"  /*lint !e9019 OTHER [MISRA 2012 Rule 20.1, advisory] OTHER AUTOSAR specified way of using MemMap*/
CanSM_InternalType CanSM_Internal = {
        .InitStatus = CANSM_STATUS_UNINIT,
        .Networks = CanSM_InternalNetworks,
        .ControllerModeBuf = CanSM_InternalControllerStatus,
};
#define CANSM_STOP_SEC_VAR_INIT_UNSPECIFIED
#include "CanSM_MemMap.h"  /*lint !e9019 OTHER [MISRA 2012 Rule 20.1, advisory] OTHER AUTOSAR specified way of using MemMap*/

#define CANSM_START_SEC_VAR_CLEARED_UNSPECIFIED
#include "CanSM_MemMap.h"  /*lint !e9019 OTHER [MISRA 2012 Rule 20.1, advisory] OTHER AUTOSAR specified way of using MemMap*/
const CanSM_ConfigType* CanSM_ConfigPtr;
#define CANSM_STOP_SEC_VAR_CLEARED_UNSPECIFIED
#include "CanSM_MemMap.h"  /*lint !e9019 OTHER [MISRA 2012 Rule 20.1, advisory] OTHER AUTOSAR specified way of using MemMap*/


Std_ReturnType CanSM_Internal_GetCanSMCtrlIdx(const uint8 controller, uint8* indexPtr)
{
    Std_ReturnType ret = E_NOT_OK;
    for(uint8 index = 0; index < NOF_CANSM_CANIF_CONTROLLERS; index++) {
        if( controller == CanSM_Internal.ControllerModeBuf[index].controllerId ) {
            *indexPtr = index;
            ret = E_OK;
            break;
        }
    }
    return ret;
}

#if (CANSM_CAN_TRCV_SUPPORT == STD_ON)
Std_ReturnType CanIfTrcvChnlToCanSMNwHdl(uint8 TransceiverId,NetworkHandleType *CanSMNetworkIndex)
{
    Std_ReturnType ret = E_NOT_OK;
    for (NetworkHandleType i=0; i < CANSM_NETWORK_COUNT; i++ )
    {
        if (CanSM_Internal.Networks[i].TransceiverModeBuf.trcvId == TransceiverId)
        {
            ret = E_OK;
            *CanSMNetworkIndex =i;
            break;
        }
    }

    return ret;
}
#endif

static void CanSM_Internal_ResetControllerIndications(NetworkHandleType CanSMNetworkIndex)
{
    const CanSM_NetworkType *NetworkCfg = &CanSM_ConfigPtr->Networks[CanSMNetworkIndex];
    for(uint8 ctrl = 0; ctrl < NetworkCfg->ControllerCount; ctrl++) {
        uint8 index;
        if( E_OK == CanSM_Internal_GetCanSMCtrlIdx(NetworkCfg->Controllers[ctrl].CanIfControllerId, &index) ) {
            CanSM_Internal.ControllerModeBuf[index].indCtrlMode = CANIF_CS_UNINIT;
        }
    }
}

static void CanSM_Internal_EnterState(NetworkHandleType CanSMNetworkIndex, CanSM_Internal_BsmStateType state)
{
    CanSM_Internal_NetworkType *Network = &CanSM_Internal.Networks[CanSMNetworkIndex];
    Network->RepeatCounter = 0;
    Network->subStateTimer = 0;
    Network->BsmState = state;
    switch(state) {
        case CANSM_BSM_S_NOT_INITIALIZED:
            break;
        case CANSM_BSM_S_PRE_NOCOM:
#if (CANSM_CAN_TRCV_SUPPORT == STD_ON)
            if ((TRUE == CanSM_ConfigPtr->Networks[CanSMNetworkIndex].CanTrcvAvailable)
                    && (TRUE == CanSM_ConfigPtr->Networks[CanSMNetworkIndex].CanSMTrcvPNEnabled))
            {
                Network->PreNoCommState = CANSM_PRENOCOMM_S_PN_CLEAR_WUF;
            } else {
                Network->PreNoCommState = CANSM_PRENOCOMM_S_CC_STOPPED;
            }
            CanSM_Internal.Networks[CanSMNetworkIndex].TransceiverModeBuf.indTrcvMode = CANTRCV_TRCVMODE_NORMAL;
#else
            Network->PreNoCommState = CANSM_PRENOCOMM_S_CC_STOPPED;
#endif
            CanSM_Internal_ResetControllerIndications(CanSMNetworkIndex);
            break;
        case CANSM_BSM_S_NOCOM:
            Network->initialNoComRun = TRUE;
            break;
        case CANSM_BSM_S_PRE_FULLCOM:
#if (CANSM_CAN_TRCV_SUPPORT == STD_ON)
            if (TRUE == CanSM_ConfigPtr->Networks[CanSMNetworkIndex].CanTrcvAvailable)
            {
                Network->PreFullCommState = CANSM_PREFULLCOMM_S_TRCV_NORMAL;
            }else {
                Network->PreFullCommState = CANSM_PREFULLCOMM_S_CC_STOPPED;
            }
            CanSM_Internal.Networks[CanSMNetworkIndex].TransceiverModeBuf.indTrcvMode = CANTRCV_TRCVMODE_NORMAL;
#else
            Network->PreFullCommState = CANSM_PREFULLCOMM_S_CC_STOPPED;
#endif
            CanSM_Internal_ResetControllerIndications(CanSMNetworkIndex);
            break;
        case CANSM_BSM_S_FULLCOM:
            Network->FullCommState = CANSM_FULLCOMM_S_BUSOFF_CHECK;
            break;
        case CANSM_BSM_S_SILENTCOM:
            break;
        default:
            break;
    }
}

static boolean CanSM_Internal_HasPendingControllerIndication(NetworkHandleType CanSMNetworkIndex)
{
    boolean hasPendingIndication = FALSE;
    const CanSM_NetworkType *NetworkCfg = &CanSM_ConfigPtr->Networks[CanSMNetworkIndex];

    for(uint8 ctrl = 0; ctrl < NetworkCfg->ControllerCount; ctrl++) {
        uint8 ctrlIndex;
        if( E_OK != CanSM_Internal_GetCanSMCtrlIdx(NetworkCfg->Controllers[ctrl].CanIfControllerId, &ctrlIndex) ) {
            hasPendingIndication = TRUE;
            break;
        }
        if( TRUE == CanSM_Internal.ControllerModeBuf[ctrlIndex].hasPendingCtrlIndication ) {
            /* No indication received after last request */
            hasPendingIndication = TRUE;
        }
    }
    return hasPendingIndication;
}

/* @req CANSM023 */
void CanSM_Init( const CanSM_ConfigType* ConfigPtr ) {

    boolean done;
    /* @req CANSM179 */
    if( NULL == ConfigPtr ) {
        CANSM_DET_REPORTERROR(CANSM_SERVICEID_INIT, CANSM_E_PARAM_POINTER);
        /*lint -e{904} ARGUMENT CHECK */
        return;
    }

    CanSM_ConfigPtr = ConfigPtr;

    for( uint8 ctrl = 0; ctrl < NOF_CANSM_CANIF_CONTROLLERS; ctrl++ ) {
        CanSM_Internal.ControllerModeBuf[ctrl].controllerId = INVALID_CONTROLLER_ID;
        CanSM_Internal.ControllerModeBuf[ctrl].indCtrlMode = CANIF_CS_UNINIT;
        CanSM_Internal.ControllerModeBuf[ctrl].hasPendingCtrlIndication = FALSE;
    }
    for (uint8 i = 0; i < CANSM_NETWORK_COUNT; ++i) {
        CanSM_Internal.Networks[i].requestedMode = COMM_NO_COMMUNICATION;
        CanSM_Internal.Networks[i].currentMode = COMM_NO_COMMUNICATION;
        CanSM_Internal.Networks[i].BsmState = CANSM_BSM_S_NOT_INITIALIZED;
        CanSM_Internal.Networks[i].FullCommState = CANSM_FULLCOMM_S_BUSOFF_CHECK;
        CanSM_Internal.Networks[i].initialNoComRun = FALSE;
        CanSM_Internal.Networks[i].RepeatCounter = 0;
        CanSM_Internal.Networks[i].requestAccepted = FALSE;
        CanSM_Internal.Networks[i].busoffCounter = 0;
        CanSM_Internal.Networks[i].busoffevent = FALSE;

#if (CANSM_CAN_TRCV_SUPPORT == STD_ON)
        if (TRUE == CanSM_ConfigPtr->Networks[i].CanTrcvAvailable)
        {
            CanSM_Internal.Networks[i].TransceiverModeBuf.trcvId = CanSM_ConfigPtr->Networks[i].CanIfTransceiverId ;
            CanSM_Internal.Networks[i].TransceiverModeBuf.hasPendingTrcvIndication = FALSE;
            CanSM_Internal.Networks[i].TransceiverModeBuf.indTrcvMode = CANTRCV_TRCVMODE_NORMAL;
            CanSM_Internal.Networks[i].PreNoCommState = CANSM_PRENOCOMM_S_TRCV_NORMAL;
            CanSM_Internal.Networks[i].PreFullCommState = CANSM_PREFULLCOMM_S_TRCV_NORMAL;
            CanSM_Internal.Networks[i].TransceiverModeBuf.clearWufPendingIndication = FALSE;
            CanSM_Internal.Networks[i].TransceiverModeBuf.checkWufPendingIndication = FALSE;
        } else {
            CanSM_Internal.Networks[i].PreNoCommState = CANSM_PRENOCOMM_S_CC_STOPPED;
            CanSM_Internal.Networks[i].PreFullCommState = CANSM_PREFULLCOMM_S_CC_STOPPED;
        }

#else
        CanSM_Internal.Networks[i].PreNoCommState = CANSM_PRENOCOMM_S_CC_STOPPED;
        CanSM_Internal.Networks[i].PreFullCommState = CANSM_PREFULLCOMM_S_CC_STOPPED;
#endif

        for(uint8 ctrl = 0; ctrl < CanSM_ConfigPtr->Networks[i].ControllerCount; ctrl++) {
            done = FALSE;
            for(uint8 index = 0; (index < NOF_CANSM_CANIF_CONTROLLERS) && (FALSE==done); index++) {
                if( CanSM_ConfigPtr->Networks[i].Controllers[ctrl].CanIfControllerId ==
                    CanSM_Internal.ControllerModeBuf[index].controllerId ) {
                    /* Controller already indexed */
                    done = TRUE;/* Will break the loop */
                } else if( INVALID_CONTROLLER_ID == CanSM_Internal.ControllerModeBuf[index].controllerId ) {
                    /* Empty slot, insert controller id */
                    CanSM_Internal.ControllerModeBuf[index].controllerId = CanSM_ConfigPtr->Networks[i].Controllers[ctrl].CanIfControllerId;
                    done = TRUE;/* Will break the loop */
                } else {
                    /* Continue.. */
                }
            }
        }
    }

    CanSM_Internal.InitStatus = CANSM_STATUS_INIT;

}

/* @req CANSM064 */
void CanSM_ControllerBusOff(uint8 ControllerId)
{
    /* @req CANSM190 */
    if( CANSM_STATUS_UNINIT == CanSM_Internal.InitStatus ) {
        CANSM_DET_REPORTERROR(CANSM_SERVICEID_CONTROLLERBUSOFF, CANSM_E_UNINIT);
        /*lint -e{904} Return statement is necessary in case of reporting a DET error */
        return;
    }

    Std_ReturnType status = E_NOT_OK;

    // Find which network has this controller
    for (uint8 i = 0; i < CANSM_NETWORK_COUNT; ++i) {
        const CanSM_NetworkType* Network = &CanSM_ConfigPtr->Networks[i];
        for (uint8 j = 0; j < Network->ControllerCount; ++j) {
            const CanSM_ControllerType* ptrController = &Network->Controllers[j];
            if(ptrController->CanIfControllerId == ControllerId)
            {
                if (CANSM_BSM_S_FULLCOM == CanSM_Internal.Networks[i].BsmState)
                {
                    /* @req CANSM235 */
                    CanSM_Internal.Networks[i].busoffevent = TRUE;
                }
                else {
                    CANSM_DET_REPORTERROR(CANSM_SERVICEID_CONTROLLERBUSOFF, CANSM_E_INVALID_INVALID_BUSOFF);
                }
                status = E_OK;
            }
        }
    }

    // Check if controller was valid
    if(status != E_OK){
        /* @req CANSM189 */
        CANSM_DET_REPORTERROR(CANSM_SERVICEID_CONTROLLERBUSOFF, CANSM_E_PARAM_CONTROLLER);
    }
}

/* @req CANSM062 */
Std_ReturnType CanSM_RequestComMode( NetworkHandleType NetworkHandle, ComM_ModeType ComM_Mode ) {

    Std_ReturnType status;
    status = E_OK;
    /* @req CANSM182 */
    /* @req CANSM369 */
    /* @req CANSM370 */
    /* @req CANSM278 */
    if( CANSM_STATUS_UNINIT == CanSM_Internal.InitStatus ) {
        /* @req CANSM184 */
        CANSM_DET_REPORTERROR(CANSM_SERVICEID_REQUESTCOMMODE, CANSM_E_UNINIT);
        /*lint -e{904} Return statement is necessary in case of reporting a DET error */
        return E_NOT_OK;
    }

    if( (NetworkHandle >= CanSM_ConfigPtr->ChannelMapSize) || (CANSM_NETWORK_COUNT <= CanSM_ConfigPtr->ChannelMap[NetworkHandle]) ) {
        /* @req CANSM183 */
        CANSM_DET_REPORTERROR(CANSM_SERVICEID_REQUESTCOMMODE, CANSM_E_INVALID_NETWORK_HANDLE);
        /*lint -e{904} ARGUMENT CHECK */
        return E_NOT_OK;
    }
    /* Not a requirement but still.. */
    if( (COMM_NO_COMMUNICATION != ComM_Mode) && (COMM_SILENT_COMMUNICATION != ComM_Mode) && (COMM_FULL_COMMUNICATION != ComM_Mode) ) {
        CANSM_DET_REPORTERROR(CANSM_SERVICEID_REQUESTCOMMODE, CANSM_E_INVALID_NETWORK_MODE);
        /*lint -e{904} Return statement is necessary in case of reporting a DET error */
        return E_NOT_OK;
    }
    NetworkHandleType CanSMNetworkId = CanSM_ConfigPtr->ChannelMap[NetworkHandle];
    CanSM_Internal_NetworkType *Network = &CanSM_Internal.Networks[CanSMNetworkId];

    /* @req CANSM278 */
    /* @req CANSM555 */
    if( CANSM_BSM_S_NOT_INITIALIZED == Network->BsmState ) {
      status = E_NOT_OK;
    }
    else if( (TRUE == CanSM_Internal_HasPendingControllerIndication(CanSMNetworkId))
#if (CANSM_CAN_TRCV_SUPPORT == STD_ON)
            /*lint -e{9007} */
            || ((TRUE == CanSM_ConfigPtr->Networks[CanSMNetworkId].CanTrcvAvailable) && (E_OK != CanSM_Internal_GetTrcvrModeInd(CanSMNetworkId)))
#endif
        )
    {
        /* @req CANSM395 */
        /* Pending indication from CanIf */
        CANSM_DET_REPORTERROR(CANSM_SERVICEID_REQUESTCOMMODE, CANSM_E_WAIT_MODE_INDICATION);
        status = E_NOT_OK;
    }
    /* @req CANSM402 */
    /* Added to this requirement that state machine cannot be in CANSM_BSM_S_PRECOM
    * if COMM_SILENT_COMMUNICATION is requested this to handle the case when
    * CANSM_BSM_S_FULLCOM exits with T_REPEAT_MAX. */
    else if( ((COMM_NO_COMMUNICATION == Network->currentMode) ||
            (CANSM_BSM_S_PRE_NOCOM == Network->BsmState)) &&
            (COMM_SILENT_COMMUNICATION == ComM_Mode)) {
        /* @req CANSM403 */
        CANSM_DET_REPORTERROR(CANSM_SERVICEID_REQUESTCOMMODE, CANSM_E_INVALID_COMM_REQUEST);
        status = E_NOT_OK;
    }
    /* Translate below to busoff state */
    /* @req CANSM375 */
    /* @req CANSM376 */
    else if( (CANSM_BSM_S_FULLCOM == Network->BsmState) &&
            (CANSM_FULLCOMM_S_BUSOFF_CHECK != Network->FullCommState) &&
            (CANSM_FULLCOMM_S_NO_BUSOFF != Network->FullCommState) ) {
        /* @req CANSM377 */
        CANSM_DET_REPORTERROR(CANSM_SERVICEID_REQUESTCOMMODE, CANSM_E_BUSOFF_RECOVERY_ACTIVE);
        status = E_NOT_OK;
    }
    /* Deny request if there is a busoff event. Do not report error here since upper layer cannot
     * know that there is a busoff event present. If busoff event has been handled in main function
     * we will never end up here as if should be handled by check above. */
    else if( TRUE == Network->busoffevent ) {
        status = E_NOT_OK;
    } else {
        /* Request accepted! */
        Network->requestedMode = ComM_Mode;
        status = E_OK;
    }
    return status;
}

/* @req CANSM063 */
Std_ReturnType CanSM_GetCurrentComMode( NetworkHandleType NetworkHandle, ComM_ModeType* ComM_ModePtr ) {

    Std_ReturnType status;
    status = E_OK;
    /* @req CANSM188 */
    if( CANSM_STATUS_UNINIT == CanSM_Internal.InitStatus ) {
        CANSM_DET_REPORTERROR(CANSM_SERVICEID_GETCURRENTCOMMODE, CANSM_E_UNINIT);
        /*lint -e{904} Return statement is necessary in case of reporting a DET error */
        return E_NOT_OK;
    }

    /* @req CANSM371 */
    /* @req CANSM372 */
    if( (NetworkHandle >= CanSM_ConfigPtr->ChannelMapSize) || (CANSM_NETWORK_COUNT <= CanSM_ConfigPtr->ChannelMap[NetworkHandle]) ) {
        /* @req CANSM187 */
        CANSM_DET_REPORTERROR(CANSM_SERVICEID_GETCURRENTCOMMODE, CANSM_E_INVALID_NETWORK_HANDLE);
        /*lint -e{904} ARGUMENT CHECK */
        return E_NOT_OK;
    }

    if( NULL == ComM_ModePtr ) {
        /* @req CANSM360 */
        CANSM_DET_REPORTERROR(CANSM_SERVICEID_GETCURRENTCOMMODE, CANSM_E_PARAM_POINTER);
        /*lint -e{904} ARGUMENT CHECK */
        return E_NOT_OK;
    }

    /* @req CANSM282 */
    if( FALSE == CanSM_Internal.Networks[CanSM_ConfigPtr->ChannelMap[NetworkHandle]].initialNoComRun ) {
        status = E_NOT_OK;
    }
    if (status == E_OK){
        /* @req CANSM186 */
        *ComM_ModePtr = CanSM_Internal.Networks[CanSM_ConfigPtr->ChannelMap[NetworkHandle]].currentMode;
    }
    return status;
}

static void CanSM_Internal_SetNetworkPduMode(NetworkHandleType CanSMNetworkIndex, CanIf_PduSetModeType pduMode)
{
    const CanSM_NetworkType *NetworkCfg = &CanSM_ConfigPtr->Networks[CanSMNetworkIndex];

    for(uint8 ctrl = 0; ctrl < NetworkCfg->ControllerCount; ctrl++) {
        if( E_OK != CanIf_SetPduMode(NetworkCfg->Controllers[ctrl].CanIfControllerId, pduMode) ) {
            /* This is unexpected.. */
            CANSM_DET_REPORTERROR(CANSM_SERVICEID_MAINFUNCTION, CANSM_E_UNEXPECTED_EXECUTION);
        }
    }
}

static void CanSM_Internal_ResetPendingIndications(NetworkHandleType CanSMNetworkIndex)
{
    const CanSM_NetworkType *NetworkCfg = &CanSM_ConfigPtr->Networks[CanSMNetworkIndex];

    for(uint8 ctrl = 0; ctrl < NetworkCfg->ControllerCount; ctrl++) {
        uint8 ctrlIndex;
        if( E_OK == CanSM_Internal_GetCanSMCtrlIdx(NetworkCfg->Controllers[ctrl].CanIfControllerId, &ctrlIndex) ) {
            CanSM_Internal.ControllerModeBuf[ctrlIndex].hasPendingCtrlIndication = FALSE;
        }
    }
#if (CANSM_CAN_TRCV_SUPPORT == STD_ON)
    CanSM_Internal.Networks[CanSMNetworkIndex].TransceiverModeBuf.hasPendingTrcvIndication = FALSE;
#endif
}



static void CanSM_Internal_ComM_BusSM_ModeIndication( NetworkHandleType CanSMNetworkIndex, ComM_ModeType ComM_Mode )
{
    /* Update internals */
    CanSM_Internal.Networks[CanSMNetworkIndex].currentMode = ComM_Mode;
    /* Indicate to ComM */
    ComM_BusSM_ModeIndication(CanSM_ConfigPtr->Networks[CanSMNetworkIndex].ComMNetworkHandle, &ComM_Mode);
}
#if (CANSM_CAN_TRCV_SUPPORT == STD_ON)
#endif

#if (CANSM_CAN_TRCV_SUPPORT == STD_ON)
CanSM_Internal_TransitionReturnType CanSM_Internal_DeinitWithPnSupport(NetworkHandleType CanSMNetworkIndex,CanSM_Internal_NetworkType *Network,CanSM_Internal_PreNoCommExitPointType *exitPoint,CanSM_Internal_PreNoCommStateType *nextSubState)
{
    CanSM_Internal_TransitionReturnType tRet = T_DONE;

    switch(Network->PreNoCommState) {
        /* @req CANSM438 *//* @req CANSM439 *//* @req CANSM440 *//* @req CANSM443 */
        case CANSM_PRENOCOMM_S_PN_CLEAR_WUF:
            tRet = CanSM_Internal_Execute(Network, CanSMNetworkIndex, T_PN_CLEAR_WUF_CC_STOPPED);
            if( T_DONE == tRet ) {
                *nextSubState = CANSM_PRENOCOMM_S_CC_STOPPED;
            }
            break;
            /* @req CANSM441 *//* @req CANSM442 *//* @req CANSM444 *//* @req CANSM445 */
        case CANSM_PRENOCOMM_S_CC_STOPPED:
            tRet = CanSM_Internal_Execute(Network, CanSMNetworkIndex, T_CC_STOPPED_TRCV_NORMAL);
            if( T_DONE == tRet ) {
                *nextSubState = CANSM_PRENOCOMM_S_TRCV_NORMAL;
            }
            break;
            /* @req CANSM446 *//* @req CANSM447 *//* @req CANSM448 *//* @req CANSM449 */
        case CANSM_PRENOCOMM_S_TRCV_NORMAL:
            tRet = CanSM_Internal_Execute(Network, CanSMNetworkIndex, T_TRCV_NORMAL_TRCV_STANDBY);
            if( T_DONE == tRet ) {
                *nextSubState = CANSM_PRENOCOMM_S_TRCV_STANDBY;
            }
            break;
            /* @req CANSM450 *//* @req CANSM451 *//* @req CANSM452 *//* @req CANSM454 */
        case CANSM_PRENOCOMM_S_TRCV_STANDBY:
            tRet = CanSM_Internal_Execute(Network, CanSMNetworkIndex, T_TRCV_STANDBY_CC_SLEEP);
            if( T_DONE == tRet ) {
                *nextSubState = CANSM_PRENOCOMM_S_CC_SLEEP;
            }
            break;
            /* @req CANSM453 *//* @req CANSM455 *//* @req CANSM456 *//* @req CANSM457 */
        case CANSM_PRENOCOMM_S_CC_SLEEP:
            tRet = CanSM_Internal_Execute(Network, CanSMNetworkIndex, T_CC_SLEEP_CHECK_WUF_IN_CC_SLEEP);
            if( T_DONE == tRet ) {
                *nextSubState = CANSM_PRENOCOMM_S_CHECK_WUF_IN_CC_SLEEP;
            } else if(T_TIMEOUT == tRet){
                *nextSubState = CANSM_PRENOCOMM_S_CHECK_WUF_IN_NOT_CC_SLEEP;
            }else{
                /* Do nothing */
            }
            break;
            /* @req CANSM458 *//* @req CANSM459 *//* @req CANSM460 *//* @req CANSM461 */
        case CANSM_PRENOCOMM_S_CHECK_WUF_IN_CC_SLEEP:
            tRet = CanSM_Internal_Execute(Network, CanSMNetworkIndex, T_CHECK_WUF_IN_CC_SLEEP_NONE);
            if( T_DONE == tRet ) {
                *exitPoint = CANSM_PRENOCOMM_EXIT_NOCOM;
            } else{
                /* Do nothing */
            }
            break;
            /* @req CANSM462 */
        case CANSM_PRENOCOMM_S_CHECK_WUF_IN_NOT_CC_SLEEP:
            tRet = CanSM_Internal_Execute(Network, CanSMNetworkIndex, T_CHECK_WUF_IN_NOT_CC_SLEEP_PN_CLEAR_WUF);
            if( T_DONE == tRet ) {
                *nextSubState = CANSM_PRENOCOMM_S_PN_CLEAR_WUF;
            }
            break;
        default:
            CANSM_DET_REPORTERROR(CANSM_SERVICEID_MAINFUNCTION, CANSM_E_UNEXPECTED_EXECUTION);
            break;
    }
    return tRet;
}
#endif

CanSM_Internal_TransitionReturnType CanSM_Internal_DeinitWithoutPnSupport(NetworkHandleType CanSMNetworkIndex,CanSM_Internal_NetworkType *Network,CanSM_Internal_PreNoCommExitPointType *exitPoint,CanSM_Internal_PreNoCommStateType *nextSubState)
{
    CanSM_Internal_TransitionReturnType tRet = T_DONE;

    switch(Network->PreNoCommState) {
#if (CANSM_CAN_TRCV_SUPPORT == STD_ON)
        /* @req CANSM472 *//* @req CANSM473 *//* @req CANSM474 *//* @req CANSM475 */
        case CANSM_PRENOCOMM_S_TRCV_NORMAL:
            tRet = CanSM_Internal_Execute(Network, CanSMNetworkIndex, T_TRCV_NORMAL_TRCV_STANDBY);
            if( T_DONE == tRet ) {
                *nextSubState = CANSM_PRENOCOMM_S_TRCV_STANDBY;
            }else{
                /* Do nothing */
            }
            break;
            /* @req CANSM476 *//* @req CANSM477 *//* @req CANSM478 *//* @req CANSM479 */
        case CANSM_PRENOCOMM_S_TRCV_STANDBY:
            tRet = CanSM_Internal_Execute(Network, CanSMNetworkIndex, T_TRCV_STANDBY_NONE);
            if( T_DONE == tRet ) {
                *exitPoint = CANSM_PRENOCOMM_EXIT_NOCOM;
            }else{
                /* Do nothing */
            }
            break;
#endif
        case CANSM_PRENOCOMM_S_CC_STOPPED:
            /* @req CANSM464 *//* @req CANSM465 *//* @req CANSM466 *//* @req CANSM467 */
            tRet = CanSM_Internal_Execute(Network, CanSMNetworkIndex, T_CC_STOPPED_CC_SLEEP);
            if( T_DONE == tRet ) {
                *nextSubState = CANSM_PRENOCOMM_S_CC_SLEEP;
            }else{
                /* Do nothing */
            }
            break;
        case CANSM_PRENOCOMM_S_CC_SLEEP:
            /* @req CANSM468 *//* @req CANSM469 *//* @req CANSM470 *//* @req CANSM471 */
            /* @req CANSM560 *//* @req CANSM556 *//* @req CANSM557 */
#if (CANSM_CAN_TRCV_SUPPORT == STD_ON)
            if (TRUE == CanSM_ConfigPtr->Networks[CanSMNetworkIndex].CanTrcvAvailable){
                tRet = CanSM_Internal_Execute(Network, CanSMNetworkIndex, T_CC_SLEEP_TRCV_NORMAL);
                if( T_DONE == tRet ) {
                    *nextSubState = CANSM_PRENOCOMM_S_TRCV_NORMAL;
                }else{
                    /* Do nothing */
                }
            } else {
                tRet = CanSM_Internal_Execute(Network, CanSMNetworkIndex, T_CC_SLEEP_NONE);
                if( T_DONE == tRet ) {
                    *exitPoint = CANSM_PRENOCOMM_EXIT_NOCOM;
                }else{
                    /* Do nothing */
                }
            }
#else
            tRet = CanSM_Internal_Execute(Network, CanSMNetworkIndex, T_CC_SLEEP_NONE);
            if( T_DONE == tRet ) {
                /* No support for trcv so we are done in this state */
                *exitPoint = CANSM_PRENOCOMM_EXIT_NOCOM;
            }else{
                /* Do nothing */
            }

#endif
            break;
        default:
            CANSM_DET_REPORTERROR(CANSM_SERVICEID_MAINFUNCTION, CANSM_E_UNEXPECTED_EXECUTION);
            break;
    }
    return tRet;
}

static CanSM_Internal_PreNoCommExitPointType CanSm_Internal_BSM_S_PRE_NOCOM( NetworkHandleType CanSMNetworkIndex )
{
    CanSM_Internal_PreNoCommExitPointType exitPoint = CANSM_PRENOCOMM_EXIT_NONE;
    CanSM_Internal_TransitionReturnType tRet ;
    CanSM_Internal_NetworkType *Network = &CanSM_Internal.Networks[CanSMNetworkIndex];
    CanSM_Internal_PreNoCommStateType nextSubState = Network->PreNoCommState;

#if (CANSM_CAN_TRCV_SUPPORT == STD_ON)
    if ((FALSE == CanSM_ConfigPtr->Networks[CanSMNetworkIndex].CanTrcvAvailable)|| (FALSE == CanSM_ConfigPtr->Networks[CanSMNetworkIndex].CanSMTrcvPNEnabled))
    {
        /* @req CANSM436 */
        tRet = CanSM_Internal_DeinitWithoutPnSupport(CanSMNetworkIndex, Network, &exitPoint, &nextSubState);
    } else if (TRUE == CanSM_ConfigPtr->Networks[CanSMNetworkIndex].CanSMTrcvPNEnabled){
        /* @req CANSM437 */
        tRet = CanSM_Internal_DeinitWithPnSupport(CanSMNetworkIndex, Network, &exitPoint, &nextSubState);
    }else{
        /* Do nothing */
        tRet =  T_FAIL;
    }
#else
    tRet = CanSM_Internal_DeinitWithoutPnSupport(CanSMNetworkIndex, Network, &exitPoint, &nextSubState);
#endif
    if( T_WAIT_INDICATION != tRet ) {
        Network->subStateTimer = 0;
        if( T_REPEAT_MAX == tRet ) {
            CanSM_Internal_ResetPendingIndications(CanSMNetworkIndex);
            /* @req CANSM480 */
            /* @req CANSM385 */
            CANSM_DET_REPORTERROR(CANSM_SERVICEID_MAINFUNCTION, CANSM_E_MODE_REQUEST_TIMEOUT);
            exitPoint = CANSM_PRENOCOMM_EXIT_REP_MAX;
        }
    }
    if( nextSubState != Network->PreNoCommState ) {
        /* Changing state */
        Network->PreNoCommState = nextSubState;
    }
    Network->subStateTimer++;
    return exitPoint;
}

static CanSM_Internal_PreFullCommExitPointType CanSm_Internal_BSM_S_PRE_FULLCOM( NetworkHandleType CanSMNetworkIndex )
{
    CanSM_Internal_PreFullCommExitPointType exitPoint = CANSM_PREFULLCOMM_EXIT_NONE;
    CanSM_Internal_TransitionReturnType tRet = T_DONE;
    CanSM_Internal_NetworkType *Network = &CanSM_Internal.Networks[CanSMNetworkIndex];
    CanSM_Internal_PreFullCommStateType nextSubState = Network->PreFullCommState;

    switch(Network->PreFullCommState) {
#if (CANSM_CAN_TRCV_SUPPORT == STD_ON)
        /* @req CANSM483 *//* @req CANSM484 *//* @req CANSM485 *//* @req CANSM486 */
        case CANSM_PREFULLCOMM_S_TRCV_NORMAL:
            tRet = CanSM_Internal_Execute(Network, CanSMNetworkIndex, T_TRCV_NORMAL_CC_STOPPED);
            if( T_DONE == tRet ) {
                nextSubState = CANSM_PREFULLCOMM_S_CC_STOPPED;
            }
            break;
#endif
            /* @req CANSM560 *//* @req CANSM558 */
        case CANSM_PREFULLCOMM_S_CC_STOPPED:
            /* @req CANSM487 *//* @req CANSM488 *//* @req CANSM489 *//* @req CANSM490 */
            tRet = CanSM_Internal_Execute(Network, CanSMNetworkIndex, T_CC_STOPPED_CC_STARTED);
            if( T_DONE == tRet ) {
                nextSubState = CANSM_PREFULLCOMM_S_CC_STARTED;
            }
            break;
        case CANSM_PREFULLCOMM_S_CC_STARTED:
            /* @req CANSM491 *//* @req CANSM492 *//* @req CANSM493 *//* @req CANSM494 */
            tRet = CanSM_Internal_Execute(Network, CanSMNetworkIndex, T_CC_STARTED_NONE);
            if( T_DONE == tRet ) {
                exitPoint = CANSM_PREFULLCOMM_EXIT_FULLCOMM;
            }
            break;
        default:
            CANSM_DET_REPORTERROR(CANSM_SERVICEID_MAINFUNCTION, CANSM_E_UNEXPECTED_EXECUTION);
            break;
    }
    if( T_WAIT_INDICATION != tRet ) {
        Network->subStateTimer = 0;
        if( T_REPEAT_MAX == tRet ) {
            CanSM_Internal_ResetPendingIndications(CanSMNetworkIndex);
            /* @req CANSM495 */
            /* @req CANSM385 */
            CANSM_DET_REPORTERROR(CANSM_SERVICEID_MAINFUNCTION, CANSM_E_MODE_REQUEST_TIMEOUT);
            exitPoint = CANSM_PREFULLCOMM_EXIT_REP_MAX;
        }
    }
    if( nextSubState != Network->PreFullCommState ) {
        /* Changing state */
        Network->PreFullCommState = nextSubState;
    }
    Network->subStateTimer++;
    return exitPoint;
}


static CanSM_Internal_FullCommExitPointType CanSm_Internal_BSM_S_FULLCOM( NetworkHandleType CanSMNetworkIndex )
{
    /* Sub state machine for requested full communication requirements
     * !req CANSM507
     * !req CANSM528
     *  */
    CanSM_Internal_FullCommExitPointType exitPoint = CANSM_FULLCOMM_EXIT_NONE;
    CanSM_Internal_NetworkType *Network = &CanSM_Internal.Networks[CanSMNetworkIndex];
    CanSM_Internal_TransitionReturnType tRet;
    const CanSM_NetworkType *NetworkCfg = &CanSM_ConfigPtr->Networks[CanSMNetworkIndex];

    switch(Network->FullCommState)  {
        case CANSM_FULLCOMM_S_BUSOFF_CHECK:
            Network->subStateTimer++;
            if( TRUE == Network->busoffevent ) {
                /* @req CANSM500 */
                Network->busoffTime = 0;
                Network->busoffevent = FALSE;
                /* Reset indications so that all controllers are restarted. */
                CanSM_Internal_ResetControllerIndications(CanSMNetworkIndex);
                if( Network->busoffCounter < NetworkCfg->CanSMBorCounterL1ToL2 ) {
                    Network->busoffCounter++;
                }
#if defined(USE_BSWM)
                /* @req CANSM508*/
                BswM_CanSM_CurrentState(NetworkCfg->ComMNetworkHandle, CANSM_BSWM_BUS_OFF);
#endif
                /* @req CANSM521 */
                CanSM_Internal_ComM_BusSM_ModeIndication(CanSMNetworkIndex, COMM_SILENT_COMMUNICATION);
#if defined(USE_DEM)
                /* @req CANSM522 */
                if( DEM_EVENT_ID_NULL != NetworkCfg->CanSMDemEventId ) {
                    Dem_ReportErrorStatus(NetworkCfg->CanSMDemEventId, DEM_EVENT_STATUS_PREFAILED);
                }
#endif
                /* Restart controller */
                CanSM_Internal_SetNetworkPduMode(CanSMNetworkIndex, CANIF_SET_OFFLINE); /* Non Autosar standard fix to possible frames are sent during busoff situation */
                tRet = CanSM_Internal_Execute(Network, CanSMNetworkIndex, T_CC_STARTED_NONE);
                if( T_REPEAT_MAX != tRet ) {
                    Network->FullCommState = CANSM_FULLCOMM_S_RESTART_CC;
                } else {
                    CanSM_Internal_ResetPendingIndications(CanSMNetworkIndex);
                    /* T_REPEAT_MAX */
                    /* @req CANSM523 */
                    /* @req CANSM385 */
                    CANSM_DET_REPORTERROR(CANSM_SERVICEID_MAINFUNCTION, CANSM_E_MODE_REQUEST_TIMEOUT);
                    exitPoint = CANSM_FULLCOMM_EXIT_PRENOCOMM;
                }
                Network->subStateTimer = 0;
            } else if( Network->subStateTimer >= NetworkCfg->CanSMBorTimeTxEnsured ) {
                /* @req CANSM496 */
                /* !req CANSM497 */
#if defined(USE_DEM)
                /* @req CANSM498 */
                if( DEM_EVENT_ID_NULL != NetworkCfg->CanSMDemEventId ) {
                    Dem_ReportErrorStatus(NetworkCfg->CanSMDemEventId, DEM_EVENT_STATUS_PASSED);
                }
#endif
#if defined(USE_CANSM_EXTENSION)
                CanSM_Extension_ReportNoBusOff(CanSMNetworkIndex);
#endif
                Network->busoffCounter = 0;
                Network->FullCommState = CANSM_FULLCOMM_S_NO_BUSOFF;
            } else {
                /* Stay in this state. */
            }
            break;
        case CANSM_FULLCOMM_S_NO_BUSOFF:
            if( TRUE == Network->busoffevent ) {
                /* @req CANSM500 */
                Network->busoffevent = FALSE;
                Network->busoffTime = 0;
                /* Reset indications so that all controllers are restarted. */
                CanSM_Internal_ResetControllerIndications(CanSMNetworkIndex);
                if( Network->busoffCounter < NetworkCfg->CanSMBorCounterL1ToL2 ) {
                    Network->busoffCounter++;
                }

#if defined(USE_BSWM)
                /* @req CANSM508 */
                BswM_CanSM_CurrentState(NetworkCfg->ComMNetworkHandle, CANSM_BSWM_BUS_OFF);
#endif
                /* @req CANSM521 */
                CanSM_Internal_ComM_BusSM_ModeIndication(CanSMNetworkIndex, COMM_SILENT_COMMUNICATION);
#if defined(USE_DEM)
                /* @req CANSM522 */
                if( DEM_EVENT_ID_NULL != NetworkCfg->CanSMDemEventId ) {
                    Dem_ReportErrorStatus(NetworkCfg->CanSMDemEventId, DEM_EVENT_STATUS_PREFAILED);
                }
#endif

                /* Restart controller */
                CanSM_Internal_SetNetworkPduMode(CanSMNetworkIndex, CANIF_SET_OFFLINE); /* Non Autosar standard fix to possible frames are sent during busoff situation */
                tRet = CanSM_Internal_Execute(Network, CanSMNetworkIndex, T_CC_STARTED_NONE);
                if( T_REPEAT_MAX != tRet ) {
                    Network->busoffevent = FALSE;
                    Network->FullCommState = CANSM_FULLCOMM_S_RESTART_CC;
                } else {
                    CanSM_Internal_ResetPendingIndications(CanSMNetworkIndex);
                    /* T_REPEAT_MAX */
                    /* @req CANSM523 */
                    /* @req CANSM385 */
                    CANSM_DET_REPORTERROR(CANSM_SERVICEID_MAINFUNCTION, CANSM_E_MODE_REQUEST_TIMEOUT);
                    exitPoint = CANSM_FULLCOMM_EXIT_PRENOCOMM;
                }
                Network->subStateTimer = 0;
            } else if( (COMM_SILENT_COMMUNICATION == Network->requestedMode) ||
                    ( COMM_NO_COMMUNICATION == Network->requestedMode ) ) {
                /* @req CANSM499 */
                /* @req CANSM554 */
                exitPoint = CANSM_FULLCOMM_EXIT_SILENTCOMM;
            } else {
                /* Stay in this state. */
            }
            break;
        case CANSM_FULLCOMM_S_RESTART_CC:
            Network->subStateTimer++;
            /* @req CANSM509 *//* @req CANSM510 *//* @req CANSM466 *//* @req CANSM467 */
            tRet = CanSM_Internal_Execute(Network, CanSMNetworkIndex, T_CC_STARTED_NONE);
            if( T_DONE == tRet ) {
                /* @req CANSM511 */
                /* @req CANSM513 */
                CanSM_Internal_SetNetworkPduMode(CanSMNetworkIndex, CANIF_SET_TX_OFFLINE);
                Network->FullCommState = CANSM_FULLCOMM_S_TX_OFF;
                Network->subStateTimer = 0;
            } else if( (T_FAIL == tRet) || (T_REQ_ACCEPTED == tRet)) {
                /* Timeout or request not accepted */
                Network->subStateTimer = 0;
            } else if( T_REPEAT_MAX == tRet ) {
                CanSM_Internal_ResetPendingIndications(CanSMNetworkIndex);
                /* T_REPEAT_MAX */
                /* @req CANSM523 */
                /* @req CANSM385 */
                CANSM_DET_REPORTERROR(CANSM_SERVICEID_MAINFUNCTION, CANSM_E_MODE_REQUEST_TIMEOUT);
                exitPoint = CANSM_FULLCOMM_EXIT_PRENOCOMM;
                Network->subStateTimer = 0;
                Network->busoffevent = FALSE;
            } else {
                /* Pending */
            }
            break;
        case CANSM_FULLCOMM_S_TX_OFF:
            Network->subStateTimer++;
            /* @req CANSM514 */
            /* @req CANSM515 */
            if( ((Network->busoffTime >= NetworkCfg->CanSMBorTimeL1) &&
                    (Network->busoffCounter < NetworkCfg->CanSMBorCounterL1ToL2)) ||
                    ((Network->busoffTime >= NetworkCfg->CanSMBorTimeL2) &&
                    (Network->busoffCounter >= NetworkCfg->CanSMBorCounterL1ToL2))) {
#if defined(USE_CANSM_EXTENSION)
                CanSM_Extension_WriteState(CanSMNetworkIndex, Network->busoffCounter, Network->busoffTime, NetworkCfg);
#endif

                /* @req CANSM516 */
                CanSM_Internal_SetNetworkPduMode(CanSMNetworkIndex, CANIF_SET_ONLINE);
#if defined(USE_BSWM)
                /* @req CANSM517*/
                BswM_CanSM_CurrentState(NetworkCfg->ComMNetworkHandle, CANSM_BSWM_FULL_COMMUNICATION);
#endif
                /* @req CANSM518 */
                CanSM_Internal_ComM_BusSM_ModeIndication(CanSMNetworkIndex, COMM_FULL_COMMUNICATION);
                Network->FullCommState = CANSM_FULLCOMM_S_BUSOFF_CHECK;
                Network->subStateTimer = 0;
            }
            break;
        default:
            CANSM_DET_REPORTERROR(CANSM_SERVICEID_MAINFUNCTION, CANSM_E_UNEXPECTED_EXECUTION);
            break;
    }
    Network->busoffTime++;
    return exitPoint;
}

/* @req CANSM065 */
void CanSM_MainFunction() {

    /* @req CANSM167 */
    /* @req CANSM266 */
    /* @req CANSM284 */
    /* @req CANSM428 */

    /* Main state machine requirements

     * Guard: G_FULL_COM_MODE_REQUESTED
     * !req CANSM427
     * Guard: G_SILENT_COM_MODE_REQUESTED
     * !req CANSM429

     * Effect: E_BR_END_FULL_COM
     * !req CANSM432
     * Effect: E_BR_END_SILENT_COM
     * !req CANSM433
     * */

    /* Sub state machine to operate a requested baud rate change
     *
     * !req CANSM524
     * !req CANSM525
     * !req CANSM526
     * !req CANSM527
     * !req CANSM529
     * !req CANSM531
     * !req CANSM532
     * !req CANSM533
     * !req CANSM534
     * !req CANSM535
     * !req CANSM536
     * !req CANSM542
     * !req CANSM543
     *  */
    boolean status;
    status = TRUE;

    if( CANSM_STATUS_UNINIT == CanSM_Internal.InitStatus ) {
        status = FALSE;
    }
    if (status == TRUE){
        for (uint8 CanSMNetworkIndex = 0; CanSMNetworkIndex < CANSM_NETWORK_COUNT; CanSMNetworkIndex++) {
            const CanSM_Internal_NetworkType *Network = &CanSM_Internal.Networks[CanSMNetworkIndex];
#if defined(USE_BSWM)
            const CanSM_NetworkType *NetworkCfg = &CanSM_ConfigPtr->Networks[CanSMNetworkIndex];
#endif
            switch(Network->BsmState) {
                case CANSM_BSM_S_NOT_INITIALIZED:
                case CANSM_BSM_S_PRE_NOCOM:
                case CANSM_BSM_S_SILENTCOM:
                    /* @req CANSM423 */
                    /* @req CANSM426 */
                    if( (CANSM_BSM_S_NOT_INITIALIZED == Network->BsmState) ||
                            ((CANSM_BSM_S_SILENTCOM == Network->BsmState) && (COMM_NO_COMMUNICATION == Network->requestedMode)) ) {
                        CanSM_Internal_EnterState(CanSMNetworkIndex, CANSM_BSM_S_PRE_NOCOM);
#if defined(USE_BSWM)
                        /* @req CANSM431 */
                        BswM_CanSM_CurrentState(NetworkCfg->ComMNetworkHandle, CANSM_BSWM_NO_COMMUNICATION);
#endif
                    }
                    if( CANSM_BSM_S_PRE_NOCOM == Network->BsmState ) {
                        CanSM_Internal_PreNoCommExitPointType preNoComExit = CanSm_Internal_BSM_S_PRE_NOCOM(CanSMNetworkIndex);
                        if( CANSM_PRENOCOMM_EXIT_NOCOM == preNoComExit ) {
                            CanSM_Internal_EnterState(CanSMNetworkIndex, CANSM_BSM_S_NOCOM);
                            /* @req CANSM430 */
                            CanSM_Internal_ComM_BusSM_ModeIndication(CanSMNetworkIndex, COMM_NO_COMMUNICATION);
                        } else if( CANSM_PRENOCOMM_EXIT_REP_MAX == preNoComExit ) {
                            CanSM_Internal_EnterState(CanSMNetworkIndex, CANSM_BSM_S_PRE_NOCOM);
                            /* Run a loop in PRE_NOCOM.
                             * NOTE: Not expected to exit that state here. */
                            if( CANSM_PRENOCOMM_EXIT_NOCOM == CanSm_Internal_BSM_S_PRE_NOCOM(CanSMNetworkIndex) ) {
                                CANSM_DET_REPORTERROR(CANSM_SERVICEID_MAINFUNCTION, CANSM_E_UNEXPECTED_EXECUTION);
                            }
                        } else {
                            /* No exit. */
                        }
                    } else if( (CANSM_BSM_S_SILENTCOM == Network->BsmState) && (COMM_FULL_COMMUNICATION == Network->requestedMode) ) {
                        // Effect: E_SILENT_TO_FULL_COM
                        /* Should be same as E_FULL_COMM. Spec. refers to CANSM435 but only obeying that
                         * requirement would not be the same as E_FULL_COMM. */
                        CanSM_Internal_EnterState(CanSMNetworkIndex, CANSM_BSM_S_FULLCOM);
                        /* @req CANSM550 */
#if defined(USE_BSWM)
                        /* @req CANSM435 */ /* @req CANSM550 */
                        BswM_CanSM_CurrentState(NetworkCfg->ComMNetworkHandle, CANSM_BSWM_FULL_COMMUNICATION);
#endif

                        /* @req CANSM435 */
                        CanSM_Internal_SetNetworkPduMode(CanSMNetworkIndex, CANIF_SET_ONLINE);
                        CanSM_Internal_ComM_BusSM_ModeIndication(CanSMNetworkIndex, COMM_FULL_COMMUNICATION);
                    } else {
                        /* Stay in state. */
                    }
                    break;
                case CANSM_BSM_S_NOCOM:
                case CANSM_BSM_S_PRE_FULLCOM:
                    /* @req CANSM425 */
                    if( (CANSM_BSM_S_NOCOM == Network->BsmState) && (COMM_FULL_COMMUNICATION == Network->requestedMode) ) {
                        /* Entered CANSM_BSM_S_PRE_NOCOM, reset pre full comm substate */
                        CanSM_Internal_EnterState(CanSMNetworkIndex, CANSM_BSM_S_PRE_FULLCOM);
                    }
                    if(CANSM_BSM_S_PRE_FULLCOM == Network->BsmState) {
                        CanSM_Internal_PreFullCommExitPointType preFullCommExit = CanSm_Internal_BSM_S_PRE_FULLCOM(CanSMNetworkIndex);
                        if( CANSM_PREFULLCOMM_EXIT_FULLCOMM == preFullCommExit ) {
                            CanSM_Internal_EnterState(CanSMNetworkIndex, CANSM_BSM_S_FULLCOM);
#if defined(USE_BSWM)
                            /* @req CANSM435 */
                            BswM_CanSM_CurrentState(NetworkCfg->ComMNetworkHandle, CANSM_BSWM_FULL_COMMUNICATION);
#endif
                            /* @req CANSM539 */
                            CanSM_Internal_SetNetworkPduMode(CanSMNetworkIndex, CANIF_SET_ONLINE);
                            /* @req CANSM540 */
                            CanSM_Internal_ComM_BusSM_ModeIndication(CanSMNetworkIndex, COMM_FULL_COMMUNICATION);
                        } else if( CANSM_PREFULLCOMM_EXIT_REP_MAX == preFullCommExit ) {
                            CanSM_Internal_EnterState(CanSMNetworkIndex, CANSM_BSM_S_PRE_NOCOM);
                            /* Run a loop in PRE_NOCOM.
                             * NOTE: Not expected to exit that state here. */
                            if( CANSM_PRENOCOMM_EXIT_NOCOM == CanSm_Internal_BSM_S_PRE_NOCOM(CanSMNetworkIndex) ) {
                                CANSM_DET_REPORTERROR(CANSM_SERVICEID_MAINFUNCTION, CANSM_E_UNEXPECTED_EXECUTION);
                            }
                        } else {
                            /* No exit. */
                        }
                    }
                    break;
                case CANSM_BSM_S_FULLCOM:
                {
                    boolean enterPreNoCom = FALSE;
                    CanSM_Internal_FullCommExitPointType fullCommExit = CanSm_Internal_BSM_S_FULLCOM(CanSMNetworkIndex);
                    if(CANSM_FULLCOMM_EXIT_PRENOCOMM == fullCommExit) {
                        /* Exit to PreNoCom is only due to T_REPEAT_MAX */
                        enterPreNoCom = TRUE;
                    } else if( CANSM_FULLCOMM_EXIT_SILENTCOMM == fullCommExit ) {
                        /* Exit due to accepted ComMode request */
                        CanSM_Internal_EnterState(CanSMNetworkIndex, CANSM_BSM_S_SILENTCOM);
#if defined(USE_BSWM)
                        /* @req CANSM434 */
                        BswM_CanSM_CurrentState(NetworkCfg->ComMNetworkHandle, CANSM_BSWM_SILENT_COMMUNICATION);
#endif
                        /* @req CANSM541 */
                        CanSM_Internal_SetNetworkPduMode(CanSMNetworkIndex, CANIF_SET_ONLINE);
                        /* @req CANSM537 */
                        CanSM_Internal_SetNetworkPduMode(CanSMNetworkIndex, CANIF_SET_TX_OFFLINE);
                        /* @req CANSM538 */
                        CanSM_Internal_ComM_BusSM_ModeIndication(CanSMNetworkIndex, COMM_SILENT_COMMUNICATION);
                    } else {
                        /* No exit. */
                    }
                    if( TRUE == enterPreNoCom ) {
                        CanSM_Internal_EnterState(CanSMNetworkIndex, CANSM_BSM_S_PRE_NOCOM);
#if defined(USE_BSWM)
                        /* @req CANSM431 */
                        BswM_CanSM_CurrentState(NetworkCfg->ComMNetworkHandle, CANSM_BSWM_NO_COMMUNICATION);
#endif
                        /* Run a loop in PRE_NOCOM.
                         * NOTE: Not expected to exit that state here. */
                        if( CANSM_PRENOCOMM_EXIT_NOCOM == CanSm_Internal_BSM_S_PRE_NOCOM(CanSMNetworkIndex) ) {
                            CANSM_DET_REPORTERROR(CANSM_SERVICEID_MAINFUNCTION, CANSM_E_UNEXPECTED_EXECUTION);
                        }
                    }
                    break;
                }
                default:
                    break;
            }
        }
    }
}

/* @req CANSM396 */
void CanSM_ControllerModeIndication( uint8 ControllerId, CanIf_ControllerModeType ControllerMode )
{
    if( CANSM_STATUS_UNINIT == CanSM_Internal.InitStatus ) {
        /* @req CANSM398 */
        CANSM_DET_REPORTERROR(CANSM_SERVICEID_CONTROLLERMODEINDICATION, CANSM_E_UNINIT);
        /*lint -e{904} Return statement is necessary in case of reporting a DET error */
        return;
    }

    uint8 controllerIndex;
    if( E_OK == CanSM_Internal_GetCanSMCtrlIdx(ControllerId, &controllerIndex) ) {
        /* Controller was valid */
        /* Ignore indication of other modes than the requested */
        if( (TRUE == CanSM_Internal.ControllerModeBuf[controllerIndex].hasPendingCtrlIndication) &&
                (ControllerMode == CanSM_Internal.ControllerModeBuf[controllerIndex].pendingCtrlMode) ) {
            CanSM_Internal.ControllerModeBuf[controllerIndex].hasPendingCtrlIndication = FALSE;
        }
        CanSM_Internal.ControllerModeBuf[controllerIndex].indCtrlMode = ControllerMode;
    } else {
        /* @req CANSM397 */
        CANSM_DET_REPORTERROR(CANSM_SERVICEID_CONTROLLERMODEINDICATION, CANSM_E_PARAM_CONTROLLER);
    }
}

#if 0
Std_ReturnType CanSm_CheckBaudrate(NetworkHandleType network, const uint16 Baudrate) {
    /* !req CANSM564 */
    /* !req CANSM565 */
    /* !req CANSM562 */
    /* !req CANSM571 */
    /* !req CANSM563 */
    /* !req CANSM566 */
    /* !req CANSM567 */
    /* !req CANSM568 */
    /* !req CANSM572 */
    return E_NOT_OK;
}

Std_ReturnType CanSm_ChangeBaudrate(NetworkHandleType network, const uint16 Baudrate) {
    /* !req CANSM569 */
    /* !req CANSM570 */
    /* !req CANSM502 */
    /* !req CANSM504 */
    /* !req CANSM505 */
    /* !req CANSM530 */
    /* !req CANSM506 */
    /* !req CANSM573 */
    /* !req CANSM574 */
    /* !req CANSM503 */

    return E_NOT_OK;
}
void CanSM_TxTimeoutException( NetworkHandleType Channel )
{
    /* !req CANSM411 */
    /* !req CANSM412 */
}
#endif
#if (CANSM_CAN_TRCV_SUPPORT == STD_ON)
/* @req CANSM413 */
void CanSM_ClearTrcvWufFlagIndication( uint8 Transceiver )
{
    NetworkHandleType CanSMNetworkIndex;

    /* @req CANSM414 */
    if( CANSM_STATUS_UNINIT == CanSM_Internal.InitStatus ) {
        CANSM_DET_REPORTERROR(CANSM_SERVICEID_CLEARTRCVWUFINDICATION, CANSM_E_UNINIT);
        /*lint -e{904} Return statement is necessary in case of reporting a DET error */
        return;
    }

    /* @req CANSM415 */
    if (E_NOT_OK == CanIfTrcvChnlToCanSMNwHdl(Transceiver, &CanSMNetworkIndex))
    {
        CANSM_DET_REPORTERROR(CANSM_SERVICEID_CLEARTRCVWUFINDICATION, CANSM_E_PARAM_TRANSCEIVER);
        /*lint -e{904} ARGUMENT CHECK */
        return;
    }
    CanSM_Internal.Networks[CanSMNetworkIndex].TransceiverModeBuf.clearWufPendingIndication = FALSE;
    return;
}
/* @req CANSM416 */
void CanSM_CheckTransceiverWakeFlagIndication( uint8 Transceiver )
{
    NetworkHandleType CanSMNetworkIndex;
    /* @req CANSM417 */
    if( CANSM_STATUS_UNINIT == CanSM_Internal.InitStatus ) {
        CANSM_DET_REPORTERROR(CANSM_SERVICEID_CHECKTRCVWUFINDICATION, CANSM_E_UNINIT);
        /*lint -e{904} Return statement is necessary in case of reporting a DET error */
        return ;
    }

    /* @req CANSM418 */
    if (E_NOT_OK == CanIfTrcvChnlToCanSMNwHdl(Transceiver, &CanSMNetworkIndex))
    {
        CANSM_DET_REPORTERROR(CANSM_SERVICEID_CHECKTRCVWUFINDICATION, CANSM_E_PARAM_TRANSCEIVER);
        /*lint -e{904} ARGUMENT CHECK */
        return;
    }
    CanSM_Internal.Networks[CanSMNetworkIndex].TransceiverModeBuf.checkWufPendingIndication = FALSE;
    return;

}

/* @req CANSM419 */
void CanSM_ConfirmPnAvailability( uint8 Transceiver )
{
    NetworkHandleType CanSMNetworkIndex;

    /* @req CANSM420 */
    if( CANSM_STATUS_UNINIT == CanSM_Internal.InitStatus ) {
            /* @req CANSM401 */
            CANSM_DET_REPORTERROR(CANSM_SERVICEID_CONFIRMPNAVAILABILITY, CANSM_E_UNINIT);
            /*lint -e{904} Return statement is necessary in case of reporting a DET error */
            return;
    }

    /* @req CANSM421 */
    if (E_NOT_OK == CanIfTrcvChnlToCanSMNwHdl(Transceiver, &CanSMNetworkIndex))
    {
        CANSM_DET_REPORTERROR(CANSM_SERVICEID_CONFIRMPNAVAILABILITY, CANSM_E_PARAM_TRANSCEIVER);
        /*lint -e{904} ARGUMENT CHECK */
        return;
    }
#if defined(USE_CANNM)
    /* @req CANSM546 */
    /* @req CANSM422 */
    CanNm_ConfirmPnAvailability(CanSM_ConfigPtr->Networks[CanSMNetworkIndex].ComMNetworkHandle);
#endif
    return;
}



/* @req CANSM399 */
void CanSM_TransceiverModeIndication(uint8 TransceiverId, CanTrcv_TrcvModeType TransceiverMode)
{
    NetworkHandleType CanSMNetworkIndex;

    if( CANSM_STATUS_UNINIT == CanSM_Internal.InitStatus ) {
            /* @req CANSM401 */
            CANSM_DET_REPORTERROR(CANSM_SERVICEID_TRANSCEIVERMODEINDICATION, CANSM_E_UNINIT);
            /*lint -e{904} Return statement is necessary in case of reporting a DET error */
            return;
    }

    if (E_NOT_OK == CanIfTrcvChnlToCanSMNwHdl(TransceiverId, &CanSMNetworkIndex))
    {
        /* @req CANSM400 */
        CANSM_DET_REPORTERROR(CANSM_SERVICEID_TRANSCEIVERMODEINDICATION, CANSM_E_PARAM_TRANSCEIVER);
        /*lint -e{904} ARGUMENT CHECK */
        return;
    }

    if( (TRUE == CanSM_Internal.Networks[CanSMNetworkIndex].TransceiverModeBuf.hasPendingTrcvIndication) &&
        (TransceiverMode == CanSM_Internal.Networks[CanSMNetworkIndex].TransceiverModeBuf.pendingTrcvMode))
    {
        CanSM_Internal.Networks[CanSMNetworkIndex].TransceiverModeBuf.hasPendingTrcvIndication = FALSE;
    }

    CanSM_Internal.Networks[CanSMNetworkIndex].TransceiverModeBuf.indTrcvMode = TransceiverMode;
    return;
}
#endif
