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

/* General requirements tagging */
/* @req FrIf05074 */ /* Dependency on other modules */
/* @req FrIf05150 */ /* source in FrIf.c and precompile time config in FrIf_Cfg.c */
/* @req FrIf05081 */ /* Naming conventions for file FrIf_ */
/* @req FrIf05115 */ /* Two states defined - online & offline */
/* @req FrIf05096 */ /* FrIf executable code shall be independent of CC and Trcv */
/* @req FrIf05078 */ /* Apis should be implemented as C code */
/* @req FrIf05080 */ /* HIS subset MISRA C standard conformance */
/* @req FrIf05089 */ /* A xml file contains vendor specific information. This is a part of our modef */
/* @req FrIf05079 */ /* Generated files are in human readable format  */
/* @req FrIf05121 */ /* A Fr frame consists of multiple I-Pdu */
/* @req FrIf05126 */ /* Update bits occupy arbitrary position */
/* @req FrIf05084 */ /* Det error reporting can be configurable */
/* @req FrIf05083 */ /* FrIf_camelCase naming convention used for global variables or Apis */
/* @req FrIf05001 */ /* Imported types used */
/* @req FrIf05043 */ /* Mandatory Fr interfaces */
/* @req FrIf05044 */ /* Optional Apis */
/* @req FrIf05045 */ /* Upper layer indications call-back */
/* @req FrIf05046 */ /* Upper layer tx confirmation call-back*/
/* @req FrIf05047 */ /* Upper layer trigger transmit call-back */
/* @req FrIf05282 */ /* VARIANT-PRE-COMPILE params are precompile time configurable  */
/* @req FrIf06117 */ /* Published info */
/* @req FrIf05083 */ /* FrIf_camelCase naming convention used for global variables and Apis */
/* @req FrIf05060 */ /* Zero based index for Fr CC */
/* @req FrIf05053 */ /* Multiple FrIf Ctrl Idx is configurable*/
/* @req FrIf05112 */ /* Multiple FrIf clusters Idx are configurable*/
/* @req FrIf05113 */ /* Atleast one absolute timer is supported */
/* @req FrIf05129 */ /* If immediate transmission is allowed one Pdu per Frame */
/* @req FrIf05052 */ /* Zero based indexing for Fr Ctrl Id & Driver */
/* @req FrIf05077 */ /* configuration parameters are organized as containers with particular multiplicity */
/* @req FrIf05091c */ /* Dem configuration defines the Event Ids */
/* @req FrIf05091d */ /* Dem configuration generates Dem_IntErrId.h */
/* @req FrIf05297 */ /* Detection of production error codes cannot be switched off */
#include <string.h>
#include "Std_Types.h"
#include "FrIf.h"
#include "FrIf_Internal.h"
#include "Fr.h"  /* @req FrIf05076d */
#include "PduR_FrIf.h" /* @req FrIf05076h */
/* @req FrIf05065 */
/* @req FrIf05076l */
#if (FRIF_DEV_ERROR_DETECT == STD_ON)
#include "Det.h"
#endif
/* @req FrIf05091a */
/* @req FrIf05091b */
/* @req FrIf05066 */
#if defined(USE_DEM)
#include "Dem.h"
#endif
#if defined (USE_FRNM)
#include "FrNm_Cbk.h" /* @req FrIf05076i */
#endif
#include "SchM_FrIf.h" /* @req FrIf05076m */ /* @req  FrIf05087 */

/* pointer to main configuration */
const FrIf_ConfigType *FrIf_ConfigCachePtr;

/* Global runtime parameters */
FrIf_Internal_GlobalType FrIf_Internal_Global;



/** APIs */

/* @req FrIf05003 */
/**
 *
 * @param FrIf_ConfigPtr
 */
void FrIf_Init(const FrIf_ConfigType* FrIf_ConfigPtr)
{

    uint8 txPduCnt;
    uint8 i;

    /* @req FrIf05155 */
    FRIF_DET_REPORTERROR((FrIf_ConfigPtr != NULL),FRIF_INIT_API_ID,FRIF_E_INV_POINTER);
    FrIf_ConfigCachePtr = FrIf_ConfigPtr;

    /* @req FrIf05156 */
    txPduCnt = FrIf_ConfigCachePtr->FrIf_TxPduCount;
    for (i = 0; i < txPduCnt; i++) {
        FrIf_Internal_Global.txPduStatistics[i].trigTxCounter = 0;
        FrIf_Internal_Global.txPduStatistics[i].txConfCounter = 0;
    }

    for (i = 0; i < FrIf_ConfigCachePtr->FrIf_RxPduCount; i++) {
        FrIf_Internal_Global.rxPduStatistics[i].pduUpdated = FALSE;
        FrIf_Internal_Global.rxPduStatistics[i].pduRxLen = 0;
    }

    for (i = 0; i < FrIf_ConfigCachePtr->FrIf_ClusterCount; i++) {
        /* @req FrIf05117 */
        FrIf_Internal_Global.clstrRunTimeData[i].clstrState = FRIF_STATE_OFFLINE;
        FrIf_Internal_Global.clstrRunTimeData[i].jobListCntr = 0;
        FrIf_Internal_Global.clstrRunTimeData[i].jobListSyncLost = TRUE;
        FrIf_Internal_Global.clstrRunTimeData[i].jobListExecLock = FALSE;
        FrIf_Internal_Global.clstrRunTimeData[i].interruptEnableSts = FALSE;
    }
    memset(FrIf_Internal_Global.lSduBuffer,0,(FRIF_MAX_N_LPDU * FRIF_MAX_LPDU_LEN)); /* Reset LSdu buffer */
    FrIf_Internal_Global.initDone = TRUE;
}


/* @req FrIf05004 */
/**
 *
 * @param FrIf_CtrlIdx
 * @return
 */
Std_ReturnType FrIf_ControllerInit(uint8 FrIf_CtrlIdx)
{

    uint8 frIdx;
    const FrIf_FrAPIType *myFuncPtr;
    Std_ReturnType ret;

    ret = E_NOT_OK;

    /* @req FrIf05298 */
    /* @req FrIf05160 */
    FRIF_DET_REPORTERROR((TRUE == FrIf_Internal_Global.initDone),FRIF_CONTROLLER_INIT_API_ID,FRIF_E_NOT_INITIALIZED,E_NOT_OK);

    /* @req FrIf05158 */
    FRIF_DET_REPORTERROR((FrIf_CtrlIdx < FrIf_ConfigCachePtr->FrIf_CtrlCount),FRIF_CONTROLLER_INIT_API_ID,FRIF_E_INV_CTRL_IDX,E_NOT_OK);

    /* @req FrIf05159 */
    /* 1) translate to Fr idx using FrIf idx */
    frIdx = FrIf_ConfigCachePtr->FrIf_CtrlPtr[FrIf_CtrlIdx].FrIf_FrCtrlIdx;

    /* 2) locate this driver api*/
    myFuncPtr = (FrIf_ConfigCachePtr->Fr_Driver_ConfigPtr[FRIF_DRIVER_0].FrIf_FrAPIPtr);

    if (myFuncPtr != NULL) {
        /* 3) Call this driver Init-function passing correct config struct */
        ret = (*myFuncPtr->Fr_ControllerInit)(frIdx);
    }

    return ret;
}

/**
 *
 * @param FrIf_CtrlIdx
 * @param FrIf_AbsTimerIdx
 * @param FrIf_Cycle
 * @param FrIf_Offset
 * @return
 */
/* @req FrIf05021 */
Std_ReturnType FrIf_SetAbsoluteTimer(uint8 FrIf_CtrlIdx, uint8 FrIf_AbsTimerIdx, uint8 FrIf_Cycle, uint16 FrIf_Offset)
{
    const FrIf_JobListType * jobListCfg;
    const FrIf_FrAPIType *myFuncPtr;
    uint8 frIdx;
    Std_ReturnType ret;
    uint8 clstrIdx;
    uint8 frAbsTimerIdx;

    ret = E_NOT_OK;

    /* @req FrIf05298 */
    /* @req FrIf05236 */
    FRIF_DET_REPORTERROR((TRUE == FrIf_Internal_Global.initDone),FRIF_SET_ABSOLUTE_TIMER_API_ID,FRIF_E_NOT_INITIALIZED,E_NOT_OK);

    /* @req FrIf05234 */
    FRIF_DET_REPORTERROR((FrIf_CtrlIdx < FrIf_ConfigCachePtr->FrIf_CtrlCount),FRIF_SET_ABSOLUTE_TIMER_API_ID,FRIF_E_INV_CTRL_IDX,E_NOT_OK);

    FRIF_DET_REPORTERROR((FrIf_AbsTimerIdx < FRIF_MAX_ABSOLUTE_TIMER),FRIF_SET_ABSOLUTE_TIMER_API_ID,FRIF_E_INV_TIMER_IDX,E_NOT_OK);

    /* @req FrIf05235 */
    /* 1) translate to Fr idx using FrIf idx */
    frIdx = FrIf_ConfigCachePtr->FrIf_CtrlPtr[FrIf_CtrlIdx].FrIf_FrCtrlIdx;
    clstrIdx = FrIf_ConfigCachePtr->FrIf_CtrlPtr[FrIf_CtrlIdx].FrIf_ClstrRefIdx;
    jobListCfg = FrIf_ConfigCachePtr->FrIf_ClusterPtr[clstrIdx].FrIf_JobListPtr;

    FRIF_DET_REPORTERROR((jobListCfg->FrIf_AbsTimerIdx == FrIf_AbsTimerIdx),FRIF_SET_ABSOLUTE_TIMER_API_ID,FRIF_E_INV_TIMER_IDX,E_NOT_OK);

    frAbsTimerIdx = jobListCfg->FrIf_FrAbsTimerIdx;
    /* 2) locate driver api*/
    myFuncPtr = (FrIf_ConfigCachePtr->Fr_Driver_ConfigPtr[FRIF_DRIVER_0].FrIf_FrAPIPtr);

    if (myFuncPtr != NULL) {
        /* 3) Call this driver set absolute timer api passing correct config struct */
        ret = (*myFuncPtr->Fr_SetAbsoluteTimer)(frIdx, frAbsTimerIdx, FrIf_Cycle, FrIf_Offset);
    }

    return ret;

}

/**
 *
 * @param FrIf_CtrlIdx
 * @param FrIf_AbsTimerIdx
 * @return
 */
/* @req FrIf05025 */
Std_ReturnType FrIf_EnableAbsoluteTimerIRQ(uint8 FrIf_CtrlIdx, uint8 FrIf_AbsTimerIdx)
{
    const FrIf_JobListType * jobListCfg;
    const FrIf_FrAPIType  *myFuncPtr;
    uint8 frIdx;
    Std_ReturnType ret;
    uint8 clstrIdx;
    uint8 frAbsTimerIdx;

    ret = E_NOT_OK;

    /* @req FrIf05298 */
    /* @req FrIf05248 */
    FRIF_DET_REPORTERROR((TRUE == FrIf_Internal_Global.initDone),FRIF_ENABLE_ABSOLUTE_TIMER_IRQ_API_ID,FRIF_E_NOT_INITIALIZED,E_NOT_OK);

    /* @req FrIf05246 */
    FRIF_DET_REPORTERROR((FrIf_CtrlIdx < FrIf_ConfigCachePtr->FrIf_CtrlCount),FRIF_ENABLE_ABSOLUTE_TIMER_IRQ_API_ID,FRIF_E_INV_CTRL_IDX,E_NOT_OK);

    FRIF_DET_REPORTERROR((FrIf_AbsTimerIdx < FRIF_MAX_ABSOLUTE_TIMER),FRIF_ENABLE_ABSOLUTE_TIMER_IRQ_API_ID,FRIF_E_INV_TIMER_IDX,E_NOT_OK);

    /* @req FrIf05247 */
    /* 1) locate the FrIdx */
    frIdx = FrIf_ConfigCachePtr->FrIf_CtrlPtr[FrIf_CtrlIdx].FrIf_FrCtrlIdx;

    clstrIdx = FrIf_ConfigCachePtr->FrIf_CtrlPtr[FrIf_CtrlIdx].FrIf_ClstrRefIdx;

    jobListCfg = FrIf_ConfigCachePtr->FrIf_ClusterPtr[clstrIdx].FrIf_JobListPtr;

    FRIF_DET_REPORTERROR((jobListCfg->FrIf_AbsTimerIdx == FrIf_AbsTimerIdx),FRIF_ENABLE_ABSOLUTE_TIMER_IRQ_API_ID,FRIF_E_INV_TIMER_IDX,E_NOT_OK);
    frAbsTimerIdx = jobListCfg->FrIf_FrAbsTimerIdx;

    /* Check if interrupts are already enabled */
    if (TRUE == FrIf_Internal_Global.clstrRunTimeData[clstrIdx].interruptEnableSts) {
        ret = E_OK;
    } else {
        /* 2) locate driver api*/
        myFuncPtr = (FrIf_ConfigCachePtr->Fr_Driver_ConfigPtr[FRIF_DRIVER_0].FrIf_FrAPIPtr);

        if (myFuncPtr != NULL) {
            /* 3) Call this driver enable absolute timer interrupt api passing correct config struct */
            ret = (*myFuncPtr->Fr_EnableAbsoluteTimerIRQ)(frIdx, frAbsTimerIdx);
        }

        if (E_OK == ret) {
            FrIf_Internal_Global.clstrRunTimeData[clstrIdx].interruptEnableSts = TRUE;
        }
    }
    return ret;
}

