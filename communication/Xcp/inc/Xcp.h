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

/** @reqSettings DEFAULT_SPECIFICATION_REVISION=4.1.3 */

/** @req 4.1.3/SWS_Xcp_00505 */ /*Provide Xcp.h - can not be tested with conventional module tests*/

#ifndef XCP_H_
#define XCP_H_


/*********************************************
 *            MODULE PARAMETERS              *
 *********************************************/

#define XCP_VENDOR_ID                   60u
#define XCP_MODULE_ID                   210u

#define XCP_SW_MAJOR_VERSION   2u
#define XCP_SW_MINOR_VERSION   0u
#define XCP_SW_PATCH_VERSION   0u
#define XCP_AR_RELEASE_MAJOR_VERSION        4u
#define XCP_AR_RELEASE_MINOR_VERSION        1u
#define XCP_AR_RELEASE_REVISION_VERSION     3u
#define XCP_PROTOCOL_MAJOR_VERSION 1u
#define XCP_PROTOCOL_MINOR_VERSION 0u
#define XCP_TRANSPORT_MAJOR_VERSION 1u
#define XCP_TRANSPORT_MINOR_VERSION 0u

#include "Xcp_Cfg.h"
#include "Xcp_ConfigTypes.h"

#if XCP_PROTOCOL == XCP_PROTOCOL_CAN
#include "XcpOnCan_Cbk.h"
#elif XCP_PROTOCOL == XCP_PROTOCOL_ETHERNET
#include "XcpOnEth_Cbk.h"
#endif


/*********************************************
 *               MAIN FUNCTIONS              *
 *********************************************/

/* This definitions are coming from ASR XCP Spec to identify Api Ids for Det report*/
#define XCP_API_ID_XCP_INIT                0x00u
#define XCP_API_ID_XCP_GETVERSIONINFO      0x01u
#define XCP_API_ID_XCP_RX_INDICATION       0x03u
#define XCP_API_ID_XCP_TX_CONFIRMATION     0x02u
#define XCP_API_ID_XCP_TRIGGERTRANSMIT     0x41u
#define XCP_API_ID_XCP_SETTRANSMISSIONMODE 0x05u
#define XCP_API_ID_XCP_MAINFUNCTION        0x04u

/* This definitions are necessary to identify additional Api Ids for Det report*/
#define XCP_API_ID_XCP_FIFO_GET            0xFFu
#define XCP_API_ID_XCP_FIFO_PUT            0xFEu
#define XCP_API_ID_XCP_FIFO_FREE           0xFDu
#define XCP_API_ID_XCP_FIFO_INIT           0xFBu
#define XCP_API_ID_XCP_PROCESSDAQ          0xFAu
#define XCP_API_ID_XCP_PROCESSCHANNEL      0xF9u
#define XCP_API_ID_XCP_TRANSMIT            0xF8u

/* Error Ids coming from ASR XCP*/
#define XCP_E_INIT_FAILED     0x04u
#define XCP_E_INV_POINTER     0x01u
#define XCP_E_NOT_INITIALIZED 0x02u
#define XCP_E_INVALID_PDUID   0x03u
#define XCP_E_NULL_POINTER    0x12u
#define XCP_E_INVALID_EVENT   0x13u

/* Additional Error Ids, necessary to identify more erroneous situation*/
#define XCP_E_MESSAGE_LOST    0xFFu

#if (XCP_VERSION_INFO_API == STD_ON)
void Xcp_GetVersionInfo(Std_VersionInfoType* versionInfo);
#endif /*XCP_VERION_INFO_API == STD_ON*/

void Xcp_Init(const Xcp_ConfigType* Xcp_ConfigPtr);
void Xcp_MainFunction(void);
void Xcp_MainFunction_Channel(uint32 channel);

#endif /* XCP_H_ */
