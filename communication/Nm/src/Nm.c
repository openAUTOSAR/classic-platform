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


#if defined(USE_CANNM)
#include "CanNm.h"
#endif
#if defined(USE_FRNM)
#include "FrNm.h"
#endif
#if defined(USE_LINNM)
#include "LinNm.h"
#endif
#if defined(USE_UDPNM)
#include "UdpNm.h"
#endif
#if defined(USE_OSEKNM)
#include "OsekNm.h"
#endif


#include "Nm.h" /** @req NM123 */

#if defined(USE_NM_EXTENSION)
#include "Nm_Extension.h"
#endif

#if  ( NM_DEV_ERROR_DETECT == STD_ON )
#include "Det.h"
#endif


/* Globally fulfilled requirements */
/** @req NM126 */ /* Development errors are uint8 */
/** @req NM170 */ /* since mainfunctions does nothing this applies */
/** @req NM091 */ /* Some are implemented as functions and some as macros*/
/** @req NM095 */ /* do not provide more APIs than specified */
/** @req NM006 */ /* convert generic calls to bus specific function calls*/
/** @req NM243 */ /* det is included based on the generated macro */
/** @req Nm022 */ /* NM_DEV_ERROR_DETECT switch is generated in Nm_cfg.h */
/** @req Nm023 */ /* Det check is based on switch NM_DEV_ERROR_DETECT */
/** @req Nm025 */ /* Det service is use based on switch NM_DEV_ERROR_DETECT */
/** @req Nm028 */ /* Nm callbacks can run either in task or in interrupt context */

/*lint -emacro(904,VALIDATE,VALIDATE_NO_RV)*/ /*904 PC-Lint exception to MISRA 14.7 (validate DET macros)*/
#if  ( NM_DEV_ERROR_DETECT == STD_ON )

#define VALIDATE(_exp,_api,_err,_ret ) \
        if( !(_exp) ) { \
            (void)Det_ReportError(NM_MODULE_ID, 0, _api, _err); \
            return _ret; \
        }

#define VALIDATE_NO_RV(_exp,_api,_err ) \
        if( !(_exp) ) { \
            (void)Det_ReportError(NM_MODULE_ID, 0, _api, _err); \
            return; \
        }

#else
#define VALIDATE(_exp,_api,_err,_ret )
#define VALIDATE_NO_RV(_exp,_api,_err )
#endif

boolean Nm_InitStatus = FALSE;

/** Initializes the NM Interface. */
void Nm_Init( void )
{
#if defined(USE_NM_EXTENSION)
    Nm_Extension_Init();
#endif
    Nm_InitStatus = TRUE;
}

/** This function calls the <BusNm>_PassiveStartUp function
  * (e.g. CanNm_PassiveStartUp function is called if channel is configured as CAN). */
Std_ReturnType Nm_PassiveStartUp( const NetworkHandleType NetworkHandle ){

    Std_ReturnType status;
    const Nm_ChannelType* ChannelConf;
    /* @req  Nm128 */
    /* @req  Nm232 */
    VALIDATE((TRUE == Nm_InitStatus), NM_SERVICEID_PASSIVESTARTUP, NM_E_UNINIT, E_NOT_OK);
    /* @req  Nm233 */
    VALIDATE(( (NetworkHandle < Nm_Config.ChannelMapSize) && (NM_CHANNEL_COUNT > Nm_Config.ChannelMap[NetworkHandle]) ), NM_SERVICEID_PASSIVESTARTUP, NM_E_HANDLE_UNDEF, E_NOT_OK);

    ChannelConf = &Nm_Config.Channels[Nm_Config.ChannelMap[NetworkHandle]];

    switch(ChannelConf->BusType) {
#if defined(USE_CANNM)
        case NM_BUSNM_CANNM:
            status = CanNm_PassiveStartUp(NetworkHandle);
            break;
#endif
#if defined(USE_FRNM)
        case NM_BUSNM_FRNM:
            status = FrNm_PassiveStartUp(NetworkHandle);
            break;
#endif
#if defined(USE_LINNM)
        case NM_BUSNM_LINNM:
            status = LinNm_PassiveStartUp(NetworkHandle);
            break;
#endif
#if defined(USE_UDPNM)
        case NM_BUSNM_UDPNM:
            status = UdpNm_PassiveStartUp(NetworkHandle);
            break;
#endif
        default: status = E_NOT_OK;
    }
    return status;
}

/** This function calls the <BusNm>_NetworkRequest
  * (e.g. CanNm_NetworkRequest function is called if channel is configured as CAN). */