/**
 *
 * @param FrIf_CtrlIdx
 * @param FrIf_AbsTimerIdx
 * @return
 */
/* @req FrIf05029 */
Std_ReturnType FrIf_AckAbsoluteTimerIRQ(uint8 FrIf_CtrlIdx, uint8 FrIf_AbsTimerIdx)
{
    const FrIf_JobListType * jobListCfg;
    const FrIf_FrAPIType  *myFuncPtr;
    uint8 frIdx;
    Std_ReturnType ret;
    uint8 clstrIdx;
    uint8 frAbsTimerIdx;

    ret = E_NOT_OK;

    /* @req FrIf05298 */
    /* @req FrIf05260 */
    FRIF_DET_REPORTERROR((TRUE == FrIf_Internal_Global.initDone),FRIF_ACK_ABSOLUTE_TIMER_IRQ_API_ID,FRIF_E_NOT_INITIALIZED,E_NOT_OK);

    /* @req FrIf05258 */
    FRIF_DET_REPORTERROR((FrIf_CtrlIdx < FrIf_ConfigCachePtr->FrIf_CtrlCount),FRIF_ACK_ABSOLUTE_TIMER_IRQ_API_ID,FRIF_E_INV_CTRL_IDX,E_NOT_OK);

    FRIF_DET_REPORTERROR((FrIf_AbsTimerIdx < FRIF_MAX_ABSOLUTE_TIMER),FRIF_ACK_ABSOLUTE_TIMER_IRQ_API_ID,FRIF_E_INV_TIMER_IDX,E_NOT_OK);

    /* @req FrIf05259 */
    /* 1) locate the FrIdx */
    frIdx = FrIf_ConfigCachePtr->FrIf_CtrlPtr[FrIf_CtrlIdx].FrIf_FrCtrlIdx;

    clstrIdx = FrIf_ConfigCachePtr->FrIf_CtrlPtr[FrIf_CtrlIdx].FrIf_ClstrRefIdx;
    jobListCfg = FrIf_ConfigCachePtr->FrIf_ClusterPtr[clstrIdx].FrIf_JobListPtr;

    FRIF_DET_REPORTERROR((jobListCfg->FrIf_AbsTimerIdx == FrIf_AbsTimerIdx),FRIF_ACK_ABSOLUTE_TIMER_IRQ_API_ID,FRIF_E_INV_TIMER_IDX,E_NOT_OK);

    frAbsTimerIdx = jobListCfg->FrIf_FrAbsTimerIdx;
    /* 2) locate this driver api*/
    myFuncPtr = (FrIf_ConfigCachePtr->Fr_Driver_ConfigPtr[FRIF_DRIVER_0].FrIf_FrAPIPtr);

    if (myFuncPtr != NULL) {
        /* 3) Call this driver acknowledge absolute timer interrupt api passing correct config struct */
        ret = (*myFuncPtr->Fr_AckAbsoluteTimerIRQ)(frIdx, frAbsTimerIdx);
    }

    return ret;

}


/* @req FrIf05005 */
/**
 *
 * @param FrIf_CtrlIdx
 * @return
 */
Std_ReturnType FrIf_StartCommunication(uint8 FrIf_CtrlIdx)
{

    Std_ReturnType ret;
    uint8 frIdx;
    const FrIf_FrAPIType  *myFuncPtr;

    ret = E_NOT_OK;
    /* @req FrIf05298 */
    /* @req FrIf05163 */
    FRIF_DET_REPORTERROR((TRUE == FrIf_Internal_Global.initDone),FRIF_START_COMMUNICATION_API_ID,FRIF_E_NOT_INITIALIZED,E_NOT_OK);

    /* @req FrIf05161 */
    FRIF_DET_REPORTERROR((FrIf_CtrlIdx < FrIf_ConfigCachePtr->FrIf_CtrlCount),FRIF_START_COMMUNICATION_API_ID,FRIF_E_INV_CTRL_IDX,E_NOT_OK);

    /* @req FrIf05162 */
    /* 1) translate to Fr idx using FrIf idx */
    frIdx = FrIf_ConfigCachePtr->FrIf_CtrlPtr[FrIf_CtrlIdx].FrIf_FrCtrlIdx;

    /* 2) locate this driver api*/
    myFuncPtr = (FrIf_ConfigCachePtr->Fr_Driver_ConfigPtr[FRIF_DRIVER_0].FrIf_FrAPIPtr);

    if (myFuncPtr != NULL) {
        /* 3) Call this driver start communication api passing correct config struct */
        ret = (*myFuncPtr->Fr_StartCommunication)(frIdx);
    }

    return ret;
}


/* @req FrIf05006 */
/**
 *
 * @param FrIf_CtrlIdx
 * @return
 */
Std_ReturnType FrIf_HaltCommunication( uint8 FrIf_CtrlIdx )
{
    Std_ReturnType ret;
    uint8 frIdx;
    const FrIf_FrAPIType  *myFuncPtr;

    ret = E_NOT_OK;

    /* @req FrIf05298 */
    /* @req FrIf05166 */
    FRIF_DET_REPORTERROR((TRUE == FrIf_Internal_Global.initDone),FRIF_HALT_COMMUNICATION_API_ID,FRIF_E_NOT_INITIALIZED,E_NOT_OK);

    /* @req FrIf05164 */
    FRIF_DET_REPORTERROR((FrIf_CtrlIdx < FrIf_ConfigCachePtr->FrIf_CtrlCount),FRIF_HALT_COMMUNICATION_API_ID,FRIF_E_INV_CTRL_IDX,E_NOT_OK);

    /* @req FrIf05165 */
    /* 1) translate to Fr idx using FrIf idx */
    frIdx = FrIf_ConfigCachePtr->FrIf_CtrlPtr[FrIf_CtrlIdx].FrIf_FrCtrlIdx;

    /* 2) locate this driver api*/
    myFuncPtr = (FrIf_ConfigCachePtr->Fr_Driver_ConfigPtr[FRIF_DRIVER_0].FrIf_FrAPIPtr);

    if (myFuncPtr != NULL) {
        /* 3) Call this driver halt communication api passing correct config struct */
        ret = (*myFuncPtr->Fr_HaltCommunication)(frIdx);
    }

    return ret;
}

/* @req FrIf05007 */
/**
 *
 * @param FrIf_CtrlIdx
 * @return
 */
Std_ReturnType FrIf_AbortCommunication( uint8 FrIf_CtrlIdx )
{
    Std_ReturnType ret;
    uint8 frIdx;
    const FrIf_FrAPIType  *myFuncPtr;

    ret = E_NOT_OK;

    /* @req FrIf05298 */
    /* @req FrIf05169 */
    FRIF_DET_REPORTERROR((TRUE == FrIf_Internal_Global.initDone),FRIF_ABORT_COMMUNICATION_API_ID,FRIF_E_NOT_INITIALIZED,E_NOT_OK);

    /* @req FrIf05167 */
    FRIF_DET_REPORTERROR((FrIf_CtrlIdx < FrIf_ConfigCachePtr->FrIf_CtrlCount),FRIF_ABORT_COMMUNICATION_API_ID,FRIF_E_INV_CTRL_IDX,E_NOT_OK);

    /* @req FrIf05168 */
    /* 1) translate to Fr idx using FrIf idx */
    frIdx = FrIf_ConfigCachePtr->FrIf_CtrlPtr[FrIf_CtrlIdx].FrIf_FrCtrlIdx;

    /* 2) locate this driver api*/
    myFuncPtr = (FrIf_ConfigCachePtr->Fr_Driver_ConfigPtr[FRIF_DRIVER_0].FrIf_FrAPIPtr);

    if (myFuncPtr != NULL) {
        /* 3) Call this driver abort communication api by passing correct config struct */
        ret = (*myFuncPtr->Fr_AbortCommunication)(frIdx);
    }

    return ret;
}


/* @req FrIf05170 */
/**
 *
 * @param FrIf_ClstIdx
 * @param FrIf_StatePtr
 * @return
 */
Std_ReturnType FrIf_GetState( uint8 FrIf_ClstIdx, FrIf_StateType* FrIf_StatePtr )
{
    /* @req FrIf05298 */
    /* @req FrIf05173 */
    FRIF_DET_REPORTERROR((TRUE == FrIf_Internal_Global.initDone),FRIF_GET_STATE_API_ID,FRIF_E_NOT_INITIALIZED,E_NOT_OK);

    /* @req FrIf05171 */
    FRIF_DET_REPORTERROR((FrIf_ClstIdx < FrIf_ConfigCachePtr->FrIf_ClusterCount),FRIF_GET_STATE_API_ID,FRIF_E_INV_CLST_IDX,E_NOT_OK);

    /* @req FrIf05172 */
    FRIF_DET_REPORTERROR((FrIf_StatePtr != NULL),FRIF_GET_STATE_API_ID,FRIF_E_INV_POINTER,E_NOT_OK);

    *FrIf_StatePtr = FrIf_Internal_Global.clstrRunTimeData[FrIf_ClstIdx].clstrState;
    return E_OK;
}


/* @req FrIf05174 */
/**
 *
 * @param FrIf_ClstIdx
 * @param FrIf_StateTransition
 * @return
 */
Std_ReturnType FrIf_SetState(uint8 FrIf_ClstIdx, FrIf_StateTransitionType FrIf_StateTransition)
{
    Std_ReturnType ret;
    ret = E_NOT_OK;
    const FrIf_ClusterType * clstrCfg;                      /* Pointer to cluster cfg */
    uint8 frIfIdx;

    /* @req FrIf05298 */
    /* @req FrIf05176 */
    FRIF_DET_REPORTERROR((TRUE == FrIf_Internal_Global.initDone),FRIF_SET_STATE_API_ID,FRIF_E_NOT_INITIALIZED,E_NOT_OK);

    /* @req FrIf05175 */
    FRIF_DET_REPORTERROR((FrIf_ClstIdx < FrIf_ConfigCachePtr->FrIf_ClusterCount),FRIF_SET_STATE_API_ID,FRIF_E_INV_CLST_IDX,E_NOT_OK);

    /* @req FrIf05118 */
    if ((FrIf_StateTransition == FRIF_GOTO_ONLINE) && (FrIf_Internal_Global.clstrRunTimeData[FrIf_ClstIdx].clstrState == FRIF_STATE_OFFLINE)) {
        FrIf_Internal_Global.clstrRunTimeData[FrIf_ClstIdx].clstrState = FRIF_STATE_ONLINE;
        ret = E_OK;
    } else if ((FrIf_StateTransition == FRIF_GOTO_OFFLINE) && (FrIf_Internal_Global.clstrRunTimeData[FrIf_ClstIdx].clstrState == FRIF_STATE_ONLINE)) {

        clstrCfg = &FrIf_ConfigCachePtr->FrIf_ClusterPtr[FrIf_ClstIdx];
        frIfIdx = clstrCfg->FrIf_ControllerPtr[FIRST_FRIF_CTRL_IDX]->FrIf_CtrlIdx; /* Find FrIf Controller Id for the first controller in the cluster */
        /* Cancel timer */
        (void)FrIf_CancelAbsoluteTimer(frIfIdx, clstrCfg->FrIf_JobListPtr->FrIf_AbsTimerIdx);
        FrIf_Internal_Global.clstrRunTimeData[FrIf_ClstIdx].jobListSyncLost = TRUE;
        FrIf_Internal_Global.clstrRunTimeData[FrIf_ClstIdx].clstrState = FRIF_STATE_OFFLINE;
        ret = E_OK;
    } else {
        ret = E_NOT_OK;
    }
    return ret;
}


