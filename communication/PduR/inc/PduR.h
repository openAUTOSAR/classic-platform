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


/* @req PDUR292 */
/* @req PDUR0778 */

#ifndef PDUR_H
#define PDUR_H


#define PDUR_VENDOR_ID		   60u
#define PDUR_AR_RELEASE_MAJOR_VERSION      4u
#define PDUR_AR_RELEASE_MINOR_VERSION      0u
#define PDUR_AR_RELEASE_REVISION_VERSION   3u

#define PDUR_MODULE_ID         51u
#define PDUR_AR_MAJOR_VERSION  PDUR_AR_RELEASE_MAJOR_VERSION
#define PDUR_AR_MINOR_VERSION  PDUR_AR_RELEASE_MINOR_VERSION
#define PDUR_AR_PATCH_VERSION  PDUR_AR_RELEASE_REVISION_VERSION

#define PDUR_SW_MAJOR_VERSION  3u
#define PDUR_SW_MINOR_VERSION  0u
#define PDUR_SW_PATCH_VERSION  1u



// ERROR CODES
/* @req PDUR231 */
#define PDUR_E_CONFIG_PTR_INVALID 				0x00u
#define PDUR_E_INVALID_REQUEST 					0x01u
#define PDUR_E_PDU_ID_INVALID					0x02u
#define PDUR_E_TP_TX_REQ_REJECTED				0x03u
#define PDUR_E_PARAM_INVALID        			0x04u
#define PDUR_E_DUPLICATE_IPDU_ID    			0x06u
#define PDUR_E_IPDU_TOO_LONG					0x07u
#define PDUR_E_ROUTING_PATH_GROUP_ID_INVALID  	0x08u
#define PDUR_E_NULL_POINTER         			0x09u
#define PDUR_E_PDU_INSTANCES_LOST				0x0au
#define PDUR_E_INIT_FAILED						0x0bu
#define PDUR_E_DATA_PTR_INVALID			        PDUR_E_NULL_POINTER
#define PDUR_E_BUFFER_ERROR				        0x0cu

#define PDUR_INSTANCE_ID	0u

#define PDUR_SERVICEID_CANIFRXINDICATION        0x00u
#define PDUR_SERVICEID_ENABLEROUTING            0xf3u
#define PDUR_SERVICEID_GETCONFIGURATIONID       0xf2u
#define PDUR_SERVICEID_DISABLEROUTING           0xf4u

#define PDUR_INVALID_CONFIGID                   0xFFu

/* @req PDUR132 */

#include "PduR_Types.h"
#include "PduR_Cfg.h"

#include "PduR_If.h"

#if PDUR_COM_SUPPORT == STD_ON
#include "PduR_Com.h"
#endif
#if PDUR_CANIF_SUPPORT == STD_ON
#include "PduR_CanIf.h"
#endif
#if PDUR_CANNM_SUPPORT == STD_ON
#include "PduR_CanNm.h"
#endif
#if PDUR_UDPNM_SUPPORT == STD_ON
#include "PduR_UdpNm.h"
#endif
#if PDUR_LINIF_SUPPORT == STD_ON
#include "PduR_LinIf.h"
#endif
#if PDUR_CANTP_SUPPORT == STD_ON
#include "PduR_CanTp.h"
#endif
#if PDUR_J1939TP_SUPPORT == STD_ON
#include "PduR_J1939Tp.h"
#endif
#if PDUR_DCM_SUPPORT == STD_ON
#include "PduR_Dcm.h"
#endif
#if PDUR_SOAD_SUPPORT == STD_ON
#include "PduR_SoAd.h"
#endif
#if PDUR_DOIP_SUPPORT == STD_ON
#include "PduR_DoIP.h"
#endif
#if PDUR_J1939TP_SUPPORT == STD_ON
#include "PduR_J1939Tp.h"
#endif
#if PDUR_IPDUM_SUPPORT == STD_ON
#include "PduR_IpduM.h"
#endif
#if PDUR_FRIF_SUPPORT == STD_ON
#include "PduR_FrIf.h"
#endif
#if PDUR_FRTP_SUPPORT == STD_ON
#include "PduR_FrTp.h"
#endif
#if PDUR_FRNM_SUPPORT == STD_ON
#include "PduR_FrNm.h"
#endif
#if PDUR_LINTP_SUPPORT == STD_ON
#include "PduR_LinTp.h"
#endif
#if PDUR_CDD_LINSLV_SUPPORT == STD_ON
#include "PduR_CDD_LinSlvIf.h"
#endif
#if PDUR_SECOC_SUPPORT == STD_ON
#include "PduR_SecOC.h"
#endif
#if PDUR_CDD_PDUR_SUPPORT == STD_ON
#include "PduR_CddPduR.h"
#endif
#if PDUR_LDCOM_SUPPORT == STD_ON
#include "PduR_LdCom.h"
#endif

