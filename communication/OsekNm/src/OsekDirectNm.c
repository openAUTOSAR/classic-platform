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

#include"OsekNm.h"
#if (OSEKNM_DIRECT_NET_NUM > 0)
#include"OsekDirectNm.h"
#include "SchM_OsekNm.h"
#include <string.h>

/** @req OSEKNM037 */ /* NM message transmission shall be enabled */
/** @req OSEKNM038 */ /* NM message transmission shall be disabled */

#define NOT_SUPPORTED_YET  0
/* Macros for OpCode */
#define ALIVE_MASK     0x01
#define RING_MASK      0x02
#define LIMPHOME_MASK  0x04
#define SLEEPIND_MASK  0x10   /* sleep indication */
#define SLEEPACK_MASK  0x20   /* sleep acknowledgement */
#define SLEEP_ACK_SET  0x32 /* (RING_MASK|SLEEPIND_MASK|SLEEPACK_MASK)*/

#define TIMER_2        0x02 /* 2ticks extra added- 1 tick for setting and decrementing in same call and
                               one more tick because of checking time-out with 1 */
/** @req OSEKNM050 */
#define STATE_CHANGE_INDICATION(state)    \
           netId = OsekNm_Internal_GetNetId(netHandleId);  \
           OsekNm_ConfigPtr->osekNmDirectCallBack(netId, state ); \

extern const OsekNm_ConfigType* OsekNm_ConfigPtr;
static OsekDirect_InternalType OsekDirectNm_Networks[OSEKNM_DIRECT_NET_NUM];

/** Static function declarations */
static void initFromBusSleep(NetIdType netHandleId);
static void initFromNormal(NetIdType netHandleId);
static void initFromLimpHome(NetIdType netHandleId);
static void addToPresentConfig(NetIdType netHandleId,NodeIdType nodeId);
static void removeFromConfig(NetIdType netHandleId,ConfigKindName configKind,NodeIdType nodeId);
static void addToConfig(NetIdType netHandleId,ConfigKindName configKind,NodeIdType nodeId);
static void sendNmMessage(NetIdType netHandleId);
static void normalStandardNm(NetIdType netHandleId,uint8 rxBufIndex);
static void addToLimphomeConfig(NetIdType netHandleId,NodeIdType nodeId);
static NodeIdType determineLSnode(NodeIdType source,NodeIdType receiver,NodeIdType logsuccesor);
static boolean isNodeSkipped(NodeIdType source, NodeIdType receiver,NodeIdType destination);
static void normalMainFunc(NetIdType netHandleId);
static void normalPrepSleepMainFunc(NetIdType netHandleId);
static void normalTwbsMainFunc(NetIdType netHandleId);
static void twbsLimphomeMainFunc(NetIdType netHandleId);
static void limphomePrepSleepMainFunc(NetIdType netHandleId);
static void limphomeMainFunc(NetIdType netHandleId);
static void rxNmpduProcessFunc(NetIdType netHandleId, uint8 rxBufIndex);
static void rxNmpduLimpHomemsg(NetIdType netHandleId, uint8 rxBufIndex);
static void rxNmpduSleepAckProc(NetIdType netHandleId,uint8 rxBufIndex);
static void txNmpduProcessFunc(NetIdType netHandleId);
/**
 * @brief Leave LimpHome
 * @param netHandleId
 */
static void initFromLimpHome(NetIdType netHandleId){
    NetIdType netId;

    /* Cancel the timer TError and reset NMMerker.limphome */
    OsekDirectNm_Networks[netHandleId].timer.osekdirectNmTError = 0;
    OsekDirectNm_Networks[netHandleId].nmMerker.B.limphome = 0;

    /* Reset Tx and Rx Count */
    OsekDirectNm_Networks[netHandleId].nmTxRetryCounter = 0;
    OsekDirectNm_Networks[netHandleId].nmRxRetryCounter = 0;

    /* Get Actual netId and enable the communication */
    netId = OsekNm_Internal_GetNetId(netHandleId);
    D_Online(netId);

    initFromNormal(netHandleId);
}

/**
 * @brief Function to send alive/ring message
 * @param netHandleId
 */
static void sendNmMessage(NetIdType netHandleId) {
    StatusType status;
    NetIdType  netId;

    /* Get Actual netId and Start sending messages */
    netId = OsekNm_Internal_GetNetId(netHandleId);
    status = D_Window_Data_req(netId,&(OsekDirectNm_Networks[netHandleId].nmTransmitPdu),OSEKNM_DLL_FRAME_LENGTH);

    /* DDL rejected the command to send actual NM Message(ring or alive) */
    if(status != E_OK){
        /* re-Transmit after TTx */ /* Figure 39 - DLL transmit Rejection */
        OsekDirectNm_Networks[netHandleId].timer.osekdirectNmTTx = (OsekNm_ConfigPtr->osekNmNetwork[netHandleId].osekDirectNmNodeconfParams->osekdirectNmTTx + 1);
    }
}

/**
 * @brief Function to add Node to present config
 * @param netHandleId
 * @param configKind
 * @param nodeId
 */
static void addToConfig(NetIdType netHandleId,ConfigKindName configKind,NodeIdType nodeId){
    ConfigRefType configNode = NULL;
    uint8 byte;
    uint8 bit;

    switch(configKind){
        case NM_CKNORMAL:
            configNode = OsekDirectNm_Networks[netHandleId].nmDirectConfig.normal;
            break;
        case NM_CKLIMPHOME:
            configNode = OsekDirectNm_Networks[netHandleId].nmDirectConfig.limphome;
            break;
        default:
            break;
    }

    if(NULL != configNode)    {
        /* Set the corresponding bit in array to represent nodeId */
        byte = nodeId / 8;
        bit  = nodeId % 8;
        configNode[byte] |= (uint8)(1u<<bit);
    }
}

/**
 * @brief Function to add Node to the LimpHome config
 * @param netHandleId
 * @param nodeId
 */
static void addToLimphomeConfig(NetIdType netHandleId, NodeIdType nodeId) {
    uint8 refConfig[OSEKNM_NODE_MAX];
    StatusType status;

    /* Store old config for reference */
    memcpy(refConfig, OsekDirectNm_Networks[netHandleId].nmDirectConfig.limphome, OSEKNM_NODE_MAX);
    addToConfig(netHandleId, NM_CKLIMPHOME, nodeId);
    /* Normal state of node is reset */
    removeFromConfig(netHandleId, NM_CKNORMAL, nodeId);
    /* check is there any change in config compared last logical ring */
    status = OsekDirectNm_CmpConfig(netHandleId,OsekDirectNm_Networks[netHandleId].nmDirectConfig.limphome, refConfig, OsekDirectNm_Networks[netHandleId].nmDirectMask.limphome);

    if(status != E_OK) {
        /* Indicate Application about change in network config */
    }
}

/**
 * @brief Function call to remove node from configuration
 * @param netHandleId
 * @param configKind
 * @param nodeId
 */
static void removeFromConfig(NetIdType netHandleId,ConfigKindName configKind,NodeIdType nodeId) {
    ConfigRefType configNode = NULL;
    uint8 byte;
    uint8 bit;

    switch(configKind){
        case NM_CKNORMAL:
            configNode = OsekDirectNm_Networks[netHandleId].nmDirectConfig.normal;
            break;
        case NM_CKLIMPHOME:
            configNode = OsekDirectNm_Networks[netHandleId].nmDirectConfig.limphome;
            break;
        default:
            break;
    }

    if(NULL != configNode){
        /* Set the corresponding bit in array to represent nodeId */
        byte = nodeId / 8;
        bit  = nodeId % 8;
        configNode[byte] &= (~(1u<<bit));
    }
}

/**
 * @brief Function to add the node into present configuration
 * @param netHandleId
 * @param nodeId
 */
static void addToPresentConfig(NetIdType netHandleId, NodeIdType nodeId) {
    uint8 refConfig[OSEKNM_NODE_MAX];
    StatusType status;

    /* Store old configuration for reference */
    memcpy(refConfig, OsekDirectNm_Networks[netHandleId].nmDirectConfig.normal, OSEKNM_NODE_MAX);
    /* Add node id to present configuration */
    addToConfig(netHandleId, NM_CKNORMAL, nodeId);
    /* limphome state of node is reset */
    removeFromConfig(netHandleId, NM_CKLIMPHOME, nodeId);

    /* check is there any change in configuration compared last logical ring */
    status = OsekDirectNm_CmpConfig(netHandleId, OsekDirectNm_Networks[netHandleId].nmDirectConfig.normal, refConfig, OsekDirectNm_Networks[netHandleId].nmDirectMask.normal);
    if(status != E_OK) {
        /* Indicate Application about change in network configuration */
    }
}