#if (NM_PASSIVE_MODE == STD_OFF)
Std_ReturnType Nm_NetworkRequest( const NetworkHandleType NetworkHandle ){

    Std_ReturnType status;
    const Nm_ChannelType* ChannelConf;
    /* @req  Nm129 */
    /* @req  Nm232 */
    VALIDATE((TRUE == Nm_InitStatus), NM_SERVICEID_NETWORKREQUEST, NM_E_UNINIT, E_NOT_OK);
    /* @req  Nm233 */
    VALIDATE(( (NetworkHandle < Nm_Config.ChannelMapSize) && (NM_CHANNEL_COUNT > Nm_Config.ChannelMap[NetworkHandle]) ), NM_SERVICEID_NETWORKREQUEST, NM_E_HANDLE_UNDEF, E_NOT_OK);

    ChannelConf = &Nm_Config.Channels[Nm_Config.ChannelMap[NetworkHandle]];

    switch(ChannelConf->BusType) {
#if defined(USE_CANNM) && defined(NM_BUSNM_CANNM_NOT_PASSIVE)
        case NM_BUSNM_CANNM:
            status = CanNm_NetworkRequest(NetworkHandle);
            break;
#endif
#if defined(USE_FRNM) && defined(NM_BUSNM_FRNM_NOT_PASSIVE)
        case NM_BUSNM_FRNM:
            status = FrNm_NetworkRequest(NetworkHandle);
            break;
#endif
#if defined(USE_LINNM) && defined(NM_BUSNM_LINNM_NOT_PASSIVE)
        case NM_BUSNM_LINNM:
            status = LinNm_NetworkRequest(NetworkHandle);
            break;
#endif
#if defined(USE_UDPNM) && defined(NM_BUSNM_UDPNM_NOT_PASSIVE)
        case NM_BUSNM_UDPNM:
            status = UdpNm_NetworkRequest(NetworkHandle);
            break;
#endif
#if defined(USE_OSEKNM) && defined(NM_BUSNM_OSEKNM_NOT_PASSIVE)
        case NM_BUSNM_OSEKNM:
            status = OsekNm_NetworkRequest(NetworkHandle);
            break;
#endif
        default: status = E_NOT_OK;
    }
    return status;
}
#endif

#if (NM_PASSIVE_MODE == STD_OFF)
/** This function calls the <BusNm>_NetworkRelease bus specific function
  * (e.g. CanNm_NetworkRelease function is called if channel is configured as CAN). */
Std_ReturnType Nm_NetworkRelease( const NetworkHandleType NetworkHandle ){

    Std_ReturnType status;
    status = E_NOT_OK;
    const Nm_ChannelType* ChannelConf;
    /* @req  Nm131 */
    /* @req  Nm232 */
    VALIDATE((TRUE == Nm_InitStatus), NM_SERVICEID_NETWORKRELEASE, NM_E_UNINIT, E_NOT_OK);
    /* @req  Nm233 */
    VALIDATE(( (NetworkHandle < Nm_Config.ChannelMapSize) && (NM_CHANNEL_COUNT > Nm_Config.ChannelMap[NetworkHandle]) ), NM_SERVICEID_NETWORKRELEASE, NM_E_HANDLE_UNDEF, E_NOT_OK);

    ChannelConf = &Nm_Config.Channels[Nm_Config.ChannelMap[NetworkHandle]];

    switch(ChannelConf->BusType) {
#if defined(USE_CANNM) && defined(NM_BUSNM_CANNM_NOT_PASSIVE)
        case NM_BUSNM_CANNM:
            status = CanNm_NetworkRelease(NetworkHandle);
            break;
#endif
#if defined(USE_FRNM) && defined(NM_BUSNM_FRNM_NOT_PASSIVE)
        case NM_BUSNM_FRNM:
            status = FrNm_NetworkRelease(NetworkHandle);
            break;
#endif
#if defined(USE_LINNM) && defined(NM_BUSNM_LINNM_NOT_PASSIVE)
        case NM_BUSNM_LINNM:
            status = LinNm_NetworkRelease(NetworkHandle);
            break;
#endif
#if defined(USE_UDPNM) && defined(NM_BUSNM_UDPNM_NOT_PASSIVE)
        case NM_BUSNM_UDPNM:
            status = UdpNm_NetworkRelease(NetworkHandle);
            break;
#endif
#if defined(USE_OSEKNM) && defined(NM_BUSNM_OSEKNM_NOT_PASSIVE)
        case NM_BUSNM_OSEKNM:
            status = OsekNm_NetworkRelease(NetworkHandle);
            break;
#endif
        default: status = E_NOT_OK;
    }
    return status;
}
#endif

#if (NM_COM_CONTROL_ENABLED == STD_ON) /* @req NM134 */
/** This function calls the CanNm_NetworkRelease bus specific function, to disable
  * the NM PDU transmission ability due to a ISO14229 Communication Control (28hex) service */
Std_ReturnType Nm_DisableCommunication( const NetworkHandleType NetworkHandle ){

    Std_ReturnType status;
    const Nm_ChannelType* ChannelConf;
    /* @req  Nm133 */
    /* @req  Nm232 */
    VALIDATE((TRUE == Nm_InitStatus), NM_SERVICEID_DISABLECOMMUNICATION, NM_E_UNINIT, E_NOT_OK);
    /* @req  Nm233 */
    VALIDATE(( (NetworkHandle < Nm_Config.ChannelMapSize) && (NM_CHANNEL_COUNT > Nm_Config.ChannelMap[NetworkHandle]) ), NM_SERVICEID_DISABLECOMMUNICATION, NM_E_HANDLE_UNDEF, E_NOT_OK);

    ChannelConf = &Nm_Config.Channels[Nm_Config.ChannelMap[NetworkHandle]];

    switch(ChannelConf->BusType) {
#if defined(USE_CANNM)
#if (CANNM_COM_CONTROL_ENABLED == STD_ON)
        case NM_BUSNM_CANNM:
            status = CanNm_DisableCommunication(NetworkHandle);
            break;
#endif
#endif
#if defined(USE_FRNM)
        case NM_BUSNM_FRNM:
            status = FrNm_DisableCommunication(NetworkHandle);
            break;
#endif
#if defined(USE_LINNM)
        case NM_BUSNM_LINNM:
            status = LinNm_DisableCommunication(NetworkHandle);
            break;
#endif
#if defined(USE_UDPNM)
        case NM_BUSNM_UDPNM:
            status = UdpNm_DisableCommunication(NetworkHandle);
            break;
#endif
        default: status = E_NOT_OK;
    }
    return status;
}
#endif

