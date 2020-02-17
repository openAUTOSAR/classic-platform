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

#ifndef LDCOM_H_
#define LDCOM_H_

/* @req 4.2.2/SWS_LDCOM_00002*/
#include "ComStack_Types.h"
#include "LdCom_Types.h"
#include "LdCom_Cbk.h"

/* General requirements */
#define LDCOM_VENDOR_ID                     60u
#define LDCOM_MODULE_ID                     49u

/* @req SWS_EthIf_00006 */
#define LDCOM_AR_RELEASE_MAJOR_VERSION      4u
#define LDCOM_AR_RELEASE_MINOR_VERSION      2u
#define LDCOM_AR_RELEASE_REVISION_VERSION   2u

#define LDCOM_AR_MAJOR_VERSION              LDCOM_AR_RELEASE_MAJOR_VERSION
#define LDCOM_AR_MINOR_VERSION              LDCOM_AR_RELEASE_MINOR_VERSION
#define LDCOM_AR_PATCH_VERSION              LDCOM_AR_RELEASE_REVISION_VERSION

#define LDCOM_SW_MAJOR_VERSION              1u
#define LDCOM_SW_MINOR_VERSION              0u
#define LDCOM_SW_PATCH_VERSION              0u

#include "LdCom_Cfg.h"

// This is needed since the RTE is using signal names (needs attention when it comes to post build)
#include "LdCom_PBCfg.h"

/* @req 4.2.2/SWS_LDCOM_00018*/
#define LDCOM_E_PARAM                0x01u
#define LDCOM_E_UNINIT               0x02u
#define LDCOM_E_PARAM_POINTER        0x03u
#define LDCOM_E_INVALID_PDU_SDU_ID   0x04u
#define LDCOM_E_INVALID_SIGNAL_ID    0x05u

/** Service IDs */
#define LDCOM_SERVICE_ID_INIT                       0x01u
#define LDCOM_SERVICE_ID_DE_INIT                    0x02u
#define LDCOM_SERVICE_ID_GET_VERSION_INFO           0x03u
#define LDCOM_SERVICE_ID_TRANSMIT                   0x05u

/* Callback notifications function service Ids */
#define LDCOM_SERVICE_ID_COPY_TX_DATA               0x43u
#define LDCOM_SERVICE_ID_TP_TX_CONFIRMATION         0x48u
#define LDCOM_SERVICE_ID_START_OF_RECEPTION         0x46u
#define LDCOM_SERVICE_ID_COPY_RX_DATA               0x44u
#define LDCOM_SERVICE_ID_TP_RX_INDICATION           0x45u
#define LDCOM_SERVICE_ID_RX_INDICATION              0x42u
#define LDCOM_SERVICE_ID_TX_CONFIRMATION            0x40u
#define LDCOM_SERVICE_ID_TRIGGER_TRANSMIT           0x41u

 /* @req 4.2.2/SWS_LDCOM_00022*/
void LdCom_Init( const LdCom_ConfigType* config );
 /* @req 4.2.2/SWS_LDCOM_00023*/
void LdCom_DeInit( void );
 /* @req 4.2.2/SWS_LDCOM_00024*/
#if (LDCOM_VERSION_INFO_API == STD_ON)
void LdCom_GetVersionInfo( Std_VersionInfoType* versioninfo );
#endif
 /* @req 4.2.2/SWS_LDCOM_00026*/
Std_ReturnType LdCom_Transmit( PduIdType Id, const PduInfoType* PduInfoPtr );

#endif /* LDCOM_H_ */

