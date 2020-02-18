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

#include "BswM.h"
#include "BswM_Internal.h"
#include "MemMap.h"

#if defined(USE_COMM)
#include "ComM.h"
#include "BswM_ComM.h"
#endif
#if defined(USE_PDUR)
#include "PduR.h"
#endif
#if defined(USE_NVM)
#include "NvM.h"
#endif
#if defined(USE_CANSM)
#include "CanSM.h"
#include "BswM_CanSM.h"
#endif
#if defined(USE_SD)
#include "SD.h"
#endif
#if defined(USE_FRSM)
#include "FrSM.h"
#include "BswM_FrSM.h"
#endif
#if defined(USE_LINSM)
#include "LinSM.h"
#include "BswM_LinSM.h"
#endif
#if defined(USE_ETHSM)
#include "EthSM.h"
#include "BswM_EthSM.h"
#endif
#if defined(USE_COM)
#include "Com.h"
#endif
#if defined(USE_LINIF)
#include "LinIf.h"
#endif
#include "EcuM.h"
#include "BswM_EcuM.h"
#if defined(USE_DCM)
#include "Dcm.h"
#endif
#if defined(USE_DET)
#include "Det.h"
#endif
#if defined(USE_DEM)
#include "Dem.h"
#endif
#if defined(USE_NM)
#include "Nm.h"
#endif

#if defined(USE_RTE)
#include "Rte_BswM.h"
#endif
#include "SchM_BswM.h"


/* General requiremnets */
/* @req BswM0001 */
/* @req BswM0008 */

/* Mode Arbitration */
/* @req BswM0009 */
/* @req BswM0035 */
/* @req BswM0010 */
/* @req BswM0012 */
/* @req BswM0117 */
/* @req BswM0147 */

/* Immediate and Deferred Operation */
/* @req BswM0061 */
/* @req BswM0013 */
/* @req BswM0014 */

/* Arbitration Behavior after Initialization */
/* @req BswM0064 */

/* Mode control */
/* @req BswM0037 */

/* Available Actions */
/* @req BswM0038 */
/* @req BswM0039 */
/* @req BswM0040 */
/* @req BswM0054 */

/* @req BswM0066 */

/* @req BswM0020 */
/* @req BswM0021 */
/* @req BswM9999 */

#if (BSWM_NUMBER_OF_RULES_MAX > 0)
#define BSWM_START_SEC_VAR_CLEARED_UNSPECIFIED
#include "BswM_BswMemMap.h"  /*lint !e9019 OTHER [MISRA 2012 Rule 20.1, advisory] OTHER AUTOSAR specified way of using MemMap*/
BswM_RuleStateType    BswM_RuleStates[BSWM_NUMBER_OF_RULES_MAX];
#define BSWM_STOP_SEC_VAR_CLEARED_UNSPECIFIED
#include "BswM_BswMemMap.h"  /*lint !e9019 OTHER [MISRA 2012 Rule 20.1, advisory] OTHER AUTOSAR specified way of using MemMap*/
#endif

#if (BSWM_NUMBER_OF_REQUESTS > 0)
extern uint32         BswM_ModeReqMirrors[BSWM_NUMBER_OF_REQUESTS];
#endif


#define BSWM_START_SEC_VAR_CLEARED_UNSPECIFIED
#include "BswM_BswMemMap.h"  /*lint !e9019 OTHER [MISRA 2012 Rule 20.1, advisory] OTHER AUTOSAR specified way of using MemMap*/
static const BswM_RuleConfigType *BswM_RuleConfigPtr; /*lint -e9003 could define variable at block scope */
#define BSWM_STOP_SEC_VAR_CLEARED_UNSPECIFIED
#include "BswM_BswMemMap.h"  /*lint !e9019 OTHER [MISRA 2012 Rule 20.1, advisory] OTHER AUTOSAR specified way of using MemMap*/

