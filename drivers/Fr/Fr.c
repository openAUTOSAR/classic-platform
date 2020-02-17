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

/** @reqSettings DEFAULT_SPECIFICATION_REVISION=4.0.3 */
/** @tagSettings DEFAULT_ARCHITECTURE=RH850F1H */


/* @req FR074 */
/* @req FR465 */
/* @req FR466 */
/* @req FR029 */
/* @req FR097 */
/* @req FR103 */
/* @req FR106 Done by hardware; is deactivated when leaving NORMAL_ACTIVE state or NORMAL_PASSIVE*/
/* @req FR115 */
/* @req FR449 */

/* @req FR075 */
/* @req FR467 */

/* @req FR344 */
/* @req FR469 */

/* @req FR005 */
/* @req FR470 */
/* @req FR471 */

/* @req FR345 */
/* @req FR473 */
/* @req FR474 */

/* @req FR477 */
/* @req FR478 */
/* @req FR438 */

/* @req FR451 */
/* !req FR452 - Will be implemented later */
/* !req FR453 */

/* @req FR454 */
/* @req FR463 */

/* Generator/Configuration requirements */
/* @req FR080 */
/* @req FR480 */
/* @req FR481 */
/* @req FR484 */
/* @req FR486 */
/* @req FR487 */
/* Buffer reconfiguration is not supported */
/* !req FR482 */
/* !req FR483 */
/* !req FR003 */
/* @req FR124 */
/* !req FR126 - If no DEM is configured its turned off */
/* End of  Generator/Configuration requirements */

/* ----------------------------[includes]------------------------------------*/
#if defined(USE_DEM) || defined(CFG_FR_DEM_TEST)
/* @req FR071 */
#include "Dem.h"
#endif
#if defined(USE_DET)
/* @req FR118 */
#include "Det.h"
#endif
/* @req FR462 */
#include "Fr.h"
#include "Fr_Internal.h"

#include "debug.h"
/* ----------------------------[private define]------------------------------*/

// #define _FR_DEBUG_RX_

#if defined(_FR_DEBUG_RX_)
#define _debug_rx_(...) printf (__VA_ARGS__);
#else
#define _debug_rx_(...)
#endif


/* ----------------------------[private macro]-------------------------------*/

/* @req FR026 */
/* @req FR127 */
/* Development error macros. */
#if ( FR_DEV_ERROR_DETECT == STD_ON )
#define VALIDATE(_exp,_api,_err ) \
        if( !(_exp) ) { \
          (void)Det_ReportError(FR_MODULE_ID,0,_api,_err); \
          return; \
        }

#define VALIDATE_W_RV(_exp,_api,_err,_rv ) \
        if( !(_exp) ) { \
          (void)Det_ReportError(FR_MODULE_ID,0,_api,_err); \
          return (_rv); \
        }
#else
#define VALIDATE(_exp,_api,_err )
#define VALIDATE_W_RV(_exp,_api,_err,_rv )
#endif

/* @req FR390 */
#if defined(USE_DEM) || defined(CFG_FR_DEM_TEST)
#define DEM_REPORT(_eventId, _status) Fr_Internal_reportDem((_eventId), (_status))
#else
#define DEM_REPORT(_eventId, _status)
#endif

//#if defined(CFG_RH850)
////The RH850 only supports one flexray controller
//#define GET_FR_HW_PTR(_unit)    (*(volatile struct FLXA_reg *)0x10020004u)
//#else
//#error Platform not specified
//#endif
/* ----------------------------[private typedef]-----------------------------*/
/* ----------------------------[private function prototypes]-----------------*/
/* ----------------------------[private variables]---------------------------*/
static Fr_ContainerType Fr_Cfg = {0}; /*lint !e910 Set all members to 0*/

/* ----------------------------[private functions]---------------------------*/
/* ----------------------------[public functions]----------------------------*/

/**
 * Initalizes the Fr.
 * @param Fr_ConfigPtr
 */
/* @req FR032 */
void Fr_Init(const Fr_ConfigType* Fr_ConfigPtr) {
    uint32 n, j;

    /* @req FR135 */
    VALIDATE( ( Fr_ConfigPtr != NULL ), FR_INIT_SERVICE_ID, FR_E_INV_POINTER);

    VALIDATE( ( Fr_ConfigPtr->FrClusterConfig != NULL ), FR_INIT_SERVICE_ID, FR_E_INV_POINTER);
    VALIDATE( ( Fr_ConfigPtr->FrCtrlParam != NULL ), FR_INIT_SERVICE_ID, FR_E_INV_POINTER);
    VALIDATE( ( Fr_ConfigPtr->FrTrigConfig != NULL ), FR_INIT_SERVICE_ID, FR_E_INV_POINTER);
    VALIDATE( ( Fr_ConfigPtr->Fr_LpduConf != NULL ), FR_INIT_SERVICE_ID, FR_E_INV_POINTER);

    /* @req FR137 */
    /* Store the location of the configuration data. */
    Fr_Cfg.Fr_ConfigPtr = Fr_ConfigPtr;

    //Setup pointer to base register depending on configured controller index
    for (n = 0; n < FR_ARC_CTRL_CONFIG_CNT; n++) {
        // Fr_Cfg.hwPtr[n] = &GET_FR_HW_PTR(Fr_Cfg.Fr_ConfigPtr->FrCtrlParam[n]->FrCtrlIdx); /*lint !e923 Must assign address to hw register*/

        //Set memory buffer setups to 0.
        for (j = 0; j < Fr_Cfg.Fr_ConfigPtr->FrTrigConfig[n].FrNbrTrigConfiged; j++) {
            Fr_Cfg.Fr_ConfigPtr->FrTrigConfig[n].FrMsgBufferCfg[j].FrDataPartitionAddr = (uint32)NULL;
            Fr_Cfg.Fr_ConfigPtr->FrTrigConfig[n].FrMsgBufferCfg[j].FrMsgBufferIdx = 0;
        }
    }

    Fr_Cfg.Fr_HasIntiailized = TRUE;
}

