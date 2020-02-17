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


#include "CanSM.h"
#include "CanIf.h"
#include "CanSM_Internal.h"

/*lint -esym(9003,CanSM_ConfigPtr )*/
extern const CanSM_ConfigType* CanSM_ConfigPtr;
extern CanSM_InternalType CanSM_Internal;
#if (CANSM_CAN_TRCV_SUPPORT == STD_ON)
typedef enum{
    CLEAR_WUF =0,
    CHECK_WUF
}CanSM_WufActionType;
#endif
typedef union {
    CanIf_ControllerModeType ccMode;
#if (CANSM_CAN_TRCV_SUPPORT == STD_ON)
    CanTrcv_TrcvModeType trcvMode;
    CanSM_WufActionType wufAction;
#endif
}CanSM_ActionMode;/*lint !e9018*/

typedef enum {
    A_CC_MODE = 0,
#if (CANSM_CAN_TRCV_SUPPORT == STD_ON)
    A_TRCV_MODE,
    A_WUF,
#endif
    A_NO_MODE
}CanSM_ActionType;

typedef struct {
    CanSM_ActionType FirstAction;
    CanSM_ActionMode FirstMode;
    CanSM_ActionType SecondAction;
    CanSM_ActionMode SecondMode;
}CanSM_TransitionType;
#define CANSM_NOF_TRANSITIONS (uint8)((uint8)T_CC_STARTED_NONE + 1u)
/*lint -esym(9003,TransitionConfig )*/
const CanSM_TransitionType TransitionConfig[T_NOF_TRANSITIONS] = {
        [T_CC_STOPPED_CC_SLEEP] = {
            .FirstAction = A_CC_MODE,
            .FirstMode = {.ccMode = CANIF_CS_STOPPED},
            .SecondAction = A_CC_MODE,
            .SecondMode = {.ccMode = CANIF_CS_SLEEP}
        },
#if (CANSM_CAN_TRCV_SUPPORT == STD_ON)
        [T_CC_SLEEP_TRCV_NORMAL] = {
            .FirstAction = A_CC_MODE,
            .FirstMode = {.ccMode = CANIF_CS_SLEEP},
            .SecondAction = A_TRCV_MODE,
            .SecondMode = {.trcvMode = CANTRCV_TRCVMODE_NORMAL}
        },
        [T_TRCV_NORMAL_TRCV_STANDBY] = {
            .FirstAction = A_TRCV_MODE,
            .FirstMode = {.trcvMode = CANTRCV_TRCVMODE_NORMAL},
            .SecondAction = A_TRCV_MODE,
            .SecondMode = {.trcvMode = CANTRCV_TRCVMODE_STANDBY}
        },
        [T_TRCV_STANDBY_NONE] = {
            .FirstAction = A_TRCV_MODE,
            .FirstMode = {.trcvMode = CANTRCV_TRCVMODE_STANDBY},
            .SecondAction = A_NO_MODE,
            .SecondMode = {.ccMode = (CanIf_ControllerModeType)0} /* Don't care */
        },
        [T_TRCV_NORMAL_CC_STOPPED] = {
            .FirstAction = A_TRCV_MODE,
            .FirstMode = {.trcvMode = CANTRCV_TRCVMODE_NORMAL},
            .SecondAction = A_CC_MODE,
            .SecondMode = {.ccMode = CANIF_CS_STOPPED}
        },
        [T_PN_CLEAR_WUF_CC_STOPPED] = {
            .FirstAction = A_WUF,
            .FirstMode = {.wufAction = CLEAR_WUF},
            .SecondAction = A_CC_MODE,
            .SecondMode = {.ccMode = CANIF_CS_STOPPED}
        },
        [T_CC_STOPPED_TRCV_NORMAL] = {
            .FirstAction = A_CC_MODE,
            .FirstMode = {.ccMode = CANIF_CS_STOPPED},
            .SecondAction = A_TRCV_MODE,
            .SecondMode = {.trcvMode = CANTRCV_TRCVMODE_NORMAL}
        },
        [T_TRCV_STANDBY_CC_SLEEP] = {
            .FirstAction = A_TRCV_MODE,
            .FirstMode = {.trcvMode = CANTRCV_TRCVMODE_STANDBY},
            .SecondAction = A_CC_MODE,
            .SecondMode = {.ccMode = CANIF_CS_SLEEP}
        },
        [T_CC_SLEEP_CHECK_WUF_IN_CC_SLEEP] = {
            .FirstAction = A_CC_MODE,
            .FirstMode = {.ccMode = CANIF_CS_SLEEP},
            .SecondAction = A_WUF,
            .SecondMode = {.wufAction = CHECK_WUF}
        },
        [T_CHECK_WUF_IN_CC_SLEEP_NONE] = {
            .FirstAction = A_WUF,
            .FirstMode = {.wufAction = CHECK_WUF},
            .SecondAction = A_NO_MODE,
            .SecondMode = {.ccMode = (CanIf_ControllerModeType)0} /* Don't care */
        },
        [T_CHECK_WUF_IN_NOT_CC_SLEEP_PN_CLEAR_WUF] = {
            .FirstAction = A_WUF,
            .FirstMode = {.wufAction = CHECK_WUF},
            .SecondAction = A_WUF,
            .SecondMode = {.wufAction = CLEAR_WUF}
        },
#endif
        [T_CC_SLEEP_NONE] = {
            .FirstAction = A_CC_MODE,
            .FirstMode = {.ccMode = CANIF_CS_SLEEP},
            .SecondAction = A_NO_MODE,
            .SecondMode = {.ccMode = (CanIf_ControllerModeType)0} /* Don't care */
        },

        [T_CC_STOPPED_CC_STARTED] = {
            .FirstAction = A_CC_MODE,
            .FirstMode = {.ccMode = CANIF_CS_STOPPED},
            .SecondAction = A_CC_MODE,
            .SecondMode = {.ccMode = CANIF_CS_STARTED}
        },
        [T_CC_STARTED_NONE] = {
            .FirstAction = A_CC_MODE,
            .FirstMode = {.ccMode = CANIF_CS_STARTED},
            .SecondAction = A_NO_MODE,
            .SecondMode = {.ccMode = (CanIf_ControllerModeType)0} /* Don't care */
        }
};

