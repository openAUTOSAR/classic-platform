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

/*----------------------------[information]----------------------------------
 * Author: mahi/hebe
 *
 * Part of Release:
 *   4.0.3
 *
 * Description:
 *   Implements the Can Interface
 *
 * Support:
 *   Private                  Have Support
 *   ---------------------------------------------------------------
 *   CANIF_[PRIVATE_]DLC_CHECK 					Y		4.0.3
 *   CANIF_[PRIVATE_]SOFTWARE_FILTER_TYPE 		N		4.0.3
 *   CANIF_NUMBER_OF_TXBUFFERS  				N		3.1.5
 *   CANIF_PRIVATE_SUPPORT_TTCAN				N       4.0.3
 *
 *
 *   Public                  Have Support
 *   ----------------------------------------------------------------
 *   CANIF_[PUBLIC_]DEV_ERROR_DETECT			Y		4.0.3
 *   CANIF_[PUBLIC_]MULTIPLE_DRIVER_SUPPORT		N		4.0.3
 *   CANIF_[PUBLIC_]NUMBER_OF_CAN_HW_UNITS		N		3.1.5
 *   CANIF_PUBLIC_TXCONFIRM_POLLING_SUPPORT		N		4.0.3
 *   CANIF_[PUBLIC_]READRXPDU_DATA_API			N		4.0.3
 *   CANIF_[PUBLIC_]READRXPDU_NOTIF_STATUS_API	N		4.0.3
 *   CANIF_[PUBLIC_]READTXPDU_NOTIF_STATUS_API	N		4.0.3
 *   CANIF_[PUBLIC_]SETDYNAMICTXID_API			N		4.0.3
 *   CANIF_[PUBLIC_]VERSION_INFO_API			Y		4.0.3
 *
 *   CANIF_PUBLIC_CANCEL_TRANSMIT_SUPPORT		N		4.0.3
 *   CANIF_PUBLIC_CDD_HEADERFILE				N		4.0.3
 *   CANIF_PUBLIC_HANDLE_TYPE_ENUM				N 		4.0.3
 *   CANIF_PUBLIC_TX_BUFFERING					Y 		4.0.3
 *   CANIF_PUBLIC_WAKEUP_CHECK_VALIDATION_API   Y 		4.0.3
 *   CANIF_PUBLIC_WAKEUP_CHECK_VALID_BY_NM	    N 		4.0.3
 *
 * Implementation Notes:
 *   Channels:
 *      The specification speaks a lot of channels and it's sometimes confusing
 *      This implementations interpretation is:
 *      - Physical Channel
 *        One physical channel is connected to one CAN controller and one CAN
 *        transceiver. One or more physical channels may be connected to a single
 *        network (the physical CAN network)
 *
 *                physical channel      physical channel
 *                       |                     |
 *                     CAN_B                  CAN_D
 *                       |                     |
 *                       +       NETWORK       +
 *
 *
 *       Since the CAN controller Id is logical controller Id (e.g. id=0 is CAN_C) channel
 *       is equal to controllerId in the implementation.
 *
 *    Drivers:
 *      There is only support for one driver and that is the on-chip CAN.
 *
 *    Configuration:
 *      3.1.5 CANIF_NUMBER_OF_TXBUFFERS is defined to be the same as for 4.x, ie
 *      only have 1 buffer for each L-PDU. In 3.1.5 this was setable.
 *
 *
 *
 */
/* General requirements */
/* @req 4.0.3/CANIF023 *//* CanIf shall avoid direct access to hardware buffers and shall access it exclusively via the CanDrv interface services */
/* @req 4.0.3/CANIF064 *//* Interrupts are disabled when buffers accessed */
/* !req 4.0.3/CANIF142 *//* Not all types imported */
/* ----------------------------[includes]------------------------------------*/
#include "Std_Types.h"
#include "CanIf.h"
#include "CanIf_ConfigTypes.h"
#include <string.h>
#include "arc_assert.h"
#include "SchM_CanIf.h"
#if  ( CANIF_PUBLIC_DEV_ERROR_DETECT == STD_ON )
#include "Det.h"
#endif
#if defined(USE_DEM)
/* @req 4.0.3/CANIF150 */
#include "Dem.h"
#endif
#if (CANIF_OSEKNM_SUPPORT == STD_ON)
#include "OsekNm.h"
#endif

/* ----------------------------[Version check]------------------------------*/
#if !(((CANIF_SW_MAJOR_VERSION == 5) && (CANIF_SW_MINOR_VERSION == 3)) )
#error CanIf: Expected BSW module version to be 5.3.*
#endif

/* @req 4.0.3/CANIF021 */
#if !(((CANIF_AR_RELEASE_MAJOR_VERSION == 4) && (CANIF_AR_RELEASE_MINOR_VERSION == 0)) )
#error CanIf: Expected AUTOSAR version to be 4.0.*
#endif
/* ----------------------------[Config check]------------------------------*/
#if (CANIF_CTRLDRV_TX_CANCELLATION == STD_ON) && (CANIF_PUBLIC_TX_BUFFERING == STD_OFF)
#error CanIf: Tx cancellation cannot be enabled without enabling tx buffering
#endif
/* ----------------------------[private define]------------------------------*/
#define EXTENDED_CANID_MAX 0x1FFFFFFF
#define STANDARD_CANID_MAX 0x7FF
#define EXT_ID_BIT_POS 31
#define CAN_FD_BIT_POS 30 /* @req 4.3.0/CAN416 */
#define EXT_ID_STD_ID_START_BIT 18
#define INVALID_CANID 0xFFFFFFFF

#define FEATURE_NOT_SUPPORTED 0
/* ----------------------------[private macro]-------------------------------*/
#if  ( CANIF_PUBLIC_DEV_ERROR_DETECT == STD_ON )
/* @req 4.0.3/CANIF018 */
/* @req 4.0.3/CANIF019 */
/* @req 4.0.3/CANIF156 */
#define DET_REPORT_ERROR(_api, _error)                   \
    do {                                                  \
        (void)Det_ReportError(CANIF_MODULE_ID, 0, _api, _error); \
    } while(0)


#else
#define DET_REPORT_ERROR(_api,_err)
#endif

#define IS_EXTENDED_CAN_ID(_x) (0 != (_x & (1ul<<EXT_ID_BIT_POS)))
#define IS_CANFD(_x) (0 != (_x & (1ul<<CAN_FD_BIT_POS)))

#define VALIDATE_NO_RV(_exp,_api,_err ) \
  if( !(_exp) ) { \
      DET_REPORT_ERROR(_api, _err); \
      return; \
  }

#define VALIDATE_RV(_exp,_api,_err,_ret ) \
  if( !(_exp) ) { \
      DET_REPORT_ERROR(_api, _err); \
      return (_ret); \
  }

#define IS_VALID_CONTROLLER_MODE(_x) \
        ((CANIF_CS_STARTED == (_x)) || \
        (CANIF_CS_SLEEP == (_x)) || \
        (CANIF_CS_STOPPED == (_x)))

// Helper to get the Can Controller refered to by a CanIf Channel
/* IMPROVMENT: Could we handle this in another way?
 *  */
#define ARC_GET_CHANNEL_CONTROLLER(_channel) \
    CanIf_ConfigPtr->Arc_ChannelConfig[_channel].CanControllerId

/* Map from CanIfTransceiverId to CanTransceiverId */
#define ARC_GET_CAN_TRANSCEIVERID(_id) \
    CanIf_ConfigPtr->CanIfTransceiverConfig[_id].CanIfTrcvCanTrcvIdRef
/* ----------------------------[private typedef]-----------------------------*/

typedef void (*RxIndicationCbType)(PduIdType , const PduInfoType*);


 typedef struct
 {
   CanIf_ControllerModeType  ControllerMode;
   CanIf_PduGetModeType  PduMode;
#if (CANIF_PUBLIC_PN_SUPPORT == STD_ON)
   /* @req 4.0.3/CANIF747 */
   boolean 					 pnTxFilterEnabled;
#endif
#if (CANIF_PUBLIC_WAKEUP_CHECK_VALIDATION_SUPPORT == STD_ON)
boolean firstRxIndication;
#endif
 } CanIf_ChannelPrivateType;

#if (CANIF_ARC_TRANSCEIVER_API == STD_ON)
typedef struct
{
    CanTrcv_TrcvModeType trcvMode;
} CanIf_TrcvPrivateType;
#endif

typedef struct
{
  boolean initRun;
  CanIf_ChannelPrivateType channelData[CANIF_CHANNEL_CNT];
#if (CANIF_ARC_TRANSCEIVER_API == STD_ON)
  CanIf_TrcvPrivateType trcvData[CANIF_TRANSCEIVER_CHANNEL_CNT];
#endif
} CanIf_GlobalType;

typedef struct  {
    PduIdType pduId;
    Can_IdType canId;
#if (CANIF_CANFD_SUPPORT == STD_ON)
    uint8	  data[64];// For flexible datarate data
#else
    uint8	  data[8];
#endif
    uint8 	  dlc;
} CanIf_LPduType;

#if (CANIF_PUBLIC_TX_BUFFERING == STD_ON)
typedef struct {
    /**/
    CanIf_LPduType lPdu;
    boolean inUse;
}CanIf_Arc_BufferEntryType;

#define CANIF_START_SEC_VAR_CLEARED_UNSPECIFIED
#include "CanIf_MemMap.h"
static CanIf_Arc_BufferEntryType TxPduBuffer[CANIF_ARC_MAX_NUM_LPDU_TX_BUF];
#define CANIF_STOP_SEC_VAR_CLEARED_UNSPECIFIED
#include "CanIf_MemMap.h"  /*lint !e9019 OTHER [MISRA 2012 Rule 20.1, advisory] OTHER AUTOSAR specified way of using MemMap*/
#endif

/* ----------------------------[private function prototypes]-----------------*/
/* ----------------------------[private variables]---------------------------*/
#if (CANIF_PUBLIC_TX_BUFFERING == STD_ON)
/* Mapping from buffer index to start index in the L-PDU buffer */
#define CANIF_START_SEC_VAR_CLEARED_UNSPECIFIED
#include "CanIf_MemMap.h"
static uint16 BufferStartIndex[CANIF_ARC_MAX_NOF_TX_BUFFERS];
#define CANIF_STOP_SEC_VAR_CLEARED_UNSPECIFIED
#include "CanIf_MemMap.h"  /*lint !e9019 OTHER [MISRA 2012 Rule 20.1, advisory] OTHER AUTOSAR specified way of using MemMap*/
#endif

#define CANIF_START_SEC_VAR_CLEARED_UNSPECIFIED
#include "CanIf_MemMap.h"
CanIf_GlobalType CanIf_Global;
#define CANIF_STOP_SEC_VAR_CLEARED_UNSPECIFIED
#include "CanIf_MemMap.h"  /*lint !e9019 OTHER [MISRA 2012 Rule 20.1, advisory] OTHER AUTOSAR specified way of using MemMap*/

/* Global configure */
#define CANIF_START_SEC_VAR_CLEARED_UNSPECIFIED
#include "CanIf_MemMap.h"
static const CanIf_ConfigType *CanIf_ConfigPtr;
#define CANIF_STOP_SEC_VAR_CLEARED_UNSPECIFIED
#include "CanIf_MemMap.h"  /*lint !e9019 OTHER [MISRA 2012 Rule 20.1, advisory] OTHER AUTOSAR specified way of using MemMap*/

#if ( CANIF_PUBLIC_READRXPDU_DATA_API == STD_ON ) && FEATURE_NOT_SUPPORTED
static CanIf_LPduType RxBuffer[CANIF_NUM_RX_LPDU];
#endif
#if ( CANIF_PUBLIC_READTXPDU_NOTIFY_STATUS_API == STD_ON ) && FEATURE_NOT_SUPPORTED
/* !req 4.0.3/CANIF472 */
#define CANIF_START_SEC_VAR_CLEARED_UNSPECIFIED
#include "CanIf_MemMap.h"
CanIf_NotifStatusType TxNotifStatusBuffer[CANIF_NUM_RX_LPDU];
#define CANIF_STOP_SEC_VAR_INIT_UNSPECIFIED
#include "CanIf_MemMap.h"  /*lint !e9019 OTHER [MISRA 2012 Rule 20.1, advisory] OTHER AUTOSAR specified way of using MemMap*/
#endif

