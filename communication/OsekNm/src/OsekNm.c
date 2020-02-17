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
#include"OsekDirectNm.h"
#include"OsekIndirectNm.h"


#if (OSEKNM_DEV_ERROR_DETECT == STD_ON)
#include "Det.h"
#endif


#ifdef HOST_TEST
boolean nwRequestCalled;
boolean nwReleaseCalled;
#endif
/* Configuration of oseknm channel */
const OsekNm_ConfigType* OsekNm_ConfigPtr = NULL;
OsekNm_Internal_InitStatusType osekNmInitStatus =OSEKNM_STATUS_UNINIT;

#if !(((OSEKNM_SW_MAJOR_VERSION == 1) && (OSEKNM_SW_MINOR_VERSION == 0)) )
#error OsekNm: Configuration file expected BSW module version to be 1.0.*
#endif



/*lint -emacro(904,OSEKNM_DET_REPORTERROR) //904 PC-Lint exception to MISRA 14.7 (validate DET macros)*/
#if (OSEKNM_DEV_ERROR_DETECT == STD_ON)
#define OSEKNM_DET_REPORTERROR(_cond ,_api, _error, ...) \
    if (!_cond) { \
        (void)Det_ReportError(OSEKNM_MODULE_ID, 0, _api, _error); \
        return __VA_ARGS__; \
    }
#else
#define OSEKNM_DET_REPORTERROR(_cond ,_api, _error, ...) \
    if (!_cond) { \
        return __VA_ARGS__; \
    }
#endif

static Std_ReturnType OsekNm_Internal_ValidateNetId(const NetIdType netId);
static Std_ReturnType OsekNm_Internal_ValidateNmChannel(const NetworkHandleType nmHandle );

/**
 * @brief Get NetHandle Id from actual netId
 * @param netId
 * @return
 */
uint8 OsekNm_Internal_CheckNetId ( uint8 netId){

    uint8 nethandleId;
    nethandleId = (uint8)OSEKNM_INVALID_NETID;

    for (uint8 i = 0; i < OsekNm_ConfigPtr->osekNmNetworkCount; i++) {
        if(netId == OsekNmConfig.osekNmNetwork[i].netId) {
            nethandleId = (uint8)i;
            break;
        }
    }
    return nethandleId;
}

/**
 * @brief - To validated the correct net ID received.
 * @param netId
 * @return
 */
static Std_ReturnType OsekNm_Internal_ValidateNetId(const NetIdType netId){

    uint8 loopIndex;
    Std_ReturnType retValue;
    retValue = E_NOT_OK;

    for (loopIndex = 0; loopIndex < (OSEKNM_INDIRECT_NET_NUM+OSEKNM_DIRECT_NET_NUM); loopIndex++) {
        if(netId == OsekNmConfig.osekNmNetwork[loopIndex].netId){
            retValue = E_OK;
            break;
        }
    }
    return retValue;
}

/**
 * @brief - To validated the correct nm channel received.
 * @param nmHandle
 * @return
 */
static Std_ReturnType OsekNm_Internal_ValidateNmChannel(const NetworkHandleType nmHandle ){

    uint8 loopIndex;
    Std_ReturnType retValue;
    retValue = E_NOT_OK;

    for (loopIndex = 0; loopIndex < (OSEKNM_INDIRECT_NET_NUM+OSEKNM_DIRECT_NET_NUM); loopIndex++) {
        if(nmHandle == OsekNmConfig.osekNmNetwork[loopIndex].osekNmNetworkHandle) {
            retValue = E_OK;
            break;
        }
    }
    return retValue;
}

/**
 * @brief To get the networkHandle from NmNetwork handle
 * @param nmHandle
 * @return
 */
uint8 OsekNm_Internal_nmHandleToNetHandle( const NetworkHandleType nmHandle){

    uint8 nethandleId;
    nethandleId = (uint8)OSEKNM_INVALID_NETID;

    for (uint8 i = 0; i < OsekNm_ConfigPtr->osekNmNetworkCount; i++) {
      if(nmHandle == OsekNmConfig.osekNmNetwork[i].osekNmNetworkHandle) {
          nethandleId = OsekNmConfig.osekNmNetwork[i].networkHandle;
          break;
      }
    }
    return nethandleId;

}