/**
 * @brief Leave the NmBusSleep and enter into StartNM sequence
 * @param netHandleId
 */
static void initFromBusSleep(NetIdType netHandleId){
    NetIdType netId;
    /* config.limphome is set as Zero */
    memset(OsekDirectNm_Networks[netHandleId].nmDirectConfig.limphome, 0x0, OSEKNM_NODE_MAX);

    /* Reset the rx and tx counter */
    /** @req OSEKNM007 */
    OsekDirectNm_Networks[netHandleId].nmRxRetryCounter = 0x0;
    OsekDirectNm_Networks[netHandleId].nmTxRetryCounter = 0x0;

    /* Get actual netId and Enable application communication */
    netId = OsekNm_Internal_GetNetId(netHandleId);
    D_Online(netId);
    OsekDirectNm_Networks[netHandleId].nmState= OSEKNM_INITRESET;

    initFromNormal(netHandleId);
}

/**
 * @brief Leave the NmNormal state and enter into StartNM sequence
 * @param netHandleId
 */
/** @req OSEKNM006 */
static void initFromNormal(NetIdType netHandleId){
#if (STD_ON == OSEKNM_DIRECT_STATE_CHANGE_INDICATION)
    NetIdType netId;
#endif
    /* config.present = own station */
    memset(OsekDirectNm_Networks[netHandleId].nmDirectConfig.normal,0,OSEKNM_NODE_MAX);
    addToPresentConfig(netHandleId, OsekNm_ConfigPtr->osekNmNetwork[netHandleId].osekDirectNmNodeconfParams->nodeId);

    /* logical successor := own station */
    OsekDirectNm_Networks[netHandleId].nmTransmitPdu.destination = OsekNm_ConfigPtr->osekNmNetwork[netHandleId].osekDirectNmNodeconfParams->nodeId;
    /** @req OSEKNM007 */
    OsekDirectNm_Networks[netHandleId].nmRxRetryCounter += (uint8)0x01;

    /* Initialize the NMPDU data and opcode */
    OsekDirectNm_Networks[netHandleId].nmTransmitPdu.OpCode.b = 0;
    memset(OsekDirectNm_Networks[netHandleId].nmTransmitPdu.ringData, 0, sizeof(RingDataType));

    OsekDirectNm_Networks[netHandleId].timer.osekdirectNmTTyp = 0;
    OsekDirectNm_Networks[netHandleId].timer.osekdirectNmTTx = 0;
    OsekDirectNm_Networks[netHandleId].timer.osekdirectNmTMax = 0;
    OsekDirectNm_Networks[netHandleId].timer.osekdirectNmTError = 0;
    OsekDirectNm_Networks[netHandleId].timer.osekdirectNmTWbs = 0;
    if(1 == OsekDirectNm_Networks[netHandleId].networkStatus.nodeStatus.active) {
        /* Send A alive message */
        OsekDirectNm_Networks[netHandleId].nmTxRetryCounter += 1;
        OsekDirectNm_Networks[netHandleId].nmTransmitPdu.OpCode.b = ALIVE_MASK;
        sendNmMessage(netHandleId);
    }

    if((OsekDirectNm_Networks[netHandleId].nmRxRetryCounter <= OSEKNM_RX_COUNT_LIMIT) &&
            (OsekDirectNm_Networks[netHandleId].nmTxRetryCounter <=    OSEKNM_TX_COUNT_LIMIT)) {
        /* Set timer TTyp*/ /** @req OSEKNM009 */
        OsekDirectNm_Networks[netHandleId].timer.osekdirectNmTTyp = (OsekNm_ConfigPtr->osekNmNetwork[netHandleId].osekDirectNmNodeconfParams->osekdirectNmTTyp + 1);
        /* Enter Normal state */
        OsekDirectNm_Networks[netHandleId].nmState = OSEKNM_NORMAL;
#if (STD_ON == ASR_OSEK_NM)
        Nm_NetworkMode(OsekNmConfig.osekNmNetwork[netHandleId].osekNmNetworkHandle);
#endif
#if (STD_ON == OSEKNM_DIRECT_STATE_CHANGE_INDICATION)
        STATE_CHANGE_INDICATION(OSEKNM_NORMAL);
#endif
    }else {
        /** @req OSEKNM008 */
        /* Set timer TError */
        OsekDirectNm_Networks[netHandleId].timer.osekdirectNmTError = (OsekNm_ConfigPtr->osekNmNetwork[netHandleId].osekDirectNmNodeconfParams->osekdirectNmTError + 1);
        /* Enter limphome state */
        OsekDirectNm_Networks[netHandleId].nmState = OSEKNM_LIMPHOME;
#if (STD_ON == OSEKNM_DIRECT_STATE_CHANGE_INDICATION)
        STATE_CHANGE_INDICATION(OSEKNM_LIMPHOME);
#endif
        OsekDirectNm_Networks[netHandleId].networkStatus.nodeStatus.limpHome = 1;
    }
}

/**
 * @brief function find out the next logical successor
 * @param source
 * @param receiver
 * @param logsuccesor
 * @return
 */
static NodeIdType determineLSnode(NodeIdType source,NodeIdType receiver,NodeIdType logsuccesor) {
    NodeIdType nextLogsuccesor = logsuccesor;
    /** @req OSEKNM010 */
    /* Figure 19 in ISO spec */
    if(logsuccesor == receiver){
        nextLogsuccesor = source;
    }else {
        if(logsuccesor < receiver){
            if(source < logsuccesor){
                /* SLR */
                nextLogsuccesor = source;
            }else {
                if(source < receiver){
                    /* LSR */
                }else{
                    /* LRS */
                    nextLogsuccesor = source;
                }
            }
        }else {
            if(source < logsuccesor){
                if(source < receiver){
                    /* SRL */
                }else{
                    /* RSL */
                    nextLogsuccesor = source;
                }
            }else{
                /* RLS */
            }
        }
    }
    return nextLogsuccesor;
}

/**
 * @brief function to check is Node is skipped from ring
 * @param source
 * @param receiver
 * @param destination
 * @return
 */
/** @req OSEKNM011 */
static boolean isNodeSkipped(NodeIdType source, NodeIdType receiver,NodeIdType destination) {
    boolean isSkippedRing = FALSE;

    /* Figure 18 in ISO spec */
    if(destination < receiver){
        if(source < destination){
            /* not skipped SDR */
        }else{
            if(source < receiver){
                /* DRS */
                isSkippedRing = TRUE;
            }else{
                /* not skipped DSR */
            }
        }
    }else{
        if(source < destination){
            if(source < receiver){
                /* SRD */
                isSkippedRing = TRUE;
            }else{
                /* RSD */
            }
        }else{
            /* RDS */
            isSkippedRing = TRUE;
        }
    }
    return isSkippedRing;
}

/**
 * @brief Function to remember list of relevant configured nodes
 * @param netId
 * @param configKind
 * @param CMask
 */
void OsekDirectNm_InitCMaskTable(NetIdType netHandleId,ConfigKindName configKind, ConfigRefType cMask){
    switch(configKind){
        case NM_CKNORMAL:
            memcpy(OsekDirectNm_Networks[netHandleId].nmDirectMask.normal, cMask, OSEKNM_NODE_MAX);
            break;
        case NM_CKLIMPHOME:
            memcpy(OsekDirectNm_Networks[netHandleId].nmDirectMask.limphome, cMask, OSEKNM_NODE_MAX);
            break;
        default:
            break;
    }
}/*lint !e818 STANDARDIZED_API */

/**
 * @brief
 * @param netId
 * @param configKind
 * @param TargetConfig
 */
void Direct_InitTargetConfigTable(NetIdType netHandleId,ConfigKindName configKind,ConfigRefType targetConfig){
    switch(configKind){
        case NM_CKNORMAL:
            memcpy(OsekDirectNm_Networks[netHandleId].nmDirectConfig.normal,targetConfig,OSEKNM_NODE_MAX);
            break;
        case NM_CKLIMPHOME:
            memcpy(OsekDirectNm_Networks[netHandleId].nmDirectConfig.limphome,targetConfig,OSEKNM_NODE_MAX);
            break;
        default:
            break;
    }
}/*lint !e818 STANDARDIZED_API */