#if (NM_COM_CONTROL_ENABLED == STD_ON) /* @req NM136 */
/** This function calls the CanNm_NetworkRequest bus specific function, to enable
  * the NM PDU transmission ability due to a ISO14229 Communication Control (28hex) service */
Std_ReturnType Nm_EnableCommunication( const NetworkHandleType NetworkHandle ){

    Std_ReturnType status;
    const Nm_ChannelType* ChannelConf;
    /* @req  Nm135 */
    /* @req  Nm232 */
    VALIDATE((TRUE == Nm_InitStatus), NM_SERVICEID_ENABLECOMMUNICATION, NM_E_UNINIT, E_NOT_OK);
    /* @req  Nm233 */
    VALIDATE(( (NetworkHandle < Nm_Config.ChannelMapSize) && (NM_CHANNEL_COUNT > Nm_Config.ChannelMap[NetworkHandle]) ), NM_SERVICEID_ENABLECOMMUNICATION, NM_E_HANDLE_UNDEF, E_NOT_OK);

    ChannelConf = &Nm_Config.Channels[Nm_Config.ChannelMap[NetworkHandle]];

    switch(ChannelConf->BusType) {
#if defined(USE_CANNM)
#if (CANNM_COM_CONTROL_ENABLED == STD_ON)
        case NM_BUSNM_CANNM:
            status = CanNm_EnableCommunication(NetworkHandle);
            break;
#endif
#endif
#if defined(USE_FRNM)
        case NM_BUSNM_FRNM:
            status = FrNm_EnableCommunication(NetworkHandle);
            break;
#endif
#if defined(USE_LINNM)
        case NM_BUSNM_LINNM:
            status = LinNm_EnableCommunication(NetworkHandle);
            break;
#endif
#if defined(USE_UDPNM)
        case NM_BUSNM_UDPNM:
            status = UdpNm_EnableCommunication(NetworkHandle);
            break;
#endif
        default: status = E_NOT_OK;
    }
    return status;
}
#endif

/** Set user data for NM messages transmitted next on the bus.
  * For that purpose <BusNm>_SetUserData shall be called
  * (e.g. CanNm_SetUserData function is called if channel is configured as CAN). */
/** @req NM241 */ /** @req NM138 */
#if (NM_COM_USER_DATA_ENABLED == STD_OFF) && (NM_USER_DATA_ENABLED == STD_ON) && (NM_PASSIVE_MODE == STD_OFF)
Std_ReturnType Nm_SetUserData( const NetworkHandleType NetworkHandle, const uint8 * const nmUserDataPtr ){

    Std_ReturnType status;
    const Nm_ChannelType* ChannelConf;
    /* @req  Nm137 */
    /* @req  Nm232 */
    VALIDATE((TRUE == Nm_InitStatus), NM_SERVICEID_SETUSERDATA, NM_E_UNINIT, E_NOT_OK);
    /* @req  Nm233 */
    VALIDATE(( (NetworkHandle < Nm_Config.ChannelMapSize) && (NM_CHANNEL_COUNT > Nm_Config.ChannelMap[NetworkHandle]) ), NM_SERVICEID_SETUSERDATA, NM_E_HANDLE_UNDEF, E_NOT_OK);
    /* @req  Nm248 */
    VALIDATE((nmUserDataPtr != NULL), NM_SERVICEID_SETUSERDATA, NM_E_PARAM_POINTER, E_NOT_OK);

    ChannelConf = &Nm_Config.Channels[Nm_Config.ChannelMap[NetworkHandle]];

    switch(ChannelConf->BusType) {
#if defined(USE_CANNM) && defined(NM_BUSNM_CANNM_NOT_PASSIVE)
        case NM_BUSNM_CANNM:
            status = CanNm_SetUserData(NetworkHandle, nmUserDataPtr);
            break;
#endif
#if defined(USE_FRNM) && defined(NM_BUSNM_FRNM_NOT_PASSIVE)
        case NM_BUSNM_FRNM:
            status = FrNm_SetUserData(NetworkHandle, nmUserDataPtr);
            break;
#endif
#if defined(USE_LINNM) && defined(NM_BUSNM_LINNM_NOT_PASSIVE)
        case NM_BUSNM_LINNM:
            status = LinNm_SetUserData(NetworkHandle, nmUserDataPtr);
            break;
#endif
#if defined(USE_UDPNM) && defined(NM_BUSNM_UDPNM_NOT_PASSIVE)
        case NM_BUSNM_UDPNM:
            status = UdpNm_SetUserData(NetworkHandle, nmUserDataPtr);
            break;
#endif
        default: status = E_NOT_OK;
    }
    return status;
}
#endif

/** Get user data out of the last successfully received NM message.
  * For that purpose <BusNm>_GetUserData shall be called
  * (e.g. CanNm_GetUserData function is called if channel is configured as CAN). */
