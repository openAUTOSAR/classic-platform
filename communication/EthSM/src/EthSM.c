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

/* Globally fulfilled requirements */
/* General requirements */
/* @req 4.2.2/SWS_EthSM_00001 */ /* ETH SM is an abstract interface to ComM for COM control */
/* @req 4.2.2/SWS_EthSM_00002 */ /*The Ethernet SM doesn't directly access Ethernet hardware but by means of Eth_IF*/

/* @req 4.2.2/SWS_EthSM_00016 */ /* The state transition shall translate the request into a respective API call to control the assigned Ethernet peripherals*/

/* @req 4.2.2/SWS_EthSM_00019 */ /* assignment between network handles and transceivers is part of EthSM configuration*/
/* @req 4.2.2/SWS_EthSM_00020 */ /*EthSM controls the Ethernet transceivers depending on the state transitions*/
/* @req 4.2.2/SWS_EthSM_00021 */ /*EthSM will use the API of the EthIf for the control of the Ethernet transceiver modes*/

/* @req 4.2.2/SWS_EthSM_00022 */ /* EthSM controls the Ethernet controller modes of each Ethernet network*/
/* @req 4.2.2/SWS_EthSM_00023 */ /* EthSM shall use the API of the EthIf to control the operating modes of the Ethernet controllers*/

/* @req 4.2.2/SWS_EthSM_00078 */ /* Dummy mode requirement */

/* @req 4.2.2/SWS_EthSM_00085 */ /* If FULL_COMMUNICATION is requested then the Ethernet controller and the Ethernet transceiver are set to the state ACTIVE*/
/* @req 4.2.2/SWS_EthSM_00086 */ /* If NO_COMMUNICATION is requested then the Ethernet controller and the Ethernet transceiver are set to the state DOWN*/


/*==================[inclusions]==============================================*/
#include "ComStack_Types.h"
#include "EthSM.h"              /* @req 4.2.2/SWS_EthSM_00007 */
#include "ComM.h"               /* @req 4.2.2/SWS_EthSM_00189 */
#include "ComM_BusSM.h"         /* @req 4.2.2/SWS_EthSM_00013 */
#if defined(USE_DEM)
#include "Dem.h"
#endif
#if ETHSM_DUMMY_MODE == STD_OFF
#include "EthIf.h"              /* @req 4.2.2/SWS_EthSM_00010 */
#endif
#if defined(USE_BSWM)
#include "BswM_EthSM.h"          /* @req 4.2.2/SWS_EthSM_00080 */
#endif
#include "TcpIp_EthSM.h"         /* @req 4.2.2/SWS_EthSM_00106 */
#include "EthSM_Cbk.h"           /* Non ASR include */
#include "MemMap.h"

//lint -emacro(904,ETHSM_VALIDATE_NO_RV,ETHSM_VALIDATE_RV) //904 PC-Lint exception to MISRA 14.7 (validate DET macros).

/*==================[Macros needed]===========================================*/

#if ( ETHSM_DEV_ERROR_DETECT == STD_ON )
/* @req 4.2.2/SWS_EthSM_00008 */
#if defined(USE_DET)
#include "Det.h"
#else
#error "EthSM: DET must be used when DEV error detect is enabled"
#endif /*USE_DET */
#define DET_REPORT_ERROR(_api,_err) (void)Det_ReportError(ETHSM_MODULE_ID,0,(_api),(_err))

#define ETHSM_VALIDATE_RV(_exp,_api,_err,_rv) \
        if( !(_exp) ) { \
          DET_REPORT_ERROR(_api,_err); \
          return (_rv); \
        }

#define ETHSM_VALIDATE_NO_RV(_exp,_api,_err) \
        if( !(_exp) ) { \
          DET_REPORT_ERROR(_api,_err); \
          return; \
        }

#define ETHSM_VALIDATE(expression, serviceId, errorId, ...)	\
    if (!(expression)) {									\
        ETHSM_DET_REPORTERROR(serviceId, errorId);			\
        return __VA_ARGS__;									\
    }
#else
#define DET_REPORT_ERROR(_api,_err)
#define ETHSM_VALIDATE_RV(_exp,_api,_err,_rv)  \
         if( !(_exp) ) { \
             return (_rv); \
         }
#define ETHSM_VALIDATE_NO_RV(_exp,_api,_err)   \
         if( !(_exp) ) { \
           return; \
         }
#endif /*ETHSM_DEV_ERROR_DETECT*/

#ifndef DEM_EVENT_ID_NULL
#define DEM_EVENT_ID_NULL                       0u
#endif

#define ETHSM_INVALID_HANDLE                    0xFFu
#define ETHSM_INVALID_CTRL                      0xFFu

/********* Mode request owners ******************/
#define	ETHSM_COMM_MODE_REQUEST                 0u
#define	ETHSM_ETHIF_ETH_MODE_REQUEST            1u
#define	ETHSM_ETHIF_ETHTRCV_MODE_REQUEST        2u
#define ETHSM_TCPIP_MODE_REQUEST                3u
/********* Mode request owners ******************/

#define ETHSM_MODE_REQUEST_CLEAR(_handle, _owner)  \
    (EthSM_Internal.Networks[_handle].RequestStream &= (~(1u << _owner)))

#define ETHSM_MODE_REQUEST_SET(_handle, _owner )   \
    (EthSM_Internal.Networks[_handle].RequestStream |=  (1u << _owner))

#define ETHSM_MODE_POS(_owner )  (1u << _owner)

#define ETHSM_GET_CHANNEL(_handle )               \
     (EthSM_Internal.SMConfig->ConfigSet->Networks[_handle].ComMNetworkHandle)

#define ETHSM_GET_CTRL_IDX(_handle )               \
     (EthSM_Internal.SMConfig->ConfigSet->Networks[_handle].EthIfControllerId)

/*==================[external data]===========================================*/


/*==================[Static variables]===========================================*/

static EthSM_Internal_NetworkType EthSM_InternalNetworks[ETHSM_NETWORK_COUNT];
static EthSM_InternalType EthSM_Internal = {
        .InitStatus  = ETHSM_STATUS_UNINIT,
        .Networks    = EthSM_InternalNetworks,
        .SMConfig    = &EthSMConfig   /* @req 4.2.2/SWS_EthSM_00061 */
};

