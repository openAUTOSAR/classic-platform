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
#if (OSEKNM_INDIRECT_NET_NUM > 0)
#include "OsekNm_ConfigTypes.h"
#include"OsekIndirectNm.h"
#include <string.h>
#include "SchM_OsekNm.h"

/** @req OSEKNM051 */ /* OSEKNM support indirect and direct modes */

typedef uint8 OsekNmNetHandleType;
extern const OsekNm_ConfigType* OsekNm_ConfigPtr;
OsekIndNm_InternalNetType OsekNm_Indirect_NetChannels[OSEKNM_INDIRECT_NET_NUM];

/** @req OSEKNM050 */
#define STATE_CHANGE_INDICATION_INDIRECT(state)    \
           netId = OsekNm_Internal_GetNetId(netHandle);  \
           OsekNm_ConfigPtr->osekNmIndirectCallBack(netId, state ); \



/** Static function declarations */
static void indirectInitNetworkStatus(OsekNmNetHandleType netHandle);
static void indirectInitPrivateConfig(OsekNmNetHandleType netHandle );
static void indirectInitConfig(OsekNmNetHandleType netHandle);
static void indirectTOBTimeoutEventHandle(OsekNmNetHandleType netHandle);
static void indirectTxandRxMsgNormalNotification(OsekNmNetHandleType netHandle, NodeIdType nodeId);
static void indirectTxandRxMsgLimphomeNotification(OsekNmNetHandleType netHandle, NodeIdType nodeId);
static uint8 indirectCheckNodeId(OsekNmNetHandleType netHandle, uint8 nodeId);
static void indirectAddToConfig(OsekNmNetHandleType netHandleId,ConfigKindName configKind,NodeIdType nodeId);
static void indirectRemoveFromConfig(OsekNmNetHandleType netHandleId,ConfigKindName configKind,NodeIdType nodeId);
static void indirectAddToPrivateConfig(OsekNmNetHandleType netHandleId,NodeIdType nodeId);


static void indirectInitExtendedNetworkStatus(OsekNmNetHandleType netHandle);
static void indirectInitExtendedConfig(OsekNmNetHandleType netHandle);
static void indirectIncConfigCount(OsekNmNetHandleType netHandle, uint8 nodeHandle);
static void indirectDecConfigCount(OsekNmNetHandleType netHandle, uint8 nodeHandle);
static void indirectIncStatusCount(OsekNmNetHandleType netHandle );
static void indirectDecStatusCount(OsekNmNetHandleType netHandle );



/**
 * @brief Initialization of the configuration as per Figure 54 and Figure 62 in OsekNM ISO17356-5 document
 *        reset all bits of representing nodes in the network
 * @param netHandle
 */
static void indirectInitConfig(OsekNmNetHandleType netHandle){

    /* Considering the config as per NmConfigParams structure all nodes are represendted as bit position per configured node ID
     * it will give the each node active status as per node ID */
    memset(OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].indirectConfig.normal, 0 ,OSEKNM_NODE_MAX);

    if(OsekNm_ConfigPtr->osekNmNetwork[netHandle].OsekNmIndirectNodeIdPrams->configName == NM_CKNORMALEXTENDED ){
        /* as per Figure 62 in OsekNM ISO17356-5 document*/
        indirectInitExtendedConfig(netHandle);
    }

}

/**
 * @brief Initialization of the private configuration as per fig 54 in OsekNM ISO17356-5 document
 *        reset all bits of representing nodes in the network
 * @param netHandle
 */
static void indirectInitPrivateConfig(OsekNmNetHandleType netHandle){

   /* this is replacement implemenation for init private config per bit per node as configured  */
   memset(OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].indirectPrivateConfig, 0 ,OSEKNM_NODE_MAX);

}

/**
 * @brief Handling of TOB events during NMNormal and NNLimpome state as per Figure 51 and 52
 * in OsekNM ISO17356-5 document.
 * @param netHandle
 */
/* @req OSEKNM059 */ /* @req OSEKNM060 */
static void indirectTOBTimeoutEventHandle(OsekNmNetHandleType netHandle){

    /* Last private config is saved in to Config and reset private config on TOB time out */
    memcpy(OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].indirectConfig.normal,OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].indirectPrivateConfig, OSEKNM_NODE_MAX);

    indirectInitPrivateConfig(netHandle);

    /* SetTimer TOB */
    OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].ToBTimeLeft = OsekNm_ConfigPtr->osekNmNetwork[netHandle].OsekNmIndirectNodeIdPrams->osekNmIndirectTOB+1u ;

}


/**
 * @brief Function is to add Node status bit to present config based on the configKind
 * @param netHandleId
 * @param configKind
 * @param nodeId
 */
static void indirectAddToConfig(OsekNmNetHandleType netHandleId,ConfigKindName configKind,NodeIdType nodeId){
    ConfigRefType ConfigNode = NULL;
    uint8 byte;
    uint8 bit;

    switch(configKind)
    {
        case NM_CKNORMAL:
            ConfigNode = OsekNm_Indirect_NetChannels[netHandleId].indirectConfig.normal;
            break;
        case NM_CKNORMALEXTENDED:
            ConfigNode = OsekNm_Indirect_NetChannels[netHandleId].indirectConfig.normalExtended;
            break;
        default:
            break;
    }

    if(NULL != ConfigNode)
    {
        /* Set the corresponding bit in array to represent nodeId */
        byte = nodeId / 8;
        bit  = nodeId % 8;
        ConfigNode[byte] |= (uint8)(1u<<bit);
    }
}


/**
 * @brief Function call is to remove node status bit from configuration based on the configKind
 * @param netHandleId
 * @param configKind
 * @param nodeId
 */