/**
 * @brief This service provides the actual configuration of the kind specified by <configKind>
 * @param netHandleId
 * @param config
 * @param configKind
 * @return
 */
/** @req OSEKNM043 */
StatusType OsekDirectNm_GetConfig(NetIdType netHandleId, ConfigRefType config,ConfigKindName configKind){

    switch(configKind){
        case NM_CKNORMAL:
            memcpy(config, OsekDirectNm_Networks[netHandleId].nmDirectConfig.normal, OSEKNM_NODE_MAX);
            break;
        case NM_CKLIMPHOME:
            memcpy(config, OsekDirectNm_Networks[netHandleId].nmDirectConfig.limphome, OSEKNM_NODE_MAX);
            break;
        default:
            break;
    }
    return E_OK;
}

/**
 * @brief The test configuration <TestConfig> is compared to the specified reference configuration
 * <RefConfig> taking account of the mask <CMask>.
 * @param netHandleId
 * @param TestConfig
 * @param RefConfig
 * @param CMask
 * @return
 */
/** @req OSEKNM044 */
StatusType OsekDirectNm_CmpConfig(NetIdType netHandleId, ConfigRefType testConfig, ConfigRefType refConfig,ConfigRefType cMask){

    StatusType retStatus = E_OK;
    uint8 result;
    uint8 i;
    (void)netHandleId;

    /* (<TestConfig> EXOR <RefConfig>) of the node within the network is identified within the <CMask> by TRUE. */
    for(i=0; i < OSEKNM_NODE_MAX; i++){
        result = testConfig[i]^refConfig[i];
        result &= cMask[i];
        result = ~result;
        if(result != 0xFFU){
            retStatus = E_NOT_OK;
            break;
        }
    }
    return retStatus;
}/*lint !e818 STANDARDIZED_API */

/**
 * @brief StartNM starts the local Network Management. This causes the state transition from NMOff to NMOn
 * @param netId
 * @return
 */
/** @req OSEKNM005 */
StatusType OsekDirectNm_StartNM(NetIdType netId){
    NetIdType netHandleId;
    StatusType retStatus = E_OK;
    uint8 config[OSEKNM_NODE_MAX];

    /* Figure 30 in ISO spec - Start Nm */
    /* Get NetHandle mapped to Actual netId */
    netHandleId = OsekNm_Internal_CheckNetId(netId);

    /* By default set NM status to active */
    OsekDirectNm_Networks[netHandleId].networkStatus.status = 0;
    OsekDirectNm_Networks[netHandleId].networkStatus.nodeStatus.active = 1;

    OsekDirectNm_Networks[netHandleId].nmTransmitPdu.OpCode.B.sleepInd = 0;
    OsekDirectNm_Networks[netHandleId].nmTransmitPdu.OpCode.B.sleepAck = 0;

    memset(config, 0x01 ,OSEKNM_NODE_MAX);
    OsekDirectNm_InitCMaskTable(netHandleId, NM_CKNORMAL, config);
    OsekDirectNm_InitCMaskTable(netHandleId, NM_CKLIMPHOME, config);
    OsekDirectNm_Networks[netHandleId].rcvdNmpdu = FALSE;
    OsekDirectNm_Networks[netHandleId].nmRxPduCount = 0;
#if (STD_OFF == ASR_OSEK_NM)
    /* Set State to NMInit */
    OsekDirectNm_Networks[netHandleId].nmState= OSEKNM_INIT;
    /* Initialize the hardware */
    D_Init(netId,BusInit);

    initFromBusSleep(netHandleId);
#else
    /* Set State to OSEKNM_BUSSLEEP to support ASR NM */
    OsekDirectNm_Networks[netHandleId].nmState= OSEKNM_BUSSLEEP;
    OsekDirectNm_Networks[netHandleId].nmMerker.b = 0;
#endif
    return retStatus;
}

/**
 * @brief StopNM stops the local Network Management. This causes the state transition from NMOn
 * to NMShutDown and after processing of the shutdown activities to NMOff
 * @param netId
 * @return
 */
/** @req OSEKNM040 */
StatusType OsekDirectNm_StopNM(NetIdType netId){

    uint8 netHandleId;

    /* Figure 30 in ISO spec - Stop NM */
    /* Get Mapped Network handle id to netId */
    netHandleId = OsekNm_Internal_CheckNetId(netId);
    /* Change the state to NMOff */
    OsekDirectNm_Networks[netHandleId].nmState = OSEKNM_OFF;
    OsekDirectNm_Networks[netHandleId].networkStatus.nodeStatus.nmOff = 1;
    return E_OK;
}

/**
 * @brief GotoMode serves to set the NM operating mode specified by <NewMode>
 * @param netId
 * @param NewMode
 * @return
 */
/** @req OSEKNM042 */
StatusType OsekDirectNm_GotoMode(NetIdType netHandleId,NMModeName newMode){
    NetIdType netId;

    if(newMode == NM_BUSSLEEP){
        /* Set status to bus sleep */
        OsekDirectNm_Networks[netHandleId].networkStatus.nodeStatus.bussleep = 1; /* Figure 39 */
        OsekDirectNm_Networks[netHandleId].networkStatus.nodeStatus.gotoModeSleep = 1;

        switch(OsekDirectNm_Networks[netHandleId].nmState){
            case OSEKNM_NORMAL: /* Figure 32 in ISO spec */ /** @req OSEKNM014 */
                if(0 == OsekDirectNm_Networks[netHandleId].networkStatus.nodeStatus.active) {
                    OsekDirectNm_Networks[netHandleId].nmState = OSEKNM_NORMALPREPSLEEP;
#if (STD_ON == OSEKNM_DIRECT_STATE_CHANGE_INDICATION)
                    STATE_CHANGE_INDICATION(OSEKNM_NORMALPREPSLEEP);
#endif
                }  /* else OSEKNM_NORMAL */
            break;

            default:
            break;
        }
    }else if(newMode == NM_AWAKE) {
        OsekDirectNm_Networks[netHandleId].networkStatus.nodeStatus.bussleep = 0; /* Figure 39 */
        OsekDirectNm_Networks[netHandleId].networkStatus.nodeStatus.gotoModeSleep = 0;
        switch(OsekDirectNm_Networks[netHandleId].nmState){
            case OSEKNM_NORMALPREPSLEEP:/* Figure 33 in ISO spec */ /** @req OSEKNM019 */
                OsekDirectNm_Networks[netHandleId].nmState = OSEKNM_NORMAL;
#if (STD_ON == ASR_OSEK_NM)
                Nm_NetworkMode(OsekNmConfig.osekNmNetwork[netHandleId].osekNmNetworkHandle);
#endif
#if (STD_ON == OSEKNM_DIRECT_STATE_CHANGE_INDICATION)
                STATE_CHANGE_INDICATION(OSEKNM_NORMAL);
#endif
            break;
            case OSEKNM_TwbsNORMAL: /* Figure 34 in ISO spec */
                /* Cancel timer TWbs */ /** @req OSEKNM022 */
                OsekDirectNm_Networks[netHandleId].timer.osekdirectNmTWbs = 0;
                OsekDirectNm_Networks[netHandleId].networkStatus.nodeStatus.twbsNormalLimp = 0;

                /* AF modified */
                /* Get Actual netId and enable the communication */
                netId = OsekNm_Internal_GetNetId(netHandleId);
                D_Online(netId);
                /* End AF modified */

                /* NmInitReset */
                initFromNormal(netHandleId);
            break;
            case OSEKNM_BUSSLEEP:/* Figure 31 in ISO spec */
                /* Get actual netId and Enable application communication */
                netId = OsekNm_Internal_GetNetId(netHandleId);
                D_Init(netId,BusAwake);
                initFromBusSleep(netHandleId);
            break;
            case OSEKNM_LIMPHOMEPREPSLEEP:/* Figure 36 in ISO spec */ /** @req OSEKNM031 */
                OsekDirectNm_Networks[netHandleId].nmState = OSEKNM_LIMPHOME;
                /* set timer TError */
                OsekDirectNm_Networks[netHandleId].timer.osekdirectNmTError = (OsekNm_ConfigPtr->osekNmNetwork[netHandleId].osekDirectNmNodeconfParams->osekdirectNmTError + 1);
#if (STD_ON == OSEKNM_DIRECT_STATE_CHANGE_INDICATION)
                STATE_CHANGE_INDICATION(OSEKNM_LIMPHOME);
#endif
            break;
            case OSEKNM_TwbsLIMPHOME:/* Figure 37 in ISO spec */ /** @req OSEKNM035 */
                /* Cancel timer TWbs */
                OsekDirectNm_Networks[netHandleId].timer.osekdirectNmTWbs = 0;
                OsekDirectNm_Networks[netHandleId].networkStatus.nodeStatus.twbsNormalLimp = 0;
                OsekDirectNm_Networks[netHandleId].networkStatus.nodeStatus.limpHome = 1;
                OsekDirectNm_Networks[netHandleId].nmState = OSEKNM_LIMPHOME;
                /* set timer TError */
                OsekDirectNm_Networks[netHandleId].timer.osekdirectNmTError = (OsekNm_ConfigPtr->osekNmNetwork[netHandleId].osekDirectNmNodeconfParams->osekdirectNmTError + 1);
#if (STD_ON == OSEKNM_DIRECT_STATE_CHANGE_INDICATION)
                STATE_CHANGE_INDICATION(OSEKNM_LIMPHOME);
#endif
            break;
            default:
                break;
        }
    }else {}
    return E_OK;
}

