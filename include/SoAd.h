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

#ifndef SOAD_H
#define SOAD_H

#include "Modules.h"

#define SOAD_VENDOR_ID		   VENDOR_ID_ARCCORE
#define SOAD_AR_RELEASE_MAJOR_VERSION      4
#define SOAD_AR_RELEASE_MINOR_VERSION      0
#define SOAD_AR_RELEASE_REVISION_VERSION   3

#define SOAD_MODULE_ID         MODULE_ID_SOAD
#define SOAD_AR_MAJOR_VERSION  SOAD_AR_RELEASE_MAJOR_VERSION
#define SOAD_AR_MINOR_VERSION  SOAD_AR_RELEASE_MINOR_VERSION
#define SOAD_AR_PATCH_VERSION  SOAD_AR_RELEASE_REVISION_VERSION

#define SOAD_SW_MAJOR_VERSION   1
#define SOAD_SW_MINOR_VERSION   0
#define SOAD_SW_PATCH_VERSION   0

#define DOIP_VENDOR_ID          1
#define DOIP_MODULE_ID          MODULE_ID_DOIP
#define DOIP_AR_MAJOR_VERSION   4
#define DOIP_AR_MINOR_VERSION   0
#define DOIP_AR_PATCH_VERSION   2

#define DOIP_SW_MAJOR_VERSION   1
#define DOIP_SW_MINOR_VERSION   0
#define DOIP_SW_PATCH_VERSION   0

#define TCPIP_VENDOR_ID          1
#define TCPIP_MODULE_ID          MODULE_ID_TCPIP
#define TCPIP_AR_MAJOR_VERSION   4
#define TCPIP_AR_MINOR_VERSION   0
#define TCPIP_AR_PATCH_VERSION   2

#define TCPIP_SW_MAJOR_VERSION   1
#define TCPIP_SW_MINOR_VERSION   0
#define TCPIP_SW_PATCH_VERSION   0

#include "SoAd_Types.h"
#include "SoAd_Cbk.h"
#include "SoAd_Cfg.h"
#include "SoAd_Callouts.h"
#include "SoAd_ConfigTypes.h"

//#include "Bsd_Types.h"
#include "ComStack_Types.h"

#if defined(USE_PDUR)
#include "PduR.h"
#endif

#if defined(USE_COM)
#include "Com.h"
#endif

#if (SOAD_DEV_ERROR_DETECT == STD_ON)
// Error codes produced by this module defined by Autosar
#define SOAD_E_NOTINIT				0x01
#define SOAD_E_NOENT				0x02
#define SOAD_WRONG_PARAM_VAL		0x03
#define SOAD_E_NULL_PTR				0x06
#define SOAD_E_BADF					0x09
#define SOAD_E_DEADLK				0x0B
#define SOAD_E_NOMEM				0x0C
#define SOAD_E_ACCES				0x0D
#define SOAD_E_NOTDIR				0x14
#define SOAD_E_ISDIR				0x15
#define SOAD_E_INVAL				0x16
#define SOAD_E_NFILE				0x17
#define SOAD_E_MFILE				0x18
#define SOAD_E_ROFS					0x1E
#define SOAD_E_DOM					0x21
#define SOAD_E_WOULDBLOCK			0x22
#define SOAD_E_INPROGRESS			0x24
#define SOAD_E_ALREADY				0x25
#define SOAD_E_NOTSOCK				0x26
#define SOAD_E_DESTADDRREQ			0x27
#define SOAD_E_MSGSIZE				0x28
#define SOAD_E_PROTOTYPE			0x29
#define SOAD_E_NOPROTOOPT			0x2A
#define SOAD_E_PROTONOSUPPORT		0x2B
#define SOAD_E_OPNOTSUPP			0x2D
#define SOAD_E_NOTSUP				0x2E
#define SOAD_E_AFNOSUPPORT			0x2F
#define SOAD_E_ADDRINUSE			0x30
#define SOAD_E_ADDRNOTAVAIL			0x31
#define SOAD_E_NOBUFS				0x37
#define SOAD_E_ISCONN				0x38
#define SOAD_E_LOOP					0x3D
#define SOAD_E_NAMETOOLONG			0x3F
#define SOAD_E_NOLCK				0x4D
#define SOAD_E_OVERFLOW				0x54
#define SOAD_E_TCPIPUNKNOWN			0x5A
#define SOAD_E_PDU2LONG				0x5B
#define SOAD_E_NOCONNECTOR			0x5C
#define SOAD_E_INVALID_TXPDUID		0x5D
#define SOAD_E_PARAM_POINTER		0x5E