#if (NM_USER_DATA_ENABLED == STD_ON)/** @req NM140 */
Std_ReturnType Nm_GetUserData( const NetworkHandleType NetworkHandle, uint8 * const nmUserDataPtr ){

    Std_ReturnType status;
    /* For some reason the signature of this service differs from its busNm equivalents... */
    Std_ReturnType userDataRet = E_NOT_OK;
    Std_ReturnType nodeIdRet = E_NOT_OK;
    uint8 nmNodeIdPtr;
    const Nm_ChannelType* ChannelConf;
    /* @req  Nm139 */
    /* @req  Nm232 */
    VALIDATE((TRUE == Nm_InitStatus), NM_SERVICEID_GETUSERDATA, NM_E_UNINIT, E_NOT_OK);
    /* @req  Nm233 */
    VALIDATE(( (NetworkHandle < Nm_Config.ChannelMapSize) && (NM_CHANNEL_COUNT > Nm_Config.ChannelMap[NetworkHandle]) ), NM_SERVICEID_GETUSERDATA, NM_E_HANDLE_UNDEF, E_NOT_OK);
    /* @req  Nm248 */
    VALIDATE((nmUserDataPtr != NULL), NM_SERVICEID_GETUSERDATA, NM_E_PARAM_POINTER, E_NOT_OK);

    ChannelConf = &Nm_Config.Channels[Nm_Config.ChannelMap[NetworkHandle]];

    switch(ChannelConf->BusType) {
#if defined(USE_CANNM)
    case NM_BUSNM_CANNM:
        userDataRet = CanNm_GetUserData(NetworkHandle, nmUserDataPtr);
        nodeIdRet = CanNm_GetNodeIdentifier(NetworkHandle, &nmNodeIdPtr);
        break;
#endif
#if defined(USE_FRNM)
    case NM_BUSNM_FRNM:
        userDataRet = FrNm_GetUserData(NetworkHandle, nmUserDataPtr);
        nodeIdRet = FrNm_GetNodeIdentifier(NetworkHandle, &nmNodeIdPtr);
        break;
#endif
#if defined(USE_LINNM)
    case NM_BUSNM_LINNM:
        userDataRet = LinNm_GetUserData(NetworkHandle, nmUserDataPtr);
        nodeIdRet = LinNm_GetNodeIdentifier(NetworkHandle, &nmNodeIdPtr);
        break;
#endif
#if defined(USE_UDPNM)
    case NM_BUSNM_UDPNM:
        userDataRet = UdpNm_GetUserData(NetworkHandle, nmUserDataPtr);
        nodeIdRet = UdpNm_GetNodeIdentifier(NetworkHandle, &nmNodeIdPtr);
        break;
#endif
    default:
        break;
    }

    if( (E_OK != userDataRet) || ( E_OK != nodeIdRet )) {
        status = E_NOT_OK;
    } else {
        status = E_OK;
    }
    return status;
}
#endif

/** Get the whole PDU data out of the most recently received NM message.
  * For that purpose CanNm_GetPduData shall be called. */
#if ((NM_NODE_ID_ENABLED == STD_ON) || (NM_NODE_DETECTION_ENABLED == STD_ON) || (NM_USER_DATA_ENABLED == STD_ON)) /** @req NM142 */
Std_ReturnType Nm_GetPduData( const NetworkHandleType NetworkHandle, uint8 * const nmPduData ){

    Std_ReturnType status;
    const Nm_ChannelType* ChannelConf;
    /* @req  Nm141 */
    /* @req  Nm232 */
    VALIDATE((TRUE == Nm_InitStatus), NM_SERVICEID_GETPDUDATA, NM_E_UNINIT, E_NOT_OK);
    /* @req  Nm233 */
    VALIDATE(( (NetworkHandle < Nm_Config.ChannelMapSize) && (NM_CHANNEL_COUNT > Nm_Config.ChannelMap[NetworkHandle]) ), NM_SERVICEID_GETPDUDATA, NM_E_HANDLE_UNDEF, E_NOT_OK);
    /* @req  Nm248 */
    VALIDATE((nmPduData != NULL), NM_SERVICEID_GETPDUDATA, NM_E_PARAM_POINTER, E_NOT_OK);

    ChannelConf = &Nm_Config.Channels[Nm_Config.ChannelMap[NetworkHandle]];

    switch(ChannelConf->BusType) {
#if defined(USE_CANNM)
        case NM_BUSNM_CANNM:
            status = CanNm_GetPduData(NetworkHandle, nmPduData);
            break;
#endif
#if defined(USE_FRNM)
        case NM_BUSNM_FRNM:
            status = FrNm_GetPduData(NetworkHandle, nmPduData);
            break;
#endif
#if defined(USE_LINNM)
        case NM_BUSNM_LINNM:
            status = LinNm_GetPduData(NetworkHandle, nmPduData);
            break;
#endif
#if defined(USE_UDPNM)
        case NM_BUSNM_UDPNM:
            status = UdpNm_GetPduData(NetworkHandle, nmPduData);
            break;
#endif
        default: status = E_NOT_OK;
    }
    return status;
}
#endif

/** Set Repeat Message Request Bit for NM messages transmitted next on the bus.
  * For that purpose <BusNm>_RepeatMessageRequest shall be called
  * (e.g. CanNm_RepeatMessageRequest function is called if channel is configured as CAN) */
