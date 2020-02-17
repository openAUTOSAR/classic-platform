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
 /** @tagSettings DEFAULT_ARCHITECTURE=LINUX */ 
/** @req 4.2.2/SWS_TCPIP_00052 */
/** @req 4.2.2/SWS_TCPIP_00148 */
/** @req 4.2.2/SWS_TCPIP_00053 */
/** @req 4.2.2/SWS_TCPIP_00095 */
/** @req 4.2.2/SWS_TCPIP_00096 */
/** @req 4.2.2/SWS_TCPIP_00102 */
/** @req 4.2.2/SWS_TCPIP_00097 */
/** @req 4.2.2/SWS_TCPIP_00098 */
/** !req 4.2.2/SWS_TCPIP_00054 */ /* Fragmentation */
/** !req 4.2.2/SWS_TCPIP_00231 */
/** @req 4.2.2/SWS_TCPIP_00055 */
/** @req 4.2.2/SWS_TCPIP_00056 */
/** !req 4.2.2/SWS_TCPIP_00091 */ /* arp TcpIpArpTableEntryTimeout */
/** @req 4.2.2/SWS_TCPIP_00092 */
/** !req 4.2.2/SWS_TCPIP_00142 */ /* Arp Table change callback */
/** @req 4.2.2/SWS_TCPIP_00093 */
/** @req 4.2.2/SWS_TCPIP_00151 */
/** @req 4.2.2/SWS_TCPIP_00057 */
/** @req 4.2.2/SWS_TCPIP_00059 */
/** !req 4.2.2/SWS_TCPIP_00099 */
/** @req 4.2.2/SWS_TCPIP_00100 */
/** @req 4.2.2/SWS_TCPIP_00130 */
/** @req 4.2.2/SWS_TCPIP_00060 */
/** @req 4.2.2/SWS_TCPIP_00103 */
/** @req 4.2.2/SWS_TCPIP_00061 */
/** @req 4.2.2/SWS_TCPIP_00062 */
/** @req 4.2.2/SWS_TCPIP_00063 */
/** @req 4.2.2/SWS_TCPIP_00064 */
/** @req 4.2.2/SWS_TCPIP_00168 */
/** !req 4.2.2/SWS_TCPIP_00200 */ /* dhcp server */
/** !req 4.2.2/SWS_TCPIP_00201 */ /* dhcp server */
/** !req 4.2.2/SWS_TCPIP_00218 */ /* dhcp server */
/** !req 4.2.2/SWS_TCPIP_00200 */ /* dhcp server */
/** @req 4.2.2/SWS_TCPIP_00058 */
/** @req 4.2.2/SWS_TCPIP_00152 */
/** @req 4.2.2/SWS_TCPIP_00169 */
/** @req 4.2.2/SWS_TCPIP_00170 */
/** @req 4.2.2/SWS_TCPIP_00171 */
/** @req 4.2.2/SWS_TCPIP_00172 */
/** @req 4.2.2/SWS_TCPIP_00173 */
/** @req 4.2.2/SWS_TCPIP_00174 */
/** @req 4.2.2/SWS_TCPIP_00175 */
/** @req 4.2.2/SWS_TCPIP_00176 */
/** @req 4.2.2/SWS_TCPIP_00177 */
/** @req 4.2.2/SWS_TCPIP_00178 */
/** @req 4.2.2/SWS_TCPIP_00179 */
/** @req 4.2.2/SWS_TCPIP_00180 */
/** !req 4.2.2/SWS_TCPIP_00191 *//* Arp handled by lwip */
/** !req 4.2.2/SWS_TCPIP_00192 *//* Arp handled by lwip */
/** @req 4.2.2/SWS_TCPIP_00008 */
/** !req 4.2.2/SWS_TCPIP_00202 *//* TcpIp tiemout handled by lwip */

/** !req 4.2.2/SWS_TCPIP_00203 */ /* hw checksum, no cfg parameter ? Always using hw chk if available*/
/** !req 4.2.2/SWS_TCPIP_00204 */ /* hw checksum, no cfg parameter ? Always using hw chk if available*/

/** !req 4.2.2/SWS_TCPIP_00149 */ /* IPv6*/
/** !req 4.2.2/SWS_TCPIP_00150 */ /* IPv6*/
/** !req 4.2.2/SWS_TCPIP_00153 *//* IPv6*/
/** !req 4.2.2/SWS_TCPIP_00154 */ /* IPv6*/
/** !req 4.2.2/SWS_TCPIP_00156 */ /* IPv6*/
/** !req 4.2.2/SWS_TCPIP_00157 */ /* IPv6*/
/** !req 4.2.2/SWS_TCPIP_00158 */ /* IPv6*/
/** !req 4.2.2/SWS_TCPIP_00149 */ /* IPv6*/
/** !req 4.2.2/SWS_TCPIP_00159 */ /* IPv6*/
/** !req 4.2.2/SWS_TCPIP_00160 */ /* IPv6*/
/** !req 4.2.2/SWS_TCPIP_00161 */ /* IPv6*/
/** !req 4.2.2/SWS_TCPIP_00155 */ /* IPv6*/
/** !req 4.2.2/SWS_TCPIP_00232 */ /* IPv6*/
/** !req 4.2.2/SWS_TCPIP_00162 */ /* IPv6*/
/** !req 4.2.2/SWS_TCPIP_00163 */ /* IPv6*/
/** !req 4.2.2/SWS_TCPIP_00164 */ /* IPv6*/
/** !req 4.2.2/SWS_TCPIP_00165 */ /* IPv6*/
/** !req 4.2.2/SWS_TCPIP_00099 */ /* IPv6*/
/** !req 4.2.2/SWS_TCPIP_00166 */ /* IPv6*/
/** !req 4.2.2/SWS_TCPIP_00167 */ /* IPv6*/
/** !req 4.2.2/SWS_TCPIP_00181 */ /* IPv6*/
/** !req 4.2.2/SWS_TCPIP_00182 */ /* IPv6*/
/** !req 4.2.2/SWS_TCPIP_00183 */ /* IPv6*/
/** !req 4.2.2/SWS_TCPIP_00184 */ /* IPv6*/
/** !req 4.2.2/SWS_TCPIP_00185 */ /* IPv6*/
/** !req 4.2.2/SWS_TCPIP_00193 */ /* IPv6*/
/** !req 4.2.2/SWS_TCPIP_00194 */ /* IPv6*/
/** !req 4.2.2/SWS_TCPIP_00101 */ /* IPv6*/

/** !req 4.2.2/SWS_TCPIP_00219 */ /* Nvm store*/

#include "TcpIp.h"
#include "Eth_GeneralTypes.h"
#include "SchM_TcpIp.h"
#include "MemMap.h"
#include <string.h>

#ifndef _WIN32
#include <stdlib.h>
#include <unistd.h> // close()
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <linux/netdevice.h>
#include <sys/socket.h>
#else
#include "os_stubs.h"
#endif

#if defined(USE_ETHSM)
#include "EthSM_Cbk.h"
#endif

#if defined(USE_ETHIF)
#include "EthIf.h"
#endif

/** @req 4.2.2/SWS_TCPIP_00068 */
#if defined(USE_DEM)
#include "Dem.h"
#endif
#if defined(USE_SOAD)
#include "SoAd.h"
#endif

#if  ( TCPIP_DEV_ERROR_DETECT == STD_ON )
#if defined(USE_DET)
#include "Det.h"
#endif

#include "linos_logger.h" /* Logger functions */

// IMPROVEMENT SimonG 
const TcpIp_ConfigType TcpIp_Config;

int gnulinuxTcpBindAny = FALSE;

#define TCPIP_DET_REPORTERROR(_api,_err)                              \
    do {                                                            \
        (void)Det_ReportError(TCPIP_MODULE_ID, 0, _api, _err);    \
    } while(0)

#else

#define TCPIP_DET_REPORTERROR(_api,_err)

#endif

#define VALIDATE(_exp,_api,_err ) \
        if( !(_exp) ) { \
          TCPIP_DET_REPORTERROR(_api, _err); \
          return E_NOT_OK; \
        }

#define VALIDATE_RV(_exp,_api,_err,_rv ) \
        if( !(_exp) ) { \
          TCPIP_DET_REPORTERROR(_api, _err); \
          return _rv; \
        }

#define VALIDATE_NO_RV(_exp,_api,_err ) \
  if( !(_exp) ) { \
          TCPIP_DET_REPORTERROR(_api, _err); \
          return; \
        }

/* Largest needed packet size  is now defined in Application config */
//#define TCPIP_RX_BUFFER_SIZE 8192u


/*
 * Local variables
 */
typedef enum {
    TCPIP_SOCKET_INIT,
    TCPIP_SOCKET_BIND,
    TCPIP_SOCKET_TCP_CONNECT_LISTEN,
    TCPIP_SOCKET_TCP_LISTENING,
    TCPIP_SOCKET_TCP_READY,
    TCPIP_SOCKET_UDP_READY

} TcpIp_SocketStateType;


typedef struct {
    TcpIp_SocketStateType               socketState;
    boolean                             socketProtocolIsTcp;
    int                                 socketHandle;
    uint8                               socketOwnerId;
    const TcpIp_LocalAddrType           *ptrLocalAddr;
    TcpIp_SockAddrType            remoteAddr;
}TcpIp_SocketAdminType;


typedef struct {
    boolean linkUp;
    const TcpIp_LocalAddrType *ptrLocalUnicastAddr;
    TcpIp_StateType tcpipEthIfCtrlState;
    TcpIp_StateType tcpipEthIfRequestedCtrlState;
    struct ifaddrs *tcpipNetif;
}TcpIp_CtrlAdminType;

//typedef struct {
//}TcpIp_LocalAddrAdminType;