#define BSWM_START_SEC_VAR_CLEARED_UNSPECIFIED
#include "BswM_BswMemMap.h"  /*lint !e9019 OTHER [MISRA 2012 Rule 20.1, advisory] OTHER AUTOSAR specified way of using MemMap*/
static const BswM_ConfigType     *BswM_ConfigPtr;
#define BSWM_STOP_SEC_VAR_CLEARED_UNSPECIFIED
#include "BswM_BswMemMap.h"  /*lint !e9019 OTHER [MISRA 2012 Rule 20.1, advisory] OTHER AUTOSAR specified way of using MemMap*/

#define BSWM_START_SEC_VAR_CLEARED_UNSPECIFIED
#include "BswM_BswMemMap.h"  /*lint !e9019 OTHER [MISRA 2012 Rule 20.1, advisory] OTHER AUTOSAR specified way of using MemMap*/
static const BswM_RuleType       *BswM_Rules;
#define BSWM_STOP_SEC_VAR_CLEARED_UNSPECIFIED
#include "BswM_BswMemMap.h"  /*lint !e9019 OTHER [MISRA 2012 Rule 20.1, advisory] OTHER AUTOSAR specified way of using MemMap*/

#define BSWM_START_SEC_VAR_CLEARED_UNSPECIFIED
#include "BswM_BswMemMap.h"  /*lint !e9019 OTHER [MISRA 2012 Rule 20.1, advisory] OTHER AUTOSAR specified way of using MemMap*/
static boolean BswM_RequestProcessingOngoing;
#define BSWM_STOP_SEC_VAR_CLEARED_UNSPECIFIED
#include "BswM_BswMemMap.h"  /*lint !e9019 OTHER [MISRA 2012 Rule 20.1, advisory] OTHER AUTOSAR specified way of using MemMap*/


#if (BSWM_NUMBER_OF_IMMEDIATE_REQUESTS > 0)
#define BSWM_START_SEC_VAR_CLEARED_UNSPECIFIED
#include "BswM_BswMemMap.h"  /*lint !e9019 OTHER [MISRA 2012 Rule 20.1, advisory] OTHER AUTOSAR specified way of using MemMap*/
static boolean BswM_DelayedRequest[BSWM_NUMBER_OF_IMMEDIATE_REQUESTS];
#define BSWM_STOP_SEC_VAR_CLEARED_UNSPECIFIED
#include "BswM_BswMemMap.h"  /*lint !e9019 OTHER [MISRA 2012 Rule 20.1, advisory] OTHER AUTOSAR specified way of using MemMap*/
#endif

/* @req BswM0128 */
#if (BSWM_PDUGROUPSWITCH_ACTIONS > 0)
#define BSWM_START_SEC_VAR_CLEARED_UNSPECIFIED
#include "BswM_BswMemMap.h"  /*lint !e9019 OTHER [MISRA 2012 Rule 20.1, advisory] OTHER AUTOSAR specified way of using MemMap*/
static boolean BswM_PduGroupSwitchActionPerformed;
#define BSWM_STOP_SEC_VAR_CLEARED_UNSPECIFIED
#include "BswM_BswMemMap.h"  /*lint !e9019 OTHER [MISRA 2012 Rule 20.1, advisory] OTHER AUTOSAR specified way of using MemMap*/

#define BSWM_START_SEC_VAR_CLEARED_UNSPECIFIED
#include "BswM_BswMemMap.h"  /*lint !e9019 OTHER [MISRA 2012 Rule 20.1, advisory] OTHER AUTOSAR specified way of using MemMap*/
static Com_IpduGroupVector BswM_ComIpduGroupVector;
#define BSWM_STOP_SEC_VAR_CLEARED_UNSPECIFIED
#include "BswM_BswMemMap.h"  /*lint !e9019 OTHER [MISRA 2012 Rule 20.1, advisory] OTHER AUTOSAR specified way of using MemMap*/


#define BSWM_START_SEC_VAR_CLEARED_UNSPECIFIED
#include "BswM_BswMemMap.h"  /*lint !e9019 OTHER [MISRA 2012 Rule 20.1, advisory] OTHER AUTOSAR specified way of using MemMap*/
static boolean BswM_ComIpduInitialize;
#define BSWM_STOP_SEC_VAR_CLEARED_UNSPECIFIED
#include "BswM_BswMemMap.h"  /*lint !e9019 OTHER [MISRA 2012 Rule 20.1, advisory] OTHER AUTOSAR specified way of using MemMap*/