#if (NM_NODE_DETECTION_ENABLED == STD_ON) /** @req NM144 */
Std_ReturnType Nm_RepeatMessageRequest( const NetworkHandleType NetworkHandle ){

    Std_ReturnType status;
    const Nm_ChannelType* ChannelConf;
    /* @req  Nm143 */
    /* @req  Nm232 */
    VALIDATE((TRUE == Nm_InitStatus), NM_SERVICEID_REPEATMESSAGEREQUEST, NM_E_UNINIT, E_NOT_OK);
    /* @req  Nm233 */
    VALIDATE(( (NetworkHandle < Nm_Config.ChannelMapSize) && (NM_CHANNEL_COUNT > Nm_Config.ChannelMap[NetworkHandle]) ), NM_SERVICEID_REPEATMESSAGEREQUEST, NM_E_HANDLE_UNDEF, E_NOT_OK);

    ChannelConf = &Nm_Config.Channels[Nm_Config.ChannelMap[NetworkHandle]];

    switch(ChannelConf->BusType) {
#if defined(USE_CANNM)
        case NM_BUSNM_CANNM:
            status = CanNm_RepeatMessageRequest(NetworkHandle);
            break;
#endif
#if defined(USE_FRNM)
        case NM_BUSNM_FRNM:
            status = FrNm_RepeatMessageRequest(NetworkHandle);
            break;
#endif
#if defined(USE_LINNM)
        case NM_BUSNM_LINNM:
            status = LinNm_RepeatMessageRequest(NetworkHandle);
            break;
#endif
#if defined(USE_UDPNM)
        case NM_BUSNM_UDPNM:
            status = UdpNm_RepeatMessageRequest(NetworkHandle);
            break;
#endif
        default: status = E_NOT_OK;
    }
    return status;
}
#endif

/** Get node identifier out of the last successfully received NM-message.
  * The function <BusNm>_GetNodeIdentifier shall be called. */
#if (NM_NODE_ID_ENABLED == STD_ON) /** @req NM146 */ /** @req NM148 */
Std_ReturnType Nm_GetNodeIdentifier( const NetworkHandleType NetworkHandle, uint8 * const nmNodeIdPtr ){

    Std_ReturnType status;
    const Nm_ChannelType* ChannelConf;
    /* @req  Nm145 */
    /* @req  Nm232 */
    VALIDATE((TRUE == Nm_InitStatus), NM_SERVICEID_GETNODEIDENTIFIER, NM_E_UNINIT, E_NOT_OK);
    /* @req  Nm233 */
    VALIDATE(( (NetworkHandle < Nm_Config.ChannelMapSize) && (NM_CHANNEL_COUNT > Nm_Config.ChannelMap[NetworkHandle]) ), NM_SERVICEID_GETNODEIDENTIFIER, NM_E_HANDLE_UNDEF, E_NOT_OK);
    /* @req  Nm248 */
    VALIDATE((nmNodeIdPtr != NULL), NM_SERVICEID_GETNODEIDENTIFIER, NM_E_PARAM_POINTER, E_NOT_OK);

    ChannelConf = &Nm_Config.Channels[Nm_Config.ChannelMap[NetworkHandle]];

    switch(ChannelConf->BusType) {
#if defined(USE_CANNM)
        case NM_BUSNM_CANNM:
            status = CanNm_GetNodeIdentifier(NetworkHandle, nmNodeIdPtr);
            break;
#endif
#if defined(USE_FRNM)
        case NM_BUSNM_FRNM:
            status = FrNm_GetNodeIdentifier(NetworkHandle, nmNodeIdPtr);
            break;
#endif
#if defined(USE_LINNM)
        case NM_BUSNM_LINNM:
            status = LinNm_GetNodeIdentifier(NetworkHandle, nmNodeIdPtr);
            break;
#endif
#if defined(USE_UDPNM)
        case NM_BUSNM_UDPNM:
            status = UdpNm_GetNodeIdentifier(NetworkHandle, nmNodeIdPtr);
            break;
#endif
        default: status = E_NOT_OK;
    }
    return status;
}

/** Get node identifier configured for the local node.
  * For that purpose <BusNm>_GetLocalNodeIdentifier shall be called
  * (e.g. CanNm_GetLocalNodeIdentifier function is called if channel is configured as CAN). */
Std_ReturnType Nm_GetLocalNodeIdentifier( const NetworkHandleType NetworkHandle, uint8 * const nmNodeIdPtr ){

    Std_ReturnType status;
    const Nm_ChannelType* ChannelConf;
    /* @req  Nm147 */
    /* @req  Nm232 */
    VALIDATE((TRUE == Nm_InitStatus), NM_SERVICEID_GETLOCALNODEIDENTIFIER, NM_E_UNINIT, E_NOT_OK);
    /* @req  Nm233 */
    VALIDATE(( (NetworkHandle < Nm_Config.ChannelMapSize) && (NM_CHANNEL_COUNT > Nm_Config.ChannelMap[NetworkHandle]) ), NM_SERVICEID_GETLOCALNODEIDENTIFIER, NM_E_HANDLE_UNDEF, E_NOT_OK);
    /* @req  Nm248 */
    VALIDATE((nmNodeIdPtr != NULL), NM_SERVICEID_GETLOCALNODEIDENTIFIER, NM_E_PARAM_POINTER, E_NOT_OK);

    ChannelConf = &Nm_Config.Channels[Nm_Config.ChannelMap[NetworkHandle]];
 
    switch(ChannelConf->BusType) {
#if defined(USE_CANNM)
        case NM_BUSNM_CANNM:
            status = CanNm_GetLocalNodeIdentifier(NetworkHandle, nmNodeIdPtr);
            break;
#endif
#if defined(USE_FRNM)
        case NM_BUSNM_FRNM:
            status = FrNm_GetLocalNodeIdentifier(NetworkHandle, nmNodeIdPtr);
            break;
#endif
#if defined(USE_LINNM)
        case NM_BUSNM_LINNM:
            status = LinNm_GetLocalNodeIdentifier(NetworkHandle, nmNodeIdPtr);
            break;
#endif
#if defined(USE_UDPNM)
        case NM_BUSNM_UDPNM:
            status = UdpNm_GetLocalNodeIdentifier(NetworkHandle, nmNodeIdPtr);
            break;
#endif
        default: status = E_NOT_OK;
    }
    return status;
}
#endif

