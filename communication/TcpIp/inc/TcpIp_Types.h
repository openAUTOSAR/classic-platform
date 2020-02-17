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

#ifndef TCPIP_TYPES_H_
#define TCPIP_TYPES_H_

#include "ComStack_Types.h"

/* This header file shall export the part of the TcpIp Types */

#define TCPIP_SA_DATA_SIZE_IPV6   20u /* To cover sockaddr_in and sockaddr_in6. */
#define TCPIP_SA_DATA_SIZE_IPV4   4u

#if (TCPIP_IPV6_ENABLED == STD_ON)
#define TCPIP_SA_DATA_SIZE   TCPIP_SA_DATA_SIZE_IPV6 /* To cover sockaddr_in and sockaddr_in6. */
#else
#define TCPIP_SA_DATA_SIZE   TCPIP_SA_DATA_SIZE_IPV4
#endif

/** @req 4.2.2/SWS_TCPIP_00009 */
typedef uint16 TcpIp_DomainType;
#define TCPIP_AF_INET 0x02u
#define TCPIP_AF_INET6 0x1Cu

/** @req 4.2.2/SWS_TCPIP_00010 */
typedef enum {
    TCPIP_IPPROTO_TCP = 0x06,
    TCPIP_IPPROTO_UDP = 0x11,
} TcpIp_ProtocolType;

/** !req 4.2.2/SWS_TCPIP_00012 */
typedef struct {
    TcpIp_DomainType domain;
    uint16 port;
#if defined(CFG_GNULINUX)
    char addr[TCPIP_SA_DATA_SIZE];
#else
    uint8 addr[TCPIP_SA_DATA_SIZE];
#endif
}TcpIp_SockAddrType;

/** @req 4.2.2/SWS_TCPIP_00013 */
typedef struct {
    TcpIp_DomainType domain;
    uint16 port;
    uint32 addr[1];
}TcpIp_SockAddrInetType;

/** @req 4.2.2/SWS_TCPIP_00014 */
typedef struct {
    TcpIp_DomainType domain;
    uint16 port;
    uint32 addr[4];
}TcpIp_SockAddrInet6Type;

/** @req 4.2.2/SWS_TCPIP_00030 */
typedef uint8 TcpIp_LocalAddrIdType;

/** @req 4.2.2/SWS_TCPIP_00038 */
typedef uint16 TcpIp_SocketIdType;
#define TCPIP_SOCKETID_INVALID 0xFFFFu

/* Specifies the TcpIp state for a specific EthIf controller */
/** @req 4.2.2/SWS_TCPIP_00073 */
typedef enum {
    TCPIP_STATE_ONLINE,
    TCPIP_STATE_ONHOLD,
    TCPIP_STATE_OFFLINE,
    TCPIP_STATE_STARTUP,
    TCPIP_STATE_SHUTDOWN,
} TcpIp_StateType;

/** @req 4.2.2/SWS_TCPIP_00082 */
typedef enum {
    TCPIP_IPADDR_STATE_ASSIGNED,
    TCPIP_IPADDR_STATE_ONHOLD,
    TCPIP_IPADDR_STATE_UNASSIGNED,
} TcpIp_IpAddrStateType;

/** @req 4.2.2/SWS_TCPIP_00031 */
typedef enum {
    TCPIP_TCP_RESET,
    TCPIP_TCP_CLOSED,
    TCPIP_TCP_FIN_RECEIVED,
    TCPIP_UDP_CLOSED,
}TcpIp_EventType;

/** @req 4.2.2/SWS_TCPIP_00065 */
typedef enum {
    TCPIP_IPADDR_ASSIGNMENT_STATIC,
    TCPIP_IPADDR_ASSIGNMENT_LINKLOCAL_DOIP,
    TCPIP_IPADDR_ASSIGNMENT_DHCP,
    TCPIP_IPADDR_ASSIGNMENT_LINKLOCAL,
    TCPIP_IPADDR_ASSIGNMENT_IPV6_ROUTER,
}TcpIp_IpAddrAssignmentType;

