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

#ifndef OSEKNM_INTERNAL_H_
#define OSEKNM_INTERNAL_H_

#include "OsekNm_ConfigTypes.h"

#define OSEKNM_VENDOR_ID                     60u
#define OSEKNM_MODULE_ID                     30u

#define OSEKNM_AR_RELEASE_MAJOR_VERSION      4u
#define OSEKNM_AR_RELEASE_MINOR_VERSION      2u
#define OSEKNM_AR_RELEASE_REVISION_VERSION   2u

#define OSEKNM_AR_MAJOR_VERSION              OSEKNM_AR_RELEASE_MAJOR_VERSION
#define OSEKNM_AR_MINOR_VERSION              OSEKNM_AR_RELEASE_MINOR_VERSION
#define OSEKNM_AR_PATCH_VERSION              OSEKNM_AR_RELEASE_REVISION_VERSION

#define OSEKNM_SW_MAJOR_VERSION              1u
#define OSEKNM_SW_MINOR_VERSION              0u
#define OSEKNM_SW_PATCH_VERSION              0u

#include <string.h>
#include "OsekNm_Cfg.h"

#if defined(USE_NM)
#include "Nm.h"
#endif



typedef enum {
    OSEKNM_STATUS_UNINIT,
    OSEKNM_STATUS_INIT
}OsekNm_Internal_InitStatusType;


/*
 * Errors codes described for OSEKNM.
 *****************************************************/
#define OSEKNM_E_NOT_INITIALIZED     0x00u
#define OSEKNM_E_INVALID_POINTER     0x01u
#define OSEKNM_E_INVALID_NETID       0x02u
#define OSEKNM_E_INVALID_NMCHANNEL   0x03u
#define OSEKNM_E_INVALID_CONFIGKIND  0x04u


/* Api Service codes */
#define OSEKNM_SERVICE_ID_INIT                         0x00u
#define OSEKNM_SERVICE_ID_NETWORK_REQUEST              0x01u
#define OSEKNM_SERVICE_ID_NETWORK_RELEASE              0x02u
#define OSEKNM_SERVICE_ID_GET_VERSION_INFO             0x03u
#define OSEKNM_SERVICE_ID_DEINIT                       0x04u
#define OSEKNM_SERVICE_ID_TX_CONFIRMATION              0x05u
#define OSEKNM_SERVICE_ID_RX_INDICATION                0x06u
#define OSEKNM_SERVICE_ID_CONTROLLER_BUSOFF            0x07u
#define OSEKNM_SERVICE_ID_TIMEOUT_NOTIFICATION         0x08u
#define OSEKNM_SERVICE_ID_INIT_CMASK_TABLE             0x09u
#define OSEKNM_SERVICE_ID_INIT_TARGETCONFIG_TABLE      0x0Au
#define OSEKNM_SERVICE_ID_CMP_CONFIG                   0x0Bu
#define OSEKNM_SERVICE_ID_GET_CONFIG                   0x0Cu
#define OSEKNM_SERVICE_ID_GET_STATUS                   0x0Du
#define OSEKNM_SERVICE_ID_MAINFUNCTION                 0x0Eu



void OsekNm_TxConfirmation(NetIdType NetId, NodeIdType NodeId);

void OsekNm_RxIndication(NetIdType NetId,NodeIdType NodeId, const OsekNm_PduType* NMPDU);

void OsekNm_MainFuntion(void);

#if ( OSEKNM_VERSION_INFO_API == STD_ON )
void OsekNm_GetVersionInfo(Std_VersionInfoType* versioninfo);
#endif /* OSEKNM_VERSION_INFO_API */

void OsekNm_ControllerBusOff(NetIdType NetId);
void OsekNm_Init( const OsekNm_ConfigType * const oseknmConfigPtr );
void OsekNm_DeInit(NetIdType netId);

void OsekNm_TimeoutNotification(NetIdType NetId,NodeIdType NodeId);

uint8 OsekNm_Internal_CheckNetId ( uint8 NetId);

uint8 OsekNm_Internal_GetNetId ( uint8 NethandleId);

uint8 OsekNm_Internal_nmHandleToNetHandle( const NetworkHandleType nmHandle);

Std_ReturnType OsekNm_NetworkRequest( const NetworkHandleType nmHandle );
Std_ReturnType OsekNm_NetworkRelease( const NetworkHandleType nmHandle );
void OsekNm_InitCMaskTable(NetIdType NetId,ConfigKindName ConfigKind, ConfigRefType CMask);
void OsekNm_InitTargetConfigTable(NetIdType NetId,ConfigKindName ConfigKind,ConfigRefType TargetConfig);
StatusType OsekNm_CmpConfig(NetIdType NetId,ConfigRefType TestConfig,ConfigRefType RefConfig,ConfigRefType CMask);
StatusType OsekNm_GetConfig(NetIdType NetId,ConfigRefType Config,ConfigKindName ConfigKind);
StatusType OsekNm_GetStatus(NetIdType netId, StatusRefType networkStatus); /*lint !e9018 'NetworkStatus' with union based type 'StatusRefType */

/*lint -e9003 */ /* could define variable 'OsekNmConfig' at block scope */
extern const OsekNm_ConfigType OsekNmConfig;

/* ********* Function required by DLL ************* */

void D_Init(NetIdType NetId,RoutineRefType InitRoutine);
void D_Offline(NetIdType NetId);
void D_Online(NetIdType NetId);
StatusType D_Window_Data_req(NetIdType netId, const OsekNm_PduType* nmPdu,uint8 dlc);



void BusInit(NetIdType NetId);
void BusSleep(NetIdType NetId);
void BusAwake(NetIdType NetId);
void BusRestart(NetIdType NetId);
void BusShutdown(NetIdType NetId);





#endif /*OSEKNM_INTERNAL_H_*/