/**
 * @brief Function to check status of self Node by application
 * @param netId
 * @param networkStatus
 * @return
 */
/*lint --e{9018}*/ /* 'networkStatus' with union based type 'StatusRefType' */
StatusType OsekDirectNm_GetStatus(NetIdType netHandleId, StatusRefType networkStatus) {
    /** @req OSEKNM045 */
    StatusType status = E_OK;

    memcpy(networkStatus, &OsekDirectNm_Networks[netHandleId].networkStatus, sizeof(NetworkStatusType));
    return status;
}


/**
 * @brief Function call to perform actions in NmNormalStandard
 * @param netHandleId Handle identifier for network
 * @param rxBufIndex  index of the rx buffer applicable
 */
static void normalStandardNm(NetIdType netHandleId, uint8 rxBufIndex ) {
    uint8 newDestination;
    boolean isSkippedRing;

    /* Figure 38 in ISO doc */
    /* Clear NMRxcount */ /** @req OSEKNM012 */
    OsekDirectNm_Networks[netHandleId].nmRxRetryCounter = 0;
    if(1 == OsekDirectNm_Networks[netHandleId].nmRxPdu[rxBufIndex].OpCode.B.limphome) {

        /* add sender to config.limphome */
        addToLimphomeConfig(netHandleId, OsekDirectNm_Networks[netHandleId].nmRxPdu[rxBufIndex].source);
    }else{
        /* add sender to config.present */
        addToPresentConfig(netHandleId, OsekDirectNm_Networks[netHandleId].nmRxPdu[rxBufIndex].source);

        /* determine logical successor */ /** @req OSEKNM010 */
        newDestination =  determineLSnode(OsekDirectNm_Networks[netHandleId].nmRxPdu[rxBufIndex].source,      \
                OsekNm_ConfigPtr->osekNmNetwork[netHandleId].osekDirectNmNodeconfParams->nodeId, \
                OsekDirectNm_Networks[netHandleId].nmTransmitPdu.destination);

        OsekDirectNm_Networks[netHandleId].nmTransmitPdu.destination = newDestination;

        if(1 == OsekDirectNm_Networks[netHandleId].nmRxPdu[rxBufIndex].OpCode.B.ring){
            /* Cancle timer TTyp and TMax */
            OsekDirectNm_Networks[netHandleId].timer.osekdirectNmTTyp = 0; /** @req OSEKNM009 */
            OsekDirectNm_Networks[netHandleId].timer.osekdirectNmTMax = 0;

            /* transfer application data via the logical ring */
            memcpy(OsekDirectNm_Networks[netHandleId].nmTransmitPdu.ringData, OsekDirectNm_Networks[netHandleId].nmRxPdu[rxBufIndex].ringData, sizeof(RingDataType));

            /* check destination station to me or destination = source */
            if((OsekDirectNm_Networks[netHandleId].nmRxPdu[rxBufIndex].destination == OsekNm_ConfigPtr->osekNmNetwork[netHandleId].osekDirectNmNodeconfParams->nodeId)
                            || (OsekDirectNm_Networks[netHandleId].nmRxPdu[rxBufIndex].destination == OsekDirectNm_Networks[netHandleId].nmRxPdu[rxBufIndex].source)) {
                /* Set timer TTyp */ /** @req OSEKNM009 */
                OsekDirectNm_Networks[netHandleId].timer.osekdirectNmTTyp = (OsekNm_ConfigPtr->osekNmNetwork[netHandleId].osekDirectNmNodeconfParams->osekdirectNmTTyp + 1);
                /* ringData indication to application */
                OsekDirectNm_Networks[netHandleId].networkStatus.nodeStatus.ringdataNotallowed = 0;

            }else {
                /* Set timer TMax */
                OsekDirectNm_Networks[netHandleId].timer.osekdirectNmTMax = (OsekNm_ConfigPtr->osekNmNetwork[netHandleId].osekDirectNmNodeconfParams->osekdirectNmTMax + 1);
                isSkippedRing = isNodeSkipped(OsekDirectNm_Networks[netHandleId].nmRxPdu[rxBufIndex].source, OsekNm_ConfigPtr->osekNmNetwork[netHandleId].osekDirectNmNodeconfParams->nodeId, OsekDirectNm_Networks[netHandleId].nmRxPdu[rxBufIndex].destination);
                if(TRUE == isSkippedRing) {
                    if(1 == OsekDirectNm_Networks[netHandleId].networkStatus.nodeStatus.active) {
                        OsekDirectNm_Networks[netHandleId].nmTransmitPdu.OpCode.b = ALIVE_MASK;
                        OsekDirectNm_Networks[netHandleId].nmTransmitPdu.destination = OsekNm_ConfigPtr->osekNmNetwork[netHandleId].osekDirectNmNodeconfParams->nodeId;

                        if(1 == OsekDirectNm_Networks[netHandleId].networkStatus.nodeStatus.bussleep){
                            OsekDirectNm_Networks[netHandleId].nmTransmitPdu.OpCode.B.sleepInd = 1;
                        }
                        /* Send message */ /** @req OSEKNM011 */
                        sendNmMessage(netHandleId);
                    }
                }
            }

        }else {
            OsekDirectNm_Networks[netHandleId].nmMerker.B.stable = 0;
            OsekDirectNm_Networks[netHandleId].networkStatus.nodeStatus.configurationstable = 0;
        }
    }
}

/**
 * @brief Actions to perform when TError elapsed In LimpHome state
 * @param netHandleId
 */
static void limphomeMainFunc(NetIdType netHandleId) {
    NetIdType netId;

    /* Figure 35 in ISO doc - Time-out TError part */
    /* timer started */
    if(0 != OsekDirectNm_Networks[netHandleId].timer.osekdirectNmTError) {
        /* Decrement timer */
        if(OsekDirectNm_Networks[netHandleId].timer.osekdirectNmTError > 1) {
            OsekDirectNm_Networks[netHandleId].timer.osekdirectNmTError--;
        }
        /* Check timeout */ /** @req OSEKNM024 */
        if(1 == OsekDirectNm_Networks[netHandleId].timer.osekdirectNmTError) {
            /* cancel timer TMax */
            OsekDirectNm_Networks[netHandleId].timer.osekdirectNmTError = 0;

            /* Get Actual netId and Enable communication */
            netId = OsekNm_Internal_GetNetId(netHandleId);
            D_Online(netId);

            OsekDirectNm_Networks[netHandleId].nmTransmitPdu.OpCode.b = LIMPHOME_MASK;
            if(1 == OsekDirectNm_Networks[netHandleId].networkStatus.nodeStatus.bussleep) {
                OsekDirectNm_Networks[netHandleId].timer.osekdirectNmTMax = (OsekNm_ConfigPtr->osekNmNetwork[netHandleId].osekDirectNmNodeconfParams->osekdirectNmTMax + 1);
                OsekDirectNm_Networks[netHandleId].nmTransmitPdu.OpCode.B.sleepInd = 1;
                OsekDirectNm_Networks[netHandleId].nmState = OSEKNM_LIMPHOMEPREPSLEEP; /** @req OSEKNM025 */
#if (STD_ON == OSEKNM_DIRECT_STATE_CHANGE_INDICATION)
                STATE_CHANGE_INDICATION(OSEKNM_LIMPHOMEPREPSLEEP);
#endif
            }else {
                OsekDirectNm_Networks[netHandleId].timer.osekdirectNmTError = (OsekNm_ConfigPtr->osekNmNetwork[netHandleId].osekDirectNmNodeconfParams->osekdirectNmTError + 1);
            }

            if(1 == OsekDirectNm_Networks[netHandleId].networkStatus.nodeStatus.active) {
                sendNmMessage(netHandleId);
            }
        }
    }
}