/*==================[Static functions]===========================================*/
#if ETHSM_DUMMY_MODE != STD_ON
#if (ETHSM_ETH_TRCV_SUPPORT == STD_ON)
/**
 * Function to Check Trcv link state Down/Active
 * @param NetworkHandle
 * @param LinkState
 * @param Flag
 * @return
 */
INLINE static boolean EthSM_Internal_CheckTrcvLink(NetworkHandleType NetworkHandle, EthTrcv_LinkStateType LinkState, boolean Active){
    boolean ret;

    if(TRUE == EthSMConfig.ConfigSet->Networks[NetworkHandle].EthTrcvAvailable){
        if(LinkState == EthSM_Internal.Networks[NetworkHandle].TrcvLinkState ){
            ret = TRUE;
        }else{
            ret = FALSE;
        }
    }else {
        /*lint -e{731} */
        if(TRUE == Active){
            ret = TRUE;   /* TRUE means By pass TRCV State making TRUE by default */
        }else {
            ret = FALSE;  /* FALSE means no need to enter TRCV related state since no Support */
        }
    }
    return ret;
}

/**
 * Function to check Ctrl Mode Down/Active
 * @param NetworkHandle
 * @param Mode
 * @return
 */
INLINE static boolean EthSM_Internal_CheckTrcvMode(NetworkHandleType NetworkHandle, EthTrcv_ModeType Mode){
    boolean ret;

    if(STD_ON == EthSMConfig.ConfigSet->Networks[NetworkHandle].EthTrcvAvailable){
        if(Mode == EthSM_Internal.Networks[NetworkHandle].TrcvMode ){
            ret = TRUE;
        }else {
            ret = FALSE;
        }
    }else {
        ret = TRUE;
    }
    return ret;
}
#endif
#endif
/**
 * Check Received CtrlIdx is configured in the configuration of the EthSM module
 * @param CtrlIdx
 * @return uint8
 */
static uint8 EthSM_Internal_CheckEthIfCtrl ( uint8 CtrlIdx){
    uint8 status;
    status = ETHSM_INVALID_CTRL;
    for (uint8 i = 0; i < ETHSM_NETWORK_COUNT; i++) {
        if(CtrlIdx == ETHSM_GET_CTRL_IDX(i)) {
            status = CtrlIdx;
            break;
        }
    }
    return status;
}

/**
 * Gives back with mapped Network handle of Ethernet interface controller index
 * @param CtrlIdx
 * @return NetworkHandleType
 */
static NetworkHandleType EthSM_Internal_GetNetworkHandleEth ( uint8 CtrlIdx){
    NetworkHandleType networkHandle;
    networkHandle = ETHSM_INVALID_HANDLE;
    for (uint8 i = 0; i < ETHSM_NETWORK_COUNT; i++) {
        if(CtrlIdx == ETHSM_GET_CTRL_IDX(i)) {
            networkHandle = i;
            break;
        }
    }return networkHandle;
}

/**
 * Gives back with mapped Network handle of associated global network / ComM
 * @param CtrlIdx
 * @return NetworkHandleType
 */
static NetworkHandleType EthSM_Internal_GetNetworkHandleComM ( NetworkHandleType Channel){

    NetworkHandleType status;
    status = ETHSM_INVALID_HANDLE;
    for (uint8 i = 0; i < ETHSM_NETWORK_COUNT; i++) {
        if(Channel == ETHSM_GET_CHANNEL(i)){
            status = (NetworkHandleType)i;
            break;
        }
    }
    return status;
}


/**
 * Change State/mode and notify same to the clients if any
 * @param NetworkHandle
 * @param State
 * @return None
 */
static void EthSM_Internal_ChangeNetworkModeState ( NetworkHandleType NetworkHandle,EthSM_NetworkModeStateType State){
    EthSM_Internal.Networks[NetworkHandle].NetworkMode  = State;
    switch (EthSM_Internal.Networks[NetworkHandle].NetworkMode){
    case ETHSM_STATE_OFFLINE:
    case ETHSM_STATE_WAIT_TRCVLINK:
    case ETHSM_STATE_WAIT_ONLINE:
        EthSM_Internal.Networks[NetworkHandle].CurrentMode = COMM_NO_COMMUNICATION;
        break;
    case ETHSM_STATE_ONLINE:
    case ETHSM_STATE_ONHOLD:
    case ETHSM_STATE_WAIT_OFFLINE:
        EthSM_Internal.Networks[NetworkHandle].CurrentMode = COMM_FULL_COMMUNICATION;
        break;
    default:
        break;
    }
#if defined(USE_BSWM)
    BswM_EthSM_CurrentState(EthSM_Internal.SMConfig->ConfigSet->Networks[NetworkHandle].ComMNetworkHandle,State);
#endif
}


/**
 * Enables or disables Ethernet controller and Ethernet tranciever,
 * loop backed here if dummy mode is ON
 * @param NetworkHandle
 * @param ComM_Mode
 * @return none
 */
