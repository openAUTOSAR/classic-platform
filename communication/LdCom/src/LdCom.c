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

 /*=================================[inclusions]======================================*/
/* General requirements */
/* @req 4.2.2/SWS_LDCOM_00051 */ /* LdCom_Lcfg.c and LdComPBcfg.c files are generated */
/* @req 4.2.2/SWS_LDCOM_00018 */ /* Development Error Types */
/* @req 4.2.2/SWS_LDCOM_00035 */ /* defines external interfaces, required to fulfill an optional functionality of the module */
/* @req 4.2.2/SWS_LDCOM_00020 */ /* ComStack_Types and Std_Types are included*/
/* @req 4.2.2/SWS_LDCOM_00052 */ /* LdCom_ConfigType implementation specific structure*/
/* @req 4.2.2/SWS_LDCOM_00036 */ /* Rte_LdComCbkCopyTxData, Large Data COM shall support Transport Protocol-like communication */
/* @req 4.2.2/SWS_LDCOM_00037 */ /* Rte_LdComCbkTpTxConfirmation, Large Data COM shall support Transport Protocol-like communication */
/* @req 4.2.2/SWS_LDCOM_00038 */ /* Rte_LdComCbkStartOfReception, Large Data COM shall support Transport Protocol-like communication */
/* @req 4.2.2/SWS_LDCOM_00039 */ /* Rte_LdComCbkCopyRxData, Large Data COM shall support Transport Protocol-like communication */
/* @req 4.2.2/SWS_LDCOM_00040 */ /* Rte_LdComCbkTpRxIndication, Large Data COM shall support Transport Protocol-like communication */
/* @req 4.2.2/SWS_LDCOM_00041 */ /* Rte_LdComCbkRxIndication, Large Data COM shall support Interface-like communication */
/* @req 4.2.2/SWS_LDCOM_00042 */ /* Rte_LdComCbkTriggerTransmit, Large Data COM shall support Transmission Triggered by lower layer */
/* @req 4.2.2/SWS_LDCOM_00053 */ /* Rte_LdComCbkTxConfirmation, AUTOSAR COM and LargeDataCOM shall provide a transmit confirmation function */
/*lint -save -e9046 */
#include "LdCom.h"
#include "LdCom_internal.h"
#include "PduR_LdCom.h"     /* @req 4.2.2/SWS_LDCOM_00001*/
#include "Rte_Cbk.h"        /* @req 4.2.2/SWS_LDCOM_00001*/

#if(LDCOM_DEV_ERROR_DETECT == STD_ON)
#include "Det.h"            /* @req 4.2.2/SWS_LDCOM_00050*/
#endif

const LdCom_ConfigType* LdComCfgPtr;
#define  INVALID_LENGTH 0x00u;
/** Static declarations */
LdCom_InternalType LdCom_Internal = {
    .initStatus = LDCOM_STATE_UNINIT,
};

/*lint -emacro(904,LDCOM_DET_REPORTERROR)*/ /*904 PC-Lint exception to MISRA 14.7 (validate DET macros)*/

/* @req 4.2.2/SWS_LDCOM_00050*/
#if (LDCOM_DEV_ERROR_DETECT == STD_ON)
#define LDCOM_DET_REPORTERROR(_cond ,_api, _error, ...) \
    if (!_cond) { \
        (void)Det_ReportError(LDCOM_MODULE_ID, 0, _api, _error); \
        return __VA_ARGS__; \
    }
#else
#define LDCOM_DET_REPORTERROR(_cond ,_api, _error, ...) \
    if (!_cond) { \
        return __VA_ARGS__; \
    }
#endif

/** AUTOSAR APIs */

/* @req 4.2.2/SWS_LDCOM_00022*/
void LdCom_Init( const LdCom_ConfigType* config ){
    LDCOM_DET_REPORTERROR((NULL != config),LDCOM_SERVICE_ID_INIT, LDCOM_E_UNINIT);
    /* @req 4.2.2/SWS_LDCOM_00007*/
    LdComCfgPtr = config;
    LdCom_Internal.initStatus = LDCOM_STATE_INIT;
}