/* @req FrIf05010 */
/**
 *
 * @param FrIf_CtrlIdx
 * @param FrIf_ChnlIdx
 * @return
 */
Std_ReturnType FrIf_SetWakeupChannel(uint8 FrIf_CtrlIdx, Fr_ChannelType FrIf_ChnlIdx)
{

    uint8 frIdx;
    const FrIf_FrAPIType  *myFuncPtr;
    Std_ReturnType ret;
    boolean tmp;

    ret = E_NOT_OK;

    /* @req FrIf05298 */
    /* @req FrIf05179 */
    FRIF_DET_REPORTERROR((TRUE == FrIf_Internal_Global.initDone),FRIF_SET_WUP_CHANNEL_API_ID,FRIF_E_NOT_INITIALIZED,E_NOT_OK);

    /* @req FrIf05500 */
    FRIF_DET_REPORTERROR((FrIf_CtrlIdx < FrIf_ConfigCachePtr->FrIf_CtrlCount),FRIF_SET_WUP_CHANNEL_API_ID,FRIF_E_INV_CTRL_IDX,E_NOT_OK);

    /* @req FrIf05177 */
    /* @req FrIf05264 */
    tmp = (FrIf_ChnlIdx == FR_CHANNEL_A) || (FrIf_ChnlIdx == FR_CHANNEL_B);
    FRIF_DET_REPORTERROR((TRUE == tmp ),FRIF_SET_WUP_CHANNEL_API_ID,FRIF_E_INV_CHNL_IDX,E_NOT_OK);

    /* @req FrIf05178 */
    /* 1) translate to Fr idx using FrIf idx */
    frIdx = FrIf_ConfigCachePtr->FrIf_CtrlPtr[FrIf_CtrlIdx].FrIf_FrCtrlIdx;

    /* 2) locate this driver api*/
    myFuncPtr = (FrIf_ConfigCachePtr->Fr_Driver_ConfigPtr[FRIF_DRIVER_0].FrIf_FrAPIPtr);

    if (myFuncPtr != NULL) {
        /* 3) Call this driver function Fr_SetWakeupChannel() */
        ret = (*myFuncPtr->Fr_SetWakeupChannel)(frIdx, FrIf_ChnlIdx);
    }

    return ret;
}


/* @req FrIf05011 */
/**
 *
 * @param FrIf_CtrlIdx
 * @return
 */
Std_ReturnType FrIf_SendWUP( uint8 FrIf_CtrlIdx ) {

    uint8 frIdx;
    const FrIf_FrAPIType  *myFuncPtr;
    Std_ReturnType ret;

    ret = E_NOT_OK;

    /* @req FrIf05298 */
    /* @req FrIf05182 */
    FRIF_DET_REPORTERROR((TRUE == FrIf_Internal_Global.initDone),FRIF_SEND_WUP_API_ID,FRIF_E_NOT_INITIALIZED,E_NOT_OK);

    /* @req FrIf05180 */
    FRIF_DET_REPORTERROR((FrIf_CtrlIdx < FrIf_ConfigCachePtr->FrIf_CtrlCount),FRIF_SEND_WUP_API_ID,FRIF_E_INV_CTRL_IDX,E_NOT_OK);

    /* @req FrIf05181 */
    /* 1) translate to Fr idx using FrIf idx */
    frIdx = FrIf_ConfigCachePtr->FrIf_CtrlPtr[FrIf_CtrlIdx].FrIf_FrCtrlIdx;

    /* 2) locate this driver api*/
    myFuncPtr = (FrIf_ConfigCachePtr->Fr_Driver_ConfigPtr[FRIF_DRIVER_0].FrIf_FrAPIPtr);

    if (myFuncPtr != NULL) {
        /* 3) Call this driver function Fr_SendWUP() */
        ret = (*myFuncPtr->Fr_SendWUP)(frIdx);
    }

    return ret;
}


/* @req FrIf05014 */
/**
 *
 * @param FrIf_CtrlIdx
 * @param FrIf_POCStatusPtr
 * @return
 */
Std_ReturnType FrIf_GetPOCStatus(uint8 FrIf_CtrlIdx, Fr_POCStatusType *FrIf_POCStatusPtr)
{
    uint8 frIdx;
    const FrIf_FrAPIType *myFuncPtr;
    Std_ReturnType ret;

    ret = E_NOT_OK;

    /* @req FrIf05298 */
    /* @req FrIf05193 */
    FRIF_DET_REPORTERROR((TRUE == FrIf_Internal_Global.initDone),FRIF_GET_POC_STATUS_API_ID,FRIF_E_NOT_INITIALIZED,E_NOT_OK);

    /* @req FrIf05190 */
    FRIF_DET_REPORTERROR((FrIf_CtrlIdx < FrIf_ConfigCachePtr->FrIf_CtrlCount),FRIF_GET_POC_STATUS_API_ID,FRIF_E_INV_CTRL_IDX,E_NOT_OK);

    FRIF_DET_REPORTERROR((FrIf_POCStatusPtr != NULL),FRIF_GET_POC_STATUS_API_ID,FRIF_E_INV_POINTER,E_NOT_OK);

    /* @req FrIf05192 */

    /* 1) translate to Fr idx using FrIf idx */
    frIdx = FrIf_ConfigCachePtr->FrIf_CtrlPtr[FrIf_CtrlIdx].FrIf_FrCtrlIdx;

    /* 2) locate this driver api*/
    myFuncPtr = (FrIf_ConfigCachePtr->Fr_Driver_ConfigPtr[FRIF_DRIVER_0].FrIf_FrAPIPtr);

    if (myFuncPtr != NULL) {
        /* 3) Call this driver Fr_GetPOCStatus api */
        ret = (*myFuncPtr->Fr_GetPOCStatus)(frIdx, FrIf_POCStatusPtr);
    }

    return ret;
}


/* @req FrIf05015 */
/**
 *
 * @param FrIf_CtrlIdx
 * @param FrIf_CyclePtr
 * @param FrIf_MacroTickPtr
 * @return
 */
Std_ReturnType FrIf_GetGlobalTime(uint8 FrIf_CtrlIdx, uint8 *FrIf_CyclePtr, uint16 *FrIf_MacroTickPtr)
{
    uint8 frIdx;
    const FrIf_FrAPIType  *myFuncPtr;
    Std_ReturnType ret;

    ret = E_NOT_OK;

    /* @req FrIf05298 */
    /* @req FrIf05196 */
    FRIF_DET_REPORTERROR((TRUE == FrIf_Internal_Global.initDone),FRIF_GET_GLOBAL_TIME_API_ID,FRIF_E_NOT_INITIALIZED,E_NOT_OK);

    /* @req FrIf05194 */
    FRIF_DET_REPORTERROR((FrIf_CtrlIdx < FrIf_ConfigCachePtr->FrIf_CtrlCount),FRIF_GET_GLOBAL_TIME_API_ID,FRIF_E_INV_CTRL_IDX,E_NOT_OK);

    FRIF_DET_REPORTERROR(((FrIf_CyclePtr != NULL) && (FrIf_MacroTickPtr != NULL)),FRIF_GET_GLOBAL_TIME_API_ID,FRIF_E_INV_POINTER,E_NOT_OK);

    /* @req FrIf05195 */
    /* 1) translate to Fr idx using FrIf idx */
    frIdx = FrIf_ConfigCachePtr->FrIf_CtrlPtr[FrIf_CtrlIdx].FrIf_FrCtrlIdx;

    /* 2) locate this driver api*/
    myFuncPtr = (FrIf_ConfigCachePtr->Fr_Driver_ConfigPtr[FRIF_DRIVER_0].FrIf_FrAPIPtr);

    if (myFuncPtr != NULL) {
        /* 3) Call this driver Fr_GetGlobalTime Api */
        ret = (*myFuncPtr->Fr_GetGlobalTime)(frIdx, FrIf_CyclePtr, FrIf_MacroTickPtr);
    }

    return ret;
}


/* @req FrIf05017 */
/**
 *
 * @param FrIf_CtrlIdx
 * @return
 */
Std_ReturnType FrIf_AllowColdstart(uint8 FrIf_CtrlIdx)
{
    /* Variable containing the value to return when the function reaches it's end. */
    Std_ReturnType ret;
    uint8 frIdx;
    const FrIf_FrAPIType  *myFuncPtr;

    ret = E_NOT_OK;

    /* @req FrIf05298 */
    /* @req FrIf05202 */
    FRIF_DET_REPORTERROR((TRUE == FrIf_Internal_Global.initDone),FRIF_ALLOW_COLD_START_API_ID,FRIF_E_NOT_INITIALIZED,E_NOT_OK);

    /* @req FrIf05200 */
    FRIF_DET_REPORTERROR((FrIf_CtrlIdx < FrIf_ConfigCachePtr->FrIf_CtrlCount),FRIF_ALLOW_COLD_START_API_ID,FRIF_E_INV_CTRL_IDX,E_NOT_OK);

    /* @req FrIf05201 */

    /* Translate to Fr idx using FrIf idx */
    frIdx = FrIf_ConfigCachePtr->FrIf_CtrlPtr[FrIf_CtrlIdx].FrIf_FrCtrlIdx;

    /* Locate this driver's api */
    myFuncPtr = (FrIf_ConfigCachePtr->Fr_Driver_ConfigPtr[FRIF_DRIVER_0].FrIf_FrAPIPtr);

    if (myFuncPtr != NULL) {
        /* Call the determined FlexRay Driver's Fr_AllowColdstart api */
        ret = (*myFuncPtr->Fr_AllowColdstart)(frIdx);
    }

    return ret;
}


/* @req FrIf05018 */
/**
 *
 * @param FrIf_CtrlIdx
 * @return
 */
uint16 FrIf_GetMacroticksPerCycle(uint8 FrIf_CtrlIdx)
{

    uint8 clstrIdx;

    /* @req FrIf05298 */
    /* @req FrIf05204 */
    FRIF_DET_REPORTERROR((TRUE == FrIf_Internal_Global.initDone),FRIF_GET_MACROTICKS_PER_CYCLE_API_ID,FRIF_E_NOT_INITIALIZED,0);

    /* @req FrIf05203 */
    FRIF_DET_REPORTERROR((FrIf_CtrlIdx < FrIf_ConfigCachePtr->FrIf_CtrlCount),FRIF_GET_MACROTICKS_PER_CYCLE_API_ID,FRIF_E_INV_CTRL_IDX,0);

    clstrIdx = FrIf_ConfigCachePtr->FrIf_CtrlPtr[FrIf_CtrlIdx].FrIf_ClstrRefIdx;
    return FrIf_ConfigCachePtr->FrIf_ClusterPtr[clstrIdx].FrIf_GMacroPerCycle;
}


/* @req FrIf05019 */
/**
 *
 * @param FrIf_CtrlIdx
 * @return
 */
uint16 FrIf_GetMacrotickDuration( uint8 FrIf_CtrlIdx )
{
    uint8 clstrIdx;

    /* @req FrIf05298 */
    FRIF_DET_REPORTERROR((TRUE == FrIf_Internal_Global.initDone),FRIF_GET_MACROTICKS_DUR_API_ID,FRIF_E_NOT_INITIALIZED,0);

    /* @req FrIf05191 */
    FRIF_DET_REPORTERROR((FrIf_CtrlIdx < FrIf_ConfigCachePtr->FrIf_CtrlCount),FRIF_GET_MACROTICKS_DUR_API_ID,FRIF_E_INV_CTRL_IDX,0);

    clstrIdx = FrIf_ConfigCachePtr->FrIf_CtrlPtr[FrIf_CtrlIdx].FrIf_ClstrRefIdx;
    return (uint16)FrIf_ConfigCachePtr->FrIf_ClusterPtr[clstrIdx].FrIf_GdMacrotick;
}