/**
 * @brief Actions to perform when TWbs elapsed In TwbsLimpHome state
 * @param netHandleId
 */
static void twbsLimphomeMainFunc(NetIdType netHandleId) {
    NetIdType netId;

    /* Figure 37 in ISO doc - Time-out Twbs */
    /* timer started */
    if(0 != OsekDirectNm_Networks[netHandleId].timer.osekdirectNmTWbs) {
        /* Decrement timer */
        if(OsekDirectNm_Networks[netHandleId].timer.osekdirectNmTWbs > 1) {
            OsekDirectNm_Networks[netHandleId].timer.osekdirectNmTWbs--;
        }
        /* Check timeout */
        if(1 == OsekDirectNm_Networks[netHandleId].timer.osekdirectNmTWbs) {
            /* cancel timer TMax */
            OsekDirectNm_Networks[netHandleId].timer.osekdirectNmTWbs = 0;
            /* Get Actual netId and Initialize the sleep mode of the bus hardware */
            netId = OsekNm_Internal_GetNetId(netHandleId);
            D_Init(netId, BusSleep);
            /* Set State to NMBussleep */
            OsekDirectNm_Networks[netHandleId].nmState = OSEKNM_BUSSLEEP; /** @req OSEKNM034 */
            OsekDirectNm_Networks[netHandleId].networkStatus.nodeStatus.twbsNormalLimp = 0;
#if (ASR_OSEK_NM == STD_ON)
            Nm_BusSleepMode(OsekNmConfig.osekNmNetwork[netHandleId].osekNmNetworkHandle);
#endif
#if (STD_ON == OSEKNM_DIRECT_STATE_CHANGE_INDICATION)
            STATE_CHANGE_INDICATION(OSEKNM_BUSSLEEP);
#endif
        }
    }
}

/**
 * @brief Actions to perform when TMax elapsed In LimphomePrepSleep state
 * @param netHandleId
 */
static void limphomePrepSleepMainFunc(NetIdType netHandleId){
    NetIdType netId;

    /* Figure 36 in ISO doc - Time-out Tmax */
    /* timer started */
    if(0 != OsekDirectNm_Networks[netHandleId].timer.osekdirectNmTMax) {
        /* Decrement timer */
        if(OsekDirectNm_Networks[netHandleId].timer.osekdirectNmTMax > 1) {
            OsekDirectNm_Networks[netHandleId].timer.osekdirectNmTMax--;
        }
        /* Check timeout */
        if(1 == OsekDirectNm_Networks[netHandleId].timer.osekdirectNmTMax) {
            /* cancel timer TMax */
            OsekDirectNm_Networks[netHandleId].timer.osekdirectNmTMax = 0;
            /* Get Actual netId and Disable communication */
            netId = OsekNm_Internal_GetNetId(netHandleId);
            D_Offline(netId);

            /* Set timer Twbs */
            OsekDirectNm_Networks[netHandleId].timer.osekdirectNmTWbs = (OsekNm_ConfigPtr->osekNmNetwork[netHandleId].osekDirectNmNodeconfParams->osekdirectNmTWbs + 1);
            /* Set State to NM_TwbsLIMPHOME */
            OsekDirectNm_Networks[netHandleId].nmState = OSEKNM_TwbsLIMPHOME; /** @req OSEKNM032 */
            OsekDirectNm_Networks[netHandleId].networkStatus.nodeStatus.limpHome = 0;
#if (STD_ON == OSEKNM_DIRECT_STATE_CHANGE_INDICATION)
            STATE_CHANGE_INDICATION(OSEKNM_TwbsLIMPHOME);
#endif
        }
    }
}

/**
 * @brief Actions to perform when TWbs elapsed In NormalTwbs state
 * @param netHandleId
 */
static void normalTwbsMainFunc(NetIdType netHandleId) {
    NetIdType netId;

    /* Figure 34 in ISO doc - Time-out Twbs part */
    /* timer started */
    if(0 != OsekDirectNm_Networks[netHandleId].timer.osekdirectNmTWbs) {
        /* Decrement timer */
        if(OsekDirectNm_Networks[netHandleId].timer.osekdirectNmTWbs > 1) {
            OsekDirectNm_Networks[netHandleId].timer.osekdirectNmTWbs--;
        }
        /* Check timeout */
        if(1 == OsekDirectNm_Networks[netHandleId].timer.osekdirectNmTWbs) {
            /* cancel timer TWbs */
            OsekDirectNm_Networks[netHandleId].timer.osekdirectNmTWbs = 0;
            OsekDirectNm_Networks[netHandleId].networkStatus.nodeStatus.twbsNormalLimp = 0;
            /* Get Actual netId and Initialize the sleep mode of the bus hardware */
            netId = OsekNm_Internal_GetNetId(netHandleId);
            D_Init(netId, BusSleep);
            OsekDirectNm_Networks[netHandleId].nmState = OSEKNM_BUSSLEEP; /** @req OSEKNM020 */
#if (ASR_OSEK_NM == STD_ON)
            Nm_BusSleepMode(OsekNmConfig.osekNmNetwork[netHandleId].osekNmNetworkHandle);
#endif
#if (STD_ON == OSEKNM_DIRECT_STATE_CHANGE_INDICATION)
            STATE_CHANGE_INDICATION(OSEKNM_BUSSLEEP);
#endif
        }
    }
}

/**
 * @brief Actions to perform when TTyp elapsed In NormalPrepSleep state
 * @param netHandleId
 */
static void normalPrepSleepMainFunc(NetIdType netHandleId) {
    NetIdType netId;
    /* Figure 33 in ISO doc - Timeout Typ part */
    /* timer started */
    if(0 != OsekDirectNm_Networks[netHandleId].timer.osekdirectNmTTyp) {
        /* Decrement timer */
        if(OsekDirectNm_Networks[netHandleId].timer.osekdirectNmTTyp > 1) {
            OsekDirectNm_Networks[netHandleId].timer.osekdirectNmTTyp--;
        }
        /* Check timeout */
        if(1 == OsekDirectNm_Networks[netHandleId].timer.osekdirectNmTTyp) {
            /* cancel timer TTyp */
            OsekDirectNm_Networks[netHandleId].timer.osekdirectNmTTyp = 0;

            if(1 == OsekDirectNm_Networks[netHandleId].networkStatus.nodeStatus.active)
            {
                /* Send ring message with set sleep.ack bit */ /** @req OSEKNM018 */
                OsekDirectNm_Networks[netHandleId].nmTransmitPdu.OpCode.b = SLEEP_ACK_SET;
                sendNmMessage(netHandleId);
            }
        }
    }

    /* Deviation:- If other nodes stop transmitting message when Node is in NMNormalPresleep , TTyp timer wont set and if GotoMode(Awake) is not called,
     * Node Stuck into NMNormalPresleep, To avoid this situation implemented below functionality */
    /* TMax timer started */
    if(0 != OsekDirectNm_Networks[netHandleId].timer.osekdirectNmTMax) {
        /* Decrement timer TMax*/
        if(OsekDirectNm_Networks[netHandleId].timer.osekdirectNmTMax > 1) {
            OsekDirectNm_Networks[netHandleId].timer.osekdirectNmTMax--;
        }

        /* Check timeout of TMax*/
        if(1 == OsekDirectNm_Networks[netHandleId].timer.osekdirectNmTMax) {
            /* cancel timer TMax */
            OsekDirectNm_Networks[netHandleId].timer.osekdirectNmTMax = 0;
            /* Get Actual netId and Disable communication */
            netId = OsekNm_Internal_GetNetId(netHandleId);
            D_Offline(netId);

            /* Set timer Twbs */
            OsekDirectNm_Networks[netHandleId].timer.osekdirectNmTWbs = (OsekNm_ConfigPtr->osekNmNetwork[netHandleId].osekDirectNmNodeconfParams->osekdirectNmTWbs + 1);
            /* Set State to NM_TwbsNORMAL */
            OsekDirectNm_Networks[netHandleId].nmState = OSEKNM_TwbsNORMAL;
            OsekDirectNm_Networks[netHandleId].networkStatus.nodeStatus.limpHome = 0;
#if (STD_ON == OSEKNM_DIRECT_STATE_CHANGE_INDICATION)
            STATE_CHANGE_INDICATION(OSEKNM_TwbsNORMAL);
#endif
        }
    }
}