/* @req 4.2.2/SWS_LDCOM_00023*/
void LdCom_DeInit( void ){

    LdCom_Internal.initStatus = LDCOM_STATE_UNINIT;
    /* @req 4.2.2/SWS_LDCOM_00008*/
    LdComCfgPtr = NULL;
}

/* @req 4.2.2/SWS_LDCOM_00045*/
#if (LDCOM_VERSION_INFO_API == STD_ON)
/* @req 4.2.2/SWS_LDCOM_00024*/
void LdCom_GetVersionInfo( Std_VersionInfoType* versioninfo ){
    LDCOM_DET_REPORTERROR((NULL != versioninfo),LDCOM_SERVICE_ID_GET_VERSION_INFO,LDCOM_E_PARAM_POINTER);

    versioninfo->moduleID = LDCOM_MODULE_ID;  /* Module ID of LDCOM */
    versioninfo->vendorID = LDCOM_VENDOR_ID;  /* Vendor Id (ARCCORE) */

    /* return the Software Version numbers*/
    versioninfo->sw_major_version = LDCOM_SW_MAJOR_VERSION;
    versioninfo->sw_minor_version = LDCOM_SW_MINOR_VERSION;
    versioninfo->sw_patch_version = LDCOM_SW_PATCH_VERSION;
}
#endif

/* @req 4.2.2/SWS_LDCOM_00026*/
Std_ReturnType LdCom_Transmit( PduIdType Id, const PduInfoType* PduInfoPtr ){
    Std_ReturnType ret;
    uint8 i;

    LDCOM_DET_REPORTERROR((LDCOM_STATE_INIT == LdCom_Internal.initStatus),LDCOM_SERVICE_ID_TRANSMIT, LDCOM_E_UNINIT, E_NOT_OK);
    LDCOM_DET_REPORTERROR((NULL_PTR != PduInfoPtr),LDCOM_SERVICE_ID_TRANSMIT,LDCOM_E_PARAM_POINTER, E_NOT_OK);
    LDCOM_DET_REPORTERROR((NULL_PTR != PduInfoPtr->SduDataPtr),LDCOM_SERVICE_ID_TRANSMIT,LDCOM_E_PARAM_POINTER, E_NOT_OK);

    for(i=0; i < LdComCfgPtr->LdComIPduCnt; i++){
       if (Id == LdComCfgPtr->LdComIPduCfg[i].LdComHandleId ) {
           break;
       }
    }
    LDCOM_DET_REPORTERROR((i < LdComCfgPtr->LdComIPduCnt ),LDCOM_SERVICE_ID_TRANSMIT,LDCOM_E_PARAM, E_NOT_OK);
    /* @req 4.2.2/SWS_LDCOM_00010*/
    /* @req 4.2.2/SWS_LDCOM_00012*/
    /* @req 4.2.2/SWS_LDCOM_00009*/
    if (LdComCfgPtr->LdComIPduCfg[i].LdComIPduDir == LDCOM_SEND) {
        ret = PduR_LdComTransmit(LdComCfgPtr->LdComIPduCfg[i].LdComPdurPduId, PduInfoPtr);
    } else {
        ret = E_NOT_OK;
    }
    return ret;
}