/* @req FrIf05033 */
/**
 *
 * @param FrIf_TxPduId
 * @param FrIf_PduInfoPtr
 * @return
 */
Std_ReturnType FrIf_Transmit(PduIdType FrIf_TxPduId, const PduInfoType *FrIf_PduInfoPtr)
{

    const FrIf_TxPduType * frIfTxPduPtr;
    const FrIf_FrAPIType  *myFuncPtr;
    const FrIf_ImmediateTxPduCfgType * immTxPduCfg;
    const uint8* Fr_LSduPtr;
    Std_ReturnType ret;
    boolean resFlag;
#if (FRIF_BIG_ENDIAN_USED == STD_ON)
    const FrIf_ControllerType * ctrlCfg;
    const FrIf_FrameStructureType * frameStructureRef;
    uint8  tempBuffForBigEndian[FLEXRAY_FRAME_LEN_MAX];
#endif


    ret = E_NOT_OK;

    /* @req FrIf05298 */
    /* @req FrIf05209 */
    FRIF_DET_REPORTERROR((TRUE == FrIf_Internal_Global.initDone),FRIF_TRANSMIT_API_ID,FRIF_E_NOT_INITIALIZED,E_NOT_OK);

    /* @req FrIf05205 */
    FRIF_DET_REPORTERROR((FrIf_TxPduId < FrIf_ConfigCachePtr->FrIf_TxPduCount),FRIF_TRANSMIT_API_ID,FRIF_E_INV_TXPDUID,E_NOT_OK);

    /* @req FrIf05206 */
    FRIF_DET_REPORTERROR((FrIf_PduInfoPtr != NULL),FRIF_TRANSMIT_API_ID,FRIF_E_INV_POINTER,E_NOT_OK);

    /* @req FrIf05207 */
    FRIF_DET_REPORTERROR((FrIf_PduInfoPtr->SduDataPtr != NULL),FRIF_TRANSMIT_API_ID,FRIF_E_INV_POINTER,E_NOT_OK);

    frIfTxPduPtr = &FrIf_ConfigCachePtr->FrIf_TxPduCfgPtr[FrIf_TxPduId];

    /* @req FrIf05208 */ /* @req FrIf05295a */
    if (TRUE == frIfTxPduPtr->FrIf_Immediate) {
        SchM_Enter_FrIf_EA_0(); /* @req FrIf05148 */ /* Data consistency of lower buffer is maintained by disabling interrupts */
        /* @req FrIf05296 */
        myFuncPtr = (FrIf_ConfigCachePtr->Fr_Driver_ConfigPtr[FRIF_DRIVER_0].FrIf_FrAPIPtr);

        immTxPduCfg = &FrIf_ConfigCachePtr->FrIf_ImmediateTxPduCfgPtr[frIfTxPduPtr->FrIf_ImmediateTxCfgIdx];

        Fr_LSduPtr = FrIf_PduInfoPtr->SduDataPtr;
#if (FRIF_BIG_ENDIAN_USED == STD_ON)
        ctrlCfg = &FrIf_ConfigCachePtr->FrIf_CtrlPtr[immTxPduCfg->FrIf_FrCtrlIdx];
        frameStructureRef = ctrlCfg->FrIf_LpduPtr[immTxPduCfg->Fr_Buffer].FrIf_VBTriggeringRef->FrIf_FrameStructureRef;

        if (FRIF_BIG_ENDIAN == frameStructureRef->FrIf_ByteOrder) {
            FrIf_Internal_SwapToBigEndian(FrIf_PduInfoPtr->SduDataPtr,tempBuffForBigEndian,(uint8)FrIf_PduInfoPtr->SduLength);
            Fr_LSduPtr = &tempBuffForBigEndian[0];
        }
#endif
        if (myFuncPtr != NULL) {
            ret = myFuncPtr->Fr_TransmitTxLPdu(immTxPduCfg->FrIf_FrCtrlIdx, immTxPduCfg->Fr_Buffer, Fr_LSduPtr, (uint8)FrIf_PduInfoPtr->SduLength);
        }
        if (E_OK == ret){
            /*  Remember that a transmission for this PDU is pending if transmission confirmation is needed */
            resFlag = ((TRUE == frIfTxPduPtr->FrIf_Confirm) && (FrIf_Internal_Global.txPduStatistics[FrIf_TxPduId].txConfCounter
                    < frIfTxPduPtr->FrIf_CounterLimit));
            if (TRUE == resFlag)  {
                /* Only increment if counter limit has not been reached */
                FrIf_Internal_Global.txPduStatistics[FrIf_TxPduId].txConfCounter++;
            }
        }

        SchM_Exit_FrIf_EA_0();
    } else {

        if ((FrIf_Internal_Global.txPduStatistics[FrIf_TxPduId].trigTxCounter > frIfTxPduPtr->FrIf_CounterLimit)
                || (FrIf_Internal_Global.txPduStatistics[FrIf_TxPduId].txConfCounter > frIfTxPduPtr->FrIf_CounterLimit)) {
            ret = E_NOT_OK;
        } else {
            /* @req FrIf05124 */
            FrIf_Internal_Global.txPduStatistics[FrIf_TxPduId].trigTxCounter++;
            ret = E_OK;
        }
    }
    return ret;
}


/* !req FrIf05034 */
/**
 *
 * @param FrIf_CtrlIdx
 * @param FrIf_ChnlIdx
 * @param FrIf_TrcvMode
 * @return
 */
Std_ReturnType FrIf_SetTransceiverMode( uint8 FrIf_CtrlIdx, Fr_ChannelType FrIf_ChnlIdx, FrTrcv_TrcvModeType FrIf_TrcvMode )
{

    /* @req FrIf05298 */
    /* @req FrIf05213 */
    FRIF_DET_REPORTERROR((TRUE == FrIf_Internal_Global.initDone),FRIF_SET_TRCV_MODE_API_ID,FRIF_E_NOT_INITIALIZED,E_NOT_OK);

    /* @req FrIf05210 */
    FRIF_DET_REPORTERROR((FrIf_CtrlIdx < FrIf_ConfigCachePtr->FrIf_CtrlCount),FRIF_SET_TRCV_MODE_API_ID,FRIF_E_INV_CTRL_IDX,E_NOT_OK);

    /* @req FrIf05211 */
    FRIF_DET_REPORTERROR(((FR_CHANNEL_A==FrIf_ChnlIdx)||(FR_CHANNEL_B==FrIf_ChnlIdx)),FRIF_SET_TRCV_MODE_API_ID,FRIF_E_INV_CHNL_IDX,E_NOT_OK);

    /* !req FrIf05212 */
    (void)FrIf_TrcvMode;
    return E_NOT_OK;
}


/* !req FrIf05035 */
/**
 *
 * @param FrIf_CtrlIdx
 * @param FrIf_ChnlIdx
 * @param FrIf_TrcvModePtr
 * @return
 */
/*lint --e{818} */
Std_ReturnType FrIf_GetTransceiverMode( uint8 FrIf_CtrlIdx, Fr_ChannelType FrIf_ChnlIdx, FrTrcv_TrcvModeType* FrIf_TrcvModePtr )
{
    /* @req FrIf05298 */
    /* @req FrIf05217 */
    FRIF_DET_REPORTERROR((TRUE == FrIf_Internal_Global.initDone),FRIF_GET_TRCV_MODE_API_ID,FRIF_E_NOT_INITIALIZED,E_NOT_OK);

    /* @req FrIf05214 */
    FRIF_DET_REPORTERROR((FrIf_CtrlIdx < FrIf_ConfigCachePtr->FrIf_CtrlCount),FRIF_GET_TRCV_MODE_API_ID,FRIF_E_INV_CTRL_IDX,E_NOT_OK);

    /* @req FrIf05215 */
    FRIF_DET_REPORTERROR(((FR_CHANNEL_A==FrIf_ChnlIdx)||(FR_CHANNEL_B==FrIf_ChnlIdx)),FRIF_GET_TRCV_MODE_API_ID,FRIF_E_INV_CHNL_IDX,E_NOT_OK);

    FRIF_DET_REPORTERROR((FrIf_TrcvModePtr != NULL_PTR),FRIF_GET_TRCV_MODE_API_ID,FRIF_E_INV_POINTER,E_NOT_OK);
    /* !req FrIf05216 */
    return E_NOT_OK;
}


/* !req FrIf05036 */
/**
 *
 * @param FrIf_CtrlIdx
 * @param FrIf_ChnlIdx
 * @param FrIf_TrcvWUReasonPtr
 * @return
 */
/*lint --e{818} */
Std_ReturnType FrIf_GetTransceiverWUReason( uint8 FrIf_CtrlIdx, Fr_ChannelType FrIf_ChnlIdx, FrTrcv_TrcvWUReasonType* FrIf_TrcvWUReasonPtr )
{
    /* @req FrIf05298 */
    /* @req FrIf05221 */
    FRIF_DET_REPORTERROR((TRUE == FrIf_Internal_Global.initDone),FRIF_GET_TRCV_WU_REASON_API_ID,FRIF_E_NOT_INITIALIZED,E_NOT_OK);

    /* @req FrIf05218 */
    FRIF_DET_REPORTERROR((FrIf_CtrlIdx < FrIf_ConfigCachePtr->FrIf_CtrlCount),FRIF_GET_TRCV_WU_REASON_API_ID,FRIF_E_INV_CTRL_IDX,E_NOT_OK);

    /* @req FrIf05219 */
    FRIF_DET_REPORTERROR(((FR_CHANNEL_A==FrIf_ChnlIdx)||(FR_CHANNEL_B==FrIf_ChnlIdx)),FRIF_GET_TRCV_WU_REASON_API_ID,FRIF_E_INV_CHNL_IDX,E_NOT_OK);

    FRIF_DET_REPORTERROR((FrIf_TrcvWUReasonPtr != NULL_PTR),FRIF_GET_TRCV_WU_REASON_API_ID,FRIF_E_INV_POINTER,E_NOT_OK);

    /* !req FrIf05220 */
    return E_NOT_OK;
}


/* !req FrIf05039 */
/**
 *
 * @param FrIf_CtrlIdx
 * @param FrIf_ChnlIdx
 * @return
 */
Std_ReturnType FrIf_ClearTransceiverWakeup( uint8 FrIf_CtrlIdx, Fr_ChannelType FrIf_ChnlIdx )
{

    /* @req FrIf05298 */
    /* @req FrIf05233 */
    FRIF_DET_REPORTERROR((TRUE == FrIf_Internal_Global.initDone),FRIF_CLR_TRCV_WUP_API_ID,FRIF_E_NOT_INITIALIZED,E_NOT_OK);

    /* @req FrIf05230 */
    FRIF_DET_REPORTERROR((FrIf_CtrlIdx < FrIf_ConfigCachePtr->FrIf_CtrlCount),FRIF_CLR_TRCV_WUP_API_ID,FRIF_E_INV_CTRL_IDX,E_NOT_OK);

    /* @req FrIf05231 */
    FRIF_DET_REPORTERROR(((FR_CHANNEL_A==FrIf_ChnlIdx)||(FR_CHANNEL_B==FrIf_ChnlIdx)),FRIF_CLR_TRCV_WUP_API_ID,FRIF_E_INV_CHNL_IDX,E_NOT_OK);

    /* !req FrIf05232 */
    return E_NOT_OK;
}


/* @req FrIf05023 */
/**
 *
 * @param FrIf_CtrlIdx
 * @param FrIf_AbsTimerIdx
 * @return
 */