static void EthSM_Internal_RequestEthIfModeChg(NetworkHandleType NetworkHandle) {
    if(((EthSM_Internal.Networks[NetworkHandle].RequestStream & (ETHSM_MODE_POS(ETHSM_ETHIF_ETH_MODE_REQUEST))) != (ETHSM_MODE_POS(ETHSM_ETHIF_ETH_MODE_REQUEST))) &&
    ((EthSM_Internal.Networks[NetworkHandle].RequestStream & (ETHSM_MODE_POS(ETHSM_ETHIF_ETHTRCV_MODE_REQUEST))) != (ETHSM_MODE_POS(ETHSM_ETHIF_ETHTRCV_MODE_REQUEST)))){
        ETHSM_MODE_REQUEST_SET(NetworkHandle,ETHSM_ETHIF_ETH_MODE_REQUEST);
#if (ETHSM_ETH_TRCV_SUPPORT == STD_ON)
        if(STD_ON == EthSM_Internal.SMConfig->ConfigSet->Networks[NetworkHandle].EthTrcvAvailable ){
            ETHSM_MODE_REQUEST_SET(NetworkHandle,ETHSM_ETHIF_ETHTRCV_MODE_REQUEST);
        }
#endif

#if ETHSM_DUMMY_MODE != STD_ON
        boolean status;
        status = TRUE;
        Std_ReturnType RetVal;
        uint8 CtrlIdx;
        Eth_ModeType CtrlMode;
        CtrlMode = ETH_MODE_DOWN;
#if (ETHSM_ETH_TRCV_SUPPORT == STD_ON)
        EthTrcv_ModeType TrcvMode;
        TrcvMode = ETHTRCV_MODE_DOWN;
#endif
        CtrlIdx = ETHSM_GET_CTRL_IDX(NetworkHandle);
        switch (EthSM_Internal.Networks[NetworkHandle].RequestedMode) {
            case COMM_NO_COMMUNICATION:
                CtrlMode = ETH_MODE_DOWN;
#if (ETHSM_ETH_TRCV_SUPPORT == STD_ON)
                TrcvMode = ETHTRCV_MODE_DOWN;
#endif
                break;
            case COMM_FULL_COMMUNICATION:
                CtrlMode = ETH_MODE_ACTIVE;
#if (ETHSM_ETH_TRCV_SUPPORT == STD_ON)
                TrcvMode = ETHTRCV_MODE_ACTIVE;
#endif
                break;
            default:
                status = FALSE;
                break;
        }
        if (status == TRUE) {
            RetVal = EthIf_SetControllerMode (CtrlIdx,CtrlMode);
            ETHSM_VALIDATE_NO_RV((RetVal == E_OK),ETHSM_GLOBAL_ID,ETHSM_E_REQ_ETH_MODE_CHG_RET_NOK);/* Retry in main loop is there */
#if (ETHSM_ETH_TRCV_SUPPORT == STD_ON)
            if(STD_ON == EthSM_Internal.SMConfig->ConfigSet->Networks[NetworkHandle].EthTrcvAvailable ){
                RetVal = EthIf_SetTransceiverMode(CtrlIdx,TrcvMode);
                ETHSM_VALIDATE_NO_RV((RetVal == E_OK),ETHSM_GLOBAL_ID,ETHSM_E_REQ_ETHTRCV_MODE_CHG_RET_NOK);/* Retry in main loop is there */
            }
#endif
        }
#endif
    }
}

/**
 * Requests mode change of TcpIp module
 * @param NetworkHandle
 * @param State
 * @return none
 */
static void EthSM_Internal_RequestTcpIpModeChg( NetworkHandleType NetworkHandle,TcpIp_StateType State){
  if((EthSM_Internal.Networks[NetworkHandle].RequestStream & (ETHSM_MODE_POS(ETHSM_TCPIP_MODE_REQUEST))) != (ETHSM_MODE_POS(ETHSM_TCPIP_MODE_REQUEST))){
      ETHSM_MODE_REQUEST_SET(NetworkHandle,ETHSM_TCPIP_MODE_REQUEST);
#if !defined(CFG_ETHSM_TCPIP_NO_SYNC)
      Std_ReturnType RetVal;
      RetVal = TcpIp_RequestComMode(ETHSM_GET_CTRL_IDX(NetworkHandle),State);
      ETHSM_VALIDATE_NO_RV((RetVal == E_OK),ETHSM_GLOBAL_ID,ETHSM_E_REQ_TCPIP_MODE_CHG_RET_NOK);/* Retry in main loop is there  */
#else
      EthSM_Internal.Networks[NetworkHandle].TcpIpState = State;
#endif
  }
}

/**
 * Gets and updates the individual controller modes of mapped Ethernet interface
 * @param NetworkHandle
 * @return none
 */
static void EthSM_Internal_UpdateControllerMode(NetworkHandleType NetworkHandle){
#if ETHSM_DUMMY_MODE != STD_ON
    uint8 CtrlIdx;
    Std_ReturnType RetVal;
    Eth_ModeType CtrlMode;
#if (ETHSM_ETH_TRCV_SUPPORT == STD_ON)
    EthTrcv_ModeType TrcvMode;
#endif
    CtrlIdx = ETHSM_GET_CTRL_IDX(NetworkHandle);
    RetVal = EthIf_GetControllerMode(CtrlIdx,&CtrlMode);
    if(RetVal == E_OK){
        EthSM_Internal.Networks[NetworkHandle].CtrlMode  = CtrlMode;
    }else{
        /*lint -e{506,774} */
        DET_REPORT_ERROR(ETHSM_GLOBAL_ID,ETHSM_E_GET_ETH_MODE_RET_NOK);/* Retry in main loop is there */
        /*lint -e{904} Return statement is necessary in case of reporting a DET error */
        return;
    }
#if (ETHSM_ETH_TRCV_SUPPORT == STD_ON)
    if(STD_ON == EthSMConfig.ConfigSet->Networks[NetworkHandle].EthTrcvAvailable){
        RetVal = EthIf_GetTransceiverMode(CtrlIdx,&TrcvMode);
        if(RetVal == E_OK){
            EthSM_Internal.Networks[NetworkHandle].TrcvMode  = TrcvMode;
        }else{
            /*lint -e{506,774} */
            DET_REPORT_ERROR(ETHSM_GLOBAL_ID,ETHSM_E_GET_ETHTRCV_MODE_RET_NOK);/* Retry in main loop is there */
            /*lint -e{904} Return statement is necessary in case of reporting a DET error */
            return;
        }
    }
#endif
#else
    (void)NetworkHandle;
#endif
}

/**
 * Sub Main function to carry out FULL communication WaitOnline finite state machine
 * @param NetworkHandle
 * @return none
 */
static void EthSM_Internal_FullComWaitOnlineSM(NetworkHandleType NetworkHandle){
    boolean ret;
    /* @req 4.2.2/SWS_EthSM_00136 */
    /* @req 4.2.2/SWS_EthSM_00137 */
    /* @req 4.2.2/SWS_EthSM_00138 */
#if ETHSM_DUMMY_MODE == STD_ON
    ret = FALSE;
#endif
    NetworkHandleType Channel = ETHSM_GET_CHANNEL(NetworkHandle);
#if ETHSM_DUMMY_MODE != STD_ON
#if (ETHSM_ETH_TRCV_SUPPORT == STD_ON)
    ret = EthSM_Internal_CheckTrcvLink(NetworkHandle, ETHTRCV_LINK_STATE_DOWN, FALSE);
#else
    ret = FALSE;
#endif
#endif
    /*lint -e{731,774} */
    if(TRUE == ret){
        if(TCPIP_STATE_OFFLINE == EthSM_Internal.Networks[NetworkHandle].TcpIpState ){
            EthSM_Internal_ChangeNetworkModeState(NetworkHandle,ETHSM_STATE_WAIT_TRCVLINK);
        }else{
            EthSM_Internal_RequestTcpIpModeChg(NetworkHandle,TCPIP_STATE_OFFLINE);
        }
    }
    /* @req 4.2.2/SWS_EthSM_00146 */
    /* @req 4.2.2/SWS_EthSM_00148 */
    /* @req 4.2.2/SWS_EthSM_00150 */
    else if(EthSM_Internal.Networks[NetworkHandle].TcpIpState == TCPIP_STATE_ONLINE){
        EthSM_Internal_ChangeNetworkModeState(NetworkHandle,ETHSM_STATE_ONLINE);
        ComM_BusSM_ModeIndication(Channel, &EthSM_Internal.Networks[NetworkHandle].CurrentMode);
    }
    else{
        /* Do nothing */
    }
}

