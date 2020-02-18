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

/* General requirements on Com module */
/* @req COM584 */
/* !req COM220 */ /* Include file structure */
/* !req COM673 */ /* Version check */
/* !req COM010 */ /* IMPROVEMENT: OSEK-COM, what is implemented? */
/* !req COM011 */ /* IMPROVEMENT: OSEK-COM, what is implemented? */
/* !req COM012 */ /* IMPROVEMENT: OSEK-COM, what is implemented? */
/* !req COM013 */ /* IMPROVEMENT: OSEK-COM, what is implemented? */
/* COM759, COM760: ComIPduType not used */
/* !req COM759 */
/* !req COM760 */
/* @req COM059 */ /* Interpretation of update-bit */
/* !req COM320 */ /* !req COM321 */ /* Reentrant/Non-reentrant functions */
/* @req COM606 */ /* Pre-compile */
/* @req COM608 */ /* Post-build */

#include "arc_assert.h"
#include <string.h>
#include "Com.h"
#include "Com_Arc_Types.h"
#include "Com_Internal.h"
#include "Com_misc.h"
#include "Cpu.h"
#include "SchM_Com.h"
#include "debug.h"


#define COM_START_SEC_VAR_INIT_UNSPECIFIED
#include "Com_MemMap.h"  /*lint !e9019 OTHER [MISRA 2012 Rule 20.1, advisory] OTHER AUTOSAR specified way of using MemMap*/
static Com_StatusType initStatus = COM_UNINIT;
#define COM_STOP_SEC_VAR_INIT_UNSPECIFIED
#include "Com_MemMap.h"  /*lint !e9019 OTHER [MISRA 2012 Rule 20.1, advisory] OTHER AUTOSAR specified way of using MemMap*/

#define COM_START_SEC_VAR_CLEARED_UNSPECIFIED
#include "Com_MemMap.h"  /*lint !e9019 OTHER [MISRA 2012 Rule 20.1, advisory] OTHER AUTOSAR specified way of using MemMap*/
const Com_ConfigType * ComConfig;
#define COM_STOP_SEC_VAR_CLEARED_UNSPECIFIED
#include "Com_MemMap.h"  /*lint !e9019 OTHER [MISRA 2012 Rule 20.1, advisory] OTHER AUTOSAR specified way of using MemMap*/

#define COM_START_SEC_VAR_CLEARED_UNSPECIFIED
#include "Com_MemMap.h"  /*lint !e9019 OTHER [MISRA 2012 Rule 20.1, advisory] OTHER AUTOSAR specified way of using MemMap*/
static Com_Arc_IPdu_type Com_Arc_IPdu[COM_MAX_N_IPDUS];
#define COM_STOP_SEC_VAR_CLEARED_UNSPECIFIED
#include "Com_MemMap.h"  /*lint !e9019 OTHER [MISRA 2012 Rule 20.1, advisory] OTHER AUTOSAR specified way of using MemMap*/


#define COM_START_SEC_VAR_CLEARED_UNSPECIFIED
#include "Com_MemMap.h"  /*lint !e9019 OTHER [MISRA 2012 Rule 20.1, advisory] OTHER AUTOSAR specified way of using MemMap*/
static Com_Arc_Signal_type Com_Arc_Signal[COM_MAX_N_SIGNALS];
#define COM_STOP_SEC_VAR_CLEARED_UNSPECIFIED
#include "Com_MemMap.h"  /*lint !e9019 OTHER [MISRA 2012 Rule 20.1, advisory] OTHER AUTOSAR specified way of using MemMap*/

#define COM_START_SEC_VAR_CLEARED_UNSPECIFIED
#include "Com_MemMap.h"  /*lint !e9019 OTHER [MISRA 2012 Rule 20.1, advisory] OTHER AUTOSAR specified way of using MemMap*/
Com_BufferPduStateType Com_BufferPduState[COM_MAX_N_IPDUS]={{0,0}};
#define COM_STOP_SEC_VAR_CLEARED_UNSPECIFIED
#include "Com_MemMap.h"  /*lint !e9019 OTHER [MISRA 2012 Rule 20.1, advisory] OTHER AUTOSAR specified way of using MemMap*/


#if (COM_SIG_GATEWAY_ENABLE == STD_ON)
#define COM_START_SEC_VAR_CLEARED_UNSPECIFIED
#include "Com_MemMap.h"  /*lint !e9019 OTHER [MISRA 2012 Rule 20.1, advisory] OTHER AUTOSAR specified way of using MemMap*/
static Com_Arc_GwSrcDesc_type Com_Arc_GwSrcDescSignals[COM_MAX_N_SUPPORTED_GWSOURCE_DESCRIPTIONS] = {{0}};
#define COM_STOP_SEC_VAR_CLEARED_UNSPECIFIED
#include "Com_MemMap.h"  /*lint !e9019 OTHER [MISRA 2012 Rule 20.1, advisory] OTHER AUTOSAR specified way of using MemMap*/
#endif

