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
#ifndef FRIF_INTERNAL_H_
#define FRIF_INTERNAL_H_
#include "FrIf.h"

//lint -emacro(904,FRIF_DET_REPORTERROR) //904 PC-Lint exception to MISRA 14.7 (validate DET macros).

/** PduInfo run time data */
typedef struct {
    boolean pduUpdated;                       /* Rx update status */
    uint8 pduRxLen;						/* Length of latest Pdu received */
} FrIf_Internal_rxPduStatisticsType;

/** Cluster runtime properties */
typedef struct {
    FrIf_StateType clstrState;     		   /* Internal State Machine for each cluster*/
    uint8 jobListCntr; 					   /* Job list counter for each cluster */
    boolean jobListSyncLost;      		   /* Flag to indicate loss of joblist synchronization */
    boolean jobListExecLock; /* Lock FrIf_JobListExec function for only one interrupt */
    boolean interruptEnableSts; /* Flag to indicate if interrupts are enabled for job list function */
} FrIf_Internal_ClusterRuntimeType;

/** Call back functions */
typedef struct {
    uint8 trigTxCounter;     /* Tx tiggering counter */
    uint8 txConfCounter;     /* Tx confirmation*/
} FrIf_Internal_txPduStatisticsType;

typedef struct {
    uint8 lSduBuffer[FRIF_MAX_N_LPDU][FRIF_MAX_LPDU_LEN]; /* LSdu buffers to hold each frame structure*/
    FrIf_Internal_rxPduStatisticsType rxPduStatistics[FRIF_MAX_N_RX_PDU];  /* Pointer to run time Rx Pdu status */
    FrIf_Internal_txPduStatisticsType txPduStatistics[FRIF_MAX_N_TX_PDU];       /* Pdu statistics for Tx LPdus*/
    FrIf_Internal_ClusterRuntimeType  clstrRunTimeData[FRIF_MAX_N_CLUSTER]; /* Runtime properties of cluster */
    boolean initDone;                               		/* Init status */
}FrIf_Internal_GlobalType;


#if (FRIF_DEV_ERROR_DETECT == STD_ON)/* @req FrIf05146 */
#define FRIF_DET_REPORTERROR(_cond ,_api, _error, ...) \
    if (!_cond) { \
        (void)Det_ReportError(FRIF_MODULE_ID, 0, _api, _error);          /* @req FrIf05299 */ \
        return __VA_ARGS__; \
    }
#else
#define FRIF_DET_REPORTERROR(_cond ,_api, _error, ...) \
    if (!_cond) { \
        return __VA_ARGS__; \
    }
#endif

/*lint -save -e9023 */
#if (FRIF_GET_CHNL_STATUS_SUPPORT == STD_ON)
#define FRIF_CLUSTER_DEM_REPORTING(_chnl) \
if (FRIF_INVALID_DEM_EVENT_ID != clstrCfg->FrIf_ClusterDemEventParamRef->FrIf_NITEventRefChnl##_chnl) {                                                 \
    if ((*FrIf_Channel##_chnl##StatusPtr & NIT_ERROR_MASK) != 0) {                              \
        Dem_ReportErrorStatus(clstrCfg->FrIf_ClusterDemEventParamRef->FrIf_NITEventRefChnl##_chnl, DEM_EVENT_STATUS_FAILED);      \
    } else {                                                                                                               \
        Dem_ReportErrorStatus(clstrCfg->FrIf_ClusterDemEventParamRef->FrIf_NITEventRefChnl##_chnl, DEM_EVENT_STATUS_PASSED);      \
    } \
} \
\
if (FRIF_INVALID_DEM_EVENT_ID != clstrCfg->FrIf_ClusterDemEventParamRef->FrIf_SWEventRefChnl##_chnl) {                                                 \
    if ((*FrIf_Channel##_chnl##StatusPtr & SW_ERROR_MASK) != 0) {                              \
        Dem_ReportErrorStatus(clstrCfg->FrIf_ClusterDemEventParamRef->FrIf_SWEventRefChnl##_chnl, DEM_EVENT_STATUS_FAILED);      \
    } else {                                                                                                               \
        Dem_ReportErrorStatus(clstrCfg->FrIf_ClusterDemEventParamRef->FrIf_SWEventRefChnl##_chnl, DEM_EVENT_STATUS_PASSED);      \
    }\
}\
\
if (FRIF_INVALID_DEM_EVENT_ID != clstrCfg->FrIf_ClusterDemEventParamRef->FrIf_ACSEventRefChnl##_chnl) {                                                 \
    if ((*FrIf_Channel##_chnl##StatusPtr & CHNL_ACS_ERROR_MASK) != 0) {                              \
        Dem_ReportErrorStatus(clstrCfg->FrIf_ClusterDemEventParamRef->FrIf_ACSEventRefChnl##_chnl, DEM_EVENT_STATUS_FAILED);      \
    } else {                                                                                                               \
        Dem_ReportErrorStatus(clstrCfg->FrIf_ClusterDemEventParamRef->FrIf_ACSEventRefChnl##_chnl, DEM_EVENT_STATUS_PASSED);      \
    }\
}
#endif
/*lint -restore */

#define FIRST_JOB_IDX					0u  /* First job index in a cluster */
#define FIRST_FRIF_CTRL_IDX				0u /* First FrIf Controller index in a cluster */
#define MIN_SLOT_DURATION_FOR_NEXT_JOB  2uL /* Minimum time difference to activate the next job */
#define FLEXRAY_FRAME_ID_MIN            0x1u /* Minimum frame Id value */
#define FLEXRAY_FRAME_ID_MAX            0x7FFu /* Maximum frame Id value */
#define FLEXRAY_FRAME_LEN_MAX           0xFEu /* Maximum frame Len value */

/* Internal Functions */
void FrIf_Internal_HandleDecoupledTransmission(uint8 frIfIdx,PduIdType frLPduIdx, uint8 frameCfgIdx);
void FrIf_Internal_ProvideTxConfirmation(uint8 frIfIdx, PduIdType frLPduIdx, uint8 frameCfgIdx);
void FrIf_Internal_HandleReceiveAndStore(uint8 frIfIdx, PduIdType frLPduIdx, uint8 frameCfgIdx);
void FrIf_Internal_HandleReceiveAndIndicate(uint8 frIfIdx, PduIdType frLPduIdx, uint8 frameCfgIdx, uint16 maxLoop );
void FrIf_Internal_ProvideRxIndication(uint8 frIfIdx, PduIdType frLPduIdx, uint8 frameCfgIdx );
void FrIf_Internal_PrepareLPdu(uint8 frIfIdx, PduIdType frLPduIdx);

#if (FRIF_BIG_ENDIAN_USED == STD_ON)
void FrIf_Internal_SwapToBigEndian(const uint8 *srcBuffer,uint8 *destBuffer, uint8 length);
#endif

#ifdef HOST_TEST
FrIf_Internal_txPduStatisticsType readPduStatistics(PduIdType pduId);
#endif
#endif /* FRIF_INTERNAL_H_ */