static void indirectRemoveFromConfig(OsekNmNetHandleType netHandleId,ConfigKindName configKind,NodeIdType nodeId) {
    ConfigRefType ConfigNode = NULL;
    uint8 byte;
    uint8 bit;

    switch(configKind)
    {
        case NM_CKNORMAL:
            ConfigNode = OsekNm_Indirect_NetChannels[netHandleId].indirectConfig.normal;
            break;
        case NM_CKNORMALEXTENDED:
            ConfigNode = OsekNm_Indirect_NetChannels[netHandleId].indirectConfig.normalExtended;
            break;
        default:
            break;
    }

    if(NULL != ConfigNode)
    {
        /* Set the corresponding bit in array to represent nodeId */
        byte = nodeId / 8;
        bit  = nodeId % 8;
        ConfigNode[byte] &= (~(1u<<bit));
    }
}


/**
 * @brief Function is to add Node status bit to private config this is applicable only to TOB mode selected
 * @param netHandleId
 * @param nodeId
 */
static void indirectAddToPrivateConfig(OsekNmNetHandleType netHandleId,NodeIdType nodeId){
    ConfigRefType ConfigNode;
    uint8 byte;
    uint8 bit;

    ConfigNode = OsekNm_Indirect_NetChannels[netHandleId].indirectPrivateConfig;

    /* Set the corresponding bit in array to represent nodeId */
    byte = nodeId / 8;
    bit  = nodeId % 8;
    ConfigNode[byte] |= (uint8)(1u<<bit);
}


/**
 * @brief Function for incrementing config count for each node
 * as per Figure 64 in OsekNM ISO17356-5 document
 * @param netHandle
 * @param nodeHandle
 */
/* @req OSEKNM068 *//* @req OSEKNM069 *//* @req OSEKNM070 */
static void indirectIncConfigCount(OsekNmNetHandleType netHandle, uint8 nodeHandle){

    uint8 configuredvalue;
    uint8 calculatedvalue;
    /* Allocation of nodehandle in local structure for master at index 0 */
    if(OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].indirectSlaveNodeCounter[nodeHandle] <=
       (OsekNm_ConfigPtr->osekNmNetwork[netHandle].OsekNmIndirectNodeIdPrams->osekNmIndirectThreshold
               - OsekNm_ConfigPtr->osekNmNetwork[netHandle].OsekNmIndirectNodeIdPrams->osekNmIndirectNodevalues[nodeHandle-1].osekNmIndirectDeltaInc )){
        configuredvalue = OsekNm_ConfigPtr->osekNmNetwork[netHandle].OsekNmIndirectNodeIdPrams->osekNmIndirectNodevalues[nodeHandle-1].osekNmIndirectDeltaInc;
        calculatedvalue = OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].indirectSlaveNodeCounter[nodeHandle] + configuredvalue;
        OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].indirectSlaveNodeCounter[nodeHandle] = calculatedvalue;
    }else{
        /* @req OSEKNM080*/
        OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].indirectSlaveNodeCounter[nodeHandle] = OsekNm_ConfigPtr->osekNmNetwork[netHandle].OsekNmIndirectNodeIdPrams->osekNmIndirectThreshold;
    }

}
/**
 * @brief Function for decrementing config count for each slave node
 * as per Figure 65 in OsekNM ISO17356-5 document
 * @param netId
 * @param nodeId
 */

static void indirectDecConfigCount(OsekNmNetHandleType netHandle, uint8 nodeHandle){

    if(OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].indirectSlaveNodeCounter[nodeHandle] >= OsekNm_ConfigPtr->osekNmNetwork[netHandle].OsekNmIndirectNodeIdPrams->osekNmIndirectNodevalues[nodeHandle-1].osekNmIndirectDeltaDec){
       OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].indirectSlaveNodeCounter[nodeHandle] = OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].indirectSlaveNodeCounter[nodeHandle]
                       -OsekNm_ConfigPtr->osekNmNetwork[netHandle].OsekNmIndirectNodeIdPrams->osekNmIndirectNodevalues[nodeHandle-1].osekNmIndirectDeltaInc;
    }else{
       OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].indirectSlaveNodeCounter[nodeHandle] = 0;
    }
}


/**
 * @brief Function for decrementing status count for monitoring node
 * as per Figure 65 in OsekNM ISO17356-5 document
 * @param netHandle
 */
/* @req OSEKNM068 *//* @req OSEKNM069 *//* @req OSEKNM070 */
static void indirectDecStatusCount(OsekNmNetHandleType netHandle ){

    if(OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].IndirectMasterNodeCount >=
                              OsekNm_ConfigPtr->osekNmNetwork[netHandle].OsekNmIndirectNodeIdPrams->osekNmIndirectDeltaDec){

          OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].IndirectMasterNodeCount = OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].IndirectMasterNodeCount - OsekNm_ConfigPtr->osekNmNetwork[netHandle].OsekNmIndirectNodeIdPrams->osekNmIndirectDeltaDec;
      }else{
          OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].IndirectMasterNodeCount = 0;
      }

}

/**
 * @brief Function for incrementing status count for monitoring node
 * as per Figure 64 in OsekNM ISO17356-5 document
 * @param netHandle
 */