#if (COM_MAX_N_GROUP_SIGNALS != 0)
#define COM_START_SEC_VAR_CLEARED_UNSPECIFIED
#include "Com_MemMap.h"  /*lint !e9019 OTHER [MISRA 2012 Rule 20.1, advisory] OTHER AUTOSAR specified way of using MemMap*/
static Com_Arc_GroupSignal_type Com_Arc_GroupSignal[COM_MAX_N_GROUP_SIGNALS];
#define COM_STOP_SEC_VAR_CLEARED_UNSPECIFIED
#include "Com_MemMap.h"  /*lint !e9019 OTHER [MISRA 2012 Rule 20.1, advisory] OTHER AUTOSAR specified way of using MemMap*/
#endif

/* Data, deferred data and shadow buffer */
#define COM_START_SEC_VAR_CLEARED_UNSPECIFIED
#include "Com_MemMap.h"  /*lint !e9019 OTHER [MISRA 2012 Rule 20.1, advisory] OTHER AUTOSAR specified way of using MemMap*/
static uint8 Com_Arc_Buffer[COM_MAX_BUFFER_SIZE];
#define COM_STOP_SEC_VAR_CLEARED_UNSPECIFIED
#include "Com_MemMap.h"  /*lint !e9019 OTHER [MISRA 2012 Rule 20.1, advisory] OTHER AUTOSAR specified way of using MemMap*/

const Com_Arc_Config_type Com_Arc_Config = {
    .ComIPdu            = Com_Arc_IPdu,
    .ComSignal          = Com_Arc_Signal,
#if (COM_MAX_N_GROUP_SIGNALS != 0)
    .ComGroupSignal     = Com_Arc_GroupSignal,
#else
    .ComGroupSignal     = NULL,
#endif
#if (COM_SIG_GATEWAY_ENABLE == STD_ON)
    .ComGwSrcDescSignal = Com_Arc_GwSrcDescSignals
#else
    .ComGwSrcDescSignal = NULL
#endif
};