/**
 * @brief Get Actual Node Id from nethandleId
 * @param nethandleId
 * @return
 */
uint8 OsekNm_Internal_GetNetId ( uint8 nethandleId){
    uint8 netId;
    netId = (uint8)OSEKNM_INVALID_NETID;

    for (uint8 i = 0; i < OsekNm_ConfigPtr->osekNmNetworkCount; i++) {
        if(nethandleId == OsekNmConfig.osekNmNetwork[i].networkHandle) {
            netId = OsekNmConfig.osekNmNetwork[i].netId;
            break;
        }
    }
    return netId;
}

/**
 * @brief function is to start of NM
 * @param oseknmConfigPtr
 */
void OsekNm_Init( const OsekNm_ConfigType * const oseknmConfigPtr ){

    uint8 net;
    NetIdType netId;
    OsekNm_ConfigPtr = oseknmConfigPtr;
    osekNmInitStatus = OSEKNM_STATUS_UNINIT;

    OSEKNM_DET_REPORTERROR((NULL != oseknmConfigPtr),OSEKNM_SERVICE_ID_INIT,OSEKNM_E_INVALID_POINTER);

    for (net=0; net < OsekNm_ConfigPtr->osekNmNetworkCount; net++){

        netId = OsekNm_Internal_GetNetId(net);

        if(OsekNm_ConfigPtr->osekNmNetwork[net].osekNmType == NM_INDIRECT){
#if (OSEKNM_INDIRECT_NET_NUM > 0)
            (void)OsekInDirectNm_StartNM(netId);
#endif
        }else if(OsekNm_ConfigPtr->osekNmNetwork->osekNmType == NM_DIRECT){
#if (OSEKNM_DIRECT_NET_NUM > 0)
            (void)OsekDirectNm_StartNM(netId);
#endif
        }else{
            /*do nothing */
        }
    }
    osekNmInitStatus =OSEKNM_STATUS_INIT;

}

/**
 * @brief to get version info of the oseknm Module
 * @param versioninfo
 */
#if ( OSEKNM_VERSION_INFO_API == STD_ON )
void OsekNm_GetVersionInfo(Std_VersionInfoType* versioninfo){

    OSEKNM_DET_REPORTERROR((NULL != versioninfo),OSEKNM_SERVICE_ID_GET_VERSION_INFO,OSEKNM_E_INVALID_POINTER);
    STD_GET_VERSION_INFO(versioninfo, OSEKNM);
    return;
}
#endif /* OSEKNM_VERSION_INFO_API */




/**
 * @brief function is to stop NM network
 * @param netId
 */
void OsekNm_DeInit(NetIdType netId){

    uint8 netHandle;
    Std_ReturnType status;

    OSEKNM_DET_REPORTERROR((OSEKNM_STATUS_INIT == osekNmInitStatus),OSEKNM_SERVICE_ID_DEINIT,OSEKNM_E_NOT_INITIALIZED);

    status = OsekNm_Internal_ValidateNetId(netId);
    /* If Net Id not found */
    OSEKNM_DET_REPORTERROR((status != E_NOT_OK),OSEKNM_SERVICE_ID_DEINIT,OSEKNM_E_INVALID_NETID);

    netHandle = OsekNm_Internal_CheckNetId(netId);
    if(OsekNm_ConfigPtr->osekNmNetwork[netHandle].osekNmType == NM_INDIRECT){
#if (OSEKNM_INDIRECT_NET_NUM > 0)
       (void)OsekInDirectNm_StopNM(netId);
#endif
    }else if(OsekNm_ConfigPtr->osekNmNetwork[netHandle].osekNmType == NM_DIRECT){
#if (OSEKNM_DIRECT_NET_NUM > 0)
       (void)OsekDirectNm_StopNM(netId);
#endif
     }else{
            /*do nothing */
     }

}




/**
 * @brief Transmission confirmation indication
 * @param netId
 * @param nodeId
 */