/* @req 4.2.2/SWS_LDCOM_00027*/
BufReq_ReturnType LdCom_CopyTxData( PduIdType id, const PduInfoType* info, RetryInfoType* retry, PduLengthType* availableDataPtr ){
    BufReq_ReturnType ret = BUFREQ_NOT_OK;
    uint8 i;

    LDCOM_DET_REPORTERROR((LDCOM_STATE_INIT == LdCom_Internal.initStatus),LDCOM_SERVICE_ID_COPY_TX_DATA, LDCOM_E_UNINIT, BUFREQ_NOT_OK);
    LDCOM_DET_REPORTERROR((NULL_PTR != info),LDCOM_SERVICE_ID_COPY_TX_DATA,LDCOM_E_PARAM_POINTER, BUFREQ_NOT_OK);
    LDCOM_DET_REPORTERROR((NULL_PTR != info->SduDataPtr),LDCOM_SERVICE_ID_COPY_TX_DATA,LDCOM_E_PARAM_POINTER, BUFREQ_NOT_OK);
    LDCOM_DET_REPORTERROR((NULL_PTR != retry),LDCOM_SERVICE_ID_COPY_TX_DATA,LDCOM_E_PARAM_POINTER, BUFREQ_NOT_OK);
    LDCOM_DET_REPORTERROR((NULL_PTR != availableDataPtr),LDCOM_SERVICE_ID_COPY_TX_DATA,LDCOM_E_PARAM_POINTER, BUFREQ_NOT_OK);

    for(i=0; i < LdComCfgPtr->LdComIPduCnt; i++){
       if (id == LdComCfgPtr->LdComIPduCfg[i].LdComHandleId ) {
           break;
       }
    }
    LDCOM_DET_REPORTERROR((i < LdComCfgPtr->LdComIPduCnt ),LDCOM_SERVICE_ID_COPY_TX_DATA,LDCOM_E_PARAM, BUFREQ_NOT_OK);

    /* @req 4.2.2/SWS_LDCOM_00048*/
    /* @req 4.2.2/SWS_LDCOM_00005*/
    if((LdComCfgPtr->LdComIPduCfg[i].LdComIPduDir == LDCOM_SEND) &&(LdComCfgPtr->LdComIPduCfg[i].LdComLowerApi ==LDCOM_TP)){
        if (LdComCfgPtr->LdComIPduCfg[i].LdComCopyTxDataCbk != NULL) {
            /* @req 4.2.2/SWS_LDCOM_00013*/
            ret = LdComCfgPtr->LdComIPduCfg[i].LdComCopyTxDataCbk(info, retry, availableDataPtr );
        } else {
            ret = BUFREQ_NOT_OK;
        }
    } else {
        ret = BUFREQ_NOT_OK;
    }
    return ret;
}

/* @req 4.2.2/SWS_LDCOM_00028*/
void LdCom_TpTxConfirmation( PduIdType id, Std_ReturnType result ){
    uint8 i;
    LDCOM_DET_REPORTERROR((LDCOM_STATE_INIT == LdCom_Internal.initStatus),LDCOM_SERVICE_ID_TP_TX_CONFIRMATION, LDCOM_E_UNINIT);
    LDCOM_DET_REPORTERROR((E_OK == result),LDCOM_SERVICE_ID_TP_TX_CONFIRMATION,LDCOM_E_PARAM);

    for(i=0; i < LdComCfgPtr->LdComIPduCnt; i++)
    {
       if (id == LdComCfgPtr->LdComIPduCfg[i].LdComHandleId ) {
           break;
       }
    }
    LDCOM_DET_REPORTERROR((i < LdComCfgPtr->LdComIPduCnt ),LDCOM_SERVICE_ID_TP_TX_CONFIRMATION,LDCOM_E_PARAM);

    /* @req 4.2.2/SWS_LDCOM_00048*/
    /* @req 4.2.2/SWS_LDCOM_00005*/
    if((LdComCfgPtr->LdComIPduCfg[i].LdComIPduDir == LDCOM_SEND) &&(LdComCfgPtr->LdComIPduCfg[i].LdComLowerApi ==LDCOM_TP)){
        if (LdComCfgPtr->LdComIPduCfg[i].LdComTpTxConfirmationCbk != NULL) {
            /* @req 4.2.2/SWS_LDCOM_00013*/
            LdComCfgPtr->LdComIPduCfg[i].LdComTpTxConfirmationCbk(result );
        }
    }
}