Std_ReturnType FrIf_CancelAbsoluteTimer( uint8 FrIf_CtrlIdx, uint8 FrIf_AbsTimerIdx )
{
    const FrIf_JobListType * jobListCfg;
    const FrIf_FrAPIType  *myFuncPtr;
    uint8 frIdx;
    Std_ReturnType ret;
    uint8 clstrIdx;
    uint8 frAbsTimerIdx;

    ret = E_NOT_OK;

    /* @req FrIf05298 */
    /* @req FrIf05242 */
    FRIF_DET_REPORTERROR((TRUE == FrIf_Internal_Global.initDone),FRIF_CANCEL_ABSOLUTE_TIMER_API_ID,FRIF_E_NOT_INITIALIZED,E_NOT_OK);

    /* @req FrIf05240 */
    FRIF_DET_REPORTERROR((FrIf_CtrlIdx < FrIf_ConfigCachePtr->FrIf_CtrlCount),FRIF_CANCEL_ABSOLUTE_TIMER_API_ID,FRIF_E_INV_CTRL_IDX,E_NOT_OK);

    FRIF_DET_REPORTERROR((FrIf_AbsTimerIdx < FRIF_MAX_ABSOLUTE_TIMER),FRIF_CANCEL_ABSOLUTE_TIMER_API_ID,FRIF_E_INV_TIMER_IDX,E_NOT_OK);
    /* @req FrIf05241 */
    /* 1) translate to Fr idx using FrIf idx */
    frIdx = FrIf_ConfigCachePtr->FrIf_CtrlPtr[FrIf_CtrlIdx].FrIf_FrCtrlIdx;

    clstrIdx = FrIf_ConfigCachePtr->FrIf_CtrlPtr[FrIf_CtrlIdx].FrIf_ClstrRefIdx;
    jobListCfg = FrIf_ConfigCachePtr->FrIf_ClusterPtr[clstrIdx].FrIf_JobListPtr;

    FRIF_DET_REPORTERROR((jobListCfg->FrIf_AbsTimerIdx == FrIf_AbsTimerIdx),FRIF_CANCEL_ABSOLUTE_TIMER_API_ID,FRIF_E_INV_TIMER_IDX,E_NOT_OK);

    frAbsTimerIdx = jobListCfg->FrIf_FrAbsTimerIdx;
    /* 2) locate driver api*/
    myFuncPtr = (FrIf_ConfigCachePtr->Fr_Driver_ConfigPtr[FRIF_DRIVER_0].FrIf_FrAPIPtr);

    if (myFuncPtr != NULL) {
        /* 3) Call this driver Fr_CancelAbsoluteTimer api */
        ret = (*myFuncPtr->Fr_CancelAbsoluteTimer)(frIdx, frAbsTimerIdx);
    }

    return ret;
}


/* @req FrIf05027 */
/**
 *
 * @param FrIf_CtrlIdx
 * @param FrIf_AbsTimerIdx
 * @param FrIf_IRQStatusPtr
 * @return
 */
Std_ReturnType FrIf_GetAbsoluteTimerIRQStatus( uint8 FrIf_CtrlIdx, uint8 FrIf_AbsTimerIdx, boolean* FrIf_IRQStatusPtr )
{
    const FrIf_JobListType * jobListCfg;
    const FrIf_FrAPIType  *myFuncPtr;
    uint8 frIdx;
    Std_ReturnType ret;
    uint8 clstrIdx;
    uint8 frAbsTimerIdx;

    ret = E_NOT_OK;

    /* @req FrIf05298 */
    /* @req FrIf05254 */
    FRIF_DET_REPORTERROR((TRUE == FrIf_Internal_Global.initDone),FRIF_GET_ABSOLUTE_TIMER_IRQ_STATUS_API_ID,FRIF_E_NOT_INITIALIZED,E_NOT_OK);

    /* @req FrIf05252 */
    FRIF_DET_REPORTERROR((FrIf_CtrlIdx < FrIf_ConfigCachePtr->FrIf_CtrlCount),FRIF_GET_ABSOLUTE_TIMER_IRQ_STATUS_API_ID,FRIF_E_INV_CTRL_IDX,E_NOT_OK);

    FRIF_DET_REPORTERROR((FrIf_IRQStatusPtr != NULL),FRIF_GET_ABSOLUTE_TIMER_IRQ_STATUS_API_ID,FRIF_E_INV_POINTER,E_NOT_OK);

    FRIF_DET_REPORTERROR((FrIf_AbsTimerIdx < FRIF_MAX_ABSOLUTE_TIMER),FRIF_GET_ABSOLUTE_TIMER_IRQ_STATUS_API_ID,FRIF_E_INV_TIMER_IDX,E_NOT_OK);

    /* @req FrIf05253 */
    /* 1) translate to Fr idx using FrIf idx */
    frIdx = FrIf_ConfigCachePtr->FrIf_CtrlPtr[FrIf_CtrlIdx].FrIf_FrCtrlIdx;

    clstrIdx = FrIf_ConfigCachePtr->FrIf_CtrlPtr[FrIf_CtrlIdx].FrIf_ClstrRefIdx;
    jobListCfg = FrIf_ConfigCachePtr->FrIf_ClusterPtr[clstrIdx].FrIf_JobListPtr;

    FRIF_DET_REPORTERROR((jobListCfg->FrIf_AbsTimerIdx == FrIf_AbsTimerIdx),FRIF_GET_ABSOLUTE_TIMER_IRQ_STATUS_API_ID,FRIF_E_INV_TIMER_IDX,E_NOT_OK);

    frAbsTimerIdx = jobListCfg->FrIf_FrAbsTimerIdx;
    /* 2) locate driver api*/
    myFuncPtr = (FrIf_ConfigCachePtr->Fr_Driver_ConfigPtr[FRIF_DRIVER_0].FrIf_FrAPIPtr);

    if (myFuncPtr != NULL) {
        /* 3) Call this driver FrIf_GetAbsoluteTimerIRQStatus api */
        ret = (*myFuncPtr->Fr_GetAbsoluteTimerIRQStatus)(frIdx, frAbsTimerIdx,FrIf_IRQStatusPtr);
    }

    return ret;
}


/* @req FrIf05031 */
/**
 *
 * @param FrIf_CtrlIdx
 * @param FrIf_AbsTimerIdx
 * @return
 */
Std_ReturnType FrIf_DisableAbsoluteTimerIRQ(uint8 FrIf_CtrlIdx, uint8 FrIf_AbsTimerIdx)
{
    const FrIf_JobListType * jobListCfg;
    const FrIf_FrAPIType  *myFuncPtr;
    uint8 frIdx;
    Std_ReturnType ret;
    uint8 clstrIdx;
    uint8 frAbsTimerIdx;

    ret = E_NOT_OK;

    /* @req FrIf05298 */
    /* @req FrIf05266 */
    FRIF_DET_REPORTERROR((TRUE == FrIf_Internal_Global.initDone),FRIF_DISABLE_ABSOLUTE_TIMER_IRQ_API_ID,FRIF_E_NOT_INITIALIZED,E_NOT_OK);

    /* @req FrIf05264 */
    FRIF_DET_REPORTERROR((FrIf_CtrlIdx < FrIf_ConfigCachePtr->FrIf_CtrlCount),FRIF_DISABLE_ABSOLUTE_TIMER_IRQ_API_ID,FRIF_E_INV_CTRL_IDX,E_NOT_OK);

    FRIF_DET_REPORTERROR((FrIf_AbsTimerIdx < FRIF_MAX_ABSOLUTE_TIMER),FRIF_DISABLE_ABSOLUTE_TIMER_IRQ_API_ID,FRIF_E_INV_TIMER_IDX,E_NOT_OK);

    /* 1) translate to Fr idx using FrIf idx */
    frIdx = FrIf_ConfigCachePtr->FrIf_CtrlPtr[FrIf_CtrlIdx].FrIf_FrCtrlIdx;

    clstrIdx = FrIf_ConfigCachePtr->FrIf_CtrlPtr[FrIf_CtrlIdx].FrIf_ClstrRefIdx;

    jobListCfg = FrIf_ConfigCachePtr->FrIf_ClusterPtr[clstrIdx].FrIf_JobListPtr;

    FRIF_DET_REPORTERROR((jobListCfg->FrIf_AbsTimerIdx == FrIf_AbsTimerIdx),FRIF_DISABLE_ABSOLUTE_TIMER_IRQ_API_ID,FRIF_E_INV_TIMER_IDX,E_NOT_OK);
    frAbsTimerIdx = jobListCfg->FrIf_FrAbsTimerIdx;

    /* Check if interrupts are already disabled */
    if (FALSE == FrIf_Internal_Global.clstrRunTimeData[clstrIdx].interruptEnableSts) {
        ret = E_OK;

    } else {
        /* 2) locate this driver api*/
        myFuncPtr = (FrIf_ConfigCachePtr->Fr_Driver_ConfigPtr[FRIF_DRIVER_0].FrIf_FrAPIPtr);

        if (myFuncPtr != NULL_PTR) {
            /* 3) Call this driver Fr_DisableAbsoluteTimerIRQ api */
            ret = (*myFuncPtr->Fr_DisableAbsoluteTimerIRQ)(frIdx, frAbsTimerIdx);
        }

        if (E_OK == ret ) {
            FrIf_Internal_Global.clstrRunTimeData[clstrIdx].interruptEnableSts = FALSE;
        }
    }
    return ret;
}


/* @req FrIf05239 */
/**
 *
 * @param FrIf_CtrlIdx
 * @return
 */
uint32 FrIf_GetCycleLength( uint8 FrIf_CtrlIdx )
{
    /* @req FrIf05298 */
    /* @req FrIf05238 */
    FRIF_DET_REPORTERROR((TRUE == FrIf_Internal_Global.initDone),FRIF_GET_CYCLE_LENGTH_API_ID,FRIF_E_NOT_INITIALIZED,0);

    /* @req FrIf05237 */
    FRIF_DET_REPORTERROR((FrIf_CtrlIdx < FrIf_ConfigCachePtr->FrIf_CtrlCount),FRIF_GET_CYCLE_LENGTH_API_ID,FRIF_E_INV_CTRL_IDX,0);

    return FrIf_ConfigCachePtr->FrIf_ClusterPtr[FrIf_ConfigCachePtr->FrIf_CtrlPtr[FrIf_CtrlIdx].FrIf_ClstrRefIdx].FrIf_GdCycle;
}

/** Optional APIs */

/* @req FrIf05412 */
#if (FRIF_ALL_SLOTS_SUPPORT == STD_ON)
/* @req FrIf05707 */
/**
 *
 * @param FrIf_CtrlIdx
 * @return
 */
Std_ReturnType FrIf_AllSlots( uint8 FrIf_CtrlIdx )
{
    /* @req FrIf05298 */
    /* @req FrIf05706 */
    FRIF_DET_REPORTERROR((TRUE == FrIf_Internal_Global.initDone),FRIF_ALL_SLOTS_API_ID,FRIF_E_NOT_INITIALIZED,E_NOT_OK);

    /* @req FrIf05707 */
    FRIF_DET_REPORTERROR((FrIf_CtrlIdx < FrIf_ConfigCachePtr->FrIf_CtrlCount),FRIF_ALL_SLOTS_API_ID,FRIF_E_INV_CTRL_IDX,E_NOT_OK);

    return Fr_AllSlots(FrIf_ConfigCachePtr->FrIf_CtrlPtr[FrIf_CtrlIdx].FrIf_FrCtrlIdx);
}
#endif


/* @req FrIf05413 */
#if (FRIF_GET_CHNL_STATUS_SUPPORT == STD_ON)
/* @req FrIf05030 */
/**
 *
 * @param FrIf_CtrlIdx
 * @param FrIf_ChannelAStatusPtr
 * @param FrIf_ChannelBStatusPtr
 * @return
 */