void OsekNm_TxConfirmation(NetIdType netId, NodeIdType nodeId){

    uint8 netHandle;
    Std_ReturnType status;
    OSEKNM_DET_REPORTERROR((OSEKNM_STATUS_INIT == osekNmInitStatus),OSEKNM_SERVICE_ID_TX_CONFIRMATION,OSEKNM_E_NOT_INITIALIZED);
    status = OsekNm_Internal_ValidateNetId(netId);
    /* If net Id not found */
    OSEKNM_DET_REPORTERROR((status != E_NOT_OK),OSEKNM_SERVICE_ID_TX_CONFIRMATION,OSEKNM_E_INVALID_NETID);

    netHandle = OsekNm_Internal_CheckNetId(netId);

    if(OsekNm_ConfigPtr->osekNmNetwork[netHandle].osekNmType == NM_INDIRECT){
#if (OSEKNM_INDIRECT_NET_NUM > 0)
        OsekInDirectNm_Com_MsgNotification(netId,nodeId);
#endif

    }else if(OsekNm_ConfigPtr->osekNmNetwork[netHandle].osekNmType == NM_DIRECT){
#if (OSEKNM_DIRECT_NET_NUM > 0)
        OsekDirectNm_TxConformation(netId);
        (void)nodeId; /* To avoid Lint warning */
#endif
    }else{

        /*do nothing */
    }

}

/**
 * @brief function is to indicate reception of NMPDU message
 * @param netId
 * @param nodeId
 * @param nmPdu
 */
void OsekNm_RxIndication(NetIdType netId,NodeIdType nodeId, const OsekNm_PduType* nmPdu){

    uint8 netHandle;
    Std_ReturnType status;
    OSEKNM_DET_REPORTERROR((OSEKNM_STATUS_INIT == osekNmInitStatus),OSEKNM_SERVICE_ID_RX_INDICATION,OSEKNM_E_NOT_INITIALIZED);
    status = OsekNm_Internal_ValidateNetId(netId);
    /* If net Id not found */
    OSEKNM_DET_REPORTERROR((status != E_NOT_OK),OSEKNM_SERVICE_ID_RX_INDICATION,OSEKNM_E_INVALID_NETID);

    OSEKNM_DET_REPORTERROR((NULL != nmPdu),OSEKNM_SERVICE_ID_RX_INDICATION,OSEKNM_E_INVALID_POINTER);

    netHandle = OsekNm_Internal_CheckNetId(netId);

    if(OsekNm_ConfigPtr->osekNmNetwork[netHandle].osekNmType == NM_INDIRECT){
#if (OSEKNM_INDIRECT_NET_NUM > 0)
        OsekInDirectNm_Com_MsgNotification(netId,nodeId);
        (void)*nmPdu;/* To avoid Lint warning */
#endif

    }else if(OsekNm_ConfigPtr->osekNmNetwork[netHandle].osekNmType == NM_DIRECT){
#if (OSEKNM_DIRECT_NET_NUM > 0)
        (void)nodeId;/* To avoid Lint warning */
        OsekDirectNm_RxIndication(netId, nmPdu);
#endif

    }else{

        /*do nothing */
    }
}

/**
 * @brief OsekNm_MainFuntion
 * @return void
 */
void OsekNm_MainFuntion(void){

    uint8 net;
    NetIdType netId;
    OSEKNM_DET_REPORTERROR((OSEKNM_STATUS_INIT == osekNmInitStatus),OSEKNM_SERVICE_ID_MAINFUNCTION,OSEKNM_E_NOT_INITIALIZED);
    for (net=0; net < OsekNm_ConfigPtr->osekNmNetworkCount; net++){

        netId = OsekNm_Internal_GetNetId(net);
        if(NM_INDIRECT == OsekNm_ConfigPtr->osekNmNetwork[net].osekNmType) {
#if (OSEKNM_INDIRECT_NET_NUM > 0)
            OsekInDirectNm_Processing(netId);
#endif
        }

        if(NM_DIRECT == OsekNm_ConfigPtr->osekNmNetwork[net].osekNmType) {
#if (OSEKNM_DIRECT_NET_NUM > 0)
            OsekDirectNm_MainFunction(netId);
#endif
         }
    }
}