#endif

#if (BSWM_DEADLINEMONITORING_ACTIONS > 0)
#define BSWM_START_SEC_VAR_INIT_UNSPECIFIED
#include "BswM_BswMemMap.h"  /*lint !e9019 OTHER [MISRA 2012 Rule 20.1, advisory] OTHER AUTOSAR specified way of using MemMap*/

static boolean BswM_DMControlPerformed;

#define BSWM_STOP_SEC_VAR_INIT_UNSPECIFIED
#include "BswM_BswMemMap.h"  /*lint !e9019 OTHER [MISRA 2012 Rule 20.1, advisory] OTHER AUTOSAR specified way of using MemMap*/

#define BSWM_START_SEC_VAR_INIT_UNSPECIFIED
#include "BswM_BswMemMap.h"  /*lint !e9019 OTHER [MISRA 2012 Rule 20.1, advisory] OTHER AUTOSAR specified way of using MemMap*/

static Com_IpduGroupVector BswM_DMControlGroupVector;

#define BSWM_STOP_SEC_VAR_INIT_UNSPECIFIED
#include "BswM_BswMemMap.h"  /*lint !e9019 OTHER [MISRA 2012 Rule 20.1, advisory] OTHER AUTOSAR specified way of using MemMap*/
#endif


#define BSWM_START_SEC_VAR_INIT_UNSPECIFIED
#include "BswM_BswMemMap.h"    /*lint !e9019 OTHER [MISRA 2012 Rule 20.1, advisory] OTHER AUTOSAR specified way of using MemMap*/

BswM_InternalType BswM_Internal = {
        .InitStatus = BSWM_UNINIT,
};

#define BSWM_STOP_SEC_VAR_INIT_UNSPECIFIED
#include "BswM_BswMemMap.h"  /*lint !e9019 OTHER [MISRA 2012 Rule 20.1, advisory] OTHER AUTOSAR specified way of using MemMap*/


#ifdef HOST_TEST
BswM_InternalType* readinternal_status(void);
BswM_InternalType* readinternal_status(void){

    return (&BswM_Internal);
}
#endif

#if (BSWM_NUMBER_OF_RULES_MAX > 0)
/**
 * @brief Evaluates a logical expression.
 * @param logicalExpressionIndex Index (determined pre-compile-time) of the logical expression to be evaluated.
 * @return BSWM_TRUE or BSWM_FALSE
 */
static BswM_RuleStateType BswM_Internal_EvaluateLogicalExpression( const uint32 logicalExpressionIndex ) {
    extern boolean (*BswM_LogicalExpressions[]) (void); /*lint -e9003 */
    boolean (*logicalExpressionFcn)(void) = BswM_LogicalExpressions[logicalExpressionIndex];
    BswM_RuleStateType retVal;

    if (TRUE == logicalExpressionFcn()) {
        retVal = BSWM_TRUE;
    } else {
        retVal = BSWM_FALSE;
    }

    return retVal;
}

/* @req BswM0015 */
/* @req BswM0065 */
/**
 * @brief Evaluates a rule.
 * @param ruleIndex Index (determined pre-compile-time and/or post-build-time) of the rule to be evaluated.
 * @return Index to the action list that should be executed as a result of the rule evaluation. Returns -1 if no action should be performed.
 */