/** @req 4.2.2/SWS_TCPIP_00083 */
static TcpIp_CtrlAdminType TcpIp_CtrlAdmin[TCPIP_NOF_CONTROLLERS];

static TcpIp_SocketAdminType TcpIp_SocketAdmin[TCPIP_MAX_NOF_SOCKETS];
static uint16 TcpIp_NofUsedSockets = 0;

boolean tcpip_initialized = FALSE;


/* External functions */
extern Std_ReturnType Det_ReportError(uint16 ModuleId, uint8 InstanceId, uint8 ApiId, uint8 ErrorId);



/*
 * Local tcpip helper functions
 */

static void TcpIp_IpAddr32To8(uint32 src, char *dest)
{
#if (CPU_BYTE_ORDER == HIGH_BYTE_FIRST)
    dest[0] = (uint8)((src & 0xff000000) >> 24);
    dest[1] = (uint8)((src & 0x00ff0000) >> 16);;
    dest[2] = (uint8)((src & 0x0000ff00) >> 8);;
    dest[3] = (uint8)(src & 0x000000ff);
#else
    dest[3] = (uint8)((src & 0xff000000) >> 24);
    dest[2] = (uint8)((src & 0x00ff0000) >> 16);;
    dest[1] = (uint8)((src & 0x0000ff00) >> 8);;
    dest[0] = (uint8)(src & 0x000000ff);
#endif
}

static void TcpIp_IpAddr8to32(const char *src, uint32 *dest)
{
#if (CPU_BYTE_ORDER == HIGH_BYTE_FIRST)
    *dest = ( ((uint32)((src[0]) & 0xff) << 24) | ((uint32)((src[1]) & 0xff) << 16) | ((uint32)((src[2]) & 0xff) << 8)  | (uint32)((src[3]) & 0xff) );
#else
    *dest = ( ((uint32)((src[3]) & 0xff) << 24) | ((uint32)((src[2]) & 0xff) << 16) | ((uint32)((src[1]) & 0xff) << 8)  | (uint32)((src[0]) & 0xff) );
#endif
}


/*
 * Simple function to display an IP address.
 *
 * buffer should be at least INET_ADDRSTRLEN long.
 *
*/
void TcpIp_IpAddrToChar(char *buffer, uint32 src)
{
#ifndef _WIN32
    char addr[INET_ADDRSTRLEN];
    // char buffer[INET_ADDRSTRLEN];

    TcpIp_IpAddr32To8(src, addr);

    inet_ntop(AF_INET,&addr, buffer, INET_ADDRSTRLEN);
#endif
}



static void TcpIp_SwitchEthIfCtrlState(uint8 CtrlIdx, TcpIp_StateType TcpIpState)
{
        /** @req 4.2.2/SWS_TCPIP_00084 */
        /* After each transition the TcpIp module shall report the new
 *         state to EthSM via EthSM_TcpIpModeIndication() */
    TcpIp_CtrlAdmin[CtrlIdx].tcpipEthIfCtrlState = TcpIpState;
#if defined(USE_ETHSM)
   (void)EthSM_TcpIpModeIndication(CtrlIdx, TcpIpState); /*no return check, nothing can do about it*/
#endif
}


/* Gets the index of a controller in the global configuration structure. */

/* Gets the index of a controller in the global configuration structure. */
static inline uint8 getLocalAddrListConfigIdx(uint8 addressId) {

    for (uint8 i = 0; i < TCPIP_NOF_LOCALADDR; i++) {
        if (TcpIp_Config.Config.LocalAddrList[i].AddressId == addressId) {
            return i;
        }
    }
    return 0;
}

static Std_ReturnType TcpIp_GetFirstFreeSocket(TcpIp_SocketIdType *SocketIdPtr){
    Std_ReturnType result = E_NOT_OK;
    SchM_Enter_TcpIp_EA_0();
    for(int i=0; i < TCPIP_MAX_NOF_SOCKETS; i++){
        if( (TcpIp_SocketAdmin[i].socketState == TCPIP_SOCKET_INIT) &&
                (TcpIp_SocketAdmin[i].socketHandle == -1))
        {
            *SocketIdPtr = i;
            TcpIp_NofUsedSockets++;
            result = E_OK;
            break;
        }
    }
    SchM_Exit_TcpIp_EA_0();
    return result;
}


static void TcpIp_FreeUpSocket(TcpIp_SocketIdType SocketId){
    SchM_Enter_TcpIp_EA_0();
    TcpIp_SocketAdmin[SocketId].socketState = TCPIP_SOCKET_INIT;
    TcpIp_SocketAdmin[SocketId].socketHandle = -1;
    TcpIp_SocketAdmin[SocketId].ptrLocalAddr = NULL;
    TcpIp_NofUsedSockets--;
    SchM_Exit_TcpIp_EA_0();
}

static void TcpIp_SocketStatusCheck(TcpIp_SocketIdType socketId)
{
#ifndef _WIN32
    int sockErr;
    uint32 sockErrLen = sizeof(sockErr);
    boolean closeSock = FALSE;
    TcpIp_EventType event;

    getsockopt(TcpIp_SocketAdmin[socketId].socketHandle, SOL_SOCKET, SO_ERROR, &sockErr, &sockErrLen);

    switch(sockErr){
        case 0:
        case EAGAIN:
            /* OK */
            break;

//        case ECONNRESET:
        default:
            closeSock = TRUE;
            break;
    }

    if(closeSock){
        close(TcpIp_SocketAdmin[socketId].socketHandle);
        if(TcpIp_SocketAdmin[socketId].socketProtocolIsTcp){
            event = TCPIP_TCP_CLOSED;
        }else{
            event = TCPIP_UDP_CLOSED;
        }
        /** @req 4.2.2/SWS_TCPIP_00144 */
        /* The TcpIp module shall indicate events related to sockets to
        the upper layer module by using the Up_TcpIpEvent API and the following events:
        TCPIP_TCP_RESET, TCPIP_TCP_CLOSED, TCPIP_TCP_FIN_RECEIVED and
        TCPIP_UDP_CLOSED. */
        if(TcpIp_Config.Config.SocketOwnerConfig.SocketOwnerList[TcpIp_SocketAdmin[socketId].socketOwnerId].SocketOwnerTcpIpEventFncPtr != NULL){
            TcpIp_Config.Config.SocketOwnerConfig.SocketOwnerList[TcpIp_SocketAdmin[socketId].socketOwnerId].SocketOwnerTcpIpEventFncPtr(socketId, event);
        }
        TcpIp_FreeUpSocket(socketId);
    }
#endif
}

#define TEMP_ETHIF_FIX 1
#if defined(TEMP_ETHIF_FIX)
static uint16 TcpIp_SendIpMessage(int SocketHandle, const uint8* DataPtr,
        const TcpIp_SockAddrType* RemoteAddrPtr, uint16 TotalLength) {

    uint16 bytesSent = 0;
    char toAddrString[INET_ADDRSTRLEN];
    int sendRet = -1;
    int saved_errno;
#ifndef _WIN32
    if (RemoteAddrPtr == NULL) {
        /* TCP */
        sendRet = send(SocketHandle, (uint8*) DataPtr, TotalLength, 0);
        if (sendRet == -1) {
            /* Saving errno before making any more POSIX system calls. */
            saved_errno = errno;
            /* Make sure bytesSent is set to zero as done in OSEK TcpIp.c */
            bytesSent = 0;
        } else {
            /* Safe conversion of positive value from signed to unsigned. */
            bytesSent = sendRet;
        }
        /* Logging */
        if (sendRet == -1) {
            logger_mod((LOGGER_MOD_TCP | LOGGER_SUB_TCP_SEND), LOG_ERR,
                    "TcpIp_SendIpMessage: Error on send call, SocketHandle %d, errno %d",
                    SocketHandle, saved_errno);
        } else {
            logger_mod((LOGGER_MOD_TCP | LOGGER_SUB_TCP_SEND), LOG_INFO,
                    "TcpIp_SendIpMessage: SocketHandle %d, BytesSent [%d] TotalLength [%d]",
                    SocketHandle, bytesSent, TotalLength);
        }
    } else {
        /* UDP */
        struct sockaddr_in toAddr;
        //uint32 toAddrLen = sizeof(toAddr);
        toAddr.sin_family = RemoteAddrPtr->domain;
        //toAddr.sin_len = sizeof(toAddr);
        //uint8 length = (RemoteAddrPtr->domain == TCPIP_AF_INET6 ? TCPIP_SA_DATA_SIZE_IPV6:TCPIP_SA_DATA_SIZE_IPV4);
        TcpIp_IpAddr8to32(RemoteAddrPtr->addr, &toAddr.sin_addr.s_addr);
        //inet_aton(RemoteAddrPtr->addr, &toAddr.sin_addr);

        toAddr.sin_port = htons(RemoteAddrPtr->port);
        sendRet = sendto(SocketHandle, (uint8*) DataPtr, TotalLength, 0,
                (struct sockaddr *) &toAddr, sizeof(toAddr));
        if (sendRet == -1) {
            /* Saving errno before making any more POSIX system calls. */
            saved_errno = errno;
            /* Make sure bytesSent is set to zero as done in OSEK TcpIp.c */
            bytesSent = 0;
        } else {
            /* Safe conversion of positive value from signed to unsigned. */
            bytesSent = sendRet;
        }

        /* Any way, initialize toAddrString. */
        TcpIp_IpAddrToChar((char *) &toAddrString, toAddr.sin_addr.s_addr);
        /* Logging */
        if (sendRet == -1) {
            logger_mod((LOGGER_MOD_TCP | LOGGER_SUB_TCP_SEND), LOG_ERR,
                    "TcpIp_SendIpMessage: Error on sendto call, SocketHandle %d, To %s:%d, errno %d",
                    SocketHandle, toAddrString, RemoteAddrPtr->port,
                    saved_errno);
        } else {
            logger_mod((LOGGER_MOD_TCP | LOGGER_SUB_TCP_SEND), LOG_INFO,
                    "TcpIp_SendIpMessage: SocketHandle %d, To %s:%d, BytesSent [%d] TotalLength [%d]",
                    SocketHandle, toAddrString, RemoteAddrPtr->port, bytesSent,
                    TotalLength);
        }
    }
#endif
    return bytesSent;
}