/*lint -save -e9018 *//* We use pointers of type enum and we are careful in handling this */
Std_ReturnType modeRequestAction(NetworkHandleType CanSMNetworkIndex, const CanSM_ActionType *action, const CanSM_ActionMode *mode);
Std_ReturnType modeReadRequest(NetworkHandleType CanSMNetworkIndex, const CanSM_ActionType *action, const CanSM_ActionMode *mode);

static Std_ReturnType CanSM_Internal_GetNetworkControllerModeIndicated(NetworkHandleType CanSMNetworkIndex)
{
    Std_ReturnType totalStatus = E_OK;
    const CanSM_NetworkType *NetworkCfg = &CanSM_ConfigPtr->Networks[CanSMNetworkIndex];

    for(uint8 ctrl = 0; ctrl < NetworkCfg->ControllerCount; ctrl++) {
        uint8 ctrlIndex;
        if( E_OK != CanSM_Internal_GetCanSMCtrlIdx(NetworkCfg->Controllers[ctrl].CanIfControllerId, &ctrlIndex) ) {
            totalStatus = E_NOT_OK;
            break;
        }
        if(CanSM_Internal.ControllerModeBuf[ctrlIndex].indCtrlMode != CanSM_Internal.ControllerModeBuf[ctrlIndex].pendingCtrlMode) {
            /* Has not indicated the last requested mode */
            totalStatus = E_NOT_OK;
        }
        if( TRUE == CanSM_Internal.ControllerModeBuf[ctrlIndex].hasPendingCtrlIndication ) {
            /* No indication received after last request */
            totalStatus = E_NOT_OK;
        }
    }
    return totalStatus;
}