/* @req 4.2.2/SWS_LDCOM_00029*/
BufReq_ReturnType LdCom_StartOfReception( PduIdType id, const PduInfoType* info, PduLengthType TpSduLength, PduLengthType* bufferSizePtr ){
    BufReq_ReturnType ret = BUFREQ_NOT_OK;
    uint8 i;
    LDCOM_DET_REPORTERROR((LDCOM_STATE_INIT == LdCom_Internal.initStatus),LDCOM_SERVICE_ID_START_OF_RECEPTION, LDCOM_E_UNINIT, BUFREQ_NOT_OK);
    LDCOM_DET_REPORTERROR((NULL != info),LDCOM_SERVICE_ID_START_OF_RECEPTION,LDCOM_E_PARAM_POINTER, BUFREQ_NOT_OK);
    LDCOM_DET_REPORTERROR((NULL != info->SduDataPtr),LDCOM_SERVICE_ID_START_OF_RECEPTION,LDCOM_E_PARAM_POINTER, BUFREQ_NOT_OK);
    LDCOM_DET_REPORTERROR(((PduLengthType)0 != TpSduLength ), LDCOM_SERVICE_ID_START_OF_RECEPTION,LDCOM_E_PARAM, BUFREQ_NOT_OK);
    LDCOM_DET_REPORTERROR((NULL != bufferSizePtr),LDCOM_SERVICE_ID_START_OF_RECEPTION,LDCOM_E_PARAM_POINTER, BUFREQ_NOT_OK);

    for(i=0; i < LdComCfgPtr->LdComIPduCnt; i++){
       if (id == LdComCfgPtr->LdComIPduCfg[i].LdComHandleId ) {
           break;
       }
    }
    LDCOM_DET_REPORTERROR((i < LdComCfgPtr->LdComIPduCnt ),LDCOM_SERVICE_ID_START_OF_RECEPTION,LDCOM_E_PARAM, BUFREQ_NOT_OK);
    /* @req 4.2.2/SWS_LDCOM_00049*/
    /* @req 4.2.2/SWS_LDCOM_00005*/
    if((LdComCfgPtr->LdComIPduCfg[i].LdComIPduDir == LDCOM_RECEIVE )&&(LdComCfgPtr->LdComIPduCfg[i].LdComLowerApi ==LDCOM_TP)){
        if (LdComCfgPtr->LdComIPduCfg[i].LdComRxStartOfReceptionCbk != NULL) {
            /* @req 4.2.2/SWS_LDCOM_00015*/
            ret = LdComCfgPtr->LdComIPduCfg[i].LdComRxStartOfReceptionCbk(info, TpSduLength, bufferSizePtr );
        } else {
            ret = BUFREQ_NOT_OK;
        }
    } else {
        ret = BUFREQ_NOT_OK;
    }
    return ret;
}
/* @req 4.2.2/SWS_LDCOM_00030*/
BufReq_ReturnType LdCom_CopyRxData( PduIdType id, const PduInfoType* info, PduLengthType* bufferSizePtr ){

    BufReq_ReturnType ret = BUFREQ_NOT_OK;
    uint8 i;
    LDCOM_DET_REPORTERROR((LDCOM_STATE_INIT == LdCom_Internal.initStatus),LDCOM_SERVICE_ID_COPY_RX_DATA, LDCOM_E_UNINIT, BUFREQ_NOT_OK);
    LDCOM_DET_REPORTERROR((NULL != info),LDCOM_SERVICE_ID_COPY_RX_DATA,LDCOM_E_PARAM_POINTER, BUFREQ_NOT_OK);
    LDCOM_DET_REPORTERROR((NULL != info->SduDataPtr),LDCOM_SERVICE_ID_COPY_RX_DATA,LDCOM_E_PARAM_POINTER, BUFREQ_NOT_OK);
    LDCOM_DET_REPORTERROR((NULL != bufferSizePtr),LDCOM_SERVICE_ID_COPY_RX_DATA,LDCOM_E_PARAM_POINTER, BUFREQ_NOT_OK);

    for(i=0; i < LdComCfgPtr->LdComIPduCnt; i++){
       if (id == LdComCfgPtr->LdComIPduCfg[i].LdComHandleId ) {
           break;
       }
    }
    LDCOM_DET_REPORTERROR((i < LdComCfgPtr->LdComIPduCnt ),LDCOM_SERVICE_ID_COPY_RX_DATA,LDCOM_E_PARAM, BUFREQ_NOT_OK);

    /* @req 4.2.2/SWS_LDCOM_00049*/
    /* @req 4.2.2/SWS_LDCOM_00005*/
    if((LdComCfgPtr->LdComIPduCfg[i].LdComIPduDir == LDCOM_RECEIVE )&&(LdComCfgPtr->LdComIPduCfg[i].LdComLowerApi ==LDCOM_TP)){
        if (LdComCfgPtr->LdComIPduCfg[i].LdComCopyRxDataCbk != NULL) {
            /* @req 4.2.2/SWS_LDCOM_00016*/
            ret = LdComCfgPtr->LdComIPduCfg[i].LdComCopyRxDataCbk( info, bufferSizePtr );
        } else {
            ret = BUFREQ_NOT_OK;
        }
    } else {
        ret = BUFREQ_NOT_OK;
     }
    return ret;
}