/**
 * Initialze a FlexRay CC.
 * @param Fr_CtrlIdx
 * @return E_OK / E_NOT_OK
 */
/* @req FR017 */
Std_ReturnType Fr_ControllerInit( uint8 Fr_CtrlIdx ) {
    Std_ReturnType retval;

    /* @req FR143 */
    VALIDATE_W_RV( (Fr_Cfg.Fr_HasIntiailized == TRUE), FR_CTRLINIT_SERVICE_ID, FR_E_NOT_INITIALIZED, E_NOT_OK );

    /* @req FR144 */
    VALIDATE_W_RV( (Fr_CtrlIdx < FR_ARC_CTRL_CONFIG_CNT), FR_CTRLINIT_SERVICE_ID, FR_E_INV_CTRL_IDX, E_NOT_OK );

    retval = Fr_Internal_EnableCtrl(&Fr_Cfg, Fr_CtrlIdx);
    if (retval == E_NOT_OK) {
        return retval;
    }

    /* @req FR149 */
    retval = Fr_Internal_ClearPendingTx(&Fr_Cfg, Fr_CtrlIdx);
    if (retval == E_NOT_OK) {
        return retval;
    }

    /* @req FR150 */
    retval = Fr_Internal_ClearPendingRx(&Fr_Cfg, Fr_CtrlIdx);
    if (retval == E_NOT_OK) {
        return retval;
    }

    /* @req FR151 */
    Fr_Arc_ClearPendingIsr(&Fr_Cfg, Fr_CtrlIdx);

    /* @req FR152 */
    Fr_Internal_DisableAllTimers(&Fr_Cfg, Fr_CtrlIdx);

    /* @req FR153 */
    Fr_Internal_DisableAllFrIsr(&Fr_Cfg, Fr_CtrlIdx);

    /* !req FR515 */
    //Fr_Internal_DisableAllLPdu();

    retval = Fr_Internal_setupAndTestCC(&Fr_Cfg, Fr_CtrlIdx);

    return retval;
}


/**
 * Invokes the CC CHI command 'FREEZE'.
 * @param Fr_CtrlIdx
 * @return
 */
/* @req FR011 */
/* @req FR191 */
Std_ReturnType Fr_AbortCommunication(uint8 Fr_CtrlIdx) {
    Std_ReturnType retval;

    /* @req FR188 */
    VALIDATE_W_RV( (Fr_Cfg.Fr_HasIntiailized == TRUE), FR_ABORTCOMMUNICATION_ID, FR_E_NOT_INITIALIZED, E_NOT_OK );
    /* @req FR189 */
    VALIDATE_W_RV( (Fr_CtrlIdx < FR_ARC_CTRL_CONFIG_CNT), FR_ABORTCOMMUNICATION_ID, FR_E_INV_CTRL_IDX, E_NOT_OK );

    retval = Fr_Internal_SetCtrlChiCmd(&Fr_Cfg, Fr_CtrlIdx, (uint8)FR_POC_CMD_FREEZE);

//    if (retval == E_OK) {
//        Fr_Cfg.Fr_CCPocState[Fr_CtrlIdx] = FR_POCSTATE_HALT;
//    }

    return retval;
}

/**
 * Invokes the CC CHI command 'DEFERRED_HALT'.
 * @param Fr_CtrlIdx
 * @return
 */
/* @req FR014 */
/* @req FR187 */
Std_ReturnType Fr_HaltCommunication(uint8 Fr_CtrlIdx) {
    Std_ReturnType retval;

    /* @req FR183 */
    VALIDATE_W_RV( (Fr_Cfg.Fr_HasIntiailized == TRUE), FR_HALTCOMMUNICATION_ID, FR_E_NOT_INITIALIZED, E_NOT_OK );
    /* @req FR184 */
    VALIDATE_W_RV( (Fr_CtrlIdx < FR_ARC_CTRL_CONFIG_CNT), FR_HALTCOMMUNICATION_ID, FR_E_INV_CTRL_IDX, E_NOT_OK );
    /* @req FR185 */
    retval = Fr_Internal_IsSynchronous(&Fr_Cfg, Fr_CtrlIdx);
    VALIDATE_W_RV( (retval == E_OK), FR_HALTCOMMUNICATION_ID, FR_E_INV_POCSTATE, E_NOT_OK );

    retval = Fr_Internal_SetCtrlChiCmd(&Fr_Cfg, Fr_CtrlIdx, (uint8)FR_POC_CMD_HALT);

    return retval;
}

/**
 * Invokes the CC CHI command 'ALLOW_COLDSTART
 * @param Fr_CtrlIdx
 * @return
 */