void Com_Init(const Com_ConfigType *config ) {


    DEBUG(DEBUG_LOW, "--Initialization of COM--\n");
    /* !req COM328 */ /* Shall not enable inter-ECU communication */
    /* !req COM483 */

    /* @req COM128 */
    /* @req COM217 */
    /* @req COM444 */
    /* @req COM772 */ /* If timeout set to 0*/

    boolean failure = FALSE;
    /* @req COM433 */
    if( NULL == config ) {
        DET_REPORTERROR(COM_INIT_ID, COM_E_PARAM_POINTER);
        /*lint -e{904} ARGUMENT CHECK */
        return;
    }
    ComConfig = config;

    boolean dataChanged = FALSE;

    const ComSignal_type *Signal;
    const ComGroupSignal_type *GroupSignal;
    uint16 bufferIndex = 0;

    for (uint16 i = 0; 0 == ComConfig->ComIPdu[i].Com_Arc_EOL; i++) {
        boolean pduHasGroupSignal = FALSE;
        const ComIPdu_type *IPdu = GET_IPdu(i);
        Com_Arc_IPdu_type *Arc_IPdu = GET_ArcIPdu(i);
        Arc_IPdu->Com_Arc_DynSignalLength = 0;
        Arc_IPdu->Com_Arc_IpduRxDMControl = TRUE; /* Enabling by default, assign 0 if not */
        Arc_IPdu->Com_Arc_TxIPduTimers.ComTxDMTimer =  0; /* Initialise Tx deadline timer*/
        if (i >= ComConfig->ComNofIPdus) {
            DET_REPORTERROR(COM_INIT_ID ,COM_E_TOO_MANY_IPDU);
            failure = TRUE;
            break;
        }
        /* Set the data ptr for this Pdu */
        Arc_IPdu->ComIPduDataPtr = (void *)&Com_Arc_Buffer[bufferIndex];
        bufferIndex += IPdu->ComIPduSize;
        /* Set TMS to TRUE as default */
        Arc_IPdu->Com_Arc_IpduTxMode = TRUE;
        /* If this is a TX and cyclic IPdu, configure the first deadline.*/
        if ( (IPdu->ComIPduDirection == COM_SEND) &&
             ((IPdu->ComTxIPdu.ComTxModeTrue.ComTxModeMode == COM_PERIODIC) || (IPdu->ComTxIPdu.ComTxModeTrue.ComTxModeMode == COM_MIXED))) {
            Arc_IPdu->Com_Arc_TxIPduTimers.ComTxModeTimePeriodTimer = IPdu->ComTxIPdu.ComTxModeTrue.ComTxModeTimeOffsetFactor + 1;
        }

        /* Initialize the memory with the default value.*/
        /* @req COM015 */
        if (IPdu->ComIPduDirection == COM_SEND) {
            memset((void *)Arc_IPdu->ComIPduDataPtr, IPdu->ComTxIPdu.ComTxIPduUnusedAreasDefault, IPdu->ComIPduSize);

            /* It is not clear if this value should be set 0 or the configured value. This means that the frames
             * will be sent the first time Com_MainFunctionTx()
             */
            Arc_IPdu->Com_Arc_TxIPduTimers.ComTxIPduMinimumDelayTimer = 0;
        }

        /* For each signal in this PDU.*/

        for (uint16 j = 0; (IPdu->ComIPduSignalRef != NULL) && (IPdu->ComIPduSignalRef[j] != NULL) ; j++) {
            Signal = IPdu->ComIPduSignalRef[j];
            Com_Arc_Signal_type * Arc_Signal = GET_ArcSignal(Signal->ComHandleId);

            /* Configure signal deadline monitoring if used.*/
            /* @req COM333 */ /* If timeout set to 0*/
            /* @req COM292 */ /* handled in generator*/
            /* @req COM290 */ /* handled also in generator*/
            /* the signal does not use an update bit, and should  use per I-PDU deadline monitoring.*/
            /* the signal uses an update bit, and hence has its own deadline monitoring.*/
            if (Signal->ComTimeoutFactor > 0){
                if(IPdu->ComIPduDirection == COM_RECEIVE ){
                    /* Configure the deadline counter */
                    Arc_Signal->Com_Arc_DeadlineCounter = Signal->ComFirstTimeoutFactor;
                } else {
                    /* @req COM445 */ /* handled in generator */
                    /* @req COM481 */
                    /* if one of the signal enabled for DM this means DM exists for this pdu */
                    Arc_IPdu->Com_Arc_TxDeadlineCounter = Signal->ComTimeoutFactor;
                }
            }

            /* Clear update bits*/
            /* @req COM117 */
            if (TRUE == Signal->ComSignalArcUseUpdateBit) {
                /*lint -e{9016} Array indexing couldn't be implemented, as parameters are of different data types */
                CLEARBIT(Arc_IPdu->ComIPduDataPtr, Signal->ComUpdateBitPosition);
            }

            /* If this signal is a signal group*/
            if (TRUE == Signal->Com_Arc_IsSignalGroup) {
                pduHasGroupSignal = TRUE;
                Arc_Signal->Com_Arc_ShadowBuffer = (void *)&Com_Arc_Buffer[bufferIndex];
                /* For each group signal of this signal group.*/
                for(uint8 h = 0; Signal->ComGroupSignal[h] != NULL; h++) {
                    GroupSignal = Signal->ComGroupSignal[h];
                    Com_Arc_GroupSignal_type *Arc_GroupSignal = GET_ArcGroupSignal(GroupSignal->ComHandleId);
                    /* Set pointer to shadow buffer*/
                    /*lint -e{9005} Com_Arc_Buffer can be modified via Arc_GroupSignal->Com_Arc_ShadowBuffer pointer*/
                    Arc_GroupSignal->Com_Arc_ShadowBuffer = ((void *)Arc_Signal->Com_Arc_ShadowBuffer);

                    /* Initialize shadowbuffer.*/
                    /* @req COM484 */
                    Com_Misc_UpdateShadowSignal(GroupSignal->ComHandleId, GroupSignal->ComSignalInitValue);
                }
                /* Initialize group signal data from shadowbuffer.*/
                /* @req COM098 */
                Com_Misc_CopySignalGroupDataFromShadowBufferToPdu(Signal->ComHandleId, false, &dataChanged);
            } else {
                /* Initialize signal data.*/
                /* @req COM098 */
                Com_Misc_WriteSignalDataToPdu(
                        Signal->ComSignalInitValue,
                        Signal->ComSignalType,
                        Arc_IPdu->ComIPduDataPtr,
                        Signal->ComBitPosition,
                        Signal->ComBitSize,
                        Signal->ComSignalEndianess,
                        &dataChanged);
            }
        }
        if( TRUE == pduHasGroupSignal ) {
            /* This pdu has includes group signals. Means that a shodow buffer was set up.
             * Increment index. */
            bufferIndex += IPdu->ComIPduSize;
        }
        if ((IPdu->ComIPduDirection == COM_RECEIVE) && (IPdu->ComIPduSignalProcessing == COM_DEFERRED)) {
            /* Set pointer to the deferred buffer */
            Arc_IPdu->ComIPduDeferredDataPtr = (void *)&Com_Arc_Buffer[bufferIndex];
            bufferIndex += IPdu->ComIPduSize;
            /* Copy the initialized pdu to deferred buffer*/
            memcpy(Arc_IPdu->ComIPduDeferredDataPtr,Arc_IPdu->ComIPduDataPtr,IPdu->ComIPduSize);
        }

#if (COM_SIG_GATEWAY_ENABLE == STD_ON)
        const ComGwDestnDesc_type * GwDestnPtr;
        /* Set the initial values for Destination description signals*/
        if((NULL != IPdu->ComIPduGwMapSigDescHandle) && (IPdu->ComIPduDirection== COM_SEND)){

            for (uint16 k=0; IPdu->ComIPduGwMapSigDescHandle[k] != INVALID_GWSIGNAL_DESCRIPTION_HANDLE; k++){

                GwDestnPtr = GET_GwDestnSigDesc(IPdu->ComIPduGwMapSigDescHandle[k]);
                Com_Misc_WriteSignalDataToPdu(
                        GwDestnPtr->ComSignalInitValue,
                        GwDestnPtr->ComSignalType,
                        Arc_IPdu->ComIPduDataPtr,
                        GwDestnPtr->ComBitPosition,
                        GwDestnPtr->ComBitSize,
                        GwDestnPtr->ComSignalEndianess,
                        &dataChanged);
            }

        }
#endif
    }
    for (uint16 i = 0; i < ComConfig->ComNofIPdus; i++) {
        Com_BufferPduState[i].currentPosition = 0;
        Com_BufferPduState[i].locked = false;
    }

#if(COM_IPDU_COUNTING_ENABLE ==  STD_ON )
    /* @req COM687 */
    ResetInternalCounterRxStartSts();
#endif

    /* An error occurred.*/
    if (TRUE == failure) {
        DEBUG(DEBUG_LOW, "--Initialization of COM failed--\n");

    } else {
        initStatus = COM_INIT;
        DEBUG(DEBUG_LOW, "--Initialization of COM completed--\n");
    }
}