static sint32 BswM_Internal_EvaluateRule( const sint16 ruleIndex ) {
    BswM_RuleStateType logicalExpression;
    const BswM_RuleType *rule = &(BswM_Rules[ruleIndex]);
    const BswM_ActionListType *actionLists = &(BswM_ConfigPtr->ActionListData[0]);
    sint32 actionListIndex = -1;

    logicalExpression = BswM_Internal_EvaluateLogicalExpression(rule->LogicalExpressionIndex);
    
    if (logicalExpression == BSWM_TRUE) {
        
        /* @req BswM0011 */
        /* @req BswM0115 */
        if (rule->ActionListTrueIndex >= 0) {
            if ((BSWM_CONDITION == actionLists[rule->ActionListTrueIndex].Execution) || (BSWM_TRUE != BswM_RuleStates[ruleIndex])) {
                actionListIndex = rule->ActionListTrueIndex;
            }
        }
        
        BswM_RuleStates[ruleIndex] = BSWM_TRUE;
        
    } else if (logicalExpression == BSWM_FALSE) {  
        
        /* @req BswM0023 */
        /* @req BswM0116 */
        if (rule->ActionListFalseIndex >= 0) {
            if ((BSWM_CONDITION == actionLists[rule->ActionListFalseIndex].Execution) || (BSWM_FALSE != BswM_RuleStates[ruleIndex])) {
                actionListIndex = rule->ActionListFalseIndex;
            }
        }
        
        BswM_RuleStates[ruleIndex] = BSWM_FALSE;
    } else {
        /* Do nothing */
    }

    return actionListIndex;
}

/**
 * @brief Executes an action list item that does not refer to another action list or rule.
 *
 * Based on the index value stored in the action list item, the function associated with the action is looked up and called.
 * @param item Pointer to the action list item that should be executed.
 * @return Forwards E_OK or E_NOT_OK from the action function.
 */
static Std_ReturnType BswM_Internal_ExecuteAtomicAction( const BswM_ActionListItemType *item ) {
    extern Std_ReturnType (*BswM_AtomicActions[]) (const BswM_ActionListItemType *item); /*lint -e9003 */
    Std_ReturnType (*atomicActionFcn)(const BswM_ActionListItemType *item) = BswM_AtomicActions[item->AtomicActionIndex];

    return atomicActionFcn(item);
}

/**
 * @brief Saves the list and item index.
 *
 * This is a helper function used to keep track of from where an action list or rule is executed within an action list execution.
 * When the called action list or rule has been executed, the execution of the calling action list need to be resumed.
 * @param stateBuffer Pointer to context buffer where the indices will be stored.
 * @param listIndex
 * @param itemIndex
 * @return False if there were no empty slots available (i.e. if the nesting level exceeds BSWM_HIERARCHIC_LIST_LEVELS_MAX), otherwise True.
 */
static boolean BswM_Internal_StoreListState(BswM_ListStateType *stateBuffer, sint32 listIndex, uint32 itemIndex) {
    boolean didStore = FALSE;

    if ( stateBuffer->levelCnt < (BSWM_HIERARCHIC_LIST_LEVELS_MAX - 1) ) {
        stateBuffer->itemIndexBuffer[stateBuffer->levelCnt] = itemIndex;
        stateBuffer->listIndexBuffer[stateBuffer->levelCnt] = listIndex;
        stateBuffer->levelCnt++;
        didStore = TRUE;
    }

    return didStore;
}

/**
 * @brief Restores the list and item index.
 *
 * This is a helper function used to keep track of from where an action list or rule is executed within an action list execution.
 * When the called action list or rule has been executed, the execution of the calling action list need to be resumed.
 * @param stateBuffer Pointer to context buffer from where the indices will be restored.
 * @param listIndex Returns list index.
 * @param itemIndex Returns item index.
 * @return False if there were no more state to restore, otherwise True.
 */
static boolean BswM_Internal_RestoreListState(BswM_ListStateType *stateBuffer, sint32 *listIndex, uint32 *itemIndex) {
    boolean didRestore = FALSE;

    if ( stateBuffer->levelCnt > 0 ) {
        stateBuffer->levelCnt--;
        *itemIndex = stateBuffer->itemIndexBuffer[stateBuffer->levelCnt];
        *listIndex = stateBuffer->listIndexBuffer[stateBuffer->levelCnt];
        stateBuffer->listIndexBuffer[stateBuffer->levelCnt] = 0;
        stateBuffer->itemIndexBuffer[stateBuffer->levelCnt] = 0;
        didRestore = TRUE;
    }

    return didRestore;
}

/* @req BswM0016 */
/* @req BswM0017 */
/* @req BswM0018 */
/**
 * @brief Iterates and executes an action list.
 *
 * The list items of a given action list @p entryListIndex will be executed.
 * The action items might refer to another action list or another rule.
 * @param entryListIndex Index to the action list that should be executed.
 * @return void
 */