/* @req FR114 */
/* @req FR182 */
Std_ReturnType Fr_AllowColdstart(uint8 Fr_CtrlIdx) {
    Std_ReturnType retval;

    /* @req FR178 */
    VALIDATE_W_RV( (Fr_Cfg.Fr_HasIntiailized == TRUE), FR_ALLOWCOLDSTART_ID, FR_E_NOT_INITIALIZED, E_NOT_OK );
    /* @req FR179 */
    VALIDATE_W_RV( (Fr_CtrlIdx < FR_ARC_CTRL_CONFIG_CNT), FR_ALLOWCOLDSTART_ID, FR_E_INV_CTRL_IDX, E_NOT_OK );
    /* @req FR180 */
    Fr_POCStateType state = Fr_Internal_GetProtState(&Fr_Cfg,Fr_CtrlIdx);

    VALIDATE_W_RV( !((state == FR_POCSTATE_DEFAULT_CONFIG) ||
                    (state  == FR_POCSTATE_CONFIG) ||
                    (state  == FR_POCSTATE_HALT)),
                    FR_ALLOWCOLDSTART_ID, FR_E_INV_POCSTATE, E_NOT_OK );

    retval = Fr_Internal_SetCtrlChiCmd(&Fr_Cfg, Fr_CtrlIdx, (uint8)FR_POC_CMD_ALLOW_COLDSTART);

    return retval;
}


/**
 * Set the CC into the startup state.
 * @param Fr_CtrlIdx
 * @return
 */
/* @req FR010 */
Std_ReturnType Fr_StartCommunication(uint8 Fr_CtrlIdx) {
    Std_ReturnType retval;

    /* @req FR173 */
    VALIDATE_W_RV( (Fr_Cfg.Fr_HasIntiailized == TRUE), FR_STARTCOMMUNICATION_ID, FR_E_NOT_INITIALIZED, E_NOT_OK );
    /* @req FR174 */
    VALIDATE_W_RV( (Fr_CtrlIdx < FR_ARC_CTRL_CONFIG_CNT), FR_STARTCOMMUNICATION_ID, FR_E_INV_CTRL_IDX, E_NOT_OK );
    /* @req FR175 */
    Fr_POCStateType state = Fr_Internal_GetProtState(&Fr_Cfg,Fr_CtrlIdx);
    VALIDATE_W_RV( (state == FR_POCSTATE_READY),
                    FR_STARTCOMMUNICATION_ID, FR_E_INV_POCSTATE, E_NOT_OK );

    /* @req FR177 */
    /* @req FR176 */
    retval = Fr_Internal_SetCtrlChiCmd(&Fr_Cfg, Fr_CtrlIdx, (uint8)FR_POC_CMD_RUN);

    return retval;
}


/**
 * Read and returns the POC state of the controller
 * @param Fr_CtrlIdx
 * @param Fr_POCStatusPtr
 * @return
 */
/* @req FR012 */
Std_ReturnType Fr_GetPOCStatus(uint8 Fr_CtrlIdx, Fr_POCStatusType* Fr_POCStatusPtr) {
    Std_ReturnType retval;

    /* @req FR213 */
    VALIDATE_W_RV( (Fr_Cfg.Fr_HasIntiailized == TRUE), FR_GETPOCSTATUS_ID, FR_E_NOT_INITIALIZED, E_NOT_OK );
    /* @req FR214 */
    VALIDATE_W_RV( (Fr_CtrlIdx < FR_ARC_CTRL_CONFIG_CNT), FR_GETPOCSTATUS_ID, FR_E_INV_CTRL_IDX, E_NOT_OK );
    /* @req FR215 */
    VALIDATE_W_RV( ( Fr_POCStatusPtr != NULL ), FR_GETPOCSTATUS_ID, FR_E_INV_POINTER, E_NOT_OK);

    /* @req FR217 */
    retval = Fr_Internal_GetChiPocState(&Fr_Cfg, Fr_CtrlIdx, Fr_POCStatusPtr);

    return retval;
}

/**
 * Invokes the CC CHI command 'WAKEUP'.
 * @param Fr_CtrlIdx
 * @return
 */
/* @req FR009 */
/* @req FR196 */
Std_ReturnType Fr_SendWUP(uint8 Fr_CtrlIdx) {
    Std_ReturnType retval;

    /* @req FR192 */
    VALIDATE_W_RV( (Fr_Cfg.Fr_HasIntiailized == TRUE), FR_SENDWUP_ID, FR_E_NOT_INITIALIZED, E_NOT_OK );
    /* @req FR193 */
    VALIDATE_W_RV( (Fr_CtrlIdx < FR_ARC_CTRL_CONFIG_CNT), FR_SENDWUP_ID, FR_E_INV_CTRL_IDX, E_NOT_OK );
    /* @req FR194 */
    Fr_POCStateType state = Fr_Internal_GetProtState(&Fr_Cfg,Fr_CtrlIdx);
    VALIDATE_W_RV( (state == FR_POCSTATE_READY),
                FR_SENDWUP_ID, FR_E_INV_POCSTATE, E_NOT_OK );

    retval = Fr_Internal_SetCtrlChiCmd(&Fr_Cfg, Fr_CtrlIdx, (uint8)FR_POC_CMD_WAKEUP);

    return retval;
}

/**
 * Sets a wakeup channel.
 * @param Fr_CtrlIdx
 * @param Fr_ChnlIdx
 * @return
 */