void Com_DeInit( void ) {
    if(COM_INIT != initStatus) {
        DET_REPORTERROR(COM_DEINIT_ID, COM_E_UNINIT);
        /*lint -e{904} Return statement is necessary in case of reporting a DET error */
        return;
    }
    /* @req COM129 */
    for (uint16 i = 0; 0 == ComConfig->ComIPdu[i].Com_Arc_EOL; i++) {
        Com_Arc_Config.ComIPdu[i].Com_Arc_IpduStarted = FALSE;
    }
    initStatus = COM_UNINIT;
}

Com_StatusType Com_GetStatus( void )
{
    return initStatus;
}

/* Prerequisite com_init()function called */
void Com_IpduGroupControl(Com_IpduGroupVector ipduGroupVector, boolean Initialize)
{
    /* !req COM614 */
    /* Starting groups */
    /* @req COM114 */
    /* !req COM787 */
    /* !req COM222 */
    /* @req COM733 */
    /* !req COM740 */
    /* Stopping groups */
    /* !req COM479 */
    /* !req COM713 */
    /* !req COM714 */
    const ComIPdu_type   * IPdu;
    const ComSignal_type * comSignal;
    Com_Arc_Signal_type  * Arc_Signal;
    if(COM_INIT != initStatus) {
        DET_REPORTERROR(COM_IPDUGROUPCONTROL_ID, COM_E_UNINIT);
        /*lint -e{904} Return statement is necessary in case of reporting a DET error */
        return;
    }
    for (uint16 i = 0; 0 == ComConfig->ComIPdu[i].Com_Arc_EOL; i++) {
        const ComIPduGroup_type *const groupRefs = ComConfig->ComIPdu[i].ComIPduGroupRefs;
        boolean started = FALSE;
        for(uint32 gri=0; ((0 == groupRefs[gri].Com_Arc_EOL) && (FALSE == started)); gri++) {
            uint16 byteIndex;
            uint8 bitIndex;
            byteIndex = groupRefs[gri].ComIPduGroupHandleId / 8;
            bitIndex = groupRefs[gri].ComIPduGroupHandleId % 8;
            /* @req COM771 */
            started |= ((ipduGroupVector[byteIndex] >> bitIndex) & 1u);
        }
        if((Com_Arc_Config.ComIPdu[i].Com_Arc_IpduStarted != started)||(Initialize == TRUE)){
            /* @req COM612 */
            /* @req COM613 */
            /* @req COM615 */
            Com_Arc_Config.ComIPdu[i].Com_Arc_IpduStarted = started;

#if(COM_IPDU_COUNTING_ENABLE ==  STD_ON )
            /* @req COM687 */
            if ((TRUE == Initialize) && (TRUE == started) && (ComConfig->ComIPdu[i].ComIpduCounterRef != NULL) ) {
                /*Partial fulfillment of COM787 - Only action for IPDU counters are implemented*/
                uint16 idx = ComConfig->ComIPdu[i].ComIpduCounterRef->ComIPduCntrIndex;
                if (ComConfig->ComIPdu[i].ComIPduDirection == COM_SEND) {
                    ResetInternalCounter(idx);
                } else {
                    ReSetInternalRxStartSts(idx);
                }
            }
#endif
            /* @req COM115 */ /* cancel deadline monitoring if stopped*/
            /* !req COM787  party satisfied for DM,If an I-PDU is started as result of a call Com_IpduGroupControl
             2) timeout attributes of I-PDUs for deadline monitoring aspect: all timeout timers (ComFirstTimeout, ComTimeout) shall restart */
            IPdu = GET_IPdu(i);
            SchM_Enter_Com_EA_0();  /* DM space */
            /* RX PDUS */
            if (IPdu->ComIPduDirection == COM_RECEIVE) {
               if(TRUE == started)		   {
                   if(TRUE == Com_Arc_Config.ComIPdu[i].Com_Arc_IpduRxDMControl) {/* if running */
                    /*   timeout attributes of I-PDUs for deadline monitoring aspect:
                     *   all timeout timers (ComFirstTimeout, ComTimeout) shall restart */
                       for (uint16 j = 0; IPdu->ComIPduSignalRef[j] != NULL; j++) {
                           comSignal = IPdu->ComIPduSignalRef[j];
                           Arc_Signal = GET_ArcSignal(comSignal->ComHandleId);

                           if(0 < comSignal->ComTimeoutFactor) {
                               if(0 < comSignal->ComFirstTimeoutFactor) {
                                 Arc_Signal->Com_Arc_DeadlineCounter = comSignal->ComFirstTimeoutFactor;
                               } else {
                                 Arc_Signal->Com_Arc_DeadlineCounter = 0;
                               }
                           }
                       }
                   }
               } else {
                   /* disable rx monitoring */
                   /*  @req COM685 */
                   /*  @req COM115 */ /* cancel pending confirmations and it does automatically when Com_Arc_IpduRxDMControl is 0 */
                   /*  rx DM timers are re initialised when it is enabled back */
                   Com_Arc_Config.ComIPdu[i].Com_Arc_IpduRxDMControl = FALSE;
               }
            } else {/* TX PDUS */
                const ComTxMode_type *txModePtr;
                if( TRUE == Com_Arc_Config.ComIPdu[i].Com_Arc_IpduTxMode ) {
                    txModePtr = &IPdu->ComTxIPdu.ComTxModeTrue;
                }
                else {
                    txModePtr = &IPdu->ComTxIPdu.ComTxModeFalse;
                }
                if((TRUE == started) &&  (0 < Com_Arc_Config.ComIPdu[i].Com_Arc_TxDeadlineCounter)) {
                    /* can pdus be started two consecutive times without state change? */
                    Com_Arc_Config.ComIPdu[i].Com_Arc_TxIPduTimers.ComTxDMTimer = 0;
                    /*  @req COM696 - redundant? */
                    if (txModePtr->ComTxModeMode == COM_NONE) {
                        Com_Arc_Config.ComIPdu[i].Com_Arc_TxIPduTimers.ComTxDMTimer = Com_Arc_Config.ComIPdu[i].Com_Arc_TxDeadlineCounter;
                    }
                } else {
                    /* fall back in any case here */
                    /*  @req COM685 */
                    /*  @req COM115 */
                    /* cancel pending confirmations and ComTxDMTimer */
                    Com_Arc_Config.ComIPdu[i].Com_Arc_TxIPduTimers.ComTxDMTimer = 0;
                    Com_Arc_Config.ComIPdu[i].Com_Arc_TxIPduTimers.ComTxIPduNumberOfTxConfirmations = 0;
                }

                if ((TRUE == Initialize) && (TRUE == started)) {
                    /* Only  parts of COM622 and COM787 are supported */
                    Com_Arc_Config.ComIPdu[i].Com_Arc_TxIPduTimers.ComTxModeRepetitionPeriodTimer = 0;
                    Com_Arc_Config.ComIPdu[i].Com_Arc_TxIPduTimers.ComTxIPduNumberOfRepetitionsLeft = 0;

                    if ((txModePtr->ComTxModeMode == COM_PERIODIC) ||
                        (txModePtr->ComTxModeMode == COM_MIXED)) {
                        Com_Arc_Config.ComIPdu[i].Com_Arc_TxIPduTimers.ComTxModeTimePeriodTimer =
                                txModePtr->ComTxModeTimeOffsetFactor + 1;
                    }

                }
            }
            SchM_Exit_Com_EA_0(); /* DM space */
        }
    }
}

