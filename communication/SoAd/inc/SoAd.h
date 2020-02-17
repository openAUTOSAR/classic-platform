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

#ifndef SOAD_H
#define SOAD_H

#include "SoAd_Types.h"

#define SOAD_VENDOR_ID		   60u
#define SOAD_AR_RELEASE_MAJOR_VERSION      4u
#define SOAD_AR_RELEASE_MINOR_VERSION      2u
#define SOAD_AR_RELEASE_REVISION_VERSION   2u

#define SOAD_MODULE_ID         56u
#define SOAD_AR_MAJOR_VERSION  SOAD_AR_RELEASE_MAJOR_VERSION
#define SOAD_AR_MINOR_VERSION  SOAD_AR_RELEASE_MINOR_VERSION
#define SOAD_AR_PATCH_VERSION  SOAD_AR_RELEASE_REVISION_VERSION

#define SOAD_SW_MAJOR_VERSION   2u
#define SOAD_SW_MINOR_VERSION   0u
#define SOAD_SW_PATCH_VERSION   0u

/** @req SWS_SOAD_00101 */
#define SOAD_E_NOTINIT          0x01u
#define SOAD_E_PARAM_POINTER   	0x02u
#define SOAD_E_INV_ARG          0x03u
#define SOAD_E_NOBUFS           0x04u
#define SOAD_E_INV_PDUHEADER_ID 0x05u
#define SOAD_E_INV_PDUID        0x06u
#define SOAD_E_INV_SOCKETID     0x07u
#define SOAD_E_INIT_FAILED      0x08u

// Service IDs in this module defined by Autosar
#define SOAD_INIT_ID                				0x01u
#define SOAD_GET_VERSION_INFO_ID					0x02u
#define SOAD_IF_TRANSMIT_ID          				0x03u
#define SOAD_TP_TRANSMIT_ID          				0x04u
#define SOAD_TP_CANCEL_TRANSMIT_ID          		0x05u
#define SOAD_TP_CANCEL_RECEIVE_ID          			0x06u
#define SOAD_GET_SOCON_ID_ID						0x07u
#define SOAD_OPEN_SOCON_ID							0x08u
#define SOAD_CLOSE_SOCON_ID							0x09u
#define SOAD_REQUEST_IP_ADDR_ASSIGNMENT_ID			0x0Au
#define SOAD_RELEASE_IP_ADDR_ASSIGNMENT_ID			0x0Bu
#define SOAD_GET_LOCAL_ADDR_ID						0x0Cu
#define SOAD_GET_PHYS_ADDR_ID						0x0Du
#define SOAD_ENABLE_ROUTING_ID						0x0Eu
#define SOAD_DISABLE_ROUTING_ID						0x0Fu
#define SOAD_SET_REMOTE_ADDRESS_ID					0x10u
#define SOAD_TP_CHANGE_PARAMETER_ID					0x11u
#define SOAD_RX_INDICATION_ID						0x12u
#define SOAD_COPY_TX_DATA_ID						0x13u
#define SOAD_TX_CONFIRMATION_ID						0x14u
#define SOAD_TCP_ACCEPTED_ID						0x15u
#define SOAD_TCP_CONNECTED_ID						0x16u
#define SOAD_TCP_IP_EVENT							0x17u
#define	SOAD_LOCAL_IP_ADDR_ASSIGNMENT_CHG_ID		0x18u
#define	SOAD_MAIN_FUNCTION_ID						0x19u
#define SOAD_READ_DHCP_HOST_NAME_OPTION_ID			0x1Au
#define SOAD_WRITE_DHCP_HOST_NAME_OPTION_ID			0x1Bu
#define SOAD_GET_REMOTE_ADDR_ID						0x1Cu
#define SOAD_IF_ROUTING_GROUP_TRANSMIT_ID			0x1Du
#define SOAD_SET_UNIQUE_REMOTE_ADDR_ID				0x1Eu
#define SOAD_IF_SPECIFIC_ROUTING_GROUP_TRANSMIT_ID	0x1Fu
#define SOAD_ENABLE_SPECIFIC_ROUTING_ID				0x20u
#define SOAD_DISABLE_SPECIFIC_ROUTING_ID			0x21u

#define SOAD_PDU_HEADER_LENGTH 8


#include "SoAd_Cfg.h"

//----------------------------------------------------