#if ( CANIF_PUBLIC_READRXPDU_NOTIFY_STATUS_API == STD_ON ) && FEATURE_NOT_SUPPORTED
/* !req 4.0.3/CANIF473 */
#define CANIF_START_SEC_VAR_CLEARED_UNSPECIFIED
#include "CanIf_MemMap.h"
CanIf_NotifStatusType RxNotifStatusBuffer[CANIF_NUM_TX_LPDU];
#define CANIF_STOP_SEC_VAR_INIT_UNSPECIFIED
#include "CanIf_MemMap.h"  /*lint !e9019 OTHER [MISRA 2012 Rule 20.1, advisory] OTHER AUTOSAR specified way of using MemMap*/

#endif


/* Declared in Can_Cfg.c */
extern const CanIfUserRxIndicationType CanIfUserRxIndications[];
extern const CanIfUserTxConfirmationType CanIfUserTxConfirmations[];
extern const CanIf_DispatchConfigType CanIfDispatchConfig;


/* ----------------------------[private functions]---------------------------*/
#if (CANIF_OSEKNM_SUPPORT == STD_ON)
void canIfOsekNmRxIndication(CanIf_Arc_ChannelIdType canifChnlId,Can_IdType canId, PduInfoType* pduInfo);
void canIfOsekNmTxConfirmation(CanIf_Arc_ChannelIdType canifChnlId,Can_IdType canId);
/**
 * @brief Indicate OsekNm about reception of the frame
 * @param canifChnlId - CanIf channel
 * @param canId - Received CAN frame Identifier
 * @param pduInfo - CAN payload
 */
void canIfOsekNmRxIndication(CanIf_Arc_ChannelIdType canifChnlId,Can_IdType canId, PduInfoType* pduInfo){
    uint8 netId;
    OsekNm_PduType nmData;

    nmData.source = (uint8)(canId & (Can_IdType) CanIf_ConfigPtr->Arc_ChannelConfig[canifChnlId].CanIfOsekNmNodeIdMask);
    nmData.destination = pduInfo->SduDataPtr[0];
    nmData.OpCode.b = pduInfo->SduDataPtr[1];
    memcpy(&(nmData.ringData),&pduInfo->SduDataPtr[2],(pduInfo->SduLength -2));
    netId = CanIf_ConfigPtr->Arc_ChannelConfig[canifChnlId].CanIfOsekNmNetId;
    if (netId != CANIF_OSEKNM_INVALID_NET_ID) {
        OsekNm_RxIndication(netId,nmData.source,&nmData);
    }
}

/**
 * @brief Indicate OsekNm about transmit confirmation
 * @param canifChnlId
 * @param canId
 */
void canIfOsekNmTxConfirmation(CanIf_Arc_ChannelIdType canifChnlId,Can_IdType canId) {
    uint8 netId;
    uint8 srcNodeId;

    netId = CanIf_ConfigPtr->Arc_ChannelConfig[canifChnlId].CanIfOsekNmNetId;
    srcNodeId = (uint8)(canId & (Can_IdType)CanIf_ConfigPtr->Arc_ChannelConfig[canifChnlId].CanIfOsekNmNodeIdMask);
    if (netId != CANIF_OSEKNM_INVALID_NET_ID) {
        OsekNm_TxConfirmation(netId,srcNodeId);
    }
}
#endif
/* ----------------------------[public functions]----------------------------*/

/* IMPROVMENT: Generator changes
 *
 * - CanIfUserRxIndication should be set according to it's type, e.g
 *    CanNm_RxIndication, PduR_CanIfRxIndication, CanTp_RxIndication, J1939Tp_RxIndication
 *    What to do with "special"?
 * -  The number of PDU's should be generated to CANIF_NUM_TX_LPDU
 *
 *
 *
 * */


#if (CANIF_PUBLIC_TX_BUFFERING == STD_ON)
static Std_ReturnType qReplaceOrAdd(const CanIf_TxBufferConfigType *bufferPtr, Can_PduType *canPduPtr, boolean replaceAllowed)
{
    /* !req CANIF033 */

    boolean found = FALSE;
    Std_ReturnType ret = E_NOT_OK;
    CanIf_Arc_BufferEntryType *buffPtr = NULL;
    uint16 startIndex;
    SchM_Enter_CanIf_EA_0();
    startIndex = BufferStartIndex[bufferPtr->CanIf_Arc_BufferId];
    for( uint16 i = startIndex; (i < (startIndex + bufferPtr->CanIfBufferSize)) && (!found); i++) {

        if( TxPduBuffer[i].inUse && (TxPduBuffer[i].lPdu.pduId == canPduPtr->swPduHandle) ) {
            /* This pdu was already stored. Replace if allowed. */
            /* @req CANIF068 */
            if( replaceAllowed ) {
                buffPtr = &TxPduBuffer[i];
            } else {
                buffPtr = NULL;
            }
            found = TRUE;
        } else if( !TxPduBuffer[i].inUse && (NULL == buffPtr)) {
            /* Not used. */
            /* @req CANIF836 */
            buffPtr = &TxPduBuffer[i];
        }
    }

    if( NULL != buffPtr ) {
        /* Found an entry to use */
        buffPtr->inUse = TRUE;
        buffPtr->lPdu.dlc = canPduPtr->length;
        buffPtr->lPdu.canId = canPduPtr->id;
        buffPtr->lPdu.pduId = canPduPtr->swPduHandle;
        memcpy(buffPtr->lPdu.data, canPduPtr->sdu, canPduPtr->length);
        ret = E_OK;
    } else {
        /* @req CANIF837 */
    }

    SchM_Exit_CanIf_EA_0();
    return ret;
}

static void qRemove(const CanIf_TxBufferConfigType *bufferPtr, Can_IdType canId) {
    /* Remove entry matching canId from buffer corresponding to bufferPtr */
    boolean found = FALSE;
    uint16 startIndex;
    SchM_Enter_CanIf_EA_0();
    startIndex = BufferStartIndex[bufferPtr->CanIf_Arc_BufferId];
    for( uint16 i = startIndex; (i < (startIndex + bufferPtr->CanIfBufferSize)) && (!found); i++) {
        if( TxPduBuffer[i].inUse && (TxPduBuffer[i].lPdu.canId == canId)) {
            TxPduBuffer[i].inUse = FALSE;
            found = TRUE;
        }
    }
    SchM_Exit_CanIf_EA_0();
}

static Std_ReturnType qGetBufferedPdu(const CanIf_TxBufferConfigType *bufferPtr, Can_PduType *canPduPtr)
{
    /* Search through the buffer and find the highest prioritized entry */
    Std_ReturnType ret;
    ret = E_NOT_OK;
    boolean found = FALSE;
    uint16 startIndex;
    Can_IdType highestPrioCanId = INVALID_CANID;
    boolean highestPrioIsExt = TRUE;
    Can_IdType canId = 0;
    SchM_Enter_CanIf_EA_0();
    startIndex = BufferStartIndex[bufferPtr->CanIf_Arc_BufferId];
    for( uint16 i = startIndex; i < (startIndex + bufferPtr->CanIfBufferSize); i++) {
        if( TxPduBuffer[i].inUse) {
            canId = TxPduBuffer[i].lPdu.canId;
            if( (canId & (1ul<<EXT_ID_BIT_POS)) != (highestPrioCanId & (1ul<<EXT_ID_BIT_POS))) {
                /* One id is extended and the other standard */
                if( highestPrioIsExt ) {
                    /* The currently highest prioritized canid is extended. Modify the
                     * standard id to a corresponding extended */
                    canId = ((canId<<EXT_ID_STD_ID_START_BIT) & EXTENDED_CANID_MAX) | (1ul<<EXT_ID_BIT_POS);
                } else {
                    /* The currently highest prioritized canid is standard. Modify the
                     * extended id to a corresponding standard */
                    canId = (canId>>EXT_ID_STD_ID_START_BIT) & STANDARD_CANID_MAX;
                }
            }
            if( (highestPrioCanId > canId) || (highestPrioIsExt && (highestPrioCanId == canId)) ) {
                /* Priority higher. */
                canPduPtr->id = TxPduBuffer[i].lPdu.canId;
                canPduPtr->length = TxPduBuffer[i].lPdu.dlc;
                canPduPtr->swPduHandle = TxPduBuffer[i].lPdu.pduId;
                canPduPtr->sdu = TxPduBuffer[i].lPdu.data;
                highestPrioCanId = TxPduBuffer[i].lPdu.canId;
                highestPrioIsExt = (0 != (TxPduBuffer[i].lPdu.canId & (1ul<<EXT_ID_BIT_POS)));
                found = TRUE;
            }
        }
    }
    SchM_Exit_CanIf_EA_0();
    if( found ) {
        ret = E_OK;
    } else {
        ret = E_NOT_OK;
    }
    return ret;
}
static void qClear(const CanIf_TxBufferConfigType *bufferPtr) {
    uint16 startIndex;
    SchM_Enter_CanIf_EA_0();
    startIndex = BufferStartIndex[bufferPtr->CanIf_Arc_BufferId];
    for( uint16 i = startIndex; i < (startIndex + bufferPtr->CanIfBufferSize); i++ ) {
        TxPduBuffer[i].inUse = FALSE;
    }
    SchM_Exit_CanIf_EA_0();
}
#endif

#if !defined(CFG_CAN_USE_SYMBOLIC_CANIF_CONTROLLER_ID)
static Std_ReturnType ControllerToChannel( uint8 controllerId, CanIf_Arc_ChannelIdType *channel )
{
    Std_ReturnType ret = E_NOT_OK;
    if( NULL != channel ) {
        for(CanIf_Arc_ChannelIdType i = 0; i < CANIF_CHANNEL_CNT; i++) {
            if(CanIf_ConfigPtr->Arc_ChannelConfig[i].CanControllerId == controllerId) {
                *channel = (CanIf_Arc_ChannelIdType)i;
                ret = E_OK;
            }
        }
    }
    return ret;
}
#endif

#if (CANIF_ARC_TRANSCEIVER_API == STD_ON)
static Std_ReturnType TrcvToChnlId( uint8 Transceiver, uint8 *Channel )
{
    Std_ReturnType ret = E_NOT_OK;
    if( NULL != Channel ) {
        for(uint8 i = 0; i < CANIF_TRANSCEIVER_CHANNEL_CNT; i++) {
            if(CanIf_ConfigPtr->CanIfTransceiverConfig[i].CanIfTrcvCanTrcvIdRef == Transceiver) {
                *Channel = i;
                ret = E_OK;
            }
        }
    }
    return ret;
}
#endif
#if (CANIF_PUBLIC_TX_BUFFERING == STD_ON)
static void ClearTxBuffers( uint8 channelId )
{
    const CanIf_Arc_ChannelConfigType channelConfigPtr = CanIf_ConfigPtr->Arc_ChannelConfig[channelId];
    for( uint8 i = 0; i < channelConfigPtr.NofTxBuffers; i++ ) {
        qClear(channelConfigPtr.TxBufferRefList[i]);
    }
}
#endif

#if (CANIF_PUBLIC_TX_BUFFERING == STD_ON)
static void ChannelOnEnterPDUMode( uint8 channelId, CanIf_PduSetModeType pduMode )
{
    switch(pduMode) {
        case CANIF_SET_OFFLINE:/* @req CANIF073 */
        case CANIF_SET_TX_OFFLINE:/* @req CANIF489 */
            ClearTxBuffers(channelId);
            break;
        default:
            break;
    }
}

static void ChannelOnEnterControllerMode( CanIf_Arc_ChannelIdType channelId, CanIf_ControllerModeType controllerMode )
{

    if(CANIF_CS_STOPPED == controllerMode) {
        /* @req CANIF485 */
        ClearTxBuffers((uint8)channelId);
    }
}
#endif
/**
 *
 * @param ConfigPtr
 */