void Com_ClearIpduGroupVector(Com_IpduGroupVector ipduGroupVector)
{
    if(COM_INIT != initStatus) {
        DET_REPORTERROR(COM_CLEARIPDUGROUPVECTOR_ID, COM_E_UNINIT);
        /*lint -e{904} Return statement is necessary in case of reporting a DET error */
        return;
    }
    memset(ipduGroupVector, 0, sizeof(Com_IpduGroupVector));
}

void Com_SetIpduGroup(Com_IpduGroupVector ipduGroupVector, Com_IpduGroupIdType ipduGroupId, boolean bitval)
{
    if(COM_INIT != initStatus) {
        DET_REPORTERROR(COM_SETIPDUGROUP_ID, COM_E_UNINIT);
        /*lint -e{904} Return statement is necessary in case of reporting a DET error */
        return;
    }
    uint16 byteIndex;
    uint8 bitIndex;
    byteIndex = ipduGroupId / 8;
    bitIndex = ipduGroupId % 8;
    /* @req COM623 */
    if( TRUE == bitval ) {
        /*lint -e{734} bit-index is limited to value 7,result will not loose precision  */
        ipduGroupVector[byteIndex] |= (1u<<bitIndex);
    } else {
        ipduGroupVector[byteIndex] &= ~(1u<<bitIndex);
    }
}
/**
 *
 * @param PduId
 * @param PduInfoPtr
 * @param RetryInfoPtr not supported
 * @param TxDataCntPtr
 * @return
 */
