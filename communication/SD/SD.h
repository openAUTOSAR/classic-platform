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

/** @reqSettings DEFAULT_SPECIFICATION_REVISION=4.2.2 */

#ifndef SD_H
#define SD_H

#include "SoAd.h"
#if defined(USE_DEM)
#include "Dem.h"
#endif

#define SD_VENDOR_ID		   60u
#define SD_AR_RELEASE_MAJOR_VERSION      4u
#define SD_AR_RELEASE_MINOR_VERSION      2u
#define SD_AR_RELEASE_REVISION_VERSION   2u

#define SD_MODULE_ID         171u
#define SD_AR_MAJOR_VERSION  SD_AR_RELEASE_MAJOR_VERSION
#define SD_AR_MINOR_VERSION  SD_AR_RELEASE_MINOR_VERSION
#define SD_AR_PATCH_VERSION  SD_AR_RELEASE_REVISION_VERSION

#define SD_SW_MAJOR_VERSION   1u
#define SD_SW_MINOR_VERSION   0u
#define SD_SW_PATCH_VERSION   0u

// Error codes produced by this module defined by Autosar
/** @req SWS_SD_00107 **/
#define SD_E_NOT_INITIALIZED  0x01u
#define SD_E_PARAM_POINTER    0x02u
#define SD_E_INV_MODE         0x03u
#define SD_E_INV_ID           0x04u
#define SD_E_INIT_FAILED      0x05u

// Service IDs in this module defined by Autosar
#define SD_INIT_ID                				0x01u
#define SD_GET_VERSION_INFO_ID					0x02u
#define SD_LOCAL_IP_ADDR_ASSIGNMENT_CHG_ID 		0x05u
#define	SD_MAIN_FUNCTION_ID						0x06u
#define SD_SERVER_SERVICE_SET_STATE_ID			0x07u
#define SD_CLIENT_SERVICE_SET_STATE_ID			0x08u
#define SD_CONSUMED_EVENT_GROUP_SET_STATE_ID	0x09u
#define SD_RX_INDICATION_ID						0x42u


#include "SD_Types.h"
#include "SD_Cfg.h"


/** @req SWS_SD_00119 */
void Sd_Init( const Sd_ConfigType* ConfigPtr );

/** @req SWS_SD_00126 */
#if (SD_VERSION_INFO_API == STD_ON)
/** @req SWS_SD_00124 */
void Sd_GetVersionInfo( Std_VersionInfoType* versioninfo );
#endif

/** @req SWS_SD_00496 */
Std_ReturnType Sd_ServerServiceSetState( uint16 SdServerServiceHandleId, Sd_ServerServiceSetStateType ServerServiceState);

/** @req SWS_SD_00409 */
Std_ReturnType Sd_ClientServiceSetState( uint16 ClientServiceHandleID, Sd_ClientServiceSetStateType ClientServiceState );

/** @req SWS_SD_00560 */
Std_ReturnType Sd_ConsumedEventGroupSetState( uint16 SdConsumedEventGroupHandleId, Sd_ConsumedEventGroupSetStateType ConsumedEventGroupState );

/** @req SWS_SD_00412 */
void Sd_LocalIpAddrAssignmentChg( SoAd_SoConIdType SoConId, TcpIp_IpAddrStateType State );

/** @req SWS_SD_00129 */
void Sd_RxIndication( PduIdType RxPduId, PduInfoType* PduInfoPtr);

/** @req SWS_SD_00130 */
void Sd_MainFunction( void );

extern const Sd_ConfigType SdConfig;

#endif