/**
 * @brief Controller bus off indication
 * @param netId
 */
void OsekNm_ControllerBusOff(NetIdType netId){

    uint8 netHandle;
    Std_ReturnType status;
    OSEKNM_DET_REPORTERROR((OSEKNM_STATUS_INIT == osekNmInitStatus),OSEKNM_SERVICE_ID_CONTROLLER_BUSOFF,OSEKNM_E_NOT_INITIALIZED);
    status = OsekNm_Internal_ValidateNetId(netId);
    /* If net Id not found */
    OSEKNM_DET_REPORTERROR((status != E_NOT_OK),OSEKNM_SERVICE_ID_CONTROLLER_BUSOFF,OSEKNM_E_INVALID_NETID);

    netHandle = OsekNm_Internal_CheckNetId(netId);

    if(OsekNm_ConfigPtr->osekNmNetwork[netHandle].osekNmType == NM_INDIRECT){
#if (OSEKNM_INDIRECT_NET_NUM > 0)
        OsekInDirectNm_FatalBusError_Handle(netId);
#endif
    }else if(OsekNm_ConfigPtr->osekNmNetwork[netHandle].osekNmType == NM_DIRECT){
#if (OSEKNM_DIRECT_NET_NUM > 0)
        OsekDirectNm_ControllerBusOff(netId);
#endif
    }else{
        /*do nothing */
    }
}

/**
 * @brief timeout notification indication
 * @param netId
 * @param nodeId
 */
void OsekNm_TimeoutNotification(NetIdType netId,NodeIdType nodeId){

    uint8 netHandle;

    Std_ReturnType status;
    OSEKNM_DET_REPORTERROR((OSEKNM_STATUS_INIT == osekNmInitStatus),OSEKNM_SERVICE_ID_TIMEOUT_NOTIFICATION,OSEKNM_E_NOT_INITIALIZED);
    status = OsekNm_Internal_ValidateNetId(netId);
    /* If net Id not found */
    OSEKNM_DET_REPORTERROR((status != E_NOT_OK),OSEKNM_SERVICE_ID_TIMEOUT_NOTIFICATION,OSEKNM_E_INVALID_NETID);

    netHandle = OsekNm_Internal_CheckNetId(netId);
    if(OsekNm_ConfigPtr->osekNmNetwork[netHandle].osekNmType == NM_INDIRECT){
#if (OSEKNM_INDIRECT_NET_NUM > 0)
        OsekInDirectNm_comMsg_TimeoutHandle(netHandle,nodeId);
#endif
    }else{
#if (OSEKNM_DIRECT_NET_NUM > 0)
        (void)nodeId; /* Added to fix lint error Direct Nm will not use this API*/
#endif
    }
}

/**
 * @brief function to request for communication
 * @param nmHandle
 * @return
 */
Std_ReturnType OsekNm_NetworkRequest( const NetworkHandleType nmHandle ){

    uint8 netHandle;
    Std_ReturnType status;

    OSEKNM_DET_REPORTERROR((OSEKNM_STATUS_INIT == osekNmInitStatus),OSEKNM_SERVICE_ID_NETWORK_REQUEST,OSEKNM_E_NOT_INITIALIZED,E_NOT_OK);
    status = OsekNm_Internal_ValidateNmChannel(nmHandle);
    /* If nm network handle not found */
    OSEKNM_DET_REPORTERROR((status != E_NOT_OK),OSEKNM_SERVICE_ID_NETWORK_REQUEST,OSEKNM_E_INVALID_NMCHANNEL,E_NOT_OK);

    netHandle = OsekNm_Internal_nmHandleToNetHandle(nmHandle);
    if(OsekNm_ConfigPtr->osekNmNetwork[netHandle].osekNmType == NM_INDIRECT){
#if (OSEKNM_INDIRECT_NET_NUM > 0)
        status = OsekInDirectNm_GotoMode(netHandle,NM_AWAKE);
#endif
    }else if(OsekNm_ConfigPtr->osekNmNetwork[netHandle].osekNmType == NM_DIRECT){
#if (OSEKNM_DIRECT_NET_NUM > 0)
        status = OsekDirectNm_GotoMode(netHandle,NM_AWAKE);
#endif
    }else{
        /*do nothing */
    }
#ifdef HOST_TEST
    nwRequestCalled = TRUE;
#endif
    return status;
}