/* map towards model */
#define TCPIP_STATIC TCPIP_IPADDR_ASSIGNMENT_STATIC
#define TCPIP_LINKLOCAL_DOIP TCPIP_IPADDR_ASSIGNMENT_LINKLOCAL_DOIP
#define TCPIP_DHCP TCPIP_IPADDR_ASSIGNMENT_DHCP
#define TCPIP_LINKLOCAL TCPIP_IPADDR_ASSIGNMENT_LINKLOCAL
#define TCPIP_IPV6_ROUTER TCPIP_IPADDR_ASSIGNMENT_IPV6_ROUTER


/** @req 4.2.2/SWS_TCPIP_00066 */
typedef enum {
    TCPIP_OK,
    TCPIP_E_NOT_OK,
    TCPIP_E_PHYS_ADDR_MISS,
}TcpIp_ReturnType;

/** @req 4.2.2/SWS_TCPIP_00126 */
typedef uint8 TcpIp_ParamIdType;
#define TCPIP_PARAMID_TCP_RXWND_MAX 0x00u
#define TCPIP_PARAMID_FRAMEPRIO 0x01u
#define TCPIP_PARAMID_TCP_NAGLE 0x02u
#define TCPIP_PARAMID_TCP_KEEPALIVE 0x03u
#define TCPIP_PARAMID_TTL 0x04u
#define TCPIP_PARAMID_TCP_KEEPALIVE_TIME 0x05u
#define TCPIP_PARAMID_TCP_KEEPALIVE_PROBES_MAX  0x06u
#define TCPIP_PARAMID_TCP_KEEPALIVE_INTERVAL 0x07u
#define TCPIP_PARAMID_VENDOR_SPECIFIC 0x80u

/** @req 4.2.2/SWS_TCPIP_00133 */
typedef uint32 TcpIp_IpAddrWildcardType;
#define TCPIP_IPADDR_ANY 0x00u

/** @req 4.2.2/SWS_TCPIP_00132 */
typedef uint32 TcpIp_Ip6AddrWildcardType;
#define TCPIP_IP6ADDR_ANY 0x00u

/** @req 4.2.2/SWS_TCPIP_00134 */
typedef uint16 TcpIp_PortWildcardType;
#define TCPIP_PORT_ANY 0x00u

/** @req 4.2.2/SWS_TCPIP_00135 */
typedef uint32 TcpIp_LocalAddrIdWildcardType;
#define TCPIP_LOCALADDRID_ANY 0x00u

typedef enum{
    TCPIP_MULTICAST,
    TCPIP_UNICAST,
}TcpIp_AddressTypeType;

typedef enum{
    TCPIP_FORGET,
    TCPIP_STORE,
}TcpIp_AssignmentLifetimeType;

typedef enum{
    TCPIP_AUTOMATIC,
    TCPIP_MANUAL,
}TcpIp_AssignmentTriggerType;

typedef void (*TcpIp_Up_RxIndication) ( TcpIp_SocketIdType SocketId, const TcpIp_SockAddrType* RemoteAddrPtr, uint8* BufPtr, uint16 Length );
typedef void (*TcpIp_Up_TcpIpEvent)(TcpIp_SocketIdType SocketId,TcpIp_EventType Event);
typedef void (*TcpIp_Up_TxConfirmation)(TcpIp_SocketIdType SocketId, uint16 Length);
typedef Std_ReturnType (*TcpIp_Up_TcpAccepted)(TcpIp_SocketIdType SocketId,TcpIp_SocketIdType SocketIdConnected, const TcpIp_SockAddrType* RemoteAddrPtr);
typedef void (*TcpIp_Up_TcpConnected)(TcpIp_SocketIdType SocketId);
typedef BufReq_ReturnType (*TcpIp_Up_CopyTxData)(TcpIp_SocketIdType SocketId, uint8* BufPtr, uint16 BufLength);
typedef void (*TcpIp_Up_LocalIpAddrAssignmentChg)(TcpIp_LocalAddrIdType IpAddrId, TcpIp_IpAddrStateType State);

#endif /*TCPIP_TYPES_H_*/