static void BswM_Internal_IterateActionList(const sint32 entryListIndex) {
    const BswM_ActionListType *actionLists = &(BswM_ConfigPtr->ActionListData[0]);
    const BswM_ActionListItemType *item;
    uint32 itemIndex = 0;
    sint32 listIndex = entryListIndex;
    sint32 newListIndex;
    boolean abortList = FALSE;
    Std_ReturnType actionResult;
    BswM_ListStateType listState = {
            .itemIndexBuffer = {0},
            .listIndexBuffer = {0},
            .levelCnt = 0
    };

    /* This loop iterates over an action list. If a list item refers to another list, 
       either explicitly or via a rule evaluation, then that list will be iterated over as well. 
       When arriving at the end of the referred list, then the iteration of the first list is continued. */
    do {

        while ((FALSE==abortList) && (itemIndex < actionLists[listIndex].NumberOfItems)) {
            
            item = actionLists[listIndex].Items[itemIndex];
            itemIndex++;

            switch (item->ActionType) {

                case BSWM_ATOMIC:
                    actionResult = BswM_Internal_ExecuteAtomicAction(item);

                    /* @req BswM0121 */
                    if (item->ReportFailToDEMRefId >= 0) {
                        if (E_NOT_OK == actionResult) {
                            BSWM_REPORT_ERROR_STATUS(item->ReportFailToDEMRefId, DEM_EVENT_STATUS_FAILED);
                        } else {
                            BSWM_REPORT_ERROR_STATUS(item->ReportFailToDEMRefId, DEM_EVENT_STATUS_PASSED);
                        }
                    }

                    /* @req BswM0055 */
                    abortList = (TRUE == item->AbortOnFail) && (E_NOT_OK == actionResult);

                    break;

                case BSWM_LIST:
                    /* Increase list level and start iterating the referred list */
                    if (TRUE == BswM_Internal_StoreListState(&listState, listIndex, itemIndex)) {
                        listIndex = item->ReferredListIndex;
                        itemIndex = 0;
                    }

                    break;

                /* @req BswM0019 */
                /* @req BswM0067 */
                case BSWM_RULE:
                    newListIndex = BswM_Internal_EvaluateRule(item->ReferredRuleIndex);
                    if (newListIndex >= 0) {
                        /* Increase list level and start iterating the referred list */
                        if (TRUE == BswM_Internal_StoreListState(&listState, listIndex, itemIndex)) {
                            listIndex = newListIndex;
                            itemIndex = 0;
                        }
                    }

                    break;

                default:
                    break;
            }
        }

        abortList = FALSE;

        /* No more items in this list or it got aborted, 
           so continue iterating lower level lists if there are any */
    } while (TRUE == BswM_Internal_RestoreListState(&listState, &listIndex, &itemIndex));
}
#endif

#if (BSWM_NUMBER_OF_IMMEDIATE_REQUESTS > 0)
/*lint -e528 OTHER*/
static inline boolean BswM_Internal_MirrorNeedsToBeResetToUndefined ( const uint32 modeReqIndex ) {
    return ( modeReqIndex <= BSWM_IMMEDIATE_REQUEST_NEED_RESET_MIRROR_IDX_MAX );
}

/* @req BswM0059 */
/* @req BswM0062 */
/**
 * @brief Evaluates all the rules referring to an immediate mode request.
 *
 * When an immediate request is performed, all the rules that use that request
 * as a condition in its logical expression needs to be evaluated.
 * @param modeReqIndex Index to the mode request source.
 * @return void
 */