/* @req OSEKNM068 *//* @req OSEKNM069 *//* @req OSEKNM070 */
static void indirectIncStatusCount(OsekNmNetHandleType netHandle ){

    if(OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].IndirectMasterNodeCount <=
            (OsekNm_ConfigPtr->osekNmNetwork[netHandle].OsekNmIndirectNodeIdPrams->osekNmIndirectThreshold
               - OsekNm_ConfigPtr->osekNmNetwork[netHandle].OsekNmIndirectNodeIdPrams->osekNmIndirectDeltaInc)){

        OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].IndirectMasterNodeCount = OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].IndirectMasterNodeCount + OsekNm_ConfigPtr->osekNmNetwork[netHandle].OsekNmIndirectNodeIdPrams->osekNmIndirectDeltaInc;
    }else{
        OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].IndirectMasterNodeCount = OsekNm_ConfigPtr->osekNmNetwork[netHandle].OsekNmIndirectNodeIdPrams->osekNmIndirectThreshold;
    }

}


/**
 * @brief Message notification from CanIF during NMNormal state
 * Monitored application message received from remote node and monitored application transmitted by own node
 * as per Figure 51 and 57 in OsekNM ISO17356-5 document
 * @param netHandle
 * @param nodeId
 */
/* @req OSEKNM053 */
static void indirectTxandRxMsgNormalNotification(OsekNmNetHandleType netHandle,NodeIdType nodeId){

    uint8 nodeHandle;

    nodeHandle = indirectCheckNodeId((netHandle-OSEKNM_DIRECT_NET_NUM),nodeId);
    /* @req OSEKNM077*//* @req OSEKNM078*/
    if(OsekNm_ConfigPtr->osekNmNetwork[netHandle].OsekNmIndirectNodeIdPrams->osekNmIndirectTimeoutConf == GLOBAL_TIMEOUT){
        /* resetTimer TOB to initial value after receiving message */
        OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].ToBTimeLeft = OsekNm_ConfigPtr->osekNmNetwork[netHandle].OsekNmIndirectNodeIdPrams->osekNmIndirectTOB;
        indirectAddToPrivateConfig((netHandle-OSEKNM_DIRECT_NET_NUM),nodeId);

    }else if(OsekNm_ConfigPtr->osekNmNetwork[netHandle].OsekNmIndirectNodeIdPrams->osekNmIndirectTimeoutConf == TIMEOUT_PER_MESSAGE){
        indirectAddToConfig((netHandle-OSEKNM_DIRECT_NET_NUM), NM_CKNORMAL, nodeId);
        if(OsekNm_ConfigPtr->osekNmNetwork[netHandle].OsekNmIndirectNodeIdPrams->configName == NM_CKNORMALEXTENDED ){
            indirectDecConfigCount(netHandle,nodeHandle );
            indirectAddToConfig((netHandle-OSEKNM_DIRECT_NET_NUM), NM_CKNORMALEXTENDED, nodeId);
        }
    }else{
        /*do nothing */
    }

    OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].networkStatus.nodeStatus.limpHome = NMLIMPHOME_OFF;
    OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].networkStatus.nodeStatus.busoff = OSEKNM_NO_BUS_ERROR;

    if(OsekNm_ConfigPtr->osekNmNetwork[netHandle].OsekNmIndirectNodeIdPrams->osekNmIndirectTimeoutConf == TIMEOUT_PER_MESSAGE){
        if(OsekNm_ConfigPtr->osekNmNetwork[netHandle].OsekNmIndirectNodeIdPrams->configName == NM_CKNORMALEXTENDED ){
            OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].networkStatus.nodeStatus.ExtendedNetworkstatus = OSEKNM_NO_BUS_ERROR;
            indirectDecStatusCount(netHandle);
        }
    }

}

/**
 * @brief Message notification from CanIF during limphome state
 * Monitored application message received from remote node and monitored application transmitted by own node
 * as per Figure 52 and 58 in OsekNM ISO17356-5 document
 * @param netHandle
 * @param nodeId
 */
static void indirectTxandRxMsgLimphomeNotification(OsekNmNetHandleType netHandle, NodeIdType nodeId){

    uint8 nodeHandle;
#if (STD_ON == OSEKNM_INDIRECT_STATE_CHANGE_INDICATION)
    NetIdType netId;
#endif

    nodeHandle = indirectCheckNodeId((netHandle-OSEKNM_DIRECT_NET_NUM),nodeId);
    /* @req OSEKNM077*//* @req OSEKNM078*/
    if(OsekNm_ConfigPtr->osekNmNetwork[netHandle].OsekNmIndirectNodeIdPrams->osekNmIndirectTimeoutConf == GLOBAL_TIMEOUT){
        /*as per Figure 52*/
        /* SetTimer TOB */
        OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].ToBTimeLeft = OsekNm_ConfigPtr->osekNmNetwork[netHandle].OsekNmIndirectNodeIdPrams->osekNmIndirectTOB ;
        indirectAddToPrivateConfig((netHandle-OSEKNM_DIRECT_NET_NUM),nodeId);
        (void)nodeHandle;
    }else if(OsekNm_ConfigPtr->osekNmNetwork[netHandle].OsekNmIndirectNodeIdPrams->configName == NM_CKNORMALEXTENDED ){
        /*as per Figure 58*/
        indirectAddToConfig((netHandle-OSEKNM_DIRECT_NET_NUM), NM_CKNORMAL, nodeId);
        indirectDecConfigCount(netHandle,nodeHandle );
        indirectAddToConfig((netHandle-OSEKNM_DIRECT_NET_NUM), NM_CKNORMALEXTENDED, nodeId);
    }else{
        /* do nothing */
    }

    if(nodeId == OsekNm_ConfigPtr->osekNmNetwork[netHandle].OsekNmIndirectNodeIdPrams->osekNmIndirectSourceNodeId){
        OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].TxOk = TRUE;
    }else{
        OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].RxOk = TRUE;
    }

    if((OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].TxOk == TRUE)&&(OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].RxOk == TRUE)){

        OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].networkStatus.nodeStatus.limpHome = NMLIMPHOME_OFF;
        OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].networkStatus.nodeStatus.busoff = OSEKNM_NO_BUS_ERROR;

        if(OsekNm_ConfigPtr->osekNmNetwork[netHandle].OsekNmIndirectNodeIdPrams->configName == NM_CKNORMALEXTENDED ){
            /*as per Figure 58*/
            OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].networkStatus.nodeStatus.ExtendedNetworkstatus = OSEKNM_NO_BUS_ERROR;
            /* As per ISO document in fig 58 it worngly mentioned as DecConfigCount instead of DecStatusCount so corrected in code */
            indirectDecStatusCount(netHandle);
        }

       OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].nmState = OSEKINDNM_NORMAL;