/* @req FR091 */
/* @req FR202 */
Std_ReturnType Fr_SetWakeupChannel(uint8 Fr_CtrlIdx, Fr_ChannelType Fr_ChnlIdx) {
    Std_ReturnType retval;

    /* @req FR197 */
    VALIDATE_W_RV( (Fr_Cfg.Fr_HasIntiailized == TRUE), FR_SETWAKEUPCHANNEL_ID, FR_E_NOT_INITIALIZED, E_NOT_OK );
    /* @req FR198 */
    VALIDATE_W_RV( (Fr_CtrlIdx < FR_ARC_CTRL_CONFIG_CNT), FR_SETWAKEUPCHANNEL_ID, FR_E_INV_CTRL_IDX, E_NOT_OK );
    /* @req FR199 */
    VALIDATE_W_RV( ((Fr_ChnlIdx == FR_CHANNEL_A) || (Fr_ChnlIdx == FR_CHANNEL_B)),
            FR_SETWAKEUPCHANNEL_ID, FR_E_INV_CHNL_IDX, E_NOT_OK );
    /* @req FR200 */
    Fr_POCStateType state = Fr_Internal_GetProtState(&Fr_Cfg,Fr_CtrlIdx);
    VALIDATE_W_RV( (state == FR_POCSTATE_READY),
            FR_SETWAKEUPCHANNEL_ID, FR_E_INV_POCSTATE, E_NOT_OK );

    retval = Fr_Internal_SetWUPChannel(&Fr_Cfg, Fr_CtrlIdx, Fr_ChnlIdx);

    return retval;
}

/**
 * Transmits data on the FlexRay network.
 * @param Fr_CtrlIdx
 * @param Fr_LPduIdx
 * @param Fr_LSduPtr
 * @param Fr_LSduLength
 * @return
 */
/* @req FR092 */
/* @req FR224 */
Std_ReturnType Fr_TransmitTxLPdu(uint8 Fr_CtrlIdx, uint16 Fr_LPduIdx, const uint8* Fr_LSduPtr, uint8 Fr_LSduLength) {
    Std_ReturnType retval = E_OK;
    uint32 trigIdx;
    uint16 msgBuffrIdx;

    /* @req FR218 */
    VALIDATE_W_RV( (Fr_Cfg.Fr_HasIntiailized == TRUE), FR_TRANSMITTXLPDU_ID, FR_E_NOT_INITIALIZED, E_NOT_OK );

    const Fr_FrIfCCTriggeringType *trigConfPtr = &Fr_Cfg.Fr_ConfigPtr->FrTrigConfig[Fr_CtrlIdx];
    const Fr_FrIfLPduContainerType *lpduConfPtr = &Fr_Cfg.Fr_ConfigPtr->Fr_LpduConf[Fr_CtrlIdx];

    /* @req FR219 */
    VALIDATE_W_RV( (Fr_CtrlIdx < FR_ARC_CTRL_CONFIG_CNT), FR_TRANSMITTXLPDU_ID, FR_E_INV_CTRL_IDX, E_NOT_OK );

    /* @req FR220 */
    VALIDATE_W_RV( (Fr_LPduIdx < lpduConfPtr->FrNbrLPdusConfigured), \
            FR_TRANSMITTXLPDU_ID, FR_E_INV_LPDU_IDX, E_NOT_OK );

    /* @req FR221 */
    trigIdx = lpduConfPtr->FrLpdu[Fr_LPduIdx].FrLpdTriggIdx;
    VALIDATE_W_RV( (Fr_LSduLength <= (uint8)trigConfPtr->FrTrigConfPtr[trigIdx].FrTrigLSduLength), \
            FR_TRANSMITTXLPDU_ID, FR_E_INV_LENGTH, E_NOT_OK );

    /* @req FR222 */
    VALIDATE_W_RV( ( Fr_LSduPtr != NULL ), FR_TRANSMITTXLPDU_ID, FR_E_INV_POINTER, E_NOT_OK);

    /* If the Fr_ControllerInit has not run then the data partitioning has not been done, return E_NOT_OK. */
    VALIDATE_W_RV( (trigConfPtr->FrMsgBufferCfg[trigIdx].FrDataPartitionAddr != 0UL), \
            FR_TRANSMITTXLPDU_ID, FR_E_ARC_DATA_PARTITION, E_NOT_OK);

    msgBuffrIdx = trigConfPtr->FrMsgBufferCfg[trigIdx].FrMsgBufferIdx;

    //Reconfigure header if FrIfAllowDynamicLSduLength and length is different
    if (trigConfPtr->FrTrigConfPtr[trigIdx].FrTrigAllowDynamicLen &&
        (trigConfPtr->FrTrigConfPtr[trigIdx].FrTrigSlotId > Fr_Cfg.Fr_ConfigPtr->FrClusterConfig[Fr_CtrlIdx].FrClusterGNumberOfStaticSlots) &&
        ((uint32)Fr_LSduLength != trigConfPtr->FrMsgBufferCfg[trigIdx].FrCurrentLengthSetup))
    {
        //Update header
        retval = Fr_Internal_UpdateHeaderLength(&Fr_Cfg, Fr_CtrlIdx, Fr_LSduLength, msgBuffrIdx, trigIdx);
    }

    if (retval == E_OK) {
        retval = Fr_Internal_SetTxData(&Fr_Cfg, Fr_CtrlIdx, Fr_LSduPtr, Fr_LSduLength, msgBuffrIdx);
    }

    return retval;
}


/**
 * Receives data from the FlexRay network.
 * @param Fr_CtrlIdx
 * @param Fr_LPduIdx
 * @param Fr_LSduPtr
 * @param Fr_LPduStatusPtr
 * @param Fr_LSduLengthPtr
 * @return
 */