#if (CANSM_CAN_TRCV_SUPPORT == STD_ON)
/* Set Trcv Mode */
Std_ReturnType CanSM_Internal_SetNetworkTransceiverMode(NetworkHandleType CanSMNetworkIndex, CanTrcv_TrcvModeType trcvMode)
{
    Std_ReturnType totalStatus = E_OK;

    CanSM_Internal.Networks[CanSMNetworkIndex].TransceiverModeBuf.hasPendingTrcvIndication = TRUE;
    CanSM_Internal.Networks[CanSMNetworkIndex].TransceiverModeBuf.pendingTrcvMode = trcvMode;

    if( E_OK != CanIf_SetTrcvMode(CanSM_Internal.Networks[CanSMNetworkIndex].TransceiverModeBuf.trcvId, trcvMode)) {
            /* Not accepted. No mode indication pending */
        CanSM_Internal.Networks[CanSMNetworkIndex].TransceiverModeBuf.hasPendingTrcvIndication  = FALSE;
        totalStatus = E_NOT_OK;
    }
    return totalStatus;
}
/* Get Trcv Mode */
Std_ReturnType CanSM_Internal_GetTrcvrModeInd(NetworkHandleType CanSMNetworkIndex)
{
    Std_ReturnType totalStatus = E_OK;

    if( TRUE == CanSM_Internal.Networks[CanSMNetworkIndex].TransceiverModeBuf.hasPendingTrcvIndication) {
        totalStatus = E_NOT_OK;
    }
    return totalStatus;
}
/* Check Wakeup flag status */
Std_ReturnType CanSM_Internal_CheckWuf(NetworkHandleType CanSMNetworkIndex)
{
    Std_ReturnType totalStatus;
    totalStatus = CanIf_CheckTrcvWakeFlag(CanSM_Internal.Networks[CanSMNetworkIndex].TransceiverModeBuf.trcvId);
    if(E_OK ==  totalStatus) {
        CanSM_Internal.Networks[CanSMNetworkIndex].TransceiverModeBuf.checkWufPendingIndication = TRUE;
    } else{
        CanSM_Internal.Networks[CanSMNetworkIndex].TransceiverModeBuf.checkWufPendingIndication = FALSE;
    }
    return totalStatus;
}
/* Clear Wakeup flag */
Std_ReturnType CanSM_Internal_ClearWuf(NetworkHandleType CanSMNetworkIndex)
{
    Std_ReturnType totalStatus;
    totalStatus = CanIf_ClearTrcvWufFlag(CanSM_Internal.Networks[CanSMNetworkIndex].TransceiverModeBuf.trcvId);
    if(E_OK ==  totalStatus) {
        CanSM_Internal.Networks[CanSMNetworkIndex].TransceiverModeBuf.clearWufPendingIndication = TRUE;
    } else {
        CanSM_Internal.Networks[CanSMNetworkIndex].TransceiverModeBuf.clearWufPendingIndication = FALSE;
    }
    return totalStatus;
}
/* Check wakeup flag indication */
Std_ReturnType CanSM_Internal_CheckWufInd(NetworkHandleType CanSMNetworkIndex)
{
    Std_ReturnType totalStatus = E_OK;

    if( TRUE == CanSM_Internal.Networks[CanSMNetworkIndex].TransceiverModeBuf.checkWufPendingIndication) {
        totalStatus = E_NOT_OK;
    }
    return totalStatus;
}
/* Clear Wakeup flag Indication provided */
Std_ReturnType CanSM_Internal_ClearWufInd(NetworkHandleType CanSMNetworkIndex)
{
    Std_ReturnType totalStatus = E_OK;

    if( TRUE == CanSM_Internal.Networks[CanSMNetworkIndex].TransceiverModeBuf.clearWufPendingIndication) {
        totalStatus = E_NOT_OK;
    }
    return totalStatus;
}
#endif

