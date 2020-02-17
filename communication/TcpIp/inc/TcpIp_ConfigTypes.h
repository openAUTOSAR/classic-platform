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

#ifndef TCPIP_CONFIG_TYPES_H_
#define TCPIP_CONFIG_TYPES_H_

#include "TcpIp_Types.h"

#if defined(USE_DEM)
#include "Dem.h"

typedef struct {
    const Dem_EventIdType		ConnRefused;
    const Dem_EventIdType		HostUnreach;
    const Dem_EventIdType		PacketToBig;
    const Dem_EventIdType		Timedout;
} TcpIp_DemEventReferencesType;
#endif

typedef struct {
    const TcpIp_AssignmentLifetimeType AssignmentLifetime; //0..1
    const TcpIp_IpAddrAssignmentType AssignmentMethod; //1
    const uint8 AssignmentPriority; //1
    const TcpIp_AssignmentTriggerType AssignmentTrigger; //1
} TcpIp_AddressAssignmentType;

typedef struct {
    boolean ArcValid;
    uint32 StaticIpAddress; //1
    uint32 DefaultRouter; //0..1
    uint8 Netmask; //0..1
} TcpIp_StaticIpAddressConfigType;

typedef struct {
    const TcpIp_LocalAddrIdType AddressId; //1
    const TcpIp_AddressTypeType AddressType; //1
    const TcpIp_DomainType Domain; //1
    const uint8 TcpIpCtrlRef; //1
    const TcpIp_AddressAssignmentType *AddressAssignment; //1..*
    TcpIp_StaticIpAddressConfigType *StaticIpAddrConfig; //0..1
} TcpIp_LocalAddrType;

typedef struct {
    const uint16 ArpTableSizeMax;
}TcpIp_ArpConfigType;

typedef struct {
    const uint8 Dummy;
}TcpIp_DhcpConfigType;

typedef struct {
    const boolean FragmentationRxEnabled;
    const uint8 NumFragments;
    const uint16 NumReassDgrams;
    const uint32 ReassTimeout;
    const uint16 ArcMaxReceiveBufferSize;
}TcpIp_FragConfigType;

typedef struct {
    const TcpIp_ArpConfigType *ArpConfigRef; //0..1
    const TcpIp_DhcpConfigType *DhcpConfigRef; //0..1
    const TcpIp_FragConfigType *FragConfigRef; //0..1
}TcpIp_IpV4CtrlType;

typedef struct {
    const TcpIp_IpV4CtrlType *IpV4Ctrl; //0..1
    //TcpIp_IpV6CtrlType *IpV6Ctrl; //0..1
}TcpIp_IpVXCtrlType;

typedef struct {
    const uint8 FramePrioDefault; //0..1
    //TcpIpDhcpServerConfigRef; //0..1
    const uint8 EthIfCtrlRef; //1
#if defined(USE_DEM)
    const TcpIp_DemEventReferencesType CtrlDemEventParameterRefs; //0..1
#endif
    //const TcpIp_IpVXCtrlType IpVXCtrl; //1
} TcpIp_TcpIpCtrlType;

typedef struct {
    const TcpIp_ArpConfigType *ArpConfigRef; //0..*
    const TcpIp_DhcpConfigType *DhcpConfigRef; //0..*
} TcpIp_IpV4ConfigType;

typedef struct {
    const TcpIp_IpV4ConfigType *IpV4Config; //0..1
} TcpIp_IpConfigType;

typedef struct {
    const char *HeaderFileName;
    const char *Name;
}TcpIp_PhysAddrChgHandlerType;

typedef struct {
    const TcpIp_PhysAddrChgHandlerType *PhysAddrChgHandler; //0..1
} TcpIp_PhysAddrConfigType;

typedef enum {
    TCPIP_CDD,
    TCPIP_SOAD,
}TcpIp_SocketOwnerUpperLayerTypeType;

typedef struct {
    //  const char *SocketOwnerHeaderFileName; //0..1
    const TcpIp_Up_CopyTxData SocketOwnerCopyTxDataFncPtr; //0..1
    const TcpIp_Up_LocalIpAddrAssignmentChg SocketOwnerLocalIpAddrAssignmentChgFncPtr; //0..1
    const TcpIp_Up_RxIndication SocketOwnerRxIndicationFncPtr; //0..1
    const TcpIp_Up_TcpAccepted SocketOwnerTcpAcceptedFncPtr; //0..1
    const TcpIp_Up_TcpConnected SocketOwnerTcpConnectedFncPtr; //0..1
    const TcpIp_Up_TcpIpEvent SocketOwnerTcpIpEventFncPtr; //0..1
    const TcpIp_Up_TxConfirmation SocketOwnerTxConfirmationFncPtr; //0..1
    const TcpIp_SocketOwnerUpperLayerTypeType SocketOwnerUpperLayerType; //1
} TcpIp_SocketOwnerType;

typedef struct {
    const TcpIp_SocketOwnerType *SocketOwnerList; //1..*
} TcpIp_SocketOwnerConfigType;

typedef struct {
    const boolean TcpCongestionAvoidanceEnabled;
    const boolean TcpFastRecoveryEnabled;
    const boolean TcpFastRetransmitEnabled;
    const uint32 TcpFinWait2Timeout;
    const boolean TcpKeepAliveEnabled;
    const uint32 TcpKeepAliveInterval;
    const uint16 TcpKeepAliveProbesMax;
    const uint32 TcpKeepAliveTime;
    const uint8 *TcpMaxRtx;//0..1
    const uint32 TcpMsl;
    const boolean TcpNagleEnabled;
    const uint16 TcpReceiveWindowMax;
    const uint32 *TcpRetransmissionTimeout;//0..1
    const boolean TcpSlowStartEnabled;
    const uint8 TcpSynMaxRtx;
    const uint32 TcpSynReceivedTimeout; //convert to milliseconds
    const uint8 TcpTtl;
} TcpIp_TcpConfig;

typedef struct {
    const uint8 UdpTtl;
} TcpIp_UdpConfig;

typedef struct {
    const TcpIp_TcpIpCtrlType *CtrlList; //1..*
    //TcpIp_DhcpServerConfig *TcpIpDhcpServerConfig; //0..*
//	const TcpIp_IpConfigType *IpConfig; //0..1
    const TcpIp_LocalAddrType *LocalAddrList; //1..*
    //TcpIp_NvmBlock *NvmBlock; //0..1
    const TcpIp_PhysAddrConfigType *PhysAddrConfig; //0..1
    const TcpIp_SocketOwnerConfigType SocketOwnerConfig; //1
    //const TcpIp_TcpConfig *TcpConfig; //0..1
    //TcpIp_UdpConfig *UdpConfig; //0..1
} TcpIp_TcpIpConfigType;

/** @req 4.2.2/SWS_TCPIP_00067 */
typedef struct {
    // Containers
    const TcpIp_TcpIpConfigType Config; // 1
    //TcpIp_TcpIpGeneralType General; // 1
} TcpIp_ConfigType;



#endif /*TCPIP_CONFIG_TYPES_H_*/