/* @req FR093 */
/* @req FR233 */
Std_ReturnType Fr_ReceiveRxLPdu( uint8 Fr_CtrlIdx,
                                uint16 Fr_LPduIdx,
                                uint8* Fr_LSduPtr,
                                Fr_RxLPduStatusType* Fr_LPduStatusPtr,
                                uint8* Fr_LSduLengthPtr )
{
    Std_ReturnType retval = E_OK;
    Std_ReturnType newData;
    uint16 msgBufferIdx;
    uint32 trigIdx;

    /* @req FR226 */
    VALIDATE_W_RV( (Fr_Cfg.Fr_HasIntiailized == TRUE), FR_RECEIVERXLPDU_ID, FR_E_NOT_INITIALIZED, E_NOT_OK );

    const Fr_FrIfCCTriggeringType *trigConfPtr = &Fr_Cfg.Fr_ConfigPtr->FrTrigConfig[Fr_CtrlIdx];
    const Fr_FrIfLPduContainerType *lpduConfPtr = &Fr_Cfg.Fr_ConfigPtr->Fr_LpduConf[Fr_CtrlIdx];

    /* @req FR227 */
    VALIDATE_W_RV( (Fr_CtrlIdx < FR_ARC_CTRL_CONFIG_CNT), FR_RECEIVERXLPDU_ID, FR_E_INV_CTRL_IDX, E_NOT_OK );
    /* @req FR228 */
    VALIDATE_W_RV( (Fr_LPduIdx < lpduConfPtr->FrNbrLPdusConfigured), \
            FR_RECEIVERXLPDU_ID, FR_E_INV_LPDU_IDX, E_NOT_OK );
    /* @req FR229 */
    VALIDATE_W_RV( ( Fr_LSduPtr != NULL ), FR_RECEIVERXLPDU_ID, FR_E_INV_POINTER, E_NOT_OK);
    /* @req FR230 */
    VALIDATE_W_RV( ( Fr_LPduStatusPtr != NULL ), FR_RECEIVERXLPDU_ID, FR_E_INV_POINTER, E_NOT_OK);
    /* @req FR231 */
    VALIDATE_W_RV( ( Fr_LSduLengthPtr != NULL ), FR_RECEIVERXLPDU_ID, FR_E_INV_POINTER, E_NOT_OK);

    trigIdx = lpduConfPtr->FrLpdu[Fr_LPduIdx].FrLpdTriggIdx;
    /* If the Fr_ControllerInit has not run then the data partitioning has not been done, return E_NOT_OK. */
    VALIDATE_W_RV( (trigConfPtr->FrMsgBufferCfg[trigIdx].FrDataPartitionAddr != 0UL), \
            FR_TRANSMITTXLPDU_ID, FR_E_ARC_DATA_PARTITION, E_NOT_OK);

    /* @req FR603 Partly */
    /* @req FR604 Partly */
    //Set this as default.
    *Fr_LSduLengthPtr = 0;
    *Fr_LPduStatusPtr = FR_NOT_RECEIVED;

    msgBufferIdx = trigConfPtr->FrMsgBufferCfg[trigIdx].FrMsgBufferIdx;

    /* @req FR237 */
    newData = Fr_Internal_CheckNewData(&Fr_Cfg, Fr_CtrlIdx, msgBufferIdx);

    if (newData == E_OK) {
        retval = Fr_Internal_GetNewData(&Fr_Cfg, Fr_CtrlIdx, trigIdx, msgBufferIdx, Fr_LSduPtr, Fr_LSduLengthPtr);

        _debug_rx_("RX  idx=%d len=%d\n",trigConfPtr->FrTrigConfPtr[trigIdx].FrTrigSlotId, *Fr_LSduLengthPtr);
        for(uint32 i=0; i<*Fr_LSduLengthPtr;i++) {
        	_debug_rx_("%02d ",Fr_LSduPtr[i]);
        }
        _debug_rx_("%c",'\n');

        if (retval == E_OK) {
            *Fr_LPduStatusPtr = FR_RECEIVED;
        }
    }

    return retval;
}


/**
 * Checks the transmit status of the LSdu.
 * @param Fr_CtrlIdx
 * @param Fr_LPduIdx
 * @param Fr_TxLPduStatusPtr
 * @return
 */
/* @req FR094 */
/* @req FR244 */
Std_ReturnType Fr_CheckTxLPduStatus(uint8 Fr_CtrlIdx, uint16 Fr_LPduIdx, Fr_TxLPduStatusType* Fr_TxLPduStatusPtr) {
    boolean isTxPending = FALSE;
    Std_ReturnType retval;
    uint32 trigIdx;

    /* @req FR240 */
    VALIDATE_W_RV( (Fr_Cfg.Fr_HasIntiailized == TRUE), FR_CHECKTXLPDUSTATUS_ID, FR_E_NOT_INITIALIZED, E_NOT_OK );

    const Fr_FrIfCCTriggeringType *trigConfPtr = &Fr_Cfg.Fr_ConfigPtr->FrTrigConfig[Fr_CtrlIdx];
    const Fr_FrIfLPduContainerType *lpduConfPtr = &Fr_Cfg.Fr_ConfigPtr->Fr_LpduConf[Fr_CtrlIdx];

    /* @req FR241 */
    VALIDATE_W_RV( (Fr_CtrlIdx < FR_ARC_CTRL_CONFIG_CNT), FR_CHECKTXLPDUSTATUS_ID, FR_E_INV_CTRL_IDX, E_NOT_OK );
    /* @req FR242 */
    VALIDATE_W_RV( (Fr_LPduIdx < lpduConfPtr->FrNbrLPdusConfigured), \
            FR_CHECKTXLPDUSTATUS_ID, FR_E_INV_LPDU_IDX, E_NOT_OK );

    /* @req FR343 */
    VALIDATE_W_RV( ( Fr_TxLPduStatusPtr != NULL ), FR_CHECKTXLPDUSTATUS_ID, FR_E_INV_POINTER, E_NOT_OK);

    trigIdx = lpduConfPtr->FrLpdu[Fr_LPduIdx].FrLpdTriggIdx;
    /* If the Fr_ControllerInit has not run then the data partitioning has not been done, return E_NOT_OK. */
    VALIDATE_W_RV( (trigConfPtr->FrMsgBufferCfg[trigIdx].FrDataPartitionAddr != 0UL), \
            FR_CHECKTXLPDUSTATUS_ID, FR_E_ARC_DATA_PARTITION, E_NOT_OK);


    retval = Fr_Internal_GetTxPending(&Fr_Cfg, Fr_CtrlIdx, trigIdx, &isTxPending);

    if (retval == E_OK) {
        //If no transmission is pending.
        if (!isTxPending) {
            *Fr_TxLPduStatusPtr = FR_TRANSMITTED;
        } else {
            *Fr_TxLPduStatusPtr = FR_NOT_TRANSMITTED;
        }
    }

    return E_OK;
}