/**
 * @brief function is to release communication and goto sleep
 * @param nmHandle
 * @return
 */
Std_ReturnType OsekNm_NetworkRelease( const NetworkHandleType nmHandle ){

    uint8 netHandle;
    Std_ReturnType status;
    OSEKNM_DET_REPORTERROR((OSEKNM_STATUS_INIT == osekNmInitStatus),OSEKNM_SERVICE_ID_NETWORK_RELEASE,OSEKNM_E_NOT_INITIALIZED,E_NOT_OK);
    status = OsekNm_Internal_ValidateNmChannel(nmHandle);
    /* If nm network handle not found */
    OSEKNM_DET_REPORTERROR((status != E_NOT_OK),OSEKNM_SERVICE_ID_NETWORK_RELEASE,OSEKNM_E_INVALID_NMCHANNEL,E_NOT_OK);
    netHandle = OsekNm_Internal_nmHandleToNetHandle(nmHandle);
    if(OsekNm_ConfigPtr->osekNmNetwork[netHandle].osekNmType == NM_INDIRECT){
#if (OSEKNM_INDIRECT_NET_NUM > 0)
        status = OsekInDirectNm_GotoMode(netHandle,NM_BUSSLEEP);
#endif
    }else if(OsekNm_ConfigPtr->osekNmNetwork[netHandle].osekNmType == NM_DIRECT){
#if (OSEKNM_DIRECT_NET_NUM > 0)
        status = OsekDirectNm_GotoMode(netHandle,NM_BUSSLEEP);
#endif
    }else{
        /*do nothing */
    }
#ifdef HOST_TEST
    nwReleaseCalled = TRUE;
#endif
    return status;
}

/**
 * @brief Function to remember list of relevant configured nodes
 * @param netId
 * @param configKind
 * @param cMask
 */
void OsekNm_InitCMaskTable(NetIdType netId,ConfigKindName configKind, ConfigRefType cMask)
{
    uint8 netHandle;
    Std_ReturnType status;
    OSEKNM_DET_REPORTERROR((OSEKNM_STATUS_INIT == osekNmInitStatus),OSEKNM_SERVICE_ID_INIT_CMASK_TABLE,OSEKNM_E_NOT_INITIALIZED);
    status = OsekNm_Internal_ValidateNetId(netId);
    /* If net Id not found */
    OSEKNM_DET_REPORTERROR((status != E_NOT_OK),OSEKNM_SERVICE_ID_INIT_CMASK_TABLE,OSEKNM_E_INVALID_NETID);
    /*lint -e{685} range check for enum */
    OSEKNM_DET_REPORTERROR(((configKind >=NM_CKNORMAL)&&(configKind <=NM_CKLIMPHOME)),OSEKNM_SERVICE_ID_INIT_CMASK_TABLE,OSEKNM_E_INVALID_CONFIGKIND);
    OSEKNM_DET_REPORTERROR((NULL != cMask),OSEKNM_SERVICE_ID_INIT_CMASK_TABLE,OSEKNM_E_INVALID_POINTER);

    netHandle = OsekNm_Internal_CheckNetId(netId);
    if(OsekNm_ConfigPtr->osekNmNetwork[netHandle].osekNmType == NM_INDIRECT){
#if (OSEKNM_INDIRECT_NET_NUM > 0)
        OsekInDirectNm_InitCMaskTable(netId,configKind,cMask);
#endif
    }else if(OsekNm_ConfigPtr->osekNmNetwork[netHandle].osekNmType == NM_DIRECT){
#if (OSEKNM_DIRECT_NET_NUM > 0)
        OsekDirectNm_InitCMaskTable(netId,configKind,cMask);
#endif
    }else{
        /*do nothing */
    }
}/*lint !e818 STANDARDIZED_API */

/**
 * @brief InitTargetConfigTable
 * @param netId
 * @param configKind
 * @param targetConfig
 */