#if (STD_ON == OSEKNM_INDIRECT_STATE_CHANGE_INDICATION)
       STATE_CHANGE_INDICATION_INDIRECT(OSEKINDNM_NORMAL);
#endif


#if (STD_ON == ASR_OSEK_NM)
       Nm_NetworkMode(OsekNmConfig.osekNmNetwork[netHandle-OSEKNM_DIRECT_NET_NUM].osekNmNetworkHandle);
#endif

    }
}


/**
 * @brief Message notification from CanIF after receiving and transmitting the message
 *        implemenation as per as per fig 57,51,52 and 58 in OsekNM ISO17356-5 document
 * @param netId
 * @param nodeId
 */
/* @req OSEKNM053 *//* @req OSEKNM056 *//* @req OSEKNM057 */
void OsekInDirectNm_Com_MsgNotification(NetIdType netId, NodeIdType nodeId){
    /* @req OSEKNM074*//* @req OSEKNM075*/
    uint8 netHandle;

    netHandle = OsekNm_Internal_CheckNetId(netId);
    /* After Message received/ Trnasmitted in nomal and limphome */
    if(OsekNm_Indirect_NetChannels[netHandle].nmState == OSEKINDNM_LIMPHOME){
        /* Message received durng LIMphome state as per fig 52 nad 58 */
        indirectTxandRxMsgLimphomeNotification(netHandle,nodeId);
    }else if(OsekNm_Indirect_NetChannels[netHandle].nmState == OSEKINDNM_NORMAL){
        /* Message received durng normal state as per fig 51 and 57 */
        indirectTxandRxMsgNormalNotification(netHandle,nodeId);
    }else{
        /* do noting */
    }
}



/**
 * @brief Initialisation of indirect Nm nodes network status
 * implemenation as per as per fig 55 for TOB and Fig 63 for extended mode,
 * @param netHandle
 */
static void indirectInitNetworkStatus(OsekNmNetHandleType netHandle){

    /* As per ISO after init state transition should be to Normal, but as per ASR state should be BUS_SLEEP
     * so to comply with ASR added this below case with condition compilation  */

#if (ISO_OSEK_NM == STD_ON)
    OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].networkStatus.nodeStatus.busoff = OSEKNM_NO_BUS_ERROR;
    OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].networkStatus.nodeStatus.nmOff = NM_ON;
    OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].networkStatus.nodeStatus.limpHome = NMLIMPHOME_OFF;
    OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].networkStatus.nodeStatus.bussleep = NMBUSSLEEP_OFF;
    OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].networkStatus.nodeStatus.twbsNormalLimp = NMWAITBUSSLEEP_OFF;
#endif
#if (ASR_OSEK_NM == STD_ON)
    OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].networkStatus.nodeStatus.busoff = OSEKNM_NO_BUS_ERROR;
    OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].networkStatus.nodeStatus.nmOff = NM_ON;
    OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].networkStatus.nodeStatus.limpHome = NMLIMPHOME_OFF;
    OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].networkStatus.nodeStatus.bussleep = NMBUSSLEEP_ON;
    OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].networkStatus.nodeStatus.twbsNormalLimp = NMWAITBUSSLEEP_OFF;
#endif

}


/**
 * @brief Initialisation of indirect Nm nodes extended network status
 * implemenation as per as per Fig 63 for extended mode,
 * @param netHandle
 */
static void indirectInitExtendedNetworkStatus(OsekNmNetHandleType netHandle){

    OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].networkStatus.nodeStatus.ExtendedNetworkstatus = OSEKNM_NO_BUS_ERROR;
    OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].IndirectMasterNodeCount= 0u;
}

/**
 * @brief Initialisation of indirect Nm nodes extended network configuration
 * implemenation as per as per Fig 62 for extended mode,
 * @param netHandle
 */
static void indirectInitExtendedConfig(OsekNmNetHandleType netHandle){

   memset(OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].indirectConfig.normalExtended, 0x00 ,OSEKNM_NODE_MAX);
   /* Considering monitoring node will be always in 0 th index */
   memset(OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].indirectSlaveNodeCounter,0, OSEKNM_INDIRECT_NODES+1);

}


/**
 * @brief Get NodeHandle Id from actual Nodeid and net handle
 * @param NethandleId
 * @param nodeId
 * @return
 */
static uint8 indirectCheckNodeId(OsekNmNetHandleType NethandleId, uint8 nodeId){

    uint8 NodehandleId;
    NodehandleId = (uint8)OSEKNM_INVALID_NODEID;

    if(OsekNm_ConfigPtr->osekNmNetwork[NethandleId].OsekNmIndirectNodeIdPrams->osekNmIndirectSourceNodeId==nodeId){

        NodehandleId = 0;
    }else{
        for (uint8 i = 0; i < OsekNm_ConfigPtr->osekNmNetwork[NethandleId].OsekNmIndirectNodeIdPrams->osekNmIndirectNodeCount; i++) {
            if(nodeId == OsekNm_ConfigPtr->osekNmNetwork[NethandleId].OsekNmIndirectNodeIdPrams->osekNmIndirectNodevalues[i].osekNmIndirectNodeId){
                NodehandleId = (uint8)i+1;
                break;
            }
        }
    }
    return NodehandleId;
}