static void BswM_Internal_EvaluateRulesForImmediateRequest( const uint32 modeReqIndex ) {

#if (BSWM_NUMBER_OF_RULES_MAX > 0)
    const sint16 *modeRequestRuleList = BswM_RuleConfigPtr->ImmediateRules[modeReqIndex];
    sint16 ruleIndex;
    sint32 actionListIndex;

    for ( ; *modeRequestRuleList >= 0; modeRequestRuleList++) {
        ruleIndex = *modeRequestRuleList;

        actionListIndex = BswM_Internal_EvaluateRule(ruleIndex);

        if (0 <= actionListIndex) {
            BswM_Internal_IterateActionList(actionListIndex);
        }
    }
#endif
#if defined(BSWM_BAC_ADAPTATION)
    if (TRUE == BswM_Internal_MirrorNeedsToBeResetToUndefined(modeReqIndex)) {
        BswM_ModeReqMirrors[modeReqIndex] = BSWM_UNDEFINED_REQUEST_VALUE;
    }
#endif
}

/**
 * @brief Handle pending requests
 *
 * If an immediate request happened during the evaluation of another request (or deferred rules),
 * they will be processed by this function.
 * @param void
 * @return void
 */
static void BswM_Internal_ProcessDelayedRequests ( void ) {
    for (uint32 modeReqIndex = 0; modeReqIndex < BSWM_NUMBER_OF_IMMEDIATE_REQUESTS; modeReqIndex++) {
        if (TRUE == BswM_DelayedRequest[modeReqIndex]) {
            BswM_Internal_EvaluateRulesForImmediateRequest(modeReqIndex);
        }
    }
}
#endif

static inline boolean BswM_Internal_AttemptToProcessRequest ( void ) {
    boolean okToProcess = FALSE; 

    if (FALSE == BswM_RequestProcessingOngoing) {
        okToProcess = TRUE;
        BswM_RequestProcessingOngoing = TRUE;
    }
    
    return okToProcess;
}

static inline void BswM_Internal_EndRequestProcessing ( void ) {
    BswM_RequestProcessingOngoing = FALSE;
}

/* !req BswM0002 */
/* @req BswM0045 */
/* @req BswM0088 */
void BswM_Init(const BswM_ConfigType* ConfigPtr) {
    /* Validation of parameter: In the failure case when DET is enabled this function will report DET error and return */
	BSWM_VALIDATE_CFGPOINTER_NORV(ConfigPtr, BSWM_SERVICEID_INIT);
    BSWM_VALIDATE_CFGPARAMRANGE_NORV((NULL != ConfigPtr->RuleCfg), BSWM_SERVICEID_INIT);
    BSWM_VALIDATE_CFGPARAMRANGE_NORV((BSWM_NUMBER_OF_RULES_MAX >= ConfigPtr->RuleCfg->NumberOfRules), BSWM_SERVICEID_INIT);

    BswM_ConfigPtr = ConfigPtr;
    BswM_RuleConfigPtr = ConfigPtr->RuleCfg;
    BswM_Rules = ConfigPtr->RuleCfg->RuleData;
    
    uint32 i;

#if (BSWM_NUMBER_OF_RULES_MAX > 0)
    for (i = 0; i < BswM_RuleConfigPtr->NumberOfRules; i++) {
        BswM_RuleStates[i] = BswM_Rules[i].RuleStateInit;
    }
#endif

#if (BSWM_NUMBER_OF_REQUESTS > 0)
    for (i = 0; i < BSWM_NUMBER_OF_REQUESTS; i++) {
        BswM_ModeReqMirrors[i] = ConfigPtr->ModeRequestData[i];
    }
#endif
    
#if (BSWM_NUMBER_OF_IMMEDIATE_REQUESTS > 0)
    for (i = 0; i < BSWM_NUMBER_OF_IMMEDIATE_REQUESTS; i++) {
        BswM_DelayedRequest[i] = FALSE;
    }
#endif

#if (BSWM_PDUGROUPSWITCH_ACTIONS > 0)
    BswM_PduGroupSwitchActionPerformed = FALSE;
    BswM_ComIpduInitialize = FALSE;
#endif

#if (BSWM_DEADLINEMONITORING_ACTIONS > 0)
    BswM_DMControlPerformed = FALSE;
#endif

    BswM_RequestProcessingOngoing = FALSE;
    BswM_Internal.InitStatus = BSWM_INIT;
}

/* !req BswM0119 */
void BswM_Deinit( void ) {
    BswM_Internal.InitStatus = BSWM_UNINIT;
}