/*lint -e{818} PduInfoPtr is declared as pointing to const in version 4.2.2*/
BufReq_ReturnType Com_CopyTxData(PduIdType PduId, PduInfoType* PduInfoPtr, RetryInfoType* RetryInfoPtr, PduLengthType* TxDataCntPtr) {
    /* IMPROVEMENT: Validate PduId, etc? */
    /* !req COM663*/
    /* !req COM783*/ /* Do not copy any data and return BUFREQ_E_NOT_OK if pdu group stopped */
    BufReq_ReturnType r = BUFREQ_OK;

    if(COM_INIT != initStatus) {
        DET_REPORTERROR(COM_COPYTXDATA_ID, COM_E_UNINIT);
        /*lint -e{904} Return statement is necessary in case of reporting a DET error */
        return BUFREQ_NOT_OK;
    }

    const ComIPdu_type *IPdu = GET_IPdu(PduId);
    boolean dirOk = ComConfig->ComIPdu[PduId].ComIPduDirection == COM_SEND;
    boolean sizeOk;
    //lint -estring(920,pointer)  /* cast to void */
    (void)RetryInfoPtr; /* get rid of compiler warning*/
    //lint +estring(920,pointer)  /* cast to void */

    SchM_Enter_Com_EA_0();
    sizeOk = (IPdu->ComIPduSize >= (Com_BufferPduState[PduId].currentPosition + PduInfoPtr->SduLength))? TRUE : FALSE;
    Com_BufferPduState[PduId].locked = TRUE;
    if ((TRUE == dirOk) && (TRUE == sizeOk)) {
        const uint8* source = (uint8 *)GET_ArcIPdu(PduId)->ComIPduDataPtr;
        memcpy(PduInfoPtr->SduDataPtr,&(source[Com_BufferPduState[PduId].currentPosition]), PduInfoPtr->SduLength);
        Com_BufferPduState[PduId].currentPosition += PduInfoPtr->SduLength;
        *TxDataCntPtr = IPdu->ComIPduSize - Com_BufferPduState[PduId].currentPosition;
    } else {
        r = BUFREQ_NOT_OK;
    }
    SchM_Exit_Com_EA_0();
    return r;
}
BufReq_ReturnType Com_CopyRxData(PduIdType PduId, const PduInfoType* PduInfoPtr, PduLengthType* RxBufferSizePtr) {
    /* !req COM782 */ /* If pdu group stopped -> return BUFREQ_E_NOT_OK */
    BufReq_ReturnType r = BUFREQ_OK;
    uint16 remainingBytes;
    boolean sizeOk;
    boolean dirOk;
    boolean lockOk;

    if(COM_INIT != initStatus) {
        DET_REPORTERROR(COM_COPYRXDATA_ID, COM_E_UNINIT);
        /*lint -e{904} Return statement is necessary in case of reporting a DET error */
        return BUFREQ_NOT_OK;
    }
    SchM_Enter_Com_EA_0();
    /* @req COM658 */ /* Interrupts disabled */

    LDEBUG_PRINTF("   Com_CopyRxData: PduId=%d, Size=%d, Pos=%d, sduLength=%d\n",PduId, GET_IPdu(PduId)->ComIPduSize,Com_BufferPduState[PduId].currentPosition, PduInfoPtr->SduLength);
    LDEBUG_PRINTF("   data[0]=%x [1]=%x...\n",PduInfoPtr->SduDataPtr[0],PduInfoPtr->SduDataPtr[1]);

    remainingBytes = ((GET_IPdu(PduId)->ComIPduSize) - (Com_BufferPduState[PduId].currentPosition));
    sizeOk = remainingBytes >= PduInfoPtr->SduLength;
    dirOk = GET_IPdu(PduId)->ComIPduDirection == COM_RECEIVE;
    lockOk = isPduBufferLocked(PduId);
    if ((TRUE == dirOk) && (TRUE == lockOk) && (TRUE == sizeOk)) {
        uint8* ComIPduDataPtr1 = (uint8 *)GET_ArcIPdu(PduId)->ComIPduDataPtr;
        memcpy((void *)(&ComIPduDataPtr1[Com_BufferPduState[PduId].currentPosition]), PduInfoPtr->SduDataPtr, PduInfoPtr->SduLength);
        Com_BufferPduState[PduId].currentPosition += PduInfoPtr->SduLength;
        *RxBufferSizePtr = GET_IPdu(PduId)->ComIPduSize - Com_BufferPduState[PduId].currentPosition;
    } else {
        r = BUFREQ_NOT_OK;
    }
    SchM_Exit_Com_EA_0();
    return r;
}