/** Check if remote sleep indication takes place or not. This in turn calls the
  * <BusNm>_CheckRemoteSleepIndication for the bus specific NM layer
  * (e.g. CanNm_CheckRemoteSleepIndication function is called if channel is configured as CAN). */
Std_ReturnType Nm_CheckRemoteSleepIndication( const NetworkHandleType NetworkHandle, boolean * const nmRemoteSleepIndPtr ){

    const Nm_ChannelType* ChannelConf;
    /* @req  Nm149 */
    /* @req  Nm232 */
    VALIDATE((TRUE == Nm_InitStatus), NM_SERVICEID_CHECKREMOTESLEEPINDICATION, NM_E_UNINIT, E_NOT_OK);
    /* @req  Nm233 */
    VALIDATE(( (NetworkHandle < Nm_Config.ChannelMapSize) && (NM_CHANNEL_COUNT > Nm_Config.ChannelMap[NetworkHandle]) ), NM_SERVICEID_CHECKREMOTESLEEPINDICATION, NM_E_HANDLE_UNDEF, E_NOT_OK);
    /* @req  Nm248 */
    VALIDATE((nmRemoteSleepIndPtr != NULL), NM_SERVICEID_CHECKREMOTESLEEPINDICATION, NM_E_PARAM_POINTER, E_NOT_OK);

    ChannelConf = &Nm_Config.Channels[Nm_Config.ChannelMap[NetworkHandle]];

    (void)NetworkHandle;
    (void)nmRemoteSleepIndPtr;/*lint !e920 */
    (void)ChannelConf;/*lint !e920 */

    return E_NOT_OK;
}

/** Returns the state of the network management. This function in turn calls the
  * <BusNm>_GetState function (e.g. CanNm_GetState function is called if channel is configured as CAN). */
Std_ReturnType Nm_GetState( const NetworkHandleType NetworkHandle, Nm_StateType* const nmStatePtr, Nm_ModeType* const nmModePtr ){

    Std_ReturnType status;
    const Nm_ChannelType* ChannelConf;
    /* @req  Nm151 */
    /* @req  Nm232 */
    VALIDATE((TRUE == Nm_InitStatus), NM_SERVICEID_GETSTATE, NM_E_UNINIT, E_NOT_OK);
    /* @req  Nm233 */
    VALIDATE(( (NetworkHandle < Nm_Config.ChannelMapSize) && (NM_CHANNEL_COUNT > Nm_Config.ChannelMap[NetworkHandle]) ), NM_SERVICEID_GETSTATE, NM_E_HANDLE_UNDEF, E_NOT_OK);
    /* @req  Nm248 */
    VALIDATE((nmStatePtr != NULL), NM_SERVICEID_GETSTATE, NM_E_PARAM_POINTER, E_NOT_OK);
    VALIDATE((nmModePtr != NULL), NM_SERVICEID_GETSTATE, NM_E_PARAM_POINTER, E_NOT_OK);

    ChannelConf = &Nm_Config.Channels[Nm_Config.ChannelMap[NetworkHandle]];

    switch(ChannelConf->BusType) {
#if defined(USE_CANNM)
        case NM_BUSNM_CANNM:
            status = CanNm_GetState(NetworkHandle, nmStatePtr, nmModePtr);
            break;
#endif
#if defined(USE_FRNM)
        case NM_BUSNM_FRNM:
            status = FrNm_GetState(NetworkHandle, nmStatePtr, nmModePtr);
            break;
#endif
#if defined(USE_LINNM)
        case NM_BUSNM_LINNM:
            status = LinNm_GetState(NetworkHandle, nmStatePtr, nmModePtr);
            break;
#endif
#if defined(USE_UDPNM)
        case NM_BUSNM_UDPNM:
            status = UdpNm_GetState(NetworkHandle, nmStatePtr, nmModePtr);
            break;
#endif
        default: status = E_NOT_OK;
    }
    return status;
}

/** Notification that a NM-message has been received in the Bus-Sleep Mode, what
  * indicates that some nodes in the network have already entered the Network Mode.
  * The callback function shall start the network management state machine. */
/** @req NM012.partially.1 */
void Nm_NetworkStartIndication( const NetworkHandleType NetworkHandle ){

    /* @req  Nm232 */
    VALIDATE_NO_RV((TRUE == Nm_InitStatus), NM_SERVICEID_NETWORKSTARTINDICATION, NM_E_UNINIT);
    /* @req  Nm233 */
    VALIDATE_NO_RV(( (NetworkHandle < Nm_Config.ChannelMapSize) && (NM_CHANNEL_COUNT > Nm_Config.ChannelMap[NetworkHandle]) ), NM_SERVICEID_NETWORKSTARTINDICATION, NM_E_HANDLE_UNDEF);

#if defined(USE_NM_EXTENSION)
    Nm_Extension_NetworkStartIndication(NetworkHandle);
#else
    /** @req NM155 */
    ComM_Nm_NetworkStartIndication(NetworkHandle);
#endif
}

/** Notification that the network management has entered Network Mode. The
  * callback function shall enable transmission of application messages. */
/** @req NM012.partially.2 */
void Nm_NetworkMode( const NetworkHandleType NetworkHandle ){

    /* @req  Nm232 */
    VALIDATE_NO_RV((TRUE == Nm_InitStatus), NM_SERVICEID_NETWORKMODE, NM_E_UNINIT);
    /* @req  Nm233 */
    VALIDATE_NO_RV(( (NetworkHandle < Nm_Config.ChannelMapSize) && (NM_CHANNEL_COUNT > Nm_Config.ChannelMap[NetworkHandle]) ), NM_SERVICEID_NETWORKMODE, NM_E_HANDLE_UNDEF);

#if defined(USE_NM_EXTENSION)
    Nm_Extension_NetworkMode(NetworkHandle);
#else
    /** @req NM158 */
    ComM_Nm_NetworkMode(NetworkHandle);
#endif
}