/**
 * @brief Actions to perform when TTyp elapsed In NormalMain state
 * @param netHandleId
 */
static void normalMainFunc(NetIdType netHandleId) {
#if (STD_ON == OSEKNM_DIRECT_STATE_CHANGE_INDICATION)
    NetIdType  netId;
#endif
    /* Figure 32 in ISO doc - Time */
    /* timer started */
    if(0 != OsekDirectNm_Networks[netHandleId].timer.osekdirectNmTTyp) {
        /* Decrement timer */
        if(OsekDirectNm_Networks[netHandleId].timer.osekdirectNmTTyp > 1) {
            OsekDirectNm_Networks[netHandleId].timer.osekdirectNmTTyp--;
        }
        /* Check timeout */
        if(1 == OsekDirectNm_Networks[netHandleId].timer.osekdirectNmTTyp) {
            /* cancel timer TTyp and TMax */
            OsekDirectNm_Networks[netHandleId].timer.osekdirectNmTTyp = 0;
            OsekDirectNm_Networks[netHandleId].timer.osekdirectNmTMax = 0;
            /* Set timer TMax*/
            OsekDirectNm_Networks[netHandleId].timer.osekdirectNmTMax = (OsekNm_ConfigPtr->osekNmNetwork[netHandleId].osekDirectNmNodeconfParams->osekdirectNmTMax + TIMER_2);

            if(1 == OsekDirectNm_Networks[netHandleId].networkStatus.nodeStatus.active) {
                OsekDirectNm_Networks[netHandleId].nmTransmitPdu.OpCode.b = RING_MASK;
                if(1 == OsekDirectNm_Networks[netHandleId].networkStatus.nodeStatus.bussleep){
                    OsekDirectNm_Networks[netHandleId].nmTransmitPdu.OpCode.B.sleepInd = 1;
                }
                sendNmMessage(netHandleId);
                OsekDirectNm_Networks[netHandleId].nmTxRetryCounter++;
                OsekDirectNm_Networks[netHandleId].networkStatus.nodeStatus.ringdataNotallowed = 1;
            }

            if(OsekDirectNm_Networks[netHandleId].nmTxRetryCounter > OSEKNM_TX_COUNT_LIMIT) {
                /* Enter state NMLimpHome */
                OsekDirectNm_Networks[netHandleId].nmState = OSEKNM_LIMPHOME;
#if (STD_ON == OSEKNM_DIRECT_STATE_CHANGE_INDICATION)
                STATE_CHANGE_INDICATION(OSEKNM_LIMPHOME);
#endif
                OsekDirectNm_Networks[netHandleId].networkStatus.nodeStatus.limpHome =  1;
                OsekDirectNm_Networks[netHandleId].timer.osekdirectNmTError = (OsekNm_ConfigPtr->osekNmNetwork[netHandleId].osekDirectNmNodeconfParams->osekdirectNmTError + 1);
            }else {
                if(1 == OsekDirectNm_Networks[netHandleId].nmMerker.B.stable) {
                    OsekDirectNm_Networks[netHandleId].networkStatus.nodeStatus.configurationstable = 1;
                }else {
                    OsekDirectNm_Networks[netHandleId].nmMerker.B.stable = 1;
                }
            }
        }
    }

    /* Figure 32 in ISO doc - Time-Out Tmax */
    /* TMax timer started */
    if(0 != OsekDirectNm_Networks[netHandleId].timer.osekdirectNmTMax) {
        /* Decrement timer TMax*/
        if(OsekDirectNm_Networks[netHandleId].timer.osekdirectNmTMax > 1) {
            OsekDirectNm_Networks[netHandleId].timer.osekdirectNmTMax--;
        }

        /* Check timeout of TMax*/
        if(1 == OsekDirectNm_Networks[netHandleId].timer.osekdirectNmTMax) {
            /* cancel timer TMax */ /** @req OSEKNM013 */
            OsekDirectNm_Networks[netHandleId].timer.osekdirectNmTMax = 0;
            initFromNormal(netHandleId);
        }
    }
}

/**
 * @brief Function is to enable passive communication
 * @param netId
 * @return
 */
/** @req OSEKNM046 */
StatusType OsekDirectNm_SilentNM(NetIdType netId){
    uint8 netHandleId;
    /* Get Mapped Network handle id to netId */
    netHandleId = OsekNm_Internal_CheckNetId(netId);

    /* Figure 31 in ISO doc */
    /* Change the N/w status to passive */
    OsekDirectNm_Networks[netHandleId].networkStatus.nodeStatus.active = 0;
    return E_OK;
}

/**
 * @brief function is to disable passive communication
 * @param netId
 * @return
 */
/** @req OSEKNM047 */
StatusType OsekDirectNm_TalkNM(NetIdType netId){
    uint8 netHandleId;
    /* Get Mapped Network handle id to netId */
    netHandleId = OsekNm_Internal_CheckNetId(netId);

    /* Figure 31 in ISO doc */
    /* Set N/W status to Active */
    OsekDirectNm_Networks[netHandleId].networkStatus.nodeStatus.active = 1;
    return E_OK;
}

/**
 * @brief Function to process the transmitted message confirmation
 * @param netHandleId
 */
static void txNmpduProcessFunc(NetIdType netHandleId) {
    NetIdType netId;

    OsekDirectNm_Networks[netHandleId].nmTxRetryCounter = 0;
    switch(OsekDirectNm_Networks[netHandleId].nmState)
    {
        case OSEKNM_NORMAL: /* Figure 32 in ISO doc - Transmit message part */
            if(1 == OsekDirectNm_Networks[netHandleId].nmTransmitPdu.OpCode.B.ring) {
                /* Cancel timer TTyp and TMax */
                OsekDirectNm_Networks[netHandleId].timer.osekdirectNmTTyp = 0;
                OsekDirectNm_Networks[netHandleId].timer.osekdirectNmTMax = 0;
                /* Set timer TMax */
                OsekDirectNm_Networks[netHandleId].timer.osekdirectNmTMax = (OsekNm_ConfigPtr->osekNmNetwork[netHandleId].osekDirectNmNodeconfParams->osekdirectNmTMax + 1);

                if(1 == OsekDirectNm_Networks[netHandleId].nmTransmitPdu.OpCode.B.sleepInd){
                    if(1 == OsekDirectNm_Networks[netHandleId].networkStatus.nodeStatus.bussleep){
                        OsekDirectNm_Networks[netHandleId].nmTransmitPdu.OpCode.B.sleepAck = 1;
                        OsekDirectNm_Networks[netHandleId].nmState = OSEKNM_NORMALPREPSLEEP; /** @req OSEKNM016 */
#if (STD_ON == OSEKNM_DIRECT_STATE_CHANGE_INDICATION)
                        STATE_CHANGE_INDICATION(OSEKNM_NORMALPREPSLEEP);
#endif
                    }
                }
            }
            break;

        case OSEKNM_NORMALPREPSLEEP: /* Figure 33 in ISO doc - Transmit message part */
            if(1 == OsekDirectNm_Networks[netHandleId].nmTransmitPdu.OpCode.B.ring) {
                netId = OsekNm_Internal_GetNetId(netHandleId);
                D_Offline(netId);
                OsekDirectNm_Networks[netHandleId].timer.osekdirectNmTWbs = (OsekNm_ConfigPtr->osekNmNetwork[netHandleId].osekDirectNmNodeconfParams->osekdirectNmTWbs + 1);
                OsekDirectNm_Networks[netHandleId].nmState = OSEKNM_TwbsNORMAL;
                OsekDirectNm_Networks[netHandleId].networkStatus.nodeStatus.twbsNormalLimp = 1;
#if (STD_ON == OSEKNM_DIRECT_STATE_CHANGE_INDICATION)
                STATE_CHANGE_INDICATION(OSEKNM_TwbsNORMAL);
#endif
            }
            break;

        case OSEKNM_LIMPHOME:/* Figure 35 in ISO doc - Transmit message part */
            if(1 == OsekDirectNm_Networks[netHandleId].nmTransmitPdu.OpCode.B.limphome) {
                OsekDirectNm_Networks[netHandleId].nmMerker.B.limphome = 1;
            }
            break;

        default:
            break;
    }
}
/**
 * @brief Function to process the received limpHome message
 * @param netHandleId Network HandleId
 * @param rxBufIndex index of rx buffer
 */