/**
 * Cancels the already pending transmission of a LPdu contained in a controllers physical transmit resource.
 * @param Fr_CtrlIdx
 * @param Fr_LPduIdx
 * @return
 */
/* @req FR610 */
/* @req FR611 */
Std_ReturnType Fr_CancelTxLPdu(uint8 Fr_CtrlIdx, uint16 Fr_LPduIdx) {
    Std_ReturnType retval;
    boolean isTxPending;
    uint16 msgBuffrIdx;
    uint32 trigIdx;

    /* @req FR614 */
    VALIDATE_W_RV( (Fr_Cfg.Fr_HasIntiailized == TRUE), FR_CANCELTXLPDU_ID, FR_E_NOT_INITIALIZED, E_NOT_OK );

    const Fr_FrIfCCTriggeringType *trigConfPtr = &Fr_Cfg.Fr_ConfigPtr->FrTrigConfig[Fr_CtrlIdx];
    const Fr_FrIfLPduContainerType *lpduConfPtr = &Fr_Cfg.Fr_ConfigPtr->Fr_LpduConf[Fr_CtrlIdx];

    /* @req FR615 */
    VALIDATE_W_RV( (Fr_CtrlIdx < FR_ARC_CTRL_CONFIG_CNT), FR_CANCELTXLPDU_ID, FR_E_INV_CTRL_IDX, E_NOT_OK );
    /* @req FR616 */
    VALIDATE_W_RV( (Fr_LPduIdx < lpduConfPtr->FrNbrLPdusConfigured), \
            FR_CANCELTXLPDU_ID, FR_E_INV_LPDU_IDX, E_NOT_OK );

    trigIdx = lpduConfPtr->FrLpdu[Fr_LPduIdx].FrLpdTriggIdx;
    /* If the Fr_ControllerInit has not run then the data partitioning has not been done, return E_NOT_OK. */
    VALIDATE_W_RV( (trigConfPtr->FrMsgBufferCfg[trigIdx].FrDataPartitionAddr != 0UL), \
            FR_CANCELTXLPDU_ID, FR_E_ARC_DATA_PARTITION, E_NOT_OK);

    msgBuffrIdx = trigConfPtr->FrMsgBufferCfg[trigIdx].FrMsgBufferIdx;

    //To check if the buffer is in any pending transmission
    retval = Fr_Internal_GetTxPending(&Fr_Cfg, Fr_CtrlIdx, trigIdx, &isTxPending);

    if ((retval == E_OK) && isTxPending) {
        retval = Fr_Internal_CancelTx(&Fr_Cfg, Fr_CtrlIdx, msgBuffrIdx);
    } else {
        retval = E_NOT_OK;
    }

    return retval;
}


/**
 * Gets the current global FlexRay time.
 * @param Fr_CtrlIdx
 * @param Fr_CyclePtr
 * @param Fr_MacroTickPtr
 * @return
 */
/* @req FR042 */
/* @req FR256 */
Std_ReturnType Fr_GetGlobalTime(uint8 Fr_CtrlIdx, uint8* Fr_CyclePtr, uint16* Fr_MacroTickPtr) {
    Std_ReturnType retval;

    /* @req FR251 */
    VALIDATE_W_RV( (Fr_Cfg.Fr_HasIntiailized == TRUE), FR_GETGLOBALTIME_ID, FR_E_NOT_INITIALIZED, E_NOT_OK );
    /* @req FR252 */
    VALIDATE_W_RV( (Fr_CtrlIdx < FR_ARC_CTRL_CONFIG_CNT), FR_GETGLOBALTIME_ID, FR_E_INV_CTRL_IDX, E_NOT_OK );
    /* @req FR253 */
    VALIDATE_W_RV( ( Fr_CyclePtr != NULL ), FR_GETGLOBALTIME_ID, FR_E_INV_POINTER, E_NOT_OK);
    /* @req FR254 */
    VALIDATE_W_RV( ( Fr_MacroTickPtr != NULL ), FR_GETGLOBALTIME_ID, FR_E_INV_POINTER, E_NOT_OK);

    retval = Fr_Internal_GetGlobalTime(&Fr_Cfg, Fr_CtrlIdx, Fr_CyclePtr, Fr_MacroTickPtr);

    return retval;
}