void SoAd_Init( const SoAd_ConfigType* SoAdConfigPtr );
/** @req SWS_SOAD_00510 */
Std_ReturnType SoAd_OpenSoCon( SoAd_SoConIdType SoConId );
/** @req SWS_SOAD_00511 */
Std_ReturnType SoAd_CloseSoCon( SoAd_SoConIdType SoConId, boolean abort );
/** @req SWS_SOAD_00515 */
Std_ReturnType SoAd_SetRemoteAddr( SoAd_SoConIdType SoConId, const TcpIp_SockAddrType* RemoteAddrPtr );
/** @req SWS_SOAD_00121 */
void SoAd_MainFunction( void );
/** @req SWS_SOAD_00091 */
Std_ReturnType SoAd_IfTransmit( PduIdType SoAdSrcPduId, const PduInfoType* SoAdSrcPduInfoPtr );
/** @req SWS_SOAD_00655 */
Std_ReturnType SoAd_GetRemoteAddr( SoAd_SoConIdType SoConId, TcpIp_SockAddrType* IpAddrPtr );
#if ( SOAD_VERSION_INFO_API == STD_ON)
void SoAd_GetVersionInfo( Std_VersionInfoType* versioninfo );
#endif
/** @req SWS_SOAD_00656 */
Std_ReturnType SoAd_IfRoutingGroupTransmit( SoAd_RoutingGroupIdType id );
/** @req SWS_SOAD_00711 */
Std_ReturnType SoAd_IfSpecificRoutingGroupTransmit( SoAd_RoutingGroupIdType id, SoAd_SoConIdType SoConId );
/** @req SWS_SOAD_00105 */
Std_ReturnType SoAd_TpTransmit( PduIdType SoAdSrcPduId, const PduInfoType* SoAdSrcPduInfoPtr );
/** @req SWS_SOAD_00522 */
Std_ReturnType SoAd_TpCancelTransmit( PduIdType PduId );
/** @req SWS_SOAD_00521 */
Std_ReturnType SoAd_TpCancelReceive( PduIdType PduId );
/** @req SWS_SOAD_00509 */
Std_ReturnType SoAd_GetSoConId( PduIdType TxPduId, SoAd_SoConIdType* SoConIdPtr );
/** @req SWS_SOAD_00520 */
Std_ReturnType SoAd_RequestIpAddrAssignment( SoAd_SoConIdType SoConId, TcpIp_IpAddrAssignmentType Type, const TcpIp_SockAddrType* LocalIpAddrPtr, uint8 Netmask, const TcpIp_SockAddrType* DefaultRouterPtr );
/** @req SWS_SOAD_00536 */
Std_ReturnType SoAd_ReleaseIpAddrAssignment( SoAd_SoConIdType SoConId );
/** @req SWS_SOAD_00506 */
Std_ReturnType SoAd_GetLocalAddr( SoAd_SoConIdType SoConId, TcpIp_SockAddrType* LocalAddrPtr, uint8* NetmaskPtr, TcpIp_SockAddrType* DefaultRouterPtr );
/** @req SWS_SOAD_00507 */
Std_ReturnType SoAd_GetPhysAddr( SoAd_SoConIdType SoConId, uint8* PhysAddrPtr );
/** @req SWS_SOAD_00516 */
Std_ReturnType SoAd_EnableRouting( SoAd_RoutingGroupIdType id );
/** @req SWS_SOAD_00714 */
Std_ReturnType SoAd_EnableSpecificRouting( SoAd_RoutingGroupIdType id, SoAd_SoConIdType SoConId );
/** @req SWS_SOAD_00517 */
Std_ReturnType SoAd_DisableRouting( SoAd_RoutingGroupIdType id );
/** @req SWS_SOAD_00717 */
Std_ReturnType SoAd_DisableSpecificRouting( SoAd_RoutingGroupIdType id, SoAd_SoConIdType SoConId );
/** @req SWS_SOAD_00671 */
Std_ReturnType SoAd_SetUniqueRemoteAddr( SoAd_SoConIdType SoConId, const TcpIp_SockAddrType* RemoteAddrPtr, SoAd_SoConIdType* AssignedSoConIdPtr );
/** @req SWS_SOAD_00508 */
Std_ReturnType SoAd_TpChangeParameter( PduIdType id, TPParameterType parameter, uint16 value );
/** @req SWS_SOAD_00681 */
Std_ReturnType SoAd_ReadDhcpHostNameOption( SoAd_SoConIdType SoConId, uint8* length, uint8* data );
/** @req SWS_SOAD_00679 */
Std_ReturnType SoAd_WriteDhcpHostNameOption( SoAd_SoConIdType SoConId, uint8 length, const uint8* data );
/* ArcCore added */
Std_ReturnType SoAd_Arc_GetSoConIdFromRxPdu( PduIdType RxPduId, SoAd_SoConIdType* SoConIdPtr );
#if defined(HOST_TEST)
SoAd_SoConModeType SoAd_Arc_GetMode(SoAd_SoConIdType SoConId);
#endif

#endif