/** Notification that the network management has entered Prepare Bus-Sleep Mode.
  * The callback function shall disable transmission of application messages. */
/** @req NM012.partially.3 */
void Nm_PrepareBusSleepMode( const NetworkHandleType NetworkHandle ){

    /* @req  Nm232 */
    VALIDATE_NO_RV((TRUE == Nm_InitStatus), NM_SERVICEID_PREPAREBUSSLEEPMODE, NM_E_UNINIT);
    /* @req  Nm233 */
    VALIDATE_NO_RV(( (NetworkHandle < Nm_Config.ChannelMapSize) && (NM_CHANNEL_COUNT > Nm_Config.ChannelMap[NetworkHandle]) ), NM_SERVICEID_PREPAREBUSSLEEPMODE, NM_E_HANDLE_UNDEF);

#if defined(USE_NM_EXTENSION)
    Nm_Extension_PrepareBusSleepMode(NetworkHandle);
#else
    /** @req NM161 */
    ComM_Nm_PrepareBusSleepMode(NetworkHandle);
#endif
}

/** Notification that the network management has entered Bus-Sleep Mode. */
/** @req NM012.partially.4 */
void Nm_BusSleepMode( const NetworkHandleType NetworkHandle ){

    /* @req  Nm232 */
    VALIDATE_NO_RV((TRUE == Nm_InitStatus), NM_SERVICEID_BUSSLEEPMODE, NM_E_UNINIT);
    /* @req  Nm233 */
    VALIDATE_NO_RV(( (NetworkHandle < Nm_Config.ChannelMapSize) && (NM_CHANNEL_COUNT > Nm_Config.ChannelMap[NetworkHandle]) ), NM_SERVICEID_BUSSLEEPMODE, NM_E_HANDLE_UNDEF);

#if defined(USE_NM_EXTENSION)
    Nm_Extension_BusSleepMode(NetworkHandle);
#else
    /** @req NM163 */
    ComM_Nm_BusSleepMode(NetworkHandle);
#endif
}

/* NEW */
/** Notification that the network management has detected that all other nodes on the network are ready to enter Bus-Sleep Mode */
/** @req NM192 */
void Nm_RemoteSleepIndication( const NetworkHandleType nmNetworkHandle )
{
    /* @req  Nm232 */
    VALIDATE_NO_RV((TRUE == Nm_InitStatus), NM_SERVICEID_REMOTESLEEPINDICATION, NM_E_UNINIT);
    /* @req  Nm233 */
    VALIDATE_NO_RV(( (nmNetworkHandle < Nm_Config.ChannelMapSize) && (NM_CHANNEL_COUNT > Nm_Config.ChannelMap[nmNetworkHandle]) ), NM_SERVICEID_REMOTESLEEPINDICATION, NM_E_HANDLE_UNDEF);

    (void)nmNetworkHandle;
}

/* NEW */
/**Notification that the network management has detected that not all other nodes on the network are longer ready to enter Bus-Sleep Mode */
/** @req NM193 */
void Nm_RemoteSleepCancellation( const NetworkHandleType nmNetworkHandle )
{
    /* @req  Nm232 */
    VALIDATE_NO_RV((TRUE == Nm_InitStatus), NM_SERVICEID_REMOTESLEEPCANCELLATION, NM_E_UNINIT);
    /* @req  Nm233 */
    VALIDATE_NO_RV(( (nmNetworkHandle < Nm_Config.ChannelMapSize) && (NM_CHANNEL_COUNT > Nm_Config.ChannelMap[nmNetworkHandle]) ), NM_SERVICEID_REMOTESLEEPCANCELLATION, NM_E_HANDLE_UNDEF);

    (void)nmNetworkHandle;
}

/* NEW */
/** Notification to the NM Coordinator functionality that this is a suitable point in time to initiate the coordination algorithm on */
/** @req NM194 */
void Nm_SynchronizationPoint( const NetworkHandleType nmNetworkHandle )
{
    /* @req  Nm232 */
    VALIDATE_NO_RV((TRUE == Nm_InitStatus), NM_SERVICEID_REMOTESYNCHRONISATIONPOINT, NM_E_UNINIT);
    /* @req  Nm233 */
    VALIDATE_NO_RV(( (nmNetworkHandle < Nm_Config.ChannelMapSize) && (NM_CHANNEL_COUNT > Nm_Config.ChannelMap[nmNetworkHandle]) ), NM_SERVICEID_REMOTESYNCHRONISATIONPOINT, NM_E_HANDLE_UNDEF);

    (void)nmNetworkHandle;
}

/** Notification that a NM message has been received. */
void Nm_PduRxIndication( const NetworkHandleType NetworkHandle )
{
    /* @req  Nm232 */
    VALIDATE_NO_RV((TRUE == Nm_InitStatus), NM_SERVICEID_PDURXINDICATION, NM_E_UNINIT);
    /* @req  Nm233 */
    VALIDATE_NO_RV(( (NetworkHandle < Nm_Config.ChannelMapSize) && (NM_CHANNEL_COUNT > Nm_Config.ChannelMap[NetworkHandle]) ), NM_SERVICEID_PDURXINDICATION, NM_E_HANDLE_UNDEF);

    (void)NetworkHandle;
}