/* @req 4.0.3/CANIF001 */
void CanIf_Init(const CanIf_ConfigType *configPtr) {

    /* @req 4.0.3/CANIF085 */ //buffers and flags must be cleared when support is added
    /* !req 4.0.3/CANIF479 */ //CCMSM enters CANIF_CS_INIT -> clear stored wakeup events
    /* !req 4.0.3/CANIF301 */

    /* @req 4.0.3/CANIF302 */
    // Only PostBuild case supported
    VALIDATE_NO_RV((NULL != configPtr),CANIF_INIT_ID, CANIF_E_PARAM_POINTER);

    CanIf_ConfigPtr = configPtr;

    /* @req 4.0.3/CANIF476 */
    /* @req 4.0.3/CANIF477 */
    /* @req 4.0.3/CANIF478 */
    /* These all cooks down to controller mode CANIF_CS_STOPPED */
    for (uint8 channel = 0; channel < CANIF_CHANNEL_CNT; channel++) {
        CanIf_Global.channelData[channel].ControllerMode = CANIF_CS_STOPPED;
//      ControllerOnEnterMode(channel, CANIF_CS_STOPPED);
        /* No req but still.. : PDU mode to OFFLINE */
        CanIf_Global.channelData[channel].PduMode = CANIF_GET_OFFLINE;

#if (CANIF_PUBLIC_PN_SUPPORT == STD_ON)
        /* @req 4.0.3/CANIF748 */
        if (CanIf_ConfigPtr->Arc_ChannelConfig[channel].CanIfCtrlPnFilterSet)
        {
            /* @req 4.1.1/CANIF863 */
            CanIf_Global.channelData[channel].pnTxFilterEnabled = TRUE;
        } else {
            CanIf_Global.channelData[channel].pnTxFilterEnabled = FALSE;
        }

#endif

#if (CANIF_PUBLIC_WAKEUP_CHECK_VALIDATION_SUPPORT == STD_ON)
        CanIf_Global.channelData[channel].firstRxIndication = FALSE;
#endif

    }

#if (CANIF_PUBLIC_TX_BUFFERING == STD_ON)
    /* Clear tx buffer */
    /* @req CANIF387 */
    for( uint8 i = 0; (i < CANIF_ARC_MAX_NUM_LPDU_TX_BUF); i++) {
        TxPduBuffer[i].inUse = FALSE;
    }
    /* Setup mapping from buffer index to start index in L_PDU buffer */
    uint16 indx = 0;
    for( uint16 bufIndex = 0; bufIndex < CanIf_ConfigPtr->InitConfig->CanIfNumberOfTxBuffers; bufIndex++ ) {
        BufferStartIndex[CanIf_ConfigPtr->InitConfig->CanIfBufferCfgPtr[bufIndex].CanIf_Arc_BufferId] = indx;
        indx += CanIf_ConfigPtr->InitConfig->CanIfBufferCfgPtr[bufIndex].CanIfBufferSize;
    }
#endif
#if ( CANIF_ARC_TRANSCEIVER_API == STD_ON )
    for (uint8 trcvChnl=0; trcvChnl<CANIF_TRANSCEIVER_CHANNEL_CNT;trcvChnl++)
    {
        /* Implementation assumes that after initialization all transceivers are in normal mode */
        CanIf_Global.trcvData[trcvChnl].trcvMode = CANTRCV_TRCVMODE_NORMAL;
    }
#endif

    CanIf_Global.initRun = TRUE;
}

/* @req 4.0.3/CANIF699 */
void CanIf_ControllerModeIndication(uint8 ControllerId,
           CanIf_ControllerModeType ControllerMode)
{
    /* !req 4.0.3/CANIF479 */
    /* !req 4.0.3/CANIF703 */
    /* @req 4.0.3/CANIF702 */
    /* @req 4.0.3/CANIF661 */
    VALIDATE_NO_RV((TRUE == CanIf_Global.initRun), CANIF_CONTROLLER_MODE_INDICATION_ID, CANIF_E_UNINIT);

    /* NOTE: Should we have this check? */
    VALIDATE_NO_RV(IS_VALID_CONTROLLER_MODE(ControllerMode), CANIF_CONTROLLER_MODE_INDICATION_ID, CANIF_E_PARAM_CTRLMODE);

    CanIf_Arc_ChannelIdType channel = 0;
#if defined(CFG_CAN_USE_SYMBOLIC_CANIF_CONTROLLER_ID)
    VALIDATE_NO_RV((ControllerId < CANIF_CHANNEL_CNT), CANIF_CONTROLLER_MODE_INDICATION_ID, CANIF_E_PARAM_CONTROLLER);
    channel = (CanIf_Arc_ChannelIdType)ControllerId;
#else
    if(E_OK != ControllerToChannel(ControllerId, &channel)) {
        /* @req 4.0.3/CANIF700 */
        DET_REPORT_ERROR(CANIF_CONTROLLER_MODE_INDICATION_ID, CANIF_E_PARAM_CONTROLLER);
        return;
    }
#endif
    /* In AUTOSAR 4.0.3 CanIf specification, all mode transitions between the states
     * CANIF_CS_STARTED, CANIF_CS_SLEEP and CANIF_CS_STOPPED have requirements
     * except between CANIF_CS_STARTED and CANIF_CS_SLEEP. These are invalid transitions.
     * CANIF474 says "The CAN Interface module shall not contain any complete controller
     * state machine. Therefore we leave it up to the CAN driver not to command these
     * transitions. We just set the controller mode to the mode indicated by the CAN driver. */
    /* @req 4.0.3/CANIF713 */
    /* @req 4.0.3/CANIF714 */
    /* @req 4.0.3/CANIF715 */
    /* @req 4.0.3/CANIF716 */
    /* @req 4.0.3/CANIF717 */
    /* @req 4.0.3/CANIF718 */
    /* @req 4.0.3/CANIF719 */
#if (CANIF_PUBLIC_TX_BUFFERING == STD_ON)
    /* @req CANIF485 */
    ChannelOnEnterControllerMode(channel, ControllerMode);
#endif
    if( NULL != CanIfDispatchConfig.CanIfControllerModeIndication ) {
        /* @req 4.0.3/CANIF687 */
        /* @req 4.0.3/CANIF711 */
        /* !req 4.0.3/CANIF688 */
        CanIfDispatchConfig.CanIfControllerModeIndication(channel, ControllerMode);
    }

    CanIf_Global.channelData[channel].ControllerMode = ControllerMode;
#if (CANIF_PUBLIC_WAKEUP_CHECK_VALIDATION_SUPPORT == STD_ON)
    if(CANIF_CS_SLEEP == ControllerMode)
    {
        /* @req 4.0.3/CANIF756 */
        CanIf_Global.channelData[channel].firstRxIndication = FALSE;
    }

#endif

}

/* @req 4.0.3/CANIF003 */
Std_ReturnType CanIf_SetControllerMode(uint8 ControllerId,
                                       CanIf_ControllerModeType ControllerMode)
{
    /* !req 4.0.3/CANIF312 */

    CanIf_ControllerModeType currMode;

    /* @req 4.0.3/CANIF661 */
    VALIDATE_RV((TRUE == CanIf_Global.initRun), CANIF_SET_CONTROLLER_MODE_ID, CANIF_E_UNINIT, E_NOT_OK);
    /* @req 4.0.3/CANIF311 */
    VALIDATE_RV((ControllerId < CANIF_CHANNEL_CNT), CANIF_SET_CONTROLLER_MODE_ID, CANIF_E_PARAM_CONTROLLERID, E_NOT_OK);
    /* @req 4.0.3/CANIF774 */
    VALIDATE_RV(IS_VALID_CONTROLLER_MODE(ControllerMode), CANIF_SET_CONTROLLER_MODE_ID, CANIF_E_PARAM_CTRLMODE, E_NOT_OK);

    uint8 canControllerId = ARC_GET_CHANNEL_CONTROLLER(ControllerId);
    currMode = CanIf_Global.channelData[ControllerId].ControllerMode;
    /* @req 4.0.3/CANIF474 */

    Can_StateTransitionType canTransition = CAN_T_START;
    Std_ReturnType ret = E_OK;
    switch(ControllerMode) {
        case CANIF_CS_STOPPED:
            if(CANIF_CS_SLEEP == currMode) {
                /* @req 4.0.3/CANIF487 */
                canTransition = CAN_T_WAKEUP;
            } else {
                /* @req 4.0.3/CANIF480 */
                /* @req 4.0.3/CANIF585 */
                canTransition = CAN_T_STOP;
            }
            break;
        case CANIF_CS_STARTED:
            /* @req 4.0.3/CANIF481 */
            /* @req 4.0.3/CANIF584 */
            /* Invalid transition if current mode is CANIF_CS_SLEEP.
             * CAN411 in CanDrv spec. says that "when the function Can_SetControllerMode( CAN_T_SLEEP )
             * is entered and the CAN controller is neither in state STOPPED nor in state SLEEP,
             * it shall detect an invalid state transition".
             * Let CAN driver detect invalid state transition. */
            canTransition = CAN_T_START;
            break;
        case CANIF_CS_SLEEP:
            /* @req 4.0.3/CANIF482 */
            /* @req 4.0.3/CANIF486 */
            /* Invalid transition if current mode is CANIF_CS_STARTED.
             * CAN411 in CanDrv spec. says that "when the function Can_SetControllerMode( CAN_T_SLEEP )
             * is entered and the CAN controller is neither in state STOPPED nor in state SLEEP,
             * it shall detect an invalid state transition".
             * Let CAN driver detect invalid state transition. */
            canTransition = CAN_T_SLEEP;
            break;
        default:
            ret = E_NOT_OK;
            break;
    }
    if(E_OK == ret) {
        /* Forward transition and let the CAN driver detect invalid transition. */
        /* @req 4.0.3/CANIF308 */
        if(CAN_NOT_OK == Can_SetControllerMode( canControllerId, canTransition )) {
            /* @req 4.0.3/CANIF475 */
            ret = E_NOT_OK;
        }
    }

    return ret;
}

/* @req 4.0.3/CANIF229 */
Std_ReturnType CanIf_GetControllerMode(uint8 ControllerId,
        CanIf_ControllerModeType *ControllerModePtr) {

    /* !req 4.0.3/CANIF316 */

    /* @req 4.0.3/CANIF661 */
    VALIDATE_RV((TRUE == CanIf_Global.initRun), CANIF_GET_CONTROLLER_MODE_ID, CANIF_E_UNINIT, E_NOT_OK);
    /* @req 4.0.3/CANIF313 */
    VALIDATE_RV((ControllerId < CANIF_CHANNEL_CNT), CANIF_GET_CONTROLLER_MODE_ID, CANIF_E_PARAM_CONTROLLERID, E_NOT_OK);
    /* @req 4.0.3/CANIF656 */
    VALIDATE_RV((NULL != ControllerModePtr), CANIF_GET_CONTROLLER_MODE_ID, CANIF_E_PARAM_POINTER, E_NOT_OK);

    /* @req 4.0.3/CANIF541 */
    *ControllerModePtr = CanIf_Global.channelData[ControllerId].ControllerMode;

    return E_OK;
}

/**
 *
 * @param canTxPduId
 * @param pduInfoPtr
 * @return
 */