static boolean TcpIp_BufferGet(uint32 size, uint8** buffPtr)
{
    boolean res;

    *buffPtr = malloc(size);
    res = (*buffPtr != NULL) ? TRUE : FALSE;

    return res;
}
static void TcpIp_BufferFree(uint8* buffPtr)
{
    free(buffPtr);
}

BufReq_ReturnType EthIf_ProvideTxBuffer( uint8  CtrlIdx, Eth_FrameType  FrameType, uint8  Priority,
            Eth_BufIdxType*  BufIdxPtr, uint8**  BufPtr, uint16*  LenBytePtr){
    BufReq_ReturnType res = BUFREQ_NOT_OK;

    if(TcpIp_BufferGet(*LenBytePtr, BufPtr)){
        res = BUFREQ_OK;
    }
    return res;
}

Std_ReturnType  EthIf_Transmit(uint8  CtrlIdx,Eth_BufIdxType  BufIdx,Eth_FrameType  FrameType,
        boolean  TxConfirmation,uint16  LenByte,const  uint8*  PhysAddrPtr)
{
    return E_OK;
}

void EthIf_GetPhysAddr(uint8 CtrlIdx,uint8*  PhysAddrPtr){
    PhysAddrPtr[0] = 0x0;
    PhysAddrPtr[1] = 0x0;
    PhysAddrPtr[2] = 0x0;
    PhysAddrPtr[3] = 0x0;
    PhysAddrPtr[4] = 0x0;
    PhysAddrPtr[5] = 0x2;
}

#endif /* Temporary EthIf replacement */

Std_ReturnType TcpIp_GetSocket(uint8 SocketOwnerId,TcpIp_DomainType Domain, TcpIp_ProtocolType Protocol, TcpIp_SocketIdType* SocketIdPtr)
{
    int sockFd;
    int sockType;
    Std_ReturnType result = E_OK;
#ifndef _WIN32
    /* @req 4.2.2/SWS_TCPIP_00128 */
    VALIDATE( (Domain == TCPIP_AF_INET), TCPIP_GETSOCKET_SERVICE_ID, TCPIP_E_AFNOSUPPORT)
    // No specified autosar error for protocol check, use TCPIP_E_NOPROTOOPT as it seems most logical
    VALIDATE( (TCPIP_IPPROTO_UDP == Protocol) || (TCPIP_IPPROTO_TCP == Protocol), TCPIP_GETSOCKET_SERVICE_ID, TCPIP_E_NOPROTOOPT)

    if (TCPIP_IPPROTO_TCP == Protocol) {
        sockType = SOCK_STREAM;
    } else {
        sockType = SOCK_DGRAM;
    }
#if defined(ANDROID_NDK)
    sockFd = socket(Domain, (sockType), 0);

    int sockFlags = fcntl(sockFd, F_GETFL, 0);
    fcntl(sockFd, F_SETFL, sockFlags | O_NONBLOCK);
#else
    sockFd = socket(Domain, (sockType|SOCK_NONBLOCK), 0); // Set socket to non block mode
#endif
    if(sockFd < 0){
        result = E_NOT_OK;
    }else{
        TcpIp_SocketIdType socketId;
        if(E_OK == TcpIp_GetFirstFreeSocket(&socketId)){
            /* Socket assigned, get first free socket index */
            TcpIp_SocketAdmin[socketId].socketHandle = sockFd;
            TcpIp_SocketAdmin[socketId].socketOwnerId = SocketOwnerId;
            TcpIp_SocketAdmin[socketId].socketState = TCPIP_SOCKET_BIND;
            TcpIp_SocketAdmin[socketId].socketProtocolIsTcp = TCPIP_IPPROTO_TCP == Protocol ? TRUE:FALSE;
            *SocketIdPtr = socketId;


        }else{
            result = E_NOT_OK;
            close(sockFd);
        }
     }
#endif
    return result;
}

/**
 * @brief This service initializes the TCP/IP Stack
 * @param SocketId - Socket handle identifying the local socket resource ConfigPtr - Configuration data ptr
 * @return void
 */
/** @req 4.2.2/SWS_TCPIP_00002 */
    void TcpIp_Init(const TcpIp_ConfigType* ConfigPtr){
        (void)ConfigPtr; /* not used */

        for(int i=0; i < TCPIP_NOF_CONTROLLERS; i++){
            TcpIp_CtrlAdmin[i].tcpipEthIfCtrlState = TCPIP_STATE_OFFLINE;
            TcpIp_CtrlAdmin[i].tcpipEthIfRequestedCtrlState = TCPIP_STATE_OFFLINE;
            TcpIp_CtrlAdmin[i].ptrLocalUnicastAddr = NULL;
        }

        for(int i=0; i < TCPIP_MAX_NOF_SOCKETS; i++){
            TcpIp_SocketAdmin[i].socketState = TCPIP_SOCKET_INIT;
            TcpIp_SocketAdmin[i].socketHandle = -1;
            TcpIp_SocketAdmin[i].ptrLocalAddr = NULL;
            TcpIp_SocketAdmin[i].socketProtocolIsTcp = FALSE;
        }

        tcpip_initialized = TRUE;

        //LwIP_Init();
    }

/**
 * @brief By this API service the TCP/IP stack is requested to close the socket and release
 *        all related resources.
 * @param SocketId - Socket handle identifying the local socket resource
 * @param Abort - TRUE: connection will immediately be terminated by sending a
 *                      RST-Segment and releasing all related resources.
 *                FALSE: connection will be terminated after performing a regular
 *                       connection termination handshake and releasing all related resources.
 * @return result of the function.
 */
/** @req 4.2.2/SWS_TCPIP_00017 */
Std_ReturnType TcpIp_Close(TcpIp_SocketIdType SocketId, boolean Abort){
    Std_ReturnType result = E_OK;

    /** @req 4.2.2/SWS_TCPIP_00109 *//* Using sockets can not handle abort */
    /** @req 4.2.2/SWS_TCPIP_00110 */
    /* Ignore abort parameter as close always perform a FIN/ACK handshake. Using
     * shutdown does not solve this either */
    close(TcpIp_SocketAdmin[SocketId].socketHandle);
    TcpIp_FreeUpSocket(SocketId);

    return result;
}

/**
 * @brief By this API service the TCP/IP stack is requested to bind a UDP or TCP socket to
 *        a local resource.
 * @param SocketId - Socket handle identifying the local socket resource
 * @param LocalAddrId  - IP address identifier representing the local IP address and EthIf
 *                       controller to bind the socket to.
 * @param PortPtr - Pointer to memory where the local port to which the socket shall
 *                  be bound is specified. In case the parameter is specified as
 *                  TCPIP_PORT_ANY, the TCP/IP stack shall choose the local port
 *                  automatically from the range 49152 to 65535 and shall update the
 *                  parameter to the chosen value.
 * @return result of the function.
 */