/**
 * @brief Handling of COM messge timeout notification for respective net and node ID
 * Time out at monitoring application message from remote node expired
 * or time out at monitoring transmission of own application message expired
 * @param netid
 * @param nodeid
 */
/* @req OSEKNM055*/ /* @req OSEKNM061*/
void OsekInDirectNm_comMsg_TimeoutHandle(NetIdType netHandle,NodeIdType nodeid){
    /* @req OSEKNM074*//* @req OSEKNM076*/
    /* implemenation as per as per Fig 57 */
    uint8 nodeHandle;
    /* @req OSEKNM077*//* @req OSEKNM079*/
    nodeHandle = indirectCheckNodeId(netHandle-OSEKNM_DIRECT_NET_NUM,nodeid);
    if((OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].nmState == OSEKINDNM_NORMAL )
            ||(OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].nmState == OSEKINDNM_LIMPHOME)){
        if(OsekNm_ConfigPtr->osekNmNetwork[netHandle].OsekNmIndirectNodeIdPrams->osekNmIndirectTimeoutConf == TIMEOUT_PER_MESSAGE){
            indirectRemoveFromConfig((netHandle-OSEKNM_DIRECT_NET_NUM), NM_CKNORMAL, nodeid);
            if(OsekNm_ConfigPtr->osekNmNetwork[netHandle].OsekNmIndirectNodeIdPrams->configName == NM_CKNORMALEXTENDED ){
                indirectIncConfigCount(netHandle, nodeHandle);
                /* @req OSEKNM080*/
                if(OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].indirectSlaveNodeCounter[nodeHandle] == OsekNm_ConfigPtr->osekNmNetwork[netHandle].OsekNmIndirectNodeIdPrams->osekNmIndirectThreshold){
                    indirectRemoveFromConfig((netHandle-OSEKNM_DIRECT_NET_NUM), NM_CKNORMALEXTENDED, nodeid);
                }else{
                    indirectAddToConfig((netHandle-OSEKNM_DIRECT_NET_NUM), NM_CKNORMALEXTENDED, nodeid);
                }
            }
        }
    }
}



/**
 * @brief This function is to execute fatal bus error like BusOff occured for CAN
 * implemenation as per as per Fig 53 for tob and 59 for extended mode
 * @param netId
 */
void OsekInDirectNm_FatalBusError_Handle(NetIdType netId){

    uint8 netHandle;

    netHandle = OsekNm_Internal_CheckNetId(netId);

    if((OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].nmState == OSEKINDNM_NORMAL )||(OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].nmState == OSEKINDNM_LIMPHOME)
            ||(OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].nmState == OSEKINDNM_WAITBUSSLEEP)
     ){
        OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].networkStatus.nodeStatus.limpHome = NMLIMPHOME_ON;
        OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].networkStatus.nodeStatus.busoff = ERROR_BUSBLOCKED;

        if(OsekNm_ConfigPtr->osekNmNetwork[netHandle].OsekNmIndirectNodeIdPrams->configName == NM_CKNORMALEXTENDED ){
            if(OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].networkStatus.nodeStatus.twbsNormalLimp == NMWAITBUSSLEEP_ON ){
                /* @req OSEKNM065 */
                OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].nmState = OSEKINDNM_WAITBUSSLEEP;
#if (STD_ON == OSEKNM_INDIRECT_STATE_CHANGE_INDICATION)
                OsekNm_ConfigPtr->osekNmIndirectCallBack(netId, OSEKINDNM_WAITBUSSLEEP );
#endif
            }
        }

#if (ISO_OSEK_NM == STD_ON)
        D_Offline(netId);
        D_Init(netId,BusRestart);
#endif
        /* SetTimer Terror */
        OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].TErrorTimeLeft = OsekNm_ConfigPtr->osekNmNetwork[netHandle].OsekNmIndirectNodeIdPrams->osekNmIndirectTError;
        OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].TxOk = FALSE;
        OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].RxOk = FALSE;

        if(OsekNm_ConfigPtr->osekNmNetwork[netHandle].OsekNmIndirectNodeIdPrams->configName == NM_CKNORMALEXTENDED ){
            indirectIncStatusCount(netHandle);
            /* @req OSEKNM080*/
            if( OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].IndirectMasterNodeCount == OsekNm_ConfigPtr->osekNmNetwork[netHandle].OsekNmIndirectNodeIdPrams->osekNmIndirectThreshold){
                OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].networkStatus.nodeStatus.ExtendedNetworkstatus = ERROR_BUS_COMM_NOT_POSSIBLE;
            }else{
                OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].networkStatus.nodeStatus.ExtendedNetworkstatus = OSEKNM_NO_BUS_ERROR;
            }
        }
        /* @req OSEKNM064 */
        OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].nmState = OSEKINDNM_LIMPHOME;
#if (STD_ON == OSEKNM_INDIRECT_STATE_CHANGE_INDICATION)
                OsekNm_ConfigPtr->osekNmIndirectCallBack(netId, OSEKINDNM_LIMPHOME );
#endif
      }
}


/**
 * @brief StartNM starts the local Network Management. This causes the state transition from NMOff to NMOn
 * implemenation as per as per Fig 50 for tob and 56 for extended mode
 * @param netId
 * @return
 */