/**
 * Sets the absolute FlexRay timer.
 * @param Fr_CtrlIdx
 * @param Fr_AbsTimerIdx
 * @param Fr_Cycle
 * @param Fr_Offset
 * @return
 */
/* @req FR033 */
Std_ReturnType Fr_SetAbsoluteTimer(uint8 Fr_CtrlIdx, uint8 Fr_AbsTimerIdx, uint8 Fr_Cycle, uint16 Fr_Offset) {
    Std_ReturnType retval;

    /* @req FR267 */
    VALIDATE_W_RV( (Fr_Cfg.Fr_HasIntiailized == TRUE), FR_SETABSOLUTETIMER_ID, FR_E_NOT_INITIALIZED, E_NOT_OK );
    /* @req FR268 */
    VALIDATE_W_RV( (Fr_CtrlIdx < FR_ARC_CTRL_CONFIG_CNT), FR_SETABSOLUTETIMER_ID, FR_E_INV_CTRL_IDX, E_NOT_OK );
    /* @req FR269 */
    VALIDATE_W_RV( (Fr_AbsTimerIdx <= Fr_Cfg.Fr_ConfigPtr->FrCtrlParam[Fr_CtrlIdx].FrArcAbsTimerMaxIdx),
            FR_SETABSOLUTETIMER_ID, FR_E_INV_TIMER_IDX, E_NOT_OK );
    /* @req FR270 */
    VALIDATE_W_RV( (Fr_Cycle <= Fr_Cfg.Fr_ConfigPtr->FrClusterConfig[Fr_CtrlIdx].FrClusterGCycleCountMax),
                FR_SETABSOLUTETIMER_ID, FR_E_INV_CYCLE, E_NOT_OK );
    /* @req FR271 */
    VALIDATE_W_RV( ((uint32)Fr_Offset < Fr_Cfg.Fr_ConfigPtr->FrClusterConfig[Fr_CtrlIdx].FrClusterGMacroPerCycle),
                FR_SETABSOLUTETIMER_ID, FR_E_INV_OFFSET, E_NOT_OK );

    /* @req FR436 */
    retval = Fr_Internal_IsSynchronous(&Fr_Cfg, Fr_CtrlIdx);
    VALIDATE_W_RV( (retval == E_OK), FR_SETABSOLUTETIMER_ID, FR_E_INV_POCSTATE, E_NOT_OK );

    /* @req FR273 */
    Fr_Internal_SetupAbsTimer(&Fr_Cfg, Fr_CtrlIdx, Fr_AbsTimerIdx, Fr_Cycle, Fr_Offset);

    return E_OK;
}

/**
 * Stops an absolute timer.
 * @param Fr_CtrlIdx
 * @param Fr_AbsTimerIdx
 * @return
 */
/* @req FR095 */
Std_ReturnType Fr_CancelAbsoluteTimer(uint8 Fr_CtrlIdx, uint8 Fr_AbsTimerIdx) {

    /* @req FR283 */
    VALIDATE_W_RV( (Fr_Cfg.Fr_HasIntiailized == TRUE), FR_CANCELABSOLUTETIMER_ID, FR_E_NOT_INITIALIZED, E_NOT_OK );
    /* @req FR284 */
    VALIDATE_W_RV( (Fr_CtrlIdx < FR_ARC_CTRL_CONFIG_CNT), FR_CANCELABSOLUTETIMER_ID, FR_E_INV_CTRL_IDX, E_NOT_OK );
    /* @req FR285 */
    VALIDATE_W_RV( (Fr_AbsTimerIdx <= Fr_Cfg.Fr_ConfigPtr->FrCtrlParam[Fr_CtrlIdx].FrArcAbsTimerMaxIdx),
            FR_CANCELABSOLUTETIMER_ID, FR_E_INV_TIMER_IDX, E_NOT_OK );

    /* @req FR287 */
    Fr_Internal_DisableAbsTimer(&Fr_Cfg, Fr_CtrlIdx, Fr_AbsTimerIdx);

    return E_OK;
}


/**
 * Gets IRQ status of an absolute timer.
 * @param Fr_CtrlIdx
 * @param Fr_AbsTimerIdx
 * @param Fr_IRQStatusPtr
 * @return
 */
/* @req FR108 */
Std_ReturnType Fr_GetAbsoluteTimerIRQStatus(uint8 Fr_CtrlIdx, uint8 Fr_AbsTimerIdx, boolean* Fr_IRQStatusPtr) {

    /* @req FR327 */
    VALIDATE_W_RV( (Fr_Cfg.Fr_HasIntiailized == TRUE), FR_GETABSTIMERIRQSTATUS_ID, FR_E_NOT_INITIALIZED, E_NOT_OK );
    /* @req FR328 */
    VALIDATE_W_RV( (Fr_CtrlIdx < FR_ARC_CTRL_CONFIG_CNT), FR_GETABSTIMERIRQSTATUS_ID, FR_E_INV_CTRL_IDX, E_NOT_OK );
    /* @req FR329 */
    VALIDATE_W_RV( (Fr_AbsTimerIdx <= Fr_Cfg.Fr_ConfigPtr->FrCtrlParam[Fr_CtrlIdx].FrArcAbsTimerMaxIdx),
            FR_GETABSTIMERIRQSTATUS_ID, FR_E_INV_TIMER_IDX, E_NOT_OK );
    /* @req FR330 */
    VALIDATE_W_RV( ( Fr_IRQStatusPtr != NULL ), FR_GETABSTIMERIRQSTATUS_ID, FR_E_INV_POINTER, E_NOT_OK);

    /* @req FR332 */
    *Fr_IRQStatusPtr = Fr_Internal_GetAbsTimerIrqStatus(&Fr_Cfg, Fr_CtrlIdx, Fr_AbsTimerIdx);

    return E_OK;
}