/** @req 4.2.2/SWS_TCPIP_00015 */
Std_ReturnType TcpIp_Bind(TcpIp_SocketIdType SocketId, TcpIp_LocalAddrIdType LocalAddrId, uint16* PortPtr){
	Std_ReturnType result = E_OK;
#ifndef _WIN32
	struct sockaddr_in sLocalAddr;
	const TcpIp_LocalAddrType *ptrLocalAddr = &TcpIp_Config.Config.LocalAddrList[getLocalAddrListConfigIdx(LocalAddrId)];
	char strAddr[INET_ADDRSTRLEN];

	/** @req 4.2.2/SWS_TCPIP_00111 */
	/** @req 4.2.2/SWS_TCPIP_00130 */
	/** @req 4.2.2/SWS_TCPIP_00015 */
	/** !req 4.2.2/SWS_TCPIP_00146 */
	/** !req 4.2.2/SWS_TCPIP_00147 */

	memset((char *)&sLocalAddr, 0, sizeof(sLocalAddr));

	/*Source*/
	sLocalAddr.sin_family = ptrLocalAddr->Domain;
	//sLocalAddr.sin_len = sizeof(sLocalAddr);
	sLocalAddr.sin_port = htons(*PortPtr);

	TcpIp_IpAddrToChar((char *)&strAddr, ptrLocalAddr->StaticIpAddrConfig->StaticIpAddress);

	if (gnulinuxTcpBindAny)
	{
		logger(LOG_INFO, "TcpIp_Bind binding SocketId %d IsTcp %d to INADDR_ANY NOT %s port %d", SocketId,
				TcpIp_SocketAdmin[SocketId].socketProtocolIsTcp, (char *) strAddr, *PortPtr);
		sLocalAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	}
	else
	{
		logger_mod((LOGGER_MOD_TCP|LOGGER_SUB_TCP_BIND), LOG_INFO, "TcpIp_Bind binding SocketId %d IsTcp %d to %s port %d", SocketId, 
				TcpIp_SocketAdmin[SocketId].socketProtocolIsTcp, (char *) strAddr, *PortPtr);
		sLocalAddr.sin_addr.s_addr = ptrLocalAddr->StaticIpAddrConfig->StaticIpAddress;
	}


	if(bind(TcpIp_SocketAdmin[SocketId].socketHandle, (struct sockaddr *)&sLocalAddr, sizeof(sLocalAddr)) < 0) {
		result = E_NOT_OK;
		logger(LOG_ERR, "TcpIp_Bind bind failure [%s] ", strerror(errno));
	}else{
		if(ptrLocalAddr->AddressType == TCPIP_MULTICAST){
			struct ip_mreq mreq;
			mreq.imr_multiaddr.s_addr = sLocalAddr.sin_addr.s_addr;
			/** @req 4.2.2/SWS_TCPIP_00178 */
			if( TcpIp_CtrlAdmin[ptrLocalAddr->TcpIpCtrlRef].ptrLocalUnicastAddr != NULL &&
				TcpIp_CtrlAdmin[ptrLocalAddr->TcpIpCtrlRef].ptrLocalUnicastAddr->StaticIpAddrConfig->ArcValid){
					mreq.imr_interface.s_addr = TcpIp_CtrlAdmin[ptrLocalAddr->TcpIpCtrlRef].ptrLocalUnicastAddr->StaticIpAddrConfig->StaticIpAddress;
			}
			else{
				mreq.imr_interface.s_addr = htonl(INADDR_ANY);
			}

			if (setsockopt(TcpIp_SocketAdmin[SocketId].socketHandle, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0)
			{
				result = E_NOT_OK;
				/* IMPROVEMENT, remove this when know working */
			}
		}

		TcpIp_SocketAdmin[SocketId].ptrLocalAddr = ptrLocalAddr;
		if(TcpIp_SocketAdmin[SocketId].socketProtocolIsTcp){
			TcpIp_SocketAdmin[SocketId].socketState = TCPIP_SOCKET_TCP_CONNECT_LISTEN;
		}else{
			TcpIp_SocketAdmin[SocketId].socketState = TCPIP_SOCKET_UDP_READY;
		}
	}
#endif
	return result;
}

/**
 * @brief By this API service the TCP/IP stack is requested to establish a TCP connection to
 *        the configured peer.
 * @param SocketId - Socket handle identifying the local socket resource
 * @param RemoteAddrPtr - IP address and port of the remote host to connect to.
 * @return result of the function.
 */
/** @req 4.2.2/SWS_TCPIP_00022 */
Std_ReturnType TcpIp_TcpConnect(TcpIp_SocketIdType SocketId, const TcpIp_SockAddrType* RemoteAddrPtr){
    Std_ReturnType result = E_NOT_OK;
#ifndef _WIN32
    struct sockaddr_in socketAddress;
    //uint16 length;
    int conRes;

    /** @req 4.2.2/SWS_TCPIP_00129 */
    VALIDATE( (NULL != RemoteAddrPtr), TCPIP_TCPCONNECT_SERVICE_ID, TCPIP_E_PARAM_POINTER)

    socketAddress.sin_family = RemoteAddrPtr->domain;
    socketAddress.sin_port = htons(RemoteAddrPtr->port);

    //length = (RemoteAddrPtr->domain == TCPIP_AF_INET6 ? TCPIP_SA_DATA_SIZE_IPV6:TCPIP_SA_DATA_SIZE_IPV4);
    //TcpIp_IpAddr8to32(RemoteAddrPtr->addr, &socketAddress.sin_addr.s_addr);
    inet_aton(RemoteAddrPtr->addr, &socketAddress.sin_addr);

    /** @req 4.2.2/SWS_TCPIP_00112 */
    conRes = connect(TcpIp_SocketAdmin[SocketId].socketHandle, (struct sockaddr *)&socketAddress, sizeof(struct sockaddr));
    if(0 == conRes){
        result = E_OK;
        TcpIp_SocketAdmin[SocketId].socketState = TCPIP_SOCKET_TCP_READY;
        /* Call upper layer */
        if(TcpIp_Config.Config.SocketOwnerConfig.SocketOwnerList[TcpIp_SocketAdmin[SocketId].socketOwnerId].SocketOwnerTcpConnectedFncPtr != NULL){
            TcpIp_Config.Config.SocketOwnerConfig.SocketOwnerList[TcpIp_SocketAdmin[SocketId].socketOwnerId].SocketOwnerTcpConnectedFncPtr(SocketId);
        }

    }
#endif
    return result;
}

/**
 * @brief By this API service the TCP/IP stack is requested to listen on the
 *        TCP socket specified by the socket identifier.
 * @param SocketId - Socket handle identifying the local socket resource
 * @param MaxChannels - Maximum number of new parallel connections established on
 *        this listen connection.
 * @return result of the function.
 */
/** @req 4.2.2/SWS_TCPIP_00023 */
Std_ReturnType TcpIp_TcpListen(TcpIp_SocketIdType SocketId, uint16 MaxChannels){
    Std_ReturnType result = E_OK;

    /** @req 4.2.2/SWS_TCPIP_00113 */
    /** @req 4.2.2/SWS_TCPIP_00114 */
#ifndef _WIN32
    listen(TcpIp_SocketAdmin[SocketId].socketHandle, MaxChannels);
#endif
    TcpIp_SocketAdmin[SocketId].socketState = TCPIP_SOCKET_TCP_LISTENING;
    return result;
}

/**
 * @brief By this API service the reception of socket data is confirmed to the TCP/IP stack.
 * @param SocketId - Socket handle identifying the local socket resource
 * @param Length - Number of bytes finally consumed by the upper layer.
 * @return result of the function.
 */
/** !req 4.2.2/SWS_TCPIP_00024 */
Std_ReturnType TcpIp_TcpReceived(TcpIp_SocketIdType SocketId, uint32 Length){
    Std_ReturnType result = E_OK;

    // Dont really understand the use...
    return result;
}

/**
 * @brief By this API service the TCP/IP stack is requested to change the TcpIp state of the
 *        communication network identified by EthIf controller index.
 * @param CtrlIdx - EthIf controller index to identify the communication network
 *                  where the TcpIp state is requested.
 * @param State - Requested TcpIp state.
 * @return result of the function.
 */
/** @req 4.2.2/SWS_TCPIP_00070 */
Std_ReturnType TcpIp_RequestComMode( uint8 CtrlIdx, TcpIp_StateType State ){
    Std_ReturnType result = E_NOT_OK;

    VALIDATE(CtrlIdx < TCPIP_NOF_CONTROLLERS, TCPIP_REQUESTCOMMODE_SERVICE_ID, TCPIP_E_INV_ARG)

    /** @req 4.2.2/SWS_TCPIP_00136 */
    if(State != TcpIp_CtrlAdmin[CtrlIdx].tcpipEthIfCtrlState){
        switch (State) {
            case TCPIP_STATE_OFFLINE:
                /** @req 4.2.2/SWS_TCPIP_00072 */
                //initiate deactivation of the TcpIp communication on the related EthIf controller
                //(e.g. close all sockets using the specified EthIf controller
                result = E_OK;
                break;
            case TCPIP_STATE_ONLINE:
                /** @req 4.2.2/SWS_TCPIP_00071 */
                //initiate activation of the TcpIp communication on the related EthIf controller (e.g.
                //start IP-Address assignment according to the configured IP address assignment
                //policy for the EthIf controller)
                result = E_OK;
                break;
            case TCPIP_STATE_ONHOLD:
                /** @req 4.2.2/SWS_TCPIP_00074 */
                //set the TcpIp communication to on hold, i.e. new transmit requests shall not be
                //accepted, but sockets and assigned IP addresses shall be kept
                if(TcpIp_CtrlAdmin[CtrlIdx].tcpipEthIfCtrlState == TCPIP_STATE_ONLINE){
                    result = E_OK; /* Can only request onhold from online */
                }
                break;
            default:
                /** @req 4.2.2/SWS_TCPIP_00089 */
                TCPIP_DET_REPORTERROR(TCPIP_REQUESTCOMMODE_SERVICE_ID, TCPIP_E_INV_ARG);
                break;
        }
    }else{
        /* IMPROVEMENT shall we respond E_OK when a request for the already active state is made? */
        result = E_OK;
    }

    if(result == E_OK){
        TcpIp_CtrlAdmin[CtrlIdx].tcpipEthIfRequestedCtrlState = State;
    }

    return result;
}


Std_ReturnType TcpIp_AutomaticIpAddrAssignment(TcpIp_LocalAddrIdType LocalAddrId, TcpIp_IpAddrAssignmentType Type){
    Std_ReturnType result = E_OK;
    logger(LOG_INFO, "TcpIp_AutomaticIpAddrAssignment TcpIp_LocalAddrIdType %d", LocalAddrId);

    return result;
}


/**
 * @brief By this API service the TCP/IP stack is requested to change a parameter of a socket.
 * @param SocketId - Socket handle identifying the local socket resource
 * @param ParameterId - Identifier of the parameter to be changed
 * @param ParameterValue - Pointer to memory containing the new parameter value.
 * @return result of the function.
 */
/** @req 4.2.2/SWS_TCPIP_00016 */
Std_ReturnType TcpIp_ChangeParameter(TcpIp_SocketIdType SocketId, TcpIp_ParamIdType ParameterId, const uint8* ParameterValue){
    Std_ReturnType result = E_OK;
#ifndef _WIN32
    /** @req 4.2.2/SWS_TCPIP_00119 */
    switch (ParameterId)
    {
        case TCPIP_PARAMID_TCP_RXWND_MAX:
            break;
        case TCPIP_PARAMID_FRAMEPRIO:
            break;
        case TCPIP_PARAMID_TCP_NAGLE:
            {
                int on = 1;
                setsockopt(TcpIp_SocketAdmin[SocketId].socketHandle, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(int)); // Set socket to no delay
                //ioctlsocket(TcpIp_SocketAdmin[SocketId].socketHandle, FIONBIO, &on);
        // The POSIX way to do the above
        int flags = fcntl(TcpIp_SocketAdmin[SocketId].socketHandle, F_GETFL, 0);
        fcntl(TcpIp_SocketAdmin[SocketId].socketHandle, F_SETFL, flags | O_NONBLOCK);
            }
            break;
        case TCPIP_PARAMID_TCP_KEEPALIVE:
            break;
        case TCPIP_PARAMID_TTL:
            break;
        case TCPIP_PARAMID_TCP_KEEPALIVE_TIME:
            break;
        case TCPIP_PARAMID_TCP_KEEPALIVE_PROBES_MAX:
            break;
        case TCPIP_PARAMID_TCP_KEEPALIVE_INTERVAL:
            break;
        case TCPIP_PARAMID_VENDOR_SPECIFIC:
            break;
        default:
            break;
    }
#endif
    return result;
}


/**
 *  * @brief By this API service the local IP address assignment for the IP address specified by
 *   *        LocalAddrId shall be initiated.
 *    * @param LocalAddrId - IP address index specifying the IP address for which an
 *     *                      assignment shall be initiated.
 *      * @param Type - Type of IP address assignment which shall be initiated
 *       * @param LocalIpAddrPtr - Pointer to structure containing the IP address which shall be
 *        *                        assigned to the EthIf controller indirectly specified via LocalAddrId.
 *         * @param Netmask - Network mask of IPv4 address or address prefix of IPv6 address in CIDR Notation.
 *          * @param DefaultRouterPtr - Pointer to structure containing the IP address of the default router
 *           *                           (gateway) which shall be assigned.
 *            * @return result of the function.
 *             */
/** @req 4.2.2/SWS_TCPIP_00037 */
Std_ReturnType TcpIp_RequestIpAddrAssignment(TcpIp_LocalAddrIdType LocalAddrId, TcpIp_IpAddrAssignmentType Type,
                const TcpIp_SockAddrType* LocalIpAddrPtr, uint8 Netmask, const TcpIp_SockAddrType* DefaultRouterPtr){
    Std_ReturnType result = E_OK;

    // IMPROVEMENT SimonG
    return result;
}


/**
 *  * @brief By this API service the local IP address assignment for the IP address specified by
 *   *        LocalAddrId shall be released.
 *    * @param LocalAddrId - IP address index specifying the IP address for which an
 *     *                      assignment shall be released.
 *      * @return result of the function.
 *       */
/** !req 4.2.2/SWS_TCPIP_00078 */
Std_ReturnType TcpIp_ReleaseIpAddrAssignment(TcpIp_LocalAddrIdType LocalAddrId){
    Std_ReturnType result = E_OK;

    // IMPROVEMENT SimonG
    return result;
}




/**
 * @brief Obtains the local IP address actually used by LocalAddrId, the netmask and
default router
 * @param LocalAddrId - Local address identifier referring to the local IP address which
 *                      shall be obtained.
 * @param IpAddrPtr - Pointer to a struct where the IP address shall be stored.
 * @param NetmaskPtr - Pointer to memory where Network mask of IPv4 address or
 *                     address prefix of IPv6 address in CIDR Notation is stored
 * @param DefaultRouterPtr - Pointer to struct where the IP address of the default router
 *							(gateway) is stored (struct member "port" is not used and of
 *							arbitrary value).
 * @return result of the function.
 */
/** @req 4.2.2/SWS_TCPIP_00032 */
Std_ReturnType TcpIp_GetIpAddr(TcpIp_LocalAddrIdType LocalAddrId, TcpIp_SockAddrType* IpAddrPtr,
        uint8* NetmaskPtr, TcpIp_SockAddrType* DefaultRouterPtr){
    Std_ReturnType result = E_OK;

    const TcpIp_LocalAddrType *ptrLocalAddr = &TcpIp_Config.Config.LocalAddrList[getLocalAddrListConfigIdx(LocalAddrId)];

    /** @req 4.2.2/SWS_TCPIP_00205 */
    VALIDATE(IpAddrPtr->domain == ptrLocalAddr->Domain, TCPIP_GETIPADDR_SERVICE_ID, TCPIP_E_INV_ARG);
    /** @req 4.2.2/SWS_TCPIP_00206 */
    VALIDATE(DefaultRouterPtr->domain == IpAddrPtr->domain, TCPIP_GETIPADDR_SERVICE_ID, TCPIP_E_INV_ARG);

    TcpIp_IpAddr32To8(ptrLocalAddr->StaticIpAddrConfig->StaticIpAddress, IpAddrPtr->addr);
    TcpIp_IpAddr32To8(ptrLocalAddr->StaticIpAddrConfig->DefaultRouter, DefaultRouterPtr->addr);

    return result;
}

/**
 * @brief Obtains the physical source address used by the EthIf controller implicitly
 *        specified via LocalAddrId.
 * @param LocalAddrId - Local address identifier referring to the local IP address which
 *                      shall be obtained.
 * @param PhysAddrPtr - Pointer to the memory where the physical source address (MAC
 *                      address) in network byte order is stored
 * @return result of the function.
 */
/** @req 4.2.2/SWS_TCPIP_00033 */
Std_ReturnType TcpIp_GetPhysAddr(TcpIp_LocalAddrIdType LocalAddrId, uint8* PhysAddrPtr){
    Std_ReturnType result = E_OK;

    const TcpIp_LocalAddrType *ptrLocalAddr = &TcpIp_Config.Config.LocalAddrList[getLocalAddrListConfigIdx(LocalAddrId)];

    EthIf_GetPhysAddr(ptrLocalAddr->TcpIpCtrlRef, PhysAddrPtr);
    return result;
}


/**
 * @brief TcpIp_GetCtrlIdx returns the index of the controller related to LocalAddrId.
 * @param LocalAddrId - Local address identifier referring to the local IP address which
 *                      shall be obtained.
 * @param CtrlIdxPtr - Pointer to the memory where the index of the controller related to
 *                     LocalAddrId is stored
 * @return result of the function.
 */
/** @req 4.2.2/SWS_TCPIP_00140 */
Std_ReturnType TcpIp_GetCtrlIdx(TcpIp_LocalAddrIdType LocalAddrId, uint8* CtrlIdxPtr){
    Std_ReturnType result = E_OK;

    /** @req 4.2.2/SWS_TCPIP_00141 */
    const TcpIp_LocalAddrType *ptrLocalAddr = &TcpIp_Config.Config.LocalAddrList[getLocalAddrListConfigIdx(LocalAddrId)];

    *CtrlIdxPtr = ptrLocalAddr->TcpIpCtrlRef;
    return result;
}


/**
 * @brief This service transmits data via UDP to a remote node. The transmission of the
 * data is immediately performed with this function call by forwarding it to EthIf.
 * @param SocketId - Socket handle identifying the local socket resource
 * @param DataPtr - Pointer to a linear buffer of TotalLength bytes containing the
 *                  data to be transmitted.
 *                  In case DataPtr is a NULL_PTR, TcpIp shall retrieve data from
 *                  upper layer via callback <Up>_CopyTxData().
 * @param RemoteAddrPtr - IP address and port of the remote host to transmit to.
 * @param TotalLength - indicates the payload size of the UDP datagram.
 * @return result of the function.
 */
/** @req 4.2.2/SWS_TCPIP_00025 */
Std_ReturnType TcpIp_UdpTransmit(TcpIp_SocketIdType SocketId, const uint8* DataPtr, const TcpIp_SockAddrType* RemoteAddrPtr, uint16 TotalLength){
    Std_ReturnType result = E_OK;
    BufReq_ReturnType  bufres;
    Eth_FrameType  frameType = 0x0800;
    uint8*  bufPtr;
    Eth_BufIdxType  bufIdx;
    uint8 priority = 0;
    uint8  ctrlIdx;
    uint16 len = TotalLength;

    /* Added some protection even though not specified */
    VALIDATE( (SocketId < TCPIP_MAX_NOF_SOCKETS), TCPIP_UDPTRANSMIT_SERVICE_ID, TCPIP_E_INV_ARG)
    VALIDATE( (TcpIp_SocketAdmin[SocketId].socketHandle >= 0), TCPIP_UDPTRANSMIT_SERVICE_ID, TCPIP_E_NOTCONN)

    //ctrlIdx = TcpIp_SocketAdmin[SocketId].ptrLocalAddr->TcpIpCtrlRef;
    ctrlIdx = 0;

    /** @req 4.2.2/SWS_TCPIP_00120 */
    //EthIf_GetPhysAddr(ctrlIdx,physAddr);

    bufres = EthIf_ProvideTxBuffer( ctrlIdx, frameType, priority, &bufIdx, &bufPtr, &len);

    /** @req 4.2.2/SWS_TCPIP_00121 */
    if(bufres == BUFREQ_OK){
        if(DataPtr == NULL){
            /* Copy data callback to upper layer  */
            if(TcpIp_Config.Config.SocketOwnerConfig.SocketOwnerList[TcpIp_SocketAdmin[SocketId].socketOwnerId].SocketOwnerCopyTxDataFncPtr != NULL){
                TcpIp_Config.Config.SocketOwnerConfig.SocketOwnerList[TcpIp_SocketAdmin[SocketId].socketOwnerId].SocketOwnerCopyTxDataFncPtr(SocketId, bufPtr, len);
            }
        }else{
            memcpy(bufPtr,DataPtr,len);
        }

        /** @req 4.2.2/SWS_TCPIP_00122 */
        if(TcpIp_SocketAdmin[SocketId].socketState == TCPIP_SOCKET_BIND){
            /* IMPROVEMENT Do we need to do anything here???
             * Not been bind to an address, use local ip address and port */
        }

        uint16 bytesSent = TcpIp_SendIpMessage(TcpIp_SocketAdmin[SocketId].socketHandle, bufPtr, RemoteAddrPtr, TotalLength);
        TcpIp_BufferFree(bufPtr);// IMPROVEMENT temp, remove with correct EthIf
        /* Same result setting as in OSEK TcpIp.c */
        if (bytesSent == 0) {
            result = E_NOT_OK;
        }

        if(TcpIp_Config.Config.SocketOwnerConfig.SocketOwnerList[TcpIp_SocketAdmin[SocketId].socketOwnerId].SocketOwnerTxConfirmationFncPtr != NULL){
            TcpIp_Config.Config.SocketOwnerConfig.SocketOwnerList[TcpIp_SocketAdmin[SocketId].socketOwnerId].SocketOwnerTxConfirmationFncPtr(SocketId,bytesSent);
        }
    }else{
        result = E_NOT_OK;
    }

    return result;
}

/**
 * @brief This service requests transmission of data via TCP to a remote node. The
 * transmission of the data is decoupled.
 * @param SocketId - Socket handle identifying the local socket resource
 * @param DataPtr - Pointer to a linear buffer of TotalLength bytes containing the
 *                  data to be transmitted.
 *                  In case DataPtr is a NULL_PTR, TcpIp shall retrieve data from
 *                  upper layer via callback <Up>_CopyTxData().
 * @param AvailableLength - Available data for transmission in bytes.
 * @param ForceRetrieve - This parameter is only valid if DataPtr is a NULL_PTR.
 * Indicates how the TCP/IP stack retrieves data from upper layer if
 * DataPtr is a NULL_PTR.
 * TRUE: the whole data indicated by availableLength shall be
 * retrieved from the upper layer via one or multiple
 * <Up>_CopyTxData() calls within the context of this transmit
 * function.
 * FALSE: The TCP/IP stack may retrieve up to availableLength
 * data from the upper layer. It is allowed to retrieve less than
 * availableLength bytes. Note: Not retrieved data will be provided
 * by upper layer with the next call to TcpIp_TcpTransmit (along with
 * new data if available).
 * @return result of the function.
 */
/** !req 4.2.2/SWS_TCPIP_00050 */
Std_ReturnType TcpIp_TcpTransmit(TcpIp_SocketIdType SocketId, const uint8* DataPtr, uint32 AvailableLength,
        boolean ForceRetrieve){
    Std_ReturnType result = E_OK;
    BufReq_ReturnType  bufres;
    Eth_FrameType  frameType = 0x0800;
    uint8*  bufPtr;
    Eth_BufIdxType  bufIdx;
    uint8 priority = 0;
    uint8  ctrlIdx;
    uint16 len = AvailableLength;

    /* Added some protection even though not specified */
    VALIDATE( (SocketId < TCPIP_MAX_NOF_SOCKETS), TCPIP_TCPTRANSMIT_SERVICE_ID, TCPIP_E_INV_ARG)
    VALIDATE( (TcpIp_SocketAdmin[SocketId].socketHandle >= 0), TCPIP_TCPTRANSMIT_SERVICE_ID, TCPIP_E_NOTCONN)

    //ctrlIdx = TcpIp_SocketAdmin[SocketId].ptrLocalAddr->TcpIpCtrlRef;
    ctrlIdx = 0;

    /** @req 4.2.2/SWS_TCPIP_00120 */
    //EthIf_GetPhysAddr(ctrlIdx,physAddr);

    bufres = EthIf_ProvideTxBuffer( ctrlIdx, frameType, priority, &bufIdx, &bufPtr, &len);

    /** @req 4.2.2/SWS_TCPIP_00121 */
    if(bufres == BUFREQ_OK){
        if(DataPtr == NULL){
            /* Copy data callback to upper layer  */
            if(TcpIp_Config.Config.SocketOwnerConfig.SocketOwnerList[TcpIp_SocketAdmin[SocketId].socketOwnerId].SocketOwnerCopyTxDataFncPtr != NULL){
                TcpIp_Config.Config.SocketOwnerConfig.SocketOwnerList[TcpIp_SocketAdmin[SocketId].socketOwnerId].SocketOwnerCopyTxDataFncPtr(SocketId, bufPtr, len);
            }
        }else{
            memcpy(bufPtr,DataPtr,len);
        }

        uint16 bytesSent = TcpIp_SendIpMessage(TcpIp_SocketAdmin[SocketId].socketHandle, bufPtr, NULL, AvailableLength);
        TcpIp_BufferFree(bufPtr);// IMPROVEMENT temp, remove with correct EthIf
        /* Same result setting as in OSEK TcpIp.c */
        if (bytesSent == 0) {
            result = E_NOT_OK;
        }

        if(TcpIp_Config.Config.SocketOwnerConfig.SocketOwnerList[TcpIp_SocketAdmin[SocketId].socketOwnerId].SocketOwnerTxConfirmationFncPtr != NULL){
            TcpIp_Config.Config.SocketOwnerConfig.SocketOwnerList[TcpIp_SocketAdmin[SocketId].socketOwnerId].SocketOwnerTxConfirmationFncPtr(SocketId,bytesSent);
        }
    }else{
        result = E_NOT_OK;
    }

    return result;
}

/**
 * @brief By this API service the TCP/IP stack gets an indication and the data of a received frame.
 * @param CtrlIdx - Index of the EthIf controller.
 * @param FrameType - Frame type of received Ethernet frame
 * @param IsBroadcast - Parameter to indicate a broadcast frame
 * @param PhysAddrPtr - Pointer to Physical source address (MAC address in network byte
 *                      order) of received Ethernet frame
 * @param DataPtr - Pointer to payload of the received Ethernet frame (i.e. Ethernet
 *                  header is not provided).
 * @param LenByte - Length of received data.
 * @return void
 */
/** !req 4.2.2/SWS_TCPIP_00029 */
void TcpIp_RxIndication(uint8 CtrlIdx, Eth_FrameType FrameType, boolean IsBroadcast, const uint8* PhysAddrPtr,
        uint8* DataPtr, uint16 LenByte){

}

static void TcpIp_NotifyLocalIpAddrAssignmentChg(uint8 CtrlIdx, TcpIp_IpAddrStateType ipAddrState)
{
    /* Call upper layer */
    for(int LocalAddrId=0; LocalAddrId < TCPIP_NOF_LOCALADDR; LocalAddrId++){
        const TcpIp_LocalAddrType *ptrLocalAddr = &TcpIp_Config.Config.LocalAddrList[getLocalAddrListConfigIdx(LocalAddrId)];
        if(ptrLocalAddr->TcpIpCtrlRef == CtrlIdx)
        {
            for(int Idx=0; Idx < TCPIP_NOF_SOCKETOWNERS; Idx++){
                if(TcpIp_Config.Config.SocketOwnerConfig.SocketOwnerList[Idx].SocketOwnerLocalIpAddrAssignmentChgFncPtr != NULL){
                    TcpIp_Config.Config.SocketOwnerConfig.SocketOwnerList[Idx].SocketOwnerLocalIpAddrAssignmentChgFncPtr(ptrLocalAddr->AddressId, ipAddrState);
                }
            }
        }
    }
}


static void TcpIp_EnableAllIpAddrAssignments(uint8 CtrlIdx){

	char addr[INET_ADDRSTRLEN];
	char route[INET_ADDRSTRLEN];

	/* We initiate all unicast first and then any multicast. We assume there is only one */
	for(int i=0; i<TCPIP_NOF_LOCALADDR;i++){
		const TcpIp_LocalAddrType *ptrLocalAddr = &TcpIp_Config.Config.LocalAddrList[i];

		if((ptrLocalAddr->TcpIpCtrlRef == CtrlIdx) && (ptrLocalAddr->AddressAssignment->AssignmentTrigger == TCPIP_AUTOMATIC) && (ptrLocalAddr->AddressType == TCPIP_UNICAST)){
			TcpIp_AutomaticIpAddrAssignment(ptrLocalAddr->AddressId, ptrLocalAddr->AddressAssignment->AssignmentMethod);

			TcpIp_IpAddrToChar((char *)&addr, ptrLocalAddr->StaticIpAddrConfig->StaticIpAddress);
			TcpIp_IpAddrToChar((char *)&route, ptrLocalAddr->StaticIpAddrConfig->DefaultRouter);

			logger(LOG_INFO, "TcpIp_EnableAllIpAddrAssignments AUTOMATIC \t\tA[%s/%d]\tR[%s]\tfor AddressId %d",
					addr, ptrLocalAddr->StaticIpAddrConfig->Netmask, route,
					ptrLocalAddr->AddressId
			      );
		}
	}
	for(int i=0; i<TCPIP_NOF_LOCALADDR;i++){
		const TcpIp_LocalAddrType *ptrLocalAddr = &TcpIp_Config.Config.LocalAddrList[i];
		if((ptrLocalAddr->TcpIpCtrlRef == CtrlIdx) && (ptrLocalAddr->AddressAssignment->AssignmentTrigger == TCPIP_AUTOMATIC) && (ptrLocalAddr->AddressType == TCPIP_MULTICAST)){
			TcpIp_AutomaticIpAddrAssignment(ptrLocalAddr->AddressId, ptrLocalAddr->AddressAssignment->AssignmentMethod);
			TcpIp_IpAddrToChar((char *)&addr, ptrLocalAddr->StaticIpAddrConfig->StaticIpAddress);
			TcpIp_IpAddrToChar((char *)&route, ptrLocalAddr->StaticIpAddrConfig->DefaultRouter);

			logger(LOG_INFO, "TcpIp_EnableAllIpAddrAssignments Multicast \t\tA[%s/%d]\tR[%s]\tfor AddressId %d",
					addr, ptrLocalAddr->StaticIpAddrConfig->Netmask, route,
					ptrLocalAddr->AddressId
			      );
		}

	}
}


static void TcpIp_HandleStateOffline(uint8 CtrlIdx)
{
    /* IMPROVEMENT SimonG use status of ethernet link */
    TcpIp_CtrlAdmin[CtrlIdx].tcpipEthIfRequestedCtrlState = TCPIP_STATE_ONLINE;

    logger(LOG_INFO, "TcpIp_HandleStateOffline CtrlIdx %d",
    CtrlIdx);

    if(TcpIp_CtrlAdmin[CtrlIdx].tcpipEthIfRequestedCtrlState == TCPIP_STATE_ONLINE){
        /** @req 4.2.2/SWS_TCPIP_00075 */
        /* If TCPIP_STATE_ONLINE is requested for an EthIf
        controller and the current state is TCPIP_STATE_OFFLINE for that EthIf controller,
        the TcpIp module shall
        (a) enable all IP address assignments according to the configured assignment
        methods (TcpIpAssignmentMethod) and triggers (TcpIpAssignmentTrigger) for
        that EthIf controller. (Note: If the assignment trigger is configured to
        TCPIP_MANUAL no assignment is actually performed but initiation by the upper
        layer enabled) and */
        TcpIp_EnableAllIpAddrAssignments(CtrlIdx);
        /* (b) enter the state TCPIP_STATE_STARTUP for the EthIf controller.⌋ */
        TcpIp_SwitchEthIfCtrlState(CtrlIdx, TCPIP_STATE_STARTUP);
    }
    else if(TcpIp_CtrlAdmin[CtrlIdx].tcpipEthIfRequestedCtrlState == TCPIP_STATE_ONHOLD){
        /* Invalid statechange, report error and reset request */
        TCPIP_DET_REPORTERROR(TCPIP_MAINFUNCTION_SERVICE_ID, TCPIP_E_ARC_ILLEGAL_STATE);
        TcpIp_CtrlAdmin[CtrlIdx].tcpipEthIfRequestedCtrlState = TCPIP_STATE_OFFLINE;
    }
}

static void TcpIp_HandleStateOnline(uint8 CtrlIdx)
{
    if(TcpIp_CtrlAdmin[CtrlIdx].tcpipEthIfRequestedCtrlState == TCPIP_STATE_OFFLINE){
        /** @req 4.2.2/SWS_TCPIP_00077 */
        /* If TCPIP_STATE_OFFLINE is requested or all assigned IP
        address have been released for an EthIf controller and the current state is
        TCPIP_STATE_ONLINE or TCPIP_STATE_ONHOLD for that EthIf controller, the
        TcpIp module shall */

        /* (a) call Up_LocalIpAddrAssignmentChg() with State
        TCPIP_IPADDR_STATE_UNASSIGNED for all assigned IP addresses of the
        related EthIf controller, */
        TcpIp_NotifyLocalIpAddrAssignmentChg(CtrlIdx, TCPIP_IPADDR_STATE_UNASSIGNED);

        /* (b) deactivate the communication within the TcpIp module for the related EthIf
        controller, */

        /* (c) release related resources, i.e. any socket using the EthIf controller shall be
        closed and thereafter any IP address assigned to the EthIf controller shall be
        unassigned,*/
        for(int SocketId=0; SocketId < TCPIP_MAX_NOF_SOCKETS; SocketId++){
            if((TcpIp_SocketAdmin[SocketId].ptrLocalAddr != NULL) && (TcpIp_SocketAdmin[SocketId].ptrLocalAddr->TcpIpCtrlRef == CtrlIdx))
            {
                TcpIp_Close(SocketId,FALSE);
            }
        }
        /* (d) in case the no EthIf controller is assigned any more, all unbound sockets shall
        be released as well */
        boolean otherCtrls = FALSE;
        for(int SocketId=0; SocketId < TCPIP_MAX_NOF_SOCKETS; SocketId++){
           if(TcpIp_SocketAdmin[SocketId].ptrLocalAddr != NULL){
               /* There are sockets assigned to other ctrl's */
               otherCtrls = TRUE;
               break;
           }
        }
        if(otherCtrls == FALSE){
            /* There are no other assigned controllers, release all unbound sockets */
            for(int SocketId=0; SocketId < TCPIP_MAX_NOF_SOCKETS; SocketId++){
                if(TcpIp_SocketAdmin[SocketId].socketHandle >= 0){
                    TcpIp_Close(SocketId,FALSE);
                }
            }
        }
        /* (e) enter the state TCPIP_STATE_SHUTDOWN for the EthIf controller. */
        TcpIp_SwitchEthIfCtrlState(CtrlIdx, TCPIP_STATE_SHUTDOWN);
    }
    else if(TcpIp_CtrlAdmin[CtrlIdx].tcpipEthIfRequestedCtrlState == TCPIP_STATE_ONHOLD){
        /** @req 4.2.2/SWS_TCPIP_00076 */
        /* If TCPIP_STATE_ONHOLD is requested for an EthIf
        controller and the current state is TCPIP_STATE_ONLINE for that EthIf controller,
        the TcpIp module shall
        (a) notify the upper layer via Up_LocalIpAddrAssignmentChg() with State
        TCPIP_IPADDR_STATE_ONHOLD for all assigned IP addresses of the
        related EthIf controller, and */
        TcpIp_NotifyLocalIpAddrAssignmentChg(CtrlIdx, TCPIP_IPADDR_STATE_ONHOLD);
        /* (b) deactivate the communication within the TcpIp module for the related EthIf
        controller, and */
    // IMPROVEMENT SimonG
        //netif_set_down(&TcpIp_CtrlAdmin[CtrlIdx].tcpipNetif);

        /* (c) enter the state TCPIP_STATE_ONHOLD for the EthIf controller.*/
        TcpIp_SwitchEthIfCtrlState(CtrlIdx, TCPIP_STATE_ONHOLD);
    }
}

static void TcpIp_HandleStateOnhold(uint8 CtrlIdx)
{

    if(TcpIp_CtrlAdmin[CtrlIdx].tcpipEthIfRequestedCtrlState == TCPIP_STATE_OFFLINE){
        /** @req 4.2.2/SWS_TCPIP_00077 */
        TcpIp_SwitchEthIfCtrlState(CtrlIdx, TCPIP_STATE_SHUTDOWN);
    }
    else if(TcpIp_CtrlAdmin[CtrlIdx].tcpipEthIfRequestedCtrlState == TCPIP_STATE_ONLINE){
        /** @req 4.2.2/SWS_TCPIP_00086 */
       /* If TCPIP_STATE_ONLINE is requested for an EthIf
        controller and the current state is TCPIP_STATE_ONHOLD for that EthIf controller,
        the TcpIp module shall
        (a) reactivate the communication within the TcpIp module for the related EthIf
        controller, */
    // IMPROVEMENT SimonG
        //netif_set_up(&TcpIp_CtrlAdmin[CtrlIdx].tcpipNetif);

        /* (b) call Up_LocalIpAddrAssignmentChg() with State
        TCPIP_IPADDR_STATE_ASSIGNED for all assigned IP addresses of the
        related EthIf controller, and */
        TcpIp_NotifyLocalIpAddrAssignmentChg(CtrlIdx, TCPIP_IPADDR_STATE_ASSIGNED);

        /* (c) enter the state TCPIP_STATE_ONLINE for the EthIf controller. */
        TcpIp_SwitchEthIfCtrlState(CtrlIdx, TCPIP_STATE_ONLINE);
    }
}

static void TcpIp_HandleStateStartup(uint8 CtrlIdx)
{
    /** @req 4.2.2/SWS_TCPIP_00088 */
    if(TcpIp_CtrlAdmin[CtrlIdx].tcpipEthIfRequestedCtrlState == TCPIP_STATE_OFFLINE){
        /* If TCPIP_STATE_OFFLINE is requested for an EthIf
        controller and the current state is TCPIP_STATE_STARTUP for that EthIf controller,
        the TcpIp module shall
        (a) abort all ongoing IP address assignment actions appropriate and */
#if LWIP_AUTOIP
        autoip_stop(&TcpIp_CtrlAdmin[CtrlIdx].tcpipNetif);
#endif
#if LWIP_DHCP
        dhcp_stop(&TcpIp_CtrlAdmin[CtrlIdx].tcpipNetif);
#endif
    // IMPROVEMENT SimonG
        //netif_set_down(&TcpIp_CtrlAdmin[CtrlIdx].tcpipNetif);
    logger (LOG_INFO, "TcpIp_HandleStateStartup Aborting TCPIP_STATE_STARTUP going down.");

        /* (b) enter the state TCPIP_STATE_OFFLINE for the EthIf controller. */
        TcpIp_SwitchEthIfCtrlState(CtrlIdx, TCPIP_STATE_OFFLINE);
    }
    else if(TcpIp_CtrlAdmin[CtrlIdx].tcpipEthIfRequestedCtrlState == TCPIP_STATE_ONLINE){

        /** @req 4.2.2/SWS_TCPIP_00085 */
        /* If at least one IP address has been successfully assigned to
        an EthIf controller and the current state is TCPIP_STATE_STARTUP for that EthIf
        controller, the TcpIp module shall enter the state TCPIP_STATE_ONLINE for the
        EthIf controller.*/
    // IMPROVEMENT SimonG
    logger (LOG_INFO, "TcpIp_HandleStateStartup Going on-line");
        //if(netif_is_link_up(&TcpIp_CtrlAdmin[CtrlIdx].tcpipNetif)){
            /* (b) enter the state TCPIP_STATE_OFFLINE for the EthIf controller. */
            TcpIp_SwitchEthIfCtrlState(CtrlIdx, TCPIP_STATE_ONLINE);
            TcpIp_NotifyLocalIpAddrAssignmentChg(CtrlIdx, TCPIP_IPADDR_STATE_ASSIGNED);
        //}
    }
}

static void TcpIp_HandleStateShutdown(uint8 CtrlIdx)
{
    /** @req 4.2.2/SWS_TCPIP_00087 */
    /* If the current state of an EthIf controller is
    TCPIP_STATE_SHUTDOWN and all related resources have been released, the
    TcpIp module shall enter the state TCPIP_STATE_OFFLINE for the EthIf controller.*/
    // IMPROVEMENT how to find out when done
    TcpIp_SwitchEthIfCtrlState(CtrlIdx, TCPIP_STATE_OFFLINE);
}

static void TcpIp_HandleSocketStateListening(TcpIp_SocketIdType SocketId)
{
#ifndef _WIN32
    int clientFd;
    struct sockaddr_in clientAddr;
    int addrlen = sizeof(clientAddr);

    clientFd = accept(TcpIp_SocketAdmin[SocketId].socketHandle, (struct sockaddr *)&clientAddr, (uint32 *)&addrlen);

    if( clientFd != (-1))
    {
        Std_ReturnType accepted = E_NOT_OK;
        /** @req 4.2.2/SWS_TCPIP_00114 */
        /* Derive a separate socket */
        TcpIp_SocketIdType socketIdConn;
        if(E_OK == TcpIp_GetFirstFreeSocket(&socketIdConn)){
            TcpIp_SocketAdmin[socketIdConn].socketHandle = clientFd;
            TcpIp_SocketAdmin[socketIdConn].remoteAddr.domain = clientAddr.sin_family;
            TcpIp_SocketAdmin[socketIdConn].remoteAddr.port = clientAddr.sin_port;
            //uint8 length = (clientAddr.sin_family == TCPIP_AF_INET6 ? TCPIP_SA_DATA_SIZE_IPV6:TCPIP_SA_DATA_SIZE_IPV4);
            TcpIp_IpAddr32To8(clientAddr.sin_addr.s_addr, TcpIp_SocketAdmin[socketIdConn].remoteAddr.addr);
            //TcpIp_SocketAdmin[socketIdConn].remoteAddr.addr = inet_ntoa(clientAddr.sin_addr.s_addr);

            TcpIp_SocketAdmin[socketIdConn].socketOwnerId = TcpIp_SocketAdmin[SocketId].socketOwnerId;

            /* Call upper layer for acceptance  */
            if(TcpIp_Config.Config.SocketOwnerConfig.SocketOwnerList[TcpIp_SocketAdmin[socketIdConn].socketOwnerId].SocketOwnerTcpAcceptedFncPtr != NULL){
                accepted = TcpIp_Config.Config.SocketOwnerConfig.SocketOwnerList[TcpIp_SocketAdmin[socketIdConn].socketOwnerId].SocketOwnerTcpAcceptedFncPtr(SocketId, socketIdConn, &TcpIp_SocketAdmin[socketIdConn].remoteAddr);
            }

            if(E_OK == accepted){
                TcpIp_SocketAdmin[socketIdConn].socketState = TCPIP_SOCKET_TCP_READY;
                TcpIp_SocketAdmin[socketIdConn].socketProtocolIsTcp = TRUE;

                //int on = 1;
                //ioctlsocket(clientFd, FIONBIO, &on);    // Set socket to non block mode
                // The POSIX way to do the above
                int flags = fcntl(clientFd, F_GETFL, 0);
                fcntl(clientFd, F_SETFL, flags | O_NONBLOCK);

            }else{
                close(clientFd);
                TcpIp_FreeUpSocket(socketIdConn);
            }
        }
        else{
            close(clientFd);
        }

    }
#endif
}
static void TcpIp_HandleSocketStateTcpReady(TcpIp_SocketIdType SocketId)
{
    int nBytes;
    uint8 *dataPtr;

    /* Note: Even it is not shown in the sequence diagram of section 9.3, TcpIp may
    decouple the data reception if required. E.g. for reassembling of incoming IP
    datagrams that are fragmented, TcpIp shall copy the received data to a TcpIp buffer
    and decouple TcpIp_RxIndication() from SoAd_RxIndication() */
    if (TcpIp_BufferGet(TCPIP_RX_BUFFER_SIZE, &dataPtr)) {
#ifndef _WIN32
		do {
			nBytes = recv(TcpIp_SocketAdmin[SocketId].socketHandle, dataPtr, TCPIP_RX_BUFFER_SIZE, 0);
			TcpIp_SocketStatusCheck(SocketId);
			if (nBytes > 0){
				/* Call upper layer */
				if(TcpIp_Config.Config.SocketOwnerConfig.SocketOwnerList[TcpIp_SocketAdmin[SocketId].socketOwnerId].SocketOwnerRxIndicationFncPtr != NULL){
					TcpIp_Config.Config.SocketOwnerConfig.SocketOwnerList[TcpIp_SocketAdmin[SocketId].socketOwnerId].SocketOwnerRxIndicationFncPtr(SocketId, &TcpIp_SocketAdmin[SocketId].remoteAddr, dataPtr, nBytes);
				}
			}
		} while (nBytes > 0);
#endif
        TcpIp_BufferFree(dataPtr);
    }else{
        /** @req 4.2.2/SWS_TCPIP_00089 */
        TCPIP_DET_REPORTERROR(TCPIP_MAINFUNCTION_SERVICE_ID, TCPIP_E_NOBUFS);
    }
}


static void TcpIp_HandleSocketStateUdpReady(TcpIp_SocketIdType SocketId)
{
#ifndef _WIN32
	char toAddrString[INET_ADDRSTRLEN];

	int nBytes;
	uint8 *dataPtr;
	struct sockaddr_in fromAddr;
	uint32 fromAddrLen = sizeof(fromAddr);

	/* Note: Even it is not shown in the sequence diagram of section 9.3, TcpIp may
	   decouple the data reception if required. E.g. for reassembling of incoming IP
	   datagrams that are fragmented, TcpIp shall copy the received data to a TcpIp buffer
	   and decouple TcpIp_RxIndication() from SoAd_RxIndication() */
	if (TcpIp_BufferGet(TCPIP_RX_BUFFER_SIZE, &dataPtr)) {
		do {
			nBytes = recvfrom(TcpIp_SocketAdmin[SocketId].socketHandle, dataPtr, TCPIP_RX_BUFFER_SIZE, 0, (struct sockaddr*)&fromAddr, &fromAddrLen);
			TcpIp_SocketStatusCheck(SocketId);
			if (nBytes > 0){
				/* Call upper layer */

				TcpIp_IpAddrToChar((char *)&toAddrString, fromAddr.sin_addr.s_addr);

				logger_mod ((LOGGER_MOD_TCP|LOGGER_SUB_TCP_RECV), LOG_INFO, "TcpIp_RecvIpMessage from %s:%d bytesRecv [%d]",
						toAddrString, ntohs(fromAddr.sin_port),
						nBytes);
				if(TcpIp_Config.Config.SocketOwnerConfig.SocketOwnerList[TcpIp_SocketAdmin[SocketId].socketOwnerId].SocketOwnerRxIndicationFncPtr != NULL){
					TcpIp_Config.Config.SocketOwnerConfig.SocketOwnerList[TcpIp_SocketAdmin[SocketId].socketOwnerId].SocketOwnerRxIndicationFncPtr(SocketId, &TcpIp_SocketAdmin[SocketId].remoteAddr, dataPtr, nBytes);
				}
			}
		} while (nBytes > 0);
		TcpIp_BufferFree(dataPtr);
	}else{
		/** @req 4.2.2/SWS_TCPIP_00089 */
		TCPIP_DET_REPORTERROR(TCPIP_MAINFUNCTION_SERVICE_ID, TCPIP_E_NOBUFS);
	}
#endif
}

/**
 * @brief TcpIp main function.
 * @param  void
 * @return void
 */
/** @req 4.2.2/SWS_TCPIP_00026 */
void TcpIp_MainFunction(void){
    for(int CtrlIdx=0; CtrlIdx < TCPIP_NOF_CONTROLLERS; CtrlIdx++)
    {
        /** @req 4.2.2/SWS_TCPIP_00026 */
        switch (TcpIp_CtrlAdmin[CtrlIdx].tcpipEthIfCtrlState) {
            case TCPIP_STATE_OFFLINE:
                TcpIp_HandleStateOffline(CtrlIdx);
                //IMPROVEMENT Simon G
                // We are always online!
                //TcpIp_CtrlAdmin[CtrlIdx].tcpipEthIfRequestedCtrlState = TCPIP_STATE_ONLINE;
        //TcpIp_CtrlAdmin[CtrlIdx].tcpipEthIfCtrlState = TCPIP_STATE_STARTUP;
                break;
            case TCPIP_STATE_ONLINE:
                TcpIp_HandleStateOnline(CtrlIdx);
                break;
            case TCPIP_STATE_ONHOLD:
                TcpIp_HandleStateOnhold(CtrlIdx);
                break;
            case TCPIP_STATE_STARTUP:
                TcpIp_HandleStateStartup(CtrlIdx);
                break;
            case TCPIP_STATE_SHUTDOWN:
                TcpIp_HandleStateShutdown(CtrlIdx);
                break;
            default:
                /* Should not happen. Let us have an DET error */
                TCPIP_DET_REPORTERROR(TCPIP_MAINFUNCTION_SERVICE_ID, TCPIP_E_ARC_ILLEGAL_STATE);
                break;
        }
    }

    for (int i = 0; i < TCPIP_MAX_NOF_SOCKETS; i++) {
        switch (TcpIp_SocketAdmin[i].socketState) {

        case TCPIP_SOCKET_TCP_LISTENING:
            TcpIp_HandleSocketStateListening(i);
            break;

        case TCPIP_SOCKET_TCP_READY:
            TcpIp_HandleSocketStateTcpReady(i);
            break;

        case TCPIP_SOCKET_UDP_READY:
            TcpIp_HandleSocketStateUdpReady(i);
            break;

        default:
            /* Do nothing */
            break;
        }
    }
}