void OsekNm_InitTargetConfigTable(NetIdType netId,ConfigKindName configKind,ConfigRefType targetConfig)
{
    uint8 netHandle;
    Std_ReturnType status;
    OSEKNM_DET_REPORTERROR((OSEKNM_STATUS_INIT == osekNmInitStatus),OSEKNM_SERVICE_ID_INIT_TARGETCONFIG_TABLE,OSEKNM_E_NOT_INITIALIZED);
    status = OsekNm_Internal_ValidateNetId(netId);
    /* If net Id not found */
    OSEKNM_DET_REPORTERROR((status != E_NOT_OK),OSEKNM_SERVICE_ID_INIT_TARGETCONFIG_TABLE,OSEKNM_E_INVALID_NETID);
    /*lint -e{685} range check for enum */
    OSEKNM_DET_REPORTERROR(((configKind >=NM_CKNORMAL)&&(configKind <=NM_CKLIMPHOME)),OSEKNM_SERVICE_ID_INIT_TARGETCONFIG_TABLE,OSEKNM_E_INVALID_CONFIGKIND);
    OSEKNM_DET_REPORTERROR((NULL != targetConfig),OSEKNM_SERVICE_ID_INIT_TARGETCONFIG_TABLE,OSEKNM_E_INVALID_POINTER);

    netHandle = OsekNm_Internal_CheckNetId(netId);
    if(OsekNm_ConfigPtr->osekNmNetwork[netHandle].osekNmType == NM_INDIRECT){
#if (OSEKNM_INDIRECT_NET_NUM > 0)
        OsekInDirectNm_InitTargetConfigTable(netHandle,configKind,targetConfig);
#endif

    }else if(OsekNm_ConfigPtr->osekNmNetwork[netHandle].osekNmType == NM_DIRECT){
#if (OSEKNM_DIRECT_NET_NUM > 0)
        Direct_InitTargetConfigTable(netHandle,configKind,targetConfig);
#endif

    }else{

        /*do nothing */
    }
}/*lint !e818 STANDARDIZED_API */

/**
 * @brief Function is to check comparison of two configuration
 * @param netId
 * @param testConfig
 * @param refConfig
 * @param cMask
 * @return
 */
StatusType OsekNm_CmpConfig(NetIdType netId,ConfigRefType testConfig,ConfigRefType refConfig,ConfigRefType cMask){

    uint8 netHandle;
    StatusType status;
    OSEKNM_DET_REPORTERROR((OSEKNM_STATUS_INIT == osekNmInitStatus),OSEKNM_SERVICE_ID_CMP_CONFIG,OSEKNM_E_NOT_INITIALIZED,E_NOT_OK);
    status = OsekNm_Internal_ValidateNetId(netId);
    /* If net Id not found */
    OSEKNM_DET_REPORTERROR((status != E_NOT_OK),OSEKNM_SERVICE_ID_CMP_CONFIG,OSEKNM_E_INVALID_NETID,E_NOT_OK);
    OSEKNM_DET_REPORTERROR((NULL != testConfig),OSEKNM_SERVICE_ID_CMP_CONFIG,OSEKNM_E_INVALID_POINTER,E_NOT_OK);
    OSEKNM_DET_REPORTERROR((NULL != refConfig),OSEKNM_SERVICE_ID_CMP_CONFIG,OSEKNM_E_INVALID_POINTER,E_NOT_OK);
    OSEKNM_DET_REPORTERROR((NULL != cMask),OSEKNM_SERVICE_ID_CMP_CONFIG,OSEKNM_E_INVALID_POINTER,E_NOT_OK);

    netHandle = OsekNm_Internal_CheckNetId(netId);
    if(OsekNm_ConfigPtr->osekNmNetwork[netHandle].osekNmType == NM_INDIRECT){
#if (OSEKNM_INDIRECT_NET_NUM > 0)
        status = OsekInDirectNm_CmpConfig(netHandle,testConfig,refConfig,cMask);
#endif

    }else if(OsekNm_ConfigPtr->osekNmNetwork[netHandle].osekNmType == NM_DIRECT){
#if (OSEKNM_DIRECT_NET_NUM > 0)
        status = OsekDirectNm_CmpConfig(netHandle,testConfig,refConfig,cMask);
#endif

    }else{

        /*do nothing */
    }

    return status;

}