static void rxNmpduLimpHomemsg(NetIdType netHandleId, uint8 rxBufIndex) {
    NetIdType netId;
    if((1 == OsekDirectNm_Networks[netHandleId].networkStatus.nodeStatus.bussleep) && (1 == OsekDirectNm_Networks[netHandleId].nmRxPdu[rxBufIndex].OpCode.B.sleepAck)) {
        netId = OsekNm_Internal_GetNetId(netHandleId);
        D_Offline(netId);
        OsekDirectNm_Networks[netHandleId].timer.osekdirectNmTWbs = (OsekNm_ConfigPtr->osekNmNetwork[netHandleId].osekDirectNmNodeconfParams->osekdirectNmTWbs + 1);
        OsekDirectNm_Networks[netHandleId].nmState = OSEKNM_TwbsLIMPHOME;
        OsekDirectNm_Networks[netHandleId].networkStatus.nodeStatus.limpHome = 0;
        OsekDirectNm_Networks[netHandleId].networkStatus.nodeStatus.twbsNormalLimp = 1;
#if (STD_ON == OSEKNM_DIRECT_STATE_CHANGE_INDICATION)
        STATE_CHANGE_INDICATION(OSEKNM_TwbsLIMPHOME);
#endif
    }else {
        initFromLimpHome(netHandleId);
    }
}

/**
 * @brief Function to process the received message with sleep acknowledgment
 * @param netHandleId Network HandleId
 * @param rxBufIndex index of rx buffer
 */
static void rxNmpduSleepAckProc(NetIdType netHandleId,uint8 rxBufIndex) {
    NetIdType netId;
    if(1 == OsekDirectNm_Networks[netHandleId].nmRxPdu[rxBufIndex].OpCode.B.sleepAck) {
        if(1 == OsekDirectNm_Networks[netHandleId].networkStatus.nodeStatus.bussleep) {
            /* NMInitBusSleep */
            netId = OsekNm_Internal_GetNetId(netHandleId);
            D_Offline(netId);
            OsekDirectNm_Networks[netHandleId].timer.osekdirectNmTWbs = (OsekNm_ConfigPtr->osekNmNetwork[netHandleId].osekDirectNmNodeconfParams->osekdirectNmTWbs + 1);
            OsekDirectNm_Networks[netHandleId].nmState = OSEKNM_TwbsNORMAL;
            OsekDirectNm_Networks[netHandleId].networkStatus.nodeStatus.twbsNormalLimp = 1;
#if (STD_ON == OSEKNM_DIRECT_STATE_CHANGE_INDICATION)
            STATE_CHANGE_INDICATION(OSEKNM_TwbsNORMAL);
#endif
        }
    }
}
/**
 * @brief Function to process the received message
 * @param netHandleId Network HandleId
 * @param rxBufIndex index of rx buffer
 */
static void rxNmpduProcessFunc(NetIdType netHandleId, uint8 rxBufIndex) {
    NetIdType netId;
    switch(OsekDirectNm_Networks[netHandleId].nmState){
        case OSEKNM_NORMAL: /* Figure 32 in ISO doc */
        case OSEKNM_NORMALPREPSLEEP:
            /* figure 38 in ISO doc */
            normalStandardNm(netHandleId,rxBufIndex);
            rxNmpduSleepAckProc(netHandleId,rxBufIndex);
            /* Figure33 - process for NM_stNormalPrepSleep only */
            if(OSEKNM_NORMALPREPSLEEP == OsekDirectNm_Networks[netHandleId].nmState){
                if(0 == OsekDirectNm_Networks[netHandleId].nmRxPdu[rxBufIndex].OpCode.B.sleepInd){
                    OsekDirectNm_Networks[netHandleId].nmState = OSEKNM_NORMAL; /** @req OSEKNM017 */
#if (STD_ON == ASR_OSEK_NM)
                    Nm_NetworkMode(OsekNmConfig.osekNmNetwork[netHandleId].osekNmNetworkHandle);
#endif
#if (STD_ON == OSEKNM_DIRECT_STATE_CHANGE_INDICATION)
                    STATE_CHANGE_INDICATION(OSEKNM_NORMAL);
#endif
                }/* Else stay in NormalPresleep */
            }
        break;
        case OSEKNM_LIMPHOME: /* Figure 35 - Received msg part */
            if(1 == OsekDirectNm_Networks[netHandleId].networkStatus.nodeStatus.active) {
                if(1 == OsekDirectNm_Networks[netHandleId].nmMerker.B.limphome) { /** @req OSEKNM029 */
                    rxNmpduLimpHomemsg(netHandleId,rxBufIndex);/** @req OSEKNM027 */ /** @req OSEKNM030 */
                }else {
                    if(1 == OsekDirectNm_Networks[netHandleId].nmRxPdu[rxBufIndex].OpCode.B.sleepAck) {
                        netId = OsekNm_Internal_GetNetId(netHandleId);
                        D_Offline(netId);
                        OsekDirectNm_Networks[netHandleId].timer.osekdirectNmTWbs = (OsekNm_ConfigPtr->osekNmNetwork[netHandleId].osekDirectNmNodeconfParams->osekdirectNmTWbs + 1);
                        OsekDirectNm_Networks[netHandleId].nmState = OSEKNM_TwbsLIMPHOME; /** @req OSEKNM028 */
                        OsekDirectNm_Networks[netHandleId].networkStatus.nodeStatus.limpHome = 0;
                        OsekDirectNm_Networks[netHandleId].networkStatus.nodeStatus.twbsNormalLimp = 1;
#if (STD_ON == OSEKNM_DIRECT_STATE_CHANGE_INDICATION)
                        STATE_CHANGE_INDICATION(OSEKNM_TwbsLIMPHOME);
#endif
                    }else {
                        initFromLimpHome(netHandleId); /** @req OSEKNM026 */
                    }
                }
            }else {
                rxNmpduLimpHomemsg(netHandleId, rxBufIndex);
            }
            break;
        case OSEKNM_LIMPHOMEPREPSLEEP: /* Figure 36 - Received msg part */ /** @req OSEKNM033 */
            if(0 == OsekDirectNm_Networks[netHandleId].nmRxPdu[rxBufIndex].OpCode.B.sleepInd) {
                OsekDirectNm_Networks[netHandleId].nmState = OSEKNM_LIMPHOME;
                OsekDirectNm_Networks[netHandleId].timer.osekdirectNmTError = (OsekNm_ConfigPtr->osekNmNetwork[netHandleId].osekDirectNmNodeconfParams->osekdirectNmTError + 1);
#if (STD_ON == OSEKNM_DIRECT_STATE_CHANGE_INDICATION)
                STATE_CHANGE_INDICATION(OSEKNM_LIMPHOME);
#endif
            }/* Else stay in LimpHomePreSleep */
            break;
        case OSEKNM_TwbsNORMAL: /* Figure 34 - Received msg part */
            if(0 == OsekDirectNm_Networks[netHandleId].nmRxPdu[rxBufIndex].OpCode.B.sleepInd) {
                /* Cancel timer TWbs */ /** @req OSEKNM023 */
                OsekDirectNm_Networks[netHandleId].timer.osekdirectNmTWbs = 0;
                OsekDirectNm_Networks[netHandleId].networkStatus.nodeStatus.twbsNormalLimp = 0;

                /* AF modified */
                /* Get Actual netId and enable the communication */
                netId = OsekNm_Internal_GetNetId(netHandleId);
                D_Online(netId);
                /* End AF modified */

                initFromNormal(netHandleId);
            }/** @req OSEKNM021 */
            break;
        case OSEKNM_TwbsLIMPHOME: /* Figure 37 - Received msg part */ /** @req OSEKNM036 */
            if(0 == OsekDirectNm_Networks[netHandleId].nmRxPdu[rxBufIndex].OpCode.B.sleepInd) {
                OsekDirectNm_Networks[netHandleId].timer.osekdirectNmTWbs = 0;
                OsekDirectNm_Networks[netHandleId].nmState = OSEKNM_LIMPHOME;
                /* Setting timer was missing in Spec */
                OsekDirectNm_Networks[netHandleId].timer.osekdirectNmTError = (OsekNm_ConfigPtr->osekNmNetwork[netHandleId].osekDirectNmNodeconfParams->osekdirectNmTError + 1);
                OsekDirectNm_Networks[netHandleId].networkStatus.nodeStatus.twbsNormalLimp = 0;
                OsekDirectNm_Networks[netHandleId].networkStatus.nodeStatus.limpHome = 1;
#if (STD_ON == OSEKNM_DIRECT_STATE_CHANGE_INDICATION)
                STATE_CHANGE_INDICATION(OSEKNM_LIMPHOME);
#endif
            }/* else stay in same state */
            break;
        default:
            break;
    }
}