static Std_ReturnType CanSM_Internal_SetNetworkControllerMode(NetworkHandleType CanSMNetworkIndex, CanIf_ControllerModeType controllerMode)
{
    Std_ReturnType totalStatus = E_OK;
    const CanSM_NetworkType *NetworkCfg = &CanSM_ConfigPtr->Networks[CanSMNetworkIndex];
    for(uint8 ctrl = 0; ctrl < NetworkCfg->ControllerCount; ctrl++) {
        uint8 value;
        if( E_OK != CanSM_Internal_GetCanSMCtrlIdx(NetworkCfg->Controllers[ctrl].CanIfControllerId, &value) ) {
            totalStatus = E_NOT_OK;
            break;
        }
#if !defined(CANSM_REPEAT_ALL_REQUESTS_ON_TIMEOUT)
        /* Only request controller mode if it differs from the last indicated */
        CanSM_Internal.ControllerModeBuf[value].pendingCtrlMode = controllerMode;
        if(CanSM_Internal.ControllerModeBuf[value].indCtrlMode != controllerMode) {
            CanSM_Internal.ControllerModeBuf[value].hasPendingCtrlIndication = TRUE;
            if( E_OK != CanIf_SetControllerMode(NetworkCfg->Controllers[ctrl].CanIfControllerId, controllerMode)) {
                /* Not accepted. No mode indication pending */
                CanSM_Internal.ControllerModeBuf[value].hasPendingCtrlIndication = FALSE;
                totalStatus = E_NOT_OK;
            }
        }
#else
        CanSM_Internal.ControllerModeBuf[value].hasPendingCtrlIndication = TRUE;
        CanSM_Internal.ControllerModeBuf[value].pendingCtrlMode = controllerMode;
        if( E_OK != CanIf_SetControllerMode(NetworkCfg->Controllers[ctrl].CanIfControllerId, controllerMode)) {
            /* Not accepted. No mode indication pending */
            CanSM_Internal.ControllerModeBuf[value].hasPendingCtrlIndication = FALSE;
            totalStatus = E_NOT_OK;
        }
#endif
    }

    return totalStatus;
}
Std_ReturnType modeRequestAction(NetworkHandleType CanSMNetworkIndex,const CanSM_ActionType *action,const CanSM_ActionMode *mode)
{
    Std_ReturnType actionRet;
    actionRet = E_NOT_OK;

    if( A_CC_MODE == *action ) {
        actionRet = CanSM_Internal_SetNetworkControllerMode(CanSMNetworkIndex,mode->ccMode);
#if (CANSM_CAN_TRCV_SUPPORT == STD_ON)
    } else if (A_TRCV_MODE == *action ) {

        actionRet = CanSM_Internal_SetNetworkTransceiverMode(CanSMNetworkIndex, mode->trcvMode);
    } else if (A_WUF == *action )
    {
        if (CHECK_WUF == mode->wufAction)
        {
            actionRet = CanSM_Internal_CheckWuf(CanSMNetworkIndex);

        } else if (CLEAR_WUF == mode->wufAction){
            actionRet = CanSM_Internal_ClearWuf(CanSMNetworkIndex);
        } else{
            /* Do nothing */
        }
#endif
    } else {
        CANSM_DET_REPORTERROR(CANSM_SERVICEID_MAINFUNCTION, CANSM_E_INVALID_ACTION);
    }
    return actionRet;
}