/* @req BswM0053 */
void BswM_MainFunction( void ) {
    BSWM_VALIDATE_INIT_NORV(BSWM_SERVICEID_MAINFUNCTION); /* @req BswM0076 */

#if ((BSWM_PDUGROUPSWITCH_ACTIONS > 0) || (BSWM_DEADLINEMONITORING_ACTIONS > 0))
    static boolean firstMainFunctionExecution = TRUE;
    /* The Com module is initialzed after the BswM so this must be done in the main funciton
     * and not in BswMInit()
     */
    if (TRUE == firstMainFunctionExecution) {
#if (BSWM_PDUGROUPSWITCH_ACTIONS > 0)
        Com_ClearIpduGroupVector(BswM_ComIpduGroupVector);
#endif
#if (BSWM_DEADLINEMONITORING_ACTIONS > 0)
        Com_ClearIpduGroupVector(BswM_DMControlGroupVector);
#endif
        firstMainFunctionExecution = FALSE;
    }
#endif

    boolean requestCanBeProcessed;  

    SchM_Enter_BswM_EA_0();
    requestCanBeProcessed = BswM_Internal_AttemptToProcessRequest();
    SchM_Exit_BswM_EA_0();

    BswM_Internal_UpdateNotificationMirrors();

    /* @req BswM0075 */
    /* @req BswM0060 */
#if (BSWM_NUMBER_OF_RULES_MAX > 0)
    sint16 ruleIndex;
    sint32 actionListIndex;

    for (uint32 i = 0; i < BswM_RuleConfigPtr->NumberOfRules; i++) {
        ruleIndex = BswM_RuleConfigPtr->DeferredRules[i];

        if (0 > ruleIndex) {
            break;
        }

        actionListIndex = BswM_Internal_EvaluateRule(ruleIndex);

        if (0 <= actionListIndex) {
            BswM_Internal_IterateActionList(actionListIndex);
        }
    }
#endif

    /* @req BswM0129 */
#if (BSWM_PDUGROUPSWITCH_ACTIONS > 0)
    if (TRUE == BswM_PduGroupSwitchActionPerformed) {
        Com_IpduGroupControl(BswM_ComIpduGroupVector, BswM_ComIpduInitialize);
        BswM_PduGroupSwitchActionPerformed = FALSE;
    }
#endif
    
#if (BSWM_DEADLINEMONITORING_ACTIONS > 0)
    if (TRUE == BswM_DMControlPerformed) {
        Com_ReceptionDMControl(BswM_DMControlGroupVector);
        BswM_DMControlPerformed = FALSE;
    }
#endif

    if (TRUE == requestCanBeProcessed) {
#if (BSWM_NUMBER_OF_IMMEDIATE_REQUESTS > 0)
        BswM_Internal_ProcessDelayedRequests();
#endif
        BswM_Internal_EndRequestProcessing();
    }
}

#if (BSWM_NUMBER_OF_IMMEDIATE_REQUESTS > 0)
/**
 * @brief Try to evaluate all the rules per immediate mode request
 *
 * This should be called within the context of an immediate mode request.
 * All rules referring to the specific mode request will be evaluated.
 * @param modeReqIndex Index to the mode request source.
 * @return void
 */
void BswM_Internal_ProcessImmediateRequest( const uint32 modeReqIndex ) {
    boolean requestCanBeProcessed;  

    SchM_Enter_BswM_EA_0();
    requestCanBeProcessed = BswM_Internal_AttemptToProcessRequest();
    SchM_Exit_BswM_EA_0();

    if (TRUE == requestCanBeProcessed) {
        BswM_Internal_EvaluateRulesForImmediateRequest(modeReqIndex);
    } else {
        BswM_DelayedRequest[modeReqIndex] = TRUE;
    }

#if (BSWM_PDUGROUPSWITCH_ACTIONS > 0)
    if (TRUE == BswM_PduGroupSwitchActionPerformed) {
        Com_IpduGroupControl(BswM_ComIpduGroupVector, BswM_ComIpduInitialize);
        BswM_PduGroupSwitchActionPerformed = FALSE;
    }
#endif

#if (BSWM_DEADLINEMONITORING_ACTIONS > 0)
    if (TRUE == BswM_DMControlPerformed) {
        Com_ReceptionDMControl(BswM_DMControlGroupVector);
        BswM_DMControlPerformed = FALSE;
    }
#endif

    if (TRUE == requestCanBeProcessed) {
        BswM_Internal_ProcessDelayedRequests();
        BswM_Internal_EndRequestProcessing();
    }
}
#endif