/* @req 4.0.3/CANIF005 */
Std_ReturnType CanIf_Transmit(PduIdType CanTxPduId,
        const PduInfoType *PduInfoPtr) {
    /* !req 4.0.3/CANIF323 */
    /* @req 4.0.3/CANIF075 */
    /* !req 4.0.3/CANIF666 */
    /* !req CANIF058 */

    Can_PduType canPdu;
    Can_ReturnType writeRet;
    Std_ReturnType ret;
    Std_ReturnType status;
    status = E_NOT_OK;
    ret = E_OK;

    /* @req 4.0.3/CANIF661 */
    VALIDATE_RV((TRUE == CanIf_Global.initRun), CANIF_TRANSMIT_ID, CANIF_E_UNINIT, E_NOT_OK);
    /* @req 4.0.3/CANIF320 */
    VALIDATE_RV((NULL != PduInfoPtr), CANIF_TRANSMIT_ID, CANIF_E_PARAM_POINTER, E_NOT_OK);
    /* @req 4.0.3/CANIF319 */
    VALIDATE_RV((CanTxPduId < CanIf_ConfigPtr->InitConfig->CanIfNumberOfCanTXPduIds), CANIF_TRANSMIT_ID, CANIF_E_INVALID_TXPDUID, E_NOT_OK);

    const CanIf_TxPduConfigType *txPduPtr = &CanIf_ConfigPtr->InitConfig->CanIfTxPduConfigPtr[CanTxPduId];
    uint8 controller = (uint8)txPduPtr->CanIfTxPduBufferRef->CanIfBufferHthRef->CanIfCanControllerIdRef;
/*############################################################################################*/
    /* IMPROVEMENT: Cleanup needed... */
    /* @req 4.0.3/CANIF382 */
    /* @req 4.0.3/CANIF073 Part of */
    /* @req 4.0.3/CANIF491 Part of */
    if( CANIF_GET_OFFLINE == CanIf_Global.channelData[controller].PduMode || CANIF_GET_RX_ONLINE == CanIf_Global.channelData[controller].PduMode ) {
        DET_REPORT_ERROR(CANIF_TRANSMIT_ID, CANIF_E_STOPPED);
        return E_NOT_OK;
    }
    /* @req 4.0.3/CANIF723 */
    /* @req 4.0.3/CANIF677 */ //Return
    if( CANIF_CS_STOPPED == CanIf_Global.channelData[controller].ControllerMode ) {
        DET_REPORT_ERROR(CANIF_TRANSMIT_ID, CANIF_E_STOPPED);
        return E_NOT_OK;
    }
    /* @req 4.0.3/CANIF317 */
    /* @req 4.0.3/CANIF491 Part of */
    /* @req 4.0.3/CANIF489 Part of*///Return
    if( (CANIF_CS_STARTED != CanIf_Global.channelData[controller].ControllerMode) ||
        (CANIF_GET_RX_ONLINE == CanIf_Global.channelData[controller].PduMode) ||
            (CANIF_GET_OFFLINE == CanIf_Global.channelData[controller].PduMode) ) {
        ret = E_NOT_OK;
    }
    if (ret == E_OK){
/*############################################################################################*/
#if (CANIF_PUBLIC_PN_SUPPORT == STD_ON)
        /* @req 4.0.3/CANIF750 */
        if ((CanIf_ConfigPtr->Arc_ChannelConfig[controller].CanIfCtrlPnFilterSet) && (CanIf_Global.channelData[controller].pnTxFilterEnabled)
                && (!txPduPtr->CanIfTxPduPnFilterEnable ))
        {
            ret = E_NOT_OK;
        }
#endif
        if (ret == E_OK){
            /* @req 4.0.3/CANIF072 */
            /* @req 4.0.3/CANIF491 Part of */
            /* !req 4.0.3/CANIF437 */
            if( (CANIF_GET_OFFLINE_ACTIVE == CanIf_Global.channelData[controller].PduMode) ||
                    (CANIF_GET_OFFLINE_ACTIVE_RX_ONLINE == CanIf_Global.channelData[controller].PduMode) ) {
                if((NO_FUNCTION_CALLOUT != txPduPtr->CanIfUserTxConfirmation) &&
                    (CanIfUserTxConfirmations[txPduPtr->CanIfUserTxConfirmation] != NULL)){
                    CanIfUserTxConfirmations[txPduPtr->CanIfUserTxConfirmation](txPduPtr->CanIfTxPduId);
                }
                status = E_OK;
            }
            if (status == E_NOT_OK) {
                /* @req 4.0.3/CANIF318 */
                canPdu.id = txPduPtr->CanIfCanTxPduIdCanId;
                /* @req 4.0.3/CANIF243 */
                if( CANIF_CAN_ID_TYPE_29 == txPduPtr->CanIfTxPduIdCanIdType ) {
                    canPdu.id |= (1ul << EXT_ID_BIT_POS);
                }
                else if( CANIF_CAN_FD_ID_TYPE_11 == txPduPtr->CanIfTxPduIdCanIdType ) {
                    canPdu.id |= (1ul << CAN_FD_BIT_POS); /*setting FD flag*/
                }
                else if( CANIF_CAN_FD_ID_TYPE_29 == txPduPtr->CanIfTxPduIdCanIdType ) {
                    canPdu.id |= (3ul << CAN_FD_BIT_POS); /*setting both IDE and FD flag*/
                }


                /* Dynamic DLC length */
                if (PduInfoPtr->SduLength < txPduPtr->CanIfCanTxPduIdDlc) {
                    canPdu.length = PduInfoPtr->SduLength;
                } else {
                    canPdu.length = txPduPtr->CanIfCanTxPduIdDlc;
                }
                canPdu.sdu = PduInfoPtr->SduDataPtr;
                canPdu.swPduHandle = CanTxPduId;

                writeRet = Can_Write(txPduPtr->CanIfTxPduBufferRef->CanIfBufferHthRef->CanIfHthIdSymRef, &canPdu);
                if( CAN_BUSY == writeRet ) {
                    ret = E_NOT_OK;

            #if (CANIF_PUBLIC_TX_BUFFERING == STD_ON)
                    /* @req CANIF381 */
                    /* @req CANIF835 */
                    /* @req CANIF063 */
                    /* Should not really have to check for BASIC here since if buffer size greater than 0
                    * and FULLCAN violates req CANIF834_Conf */
                    if( (0 != txPduPtr->CanIfTxPduBufferRef->CanIfBufferSize) &&
                        (CANIF_HANDLE_TYPE_BASIC == txPduPtr->CanIfTxPduBufferRef->CanIfBufferHthRef->CanIfHthType) ) {
                        ret = qReplaceOrAdd(txPduPtr->CanIfTxPduBufferRef, &canPdu, TRUE);
                    }
            #endif
                } else if( CAN_NOT_OK == writeRet ) {
                    /* Nothing to do. Throw message */
                    ret = E_NOT_OK;
                } else if( CAN_OK == writeRet ) {
                    /* @req 4.0.3/CANIF162 */
                    ret = E_OK;
                }
            }else {
                ret = status;
            }
        }
    }
    return ret;
}

/* @req 4.0.3/CANIF007 */
void CanIf_TxConfirmation(PduIdType canTxPduId) {
    const CanIf_TxPduConfigType *txPduPtr;

    /* !req 4.0.3/CANIF413 */
    /* !req 4.0.3/CANIF414 */
    /* !req CANIF058 */

    /* @req 4.0.3/CANIF412 */
    /* @req 4.0.3/CANIF661 */
    VALIDATE_NO_RV((TRUE == CanIf_Global.initRun), CANIF_TXCONFIRMATION_ID, CANIF_E_UNINIT);
    /* @req 4.0.3/CANIF410 */
    VALIDATE_NO_RV((canTxPduId < CanIf_ConfigPtr->InitConfig->CanIfNumberOfCanTXPduIds), CANIF_TXCONFIRMATION_ID, CANIF_E_PARAM_LPDU);

    txPduPtr = &CanIf_ConfigPtr->InitConfig->CanIfTxPduConfigPtr[canTxPduId];

#if (CANIF_PUBLIC_TXCONFIRM_POLLING_SUPPORT == STD_ON)
    /* !req 4.0.3/CANIF740 */
#endif

    CanIf_PduGetModeType mode;
    if( E_OK == CanIf_GetPduMode(txPduPtr->CanIfTxPduBufferRef->CanIfBufferHthRef->CanIfCanControllerIdRef, &mode) ) {
        /* @req 4.0.3/CANIF489*/
        /* @req 4.0.3/CANIF491 Part of */
        /* @req 4.0.3/CANIF075 */
        if ((mode == CANIF_GET_TX_ONLINE) || (mode == CANIF_GET_ONLINE) ||
                (mode == CANIF_GET_OFFLINE_ACTIVE) || (mode == CANIF_GET_OFFLINE_ACTIVE_RX_ONLINE) ) {

#if (CANIF_PUBLIC_TX_BUFFERING == STD_ON)
            if((mode == CANIF_GET_TX_ONLINE) || (mode == CANIF_GET_ONLINE) ) {
                Can_PduType canPdu;
                /* @req CANIF386 */ //Evaluate if there are pending pdus in buffers, assigned to the new free hth
                if(E_OK == qGetBufferedPdu(txPduPtr->CanIfTxPduBufferRef, &canPdu)) {
                    /* Was something in queue. Assume that qGetBufferedPdu returned the pdu
                    * with the highest priority */
                    /* @req CANIF668 */ //Initiate transmit of the highest prio pdu
                    /* @req CANIF070 */ //transmit highest prio pdu
                    /* NOTE: Foreach hth referenced? */
                    if( CAN_OK == Can_Write(txPduPtr->CanIfTxPduBufferRef->CanIfBufferHthRef->CanIfHthIdSymRef, &canPdu) ) {
                        /* @req CANIF183 */ //Remove from buffer if E_OK == Can_Write
                        qRemove(txPduPtr->CanIfTxPduBufferRef, canPdu.id);
                    }
                }
            }
#endif
#if (CANIF_PUBLIC_READTXPDU_NOTIFY_STATUS_API == STD_ON) && FEATURE_NOT_SUPPORTED
        /* !req 4.0.3/CANIF391 */
#endif
#if (CANIF_PUBLIC_PN_SUPPORT == STD_ON)
            uint8 canIfChannel = txPduPtr->CanIfTxPduBufferRef->CanIfBufferHthRef->CanIfCanControllerIdRef;
            /* @req 4.0.3/CANIF751 */
            if ((CanIf_ConfigPtr->Arc_ChannelConfig[canIfChannel].CanIfCtrlPnFilterSet) && (CanIf_Global.channelData[canIfChannel].pnTxFilterEnabled)
                    && (txPduPtr->CanIfTxPduPnFilterEnable ))
            {
                CanIf_Global.channelData[canIfChannel].pnTxFilterEnabled = FALSE;
            }
#endif
            /* @req 4.0.3/CANIF383 */
            if( (NO_FUNCTION_CALLOUT != txPduPtr->CanIfUserTxConfirmation ) &&
                (CanIfUserTxConfirmations[txPduPtr->CanIfUserTxConfirmation]) != NULL){
                /* @req 4.0.3/CANIF011 */
                /* !req 4.0.3/CANIF437 */
                CanIfUserTxConfirmations[txPduPtr->CanIfUserTxConfirmation](txPduPtr->CanIfTxPduId);
            }
#if (CANIF_OSEKNM_SUPPORT == STD_ON)
            if (STD_ON == txPduPtr->OsekNmTxConfirmationSupport) {
                canIfOsekNmTxConfirmation(txPduPtr->CanIfTxPduBufferRef->CanIfBufferHthRef->CanIfCanControllerIdRef, txPduPtr->CanIfCanTxPduIdCanId);
            }
#endif
        }
    }
}
#if(CANIF_PUBLIC_WAKEUP_CHECK_VALIDATION_SUPPORT == STD_ON) && (( CANIF_CTRL_WAKEUP_SUPPORT == STD_ON ) || (CANIF_TRCV_WAKEUP_SUPPORT == STD_ON))
static inline void setFirstRxIndication(uint8 controllerId);
static inline void setFirstRxIndication(uint8 controllerId)
{
    /* @req 4.0.3/CANIF286 */ /*Store event of first call of a CAN controller which has been set to CANIF_CS_STARTED */
    /*lint --e{661} */ /* Previously controllerId is checked for validity */
    if(!CanIf_Global.channelData[controllerId].firstRxIndication){
        CanIf_Global.channelData[controllerId].firstRxIndication = TRUE;
    }
    return;
}
#endif

#if defined(CANIF_PRIVATE_SOFTWARE_FILTER_TYPE_BINARY)
/**
 * Searches for CanId using binary search
 * @param rxPduCfgPtr
 * @param nofEntries
 * @param canId
 * @param rxPdu
 * @return TRUE: CanId found, False: CanId not found
 */