/** @req OSEKNM005 */
StatusType OsekInDirectNm_StartNM(NetIdType netId){

    uint8 config[OSEKNM_NODE_MAX];

    uint8 netHandle;
    netHandle = OsekNm_Internal_CheckNetId(netId);

    StatusType retStatus = E_OK;

    OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].nmState = OSEKINDNM_OFF;
    OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].RxOk = FALSE;
    OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].TxOk = FALSE;

#if (ISO_OSEK_NM == STD_ON)
    D_Init(netId,BusInit);
    D_Online(netId);
#endif

    memset(config, 0x01 ,OSEKNM_NODE_MAX);
    OsekInDirectNm_InitCMaskTable(netHandle, NM_CKNORMAL, config);

    if(OsekNm_ConfigPtr->osekNmNetwork[netHandle].OsekNmIndirectNodeIdPrams->osekNmIndirectTimeoutConf == GLOBAL_TIMEOUT){
        /* SetTimer TOB */
        OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].ToBTimeLeft = OsekNm_ConfigPtr->osekNmNetwork[netHandle].OsekNmIndirectNodeIdPrams->osekNmIndirectTOB ;
        indirectInitPrivateConfig(netHandle);
    }
    indirectInitConfig(netHandle);
    /* As per ISO flow chart in Figure 56 Pg(69) InitExtendedConfig is called after InitConfig, But removed here
     * because in InitConfig already InitExtendedConfig function is called   */
    indirectInitNetworkStatus(netHandle);

    /*As per ISO flow chart in Figure 50 Pg(64) during init nmState is Normal, but as per ASR initialised to BUSSLEEP */
    if(OsekNm_ConfigPtr->osekNmNetwork[netHandle].OsekNmIndirectNodeIdPrams->configName == NM_CKNORMALEXTENDED ){
        OsekInDirectNm_InitCMaskTable(netHandle, NM_CKNORMALEXTENDED, config);
        indirectInitExtendedNetworkStatus(netHandle);
     }
#if (ISO_OSEK_NM == STD_ON)
    OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].nmState = OSEKINDNM_NORMAL;
#endif
#if (ASR_OSEK_NM == STD_ON)
    /* @req OSEKNM066 */
    OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].nmState = OSEKINDNM_BUSSLEEP;
#endif
    return retStatus;


}

/**
 * @brief OsekIndNm_Processing contains state machine and timeouts handling
 * @param netId
 * @return void
 */
/** @req OSEKNM041 */ /** @req OSEKNM063 */
void OsekInDirectNm_Processing(NetIdType netId){

    uint8 netHandle;

    SchM_Enter_OsekNm_EA_0();
    netHandle = OsekNm_Internal_CheckNetId(netId);

    /* As per fig 59 and fig 53 handling of fatal bus error in  limphome ste to decrement Terror timer
     * in both extended and normal, TOB and timeout indication *//* @req OSEKNM062 */
    if(OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].nmState == OSEKINDNM_LIMPHOME){

        if (0 !=  OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].TErrorTimeLeft ){

            if(OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].TErrorTimeLeft ==1){
                OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].TErrorTimeLeft =0;
#if (ISO_OSEK_NM == STD_ON)
                D_Online(netId);
#endif
            }
            if(OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].TErrorTimeLeft >1){
                OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].TErrorTimeLeft--;
            }

        }

    }
    /* @req OSEKNM077*//* @req OSEKNM078*/
    if(OsekNm_ConfigPtr->osekNmNetwork[netHandle].OsekNmIndirectNodeIdPrams->osekNmIndirectTimeoutConf == GLOBAL_TIMEOUT){
        /* This as per fig 51 and fig 52 TOB timeout events handling as per main function if message not received   */
        if((OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].nmState == OSEKINDNM_LIMPHOME)
                ||(OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].nmState == OSEKINDNM_NORMAL)){
            if (0!=OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].ToBTimeLeft ){

               if(OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].ToBTimeLeft==1){
                   OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].ToBTimeLeft =0;
                   indirectTOBTimeoutEventHandle(netHandle);
               }
               if(OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].ToBTimeLeft>1){
                 OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].ToBTimeLeft--;
               }
            }
        }
    }else if(OsekNm_ConfigPtr->osekNmNetwork[netHandle].OsekNmIndirectNodeIdPrams->osekNmIndirectTimeoutConf == TIMEOUT_PER_MESSAGE){
        /* As per fig 60 handling  Twait bus sleep timer time out and enter to bus sleep */
        if(OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].nmState == OSEKINDNM_WAITBUSSLEEP){

            if (OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].TwbsTimeLeft !=0){

                if(OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].TwbsTimeLeft==1){
                    OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].TwbsTimeLeft =0;
#if (ISO_OSEK_NM == STD_ON)
                    D_Init(netId, BusSleep);
#endif
                    OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].networkStatus.nodeStatus.twbsNormalLimp = NMWAITBUSSLEEP_OFF;
                    OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].networkStatus.nodeStatus.bussleep = NMBUSSLEEP_ON;
                    /* @req OSEKNM066 */
                    OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].nmState = OSEKINDNM_BUSSLEEP;
#if (STD_ON == OSEKNM_INDIRECT_STATE_CHANGE_INDICATION)
                    OsekNm_ConfigPtr->osekNmIndirectCallBack(netId, OSEKINDNM_BUSSLEEP );
#endif
#if (ASR_OSEK_NM == STD_ON)
                    Nm_BusSleepMode(OsekNmConfig.osekNmNetwork[netHandle].osekNmNetworkHandle);
#endif
                }
                if(OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].TwbsTimeLeft>1){
                   OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].TwbsTimeLeft--;
                }
            }

        }
    }else{
        /* do nothing */
    }
    SchM_Exit_OsekNm_EA_0();
}