Std_ReturnType modeReadRequest(NetworkHandleType CanSMNetworkIndex,const CanSM_ActionType *action,const CanSM_ActionMode *mode)
{
    Std_ReturnType indicationRet;
    indicationRet = E_NOT_OK;
    /*lint --e{715} */ /* mode will not be used when CANSM_CAN_TRCV_SUPPORT is off */
    if( A_CC_MODE == *action ) {
        indicationRet = CanSM_Internal_GetNetworkControllerModeIndicated(CanSMNetworkIndex);
    }
#if (CANSM_CAN_TRCV_SUPPORT == STD_ON)
    else if( A_TRCV_MODE == *action ) {
        indicationRet = CanSM_Internal_GetTrcvrModeInd(CanSMNetworkIndex);
    }
    else if (A_WUF == *action ) {
        if (CHECK_WUF == mode->wufAction)
        {
            indicationRet = CanSM_Internal_CheckWufInd(CanSMNetworkIndex);

        } else if (CLEAR_WUF == mode->wufAction){
            indicationRet = CanSM_Internal_ClearWufInd(CanSMNetworkIndex);
        }else{
            /* Do nothing */
        }
    }
#endif
    else {
       /* Do nothing */
       /* Avoid compiler warning */
       (void)mode; /*lint !e920 Avoid compiler warning (variable not used) */
    }

    return indicationRet;
}