Std_ReturnType FrIf_GetChannelStatus( uint8 FrIf_CtrlIdx, uint16* FrIf_ChannelAStatusPtr, uint16* FrIf_ChannelBStatusPtr )
{
    uint8 frIdx;
    Std_ReturnType ret;

    /* @req FrIf05298 */
    /* @req FrIf05708 */
    FRIF_DET_REPORTERROR((TRUE == FrIf_Internal_Global.initDone),FRIF_GET_CHNL_STATUS_API_ID,FRIF_E_NOT_INITIALIZED,E_NOT_OK);

    /* @req FrIf05709 */
    FRIF_DET_REPORTERROR((FrIf_CtrlIdx < FrIf_ConfigCachePtr->FrIf_CtrlCount),FRIF_GET_CHNL_STATUS_API_ID,FRIF_E_INV_CTRL_IDX,E_NOT_OK);

    FRIF_DET_REPORTERROR(((FrIf_ChannelAStatusPtr != NULL) && (FrIf_ChannelBStatusPtr != NULL)),FRIF_GET_CHNL_STATUS_API_ID,FRIF_E_INV_POINTER,E_NOT_OK);

    frIdx = FrIf_ConfigCachePtr->FrIf_CtrlPtr[FrIf_CtrlIdx].FrIf_FrCtrlIdx;
    ret =  Fr_GetChannelStatus(frIdx,FrIf_ChannelAStatusPtr,FrIf_ChannelBStatusPtr);

    if (E_OK == ret) {
#if defined(USE_DEM)
        const FrIf_ClusterType * clstrCfg;
        clstrCfg = &FrIf_ConfigCachePtr->FrIf_ClusterPtr[FrIf_ConfigCachePtr->FrIf_CtrlPtr[FrIf_CtrlIdx].FrIf_ClstrRefIdx];
        /* @req FrIf05120b */
        /* @req FrIf05120c */     /* @req FrIf05120d */     /* @req FrIf05120e */
        /* @req FrIf05120f */     /* @req FrIf05120g */     /* @req FrIf05120h */
        if (NULL != clstrCfg->FrIf_ClusterDemEventParamRef ) {
            /* @req FrIf05300 */
            FRIF_CLUSTER_DEM_REPORTING(A);
            FRIF_CLUSTER_DEM_REPORTING(B);
        }
#endif
    } else {
        ret = E_NOT_OK;
    }
    return ret;
}
#endif

/* @req FrIf05414 */
#if (FRIF_GET_CLK_CORRECTION_SUPPORT == STD_ON)
/* @req FrIf05071 */
/**
 *
 * @param FrIf_CtrlIdx
 * @param FrIf_RateCorrectionPtr
 * @param FrIf_OffsetCorrectionPtr
 * @return
 */
Std_ReturnType FrIf_GetClockCorrection( uint8 FrIf_CtrlIdx, sint16* FrIf_RateCorrectionPtr, sint32* FrIf_OffsetCorrectionPtr )
{
    uint8 frIdx;

    /* @req FrIf05298 */
    /* @req FrIf05711 */
    FRIF_DET_REPORTERROR((TRUE == FrIf_Internal_Global.initDone),FRIF_GET_CLK_CORRECTION_API_ID,FRIF_E_NOT_INITIALIZED,E_NOT_OK);

    /* @req FrIf05712 */
    FRIF_DET_REPORTERROR((FrIf_CtrlIdx < FrIf_ConfigCachePtr->FrIf_CtrlCount),FRIF_GET_CLK_CORRECTION_API_ID,FRIF_E_INV_CTRL_IDX,E_NOT_OK);

    FRIF_DET_REPORTERROR(((FrIf_RateCorrectionPtr != NULL) && (FrIf_OffsetCorrectionPtr != NULL)) ,FRIF_GET_CLK_CORRECTION_API_ID,FRIF_E_INV_POINTER,E_NOT_OK);

    frIdx = FrIf_ConfigCachePtr->FrIf_CtrlPtr[FrIf_CtrlIdx].FrIf_FrCtrlIdx;

    return Fr_GetClockCorrection(frIdx,FrIf_RateCorrectionPtr,FrIf_OffsetCorrectionPtr);
}
#endif

/* @req FrIf05415 */
#if (FRIF_GET_SYNC_FRAME_LIST_SUPPORT == STD_ON)
/* @req FrIf05072 */
/**
 *
 * @param FrIf_CtrlIdx
 * @param FrIf_ListSize
 * @param FrIf_ChannelAEvenListPtr
 * @param FrIf_ChannelBEvenListPtr
 * @param FrIf_ChannelAOddListPtr
 * @param FrIf_ChannelBOddListPtr
 * @return
 */
Std_ReturnType FrIf_GetSyncFrameList( uint8 FrIf_CtrlIdx, uint8 FrIf_ListSize, uint16* FrIf_ChannelAEvenListPtr, uint16* FrIf_ChannelBEvenListPtr, uint16* FrIf_ChannelAOddListPtr, uint16* FrIf_ChannelBOddListPtr )
{
    uint8 frIdx;
    boolean res;

    /* @req FrIf05298 */
    /* @req FrIf05715 */
    FRIF_DET_REPORTERROR((TRUE == FrIf_Internal_Global.initDone),FRIF_GET_SYNC_FRAME_LIST_API_ID,FRIF_E_NOT_INITIALIZED,E_NOT_OK);

    /* @req FrIf05716 */
    FRIF_DET_REPORTERROR((FrIf_CtrlIdx < FrIf_ConfigCachePtr->FrIf_CtrlCount),FRIF_GET_SYNC_FRAME_LIST_API_ID,FRIF_E_INV_CTRL_IDX,E_NOT_OK);

    res = (FrIf_ChannelAEvenListPtr != NULL) && (FrIf_ChannelBEvenListPtr != NULL) && (FrIf_ChannelAOddListPtr != NULL) && (FrIf_ChannelBOddListPtr != NULL);
    FRIF_DET_REPORTERROR((TRUE == res),FRIF_GET_SYNC_FRAME_LIST_API_ID,FRIF_E_INV_POINTER,E_NOT_OK);

    frIdx = FrIf_ConfigCachePtr->FrIf_CtrlPtr[FrIf_CtrlIdx].FrIf_FrCtrlIdx;
    return Fr_GetSyncFrameList(frIdx,FrIf_ListSize,FrIf_ChannelAEvenListPtr,FrIf_ChannelBEvenListPtr,FrIf_ChannelAOddListPtr,FrIf_ChannelBOddListPtr);
}
#endif

/* @req FrIf05416 */
#if (FRIF_GET_NUM_STARTUP_FRAMES_SUPPORT == STD_ON)
/* @req FrIf05073 */
/**
 *
 * @param FrIf_CtrlIdx
 * @param FrIf_NumOfStartupFramesPtr
 * @return
 */
Std_ReturnType FrIf_GetNumOfStartupFrames( uint8 FrIf_CtrlIdx, uint8* FrIf_NumOfStartupFramesPtr )
{
    uint8 frIdx;

    /* @req FrIf05298 */
    /* @req FrIf05721 */
    FRIF_DET_REPORTERROR((TRUE == FrIf_Internal_Global.initDone),FRIF_GET_NUM_STARTUP_FRAMES_API_ID,FRIF_E_NOT_INITIALIZED,E_NOT_OK);

    /* @req FrIf05722 */
    FRIF_DET_REPORTERROR((FrIf_CtrlIdx < FrIf_ConfigCachePtr->FrIf_CtrlCount),FRIF_GET_NUM_STARTUP_FRAMES_API_ID,FRIF_E_INV_CTRL_IDX,E_NOT_OK);

    FRIF_DET_REPORTERROR((FrIf_NumOfStartupFramesPtr != NULL),FRIF_GET_NUM_STARTUP_FRAMES_API_ID,FRIF_E_INV_POINTER,E_NOT_OK);

    frIdx = FrIf_ConfigCachePtr->FrIf_CtrlPtr[FrIf_CtrlIdx].FrIf_FrCtrlIdx;
    return Fr_GetNumOfStartupFrames(frIdx,FrIf_NumOfStartupFramesPtr);
}
#endif

/* @req FrIf05417 */
#if (FRIF_GET_WUP_RX_STATUS_SUPPORT == STD_ON)
/* @req FrIf05102 */
/**
 *
 * @param FrIf_CtrlIdx
 * @param FrIf_WakeupRxStatusPtr
 * @return
 */
Std_ReturnType FrIf_GetWakeupRxStatus( uint8 FrIf_CtrlIdx, uint8* FrIf_WakeupRxStatusPtr )
{
    uint8 frIdx;

    /* @req FrIf05298 */
    /* @req FrIf05700 */
    FRIF_DET_REPORTERROR((TRUE == FrIf_Internal_Global.initDone),FRIF_GET_WUP_RX_STATUS_API_ID,FRIF_E_NOT_INITIALIZED,E_NOT_OK);

    /* @req FrIf05701 */
    FRIF_DET_REPORTERROR((FrIf_CtrlIdx < FrIf_ConfigCachePtr->FrIf_CtrlCount),FRIF_GET_WUP_RX_STATUS_API_ID,FRIF_E_INV_CTRL_IDX,E_NOT_OK);

    FRIF_DET_REPORTERROR((FrIf_WakeupRxStatusPtr != NULL),FRIF_GET_WUP_RX_STATUS_API_ID,FRIF_E_INV_POINTER,E_NOT_OK);

    frIdx = FrIf_ConfigCachePtr->FrIf_CtrlPtr[FrIf_CtrlIdx].FrIf_FrCtrlIdx;

    return Fr_GetWakeupRxStatus(frIdx,FrIf_WakeupRxStatusPtr);
}
#endif


/* @req FrIf05713 */
#if (FRIF_CANCEL_TRANSMIT_SUPPORT == STD_ON)
/* !req FrIf05070 */
/**
 *
 * @param FrIf_TxPduId
 * @return
 */
Std_ReturnType FrIf_CancelTransmit( PduIdType FrIf_TxPduId )
{
     /* @req FrIf05298 */
     /* @req FrIf05703 */
     FRIF_DET_REPORTERROR((TRUE == FrIf_Internal_Global.initDone),FRIF_CANCEL_TRANSMIT_API_ID,FRIF_E_NOT_INITIALIZED,E_NOT_OK);

     /* @req FrIf05704 */
     FRIF_DET_REPORTERROR((FrIf_TxPduId < FrIf_ConfigCachePtr->FrIf_TxPduCount),FRIF_CANCEL_TRANSMIT_API_ID,FRIF_E_INV_TXPDUID,E_NOT_OK);

     /* !req FrIf05705 */
     return E_NOT_OK;
}
#endif

/* @req FrIf05418 */
#if (FRIF_DISABLE_LPDU_SUPPORT == STD_ON)
/* @req FrIf05710 */
/**
 *
 * @param FrIf_CtrlIdx
 * @param FrIf_LPduIdx
 * @return
 */
Std_ReturnType FrIf_DisableLPdu( uint8 FrIf_CtrlIdx, uint16 FrIf_LPduIdx )
{
    uint8 frIdx;
    uint16 frLpduIdx;

    /* @req FrIf05298 */
    /* @req FrIf05717 */
    FRIF_DET_REPORTERROR((TRUE == FrIf_Internal_Global.initDone),FRIF_DISABLE_LPDU_API_ID,FRIF_E_NOT_INITIALIZED,E_NOT_OK);

    /* @req FrIf05714 */
    FRIF_DET_REPORTERROR((FrIf_CtrlIdx < FrIf_ConfigCachePtr->FrIf_CtrlCount),FRIF_DISABLE_LPDU_API_ID,FRIF_E_INV_CTRL_IDX,E_NOT_OK);

    FRIF_DET_REPORTERROR((FrIf_LPduIdx < FrIf_ConfigCachePtr->FrIf_CtrlPtr[FrIf_CtrlIdx].FrIf_LPduCount),FRIF_DISABLE_LPDU_API_ID,FRIF_E_INV_LPDU_IDX,E_NOT_OK);

    frIdx = FrIf_ConfigCachePtr->FrIf_CtrlPtr[FrIf_CtrlIdx].FrIf_FrCtrlIdx;
    frLpduIdx = FrIf_ConfigCachePtr->FrIf_CtrlPtr[FrIf_CtrlIdx].FrIf_LpduPtr[FrIf_LPduIdx].Fr_Buffer;

    return Fr_DisableLPdu(frIdx,frLpduIdx);
}
#endif

/* @req FrIf05419 */
#if (FRIF_GET_TRCV_ERROR_SUPPORT == STD_ON)
/* !req FrIf05032 */
/**
 *
 * @param FrIf_CtrlIdx
 * @param FrIf_ChnlIdx
 * @param FrIf_BranchIdx
 * @param FrIf_BusErrorState
 * @return
 */