/**
 * Sub Main function to carry out FULL communication Online finite state machine
 * @param NetworkHandle
 * @return none
 */
static void EthSM_Internal_FullComOnlineSM(NetworkHandleType NetworkHandle){
    /* @req 4.2.2/SWS_EthSM_00166 */
    /* @req 4.2.2/SWS_EthSM_00167 */
    /* @req 4.2.2/SWS_EthSM_00168 */
    boolean ret;
#if ETHSM_DUMMY_MODE == STD_ON
    ret = FALSE;
#endif

#if ETHSM_DUMMY_MODE != STD_ON
#if (ETHSM_ETH_TRCV_SUPPORT == STD_ON)
    ret = EthSM_Internal_CheckTrcvLink(NetworkHandle, ETHTRCV_LINK_STATE_DOWN, FALSE);
#else
    ret = FALSE;
#endif
#endif
    NetworkHandleType Channel = ETHSM_GET_CHANNEL(NetworkHandle);
    /*lint -e{731,774} */
    if(TRUE == ret){
        /* @req 4.2.2/SWS_EthSM_00188 */
#if defined(USE_DEM)
        if(DEM_EVENT_ID_NULL != EthSM_Internal.SMConfig->ConfigSet->Networks[NetworkHandle].ETHSM_E_LINK_DOWN) {
            Dem_ReportErrorStatus(EthSM_Internal.SMConfig->ConfigSet->Networks[NetworkHandle].ETHSM_E_LINK_DOWN, DEM_EVENT_STATUS_FAILED);
        }
#endif
        if(EthSM_Internal.Networks[NetworkHandle].TcpIpState == TCPIP_STATE_ONHOLD){
            EthSM_Internal_ChangeNetworkModeState(NetworkHandle,ETHSM_STATE_ONHOLD);
        }else{
            EthSM_Internal_RequestTcpIpModeChg(NetworkHandle,TCPIP_STATE_ONHOLD);
        }
    }
    /* @req 4.2.2/SWS_EthSM_00151 *//* @req 4.2.2/SWS_EthSM_00152 *//* @req 4.2.2/SWS_EthSM_00154 */
    if(EthSM_Internal.Networks[NetworkHandle].TcpIpState == TCPIP_STATE_OFFLINE){
        EthSM_Internal_ChangeNetworkModeState(NetworkHandle,ETHSM_STATE_WAIT_ONLINE);
        /* @req 4.2.2/SWS_EthSM_00018 */
        ComM_BusSM_ModeIndication(Channel, &EthSM_Internal.Networks[NetworkHandle].CurrentMode);
        /* Note: The ntw state will reach ETHSM_STATE_WAIT_ONLINE, who will set up TCP active again?,
         * no specification in Autosar, it will be hung in ETHSM_STATE_WAIT_ONLINE as long as link state is active */
    }
    else{
        /* Do nothing */
    }
}

/**
 * Sub Main function to carry out FULL communication OnHold finite state machine
 * @param NetworkHandle
 * @return none
 */
static void EthSM_Internal_FullComOnHoldSM(NetworkHandleType NetworkHandle){
    /* @req 4.2.2/SWS_EthSM_00174 */
    /* @req 4.2.2/SWS_EthSM_00175 */
    /* @req 4.2.2/SWS_EthSM_00177 */
    boolean ret;
#if ETHSM_DUMMY_MODE == STD_ON
    ret = TRUE;
#endif

#if ETHSM_DUMMY_MODE != STD_ON
#if (ETHSM_ETH_TRCV_SUPPORT == STD_ON)
    ret = EthSM_Internal_CheckTrcvLink(NetworkHandle, ETHTRCV_LINK_STATE_ACTIVE, TRUE);
#else
    ret = TRUE;
#endif
#endif
    NetworkHandleType Channel = ETHSM_GET_CHANNEL(NetworkHandle);
    if(EthSM_Internal.Networks[NetworkHandle].TcpIpState == TCPIP_STATE_OFFLINE){
        EthSM_Internal_ChangeNetworkModeState(NetworkHandle,ETHSM_STATE_WAIT_TRCVLINK);
        ComM_BusSM_ModeIndication(Channel, &EthSM_Internal.Networks[NetworkHandle].CurrentMode);
    }
    /*lint -e731 *//*lint -e774 */
    else if (TRUE == ret){
        /* @req 4.2.2/SWS_EthSM_00170 */
        /* @req 4.2.2/SWS_EthSM_00171 */
        /* @req 4.2.2/SWS_EthSM_00172 */
        /* @req 4.2.2/SWS_EthSM_00196 */
#if defined(USE_DEM)
        if(DEM_EVENT_ID_NULL != EthSM_Internal.SMConfig->ConfigSet->Networks[NetworkHandle].ETHSM_E_LINK_DOWN) {
            Dem_ReportErrorStatus(EthSM_Internal.SMConfig->ConfigSet->Networks[NetworkHandle].ETHSM_E_LINK_DOWN, DEM_EVENT_STATUS_PASSED);
        }
#endif
        if(EthSM_Internal.Networks[NetworkHandle].TcpIpState == TCPIP_STATE_ONLINE){
            EthSM_Internal_ChangeNetworkModeState(NetworkHandle,ETHSM_STATE_ONLINE);
        }else{
            EthSM_Internal_RequestTcpIpModeChg(NetworkHandle,TCPIP_STATE_ONLINE);
        }
    }
    else{
        /* Do nothing */
    }
}

/*lint -save -e522  EthSM_Internal_UpdateControllerMode() gets compliant because of dummy mode */
/**
 * Sub Main function to carry out FULL communication finite state machine
 * @param NetworkHandle
 * @param State
 * @return none
 */