static boolean binarySearch(const CanIf_RxPduConfigType *rxPduCfgPtr, uint16 nofEntries, uint32 canId, const CanIf_RxPduConfigType **rxPdu)
{
    boolean found = FALSE;
    boolean done = FALSE;
    uint16 currentIndex;
    sint32 lowerIndex = 0;
    sint32 upperIndex = nofEntries - 1;
    uint32 actualCanId;
    boolean isExtendedId = IS_EXTENDED_CAN_ID(canId);
#if (CANIF_CANFD_SUPPORT == STD_ON)
    actualCanId = canId & ~(3ul<<CAN_FD_BIT_POS);
#else
    actualCanId = canId & ~(1ul<<EXT_ID_BIT_POS);
#endif
    if(0u != nofEntries) {
        while(!done) {
            currentIndex = (lowerIndex + upperIndex) / 2;
            if( lowerIndex > upperIndex ) {
                /* Search done */
                done = TRUE;
            }
            else {
                /* @req 4.0.3/CANIF645 */
                /* @req 4.0.3/CANIF646 */
                if( (actualCanId <= rxPduCfgPtr[currentIndex].CanIfCanRxPduUpperCanId) &&
                    (actualCanId >= rxPduCfgPtr[currentIndex].CanIfCanRxPduLowerCanId) &&
                    (isExtendedId == rxPduCfgPtr[currentIndex].CanIdIsExtended) ) {
                    found = TRUE;
                    done = TRUE;
                    *rxPdu = &rxPduCfgPtr[currentIndex];
                }
                else {
                    if( rxPduCfgPtr[currentIndex].CanIfCanRxPduLowerCanId < actualCanId ) {
                        lowerIndex = currentIndex + 1;
                    }
                    else {
                        upperIndex = currentIndex - 1;
                    }
                }
            }
        }
    }

    return found;
}
#elif defined(CANIF_PRIVATE_SOFTWARE_FILTER_TYPE_LINEAR)
/**
 * Searches for CanId using linear search
 * @param rxPduCfgPtr
 * @param nofEntries
 * @param canId
 * @param rxPdu
 * @return TRUE: CanId found, False: CanId not found
 */
static boolean linearSearch(const CanIf_RxPduConfigType *rxPduCfgPtr, uint16 nofEntries, uint32 canId, const CanIf_RxPduConfigType **rxPdu)
{
    boolean found = FALSE;
    boolean isExtendedId = IS_EXTENDED_CAN_ID(canId);
    uint32 actualCanId;
#if (CANIF_CANFD_SUPPORT == STD_ON)
    actualCanId = canId & ~(3ul<<CAN_FD_BIT_POS);
#else
    actualCanId = canId & ~(1ul<<EXT_ID_BIT_POS);
#endif
    for(uint16 i = 0; (i < nofEntries) && !found; i++ ) {
        /* @req 4.0.3/CANIF645 */
        /* @req 4.0.3/CANIF646 */
        if( (actualCanId <= rxPduCfgPtr[i].CanIfCanRxPduUpperCanId) &&
            (actualCanId >= rxPduCfgPtr[i].CanIfCanRxPduLowerCanId) &&
            (isExtendedId == rxPduCfgPtr[i].CanIdIsExtended) ) {
            found = TRUE;
            *rxPdu = &rxPduCfgPtr[i];
        }
    }

    return found;
}
#endif