#if (BSWM_DEADLINEMONITORING_ACTIONS > 0)
/**
 * @brief Perform a Deadline Monitoring Control action
 *
 * Enabling and disabling of deadline monitoring for one or several PDU.
 * @param item Pointer to the action list item that contains references to the PDUs.
 * @return void
 */
void BswM_Internal_DMControlAction( const BswM_ActionListItemType *item ) {
    const sint32 *pduGroup = item->EnabledPduGroupRefId;

    if (NULL != pduGroup) {
        while (*pduGroup >= 0) {
            Com_SetIpduGroup(BswM_DMControlGroupVector, (Com_IpduGroupIdType) *pduGroup++, TRUE);
            BswM_DMControlPerformed = TRUE;
        }
    }

    pduGroup = item->DisabledPduGroupRefId;

    if (NULL != pduGroup) {
        while (*pduGroup >= 0) {
            Com_SetIpduGroup(BswM_DMControlGroupVector, (Com_IpduGroupIdType) *pduGroup++, FALSE);
            BswM_DMControlPerformed = TRUE;
        }
    }
}
#endif

#if (BSWM_PDUGROUPSWITCH_ACTIONS > 0)
/**
 * @brief Perform a PDU Group Switch action
 *
 * Enabling and disabling of PDU groups.
 * @param item Pointer to the action list item that contains references to the PDUs.
 * @return void
 */
void BswM_Internal_PduGroupSwitchAction( const BswM_ActionListItemType *item ) {
    const sint32 *pduGroup = item->EnabledPduGroupRefId;

    if (NULL != pduGroup) {
        while (*pduGroup >= 0) {
            Com_SetIpduGroup(BswM_ComIpduGroupVector, (Com_IpduGroupIdType) *pduGroup++, TRUE);
            BswM_PduGroupSwitchActionPerformed = TRUE;
        }
    }

    pduGroup = item->DisabledPduGroupRefId;

    if (NULL != pduGroup) {
        while (*pduGroup >= 0) {
            Com_SetIpduGroup(BswM_ComIpduGroupVector, (Com_IpduGroupIdType) *pduGroup++, FALSE);
            BswM_PduGroupSwitchActionPerformed = TRUE;
        }
    }

    BswM_ComIpduInitialize = item->PduGroupSwitchReinit; 
}
#endif

#if (BSWM_PDUROUTERCONTROL_ACTIONS > 0)
/**
 * @brief Perform a PDU Routing Path Groups action
 *
 * Enabling and disabling of routing of Routing Path Groups in the PDU Router.
 * @param item Pointer to the action list item that contains references to PDU Routing Path Group.
 * @return void
 */
void BswM_Internal_PduRouterControlAction( const BswM_ActionListItemType *item ) {
    const sint32 *routingPathGroup = item->PduRoutingPathGroupRefId;

    while (*routingPathGroup >= 0) {
        if (BSWM_PDUR_ENABLE == item->PduRouterAction) {
            PduR_EnableRouting((PduR_RoutingPathGroupIdType) *routingPathGroup);
        } else {
            PduR_DisableRouting((PduR_RoutingPathGroupIdType) *routingPathGroup);
        }
        routingPathGroup++;
    }
}
#endif

#if (BSWM_VERSION_INFO_API == STD_ON)
void BswM_GetVersionInfo( Std_VersionInfoType* VersionInfo ) {
    BSWM_VALIDATE_POINTER_NORV(VersionInfo, BSWM_SERVICEID_GETVERSIONINFO);
    /* @req BswM0004 */
    /* @req BswM0136 */
    STD_GET_VERSION_INFO(VersionInfo,BSWM);
}
#endif /* BSWM_VERSION_INFO_API */