/**
 * @brief OsekNm Direct main function processing
 * @param netId
 */
/** @req OSEKNM041 */
void OsekDirectNm_MainFunction(NetIdType netId){
    NetIdType netHandleId;
    uint8 i;
    boolean exitFlag = FALSE;

    /* Get NetHandle mapped to Actual netId */
    netHandleId = OsekNm_Internal_CheckNetId(netId);

    /* Enter Critical section */
    SchM_Enter_OsekNm_EA_0();
    /* Check Rx indication is set */
    if(TRUE == OsekDirectNm_Networks[netHandleId].rcvdNmpdu) {
        for (i=0; i < OsekDirectNm_Networks[netHandleId].nmRxPduCount; i++) {
            rxNmpduProcessFunc(netHandleId, i);
        }
        OsekDirectNm_Networks[netHandleId].rcvdNmpdu = FALSE;
        OsekDirectNm_Networks[netHandleId].nmRxPduCount = 0;
    }

    /* Check Tx Indication is set */
    if(TRUE == OsekDirectNm_Networks[netHandleId].transNmpdu) {
        OsekDirectNm_Networks[netHandleId].transNmpdu = FALSE;
        txNmpduProcessFunc(netHandleId);
    }
    /* exit Critical section */
    SchM_Exit_OsekNm_EA_0();

    /* Figure 39 - DLL transmit Rejection */
    /* timer started */
    if(0 != OsekDirectNm_Networks[netHandleId].timer.osekdirectNmTTx) {
        /* Decrement timer */
        if(OsekDirectNm_Networks[netHandleId].timer.osekdirectNmTTx > 1) {
            OsekDirectNm_Networks[netHandleId].timer.osekdirectNmTTx--;
        }

        /* Check timeout */
        if(1 == OsekDirectNm_Networks[netHandleId].timer.osekdirectNmTTx) {
            /* cancel timer TTx */
            OsekDirectNm_Networks[netHandleId].timer.osekdirectNmTTx = 0;

            sendNmMessage(netHandleId);
            exitFlag = TRUE;
        }
    }

    if(FALSE == exitFlag){
        switch(OsekDirectNm_Networks[netHandleId].nmState){
            case OSEKNM_NORMAL:
                normalMainFunc(netHandleId);
                break;

            case OSEKNM_NORMALPREPSLEEP:
                normalPrepSleepMainFunc(netHandleId);
                break;

            case OSEKNM_LIMPHOME:
                limphomeMainFunc(netHandleId);
                break;

            case OSEKNM_LIMPHOMEPREPSLEEP:
                limphomePrepSleepMainFunc(netHandleId);
                break;

            case OSEKNM_TwbsNORMAL:
                normalTwbsMainFunc(netHandleId);
                break;

            case OSEKNM_TwbsLIMPHOME:
                twbsLimphomeMainFunc(netHandleId);
                break;

            default:
                break;
        }
    }
}

#if NOT_SUPPORTED_YET
/**
 * @brief function is to read received ring data by application
 * @param netId
 * @param ringData
 */
void ReadRingData(NetIdType netId,RingDataRefType ringData){
    NetIdType netHandleId;
    /* Get NetHandle mapped to Actual netId */
    netHandleId = OsekNm_Internal_CheckNetId(netId);

    memcpy(ringData, OsekDirectNm_Networks[netHandleId].nmTransmitPdu.ringData, sizeof(RingDataType));
}

/**
 * @brief function is to update Transmit ring data by application
 * @param netId
 * @param ringData
 */
/*lint -e{818} STANDARDIZED_API */
void TransmitRingData(NetIdType netId,RingDataRefType ringData){
    NetIdType netHandleId;
    /* Get NetHandle mapped to Actual netId */
    netHandleId = OsekNm_Internal_CheckNetId(netId);

    memcpy(OsekDirectNm_Networks[netHandleId].nmTransmitPdu.ringData, ringData, sizeof(RingDataType));
}
#endif
/**
 * @brief Controller bus-off function
 * @param netId
 */
void OsekDirectNm_ControllerBusOff(NetIdType netId){
    uint8 netHandleId;

    /* Figure 31 in ISO doc */ /** @req OSEKNM039 */
    /* Get Mapped Network handle id to netId */
    netHandleId = OsekNm_Internal_CheckNetId(netId);
    D_Offline(netId);
    D_Init(netId,BusRestart);

    /* Cancel all timers */
    OsekDirectNm_Networks[netHandleId].timer.osekdirectNmTTyp = 0;
    OsekDirectNm_Networks[netHandleId].timer.osekdirectNmTTx = 0;
    OsekDirectNm_Networks[netHandleId].timer.osekdirectNmTMax = 0;
    OsekDirectNm_Networks[netHandleId].timer.osekdirectNmTError = 0;
    OsekDirectNm_Networks[netHandleId].timer.osekdirectNmTWbs = 0;

    /* set timer TError */
    OsekDirectNm_Networks[netHandleId].timer.osekdirectNmTError = (OsekNm_ConfigPtr->osekNmNetwork[netHandleId].osekDirectNmNodeconfParams->osekdirectNmTError + TIMER_2);
    OsekDirectNm_Networks[netHandleId].networkStatus.nodeStatus.busoff = 1;
    OsekDirectNm_Networks[netHandleId].networkStatus.nodeStatus.limpHome = 1;
    OsekDirectNm_Networks[netHandleId].nmState = OSEKNM_LIMPHOME;
#if (STD_ON == OSEKNM_DIRECT_STATE_CHANGE_INDICATION)
    OsekNm_ConfigPtr->osekNmDirectCallBack(netId,OSEKNM_LIMPHOME);
#endif
}

/**
 * @brief Transmission confirmation function
 * @param netId
 */
void OsekDirectNm_TxConformation(NetIdType netId) {
    uint8 netHandleId;
    /* Get Mapped Network handle id to netId */
    netHandleId = OsekNm_Internal_CheckNetId(netId);

    OsekDirectNm_Networks[netHandleId].networkStatus.nodeStatus.busoff = 0;
    OsekDirectNm_Networks[netHandleId].transNmpdu = TRUE;
}

/**
 * @brief Received message indication function
 * @param netId
 * @param NMPDU
 */
void OsekDirectNm_RxIndication(NetIdType netId, const OsekNm_PduType* nmPdu) {
    uint8 netHandleId;
    uint8 rxCount;

    /* Get Mapped Network handle id to netId */
    netHandleId = OsekNm_Internal_CheckNetId(netId);

    rxCount = OsekDirectNm_Networks[netHandleId].nmRxPduCount;
    OsekDirectNm_Networks[netHandleId].nmRxPduCount++;
    OsekDirectNm_Networks[netHandleId].networkStatus.nodeStatus.busoff = 0;
    memcpy(&OsekDirectNm_Networks[netHandleId].nmRxPdu[rxCount], nmPdu, sizeof(OsekNm_PduType));
    OsekDirectNm_Networks[netHandleId].rcvdNmpdu = TRUE;
}

#if (ISO_OSEK_NM == STD_ON)
/**
 * @brief WakeUp indication function
 * @param netId
 */
void OsekDirectNm_WakeupIndication(NetIdType netId){

    uint8 netHandleId;
    /* Get Mapped Network handle id to netId */
    netHandleId = OsekNm_Internal_CheckNetId(netId);

    OsekDirectNm_Networks[netHandleId].networkStatus.nodeStatus.busoff = 0;
    if(OSEKNM_BUSSLEEP == OsekDirectNm_Networks[netHandleId].nmState) {
        /* Wake up signal of the bus*/
        D_Init(netId,BusAwake);
        initFromBusSleep(netHandleId);
    }
}
#endif

#ifdef HOST_TEST
OsekDirect_InternalType* readinternal_OsekNmDirectStat(void );
OsekDirect_InternalType* readinternal_OsekNmDirectStat(void)
{
    return OsekDirectNm_Networks;
}
#endif

#endif