static void EthSM_Internal_FullComFSM(NetworkHandleType NetworkHandle){
    boolean ret;
    ret = TRUE;
    switch (EthSM_Internal.Networks[NetworkHandle].NetworkMode){
    case ETHSM_STATE_OFFLINE:
        /* @req 4.2.2/SWS_EthSM_00026 */
        /* @req 4.2.2/SWS_EthSM_00088 */
        /* @req 4.2.2/SWS_EthSM_00089 */
        /* @req 4.2.2/SWS_EthSM_00097 */
        EthSM_Internal_UpdateControllerMode(NetworkHandle);
#if ETHSM_DUMMY_MODE != STD_ON
#if (ETHSM_ETH_TRCV_SUPPORT == STD_ON)
        ret = EthSM_Internal_CheckTrcvMode(NetworkHandle, ETHTRCV_MODE_ACTIVE);
#else
        ret = TRUE;
#endif
#endif
        /*lint -e{731,774} */
        if((TRUE == ret) && (EthSM_Internal.Networks[NetworkHandle].CtrlMode == ETH_MODE_ACTIVE)){
            EthSM_Internal_ChangeNetworkModeState(NetworkHandle,ETHSM_STATE_WAIT_TRCVLINK);
        }else{
            EthSM_Internal_RequestEthIfModeChg(NetworkHandle);
        }
        break;
    case ETHSM_STATE_WAIT_TRCVLINK:
        /* @req 4.2.2/SWS_EthSM_00132 */
        /* @req 4.2.2/SWS_EthSM_00133 */
        /* @req 4.2.2/SWS_EthSM_00134 */
#if ETHSM_DUMMY_MODE != STD_ON
#if (ETHSM_ETH_TRCV_SUPPORT == STD_ON)
        ret = EthSM_Internal_CheckTrcvLink(NetworkHandle, ETHTRCV_LINK_STATE_ACTIVE, TRUE);
#else
        ret = TRUE;
#endif
#endif
        /*lint -e{731,774} */
        if(TRUE == ret){
            if(EthSM_Internal.Networks[NetworkHandle].TcpIpState == TCPIP_STATE_ONLINE){
                EthSM_Internal_ChangeNetworkModeState(NetworkHandle,ETHSM_STATE_WAIT_ONLINE);
            }else{
                EthSM_Internal_RequestTcpIpModeChg(NetworkHandle,TCPIP_STATE_ONLINE);
            }
        }
        break;
    case ETHSM_STATE_WAIT_ONLINE:
        EthSM_Internal_FullComWaitOnlineSM(NetworkHandle);
        break;
    case ETHSM_STATE_ONLINE:
        EthSM_Internal_FullComOnlineSM(NetworkHandle);
        break;
    case ETHSM_STATE_ONHOLD:
        EthSM_Internal_FullComOnHoldSM(NetworkHandle);
        break;
    default:
        break;
    }
}

/**
 * Sub Main function to transfer to offline mode
 * @param NetworkHandle
 * @return none
 */
static void EthSM_Internal_TransferOffline(NetworkHandleType NetworkHandle){
    boolean ret;
#if ETHSM_DUMMY_MODE == STD_ON
    ret = TRUE;
#endif

    EthSM_Internal_UpdateControllerMode(NetworkHandle);
#if ETHSM_DUMMY_MODE != STD_ON
#if (ETHSM_ETH_TRCV_SUPPORT == STD_ON)
    ret = EthSM_Internal_CheckTrcvMode(NetworkHandle, ETHTRCV_MODE_DOWN);
#else
        ret = TRUE;
#endif
#endif
    if(EthSM_Internal.Networks[NetworkHandle].TcpIpState == TCPIP_STATE_OFFLINE){
        if((EthSM_Internal.Networks[NetworkHandle].CtrlMode == ETH_MODE_DOWN) &&
           /*lint -e{731,774} */
           (TRUE == ret)){
            EthSM_Internal_ChangeNetworkModeState(NetworkHandle,ETHSM_STATE_OFFLINE);
        }else{
            EthSM_Internal_RequestEthIfModeChg(NetworkHandle);
        }
    }else{
        EthSM_Internal_RequestTcpIpModeChg(NetworkHandle,TCPIP_STATE_OFFLINE);
    }
}

/**
 * Sub Main function to carry out NO communication finite state machine
 * @param NetworkHandle
 * @param State
 * @return none
 */