#if (NM_STATE_CHANGE_IND_ENABLED == STD_ON)
/** Notification that the CAN or FR Generic NM state has changed. */
void Nm_StateChangeNotification(const NetworkHandleType nmNetworkHandle, const Nm_StateType nmPreviousState, const Nm_StateType nmCurrentState )
{
    const Nm_ChannelType* ChannelConf;

    /* @req  Nm232 */
    VALIDATE_NO_RV((TRUE == Nm_InitStatus), NM_SERVICEID_STATECHANGENOTIFICATION, NM_E_UNINIT);
    /* @req  Nm233 */
    VALIDATE_NO_RV(( (nmNetworkHandle < Nm_Config.ChannelMapSize) && (NM_CHANNEL_COUNT > Nm_Config.ChannelMap[nmNetworkHandle]) ), NM_SERVICEID_STATECHANGENOTIFICATION, NM_E_HANDLE_UNDEF);

    ChannelConf = &Nm_Config.Channels[Nm_Config.ChannelMap[nmNetworkHandle]];
    (void)(nmPreviousState);

    switch(ChannelConf->BusType) {
#if defined(USE_CANNM)
        case NM_BUSNM_CANNM:
#if defined(USE_NM_EXTENSION)
                Nm_Extension_WriteState(nmNetworkHandle, nmCurrentState);
#else
                (void)nmCurrentState;
#endif
        break;
#endif
#if defined(USE_FRNM)
        case NM_BUSNM_FRNM:
#if defined(USE_NM_EXTENSION)
                Nm_Extension_WriteState(nmNetworkHandle, nmCurrentState);
#else
                (void)nmCurrentState;
#endif
        break;
#endif
        default:
            break;
    }
}
#endif

/* NEW */
/** Service to indicate that an NM message with set Repeat Message Request Bit has been received */
/** @req NM230 */
void Nm_RepeatMessageIndication( const NetworkHandleType nmNetworkHandle )
{
    /* @req  Nm232 */
    VALIDATE_NO_RV((TRUE == Nm_InitStatus), NM_SERVICEID_REPEATMESSAGEINDICATION, NM_E_UNINIT);
    /* @req  Nm233 */
    VALIDATE_NO_RV(( (nmNetworkHandle < Nm_Config.ChannelMapSize) && (NM_CHANNEL_COUNT > Nm_Config.ChannelMap[nmNetworkHandle]) ), NM_SERVICEID_REPEATMESSAGEINDICATION, NM_E_HANDLE_UNDEF);


    (void)nmNetworkHandle;
}

/* NEW */
/** Service to indicate that an attempt to send an NM message failed */
/** @req NM234 */
void Nm_TxTimeoutException( const NetworkHandleType nmNetworkHandle )
{
    /* @req  Nm232 */
    VALIDATE_NO_RV((TRUE == Nm_InitStatus), NM_SERVICEID_TXTIMEOUTEXCEPTION, NM_E_UNINIT);
    /* @req  Nm233 */
    VALIDATE_NO_RV(( (nmNetworkHandle < Nm_Config.ChannelMapSize) && (NM_CHANNEL_COUNT > Nm_Config.ChannelMap[nmNetworkHandle]) ), NM_SERVICEID_TXTIMEOUTEXCEPTION, NM_E_HANDLE_UNDEF);


    (void)nmNetworkHandle;
}

/* NEW */
/** This function is called by a <Bus>Nm to indicate reception of a CWU request */
/** @req NM250 */
void Nm_CarWakeUpIndication( const NetworkHandleType nmNetworkHandle )
{
    /* @req  Nm232 */
    VALIDATE_NO_RV((TRUE == Nm_InitStatus), NM_SERVICEID_CARWAKEUPINDICATION, NM_E_UNINIT);
    /* @req  Nm233 */
    VALIDATE_NO_RV(( (nmNetworkHandle < Nm_Config.ChannelMapSize) && (NM_CHANNEL_COUNT > Nm_Config.ChannelMap[nmNetworkHandle]) ), NM_SERVICEID_CARWAKEUPINDICATION, NM_E_HANDLE_UNDEF);


    (void)nmNetworkHandle;
}

/* NEW */
/** Seta an indication, when the NM coordinator Sleep Ready bit in the Control Bit Vector is set */
/** @req NM254 */
void Nm_CoordReadyToSleepIndication( const NetworkHandleType nmNetworkHandle )
{
    /* @req  Nm232 */
    VALIDATE_NO_RV((TRUE == Nm_InitStatus), NM_SERVICEID_COORDREADYTOSLEEPINDICATION, NM_E_UNINIT);
    /* @req  Nm233 */
    VALIDATE_NO_RV(( (nmNetworkHandle < Nm_Config.ChannelMapSize) && (NM_CHANNEL_COUNT > Nm_Config.ChannelMap[nmNetworkHandle]) ), NM_SERVICEID_COORDREADYTOSLEEPINDICATION, NM_E_HANDLE_UNDEF);


    (void)nmNetworkHandle;
}

/** This function implements the processes of the NM Interface, which need a fix
  * cyclic scheduling. This function is supplied for the NM coordinator functionality
  * (Nm020). However, specific implementation may not need it (Nm093) */
/** @req 121 */
void Nm_MainFunction()
{
    /* @req  Nm232 */
    VALIDATE_NO_RV((TRUE == Nm_InitStatus), NM_SERVICEID_MAINFUNCTION, NM_E_UNINIT);
}