static void Com_SetDynSignalLength(PduIdType ComRxPduId,PduLengthType TpSduLength) {
    const ComIPdu_type *IPdu = GET_IPdu(ComRxPduId);
    if (IPdu->ComIPduDynSignalRef != 0) {
        const ComSignal_type * const dynSignal = IPdu->ComIPduDynSignalRef;
        Com_Arc_IPdu_type *Arc_IPdu = GET_ArcIPdu(ComRxPduId);
        /* @req COM758 */
        if( TpSduLength > (dynSignal->ComBitPosition/8) ) {
            Arc_IPdu->Com_Arc_DynSignalLength = TpSduLength - (dynSignal->ComBitPosition/8);
        } else {
            Arc_IPdu->Com_Arc_DynSignalLength = 0;
        }
    }
    return;
}

BufReq_ReturnType Com_StartOfReception(PduIdType ComRxPduId, PduLengthType TpSduLength, PduLengthType* RxBufferSizePtr) {
    /* IMPROVEMENT: Validate ComRxPduId? */
    PduLengthType ComIPduSize;
    BufReq_ReturnType r = BUFREQ_OK;

    if(COM_INIT != initStatus) {
        DET_REPORTERROR(COM_STARTOFRECEPTION_ID, COM_E_UNINIT);
        /*lint -e{904} Return statement is necessary in case of reporting a DET error */
        return BUFREQ_NOT_OK;
    }

    const Com_Arc_IPdu_type *Arc_IPdu = GET_ArcIPdu(ComRxPduId);

    SchM_Enter_Com_EA_0();
    if (TRUE == Arc_IPdu->Com_Arc_IpduStarted) {
        if (GET_IPdu(ComRxPduId)->ComIPduDirection == COM_RECEIVE) {
            /* !req COM657 */
            if (FALSE == Com_BufferPduState[ComRxPduId].locked) {
                ComIPduSize = GET_IPdu(ComRxPduId)->ComIPduSize;
                if (ComIPduSize >= TpSduLength) {
                    Com_BufferPduState[ComRxPduId].locked = true;
                    *RxBufferSizePtr = ComIPduSize;
                    /* @req COM656 */
                    Com_SetDynSignalLength(ComRxPduId,TpSduLength);
                } else {
                    /* @req COM654 */
                    /* @req COM655 */
                    r = BUFREQ_OVFL;
                }
            } else {
                r = BUFREQ_BUSY;
            }
        }
    } else {
        /* @req COM721 */
        r = BUFREQ_NOT_OK;
    }
    SchM_Exit_Com_EA_0();
    return r;
}


/*  @req COM752 */
/**
 * This service enables or disables RX I-PDU group Deadline Monitoring
 * @param ipduGroupVector
 * @return none
 */