static void EthSM_Internal_NoComFSM(NetworkHandleType NetworkHandle){
    boolean ret;
    NetworkHandleType Channel;
    ret = TRUE;
    Channel = ETHSM_GET_CHANNEL(NetworkHandle);
    switch (EthSM_Internal.Networks[NetworkHandle].NetworkMode){
    case ETHSM_STATE_WAIT_TRCVLINK:
        /* @req 4.2.2/SWS_EthSM_00127 */
        /* @req 4.2.2/SWS_EthSM_00128 */
        /* @req 4.2.2/SWS_EthSM_00129 */
        /* @req 4.2.2/SWS_EthSM_00130 */
        EthSM_Internal_UpdateControllerMode(NetworkHandle);
#if ETHSM_DUMMY_MODE != STD_ON
#if (ETHSM_ETH_TRCV_SUPPORT == STD_ON)
        ret = EthSM_Internal_CheckTrcvMode(NetworkHandle, ETHTRCV_MODE_DOWN);
#else
        ret = TRUE;
#endif
#endif
        if((EthSM_Internal.Networks[NetworkHandle].CtrlMode == ETH_MODE_DOWN) &&
                /*lint -e{731,774} */
                (TRUE == ret)){
            EthSM_Internal_ChangeNetworkModeState(NetworkHandle,ETHSM_STATE_OFFLINE);
        }else{
            EthSM_Internal_RequestEthIfModeChg(NetworkHandle);
        }
        break;
    case ETHSM_STATE_WAIT_ONLINE:
        /* @req 4.2.2/SWS_EthSM_00140 */
        /* @req 4.2.2/SWS_EthSM_00141 */
        /* @req 4.2.2/SWS_EthSM_00142 */
        /* @req 4.2.2/SWS_EthSM_00143 */
        /* @req 4.2.2/SWS_EthSM_00144 */
        EthSM_Internal_TransferOffline(NetworkHandle);
        break;
    case ETHSM_STATE_ONLINE:
        /* @req 4.2.2/SWS_EthSM_00155 */
        /* @req 4.2.2/SWS_EthSM_00157 */
        /* @req 4.2.2/SWS_EthSM_00158 */
        if(EthSM_Internal.Networks[NetworkHandle].TcpIpState == TCPIP_STATE_OFFLINE){
            EthSM_Internal_ChangeNetworkModeState(NetworkHandle,ETHSM_STATE_WAIT_OFFLINE);
        }else{
            EthSM_Internal_RequestTcpIpModeChg(NetworkHandle,TCPIP_STATE_OFFLINE);
        }
        break;
    case ETHSM_STATE_ONHOLD:
        /* @req 4.2.2/SWS_EthSM_00178 */
        /* @req 4.2.2/SWS_EthSM_00179 */
        /* @req 4.2.2/SWS_EthSM_00180 */
        /* @req 4.2.2/SWS_EthSM_00181 */
        /* @req 4.2.2/SWS_EthSM_00182 */
        /* @req 4.2.2/SWS_EthSM_00184 */
        EthSM_Internal_TransferOffline(NetworkHandle);
        if(EthSM_Internal.Networks[NetworkHandle].NetworkMode == ETHSM_STATE_ONHOLD){
            ComM_BusSM_ModeIndication(Channel, &EthSM_Internal.Networks[NetworkHandle].CurrentMode);
        }
        break;
    case ETHSM_STATE_WAIT_OFFLINE:
        /* @req 4.2.2/SWS_EthSM_00160 */
        /* @req 4.2.2/SWS_EthSM_00161 */
        /* @req 4.2.2/SWS_EthSM_00162 */
        /* @req 4.2.2/SWS_EthSM_00163 */
        /* @req 4.2.2/SWS_EthSM_00165 */
        EthSM_Internal_UpdateControllerMode(NetworkHandle);
#if ETHSM_DUMMY_MODE != STD_ON
#if (ETHSM_ETH_TRCV_SUPPORT == STD_ON)
        ret = EthSM_Internal_CheckTrcvMode(NetworkHandle, ETHTRCV_MODE_DOWN);
#else
            ret= TRUE;
#endif
#endif
        if(EthSM_Internal.Networks[NetworkHandle].TcpIpState == TCPIP_STATE_OFFLINE){
            if((EthSM_Internal.Networks[NetworkHandle].CtrlMode == ETH_MODE_DOWN) &&
               /*lint -e{731,774} */
               (TRUE == ret)){
                EthSM_Internal_ChangeNetworkModeState(NetworkHandle,ETHSM_STATE_OFFLINE);
                /* @req 4.2.2/SWS_EthSM_00018 */
                ComM_BusSM_ModeIndication(Channel, &EthSM_Internal.Networks[NetworkHandle].CurrentMode);
            }else{
                EthSM_Internal_RequestEthIfModeChg(NetworkHandle);
            }
        }
        break;
    case ETHSM_STATE_OFFLINE:
        break;
    default:
        break;
    }
}
/*lint -restore*/
/*==================[AUTOSAR APIs]===========================================*/

/**
 * Initializes the Ethernet State Manager
 * @param void
 * @return none
 */
/* @req 4.2.2/SWS_EthSM_00043 */
void EthSM_Init(void) {
    /* @req 4.2.2/SWS_EthSM_00025 */
    for (uint8 NetworkHandle = 0; NetworkHandle < ETHSM_NETWORK_COUNT; NetworkHandle++) {
        EthSM_Internal.Networks[NetworkHandle].RequestedMode = COMM_NO_COMMUNICATION;
        EthSM_Internal.Networks[NetworkHandle].CurrentMode   = COMM_NO_COMMUNICATION;
        EthSM_Internal.Networks[NetworkHandle].NetworkMode   = ETHSM_STATE_OFFLINE;
        EthSM_Internal.Networks[NetworkHandle].TrcvLinkState = ETHTRCV_LINK_STATE_DOWN;
        EthSM_Internal.Networks[NetworkHandle].CtrlMode      = ETH_MODE_DOWN;
        EthSM_Internal.Networks[NetworkHandle].TrcvMode      = ETHTRCV_MODE_DOWN;
        EthSM_Internal.Networks[NetworkHandle].RequestStream = 0;
        EthSM_Internal.Networks[NetworkHandle].TcpIpState    = TCPIP_STATE_OFFLINE;
    }
    EthSM_Internal.InitStatus = ETHSM_STATUS_INIT;
}

/* @req 4.2.2/SWS_EthSM_00046*/
#if (ETHSM_VERSION_INFO_API == STD_ON)
/**
 * Returns the version information of this module
 * @param VersionInfoPtr
 */
void EthSM_GetVersionInfo(Std_VersionInfoType* VersionInfoPtr)
{
    ETHSM_VALIDATE_NO_RV((NULL != VersionInfoPtr), ETHSM_GETVERSIONINFO_ID, ETHSM_E_PARAM_POINTER); /* not requested DET error */
    /* Vendor and Module Id */
    VersionInfoPtr->vendorID = ETHSM_VENDOR_ID;
    VersionInfoPtr->moduleID = ETHSM_MODULE_ID;

    /* software version */
    VersionInfoPtr->sw_major_version = ETHSM_SW_MAJOR_VERSION;
    VersionInfoPtr->sw_minor_version = ETHSM_SW_MINOR_VERSION;
    VersionInfoPtr->sw_patch_version = ETHSM_SW_PATCH_VERSION;
}
#endif

/**
 * Available to request corresponding network communication mode
 * Depending up on the parameters handed over, ETHSM executes state machine change
 * of the network
 * @param NetworkHandle
 * @param ComM_Mode
 * @return E_OK or E_NOT_OK
 */