#if defined(CFG_CANIF_ASR_4_3_1)
/**
*
@param Mailbox
@param PduInfoPtr
*/
/* @req 4.3.0/CANIF006 */
void CanIf_RxIndication (const Can_HwType* Mailbox,
const PduInfoType* PduInfoPtr) {
	uint8 *canSduPtr;
	Can_IdType canId;
	Can_HwHandleType hrh;
	uint8 canDlc;

	VALIDATE_NO_RV((NULL != PduInfoPtr), CANIF_RXINDICATION_ID, CANIF_E_PARAM_POINTER);
	VALIDATE_NO_RV((NULL != Mailbox), CANIF_RXINDICATION_ID, CANIF_E_PARAM_POINTER);

	hrh = Mailbox->Hoh;
	canId = Mailbox->CanId;
	canSduPtr = PduInfoPtr->SduDataPtr;
    canDlc = PduInfoPtr->SduLength;

#else
/**
*
* @param hrh
* @param canId
* @param canDlc
* @param canSduPtr
*/
/* @req 4.0.3/CANIF006 */
void CanIf_RxIndication(Can_HwHandleType hrh, Can_IdType canId, uint8 canDlc,
const uint8 *canSduPtr) {
#endif

    CanIf_PduGetModeType mode;
    PduInfoType pduInfo;
    boolean pduMatch = FALSE;
    boolean idCndFlag;

    /* !req 4.0.3/CANIF389 */ //Software filter if configured and if no match end

#if (CANIF_PUBLIC_READRXPDU_NOTIFY_STATUS_API == STD_ON) && FEATURE_NOT_SUPPORTED
    /* !req 4.0.3/CANIF392 */ //Notification status stored if api on and configured
#endif

    /* !req 4.0.3/CANIF422 */
    /* !req 4.0.3/CANIF423 */
    /* !req 4.0.3/CANIF297  */
    /* !req 4.0.3/CANIF198 */
    /* !req 4.0.3/CANIF199 */

    /* !req 4.0.3/CANIF664 */
    //If multiple HRHs, each HRH shall belong at least to a single or fixed group of Rx L-PDU handles (CanRxPduIds)

    /* @req 4.0.3/CANIF421 */
    /* @req 4.0.3/CANIF661 */
    VALIDATE_NO_RV((TRUE == CanIf_Global.initRun), CANIF_RXINDICATION_ID, CANIF_E_UNINIT);

    /* @req 4.0.3/CANIF416 */
    VALIDATE_NO_RV((hrh < CanIf_ConfigPtr->InitConfig->CanIfHohConfigPtr->HrhListSize), CANIF_RXINDICATION_ID, CANIF_E_PARAM_HRH);

    VALIDATE_NO_RV((NO_CANIF_HRH != CanIf_ConfigPtr->InitConfig->CanIfHohConfigPtr->CanHohToCanIfHrhMap[hrh]), CANIF_RXINDICATION_ID, CANIF_E_PARAM_HRH);
    const CanIf_HrhConfigType *hrhConfig = &CanIf_ConfigPtr->InitConfig->CanIfHohConfigPtr->CanIfHrhConfig[CanIf_ConfigPtr->InitConfig->CanIfHohConfigPtr->CanHohToCanIfHrhMap[hrh]];

    /* @req 4.0.3/CANIF417 */

    idCndFlag = ( (IS_EXTENDED_CAN_ID(canId) && ((canId & ~(3ul<<CAN_FD_BIT_POS)) > EXTENDED_CANID_MAX)) ||
            (!IS_EXTENDED_CAN_ID(canId) && ((canId & ~(3ul<<CAN_FD_BIT_POS)) > STANDARD_CANID_MAX)));

#if (CANIF_CANFD_SUPPORT == STD_OFF)
    if((idCndFlag == TRUE) || IS_CANFD(canId)) {
#else
    if( idCndFlag == TRUE) {
#endif
        DET_REPORT_ERROR(CANIF_RXINDICATION_ID, CANIF_E_PARAM_CANID);
        return;
    }
    /* @req 4.0.3/CANIF418 */
#if (CANIF_CANFD_SUPPORT == STD_ON)
    VALIDATE_NO_RV((canDlc <= 64), CANIF_RXINDICATION_ID, CANIF_E_PARAM_DLC);
#else
    VALIDATE_NO_RV((canDlc <= 8), CANIF_RXINDICATION_ID, CANIF_E_PARAM_DLC);
#endif
    /* @req 4.0.3/CANIF419 */
    VALIDATE_NO_RV((NULL != canSduPtr), CANIF_RXINDICATION_ID, CANIF_E_PARAM_POINTER);

    /* @req 4.0.3/CANIF075 */
    /* @req 4.0.3/CANIF490 */
    /* @req 4.0.3/CANIF492 */
    if ( FALSE == ((E_OK != CanIf_GetPduMode(hrhConfig->CanIfHrhCanCtrlIdRef, &mode)) ||
            (CANIF_GET_OFFLINE == mode) || (CANIF_GET_TX_ONLINE == mode)
            || (CANIF_GET_OFFLINE_ACTIVE == mode)) ) {
        const CanIf_RxPduConfigType *rxPduCfgPtr =  hrhConfig->RxPduList;

        if( 0u != hrhConfig->NofRxPdus ) {
            /* @req 4.0.3/CANIF663 */
            /* @req 4.0.3/CANIF665 */
            if( (CANIF_HANDLE_TYPE_FULL == hrhConfig->CanIfHrhType) || !hrhConfig->CanIfSoftwareFilterHrh) {
                /* No sw filtering or FULL-CAN. Assume match... */
                pduMatch = TRUE;
            }
            else {
                /* Software filtering */
                /* @req 4.0.3/CANIF211 */
                /* @req 4.0.3/CANIF281 */
#if defined(CANIF_PRIVATE_SOFTWARE_FILTER_TYPE_BINARY)
                pduMatch = binarySearch(hrhConfig->RxPduList, hrhConfig->NofRxPdus, canId, &rxPduCfgPtr);
#elif defined(CANIF_PRIVATE_SOFTWARE_FILTER_TYPE_LINEAR)
                pduMatch = linearSearch(hrhConfig->RxPduList, hrhConfig->NofRxPdus, canId, &rxPduCfgPtr);
#else
#error "CanIf: Filtering method not supported"
#endif
            }
        }

        if( pduMatch ) {
            /* @req 4.0.3/CANIF030 */
#if (CANIF_PRIVATE_DLC_CHECK == STD_ON)
            /* @req 4.0.3/CANIF390 */
            /* @req 4.0.3/CANIF026 */
            if( canDlc < rxPduCfgPtr->CanIfCanRxPduDlc ) {
                /* @req 4.0.3/CANIF168 */
                DET_REPORT_ERROR(CANIF_RXINDICATION_ID, CANIF_E_PARAM_DLC);
                return;
            }
#endif

#if ( CANIF_PUBLIC_READRXPDU_DATA_API == STD_ON ) && FEATURE_NOT_SUPPORTED
        /* IMPROVEMENT: Add support for CANIF_PUBLIC_READRXPDU_DATA_API */
#endif

#if(CANIF_PUBLIC_WAKEUP_CHECK_VALIDATION_SUPPORT == STD_ON) && (( CANIF_CTRL_WAKEUP_SUPPORT == STD_ON ) || (CANIF_TRCV_WAKEUP_SUPPORT == STD_ON))
            setFirstRxIndication(hrhConfig->CanIfHrhCanCtrlIdRef);
#endif
            /* @req 4.0.3/CANIF012 */
            /* @req 4.0.3/CANIF056 */ //If accepted during DLC check, identify target upper layer module if configured
            if (( rxPduCfgPtr->CanIfUserRxIndication != NO_FUNCTION_CALLOUT ) &&
                (CanIfUserRxIndications[rxPduCfgPtr->CanIfUserRxIndication] != NULL)){
                /* @req 4.0.3/CANIF135 */
                /* @req 4.0.3/CANIF830 */
                /* @req 4.0.3/CANIF829 */
                /* @req 4.0.3/CANIF415 */
                /* @req 4.0.3/CANIF057 */
                /* !req 4.0.3/CANIF440 */
                pduInfo.SduLength = canDlc;
                pduInfo.SduDataPtr = (uint8 *)canSduPtr;  /* throw away const for some reason */
                CanIfUserRxIndications[rxPduCfgPtr->CanIfUserRxIndication](rxPduCfgPtr->CanIfCanRxPduId, &pduInfo);
            }
#if (CANIF_OSEKNM_SUPPORT == STD_ON)
            if (STD_ON == rxPduCfgPtr->OsekNmRxIndicationSupport) {
                pduInfo.SduLength = canDlc;
                pduInfo.SduDataPtr = (uint8 *)canSduPtr;
                canIfOsekNmRxIndication(hrhConfig->CanIfHrhCanCtrlIdRef,canId, &pduInfo);
            }
#endif
        }
    }
}


/* !req 4.0.3/CANIF521 */
#if (CANIF_PUBLIC_CANCEL_TRANSMIT_SUPPORT == STD_ON) && FEATURE_NOT_SUPPORTED
/* !req 4.0.3/CANIF520 */
Std_ReturnType CanIf_CancelTransmit(PduIdType CanTxPduId)
{
    /* !req 4.0.3/CANIF652 */
    return E_OK;
}

#endif

#if (CANIF_CTRLDRV_TX_CANCELLATION == STD_ON)
/* @req CANIF428 */
/* @req CANIF101 */
void CanIf_CancelTxConfirmation(PduIdType canTxPduId,  const PduInfoType *pduInfoPtr)
{
    /* !req CANIF427 */
    /* !req CANIF058 */
    /* @req CANIF426 */
    VALIDATE_NO_RV((TRUE == CanIf_Global.initRun), CANIF_CANCELTXCONFIRMATION_ID, CANIF_E_UNINIT);
    /* @req CANIF424 */
    VALIDATE_NO_RV((canTxPduId < CanIf_ConfigPtr->InitConfig->CanIfNumberOfCanTXPduIds), CANIF_CANCELTXCONFIRMATION_ID, CANIF_E_PARAM_LPDU);
    /* @req CANIF828 */
    VALIDATE_NO_RV((NULL != pduInfoPtr), CANIF_CANCELTXCONFIRMATION_ID, CANIF_E_PARAM_POINTER);

#if (CANIF_PUBLIC_TX_BUFFERING == STD_ON)
    const CanIf_TxPduConfigType *txPduPtr;
    txPduPtr = &CanIf_ConfigPtr->InitConfig->CanIfTxPduConfigPtr[canTxPduId];
    CanIf_PduGetModeType mode = CanIf_Global.channelData[txPduPtr->CanIfTxPduBufferRef->CanIfBufferHthRef->CanIfCanControllerIdRef].PduMode;

    if( ((CANIF_GET_TX_ONLINE == mode) || (CANIF_GET_ONLINE == mode)) &&
            (CANIF_CS_STARTED == CanIf_Global.channelData[txPduPtr->CanIfTxPduBufferRef->CanIfBufferHthRef->CanIfCanControllerIdRef].ControllerMode )) {
        Can_PduType canPdu;
        canPdu.length = pduInfoPtr->SduLength;
        canPdu.id = txPduPtr->CanIfCanTxPduIdCanId;
        if( CANIF_CAN_ID_TYPE_29 == txPduPtr->CanIfTxPduIdCanIdType ) {
            canPdu.id |= (1ul << EXT_ID_BIT_POS);
        }
        else if( CANIF_CAN_FD_ID_TYPE_11 == txPduPtr->CanIfTxPduIdCanIdType ) {
            canPdu.id |= (1ul << CAN_FD_BIT_POS); /*setting FD flag*/
        }
        else if( CANIF_CAN_FD_ID_TYPE_29 == txPduPtr->CanIfTxPduIdCanIdType ) {
            canPdu.id |= (3ul << CAN_FD_BIT_POS); /*setting both IDE and FD flag*/
        }

        canPdu.sdu = pduInfoPtr->SduDataPtr;
        canPdu.swPduHandle = canTxPduId;
        /* @req CANIF054 */
        /* @req CANIF176 */
        (void)qReplaceOrAdd( txPduPtr->CanIfTxPduBufferRef, &canPdu, FALSE);
        /* @req CANIF386 */ //Evaluate if there are pending pdus in buffers, assigned to the new free hth
        if(E_OK == qGetBufferedPdu(txPduPtr->CanIfTxPduBufferRef, &canPdu)) {
            /* Was something in queue. Assume that qGetBufferedPdu returned the pdu
             * with the highest priority */
            /* @req CANIF668 */ //Initiate transmit of the highest prio pdu
            /* @req CANIF070 */ //transmit highest prio pdu
            if( CAN_OK == Can_Write(txPduPtr->CanIfTxPduBufferRef->CanIfBufferHthRef->CanIfHthIdSymRef, &canPdu) ) {
                /* @req CANIF183 */ //Remove from buffer if E_OK == Can_Write
                qRemove(txPduPtr->CanIfTxPduBufferRef, canPdu.id);
                /* IMPROVEMENT: Should we try to insert the cancelled pdu into the buffer if
                 * the attempt above failed due to lack of space? */
            }
        }
    }
#else
    (void)canTxPduId;
    (void)pduInfoPtr;
#endif

}

#endif


/* !req 4.0.3/CANIF330 */
#if ( CANIF_PUBLIC_READRXPDU_DATA_API == STD_ON ) && FEATURE_NOT_SUPPORTED

/**
 *
 *
 * @param canRxPduId
 * @param pduInfoPtr
 * @return
 */
/* !req 4.0.3/CANIF194*/
Std_ReturnType CanIf_ReadRxPduData(PduIdType canRxPduId,  PduInfoType *pduInfoPtr)
{
    /* !req 4.0.3/CANIF324*/
    /* !req 4.0.3/CANIF325 */
    /* !req 4.0.3/CANIF329 */
    /* !req CANIF058 */

    VALIDATE_NO_RV(FALSE, CANIF_READTXPDUDATA_ID, CANIF_E_NOK_NOSUPPORT);
    //Requirement 661
    VALIDATE_NO_RV(CanIf_Global.initRun == STD_ON, CANIF_READTXPDUDATA_ID, CANIF_E_UNINIT );
    /* !req 4.0.3/CANIF326 */
    VALIDATE_NO_RV(pduInfoPtr != 0, CANIF_READTXPDUDATA_ID, CANIF_E_PARAM_POINTER );

    /* IMPROVMENT: Do it?
     * This API is actually not used by any upper layers
     */


  return E_NOT_OK;
}
#endif


/* !req 4.0.3/CANIF335 */
#if ( CANIF_PUBLIC_READTXPDU_NOTIFY_STATUS_API == STD_ON ) && FEATURE_NOT_SUPPORTED
/* !req 4.0.3/CANIF202 */
CanIf_NotifStatusType CanIf_ReadTxNotifStatus(PduIdType CanTxPduId)
{
    /* !req 4.0.3/CANIF331 */
    /* !req 4.0.3/CANIF334 */
    /* !req 4.0.3/CANIF393 */
    return CANIF_NO_NOTIFICATION;
}
#endif
/* !req 4.0.3/CANIF340 */
#if ( CANIF_PUBLIC_READRXPDU_NOTIFY_STATUS_API == STD_ON ) && FEATURE_NOT_SUPPORTED
/* !req 4.0.3/CANIF230 */
CanIf_NotifStatusType CanIf_ReadRxNotifStatus(PduIdType CanRxPduId)
{
    /* !req 4.0.3/CANIF336 */
    /* !req 4.0.3/CANIF339 */
    /* !req 4.0.3/CANIF394 */
    return CANIF_NO_NOTIFICATION;
}
#endif

#if (CANIF_PUBLIC_PN_SUPPORT == STD_ON)
static inline void setPnFilterEnable(uint8 ControllerId);

static inline void setPnFilterEnable(uint8 ControllerId)
{
    /* @req 4.0.3/CANIF749*/ /* @req 4.0.3/CANIF748 */ /* @req 4.0.3/CANIF752 */
    if (CanIf_ConfigPtr->Arc_ChannelConfig[ControllerId].CanIfCtrlPnFilterSet)
    {
        CanIf_Global.channelData[ControllerId].pnTxFilterEnabled = TRUE;
    }
    return;
}
#endif

/* @req 4.0.3/CANIF008 */
Std_ReturnType CanIf_SetPduMode( uint8 ControllerId, CanIf_PduSetModeType PduModeRequest )
{
    /* !req 4.0.3/CANIF344*/

    /* NOTE: The channel id is used and not the controller */
    Std_ReturnType ret;
    ret = E_OK;
    CanIf_PduGetModeType currMode;
    CanIf_PduGetModeType newMode;

    /* @req 4.0.3/CANIF661 */
    VALIDATE_RV((TRUE == CanIf_Global.initRun), CANIF_SETPDUMODE_ID, CANIF_E_UNINIT, E_NOT_OK);
    /* @req 4.0.3/CANIF341*/
    VALIDATE_RV((ControllerId < CANIF_CHANNEL_CNT), CANIF_SETPDUMODE_ID, CANIF_E_PARAM_CONTROLLERID, E_NOT_OK);

    currMode = CanIf_Global.channelData[ControllerId].PduMode;
    newMode = currMode;

    switch(PduModeRequest) {
        case CANIF_SET_OFFLINE:
            newMode = CANIF_GET_OFFLINE;
            break;
        case CANIF_SET_ONLINE:
#if (CANIF_PUBLIC_PN_SUPPORT == STD_ON)
            setPnFilterEnable(ControllerId);
#endif
            newMode = CANIF_GET_ONLINE;
            break;
        case CANIF_SET_RX_OFFLINE:
            if( CANIF_GET_ONLINE == currMode ) {
                newMode = CANIF_GET_TX_ONLINE;
            } else if( CANIF_GET_RX_ONLINE == currMode ) {
                newMode = CANIF_GET_OFFLINE;
            } else if( CANIF_GET_OFFLINE_ACTIVE_RX_ONLINE == currMode ) {
                newMode = CANIF_GET_OFFLINE_ACTIVE;
            }
            break;
        case CANIF_SET_RX_ONLINE:
            if(CANIF_GET_TX_ONLINE == currMode ) {
                newMode = CANIF_GET_ONLINE;
            } else if( CANIF_GET_OFFLINE == currMode ) {
                newMode = CANIF_GET_RX_ONLINE;
            } else if( CANIF_GET_OFFLINE_ACTIVE == currMode ) {
                newMode = CANIF_GET_OFFLINE_ACTIVE_RX_ONLINE;
            }
            break;
        case CANIF_SET_TX_OFFLINE:
            if( (CANIF_GET_ONLINE == currMode) || (CANIF_GET_OFFLINE_ACTIVE_RX_ONLINE == currMode) ) {
                newMode = CANIF_GET_RX_ONLINE;
            } else if( (CANIF_GET_TX_ONLINE == currMode) || (CANIF_GET_OFFLINE_ACTIVE == currMode) ) {
                newMode = CANIF_GET_OFFLINE;
            }
            break;
        case CANIF_SET_TX_OFFLINE_ACTIVE:
            if( CANIF_GET_OFFLINE == currMode || CANIF_GET_TX_ONLINE == currMode) {
                newMode = CANIF_GET_OFFLINE_ACTIVE;
            } else if( (CANIF_GET_ONLINE == currMode) || (CANIF_GET_RX_ONLINE == currMode) ) {
                newMode = CANIF_GET_OFFLINE_ACTIVE_RX_ONLINE;
            }
            break;
        case CANIF_SET_TX_ONLINE:
#if (CANIF_PUBLIC_PN_SUPPORT == STD_ON)
            setPnFilterEnable(ControllerId);
#endif
            if( CANIF_GET_OFFLINE == currMode ) {
                newMode = CANIF_GET_TX_ONLINE;
            } else if( (CANIF_GET_RX_ONLINE == currMode) || (CANIF_GET_OFFLINE_ACTIVE_RX_ONLINE == currMode) ) {
                newMode = CANIF_GET_ONLINE;
            } else if (CANIF_GET_OFFLINE_ACTIVE == currMode ) {
                newMode = CANIF_GET_TX_ONLINE;
            }
            break;
        default:
//            DET_REPORT_ERROR(CANIF_SETPDUMODE_ID, CANIF_E_PARAM_PDUMODE);
            ret = E_NOT_OK;
            break;
    }
#if (CANIF_PUBLIC_TX_BUFFERING == STD_ON)
    if(E_OK == ret) {
        /* ChannelOnEnterPDUMode will tx buffers for this channel and thus obey
         * parts of the following requirements */
        /* CANIF073 */
        /* CANIF489 */
        ChannelOnEnterPDUMode(ControllerId, PduModeRequest);
    }
#endif
    CanIf_Global.channelData[ControllerId].PduMode = newMode;

    return ret;
}

/* @req 4.0.3/CANIF009 */
Std_ReturnType CanIf_GetPduMode( uint8 ControllerId, CanIf_PduGetModeType *PduModePtr )
{
    /* !req 4.0.3/CANIF349 */

    /* @req 4.0.3/CANIF661 */
    VALIDATE_RV((TRUE == CanIf_Global.initRun), CANIF_GETPDUMODE_ID, CANIF_E_UNINIT, E_NOT_OK);
    /* @req 4.0.3/CANIF346 */
    VALIDATE_RV((ControllerId < CANIF_CHANNEL_CNT), CANIF_GETPDUMODE_ID, CANIF_E_PARAM_CONTROLLERID, E_NOT_OK);
    /* @req 4.0.3/CANIF657 */
    VALIDATE_RV((NULL != PduModePtr), CANIF_GETPDUMODE_ID, CANIF_E_PARAM_POINTER, E_NOT_OK);

    *PduModePtr = CanIf_Global.channelData[ControllerId].PduMode;

    return E_OK;
}

/* @req 4.0.3/CANIF218 */
void CanIf_ControllerBusOff( uint8 ControllerId )
{
    CanIf_Arc_ChannelIdType channel = 0;

    /* !req 4.0.3/CANIF432 */
    /* !req 4.0.3/CANIF433 */
    /* @req 4.0.3/CANIF118 */

    /* @req 4.0.3/CANIF661 */
    /* @req 4.0.3/CANIF431 */
    VALIDATE_NO_RV((TRUE == CanIf_Global.initRun), CANIF_CONTROLLER_BUSOFF_ID, CANIF_E_UNINIT);

#if defined(CFG_CAN_USE_SYMBOLIC_CANIF_CONTROLLER_ID)
    VALIDATE_NO_RV((ControllerId < CANIF_CHANNEL_CNT), CANIF_CONTROLLER_BUSOFF_ID, CANIF_E_PARAM_CONTROLLER);
    channel = (CanIf_Arc_ChannelIdType)ControllerId;
#else
    if(E_OK != ControllerToChannel(ControllerId, &channel)) {
        /* @req 4.0.3/CANIF429 */
        DET_REPORT_ERROR(CANIF_CONTROLLER_BUSOFF_ID, CANIF_E_PARAM_CONTROLLER);
        return;
    }
#endif
    /* @req 4.0.3/CANIF298 */
    /* @req 4.0.3/CANIF488 */
    if( CANIF_CS_UNINIT != CanIf_Global.channelData[channel].ControllerMode ) {
#if (CANIF_PUBLIC_TX_BUFFERING == STD_ON)
        ChannelOnEnterControllerMode(channel, CANIF_CS_STOPPED);
#endif
        CanIf_Global.channelData[channel].ControllerMode = CANIF_CS_STOPPED;
    }
#if (CANIF_PUBLIC_TXCONFIRM_POLLING_SUPPORT == STD_ON) && FEATURE_NOT_SUPPORTED
    /* !req 4.0.3/CANIF739 */
#endif
    /* @req 4.0.3/CANIF724 */
    if(NULL != CanIfDispatchConfig.CanIfBusOffNotification) {
        /* @req 4.0.3/CANIF014 */
        /* !req 4.0.3/CANIF449 */
        CanIfDispatchConfig.CanIfBusOffNotification(channel);
    }
#if (CANIF_OSEKNM_SUPPORT == STD_ON)
    if (CanIf_ConfigPtr->Arc_ChannelConfig[channel].CanIfOsekNmNetId != CANIF_OSEKNM_INVALID_NET_ID) {
        OsekNm_ControllerBusOff(CanIf_ConfigPtr->Arc_ChannelConfig[channel].CanIfOsekNmNetId);
    }
#endif
}

/* !req 4.0.3/CANIF780 */
/* !req 4.0.3/CANIF784 */
#if ( CANIF_PUBLIC_CHANGE_BAUDRATE_SUPPORT == STD_ON ) && FEATURE_NOT_SUPPORTED
/* !req 4.0.3/CANIF775 */
Std_ReturnType CanIf_CheckBaudrate(uint8 ControllerId, const uint16 Baudrate)
{
    /* !req 4.0.3/CANIF778 */
    /* !req 4.0.3/CANIF779 */
    return E_NOT_OK;
}
/* !req 4.0.3/CANIF776 */
Std_ReturnType CanIf_ChangeBaudrate(uint8 ControllerId, const uint16 Baudrate)
{
    /* !req 4.0.3/CANIF782 */
    /* !req 4.0.3/CANIF783 */
    /* !req 4.0.3/CANIF787 */
    return E_NOT_OK;
}
#endif

/* !req 4.0.3/CANIF357 */
#if ( CANIF_PUBLIC_SETDYNAMICTXID_API == STD_ON ) && FEATURE_NOT_SUPPORTED
/* !req 4.0.3/CANIF189 */
void CanIf_SetDynamicTxId( PduIdType CanTxPduId, Can_IdType CanId )
{
    /* !req 4.0.3/CANIF188 */ //Maybe this req. should be elsewhere
    /* !req 4.0.3/CANIF352 */
    /* !req 4.0.3/CANIF353 */
    /* !req 4.0.3/CANIF355 */
    /* !req 4.0.3/CANIF356 */
    /* !req 4.0.3/CANIF673 */
}
#endif

/* @req 4.0.3/CANIF362 */
/* @req 4.0.3/CANIF367 */
/* @req 4.0.3/CANIF371 */
/* @req 4.0.3/CANIF373 */
/* @req 4.0.3/CANIF730 */
#if ( CANIF_ARC_TRANSCEIVER_API == STD_ON )
/* @req 4.0.3/CANIF287 */
Std_ReturnType CanIf_SetTrcvMode( uint8 TransceiverId, CanTrcv_TrcvModeType TransceiverMode )
{
    uint8 trcvHwCnlId;
    boolean validSts;

    /* @req 4.0.3/CANIF661 */
    VALIDATE_RV((TRUE == CanIf_Global.initRun), CANIF_SET_TRANSCEIVERMODE_ID, CANIF_E_UNINIT,E_NOT_OK);

    /* @req 4.0.3/CANIF538 */
    VALIDATE_RV((TransceiverId < CANIF_TRANSCEIVER_CHANNEL_CNT), CANIF_SET_TRANSCEIVERMODE_ID, CANIF_E_PARAM_TRCV,E_NOT_OK);

    /* @req 4.0.3/CANIF648 */
    validSts = ((CANTRCV_TRCVMODE_STANDBY == TransceiverMode)||(CANTRCV_TRCVMODE_SLEEP == TransceiverMode) ||(CANTRCV_TRCVMODE_NORMAL == TransceiverMode));
    VALIDATE_RV((validSts), CANIF_SET_TRANSCEIVERMODE_ID, CANIF_E_PARAM_TRCVMODE,E_NOT_OK);

    /* Maps from CanIfTransceiverId to CanTransceiverId */
    trcvHwCnlId = ARC_GET_CAN_TRANSCEIVERID(TransceiverId);

    /* @req 4.0.3/CANIF358 */
    return CanTrcv_SetOpMode(trcvHwCnlId, TransceiverMode);
}

/* @req 4.0.3/CANIF288 */
Std_ReturnType CanIf_GetTrcvMode( CanTrcv_TrcvModeType *TransceiverModePtr, uint8 TransceiverId)
{
    uint8 trcvHwCnlId;

    /* @req 4.0.3/CANIF661 */
    VALIDATE_RV((TRUE == CanIf_Global.initRun), CANIF_GET_TRANSCEIVERMODE_ID, CANIF_E_UNINIT,E_NOT_OK);

    /* @req 4.0.3/CANIF364 */
    VALIDATE_RV((TransceiverId < CANIF_TRANSCEIVER_CHANNEL_CNT), CANIF_GET_TRANSCEIVERMODE_ID, CANIF_E_PARAM_TRCV,E_NOT_OK);

    /* @req 4.0.3/CANIF650 */
    VALIDATE_RV(( NULL != TransceiverModePtr ), CANIF_GET_TRANSCEIVERMODE_ID, CANIF_E_PARAM_POINTER,E_NOT_OK);

    /* Maps from CanIfTransceiverId to CanTransceiverId */
    trcvHwCnlId = ARC_GET_CAN_TRANSCEIVERID(TransceiverId);

    /* @req 4.0.3/CANIF363 */
    return CanTrcv_GetOpMode(trcvHwCnlId, TransceiverModePtr);

}

/* @req 4.0.3/CANIF289 */
Std_ReturnType CanIf_GetTrcvWakeupReason(uint8 TransceiverId, CanTrcv_TrcvWakeupReasonType* TrcvWuReasonPtr)
{
    uint8 trcvHwCnlId;

    /* @req 4.0.3/CANIF661 */
    VALIDATE_RV((TRUE == CanIf_Global.initRun), CANIF_GET_TRCVWAKEUPREASON_ID, CANIF_E_UNINIT,E_NOT_OK);

    /* @req 4.0.3/CANIF537 */
    VALIDATE_RV((TransceiverId < CANIF_TRANSCEIVER_CHANNEL_CNT), CANIF_GET_TRCVWAKEUPREASON_ID, CANIF_E_PARAM_TRCV,E_NOT_OK);

    /* @req 4.0.3/CANIF649 */
    VALIDATE_RV(( NULL != TrcvWuReasonPtr ), CANIF_GET_TRCVWAKEUPREASON_ID, CANIF_E_PARAM_POINTER,E_NOT_OK);

    /* Maps from CanIfTransceiverId to CanTransceiverId */
    trcvHwCnlId = ARC_GET_CAN_TRANSCEIVERID(TransceiverId);

    /* @req 4.0.3/CANIF368 */
    return CanTrcv_GetBusWuReason(trcvHwCnlId, TrcvWuReasonPtr);
}

/* @req 4.0.3/CANIF290 */
Std_ReturnType CanIf_SetTrcvWakeupMode(uint8 TransceiverId,CanTrcv_TrcvWakeupModeType TrcvWakeupMode)
{
    uint8 trcvHwCnlId;
    boolean validSts;

    /* @req 4.0.3/CANIF661 */
    VALIDATE_RV((TRUE == CanIf_Global.initRun), CANIF_SET_TRANSCEIVERWAKEMODE_ID, CANIF_E_UNINIT,E_NOT_OK);

    /* @req 4.0.3/CANIF535 */
    VALIDATE_RV((TransceiverId < CANIF_TRANSCEIVER_CHANNEL_CNT), CANIF_SET_TRANSCEIVERWAKEMODE_ID, CANIF_E_PARAM_TRCV,E_NOT_OK);

    /* @req 4.0.3/CANIF536 */
    validSts = (( TrcvWakeupMode == CANTRCV_WUMODE_ENABLE) || (TrcvWakeupMode == CANTRCV_WUMODE_DISABLE) || ( TrcvWakeupMode == CANTRCV_WUMODE_CLEAR));
    VALIDATE_RV((validSts), CANIF_SET_TRANSCEIVERWAKEMODE_ID, CANIF_E_PARAM_TRCVWAKEUPMODE,E_NOT_OK);

    /* Maps from CanIfTransceiverId to CanTransceiverId */
    trcvHwCnlId = ARC_GET_CAN_TRANSCEIVERID(TransceiverId);

    /* @req 4.0.3/CANIF372 */
    return CanTrcv_SetWakeupMode (trcvHwCnlId,TrcvWakeupMode);
}

/* @req 4.0.3/CANIF764 */
void CanIf_TrcvModeIndication( uint8 TransceiverId, CanTrcv_TrcvModeType TransceiverMode )
{
    uint8 Channel;
    /* @req 4.0.3/CANIF710 */
    /* @req 4.0.3/CANIF661 */
    /* @req 4.0.3/CANIF708 */
    /* @req 4.0.3/CANIF709 */
    VALIDATE_NO_RV((TRUE == CanIf_Global.initRun), CANIF_TRANSCEIVER_MODE_INDICATION_ID, CANIF_E_UNINIT);

    /* @req 4.0.3/CANIF706 */
    VALIDATE_NO_RV((E_OK == TrcvToChnlId(TransceiverId,&Channel)), CANIF_TRANSCEIVER_MODE_INDICATION_ID, CANIF_E_PARAM_TRCV);

    CanIf_Global.trcvData[Channel].trcvMode = TransceiverMode;

    if( NULL != CanIfDispatchConfig.CanIfTrcvModeChangeNotification )
    {
      CanIfDispatchConfig.CanIfTrcvModeChangeNotification(Channel, TransceiverMode);
    }

    return;
}
#endif


/* @req 4.0.3/CANIF402 */
/* @req 4.0.3/CANIF180 */
#if (( CANIF_CTRL_WAKEUP_SUPPORT == STD_ON ) || (CANIF_TRCV_WAKEUP_SUPPORT == STD_ON))
/* @req 4.0.3/CANIF219 */
Std_ReturnType CanIf_CheckWakeup( EcuM_WakeupSourceType WakeupSource )
{
    Std_ReturnType ApiReturn = E_NOT_OK;
    uint8 trcvHwCnlId;
    uint64 wakUpSrc;
    EcuM_WakeupSourceType lastWakUpSrcVal;
    boolean wakUpSrcFound;
    uint8 i;

    /* @req 4.0.3/CANIF661 */
    /* @req 4.0.3/CANIF401 */
    VALIDATE_RV((TRUE == CanIf_Global.initRun), CANIF_CHECKWAKEUP_ID, CANIF_E_UNINIT,E_NOT_OK);

    /* @req 4.0.3/CANIF398 */
    /* Wakeup source is bit coded message only one unique bit is set to indicate a source, find the last in the list */
    lastWakUpSrcVal = (1<<(ECUM_WKSOURCE_USER_CNT+ECUM_WKSOURCE_SYSTEM_CNT-1));
    wakUpSrcFound = FALSE;
    for (wakUpSrc=1; wakUpSrc <=lastWakUpSrcVal ;wakUpSrc = (wakUpSrc<<1u))
    {
        if ((uint32)wakUpSrc == WakeupSource)
        {
            wakUpSrcFound = TRUE;
            break;
        }
    }
    VALIDATE_RV((wakUpSrcFound), CANIF_CHECKWAKEUP_ID, CANIF_E_PARAM_WAKEUPSOURCE,E_NOT_OK);

    #if ( CANIF_TRCV_WAKEUP_SUPPORT == STD_ON )
    /* @req 4.0.3/CANIF395 */
    /* @req 4.0.3/CANIF720 */
    /* @req 4.0.3/CANIF678 */

    for( i = 0; i < CANIF_TRANSCEIVER_CHANNEL_CNT; i++) {

        if((CanIf_ConfigPtr->CanIfTransceiverConfig[i].CanIfTrcvWakeupSupport) && (CanIf_ConfigPtr->CanIfTransceiverConfig[i].CanIfTrcvCanTrcvWakeupSrc == WakeupSource)){
            trcvHwCnlId = ARC_GET_CAN_TRANSCEIVERID(i);

            if(E_OK == CanTrcv_CheckWakeup(trcvHwCnlId)){
                ApiReturn = E_OK;
            }
        }
    }
    #endif
    #if ( CANIF_CTRL_WAKEUP_SUPPORT == STD_ON )
    /* @req 4.0.3/CANIF395 */ /* Not fulfilled for Can controller */
    /* @req 4.0.3/CANIF720 */
    /* @req 4.0.3/CANIF678 */
    /* !req 4.0.3/CANIF679 */

    #endif

    return ApiReturn;
}

#endif

/* @req 4.0.3/CANIF226 */
/* @req 4.0.3/CANIF408 */
#if (CANIF_PUBLIC_WAKEUP_CHECK_VALIDATION_SUPPORT == STD_ON)
/* @req 4.0.3/CANIF178 */
Std_ReturnType CanIf_CheckValidation( EcuM_WakeupSourceType WakeupSource )
{
    Std_ReturnType status;
    Std_ReturnType ret;
    uint64 wakUpSrc;
    EcuM_WakeupSourceType lastWakUpSrcVal;
    EcuM_WakeupSourceType temp;
    boolean wakUpSrcFound;
    uint8 i;
    uint8 canIfCtrlChnl;
    ret = E_OK;
    status = E_NOT_OK;

    /* @req 4.0.3/CANIF661 */
    /* @req 4.0.3/CANIF407 */
    VALIDATE_RV((TRUE == CanIf_Global.initRun), CANIF_CHECKVALIDATION_ID, CANIF_E_UNINIT,E_NOT_OK);

    /* @req 4.0.3/CANIF404 */
    /* Wakeup source is bit coded message only one unique bit is set to indicate a source, find the last in the list */
    lastWakUpSrcVal = (1UL<<(ECUM_WKSOURCE_USER_CNT+ECUM_WKSOURCE_SYSTEM_CNT-1));
    wakUpSrcFound = FALSE;

    for (wakUpSrc=1; wakUpSrc <=lastWakUpSrcVal ;wakUpSrc = (wakUpSrc<<1u))
    {
        if ((uint32)wakUpSrc == WakeupSource)
        {
            wakUpSrcFound = TRUE;
            break;
        }
    }
    VALIDATE_RV((wakUpSrcFound), CANIF_CHECKVALIDATION_ID, CANIF_E_PARAM_WAKEUPSOURCE,E_NOT_OK);

#if ( CANIF_TRCV_WAKEUP_SUPPORT == STD_ON )
    wakUpSrcFound = FALSE;
    for (i=0;i<CANIF_TRANSCEIVER_CHANNEL_CNT;i++)
    {
        if(CanIf_ConfigPtr->CanIfTransceiverConfig[i].CanIfTrcvCanTrcvWakeupSrc == WakeupSource)
        {
            wakUpSrcFound = TRUE;
            canIfCtrlChnl = CanIf_ConfigPtr->CanIfTransceiverConfig[i].CanIfTrcvChnlToCanIfCtrlChnlRef;
            break;
        }
    }

    if (!wakUpSrcFound)
    {
        status = E_OK; /* The API is accepted but no not notification available because no matching wake up source configured */
    }
    if (status == E_NOT_OK){
        /* @req 4.0.3/CANIF286 */
        /*lint --e{661} --e{644} */ /* If wakeup source is unidentified the following code is not executed */
        if ((CANTRCV_TRCVMODE_NORMAL == CanIf_Global.trcvData[i].trcvMode)
                && (CANIF_CS_STARTED == CanIf_Global.channelData[canIfCtrlChnl].ControllerMode))
        {

            if (CanIf_Global.channelData[canIfCtrlChnl].firstRxIndication)
            {
                /* @req 4.0.3/CANIF179 */
                temp = WakeupSource;
            } else{
                /* @req 4.0.3/CANIF681 */
                temp = 0;
            }
            if (CanIfDispatchConfig.CanIfWakeupValidNotification !=  NULL)
            {
                CanIfDispatchConfig.CanIfWakeupValidNotification(temp);
            }
            ret = E_OK;
        }
    } else {
        ret = status;
    }

#endif
#if( CANIF_CTRL_WAKEUP_SUPPORT == STD_ON )
#endif

    return ret;
}
#endif


/* !req 4.0.3/CANIF738 */
#if ( CANIF_PUBLIC_TXCONFIRM_POLLING_SUPPORT == STD_ON ) && FEATURE_NOT_SUPPORTED
/* !req 4.0.3/CANIF734 */
CanIf_NotifStatusType CanIf_GetTxConfirmationState( uint8 ControllerId )
{
    /* !req 4.0.3/CANIF736 */
    /* !req 4.0.3/CANIF737 */
    return CANIF_NO_NOTIFICATION;
}
#endif

/* @req 4.0.3/CANIF771 */
#if ( CANIF_PUBLIC_PN_SUPPORT == STD_ON )
/* @req 4.0.3/CANIF760 */
Std_ReturnType CanIf_ClearTrcvWufFlag( uint8 TransceiverId )
{
    uint8 trcvHwCnlId;

    /* @req 4.0.3/CANIF661 */
    VALIDATE_RV((TRUE == CanIf_Global.initRun), CANIF_CLEARTRCVWUFFLAG_ID, CANIF_E_UNINIT,E_NOT_OK);

    /* @req 4.0.3/CANIF769 */
    VALIDATE_RV((TransceiverId < CANIF_TRANSCEIVER_CHANNEL_CNT), CANIF_CLEARTRCVWUFFLAG_ID, CANIF_E_PARAM_TRCV,E_NOT_OK);

    /* Maps from CanIfTransceiverId to CanTransceiverId */
    trcvHwCnlId = ARC_GET_CAN_TRANSCEIVERID(TransceiverId);

    /* @req 4.0.3/CANIF766 */
    return CanTrcv_ClearTrcvWufFlag(trcvHwCnlId);
}
#endif

/* @req 4.0.3/CANIF813 */
#if ( CANIF_PUBLIC_PN_SUPPORT == STD_ON )
/* @req 4.0.3/CANIF761 */
Std_ReturnType CanIf_CheckTrcvWakeFlag( uint8 TransceiverId )
{
    uint8 trcvHwCnlId;

    /* @req 4.0.3/CANIF661 */
    VALIDATE_RV((TRUE == CanIf_Global.initRun), CANIF_CHECKTRCVWAKEFLAG_ID, CANIF_E_UNINIT,E_NOT_OK);

    /* @req 4.0.3/CANIF770 */
    VALIDATE_RV((TransceiverId < CANIF_TRANSCEIVER_CHANNEL_CNT), CANIF_CHECKTRCVWAKEFLAG_ID, CANIF_E_PARAM_TRCV,E_NOT_OK);

    /* Maps from CanIfTransceiverId to CanTransceiverId */
    trcvHwCnlId = ARC_GET_CAN_TRANSCEIVERID(TransceiverId);

    /* @req 4.0.3/CANIF765 */
    return CanTrcv_CheckWakeFlag(trcvHwCnlId);
}
#endif


/* @req 4.0.3/CANIF754 */
#if ( CANIF_PUBLIC_PN_SUPPORT == STD_ON )
/* @req 4.0.3/CANIF815 */
void CanIf_ConfirmPnAvailability( uint8 TransceiverId )
{
    uint8 Channel;

    /* @req 4.0.3/CANIF661 */
    /* @req 4.0.3/CANIF817 */
    /* @req 4.0.3/CANIF818 */
    VALIDATE_NO_RV((TRUE == CanIf_Global.initRun), CANIF_CONFIRM_PNAVAILABILITY_ID, CANIF_E_UNINIT);

    /* @req 4.0.3/CANIF816 */
    VALIDATE_NO_RV((E_OK == TrcvToChnlId(TransceiverId,&Channel)), CANIF_CONFIRM_PNAVAILABILITY_ID, CANIF_E_PARAM_TRCV);

    /* @req 4.0.3/CANIF753 */
    if (CanIfDispatchConfig.CanIfTrcvConfirmPnAvailabilityNotification != NULL)
    {
        CanIfDispatchConfig.CanIfTrcvConfirmPnAvailabilityNotification(Channel);
    }

    return;
}
#endif


/* @req 4.0.3/CANIF808 */
#if ( CANIF_PUBLIC_PN_SUPPORT == STD_ON )
/* @req 4.0.3/CANIF762 */
void CanIf_ClearTrcvWufFlagIndication( uint8 TransceiverId )
{
    uint8 Channel;

    /* @req 4.0.3/CANIF661 */
    /* @req 4.0.3/CANIF806 */
    /* @req 4.0.3/CANIF807 */
    VALIDATE_NO_RV((TRUE == CanIf_Global.initRun), CANIF_CLEARTRCV_WUFFLAG_INDICATION, CANIF_E_UNINIT);

    /* @req 4.0.3/CANIF805 */
    VALIDATE_NO_RV((E_OK == TrcvToChnlId(TransceiverId,&Channel)), CANIF_CLEARTRCV_WUFFLAG_INDICATION, CANIF_E_PARAM_TRCV);

    /* @req 4.0.3/CANIF757 */
    if (CanIfDispatchConfig.CanIfTrcvClearWakeFlagNotification != NULL)
    {
        CanIfDispatchConfig.CanIfTrcvClearWakeFlagNotification(Channel);
    }

    return;
}
#endif

/* @req 4.0.3/CANIF812 */
#if ( CANIF_PUBLIC_PN_SUPPORT == STD_ON )
/* @req 4.0.3/CANIF763 */
void CanIf_CheckTrcvWakeFlagIndication( uint8 TransceiverId )
{
    uint8 Channel;

    /* @req 4.0.3/CANIF661 */
    /* @req 4.0.3/CANIF810 */
    /* @req 4.0.3/CANIF811 */
    VALIDATE_NO_RV((TRUE == CanIf_Global.initRun), CANIF_CHECKTRCV_WAKEFLAG_INDICATION, CANIF_E_UNINIT);

    /* @req 4.0.3/CANIF809 */
    VALIDATE_NO_RV((E_OK == TrcvToChnlId(TransceiverId,&Channel)), CANIF_CHECKTRCV_WAKEFLAG_INDICATION, CANIF_E_PARAM_TRCV);

    /* @req 4.0.3/CANIF759 */
    if (CanIfDispatchConfig.CanIfTrcvWakeFlagNotification != NULL)
    {
      CanIfDispatchConfig.CanIfTrcvWakeFlagNotification(Channel);
    }
    return;
}
#endif