void Com_ReceptionDMControl(Com_IpduGroupVector ipduGroupVector)
{
    if(COM_INIT != initStatus) {
        DET_REPORTERROR(COM_RECEPTIONDMCONTROL_ID, COM_E_UNINIT);
        /*lint -e{904} Return statement is necessary in case of reporting a DET error */
        return;
    }

    for (uint16 i = 0; 0 == ComConfig->ComIPdu[i].Com_Arc_EOL; i++)
    {
        const ComIPduGroup_type *const groupRefs = ComConfig->ComIPdu[i].ComIPduGroupRefs;

        boolean enabled = FALSE;
        /* @req COM534 */
        if(ComConfig->ComIPdu[i].ComIPduDirection == COM_RECEIVE) {/* check for only rx pdus */
            for(uint16 gri=0; ((0 == groupRefs[gri].Com_Arc_EOL) && (FALSE == enabled)); gri++) {
                uint16 byteIndex;
                uint8 bitIndex;
                byteIndex = groupRefs[gri].ComIPduGroupHandleId / 8;
                bitIndex = groupRefs[gri].ComIPduGroupHandleId % 8;
                enabled |= ((ipduGroupVector[byteIndex] >> bitIndex) & 1u);
            }
            /* @req COM486 */
            /* @req COM225 */
            /* @req COM616 */
            /* @req COM618 */
            if ( Com_Arc_Config.ComIPdu[i].Com_Arc_IpduRxDMControl != enabled) {/* change of state check space*/
                const ComSignal_type *comSignal;
                const ComIPdu_type *IPdu = GET_IPdu(i);
                Com_Arc_Signal_type * Arc_Signal;
                SchM_Enter_Com_EA_0();
                Com_Arc_Config.ComIPdu[i].Com_Arc_IpduRxDMControl = enabled;
                /* @req COM224 */
                if(TRUE == enabled)				{
                    for (uint16 j = 0; IPdu->ComIPduSignalRef[j] != NULL; j++) {
                        comSignal = IPdu->ComIPduSignalRef[j];
                        Arc_Signal = GET_ArcSignal(comSignal->ComHandleId);
                        if(comSignal->ComTimeoutFactor > 0) {
                            /* Reset the deadline monitoring timer */
                            /* @req COM292 */  /* taken care in the generator  */
                            /* @req COM291 */  /* taken care in the generator  */
                            if(0 < comSignal->ComFirstTimeoutFactor) {
                               Arc_Signal->Com_Arc_DeadlineCounter = comSignal->ComFirstTimeoutFactor;
                            }
                            else{ /* wait for first reception to occur, since first time factor is not configured */
                                Arc_Signal->Com_Arc_DeadlineCounter = 0;
                            }
                        }
                    }
                }
                SchM_Exit_Com_EA_0();
            } /* change of state check space*/
        }
    }
}

/**
 * The service Com_SwitchIpduTxMode sets the transmission mode of the I-PDU referenced
 * by PduId to Mode. In case the transmission mode changes, the new mode shall immediately
 * be effective (see COM239). In case the requested transmission mode was already active
 * for this I-PDU, the call will have no effect.
 * @param PduId
 * @param Mode
 */
/* @req COM784 */
void Com_SwitchIpduTxMode(PduIdType PduId, boolean Mode)
{
    /* @req COM238 */
    /* @req COM239 */
    /* !req COM582 */
    const ComIPdu_type *IPdu;
    Com_Arc_IPdu_type *Arc_IPdu;
    if(COM_INIT != initStatus) {
        DET_REPORTERROR(COM_SWITCHIPDUTXMODE_ID, COM_E_UNINIT);
        /*lint -e{904} Return statement is necessary in case of reporting a DET error */
        return;
    }
    if(PduId >= ComConfig->ComNofIPdus) {
        DET_REPORTERROR(COM_SWITCHIPDUTXMODE_ID, COM_E_PARAM);
        /*lint -e{904} ARGUMENT CHECK */
        return;
    }
    IPdu = GET_IPdu(PduId);
    if( COM_SEND != IPdu->ComIPduDirection ) {
        DET_REPORTERROR(COM_SWITCHIPDUTXMODE_ID, COM_E_PARAM);
        /*lint -e{904} Return statement is necessary in case of reporting a DET error */
        return;
    }
    Arc_IPdu = GET_ArcIPdu(PduId);
    if( Arc_IPdu->Com_Arc_IpduTxMode != Mode ) {
        /* Switching mode */
        const ComTxMode_type *txModePtr;
        if( TRUE == Mode) {
            txModePtr = &IPdu->ComTxIPdu.ComTxModeTrue;
        }
        else {
            txModePtr = &IPdu->ComTxIPdu.ComTxModeFalse;
        }
        Arc_IPdu->Com_Arc_IpduTxMode = Mode;/* @req COM032 */
        Arc_IPdu->Com_Arc_TxIPduTimers.ComTxModeRepetitionPeriodTimer = 0;
        Arc_IPdu->Com_Arc_TxIPduTimers.ComTxIPduNumberOfRepetitionsLeft = 0;

        if ((txModePtr->ComTxModeMode == COM_PERIODIC) ||
            (txModePtr->ComTxModeMode == COM_MIXED)) {
            /* @req COM244 */
            Arc_IPdu->Com_Arc_TxIPduTimers.ComTxModeTimePeriodTimer = txModePtr->ComTxModeTimePeriodFactor;
        }
    }
}