/* @req 4.2.2/SWS_EthSM_00050 */
/* @req 4.2.2/SWS_EthSM_00014 */
Std_ReturnType EthSM_RequestComMode( NetworkHandleType NetworkHandle, ComM_ModeType ComM_Mode ) {
    /* @req 4.2.2/SWS_EthSM_00054 */
    /* @req 4.2.2/SWS_EthSM_00087 */
    NetworkHandleType EthSMNetworkHandle = EthSM_Internal_GetNetworkHandleComM(NetworkHandle);
    ETHSM_VALIDATE_RV((EthSM_Internal.InitStatus == ETHSM_STATUS_INIT),ETHSM_REQUESTCOMMODE_ID,ETHSM_E_UNINIT, E_NOT_OK);
    /* @req 4.2.2/SWS_EthSM_00051 */
    /* @req 4.2.2/SWS_EthSM_00052 */
    ETHSM_VALIDATE_RV((EthSMNetworkHandle < ETHSM_NETWORK_COUNT),ETHSM_REQUESTCOMMODE_ID,ETHSM_E_INVALID_NETWORK_HANDLE,E_NOT_OK);
    /* @req 4.2.2/SWS_EthSM_00095 */
    /* @req 4.2.2/SWS_EthSM_00199 */
    ETHSM_VALIDATE_RV((ComM_Mode <= COMM_FULL_COMMUNICATION),ETHSM_REQUESTCOMMODE_ID,ETHSM_E_INVALID_NETWORK_MODE,E_NOT_OK);
    /* @req 4.2.2/SWS_EthSM_00053 */
    EthSM_Internal.Networks[EthSMNetworkHandle].RequestedMode = ComM_Mode;

    /* @req 4.2.2/SWS_EthSM_00078 */
#if ETHSM_DUMMY_MODE == STD_ON /*IMPROVEMENT:  Change these items later  */
    if(ComM_Mode == COMM_FULL_COMMUNICATION){
        EthSM_Internal.Networks[EthSMNetworkHandle].CtrlMode = ETH_MODE_ACTIVE;
        EthSM_Internal.Networks[EthSMNetworkHandle].TrcvMode = ETHTRCV_MODE_ACTIVE;

    }else if(ComM_Mode == COMM_NO_COMMUNICATION){
        EthSM_Internal.Networks[EthSMNetworkHandle].CtrlMode = ETH_MODE_DOWN;
        EthSM_Internal.Networks[EthSMNetworkHandle].TrcvMode = ETHTRCV_MODE_DOWN;
    }else{
        /* Do nothing */
    }
#endif
    return E_OK;
}

/**
 * Available to get the corresponding network communication mode
 * @param NetworkHandle
 * @param ComM_ModePtr
 * @return E_OK or E_NOT_OK
 */
/* @req 4.2.2/SWS_EthSM_00055 */
/* @req 4.2.2/SWS_EthSM_00017 */
Std_ReturnType EthSM_GetCurrentComMode( NetworkHandleType NetworkHandle, ComM_ModeType* ComM_ModePtr ) {
    NetworkHandleType EthSMNetworkHandle = EthSM_Internal_GetNetworkHandleComM(NetworkHandle);
    /* @req 4.2.2/SWS_EthSM_00060 */
    ETHSM_VALIDATE_RV((EthSM_Internal.InitStatus == ETHSM_STATUS_INIT),ETHSM_GETCURRENTCOMMODE_ID,ETHSM_E_UNINIT, E_NOT_OK);
    /* @req 4.2.2/SWS_EthSM_00057 */
    /* @req 4.2.2/SWS_EthSM_00058 */
    ETHSM_VALIDATE_RV((EthSMNetworkHandle < ETHSM_NETWORK_COUNT),ETHSM_GETCURRENTCOMMODE_ID,ETHSM_E_INVALID_NETWORK_HANDLE,E_NOT_OK);

    ETHSM_VALIDATE_RV((ComM_ModePtr != NULL),ETHSM_GETCURRENTCOMMODE_ID,ETHSM_E_PARAM_POINTER,E_NOT_OK);
    /* @req 4.2.2/SWS_EthSM_00059 */
    *ComM_ModePtr = EthSM_Internal.Networks[EthSMNetworkHandle].CurrentMode;

    return E_OK;
}

#if ETHSM_DUMMY_MODE != STD_ON
/**
 * Ethernet Interface reports a transceiver link state change.
 * @param CtrlIdx
 * @param TransceiverLinkState
 * @return none
 */
/* @req 4.2.2/SWS_EthSM_00109*/
void EthSM_TrcvLinkStateChg( uint8 CtrlIdx,EthTrcv_LinkStateType TransceiverLinkState ){
    /* @req 4.2.2/SWS_EthSM_00115*/
    ETHSM_VALIDATE_NO_RV((EthSM_Internal.InitStatus == ETHSM_STATUS_INIT),ETHSM_TRCVLINKSTATECHG_ID,ETHSM_E_UNINIT);

    /* @req 4.2.2/SWS_EthSM_00112*/
    ETHSM_VALIDATE_NO_RV((ETHSM_INVALID_CTRL != EthSM_Internal_CheckEthIfCtrl(CtrlIdx)),ETHSM_TRCVLINKSTATECHG_ID,ETHSM_E_PARAM_CONTROLLER);

    ETHSM_VALIDATE_NO_RV((TransceiverLinkState <= ETHTRCV_LINK_STATE_ACTIVE),ETHSM_TRCVLINKSTATECHG_ID, ETHSM_E_INVALID_TRCV_LINK_STATE);/*lint !e685 '<=' has no problem */

    /* @req 4.2.2/SWS_EthSM_00114*/
    EthSM_Internal.Networks[EthSM_Internal_GetNetworkHandleEth(CtrlIdx)].TrcvLinkState = TransceiverLinkState;
}
#endif
/**
 * This service is called by the TcpIp to report the actual TcpIp state (e.g. online, offline).
 * @param CtrlIdx
 * @param TcpIpState
 * @return E_OK or E_NOT_OK
 */
/* @req 4.2.2/SWS_EthSM_00110*/
Std_ReturnType EthSM_TcpIpModeIndication( uint8 CtrlIdx, TcpIp_StateType TcpIpState ){
    NetworkHandleType NetworkHandle;

    /* @req 4.2.2/SWS_EthSM_00120 */
    ETHSM_VALIDATE_RV((EthSM_Internal.InitStatus == ETHSM_STATUS_INIT),ETHSM_TCPIPMODEINDICATION_ID,ETHSM_E_UNINIT, E_NOT_OK);

    /* @req 4.2.2/SWS_EthSM_00116 */
    ETHSM_VALIDATE_RV((ETHSM_INVALID_CTRL != EthSM_Internal_CheckEthIfCtrl(CtrlIdx)),ETHSM_TCPIPMODEINDICATION_ID,ETHSM_E_PARAM_CONTROLLER, E_NOT_OK);

    /* @req 4.2.2/SWS_EthSM_00118 */
    ETHSM_VALIDATE_RV((TcpIpState <= TCPIP_STATE_SHUTDOWN),ETHSM_TCPIPMODEINDICATION_ID,ETHSM_E_INVALID_TCP_IP_MODE, E_NOT_OK);

    /* @req 4.2.2/SWS_EthSM_00119 */
    NetworkHandle =  EthSM_Internal_GetNetworkHandleEth(CtrlIdx);
    ETHSM_MODE_REQUEST_CLEAR(NetworkHandle,ETHSM_TCPIP_MODE_REQUEST);

    EthSM_Internal.Networks[NetworkHandle].TcpIpState = TcpIpState;

    return E_OK;
}

