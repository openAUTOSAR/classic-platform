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


#ifndef TCPIP_H
#define TCPIP_H

#define TCPIP_VENDOR_ID          1
#define TCPIP_MODULE_ID          MODULE_ID_TCPIP
#define TCPIP_AR_MAJOR_VERSION   4
#define TCPIP_AR_MINOR_VERSION   0
#define TCPIP_AR_PATCH_VERSION   2

#define TCPIP_SW_MAJOR_VERSION   1
#define TCPIP_SW_MINOR_VERSION   0
#define TCPIP_SW_PATCH_VERSION   0

#define TCPIP_SA_DATA_SIZE_IPV6   20 /* To cover sockaddr_in and sockaddr_in6. */
#define TCPIP_SA_DATA_SIZE_IPV4   4

#if (TCPIP_IPV6_ENABLED == STD_ON)
#define TCPIP_SA_DATA_SIZE   TCPIP_SA_DATA_SIZE_IPV6 /* To cover sockaddr_in and sockaddr_in6. */
#else
#define TCPIP_SA_DATA_SIZE   TCPIP_SA_DATA_SIZE_IPV4
#endif

/** @req 4.2.2/SWS_TCPIP_00082 */
typedef enum {
    TCPIP_IPADDR_STATE_ASSIGNED,
    TCPIP_IPADDR_STATE_ONHOLD,
    TCPIP_IPADDR_STATE_UNASSIGNED,
} TcpIp_IpAddrStateType;

/** @req 4.2.2/SWS_TCPIP_00009 */
typedef uint16 TcpIp_DomainType;
#define TCPIP_AF_INET 0x02u
#define TCPIP_AF_INET6 0x1Cu
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

/** @req 4.2.2/SWS_TCPIP_00065 */
typedef enum {
    TCPIP_IPADDR_ASSIGNMENT_STATIC,
    TCPIP_IPADDR_ASSIGNMENT_LINKLOCAL_DOIP,
    TCPIP_IPADDR_ASSIGNMENT_DHCP,
    TCPIP_IPADDR_ASSIGNMENT_LINKLOCAL,
    TCPIP_IPADDR_ASSIGNMENT_IPV6_ROUTER,
}TcpIp_IpAddrAssignmentType;


void TcpIp_Init(void);	/** @req SOAD193 */
void TcpIp_Shutdown(void);	/** @req SOAD194 */
void TcpIp_MainFunctionCyclic(void);	/** @req SOAD143 */
Std_ReturnType TcpIp_SetDhcpHostNameOption(uint8* HostNameOption, uint8 HostNameLen);	/** @req SOAD196*/

#endif /* TCPIP_H */