CanSM_Internal_TransitionReturnType CanSM_Internal_Execute(CanSM_Internal_NetworkType *Network, NetworkHandleType CanSMNetworkIndex, CanSM_Internal_TransitionType transition) {
    CanSM_Internal_TransitionReturnType ret = T_WAIT_INDICATION;
    const CanSM_TransitionType *transitionPtr = &TransitionConfig[transition];
    Std_ReturnType actionRet;
    Std_ReturnType indicationRet;
    if( FALSE == Network->requestAccepted ) {
        Network->RepeatCounter++;
        /* @req CANSM464 *//* PreNoCom CC_STOPPED */
        /* @req CANSM468 *//* PreNoCom CC_SLEEP */
        /* @req CANSM487 *//* PreFullCom CC_STOPPED */
        /* @req CANSM491 *//* PreFullCom CC_STARTED */
        /* @req CANSM509 *//* FullCom RESTART_CC */
        /* @req CANSM441 *//* PreNoCom (DEINIT_WITH_PN) CC_STOPPED */
        /* @req CANSM453 *//* PreNoCom (DEINIT_WITH_PN) CC_SLEEP */
        /* @req CANSM446 *//* PreNoCom (DEINIT_WITH_PN) TRCV_NORMAL */
        /* @req CANSM450 *//* PreNoCom (DEINIT_WITH_PN) TRCV_STANDBY */
        /* @req CANSM472 *//* PreNoCom (DEINIT_WITHOUT_PN) TRCV_NORMAL */
        /* @req CANSM476 *//* PreNoCom (DEINIT_WITHOUT_PN) TRCV_STANDBY */
        /* @req CANSM483 *//* PreFullCom TRCV_NORMAL */
        /* @req CANSM458 *//* PreNoCom (DEINIT_WITH_PN) PN_CHECK_WUF */
        /* @req CANSM462 *//* PreNoCom (DEINIT_WITH_PN) PN_CHECK_WUF_IN_NOT_CC_SLEEP */
        /* @req CANSM438 */ /* PreNoCom (DEINIT_WITH_PN) PN_CLEAR_WUF */

        actionRet = modeRequestAction(CanSMNetworkIndex,&(transitionPtr->FirstAction),&(transitionPtr->FirstMode));
        if( E_OK == actionRet ) {
            /* @req CANSM465 *//* PreNoCom CC_STOPPED */
            /* @req CANSM469 *//* PreNoCom CC_SLEEP */
            /* @req CANSM488 *//* PreFullCom CC_STOPPED */
            /* @req CANSM492 *//* PreFullCom CC_STARTED */
            /* @req CANSM510 *//* FullCom RESTART_CC */
            /* @req CANSM439 *//* PreNoCom (DEINIT_WITH_PN) PN_CLEAR_WUF */
            /* @req CANSM442 *//* PreNoCom (DEINIT_WITH_PN) CC_STOPPED */
            /* @req CANSM447 *//* PreNoCom (DEINIT_WITH_PN) TRCV_NORMAL */
            /* @req CANSM451 *//* PreNoCom (DEINIT_WITH_PN) TRCV_STANDBY */
            /* @req CANSM455 *//* PreNoCom (DEINIT_WITH_PN) CC_SLEEP */
            /* @req CANSM459 *//* PreNoCom (DEINIT_WITH_PN) PN_CHECK_WUF */
            /* @req CANSM473 *//* PreNoCom (DEINIT_WITHOUT_PN) TRCV_NORMAL */
            /* @req CANSM477 *//* PreNoCom (DEINIT_WITHOUT_PN) TRCV_STANDBY */
            /* @req CANSM484 *//* PreFullCom TRCV_NORMAL */
            Network->requestAccepted = TRUE;
            ret = T_REQ_ACCEPTED;
        } else {
            /* Request not accepted */
            ret = T_FAIL;
            if( Network->RepeatCounter > CanSM_ConfigPtr->CanSMModeRequestRepetitionMax ) {
                /* @req CANSM463 */
                /* @req CANSM480 */
                /* T_REPEAT_MAX */
                Network->RepeatCounter = 0;
                ret = T_REPEAT_MAX;
            }
        }
    } else {
        /* Check if mode has been indicated */

        indicationRet = modeReadRequest(CanSMNetworkIndex,&(transitionPtr->FirstAction),&(transitionPtr->FirstMode));
        if(E_OK == indicationRet) {
            /* @req CANSM466 *//* PreNoCom CC_STOPPED */
            /* @req CANSM470 *//* PreNoCom CC_SLEEP */
            /* @req CANSM489 *//* PreFullCom CC_STOPPED */
            /* @req CANSM493 *//* PreFullCom CC_STARTED */
            /* @req CANSM511 *//* FullCom RESTART_CC */
            /* @req CANSM440 *//* PreNoCom (DEINIT_WITH_PN) PN_CLEAR_WUF */
            /* @req CANSM444 *//* PreNoCom (DEINIT_WITH_PN) CC_STOPPED */
            /* @req CANSM448 *//* PreNoCom (DEINIT_WITH_PN) TRCV_NORMAL */
            /* @req CANSM452 *//* PreNoCom (DEINIT_WITH_PN) TRCV_STANDBY */
            /* @req CANSM456 *//* PreNoCom (DEINIT_WITH_PN) CC_SLEEP */
            /* @req CANSM460 *//* PreNoCom (DEINIT_WITH_PN) PN_CHECK_WUF */
            /* @req CANSM474 *//* PreNoCom (DEINIT_WITHOUT_PN) TRCV_NORMAL */
            /* @req CANSM478 *//* PreNoCom (DEINIT_WITHOUT_PN) TRCV_STANDBY */
            /* @req CANSM485 *//* PreFullCom TRCV_NORMAL */
            /* Mode indicated, new request */
            Network->RepeatCounter = 0;
            Network->requestAccepted = FALSE;

            if( A_NO_MODE != transitionPtr->SecondAction ) {
                actionRet = modeRequestAction(CanSMNetworkIndex,&(transitionPtr->SecondAction),&(transitionPtr->SecondMode));

                Network->RepeatCounter++;
                if( E_OK == actionRet ) {
                    /* Request accepted */
                    Network->requestAccepted = TRUE;
                    ret = T_DONE;
                } else {
                    /* Not accepted */
                    if( Network->RepeatCounter <= CanSM_ConfigPtr->CanSMModeRequestRepetitionMax ) {
                        /* proceed to next state state */
                        ret = T_DONE;
                    } else {
                        /* @req CANSM463 */
                        /* @req CANSM480 */
                        /* T_REPEAT_MAX */
                        ret = T_REPEAT_MAX;
                        Network->RepeatCounter = 0;
                    }
                }
            } else {
                /* No second action */
                ret = T_DONE;
            }
        } else if( Network->subStateTimer >= CanSM_ConfigPtr->CanSMModeRequestRepetitionTime ) {
            /* Timeout */
            /* @req CANSM467 *//* PreNoCom CC_STOPPED */
            /* @req CANSM471 *//* PreNoCom CC_SLEEP */
            /* @req CANSM490 *//* PreFullCom CC_STOPPED */
            /* @req CANSM494 *//* PreFullCom CC_STARTED */
            /* @req CANSM512 *//* FullCom RESTART_CC */
            /* @req CANSM443 *//* PreNoCom (DEINIT_WITH_PN) PN_CLEAR_WUF */
            /* @req CANSM445 *//* PreNoCom (DEINIT_WITH_PN) CC_STOPPED */
            /* @req CANSM449 *//* PreNoCom (DEINIT_WITH_PN) TRCV_NORMAL */
            /* @req CANSM454 *//* PreNoCom (DEINIT_WITH_PN) TRCV_STANDBY */
            /* @req CANSM457 *//* PreNoCom (DEINIT_WITH_PN) CC_SLEEP */
            /* @req CANSM461 *//* PreNoCom (DEINIT_WITH_PN) PN_CHECK_WUF */
            /* @req CANSM475 *//* PreNoCom (DEINIT_WITHOUT_PN) TRCV_NORMAL */
            /* @req CANSM479 *//* PreNoCom (DEINIT_WITHOUT_PN) TRCV_STANDBY */
            /* @req CANSM486 *//* PreFullCom TRCV_NORMAL */
            Network->requestAccepted = FALSE;

#if (CANSM_CAN_TRCV_SUPPORT == STD_ON)
            if ((TRUE == CanSM_ConfigPtr->Networks[CanSMNetworkIndex].CanSMTrcvPNEnabled) && ( A_CC_MODE == transitionPtr->FirstAction )
                    && (CANIF_CS_SLEEP == transitionPtr->FirstMode.ccMode))
            {
                ret = T_TIMEOUT;
            }else{
                /* Do nothing */
            }

#endif
            /*lint -e{774}  ret value can be T_TIMEOUT depending on CANSM_CAN_TRCV_SUPPORT is STD_ON or STD_OFF*/
            if (ret != T_TIMEOUT) {
                if( Network->RepeatCounter <= CanSM_ConfigPtr->CanSMModeRequestRepetitionMax ) {
                    Network->RepeatCounter++;
                    /* Try again */
                    actionRet = modeRequestAction(CanSMNetworkIndex,&(transitionPtr->FirstAction),&(transitionPtr->FirstMode));

                    if( E_OK == actionRet ) {
                        /* Request accepted */
                        Network->requestAccepted = TRUE;
                        ret = T_REQ_ACCEPTED;
                    } else {
                        if( Network->RepeatCounter > CanSM_ConfigPtr->CanSMModeRequestRepetitionMax ) {
                            /* T_REPEAT_MAX */
                            /* @req CANSM480 *//* PreNoCom */
                            /* @req CANSM495 *//* PreFullCom */
                            /* @req CANSM523 *//* FullCom */
                            /* @req CANSM463 */
                            Network->RepeatCounter = 0;
                            ret = T_REPEAT_MAX;
                        } else {
                            /* Keep state */
                        }
                    }
                } else {
                    /* T_REPEAT_MAX */
                    /* @req CANSM480 *//* PreNoCom */
                    /* @req CANSM495 *//* PreFullCom */
                    /* @req CANSM523 *//* FullCom */
                    Network->RepeatCounter = 0;
                    ret = T_REPEAT_MAX;
                }
            }
        } else {
            /* Keep waiting. */
        }
    }
    return ret;
}
/*lint -restore */