/*lint --e{818} */
Std_ReturnType FrIf_GetTransceiverError( uint8 FrIf_CtrlIdx, Fr_ChannelType FrIf_ChnlIdx, uint8 FrIf_BranchIdx, uint32* FrIf_BusErrorState )
{
    /* @req FrIf05298 */
    /* @req FrIf05718 */
    FRIF_DET_REPORTERROR((TRUE == FrIf_Internal_Global.initDone),FRIF_GET_TRV_ERROR_API_ID,FRIF_E_NOT_INITIALIZED,E_NOT_OK);

    /* @req FrIf05719 */
    FRIF_DET_REPORTERROR((FrIf_CtrlIdx < FrIf_ConfigCachePtr->FrIf_CtrlCount),FRIF_GET_TRV_ERROR_API_ID,FRIF_E_INV_CTRL_IDX,E_NOT_OK);

    /* @req FrIf05720 */
    FRIF_DET_REPORTERROR(((FR_CHANNEL_A==FrIf_ChnlIdx)||(FR_CHANNEL_B==FrIf_ChnlIdx)),FRIF_GET_TRV_ERROR_API_ID,FRIF_E_INV_CHNL_IDX,E_NOT_OK);

    FRIF_DET_REPORTERROR((FrIf_BusErrorState != NULL),FRIF_GET_TRV_ERROR_API_ID,FRIF_E_INV_POINTER,E_NOT_OK);

    /* !req FrIf05728 */
    (void)FrIf_BranchIdx;
    return E_NOT_OK;
}
#endif

/* @req FrIf05420 */
#if (FRIF_ENABLE_TRCV_BRANCH_SUPPORT == STD_ON)
/* !req FrIf05085 */
/**
 *
 * @param FrIf_CtrlIdx
 * @param FrIf_ChnlIdx
 * @param FrIf_BranchIdx
 * @return
 */
Std_ReturnType FrIf_EnableTransceiverBranch( uint8 FrIf_CtrlIdx, Fr_ChannelType FrIf_ChnlIdx, uint8 FrIf_BranchIdx )
{
    /* @req FrIf05298 */
    /* @req FrIf05307 */
    FRIF_DET_REPORTERROR((TRUE == FrIf_Internal_Global.initDone),FRIF_ENABLE_TRCV_BRANCH_API_ID,FRIF_E_NOT_INITIALIZED,E_NOT_OK);

    /* @req FrIf05302 */
    FRIF_DET_REPORTERROR((FrIf_CtrlIdx < FrIf_ConfigCachePtr->FrIf_CtrlCount),FRIF_ENABLE_TRCV_BRANCH_API_ID,FRIF_E_INV_CTRL_IDX,E_NOT_OK);

    /* @req FrIf05304 */
    FRIF_DET_REPORTERROR(((FR_CHANNEL_A==FrIf_ChnlIdx)||(FR_CHANNEL_B==FrIf_ChnlIdx)),FRIF_ENABLE_TRCV_BRANCH_API_ID,FRIF_E_INV_CHNL_IDX,E_NOT_OK);

    /* !req FrIf05306 */
    (void)FrIf_BranchIdx;
    return E_NOT_OK;
}
#endif

/* @req FrIf05421 */
/* @req FrIf05425 */
#if (FRIF_DIABLE_TRCV_BRANCH_SUPPORT == STD_ON)
/* !req FrIf05028 */
/**
 *
 * @param FrIf_CtrlIdx
 * @param FrIf_ChnlIdx
 * @param FrIf_BranchIdx
 * @return
 */
Std_ReturnType FrIf_DisableTransceiverBranch( uint8 FrIf_CtrlIdx, Fr_ChannelType FrIf_ChnlIdx, uint8 FrIf_BranchIdx )
{
    /* @req FrIf05298 */
    /* @req FrIf05308 */
    FRIF_DET_REPORTERROR((TRUE == FrIf_Internal_Global.initDone),FRIF_DISABLE_TRCV_BRANCH_API_ID,FRIF_E_NOT_INITIALIZED,E_NOT_OK);

    /* @req FrIf05303 */
    FRIF_DET_REPORTERROR((FrIf_CtrlIdx < FrIf_ConfigCachePtr->FrIf_CtrlCount),FRIF_DISABLE_TRCV_BRANCH_API_ID,FRIF_E_INV_CTRL_IDX,E_NOT_OK);

    /* @req FrIf05243 */
    FRIF_DET_REPORTERROR(((FR_CHANNEL_A==FrIf_ChnlIdx)||(FR_CHANNEL_B==FrIf_ChnlIdx)),FRIF_DISABLE_TRCV_BRANCH_API_ID,FRIF_E_INV_CHNL_IDX,E_NOT_OK);

    /* !req FrIf05305 */
    (void)FrIf_BranchIdx;
    return E_NOT_OK;
}
#endif

/* @req FrIf05422 */
#if (FRIF_RECONFIG_LPDU_SUPPORT == STD_ON)
/* @req FrIf05048 */
/**
 *
 * @param FrIf_CtrlIdx
 * @param FrIf_LPduIdx
 * @param FrIf_FrameId
 * @param FrIf_ChnlIdx
 * @param FrIf_CycleRepetition
 * @param FrIf_CycleOffset
 * @param FrIf_PayloadLength
 * @param FrIf_HeaderCRC
 * @return
 */
Std_ReturnType FrIf_ReconfigLPdu( uint8 FrIf_CtrlIdx, uint16 FrIf_LPduIdx, uint16 FrIf_FrameId, Fr_ChannelType FrIf_ChnlIdx,
        uint8 FrIf_CycleRepetition, uint8 FrIf_CycleOffset, uint8 FrIf_PayloadLength, uint16 FrIf_HeaderCRC)
{
    const FrIf_ControllerType * ctrlCfg;

    /* @req FrIf05298 */
    FRIF_DET_REPORTERROR((TRUE == FrIf_Internal_Global.initDone),FRIF_RECONFIG_LPDU_API_ID,FRIF_E_NOT_INITIALIZED,E_NOT_OK);

    /* @req FrIf05309 */
    FRIF_DET_REPORTERROR((FrIf_CtrlIdx < FrIf_ConfigCachePtr->FrIf_CtrlCount),FRIF_RECONFIG_LPDU_API_ID,FRIF_E_INV_CTRL_IDX,E_NOT_OK);

    /* @req FrIf05310 */
    FRIF_DET_REPORTERROR(((FR_CHANNEL_A==FrIf_ChnlIdx)||(FR_CHANNEL_B==FrIf_ChnlIdx)),FRIF_RECONFIG_LPDU_API_ID,FRIF_E_INV_CHNL_IDX,E_NOT_OK);

    ctrlCfg = &FrIf_ConfigCachePtr->FrIf_CtrlPtr[FrIf_CtrlIdx];

    /* @req FrIf05311 */
    FRIF_DET_REPORTERROR((FrIf_LPduIdx < ctrlCfg->FrIf_LPduCount),FRIF_RECONFIG_LPDU_API_ID,FRIF_E_INV_LPDU_IDX,E_NOT_OK);
    FRIF_DET_REPORTERROR((TRUE == ctrlCfg->FrIf_LpduPtr[FrIf_LPduIdx].FrIf_Reconfigurable),FRIF_RECONFIG_LPDU_API_ID,FRIF_E_INV_LPDU_IDX,E_NOT_OK);

    /* @req FrIf05312 */
    FRIF_DET_REPORTERROR(((FrIf_FrameId >= FLEXRAY_FRAME_ID_MIN) && (FrIf_FrameId <= FLEXRAY_FRAME_ID_MAX)),FRIF_RECONFIG_LPDU_API_ID,FRIF_E_INV_FRAME_ID,E_NOT_OK);

    return Fr_ReconfigLPdu(ctrlCfg->FrIf_FrCtrlIdx, ctrlCfg->FrIf_LpduPtr[FrIf_LPduIdx].Fr_Buffer, FrIf_FrameId, FrIf_ChnlIdx, FrIf_CycleRepetition, FrIf_CycleOffset, FrIf_PayloadLength, FrIf_HeaderCRC);
}
#endif

/* @req FrIf05423 */
#if (FRIF_GET_NM_VECTOR_SUPPORT == STD_ON)
/* !req FrIf05016 */
/**
 *
 * @param FrIf_CtrlIdx
 * @param FrIf_NmVectorPtr
 * @return
 */
/*lint --e{818} */
Std_ReturnType FrIf_GetNmVector( uint8 FrIf_CtrlIdx, uint8* FrIf_NmVectorPtr )
{
    /* @req FrIf05298 */
    /* @req FrIf05199 */
    FRIF_DET_REPORTERROR((TRUE == FrIf_Internal_Global.initDone),FRIF_GET_NM_VECTOR_API_ID,FRIF_E_NOT_INITIALIZED,E_NOT_OK);

    /* @req FrIf05197 */
    FRIF_DET_REPORTERROR((FrIf_CtrlIdx < FrIf_ConfigCachePtr->FrIf_CtrlCount),FRIF_GET_NM_VECTOR_API_ID,FRIF_E_INV_CTRL_IDX,E_NOT_OK);

    FRIF_DET_REPORTERROR((FrIf_NmVectorPtr != NULL),FRIF_GET_NM_VECTOR_API_ID,FRIF_E_INV_POINTER,E_NOT_OK);

    /* !req FrIf05198 */

    return E_NOT_OK;
}
#endif


/* @req FrIf05153 */ /* @req FrIf05154 */ /* @req FrIf05424 */
#if (FRIF_VERSION_INFO_API == STD_ON)
/* @req FrIf05002 */
/**
 *
 * @param FrIf_VersionInfoPtr
 */
void FrIf_GetVersionInfo( Std_VersionInfoType* FrIf_VersionInfoPtr )
{
    /* @req FrIf05151 */
    FRIF_DET_REPORTERROR((NULL != FrIf_VersionInfoPtr),FRIF_GET_VERSION_INFO_API_ID,FRIF_E_INV_POINTER);

    /* @req FrIf05152 */
    STD_GET_VERSION_INFO(FrIf_VersionInfoPtr, FRIF);
}
#endif

/* @req FrIf05313 */
/**
 *
 * @param FrIf_CtrlIdx
 * @param FrIf_ConfigParamIdx
 * @param FrIf_ConfigParamValuePtr
 * @return
 */
Std_ReturnType FrIf_ReadCCConfig( uint8 FrIf_CtrlIdx, uint8 FrIf_ConfigParamIdx, uint32* FrIf_ConfigParamValuePtr )
{
    uint8 frIdx;

    /* @req FrIf05298 */
    FRIF_DET_REPORTERROR((TRUE == FrIf_Internal_Global.initDone),FRIF_READ_CC_CONFIG_API_ID,FRIF_E_NOT_INITIALIZED,E_NOT_OK);

    /* @req FrIf05315 */
    FRIF_DET_REPORTERROR((FrIf_CtrlIdx < FrIf_ConfigCachePtr->FrIf_CtrlCount),FRIF_READ_CC_CONFIG_API_ID,FRIF_E_INV_CTRL_IDX,E_NOT_OK);

    FRIF_DET_REPORTERROR((FrIf_ConfigParamValuePtr != NULL),FRIF_READ_CC_CONFIG_API_ID,FRIF_E_INV_POINTER,E_NOT_OK);
    frIdx = FrIf_ConfigCachePtr->FrIf_CtrlPtr[FrIf_CtrlIdx].FrIf_FrCtrlIdx;

    /* @req FrIf05314 */
    return Fr_ReadCCConfig(frIdx,FrIf_ConfigParamIdx,FrIf_ConfigParamValuePtr);
}

/**
 * @brief Handle Communication operation
 * @param comOpPtr Pointer to Communication operation configuration
 * @param frIfIdx  Index of FrIf Ctrl
 * @param Fr_LPduIdx LPdu index
 * @param frameConfigIdx Frame configuration index
 */
static inline void comOperation(const FrIf_CommunicationOperationType * comOpPtr, uint8 frIfIdx,PduIdType Fr_LPduIdx, uint8 frameConfigIdx);
static inline void comOperation(const FrIf_CommunicationOperationType * comOpPtr, uint8 frIfIdx,PduIdType Fr_LPduIdx, uint8 frameConfigIdx)
{
    switch (comOpPtr->FrIf_CommunicationAction) {

    /* @req FrIf05063 */ /* @req FrIf05295a */
    case DECOUPLED_TRANSMISSION:
        FrIf_Internal_HandleDecoupledTransmission(frIfIdx,Fr_LPduIdx,frameConfigIdx);
        break;

        /* @req FrIf05064 */
    case TX_CONFIRMATION:
        FrIf_Internal_ProvideTxConfirmation(frIfIdx, Fr_LPduIdx, frameConfigIdx);
        break;

    /* @req FrIf05289 */
    case RECEIVE_AND_STORE:
        FrIf_Internal_HandleReceiveAndStore(frIfIdx,Fr_LPduIdx,frameConfigIdx);
        break;

    /* @req FrIf05292 */
    case RECEIVE_AND_INDICATE:
        FrIf_Internal_HandleReceiveAndIndicate(frIfIdx,  Fr_LPduIdx, frameConfigIdx,
                comOpPtr->FrIf_RxComOpMaxLoop);
        break;

    /* @req FrIf05062 */
    case RX_INDICATION:
        FrIf_Internal_ProvideRxIndication(frIfIdx, Fr_LPduIdx, frameConfigIdx);
        break;

    /* @req FrIf05061 */
    case PREPARE_LPDU:
        FrIf_Internal_PrepareLPdu(frIfIdx, Fr_LPduIdx);
        break;

    case FREE_OP_A:
    case FREE_OP_B:
    default:
        /* Job operation is is invalid */
        break;
    }
}