/* @req 4.2.2/SWS_LDCOM_00031*/
void LdCom_TpRxIndication( PduIdType id, Std_ReturnType result ){
    uint8 i;
    LDCOM_DET_REPORTERROR((LDCOM_STATE_INIT == LdCom_Internal.initStatus),LDCOM_SERVICE_ID_TP_RX_INDICATION, LDCOM_E_UNINIT);
    LDCOM_DET_REPORTERROR((E_OK == result),LDCOM_SERVICE_ID_TP_RX_INDICATION,LDCOM_E_PARAM);
    for(i=0; i < LdComCfgPtr->LdComIPduCnt; i++){
       if (id == LdComCfgPtr->LdComIPduCfg[i].LdComHandleId ) {
           break;
       }
    }
    LDCOM_DET_REPORTERROR((i < LdComCfgPtr->LdComIPduCnt ),LDCOM_SERVICE_ID_TP_RX_INDICATION,LDCOM_E_PARAM);
    /* @req 4.2.2/SWS_LDCOM_00005*/
    /* @req 4.2.2/SWS_LDCOM_00049*/
    if((LdComCfgPtr->LdComIPduCfg[i].LdComIPduDir == LDCOM_RECEIVE )&&(LdComCfgPtr->LdComIPduCfg[i].LdComLowerApi ==LDCOM_TP)){
        if(LdComCfgPtr->LdComIPduCfg[i].LdComTpRxIndicationCbk != NULL){
            /* @req 4.2.2/SWS_LDCOM_00017*/
            LdComCfgPtr->LdComIPduCfg[i].LdComTpRxIndicationCbk( result );//Rte_LdComCbkTpRxIndication_<sn>
        }
    }
}

/* @req 4.2.2/SWS_LDCOM_00032*/
void LdCom_RxIndication( PduIdType RxPduId, const PduInfoType* PduInfoPtr ){
    uint8 i;
    LDCOM_DET_REPORTERROR((LDCOM_STATE_INIT == LdCom_Internal.initStatus),LDCOM_SERVICE_ID_RX_INDICATION, LDCOM_E_UNINIT);
    LDCOM_DET_REPORTERROR((NULL != PduInfoPtr),LDCOM_SERVICE_ID_RX_INDICATION,LDCOM_E_PARAM_POINTER);
    LDCOM_DET_REPORTERROR((NULL != PduInfoPtr->SduDataPtr),LDCOM_SERVICE_ID_RX_INDICATION,LDCOM_E_PARAM_POINTER);

    for(i=0; i < LdComCfgPtr->LdComIPduCnt; i++){
       if (RxPduId == LdComCfgPtr->LdComIPduCfg[i].LdComHandleId ) {
           break;
       }
    }
    LDCOM_DET_REPORTERROR((i < LdComCfgPtr->LdComIPduCnt ),LDCOM_SERVICE_ID_RX_INDICATION,LDCOM_E_PARAM);
    /* @req 4.2.2/SWS_LDCOM_00005*/
    /* @req 4.2.2/SWS_LDCOM_00055*/
    if((LdComCfgPtr->LdComIPduCfg[i].LdComIPduDir == LDCOM_RECEIVE )&&(LdComCfgPtr->LdComIPduCfg[i].LdComLowerApi ==LDCOM_IF)){
        if(LdComCfgPtr->LdComIPduCfg[i].LdComRxIndicationCbk != NULL){
            /* @req 4.2.2/SWS_LDCOM_00014*/
            LdComCfgPtr->LdComIPduCfg[i].LdComRxIndicationCbk( PduInfoPtr);
        }
    }
}

