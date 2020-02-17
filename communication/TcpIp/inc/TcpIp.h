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

#include "ComStack_Types.h"

#define TCPIP_VENDOR_ID          60u
#define TCPIP_MODULE_ID          170u
#define TCPIP_AR_RELEASE_MAJOR_VERSION   4u
#define TCPIP_AR_RELEASE_MINOR_VERSION   2u
#define TCPIP_AR_RELEASE_PATCH_VERSION   2u

#define TCPIP_SW_MAJOR_VERSION   2u
#define TCPIP_SW_MINOR_VERSION   0u
#define TCPIP_SW_PATCH_VERSION   0u

/** @req 4.2.2/SWS_TCPIP_00042 */
#define TCPIP_E_NOTINIT         0x01u
#define TCPIP_E_PARAM_POINTER   0x02u
#define TCPIP_E_INV_ARG         0x03u
#define TCPIP_E_NOBUFS          0x04u
#define TCPIP_E_MSGSIZE         0x07u
#define TCPIP_E_PROTOTYPE       0x08u
#define TCPIP_E_ADDRINUSE       0x09u
#define TCPIP_E_ADDRNOTAVAIL    0x0Au
#define TCPIP_E_ISCONN          0x0Bu
#define TCPIP_E_NOTCONN         0x0Cu
#define TCPIP_E_NOPROTOOPT      0x0Du
#define TCPIP_E_AFNOSUPPORT     0x0Eu
#define TCPIP_E_INIT_FAILED     0x0Fu
#define TCPIP_E_ARC_ILLEGAL_STATE                 0x10u
#define TCPIP_E_ARC_ETHIF_INIT_STATE_FAILED       0x11u
#define TCPIP_E_ARC_TRCV_INIT_STATE_FAILED        0x12u
#define TCPIP_E_ARC_GENERAL_FAILURE_TCPIP         0x13u
#define TCPIP_E_ARC_GENERAL_FAILURE_TCPIP_STACK   0x14u


/*Service id's */
#define TCPIP_INIT_SERVICE_ID                         0x01u
#define TCPIP_GETVERSIONINFO_SERVICE_ID               0x02u
#define TCPIP_GETSOCKET_SERVICE_ID                    0x03u
#define TCPIP_CLOSE_SERVICE_ID                        0x04u
#define TCPIP_BIND_SERVICE_ID                         0x05u
#define TCPIP_TCPCONNECT_SERVICE_ID                   0x06u
#define TCPIP_TCPLISTEN_SERVICE_ID                    0x07u
#define TCPIP_TCPRECEIVED_SERVICE_ID                  0x08u
#define TCPIP_REQUESTCOMMODE_SERVICE_ID               0x09u
#define TCPIP_REQUESTIPADDRASSIGNMENT_SERVICE_ID      0x0au
#define TCPIP_RELEASEIPADDRASSIGNMENT_SERVICE_ID      0x0bu
#define TCPIP_ICMPTRANSMIT_SERVICE_ID                 0x0cu
#define TCPIP_DHCPREADOPTION_SERVICE_ID               0x0du
#define TCPIP_DHCPWRITEOPTION_SERVICE_ID              0x0eu
#define TCPIP_CHANGEPARAMETER_SERVICE_ID              0x0fu
#define TCPIP_GETIPADDR_SERVICE_ID                    0x10u
#define TCPIP_GETPHYSADDR_SERVICE_ID                  0x11u
#define TCPIP_UDPTRANSMIT_SERVICE_ID                  0x12u
#define TCPIP_TCPTRANSMIT_SERVICE_ID                  0x13u
#define TCPIP_RXINDICATION_SERVICE_ID                 0x14u
#define TCPIP_MAINFUNCTION_SERVICE_ID                 0x15u
#define TCPIP_GETREMOTEPHYSADDR_SERVICE_ID            0x16u
#define TCPIP_GETCTRLIDX_SERVICE_ID                   0x17u
#define TCPIP_ICMPV6TRANSMIT_SERVICE_ID               0x18u
#define TCPIP_DHCPV6READOPTION_SERVICE_ID             0x19u
#define TCPIP_DHCPV6WRITEOPTION_SERVICE_ID            0x1au
#define TCPIP_RESETIPASSIGNMENT_SERVICE_ID            0x1bu

#include "TcpIp_Types.h"
#include "TcpIp_ConfigTypes.h"
#include "TcpIp_Cfg.h"
#include "Platform_Types.h"

#if (CPU_BYTE_ORDER == HIGH_BYTE_FIRST)
/** Set an IP address given by the four byte-parts */
#define IP4_ADDR_CHR_TO_UINT32(a,b,c,d) ( ((uint32)((a) & 0xff) << 24) | ((uint32)((b) & 0xff) << 16) | ((uint32)((c) & 0xff) << 8)  | (uint32)((d) & 0xff) )
#else
#define IP4_ADDR_CHR_TO_UINT32(a,b,c,d) ( ((uint32)((d) & 0xff) << 24) | ((uint32)((c) & 0xff) << 16) | ((uint32)((b) & 0xff) << 8)  | (uint32)((a) & 0xff) )
#endif