/* Contain the current state of the PDU router. The router is uninitialized
 * until PduR_Init has been run.
 */
//PduR_StateType PduRState;
/*lint -e{9046} */
extern const PduR_PBConfigType *PduRConfig;

/*
 *  Allocated RAM buffers for PduR
 */
/*lint -e9003 PduR_RamBufCfg cannot be defined at block scope because it is used in multiple places */
extern const PduR_RamBufCfgType PduR_RamBufCfg;

/*
 * The state of the PDU router.
 */
extern PduR_StateType PduRState;

#define PduR_IsUpModule(_mod) ((_mod > ARC_PDUR_UP_MODULES) && (_mod < ARC_PDUR_LOIF_MODULES))
#define PduR_IsIfModule(_mod) ((_mod > ARC_PDUR_LOIF_MODULES) && (_mod < ARC_PDUR_LOTP_MODULES))
#define PduR_IsTpModule(_mod) ((_mod > ARC_PDUR_LOTP_MODULES) && (_mod < ARC_PDUR_END_OF_MODULES))
#define PduR_IsLoModule(_mod) (PduR_IsIfModule(_mod) || PduR_IsTpModule(_mod))


/* @req PDUR101 */
/* @req PDUR331 */
#if (PDUR_DEV_ERROR_DETECT == STD_ON)

#define PDUR_DET_REPORTERROR(_x,_y,_z,_o) (void)Det_ReportError(_x,_y,_z,_o)

#define PDUR_VALIDATE_INITIALIZED(_api,...) \
    if ((PduRState == PDUR_UNINIT) || (PduRState == PDUR_REDUCED)) { \
        (void)Det_ReportError(PDUR_MODULE_ID, PDUR_INSTANCE_ID, _api, PDUR_E_INVALID_REQUEST); \
        return __VA_ARGS__; \
    }

#define PDUR_VALIDATE_PDUPTR(_api, _pduPtr, ...) \
    if (_pduPtr == NULL) { \
        (void)Det_ReportError(PDUR_MODULE_ID, PDUR_INSTANCE_ID, _api, PDUR_E_NULL_POINTER); \
        return __VA_ARGS__; \
    }

#define PDUR_VALIDATE_PDUID(_api, _pduId, ...) \
    if (_pduId >= PduRConfig->NRoutingPaths) { \
        (void)Det_ReportError(PDUR_MODULE_ID, PDUR_INSTANCE_ID, _api, PDUR_E_PDU_ID_INVALID); \
        return __VA_ARGS__; \
    }


#else
#define PDUR_DET_REPORTERROR(_x,_y,_z,_o)
#define PDUR_VALIDATE_INITIALIZED(_api,...) \
    if ((PduRState == PDUR_UNINIT) || (PduRState == PDUR_REDUCED)) { \
        return __VA_ARGS__; \
    }

#define PDUR_VALIDATE_PDUPTR(_api, _pduPtr, ...) \
    if (_pduPtr == NULL) { \
        return __VA_ARGS__; \
    }

#define PDUR_VALIDATE_PDUID(_api, _pduId, ...) \
    if (_pduId >= PduRConfig->NRoutingPaths) { \
        return __VA_ARGS__; \
    }

#endif

/* Zero Cost Operation function definitions
 * These macros replaces the original functions if zero cost
 * operation is desired. */
#if PDUR_ZERO_COST_OPERATION == STD_ON
#define PduR_Init(...)
#define PduR_GetVersionInfo(...)
#define PduR_GetConfigurationId(...) 0

#else // Not zero cost operation
//#error fail

/* @req PDUR334 */
void PduR_Init(const PduR_PBConfigType* ConfigPtr);

#if PDUR_VERSION_INFO_API == STD_ON
/* @req PDUR234 */
void PduR_GetVersionInfo(Std_VersionInfoType* versionInfo);
#endif

PduR_PBConfigIdType PduR_GetConfigurationId(void);
void PduR_EnableRouting(PduR_RoutingPathGroupIdType id);
void PduR_DisableRouting(PduR_RoutingPathGroupIdType id);

#endif

#endif /* PDUR_H */