/* @req 4.2.2/SWS_LDCOM_00056*/
void LdCom_TxConfirmation( PduIdType TxPduId ){
    uint8 i;
    LDCOM_DET_REPORTERROR((LDCOM_STATE_INIT == LdCom_Internal.initStatus),LDCOM_SERVICE_ID_TX_CONFIRMATION, LDCOM_E_UNINIT);

    for(i=0; i < LdComCfgPtr->LdComIPduCnt; i++){
       if (TxPduId == LdComCfgPtr->LdComIPduCfg[i].LdComHandleId ) {
           break;
       }
    }
    LDCOM_DET_REPORTERROR((i < LdComCfgPtr->LdComIPduCnt ),LDCOM_SERVICE_ID_TX_CONFIRMATION,LDCOM_E_PARAM);
    /* @req 4.2.2/SWS_LDCOM_00054*/
    /* @req 4.2.2/SWS_LDCOM_00005*/
    if((LdComCfgPtr->LdComIPduCfg[i].LdComIPduDir == LDCOM_SEND) &&(LdComCfgPtr->LdComIPduCfg[i].LdComLowerApi ==LDCOM_IF)){
        if(LdComCfgPtr->LdComIPduCfg[i].LdComTxConfirmationCbk != NULL){
            /* @req 4.2.2/SWS_LDCOM_00046*/
            LdComCfgPtr->LdComIPduCfg[i].LdComTxConfirmationCbk();
        }
    }
}

/* @req 4.2.2/SWS_LDCOM_00033*/
Std_ReturnType LdCom_TriggerTransmit( PduIdType TxPduId, PduInfoType* PduInfoPtr ){
    uint8 i;
    Std_ReturnType ret = E_NOT_OK;
    LDCOM_DET_REPORTERROR((LDCOM_STATE_INIT == LdCom_Internal.initStatus),LDCOM_SERVICE_ID_TRIGGER_TRANSMIT, LDCOM_E_UNINIT, E_NOT_OK);
    LDCOM_DET_REPORTERROR((NULL_PTR != PduInfoPtr),LDCOM_SERVICE_ID_TRIGGER_TRANSMIT,LDCOM_E_PARAM_POINTER, E_NOT_OK);
    LDCOM_DET_REPORTERROR((NULL_PTR != PduInfoPtr->SduDataPtr),LDCOM_SERVICE_ID_TRIGGER_TRANSMIT,LDCOM_E_PARAM_POINTER, E_NOT_OK);

    for(i=0; i < LdComCfgPtr->LdComIPduCnt; i++){
       if (TxPduId == LdComCfgPtr->LdComIPduCfg[i].LdComHandleId ) {
           break;
       }
    }
    LDCOM_DET_REPORTERROR((i < LdComCfgPtr->LdComIPduCnt ),LDCOM_SERVICE_ID_TRIGGER_TRANSMIT,LDCOM_E_PARAM, E_NOT_OK);
    /* @req 4.2.2/SWS_LDCOM_00005*/
    /* @req 4.2.2/SWS_LDCOM_00047*/
    /* @req 4.2.2/SWS_LDCOM_00005*/
    if(LdComCfgPtr->LdComIPduCfg[i].LdComTxTriggerTransmitCbk != NULL){
        /* @req 4.2.2/SWS_LDCOM_00011*/
        ret = LdComCfgPtr->LdComIPduCfg[i].LdComTxTriggerTransmitCbk(PduInfoPtr );
    }else{
        ret = E_NOT_OK;
    }
    return ret;
}

#ifdef HOST_TEST
LdCom_InternalType* readinternal_status(void );
LdCom_InternalType* readinternal_status(void)
{
    return &LdCom_Internal;
}
#endif
/*lint -restore */
/*END*/