// Other error codes reported by this module
#define SOAD_E_UL_RXBUFF			0xfa
#define SOAD_E_CONFIG_INVALID		0xfb
#define SOAD_E_UNEXPECTED_EXECUTION	0xfc
#define SOAD_E_SHALL_NOT_HAPPEN		0xfd
#define SOAD_E_NOT_SUPPORTED		0xfe
#define SOAD_E_NOT_IMPLEMENTED_YET	0xff

// Service IDs in this module defined by Autosar
#define SOAD_INIT_ID                		0x01
#define SOAD_SOCKET_RESET_ID        		0x07
#define SOAD_IF_TRANSMIT_ID          		0x08
#define SOAD_SHUTDOWN_ID            		0x09
#define SOAD_GET_VERSION_INFO_ID			0x0b
#define SOAD_TP_TRANSMIT_ID          		0x0f
#define SOAD_MAIN_FUNCTION_ID       		0x10

#define DOIP_GET_VERSION_INFO_ID			0x60

#define TCPIP_INIT_ID               		0x80
#define TCPIP_SHUTDOWN_ID           		0x81
#define TCPIP_GET_VERION_INFO_ID			0x8a
#define TCPIP_MAIN_FUNCTION_CYCLIC_ID 		0x8b
#define TCPIP_SET_DHCP_HOST_NAME_OPTION_ID	0x89

// Other service IDs reported by this module
#define SOAD_SOCKET_TCP_READ_ID				0xa0
#define SOAD_SOCKET_UDP_READ_ID				0xa1
#define SOAD_SCAN_SOCKETS_ID				0xa2
#define SOAD_SOCKET_CLOSE_ID				0xa3

#define SOAD_DOIP_HANDLE_DIAG_MSG_ID		0xb0
#define SOAD_DOIP_CREATE_AND_SEND_NACK_ID	0xb1
#define SOAD_DOIP_CREATE_AND_SEND_D_ACK_ID	0xb2
#define SOAD_DOIP_CREATE_AND_SEND_D_NACK_ID	0xb3
#define SOAD_DOIP_HANDLE_VEHICLE_ID_REQ_ID	0xb4
#define SOAD_DOIP_ROUTING_ACTIVATION_REQ_ID	0xb5
#define SOAD_DOIP_HANDLE_TP_TRANSMIT_ID		0xb6
#define SOAD_DOIP_HANDLE_TCP_RX_ID			0xb7
#define SOAD_DOIP_HANDLE_UDP_RX_ID			0xb8
#define SOAD_DOIP_ENTITY_STATUS_REQ_ID			0xb9
#define SOAD_DOIP_CREATE_AND_SEND_ALIVE_CHECK_ID 0xba
#define DOIP_HANDLE_ALIVECHECK_RESP 0xbb
#define DOIP_HANDLE_ALIVECHECK_TIMEOUT 0xbc
#endif




void SoAd_Init(void);	/** @req SOAD093 */
Std_ReturnType SoAd_Shutdown(void);	/** @req SOAD092 */
void SoAd_MainFunction(void);	/** @req SOAD121 */
void SoAd_SocketReset(void);	/** @req SOAD127 */
Std_ReturnType SoAdIf_Transmit(PduIdType SoAdSrcPduId, const PduInfoType* SoAdSrcPduInfoPtr);	/** @req SOAD091 */
Std_ReturnType SoAdTp_Transmit(PduIdType SoAdSrcPduId, const PduInfoType* SoAdSrcPduInfoPtr);	/** @req SOAD105 */

void TcpIp_Init(void);	/** @req SOAD193 */
void TcpIp_Shutdown(void);	/** @req SOAD194 */
void TcpIp_MainFunctionCyclic(void);	/** @req SOAD143 */
Std_ReturnType TcpIp_SetDhcpHostNameOption(uint8* HostNameOption, uint8 HostNameLen);	/** @req SOAD196*/

void DoIp_Init();


#if ( SOAD_VERSION_INFO_API == STD_ON )
#define SoAd_GetVersionInfo(_vi) STD_GET_VERSION_INFO(_vi,SOAD)	/** @req SOAD096 */
#endif

#if ( DOIP_VERSION_INFO_API == STD_ON )
#define DoIp_GetVersionInfo(_vi) STD_GET_VERSION_INFO(_vi,DOIP)	/** @req SOAD095 */
#endif

#if ( TCPIP_VERSION_INFO_API == STD_ON )
#define TcpIp_GetVersionInfo(_vi) STD_GET_VERSION_INFO(_vi,TCPIP)	/** @req SOAD094 */
#endif

#endif