/**
 * @brief StopNM stops the local Network Management. This causes the state transition from NMOn
 * to NMShutDown and after processing of the shutdown activities to NMOff
 * implemenation as per as per Fig 50 for tob and 56 for extended mode
 * @param netId
 * @return
 */
/** @req OSEKNM040 */
StatusType OsekInDirectNm_StopNM(NetIdType netId){

    uint8 netHandle;

    netHandle = OsekNm_Internal_CheckNetId(netId);
#if (ISO_OSEK_NM == STD_ON)
    /* To perform shutdown activities*/
    D_Init(netId,BusShutdown);
#endif
    OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].networkStatus.nodeStatus.nmOff = NM_OFF;
    OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].nmState = OSEKINDNM_OFF;
#if (STD_ON == OSEKNM_INDIRECT_STATE_CHANGE_INDICATION)
    OsekNm_ConfigPtr->osekNmIndirectCallBack(netId, OSEKINDNM_OFF );
#endif
    OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].networkStatus.nodeStatus.twbsNormalLimp = NMWAITBUSSLEEP_OFF;
    OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].networkStatus.nodeStatus.bussleep = NMBUSSLEEP_OFF;
    OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].networkStatus.nodeStatus.limpHome = NMLIMPHOME_OFF;
    if(OsekNm_ConfigPtr->osekNmNetwork[netHandle].OsekNmIndirectNodeIdPrams->configName == NM_CKNORMALEXTENDED ){
        memset(OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].indirectSlaveNodeCounter,0, OSEKNM_INDIRECT_NODES+1);
        OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].IndirectMasterNodeCount =0;
    }
    OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].ToBTimeLeft =0;
    OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].TwbsTimeLeft =0;  /* time out indirectSlaveNodeCounter for wait bus sleep  */
    OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].TErrorTimeLeft =0;
    return E_OK;
}

/**
 * @brief GotoMode serves to set the NM operating mode specified by <newMode>
 * implemenation as per as per Fig 60 for tob and 61 for extended mode
 * @param netId
 * @param newMode
 * @return
 */
/* @req OSEKNM062 */ /* @req OSEKNM071 */
StatusType OsekInDirectNm_GotoMode(NetIdType netHandle,NMModeName newMode){
#if (STD_ON == OSEKNM_INDIRECT_STATE_CHANGE_INDICATION)||(ISO_OSEK_NM == STD_ON)
    NetIdType netId;
    netId = OsekNm_Internal_GetNetId(netHandle);
#endif
   SchM_Enter_OsekNm_EA_0();
    if(newMode == NM_BUSSLEEP) {
        OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].ToBTimeLeft =0;
        OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].TErrorTimeLeft=0;
        switch(OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].nmState  ){
            case OSEKINDNM_NORMAL:
            case OSEKINDNM_LIMPHOME:

#if (ISO_OSEK_NM == STD_ON)
                D_Offline(netId);
#endif
                /* SetTimer Twait bus sleep */
                OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].TwbsTimeLeft = OsekNm_ConfigPtr->osekNmNetwork[netHandle].OsekNmIndirectNodeIdPrams->osekNmIndirectTWBS;
                OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].networkStatus.nodeStatus.twbsNormalLimp = NMWAITBUSSLEEP_ON;
                /* @req OSEKNM065*/
                OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].nmState =OSEKINDNM_WAITBUSSLEEP;
#if (STD_ON == OSEKNM_INDIRECT_STATE_CHANGE_INDICATION)
                STATE_CHANGE_INDICATION_INDIRECT(OSEKINDNM_WAITBUSSLEEP)
#endif
                break;
            default:
                /*do nothing*/
                break;
        }

    } else if(newMode == NM_AWAKE) {
        switch(OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].nmState  ){
           case OSEKINDNM_WAITBUSSLEEP:

#if (ISO_OSEK_NM == STD_ON)
               D_Online(netId);
#endif
               OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].networkStatus.nodeStatus.twbsNormalLimp = NMWAITBUSSLEEP_OFF;
               OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].nmState =OSEKINDNM_NORMAL;
#if (STD_ON == OSEKNM_INDIRECT_STATE_CHANGE_INDICATION)
               STATE_CHANGE_INDICATION_INDIRECT(OSEKINDNM_NORMAL)
#endif
#if (STD_ON == ASR_OSEK_NM)
               Nm_NetworkMode(OsekNmConfig.osekNmNetwork[netHandle-OSEKNM_DIRECT_NET_NUM].osekNmNetworkHandle);
#endif
               break;
           case OSEKINDNM_BUSSLEEP:
               OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].networkStatus.nodeStatus.bussleep = NMBUSSLEEP_OFF;
#if (ISO_OSEK_NM == STD_ON)
               D_Init(netId,BusAwake);
               D_Online(netId);
#endif
               OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].nmState =OSEKINDNM_NORMAL;
#if (STD_ON == OSEKNM_INDIRECT_STATE_CHANGE_INDICATION)
               STATE_CHANGE_INDICATION_INDIRECT(OSEKINDNM_NORMAL)
#endif
#if (STD_ON == ASR_OSEK_NM)
               Nm_NetworkMode(OsekNmConfig.osekNmNetwork[netHandle-OSEKNM_DIRECT_NET_NUM].osekNmNetworkHandle);
#endif
               break;
           default:
               /*do nothing*/
               break;
        }

    }else{
        /*do nothing */
    }
    SchM_Exit_OsekNm_EA_0();
    return E_OK;
}




/**
 * @brief This service provides the current status of the network.
 * @param netId
 * @param networkStatus
 * @return
 */
