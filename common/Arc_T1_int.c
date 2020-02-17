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

/* ----------------------------[includes]------------------------------------*/

#include "Os.h"
#include "arc.h"
#include "isr.h"
#include "timer.h"
#include "Arc_T1_Int.h"
#if defined(USE_CANIF)
#include "CanIf.h"
#include "CanIf_PBCfg.h"
#endif
#include "T1_AppInterface.h"
#include "T1_AppInterface.h"
#include "ARCCORE_T1_interface.h"
#include <string.h>

#if defined(CFG_T1_COREID_CBK)
/* Create your own Arc_T1_Cbk.h file with the callback that defines Arc_T1_GetCoreId() */
#include "Arc_T1_Cbk.h"
#else
#define Arc_T1_GetCoreId()  GetCoreID()
#endif

/* ----------------------------[private define]------------------------------*/
/* ----------------------------[private macro]-------------------------------*/
/* ----------------------------[private typedef]-----------------------------*/
/* ----------------------------[private function prototypes]-----------------*/
/* ----------------------------[private variables]---------------------------*/

static boolean initCalled = false;

static ArcT1Info_t isrInfo[OS_ISR_MAX_CNT];

/* ----------------------------[public functions]----------------------------*/

static boolean taskRunning[OS_TASK_CNT];
/**
 * Arc_T1_Init, function for initializing T1
 * @param
 */
void Arc_T1_Init(void) {

    if (initCalled) {
        return;
    }
    initCalled = true;

    for(uint8_t i=0;i<OS_TASK_CNT;i++){
        taskRunning[i] = FALSE;
    }

    Timer_Init();
    T1_AppInit();
}

/**
 * Arc_T1_GetInfoAboutIsr, function for retrieving info about installed ISRs
 * @param ptr to len
 */
ArcT1Info_t * Arc_T1_GetInfoAboutIsr(uint16_t *len){
    *len = Os_Arc_GetIsrCount();

    for (uint16_t i = 0; i < *len; i++) {
        isrInfo[i].id = i;
        strncpy(isrInfo[i].name,Os_IsrVarList[i].constPtr->name,OS_ARC_PCB_NAME_SIZE);
        isrInfo[i].core = Os_IsrVarList[i].constPtr->core;
        isrInfo[i].priority = Os_IsrVarList[i].constPtr->priority;
        isrInfo[i].vector = Os_IsrVarList[i].constPtr->vector;
        isrInfo[i].type = Os_IsrVarList[i].constPtr->type;
        isrInfo[i].entry = Os_IsrVarList[i].constPtr->entry;
    }
    return isrInfo;
}

/**
 * Arc_T1_MainFunction, main function. Called periodically by BSW task
 * @param
 */
void Arc_T1_MainFunction(void) {
    T1_AppHandler();
}

/**
 * Arc_T1_BackgroundFunction, background function, Called from Idle loop
 * @param
 */
void Arc_T1_BackgroundFunction(void) {
    T1_AppBackgroundHandler();
}

#if defined(USE_CANIF)
/**
 * Arc_T1_Transmit, function for sending data
 * @param data
 */
T1_status_t Arc_T1_Transmit(uint8 *data){
    T1_status_t retVal = T1_FAILED;
    Std_ReturnType status;
    PduInfoType pduInfo;

    pduInfo.SduLength=8UL;
    pduInfo.SduDataPtr = data;
    status = CanIf_Transmit( CANIF_PDU_ID_T1_TX_PDU,&pduInfo);

    if (E_OK == status) {
        retVal = T1_OK;
    }
    return retVal;
}

/**
 * Arc_T1_Rx_Cbk, called when receiving data
 * @param PduId, PduInfo
 */
void Arc_T1_Rx_Cbk(PduIdType id, PduInfoType *pduInfo){
    (void)id;
    T1_AppRxCallback( pduInfo->SduDataPtr );
}


/**
 * Arc_T1_Tx_Cbk, called at successful sending
 * @param
 */
void Arc_T1_Tx_Cbk(void){
}
#endif

/**
 * Os_WaitEventHook, called at the suspension of a task
 * @param taskId
 */
void Os_WaitEventHook(TaskType taskId){
    OSTH_SUSPEND_NOSUSP(taskId, Arc_T1_GetCoreId(), Timer_GetTicks());
}

/**
 * Os_ResumeEventHook, called at the resumption of a task
 * @param taskId
 */
void Os_ResumeEventHook(TaskType taskId){
    OSTH_RESUME_NOSUSP(taskId, Arc_T1_GetCoreId(), Timer_GetTicks());
}

/**
 * Os_ReleaseEventHook, called at the release of a waiting task
 * @param taskId
 */
void Os_ReleaseEventHook(TaskType taskId){
    OSTH_RELEASE_NOSUSP(taskId, Arc_T1_GetCoreId(), Timer_GetTicks());
}

/**
 * Os_ActivateTaskHook, called at the activation of a task
 * @param taskId
 */
void Os_ActivateTaskHook(TaskType taskId){
    OSTH_ACTIVATE_NOSUSP(taskId, Arc_T1_GetCoreId(), Timer_GetTicks());
}

/**
 * Os_StopTaskHook, called at the end of a task
 * @param taskId
 */
void Os_StopTaskHook(TaskType taskId){
    taskRunning[taskId] = FALSE;
    OSTH_STOP_NOSUSP(taskId, Arc_T1_GetCoreId(), Timer_GetTicks());
}

/**
 * Os_StartTaskHook, called at the start of a task
 * @param taskId
 */
void Os_StartTaskHook(TaskType taskId){
    if(taskRunning[taskId] == FALSE){
        taskRunning[taskId] = TRUE;
        OSTH_START_NOSUSP(taskId, Arc_T1_GetCoreId(), Timer_GetTicks());
    }
}

/**
 * Os_StartStopTaskHook, called at the end of one task and start of another
 * @param taskIdStop, taskIdStart
 */
void Os_StartStopTaskHook(TaskType taskIdStop, TaskType taskIdStart){
    OSTH_STOP_START_NOSUSP(taskIdStop, taskIdStart, Arc_T1_GetCoreId(), Timer_GetTicks());
}

/**
 * Os_PreIsrHook, called at the start of an ISR. Add offset for OS_TASK_CNT since
 * T1 require continues unique id task and isr
 * @param isr
 */
void Os_PreIsrHook(ISRType isr) {
    OSTH_START_NOSUSP(isr+OS_TASK_CNT, Arc_T1_GetCoreId(), Timer_GetTicks());
}

/**
 * Os_PostIsrHook, called at the stop of an ISR. Add offset for OS_TASK_CNT since
 * T1 require continues unique id task and isr
 * @param isr
 */
void Os_PostIsrHook(ISRType isr) {
    OSTH_STOP_NOSUSP(isr+OS_TASK_CNT, Arc_T1_GetCoreId(), Timer_GetTicks());
}