/**
 * This callback shall notify the EthSM module about a Ethernet controller mode change.
 * @param CtrlIdx
 * @param CtrlMode
 * @return none
 */
/* @req 4.2.2/SWS_EthSM_00190*/
void EthSM_CtrlModeIndication(uint8 CtrlIdx, Eth_ModeType CtrlMode){
    /* @req 4.2.2/SWS_EthSM_00192 */
    ETHSM_VALIDATE_NO_RV((EthSM_Internal.InitStatus == ETHSM_STATUS_INIT),ETHSM_CTRLMODEINDICATION_ID,ETHSM_E_UNINIT);
    /* @req 4.2.2/SWS_EthSM_00191 */
    ETHSM_VALIDATE_NO_RV((ETHSM_INVALID_CTRL != EthSM_Internal_CheckEthIfCtrl(CtrlIdx)),ETHSM_CTRLMODEINDICATION_ID,ETHSM_E_PARAM_CONTROLLER);

    NetworkHandleType NetworkHandle =  EthSM_Internal_GetNetworkHandleEth(CtrlIdx);

    ETHSM_MODE_REQUEST_CLEAR(NetworkHandle,ETHSM_ETHIF_ETH_MODE_REQUEST);

    EthSM_Internal.Networks[NetworkHandle].CtrlMode  = CtrlMode;
 }

/**
 * This callback shall notify the EthSM module about a Ethernet transceiver mode change.
 * @param TrcvIdx
 * @param TrcvMode
 * @return none
 */
/* @req 4.2.2/SWS_EthSM_00193*/
void EthSM_TrcvModeIndication(uint8 CtrlIdx, EthTrcv_ModeType TrcvMode){
    /* @req 4.2.2/SWS_EthSM_00195 */
    ETHSM_VALIDATE_NO_RV((EthSM_Internal.InitStatus == ETHSM_STATUS_INIT),ETHSM_TRCVMODEINDICATION_ID,ETHSM_E_UNINIT);
    /* @req 4.2.2/SWS_EthSM_00194 */
    ETHSM_VALIDATE_NO_RV((ETHSM_INVALID_CTRL != EthSM_Internal_CheckEthIfCtrl(CtrlIdx)),ETHSM_TRCVMODEINDICATION_ID,ETHSM_E_PARAM_CONTROLLER);

    NetworkHandleType NetworkHandle =  EthSM_Internal_GetNetworkHandleEth(CtrlIdx);

    ETHSM_MODE_REQUEST_CLEAR(NetworkHandle,ETHSM_ETHIF_ETHTRCV_MODE_REQUEST);

    EthSM_Internal.Networks[NetworkHandle].TrcvMode = TrcvMode;
}

/**
 * This service shall put out the current internal mode of a Ethernet network
 * @param NetworkHandle
 * @param EthSM_InternalMode
 * @return E_OK or E_NOT_OK
 */
/* @req 4.2.2/SWS_EthSM_00121*/
/* @req 4.2.2/SWS_EthSM_00014 */
Std_ReturnType EthSM_GetCurrentInternalMode( NetworkHandleType NetworkHandle, EthSM_NetworkModeStateType* EthSM_InternalMode ){
    NetworkHandleType EthSMNetworkHandle = EthSM_Internal_GetNetworkHandleComM(NetworkHandle);
    /* @req 4.2.2/SWS_EthSM_00125 */
    ETHSM_VALIDATE_RV((EthSM_Internal.InitStatus == ETHSM_STATUS_INIT),ETHSM_GETCURRENTINTERNALMODE_ID,ETHSM_E_UNINIT, E_NOT_OK);
    /* @req 4.2.2/SWS_EthSM_00122 */
    /* @req 4.2.2/SWS_EthSM_00123 */
    ETHSM_VALIDATE_RV((EthSMNetworkHandle < ETHSM_NETWORK_COUNT),ETHSM_GETCURRENTINTERNALMODE_ID,ETHSM_E_INVALID_NETWORK_HANDLE,E_NOT_OK);

    ETHSM_VALIDATE_RV((EthSM_InternalMode != NULL),ETHSM_GETCURRENTINTERNALMODE_ID,ETHSM_E_PARAM_POINTER,E_NOT_OK);

    /* @req 4.2.2/SWS_EthSM_00124 */
    *EthSM_InternalMode = EthSM_Internal.Networks[EthSMNetworkHandle].NetworkMode;
    return E_OK;
}

/**
 * Main function to be called with a periodic time frame
 * @param none
 * @return none
 */
/* @req 4.2.2/SWS_EthSM_00035*/
void EthSM_MainFunction(void) {
    ETHSM_VALIDATE_NO_RV((EthSM_Internal.InitStatus == ETHSM_STATUS_INIT),ETHSM_MAINFUNCTION_ID,ETHSM_E_UNINIT);
    /* @req 4.2.2/SWS_EthSM_00197 */
    /* @req 4.2.2/SWS_EthSM_00198 */
    /* @req 4.2.2/SWS_EthSM_00038 */
    /* @req 4.2.2/SWS_EthSM_00083 */
    /* @req 4.2.2/SWS_EthSM_00024 */
    /* @req 4.2.2/SWS_EthSM_00015 */
    for (NetworkHandleType NetworkHandle = 0; NetworkHandle < ETHSM_NETWORK_COUNT; NetworkHandle++) {
        if(EthSM_Internal.Networks[NetworkHandle].RequestedMode == COMM_FULL_COMMUNICATION){
            EthSM_Internal_FullComFSM(NetworkHandle);
        }else if(EthSM_Internal.Networks[NetworkHandle].RequestedMode == COMM_NO_COMMUNICATION){
            EthSM_Internal_NoComFSM(NetworkHandle);
        }else{
            /* Do nothing for COMM_SILENT_COMMUNICATION */
        }
    }
}

#ifdef HOST_TEST
EthSM_InternalType* readinternal_status(void );
EthSM_InternalType* readinternal_status(void)
{
    return &EthSM_Internal;
}
#endif
/*==================[END]===========================================*/