/** @req OSEKNM054 *//* @req OSEKNM055*/
/* @req OSEKNM058 */
/*lint --e{9018}*/ /* 'networkStatus' with union based type 'StatusRefType' */
StatusType OsekInDirectNm_GetStatus(NetIdType netHandle,StatusRefType networkStatus){
    /** @req OSEKNM045 */
    StatusType status = E_OK;
    memcpy(networkStatus, &OsekNm_Indirect_NetChannels[netHandle-OSEKNM_DIRECT_NET_NUM].networkStatus, sizeof(NetworkStatusType));
    return status;
}


/**
 * @brief InitCMaskTable is a directive for initializing an element of a table of relevant configuration
 * masks to be used by the signalling of changed configurations.
 * @param netId
 * @param configKind
 * @param cMask
 */
void OsekInDirectNm_InitCMaskTable(NetIdType netHandleId,ConfigKindName configKind, ConfigRefType cMask)
{
    switch(configKind)
    {
        case NM_CKNORMAL:
            memcpy(OsekNm_Indirect_NetChannels[netHandleId-OSEKNM_DIRECT_NET_NUM].NmInDirectMask.normal, cMask, OSEKNM_NODE_MAX);
            break;
        case NM_CKNORMALEXTENDED:
            memcpy(OsekNm_Indirect_NetChannels[netHandleId-OSEKNM_DIRECT_NET_NUM].NmInDirectMask.normalExtended, cMask, OSEKNM_NODE_MAX);
            break;
        default:
            break;
    }
}/*lint !e818 STANDARDIZED_API */

/**
 * @brief This service provides the actual configuration of the kind specified by <configKind>
 * @param netId
 * @param configKind
 * @param targetConfig
 */
void OsekInDirectNm_InitTargetConfigTable(NetIdType netHandleId,ConfigKindName configKind,ConfigRefType targetConfig)
{
    switch(configKind)
    {
        case NM_CKNORMAL:
            memcpy(OsekNm_Indirect_NetChannels[netHandleId-OSEKNM_DIRECT_NET_NUM].indirectConfig.normal,targetConfig,OSEKNM_NODE_MAX);
            break;
        case NM_CKNORMALEXTENDED:
            memcpy(OsekNm_Indirect_NetChannels[netHandleId-OSEKNM_DIRECT_NET_NUM].indirectConfig.normalExtended,targetConfig,OSEKNM_NODE_MAX);
            break;
        default:
            break;
    }
}/*lint !e818 STANDARDIZED_API */

/**
 * @brief This service provides the actual configuration of the kind specified by <configKind>
 * @param netHandleId
 * @param Config
 * @param configKind
 * @return
 */
/** @req OSEKNM043 */
StatusType OsekInDirectNm_GetConfig(NetIdType netHandleId,ConfigRefType Config,ConfigKindName configKind){

    switch(configKind)
    {
        case NM_CKNORMAL:
            memcpy(Config, OsekNm_Indirect_NetChannels[netHandleId-OSEKNM_DIRECT_NET_NUM].indirectConfig.normal, OSEKNM_NODE_MAX);
            break;
        case NM_CKNORMALEXTENDED:
            memcpy(Config, OsekNm_Indirect_NetChannels[netHandleId-OSEKNM_DIRECT_NET_NUM].indirectConfig.normalExtended, OSEKNM_NODE_MAX);
            break;
        default:
            break;
    }
    return E_OK;

}

/**
 * @brief The test configuration <testConfig> is compared to the specified reference configuration
 * <RefConfig> taking account of the mask <cMask>.
 * @param netHandleId
 * @param testConfig
 * @param RefConfig
 * @param cMask
 * @return
 */
/** @req OSEKNM044 */
StatusType OsekInDirectNm_CmpConfig(NetIdType netHandleId,ConfigRefType testConfig,ConfigRefType RefConfig,ConfigRefType cMask){

    StatusType retStatus = E_OK;
    uint8 result;
    uint8 i;
    (void)netHandleId;

    /* (<testConfig> EXOR <RefConfig>) of the node within the network is identified within the <cMask> by TRUE. */
    for(i=0; i < OSEKNM_NODE_MAX; i++)
    {
        result = testConfig[i]^RefConfig[i];
        result &= cMask[i];
        result = ~result;
        if(result != 0xFFU)
        {
            retStatus = E_NOT_OK;
            break;
        }
    }
    return retStatus;
}/*lint !e818 STANDARDIZED_API */




#if (ISO_OSEK_NM == STD_ON)
/**
 * @brief WakeUp indication function
 * @param netId
 */
void OsekInDirectNm_WakeupIndication(NetIdType netId)
{
    uint8 netHandleId;
    netHandleId = OsekNm_Internal_CheckNetId(netId);
    OsekNm_Indirect_NetChannels[netHandleId-OSEKNM_DIRECT_NET_NUM].networkStatus.nodeStatus.bussleep = NMBUSSLEEP_OFF;
    D_Init(netId,BusAwake);
    D_Online(netId);
    OsekNm_Indirect_NetChannels[netHandleId-OSEKNM_DIRECT_NET_NUM].nmState =OSEKINDNM_NORMAL;
#if (STD_ON == OSEKNM_INDIRECT_STATE_CHANGE_INDICATION)
    OsekNm_ConfigPtr->osekNmIndirectCallBack(netId, OSEKINDNM_NORMAL );
#endif
}
#endif


#ifdef HOST_TEST
OsekIndNm_InternalNetType* readInternal_OsekNmIndirectStatus(void );
OsekIndNm_InternalNetType* readInternal_OsekNmIndirectStatus(void)
{
    return OsekNm_Indirect_NetChannels;
}
#endif


#endif