/**
 * @brief function is to make current configuration available
 * @param netId
 * @param Config
 * @param configKind
 * @return
 */
StatusType OsekNm_GetConfig(NetIdType netId,ConfigRefType config,ConfigKindName configKind){

    uint8 netHandle;
    StatusType status;
    OSEKNM_DET_REPORTERROR((OSEKNM_STATUS_INIT == osekNmInitStatus),OSEKNM_SERVICE_ID_GET_CONFIG,OSEKNM_E_NOT_INITIALIZED,E_NOT_OK);
    status = OsekNm_Internal_ValidateNetId(netId);
    /* If net Id not found */
    OSEKNM_DET_REPORTERROR((status != E_NOT_OK),OSEKNM_SERVICE_ID_GET_CONFIG,OSEKNM_E_INVALID_NETID,E_NOT_OK);
    OSEKNM_DET_REPORTERROR((NULL != config),OSEKNM_SERVICE_ID_GET_CONFIG,OSEKNM_E_INVALID_POINTER,E_NOT_OK);
    /*lint -e{685} range check for enum */
    OSEKNM_DET_REPORTERROR(((configKind >=NM_CKNORMAL)&&(configKind <=NM_CKLIMPHOME)),OSEKNM_SERVICE_ID_GET_CONFIG,OSEKNM_E_INVALID_CONFIGKIND,E_NOT_OK);

    netHandle = OsekNm_Internal_CheckNetId(netId);
    if(OsekNm_ConfigPtr->osekNmNetwork[netHandle].osekNmType == NM_INDIRECT){
#if (OSEKNM_INDIRECT_NET_NUM > 0)
        status = OsekInDirectNm_GetConfig(netHandle,config,configKind);
#endif

    }else if(OsekNm_ConfigPtr->osekNmNetwork[netHandle].osekNmType == NM_DIRECT){
#if (OSEKNM_DIRECT_NET_NUM > 0)
        status = OsekDirectNm_GetConfig(netHandle,config,configKind);
#endif
    }else{

        /*do nothing */
    }

    return status;
}

/**
 * @brief Function is to make current status available
 * @param netId
 * @param networkStatus
 * @return
 */
/*lint --e{9018}*/ /* 'networkStatus' with union based type 'StatusRefType' */
StatusType OsekNm_GetStatus(NetIdType netId, StatusRefType networkStatus) {
    uint8 netHandle;
    StatusType status;
    OSEKNM_DET_REPORTERROR((OSEKNM_STATUS_INIT == osekNmInitStatus),OSEKNM_SERVICE_ID_GET_STATUS,OSEKNM_E_NOT_INITIALIZED,E_NOT_OK);
    status = OsekNm_Internal_ValidateNetId(netId);
    /* If net Id not found */
    OSEKNM_DET_REPORTERROR((status != E_NOT_OK),OSEKNM_SERVICE_ID_GET_STATUS,OSEKNM_E_INVALID_NETID,E_NOT_OK);
    OSEKNM_DET_REPORTERROR((NULL != networkStatus),OSEKNM_SERVICE_ID_GET_STATUS,OSEKNM_E_INVALID_POINTER,E_NOT_OK);
    netHandle = OsekNm_Internal_CheckNetId(netId);
    if(OsekNm_ConfigPtr->osekNmNetwork[netHandle].osekNmType == NM_INDIRECT){
#if (OSEKNM_INDIRECT_NET_NUM > 0)
        status = OsekInDirectNm_GetStatus(netHandle,networkStatus);
#endif

    }else if(OsekNm_ConfigPtr->osekNmNetwork[netHandle].osekNmType == NM_DIRECT){
#if (OSEKNM_DIRECT_NET_NUM > 0)
        status = OsekDirectNm_GetStatus(netHandle,networkStatus);
#endif
    }else{

        /*do nothing */
    }
    return status;
    /*lint --e{715,818}*/ /* 'networkStatus' with union based type 'StatusRefType' */
}





