/** ISRs */
/* @req FrIf05271 */
/**
 * @brief Job list execution function handling all clusters
 * @param clstrIdx
 */
void FrIf_JobListExec(uint8 clstrIdx) {
    const FrIf_JobType * jobCfgPtr;
    const FrIf_ClusterType * clstrCfg;
    const FrIf_CommunicationOperationType * comOpPtr;
    const FrIf_JobType * FrIf_NextJobConfigPtr;
    uint16 macroTick;
    PduIdType Fr_LPduIdx;
    uint8 frIfCycle;
    uint16 deviationFromGlobalTime=0;
    uint8 i;
    uint8 frameConfigIdx;
    uint8 frIfIdx;
    Std_ReturnType ret;

    /* @req FrIf05298 */
    /* @req FrIf05272 */
    FRIF_DET_REPORTERROR((TRUE == FrIf_Internal_Global.initDone),FRIF_JOB_LIST_EXECUTE_API_ID,FRIF_E_NOT_INITIALIZED);

    /* Data consistency of lower buffer is maintained by locking FrIf_JobListExec for one interrupt */
    if (TRUE == FrIf_Internal_Global.clstrRunTimeData[clstrIdx].jobListExecLock ) {
        /*lint -e{904} Return statement is necessary to avoid multiple if loops and hence increase readability in case of argument check */
        return;
    } else {
        FrIf_Internal_Global.clstrRunTimeData[clstrIdx].jobListExecLock = TRUE;
    }

    clstrCfg = &FrIf_ConfigCachePtr->FrIf_ClusterPtr[clstrIdx];
    frIfIdx = clstrCfg->FrIf_ControllerPtr[FIRST_FRIF_CTRL_IDX]->FrIf_CtrlIdx;

    if(TRUE == FrIf_Internal_Global.clstrRunTimeData[clstrIdx].jobListSyncLost) {
        FrIf_Internal_Global.clstrRunTimeData[clstrIdx].jobListExecLock = FALSE;
        /* If synchronization is lost return */
    } else {
        /* @req FrIf05137 */ /* @req FrIf05138 */
        if (FrIf_GetGlobalTime(frIfIdx, &frIfCycle, &macroTick) == E_OK) {
            ret = E_OK;
            /* retrieve job cluster */
            jobCfgPtr = &(clstrCfg->FrIf_JobListPtr->FrIf_JobPtr[FrIf_Internal_Global.clstrRunTimeData[clstrIdx].jobListCntr]);

            /* check if synchronized to Global Time*/
            if (macroTick > jobCfgPtr->FrIf_Macrotick) {
                deviationFromGlobalTime = macroTick - (uint16)jobCfgPtr->FrIf_Macrotick;
            } else {
                deviationFromGlobalTime = (uint16)jobCfgPtr->FrIf_Macrotick - macroTick;
            }
        } else {
            ret = E_NOT_OK;
        }

        /*lint -e{644} */
        /* Check if not synchronized to Global time */
        if ((ret != E_OK) || (deviationFromGlobalTime > clstrCfg->FrIf_MaxIsrDelay) || (frIfCycle != jobCfgPtr->FrIf_Cycle)) {

            /* Set the flag */
            FrIf_Internal_Global.clstrRunTimeData[clstrIdx].jobListSyncLost = TRUE;
            /* Disable Absolute Timer interrupt */
            (void)FrIf_CancelAbsoluteTimer(frIfIdx, clstrCfg->FrIf_JobListPtr->FrIf_AbsTimerIdx);

            FrIf_Internal_Global.clstrRunTimeData[clstrIdx].jobListExecLock = FALSE;
            FRIF_DET_REPORTERROR((FALSE == FrIf_Internal_Global.clstrRunTimeData[clstrIdx].jobListSyncLost),FRIF_JOB_LIST_EXECUTE_API_ID,FRIF_E_JLE_SYNC);
        } else {
            /* @req FrIf05133 */

            /* Job is accomplished increase joblist counter*/
            FrIf_Internal_Global.clstrRunTimeData[clstrIdx].jobListCntr = (FrIf_Internal_Global.clstrRunTimeData[clstrIdx].jobListCntr + 1)
                    %(clstrCfg->FrIf_JobListPtr->FrIf_NbrOfJobs);

            /* get the start time of next job and set the CC timer */
            FrIf_NextJobConfigPtr = &(clstrCfg->FrIf_JobListPtr->FrIf_JobPtr[FrIf_Internal_Global.clstrRunTimeData[clstrIdx].jobListCntr]);
            /* re-trigg ISR timer to invoke next re-start of this joblist execution */

            /* Acknowledge the absolute timer interrupts */
            (void)FrIf_AckAbsoluteTimerIRQ(frIfIdx, clstrCfg->FrIf_JobListPtr->FrIf_AbsTimerIdx);

            (void)FrIf_SetAbsoluteTimer(frIfIdx, clstrCfg->FrIf_JobListPtr->FrIf_AbsTimerIdx,
                    FrIf_NextJobConfigPtr->FrIf_Cycle, (uint16)FrIf_NextJobConfigPtr->FrIf_Macrotick);

            /* @req FrIf05148 */
            for (i = 0; i < jobCfgPtr->FrIf_NbrOfOperations; i++) {
                /* get next operations in this job at correct start index in List of Operations */
                comOpPtr = &(jobCfgPtr->FrIf_CommunicationOperationPtr[i]);
                /* Retrieve Indexes for Controller and its Driver in order use appropriate FlexRay Driver */
                frameConfigIdx = comOpPtr->FrIf_FrameConfigIdx;
                Fr_LPduIdx = comOpPtr->FrIf_LPduIdxRef->Fr_Buffer;

                if (FrIf_Internal_Global.clstrRunTimeData[clstrIdx].clstrState == FRIF_STATE_ONLINE) {
                    /* @req FrIf05050 */
                    /* @req FrIf05130 */
                    /* @req FrIf05134 */

                    frIfIdx = comOpPtr->FrIf_CtrlIdx; /* Get FrIf index for this communication operation */
                    comOperation(comOpPtr,frIfIdx,Fr_LPduIdx,frameConfigIdx);
                }
            }
        }
        FrIf_Internal_Global.clstrRunTimeData[clstrIdx].jobListExecLock = FALSE;
    }
}


/* !req FrIf05041 */
/**
 *
 * @param FrIf_CtrlIdx
 * @param FrIf_ChnlIdx
 */
void FrIf_CheckWakeupByTransceiver( uint8 FrIf_CtrlIdx, Fr_ChannelType FrIf_ChnlIdx )
{
    /* @req FrIf05298 */
    /* @req FrIf05277 */
    FRIF_DET_REPORTERROR((TRUE == FrIf_Internal_Global.initDone),FRIF_CHECK_WUP_BY_TRCV_API_ID,FRIF_E_NOT_INITIALIZED);

    /* @req FrIf05274 */
    FRIF_DET_REPORTERROR((FrIf_CtrlIdx < FrIf_ConfigCachePtr->FrIf_CtrlCount),FRIF_CHECK_WUP_BY_TRCV_API_ID,FRIF_E_INV_CTRL_IDX);

    /* @req FrIf05275 */
    FRIF_DET_REPORTERROR(((FR_CHANNEL_A==FrIf_ChnlIdx)||(FR_CHANNEL_B==FrIf_ChnlIdx)),FRIF_CHECK_WUP_BY_TRCV_API_ID,FRIF_E_INV_CHNL_IDX);

    /* !req FrIf05276 */
}

/** Scheduled APIs  */

/**
 * @brief Main function handling all cluster
 * @param clstrIdx
 */
void FrIf_MainFunction(uint8 clstrIdx) {
    const FrIf_ClusterType * clstrCfg;                      /* Pointer to cluster cfg */
    const FrIf_JobType * jobCfgPtr=NULL;
    uint32 jobExeTime;
    uint32 currTime;
    uint16 macroTicksPerCycle;
    uint16 macroTick;
    uint16 jitter;

    uint8 job;
    uint8 cycle;
    uint8 timerIdx;
    uint8 frIfIdx;
    boolean foundNextJob;

    /* @req FrIf05279 clstrIdx */

    /* @req FrIf05298 */
    /* @req FrIf05280 */
    FRIF_DET_REPORTERROR((TRUE == FrIf_Internal_Global.initDone),FRIF_MAINFUNCTION_API_ID,FRIF_E_NOT_INITIALIZED);

    clstrCfg = &FrIf_ConfigCachePtr->FrIf_ClusterPtr[clstrIdx];
    frIfIdx = clstrCfg->FrIf_ControllerPtr[FIRST_FRIF_CTRL_IDX]->FrIf_CtrlIdx; /* Find FrIf Controller Id for the first controller in the cluster */

    if (FrIf_Internal_Global.clstrRunTimeData[clstrIdx].clstrState == FRIF_STATE_ONLINE) {
        /* @req FrIf05120a */

        jitter =  (clstrCfg->FrIf_GdStaticSlots * MIN_SLOT_DURATION_FOR_NEXT_JOB); /* slot duration offset for next job activation */
        macroTicksPerCycle = clstrCfg->FrIf_GMacroPerCycle; /* macroticks per cycle */
        foundNextJob = FALSE;
        timerIdx = clstrCfg->FrIf_JobListPtr->FrIf_AbsTimerIdx;

        if (TRUE == FrIf_Internal_Global.clstrRunTimeData[clstrIdx].jobListSyncLost) {
            /* Find the next job */
            /* @req FrIf05120i */
            if (FrIf_GetGlobalTime(frIfIdx, &cycle, &macroTick) == E_OK) {

                for (job = 0; job < clstrCfg->FrIf_JobListPtr->FrIf_NbrOfJobs; job++) {

                    jobCfgPtr = &(clstrCfg->FrIf_JobListPtr->FrIf_JobPtr[job]);
                    jobExeTime = ((uint32)jobCfgPtr->FrIf_Cycle * macroTicksPerCycle) + jobCfgPtr->FrIf_Macrotick; /* Job execution absolute time */
                    currTime = ((uint32)cycle * macroTicksPerCycle) + macroTick; /* current absolute time */

                    /* Determine if there is sufficient time to schedule next job */
                    if (jobExeTime > (currTime + jitter))  {
                        FrIf_Internal_Global.clstrRunTimeData[clstrIdx].jobListCntr = job; /* Find next job */
                        foundNextJob = TRUE;
                        break;
                    }
                }
                if (FALSE == foundNextJob) {
                    /*No later job found, then begin with the first Job */
                    jobCfgPtr = &(clstrCfg->FrIf_JobListPtr->FrIf_JobPtr[FIRST_JOB_IDX]);
                    FrIf_Internal_Global.clstrRunTimeData[clstrIdx].jobListCntr = FIRST_JOB_IDX;
                }
                FrIf_Internal_Global.clstrRunTimeData[clstrIdx].jobListSyncLost = FALSE;

                (void)FrIf_AckAbsoluteTimerIRQ(frIfIdx, timerIdx);      /* Might be a interrupt waiting */
                (void)FrIf_EnableAbsoluteTimerIRQ(frIfIdx, timerIdx);	  /* Enable absolute timer */

                if(jobCfgPtr!=NULL){
                    (void)FrIf_SetAbsoluteTimer(frIfIdx, timerIdx, jobCfgPtr->FrIf_Cycle, (uint16)jobCfgPtr->FrIf_Macrotick); /* Set absolute timer */
                }

            }
        }
    }

    }