/* @req 4.2.2/SWS_TCPIP_00006 */
#if ( TCPIP_VERSION_INFO_API == STD_ON)
/**
 * Gets the version info
 * @param versioninfo - struct holding the version info
 */
/* @req 4.2.2/SWS_TCPIP_00004 */
/* @req 4.2.2/SWS_TCPIP_00005 */
#if ( TCPIP_VERSION_INFO_API == STD_ON )
#define TcpIp_GetVersionInfo(_vi) STD_GET_VERSION_INFO(_vi,TCPIP)
#endif /* TCPIP_VERSION_INFO_API */
#endif

void TcpIp_Init(const TcpIp_ConfigType* ConfigPtr);
Std_ReturnType TcpIp_Close(TcpIp_SocketIdType SocketId, boolean Abort);
Std_ReturnType TcpIp_Bind(TcpIp_SocketIdType SocketId, TcpIp_LocalAddrIdType LocalAddrId, uint16* PortPtr);
Std_ReturnType TcpIp_TcpConnect(TcpIp_SocketIdType SocketId, const TcpIp_SockAddrType* RemoteAddrPtr);
Std_ReturnType TcpIp_TcpListen(TcpIp_SocketIdType SocketId, uint16 MaxChannels);
Std_ReturnType TcpIp_TcpReceived(TcpIp_SocketIdType SocketId, uint32 Length);
Std_ReturnType TcpIp_RequestIpAddrAssignment(TcpIp_LocalAddrIdType LocalAddrId, TcpIp_IpAddrAssignmentType Type,
        const TcpIp_SockAddrType* LocalIpAddrPtr, uint8 Netmask, const TcpIp_SockAddrType* DefaultRouterPtr);
Std_ReturnType TcpIp_ReleaseIpAddrAssignment(TcpIp_LocalAddrIdType LocalAddrId);
Std_ReturnType TcpIp_ResetIpAssignment(void);
Std_ReturnType TcpIp_IcmpTransmit(TcpIp_LocalAddrIdType LocalIpAddrId, const TcpIp_SockAddrType* RemoteAddrPtr,
        uint8 Ttl, uint8 Type, uint8 Code, uint16 DataLength, const uint8* DataPtr);
Std_ReturnType TcpIp_IcmpV6Transmit(TcpIp_LocalAddrIdType LocalIpAddrId, const TcpIp_SockAddrType* RemoteAddrPtr,
        uint8 HopLimit, uint8 Type, uint8 Code, uint16 DataLength, const uint8* DataPtr);
Std_ReturnType TcpIp_DhcpReadOption(TcpIp_LocalAddrIdType LocalIpAddrId, uint8 Option, uint8* DataLength, uint8* DataPtr);
Std_ReturnType TcpIp_DhcpV6ReadOption(TcpIp_LocalAddrIdType LocalIpAddrId, uint16 Option, uint16* DataLength, uint8* DataPtr);
Std_ReturnType TcpIp_DhcpWriteOption(TcpIp_LocalAddrIdType LocalIpAddrId, uint8 Option, uint8 DataLength, const uint8* DataPtr);
Std_ReturnType TcpIp_DhcpV6WriteOption(TcpIp_LocalAddrIdType LocalIpAddrId,uint16 Option,uint16 DataLength,const uint8* DataPtr);
Std_ReturnType TcpIp_ChangeParameter(TcpIp_SocketIdType SocketId, TcpIp_ParamIdType ParameterId, const uint8* ParameterValue);
Std_ReturnType TcpIp_GetIpAddr(TcpIp_LocalAddrIdType LocalAddrId, TcpIp_SockAddrType* IpAddrPtr,
        uint8* NetmaskPtr, TcpIp_SockAddrType* DefaultRouterPtr);
Std_ReturnType TcpIp_GetPhysAddr(TcpIp_LocalAddrIdType LocalAddrId, uint8* PhysAddrPtr);
TcpIp_ReturnType TcpIp_GetRemotePhysAddr(uint8 CtrlIdx, const TcpIp_SockAddrType* IpAddrPtr, uint8 *PhysAddrPtr, boolean initRes);
Std_ReturnType TcpIp_GetCtrlIdx(TcpIp_LocalAddrIdType LocalAddrId, uint8* CtrlIdxPtr);
Std_ReturnType TcpIp_UdpTransmit(TcpIp_SocketIdType SocketId, const uint8* DataPtr, const TcpIp_SockAddrType* RemoteAddrPtr, uint16 TotalLength);
Std_ReturnType TcpIp_TcpTransmit(TcpIp_SocketIdType SocketId, const uint8* DataPtr, uint32 AvailableLength, boolean ForceRetrieve);
void TcpIp_MainFunction(void);

/*
 * Make the TcpIp_Config visible for others.
 */
extern const TcpIp_ConfigType TcpIp_Config;

#endif /* TCPIP_H */