/**
 * Resets the interrupt condition of an absolute timer.
 * @param Fr_CtrlIdx
 * @param Fr_AbsTimerIdx
 * @return
 */
/* @req FR036 */
Std_ReturnType Fr_AckAbsoluteTimerIRQ(uint8 Fr_CtrlIdx, uint8 Fr_AbsTimerIdx) {

    /* @req FR305 */
    VALIDATE_W_RV( (Fr_Cfg.Fr_HasIntiailized == TRUE), FR_ACKABSOLUTETIMERIRQ_ID, FR_E_NOT_INITIALIZED, E_NOT_OK );
    /* @req FR306 */
    VALIDATE_W_RV( (Fr_CtrlIdx < FR_ARC_CTRL_CONFIG_CNT), FR_ACKABSOLUTETIMERIRQ_ID, FR_E_INV_CTRL_IDX, E_NOT_OK );
    /* @req FR307 */
    VALIDATE_W_RV( (Fr_AbsTimerIdx <= Fr_Cfg.Fr_ConfigPtr->FrCtrlParam[Fr_CtrlIdx].FrArcAbsTimerMaxIdx),
            FR_ACKABSOLUTETIMERIRQ_ID, FR_E_INV_TIMER_IDX, E_NOT_OK );

    /* @req FR309 */
    Fr_Internal_ResetAbsTimerIsrFlag(&Fr_Cfg, Fr_CtrlIdx, Fr_AbsTimerIdx);

    return E_OK;
}

/**
 * Disables the interrupt line of an absolute timer.
 * @param Fr_CtrlIdx
 * @param Fr_AbsTimerIdx
 * @return
 */
/* @req FR035 */
Std_ReturnType Fr_DisableAbsoluteTimerIRQ(uint8 Fr_CtrlIdx, uint8 Fr_AbsTimerIdx) {

    /* @req FR316 */
    VALIDATE_W_RV( (Fr_Cfg.Fr_HasIntiailized == TRUE), FR_DISABLEABSTIMERIRQ_ID, FR_E_NOT_INITIALIZED, E_NOT_OK );
    /* @req FR317 */
    VALIDATE_W_RV( (Fr_CtrlIdx < FR_ARC_CTRL_CONFIG_CNT), FR_DISABLEABSTIMERIRQ_ID, FR_E_INV_CTRL_IDX, E_NOT_OK );
    /* @req FR318 */
    VALIDATE_W_RV( (Fr_AbsTimerIdx <= Fr_Cfg.Fr_ConfigPtr->FrCtrlParam[Fr_CtrlIdx].FrArcAbsTimerMaxIdx),
            FR_DISABLEABSTIMERIRQ_ID, FR_E_INV_TIMER_IDX, E_NOT_OK );

    /* @req FR320 */
    Fr_Internal_DisableAbsTimerIrq(&Fr_Cfg, Fr_CtrlIdx, Fr_AbsTimerIdx);

    return E_OK;
}

/**
 * Enables the interrupt line of an absolute timer.
 * @param Fr_CtrlIdx
 * @param Fr_AbsTimerIdx
 * @return
 */
/* @req FR034 */
Std_ReturnType Fr_EnableAbsoluteTimerIRQ(uint8 Fr_CtrlIdx, uint8 Fr_AbsTimerIdx) {

    /* @req FR294 */
    VALIDATE_W_RV( (Fr_Cfg.Fr_HasIntiailized == TRUE), FR_ENABLEABSTIMERIRQ_ID, FR_E_NOT_INITIALIZED, E_NOT_OK );
    /* @req FR295 */
    VALIDATE_W_RV( (Fr_CtrlIdx < FR_ARC_CTRL_CONFIG_CNT), FR_ENABLEABSTIMERIRQ_ID, FR_E_INV_CTRL_IDX, E_NOT_OK );
    /* @req FR296 */
    VALIDATE_W_RV( (Fr_AbsTimerIdx <= Fr_Cfg.Fr_ConfigPtr->FrCtrlParam[Fr_CtrlIdx].FrArcAbsTimerMaxIdx),
            FR_ENABLEABSTIMERIRQ_ID, FR_E_INV_TIMER_IDX, E_NOT_OK );

    /* @req FR298 */
    Fr_Internal_EnableAbsTimerIrq(&Fr_Cfg, Fr_CtrlIdx, Fr_AbsTimerIdx);

    return E_OK;
}

/**
 * Returns the version information of this module.
 * @param VersioninfoPtr
 */
/* @req FR070 */
/* @req FR342 */
#if ( FR_VERSION_INFO_API == STD_ON )
void Fr_GetVersionInfo(Std_VersionInfoType* VersioninfoPtr) {

    /* @req FR340 */
    VALIDATE( ( VersioninfoPtr != NULL ), FR_GETVERSIONINFO_ID, FR_E_INV_POINTER);

    /* @req FR341 */
    VersioninfoPtr->vendorID = FR_VENDOR_ID;
    VersioninfoPtr->moduleID = FR_MODULE_ID;
    VersioninfoPtr->sw_major_version = FR_SW_MAJOR_VERSION;
    VersioninfoPtr->sw_minor_version = FR_SW_MINOR_VERSION;
    VersioninfoPtr->sw_patch_version = FR_SW_PATCH_VERSION;
}
#endif
