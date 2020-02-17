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
 /*lint -save -e9045 *//* Note: 9045 PC-lint: non-hidden definition of type -from stack suppressed */
/** @req 4.2.2/SWS_TCPIP_00052 */
/** @req 4.2.2/SWS_TCPIP_00148 */
/** @req 4.2.2/SWS_TCPIP_00053 */
/** @req 4.2.2/SWS_TCPIP_00054 */
/** @req 4.2.2/SWS_TCPIP_00095 */
/** @req 4.2.2/SWS_TCPIP_00096 */
/** @req 4.2.2/SWS_TCPIP_00102 */
/** @req 4.2.2/SWS_TCPIP_00097 */
/** @req 4.2.2/SWS_TCPIP_00098 */
/** !req 4.2.2/SWS_TCPIP_00231 */
/** @req 4.2.2/SWS_TCPIP_00055 */
/** @req 4.2.2/SWS_TCPIP_00056 */
/** !req 4.2.2/SWS_TCPIP_00091 */ /* arp TcpIpArpTableEntryTimeout */
/** @req 4.2.2/SWS_TCPIP_00092 */
/** !req 4.2.2/SWS_TCPIP_00142 */ /* Arp Table change callback */
/** !req 4.2.2/SWS_TCPIP_00093 */ /* partial - TcpIpArpNumGratuitousARPonStartup is not configured */
/** @req 4.2.2/SWS_TCPIP_00151 */
/** @req 4.2.2/SWS_TCPIP_00057 */
/** @req 4.2.2/SWS_TCPIP_00059 */
/** !req 4.2.2/SWS_TCPIP_00099 */
/** @req 4.2.2/SWS_TCPIP_00100 */
/** @req 4.2.2/SWS_TCPIP_00130 */
/** @req 4.2.2/SWS_TCPIP_00060 */
/** @req 4.2.2/SWS_TCPIP_00103 */
/** @req 4.2.2/SWS_TCPIP_00061 */
/** @req 4.2.2/SWS_TCPIP_00104 */
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
/** !req 4.2.2/SWS_TCPIP_00230 */ /* IPv6*/
/** !req 4.2.2/SWS_TCPIP_00219 */ /* Nvm store*/
/** @req 4.2.2/SWS_TCPIP_00028 */  /* optional interfaces */
/** !req 4.2.2/SWS_TCPIP_00214 */
/** !req 4.2.2/SWS_TCPIP_00216 */
/** !req 4.2.2/SWS_TCPIP_00217 */
/** !req 4.2.2/SWS_TCPIP_00211 */
/** !req 4.2.2/SWS_TCPIP_00212 */
/** !req 4.2.2/SWS_TCPIP_00213 */
/** !req 4.2.2/SWS_TCPIP_00214 */
/** !req 4.2.2/SWS_TCPIP_00143 */

#include "TcpIp.h"
#include "TcpIp_Cbk.h"
#include "Eth_GeneralTypes.h"
#include "SchM_TcpIp.h"
#include "MemMap.h"
#include "EthSM_Cbk.h"
#include "TcpIp_EthSM.h"

#include "Bsd.h"
#include <string.h>
#include "lwip/def.h"
#include "lwip/opt.h"
#include "lwip/tcp.h"
#include "lwip/udp.h"
#include "lwip/snmp.h"
#include "lwip/mem.h"
#include "lwip/memp.h"
#include "lwip/tcpip.h"
#include "lwip/netif.h"
#include "netif/etharp.h"
#include "lwip/dhcp.h"
#include "lwip/autoip.h"
#include "lwip/ethip6.h"
#include "netbios.h"
#include "Cpu.h"

#if !defined(USE_NO_ETHIF)
#include "EthIf.h"
#else
#include "ethernetif.h" /* for backward compatibility, for non-ASR drivers */
#endif
#if defined(USE_CDDETHTRCV)
#include "CDD_EthTrcv.h"
#endif

/** @req 4.2.2/SWS_TCPIP_00068 */
#if defined(USE_DEM)
#include "Dem.h"
#endif

#if  ( TCPIP_DEV_ERROR_DETECT == STD_ON )
#if defined(USE_DET)
#include "Det.h"
#endif
#define TCPIP_DET_REPORTERROR(_api,_err)   (void)Det_ReportError(TCPIP_MODULE_ID, 0, (_api), (_err))
#else
#define TCPIP_DET_REPORTERROR(_api,_err)
#endif
/*lint -emacro(904,VALIDATE,VALIDATE_RV) 904 PC-Lint exception to MISRA 14.7 (validate DET macros)*/
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

#define  TCPIP_MTU_SIZE                          1500u
#define  TCPIP_ETH_HEADER_SIZE                   14u
#define  TCPIP_ETH_MAC_ADDR_SIZE                 6u
#define  TCPIP_ETH_FRAMETYPE_SIZE                2u
/* Ethernet types used */
#define  TCPIP_FRAME_TYPE_IPV4                   0x0800
#define  TCPIP_FRAME_TYPE_ARP                    0x0806
/*
 * Local variables
 */
typedef enum {
    TCPIP_SOCKET_INIT,
    TCPIP_SOCKET_BIND,
    TCPIP_SOCKET_TCP_CONNECT_LISTEN,
    TCPIP_SOCKET_TCP_CONNECTING,
    TCPIP_SOCKET_TCP_LISTENING,
    TCPIP_SOCKET_TCP_READY,
    TCPIP_SOCKET_UDP_READY
} TcpIp_SocketStateType;

#define TCPIP_NOF_ERR_BEFORE_RESET 100

/*lint -save -e9027 -e734  */ /* 1.operands to Operators '<<''>>','&','|' are required to use */ /* 2..loss of precision higher to lower->OK
 * 3. 9045 -unhidden declarations */

typedef struct {
    TcpIp_SocketStateType               socketState;
    boolean                             socketProtocolIsTcp;
    sint32                              socketHandle;
    uint8                               socketOwnerId;
    const TcpIp_LocalAddrType           *ptrLocalAddr;
    TcpIp_SockAddrType                  remoteAddr;
    uint16                              errCnt;
#ifdef CFG_TCPIP_DEBUG
    uint16                              errCntMax;
    uint16                              nofCloseSock;
    sint32                              sockErrorsArr[TCPIP_NOF_ERR_BEFORE_RESET];
#endif
}TcpIp_SocketAdminType;


typedef struct {
    const TcpIp_LocalAddrType *ptrLocalUnicastAddr;
    TcpIp_StateType tcpipEthIfCtrlState;
    TcpIp_StateType tcpipEthIfRequestedCtrlState;
    struct netif tcpipNetif;
    boolean linkUp; /* Link status */
}TcpIp_CtrlAdminType;

static const TcpIp_ConfigType* Config;

/** @req 4.2.2/SWS_TCPIP_00083 */
static TcpIp_CtrlAdminType TcpIp_CtrlAdmin[TCPIP_NOF_CONTROLLERS];
static TcpIp_SocketAdminType TcpIp_SocketAdmin[TCPIP_MAX_NOF_SOCKETS];
/* static uint8  SocketIdInTransfer; */
boolean tcpip_initialized = FALSE;

#if !defined(USE_NO_ETHIF)
/* Define those to better describe your network interface. */
#define IFNAME0 'A'
#define IFNAME1 'R'
#define TCPIP_ETH_CTRL_INDEX 0  /* Fix this */
static struct netif *netIfPtrActive = NULL;/* Fix this */
#endif

/* Prototypes of static functions *//* Forward Function declarations */
#if LWIP_NETIF_STATUS_CALLBACK
void LwIP_StatusCallback(struct netif *netif);
#endif
#if !defined(USE_NO_ETHIF)
static err_t ethernetif_init(struct netif *netif);
static void LwIp_RxIndication( uint8 CtrlIdx, Eth_FrameType FrameType, boolean IsBroadcast, const uint8* PhysAddrPtr,
                        const Eth_DataType* DataPtr, uint16 LenByte );
static err_t LwIp_Output(struct netif *netif, struct pbuf *p);
#endif
static Std_ReturnType TcpIp_AutomaticIpAddrAssignment(TcpIp_LocalAddrIdType LocalAddrId, TcpIp_IpAddrAssignmentType Type);

#if LWIP_NETIF_STATUS_CALLBACK
/**
  * @brief  LWIP status callback after netif setup or down or link change
  * @param  netif
  * @retval None
  */
void LwIP_StatusCallback(struct netif *netif) {
    uint8 isLinkUp = netif_is_link_up(netif);

    for(uint8 CtrlIdx=0; CtrlIdx < TCPIP_NOF_CONTROLLERS; CtrlIdx++){
        if(&TcpIp_CtrlAdmin[CtrlIdx].tcpipNetif == netif){
            if(TcpIp_CtrlAdmin[CtrlIdx].ptrLocalUnicastAddr != NULL){
                TcpIp_CtrlAdmin[CtrlIdx].ptrLocalUnicastAddr->StaticIpAddrConfig->StaticIpAddress = netif->ip_addr.addr;
                TcpIp_CtrlAdmin[CtrlIdx].ptrLocalUnicastAddr->StaticIpAddrConfig->DefaultRouter = netif->gw.addr;
                TcpIp_CtrlAdmin[CtrlIdx].ptrLocalUnicastAddr->StaticIpAddrConfig->Netmask = 32 - ilog2((ntohl(netif->netmask.addr)));
                TcpIp_CtrlAdmin[CtrlIdx].ptrLocalUnicastAddr->StaticIpAddrConfig->ArcValid = TRUE;
            }
            TcpIp_CtrlAdmin[CtrlIdx].linkUp = isLinkUp;
        }
    }

}  /*lint !e818 */
#endif

#if !defined(USE_NO_ETHIF)
/**
  * @brief  LWIP Network interface
  * @param  netif
  * @retval None
  */
static inline void LwIP_EthernetifStart(struct netif *netIf){
    netIfPtrActive = netIf;
}


/**
  * @brief  HW initialisation function (low level init)
  * @param  netif
  * @retval None
  */
static void LwIp_EthernetifInit(struct netif *netif){
    uint8 macAddr[TCPIP_ETH_MAC_ADDR_SIZE] = ETH_MAC_ADDR;
    /* set MAC hardware address length */
    netif->hwaddr_len = TCPIP_ETH_MAC_ADDR_SIZE;
    netif->hwaddr[0] =  macAddr[0];
    netif->hwaddr[1] =  macAddr[1];
    netif->hwaddr[2] =  macAddr[2];
    netif->hwaddr[3] =  macAddr[3];
    netif->hwaddr[4] =  macAddr[4];
    netif->hwaddr[5] =  macAddr[5];

    /* maximum transfer unit */
    netif->mtu = TCPIP_MTU_SIZE;

    /* device capabilities */
    /* don't set NETIF_FLAG_ETHARP if this device is not an ethernet one */
    netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP | NETIF_FLAG_IGMP;
}

/**
  * @brief  Function called at set up netif and subsequently HW initialisation
  * This function should be passed as a parameter to netif_add() and hence it has file independent name
  * @param  netif
  * @retval None
  */
static err_t ethernetif_init(struct netif *netif){
    err_t ret;
    ret = (err_t)ERR_ARG;

    if(netif != NULL){
        /*
        * Initialize the snmp variables and counters inside the struct netif.
        * The last argument should be replaced with your link speed, in units
        * of bits per second.
        */
        NETIF_INIT_SNMP(netif, snmp_ifType_ethernet_csmacd, 100000000);

        netif->name[0] = IFNAME0;
        netif->name[1] = IFNAME1;
        /* We directly use etharp_output() here to save a function call.
        * You can instead declare your own function an call etharp_output()
        * from it if you have to do some checks before sending (e.g. if link
        * is available...) */
        netif->output = etharp_output;
        netif->linkoutput = LwIp_Output;
    #if LWIP_IPV6
        netif->output_ip6 = ethip6_output;
    #endif /* LWIP_IPV6 */

        /* initialize the hardware */
        LwIp_EthernetifInit(netif);

        LwIP_EthernetifStart(netif);
        ret = (err_t)ERR_OK;
    }

    return ret;
}

/**
  * @brief  Temporary Low level / OSI layer 2 control
  * @retval None
  */
static void LwIp_EthernetifPreTcpIpInit(void){
    /* ETHSM in dummy mode, control is here */
#if (ETHSM_DUMMY_MODE == STD_ON)
    if(E_OK != EthIf_SetControllerMode(TCPIP_ETH_CTRL_INDEX,ETH_MODE_ACTIVE)){
        TCPIP_DET_REPORTERROR(TCPIP_INIT_SERVICE_ID, TCPIP_E_ARC_ETHIF_INIT_STATE_FAILED);
    }
#endif

#if ( ETHIF_TRCV_SUPPORT == STD_OFF) && defined(USE_CDDETHTRCV)
    /* EthIf does not support Trcv module control, hence from here   */
    if(E_OK != EthTrcv_TransceiverInit(TCPIP_ETH_CTRL_INDEX)){
        TCPIP_DET_REPORTERROR(TCPIP_INIT_SERVICE_ID, TCPIP_E_ARC_TRCV_INIT_STATE_FAILED);
    }
#endif

#if defined(CFG_ZYNQ)  || defined(CFG_JAC6)
    uint8 macAddr[TCPIP_ETH_MAC_ADDR_SIZE] = ETH_MAC_ADDR;
    if(E_OK != EthIf_UpdatePhysAddrFilter(TCPIP_ETH_CTRL_INDEX,macAddr,ETH_ADD_TO_FILTER)){
        TCPIP_DET_REPORTERROR(TCPIP_INIT_SERVICE_ID, TCPIP_E_ARC_ETHIF_INIT_STATE_FAILED);
    }
#endif
}

/**
  * @brief  Frames reception function
  * @param  CtrlIdx
  * @param  FrameType
  * @param  IsBroadcast
  * @param  PhysAddrPtr
  * @param  DataPtr
  * @param  LenByte
  * @retval None
  */
static void LwIp_RxIndication( uint8 CtrlIdx, Eth_FrameType FrameType, boolean IsBroadcast, const uint8* PhysAddrPtr,
                        const Eth_DataType* DataPtr, uint16 LenByte ){
    struct pbuf *p;
    uint8 macAdrs[TCPIP_ETH_MAC_ADDR_SIZE];
    uint8 macBroadcastAdrs[TCPIP_ETH_MAC_ADDR_SIZE] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
    uint16 AdaptFrameType;
    err_t res;

#if ETH_PAD_SIZE
    LenByte += ETH_PAD_SIZE; /* allow room for Ethernet padding */
#endif
    p = pbuf_alloc(PBUF_RAW, LenByte+TCPIP_ETH_HEADER_SIZE, PBUF_POOL);
    if (p != NULL){
#if ETH_PAD_SIZE
       pbuf_header(p, -ETH_PAD_SIZE); /* drop the padding word */
#endif
       if(IsBroadcast == TRUE){
          MEMCPY((uint8*)p->payload,macBroadcastAdrs,TCPIP_ETH_MAC_ADDR_SIZE);
       }else{
          EthIf_GetPhysAddr(CtrlIdx,macAdrs);
          MEMCPY((uint8*)p->payload,macAdrs,TCPIP_ETH_MAC_ADDR_SIZE);
       }
       MEMCPY((uint8*)p->payload+TCPIP_ETH_MAC_ADDR_SIZE,PhysAddrPtr,TCPIP_ETH_MAC_ADDR_SIZE); /*lint !e9016, array index is not usable */
       AdaptFrameType =  PP_HTONS(FrameType);
       MEMCPY((uint8*)p->payload+(2*TCPIP_ETH_MAC_ADDR_SIZE),(void*)&AdaptFrameType,TCPIP_ETH_FRAMETYPE_SIZE);/*lint !e9016,array index is not usable */
       MEMCPY((uint8*)p->payload+TCPIP_ETH_HEADER_SIZE,DataPtr,LenByte); /*lint !e9016,array index is not usable */
       if(netIfPtrActive != NULL){ /* only one instance at the moment */
            res = tcpip_input(p, netIfPtrActive);
            if((err_t)ERR_OK != res){
            	(void)pbuf_free(p);
            }
       }else{
         (void)pbuf_free(p);
       }
    }else{
        /* Out of memory, Nothing to do but to through away data */
    }
}

/**
  * @brief  Frames Transmission function
  * @param  netif
  * @param  p
  * @retval err_t
  */
static err_t LwIp_Output(struct netif *netif, struct pbuf *p){ /*lint !e715 unused parameter netif  */
    const struct pbuf *q;
    uint16 l = 0;
    uint8 *buffer;
    uint8 destMacAdrs[TCPIP_ETH_MAC_ADDR_SIZE];
    uint8 frameTypeBuffer[TCPIP_ETH_FRAMETYPE_SIZE];
    uint32 baseAddress;
    uint32_t cnt;
    Eth_BufIdxType bufIndex = 0;
    uint16 lenByte;
    Eth_FrameType frameType;
    BufReq_ReturnType bufRes;
    Std_ReturnType txRes;
    uint8 ctrlIdx;
    uint8 priority = 0;
    err_t ret;

    ret = E_OK;
#if ETH_PAD_SIZE
    pbuf_header(p, -ETH_PAD_SIZE); /* drop the padding word */
#endif
	lenByte = (p->tot_len - TCPIP_ETH_HEADER_SIZE);
	MEMCPY(frameTypeBuffer, ((uint8*)p->payload + (2*TCPIP_ETH_MAC_ADDR_SIZE)), TCPIP_ETH_FRAMETYPE_SIZE); /*lint !e9016, array index is not usable */
	frameType = ((frameTypeBuffer[0] << 8) | frameTypeBuffer[1]); /*lint !e9053 !e9031 */
#if 0
    if(frameType != TCPIP_FRAME_TYPE_ARP){
        ctrlIdx = TcpIp_SocketAdmin[SocketIdInTransfer].ptrLocalAddr->TcpIpCtrlRef;
    }
#endif
    ctrlIdx = TCPIP_ETH_CTRL_INDEX;
    MEMCPY(destMacAdrs,(uint8*)p->payload,TCPIP_ETH_MAC_ADDR_SIZE);
#if (ETHIF_ENABLE_TX_INTERRUPT == STD_OFF)
    EthIf_MainFunctionTx(); // No iterations inside
#endif
    /* Note: take care to protect buffer allocation and transmit functionalities depending on the driver ,
     * Autosar spec does not say about this, and we will not protect it here */
    bufRes =  EthIf_ProvideTxBuffer(ctrlIdx,frameType,priority,&bufIndex,(Eth_DataType **) &baseAddress, &lenByte);/*lint !e929 !e740 pointer access No harm */
    if((bufRes != BUFREQ_OK)){
        /* notify error and return */
        /* Try to increase the number of TX buffers in the Lower layer */
        ret = (err_t)ERR_MEM;
    }
    if (ret == E_OK) { /*lint !e737 loss of sign in promotion of ret to unsigned during comparison can be ignored */
        buffer = (uint8*)(baseAddress);/*lint !e923 pointer access No harm */

        /* Skip Ethernet header, ARP responses includes adding Ethernet header in the stack itself which is not easily
         * bypassed from the stack at the moment, response callback other than ARP can be moved to ASR TcpIp layer. */
        for(q = p; q != NULL; q = q->next){
            if(l == 0u){
                MEMCPY(&buffer[l], ((uint8*)q->payload + TCPIP_ETH_HEADER_SIZE), (q->len - TCPIP_ETH_HEADER_SIZE));/*lint !e9031 !e9016 */ /* array index is not usable */
                l = (q->len - TCPIP_ETH_HEADER_SIZE);
            }else{
                MEMCPY(&buffer[l], (uint8*)q->payload, q->len);
                l = l + q->len;
            }
        }
    #if defined(ETH_CTRL_TX_FRAME_MIN_SIZE)
        if(l < (ETH_CTRL_TX_FRAME_MIN_SIZE - TCPIP_ETH_HEADER_SIZE)){
            for(uint8 i = l; i < (ETH_CTRL_TX_FRAME_MIN_SIZE - TCPIP_ETH_HEADER_SIZE); i++){
              buffer[i] = 0u;
            }
            l = (ETH_CTRL_TX_FRAME_MIN_SIZE - TCPIP_ETH_HEADER_SIZE);
        }
    #endif
        /* Try to send */
        cnt = 0;
        /** !req 4.2.2/SWS_TCPIP_00131 *//* Limitation in some drivers due to buffer getting free immediately when txconf is FALSE */
        txRes = EthIf_Transmit(ctrlIdx,bufIndex,frameType,TRUE/*txconf*/,l,destMacAdrs);
        // redundant piece of code
        while(txRes == E_PENDING){
            if((cnt % 500) == 0){
    #if (ETHIF_ENABLE_TX_INTERRUPT == STD_OFF)
                EthIf_MainFunctionTx();
    #endif
                txRes = EthIf_Transmit(ctrlIdx,bufIndex,frameType,TRUE,l,destMacAdrs);
            }
            cnt++;
            if(cnt > 1000000){
                /* timeout, notify error and return */
                ret = (err_t)ERR_TIMEOUT;
                break;
            }
        }
#if ETH_PAD_SIZE
        if (E_OK == ret ) {
            pbuf_header(p, ETH_PAD_SIZE); /* reclaim the padding word */
            /* Return SUCCESS */
        }
#endif
    }

    return ret;
} /*lint !e818 , const parameter is avoided due to callback *//*lint !e715 , netif parameter is not used */
#endif

/**
  * @brief  Callback function after TcpIp stack initilisation
  * @param  arg
  * @retval none
  */
static void LwIp_TcpIpInitDone(void *arg)
{
    tcpip_initialized = TRUE;
}/*lint !e818 , const parameter is avoided due to callback *//*lint !e715 *//* argument is not used */

static void LwIP_Init(void)
{
#if !defined(USE_NO_ETHIF)
    LwIp_EthernetifPreTcpIpInit(); /*lint !e522 because of TRCV module */
#endif
#if NO_SYS
#if (MEM_LIBC_MALLOC==0)
    mem_init();
#endif
#if (MEMP_MEM_MALLOC==0)
    memp_init();
#endif
#else
    pre_sys_init();
    tcpip_init(LwIp_TcpIpInitDone, NULL);
    while(tcpip_initialized == FALSE){
        (void)Sleep(1);
    };
#endif
}

/**
  * @brief  Internal function to set up network interface
  * @param  ..
  * @retval E_OK or E_NOT_OK
  */
static boolean LwIP_AddNetIf( const ip_addr_t *ipaddr, const ip_addr_t *netmask, const ip_addr_t *gw,
                          netif_init_fn initFnc, netif_input_fn inputFnc, struct netif *netif)
{
    boolean ret;
    ret = TRUE;
    /* Add network interface to the netif_list */
    if(NULL == netif_add(netif, ipaddr, netmask, gw, NULL, initFnc, inputFnc)){
        ret = FALSE;
    }
    if (TRUE == ret) {
        /*  Registers the default network interface.*/
        netif_set_default(netif);
#if LWIP_NETIF_STATUS_CALLBACK
        netif_set_status_callback(netif, LwIP_StatusCallback);
#endif

#if LWIP_NETIF_HOSTNAME
        netif_set_hostname(netif, "LWIP"); /*lint !e1778 due to LWIP */ /*lint !e9036 due to LWIP */
#endif
    }
    return ret;
}

/*
 * Local tcpip helper functions
 */

static void TcpIp_IpAddr32To8(uint32 src, uint8 *dest)
{
#if (CPU_BYTE_ORDER == HIGH_BYTE_FIRST)
    dest[0] = (uint8)((src & 0xff000000U) >> 24);
    dest[1] = (uint8)((src & 0x00ff0000U) >> 16);;
    dest[2] = (uint8)((src & 0x0000ff00U) >> 8);;
    dest[3] = (uint8)(src  & 0x000000ffU);
#else
    dest[3] = (uint8)((src & 0xff000000U) >> 24);
    dest[2] = (uint8)((src & 0x00ff0000U) >> 16);;
    dest[1] = (uint8)((src & 0x0000ff00U) >> 8);;
    dest[0] = (uint8)(src  & 0x000000ffU);
#endif
}

/*lint -save -e9032 -e9033 */
static void TcpIp_IpAddr8to32(const uint8 *src, uint32 *dest)
{
#if (CPU_BYTE_ORDER == HIGH_BYTE_FIRST)
    *dest = ( ((uint32)((src[0]) & 0xff) << 24) | ((uint32)((src[1]) & 0xff) << 16) | ((uint32)((src[2]) & 0xff) << 8)  | (uint32)((src[3]) & 0xff) );
#else
    *dest = ( ((uint32)((src[3]) & 0xff) << 24) | ((uint32)((src[2]) & 0xff) << 16) | ((uint32)((src[1]) & 0xff) << 8)  | (uint32)((src[0]) & 0xff) );
#endif
}

static void IP4NetmaskToIpAddr(ip_addr_t *NetmaskAddr, uint8 Netmask)
{
    uint8 addr[4];

    if(Netmask >= 24){
        addr[0] = 255;
        addr[1] = 255;
        addr[2] = 255;
        addr[3] = (uint8)(0xff00 >> (Netmask-24));
    }else if(Netmask >= 16){
        addr[0] = 255;
        addr[1] = 255;
        addr[2] = (uint8)(0xff00 >> (Netmask-16));
        addr[3] = 0;
    }else if(Netmask >= 8){
        addr[0] = 255;
        addr[1] = (uint8)(0xff00 >> (Netmask-8));
        addr[2] = 0;
        addr[3] = 0;
    }else{
        addr[0] = (uint8)(0xff00 >> Netmask);
        addr[1] = 0;
        addr[2] = 0;
        addr[3] = 0;
    }

    TcpIp_IpAddr8to32(addr, &NetmaskAddr->addr);
}
/*lint -restore*/ /* end for 9032 9033 */

/* Gets the index of a controller in the global configuration structure. */
static inline uint8 getLocalAddrListConfigIdx(uint8 addressId) {

    uint8 ret;
    ret = 0;
    for (uint8 i = 0; i < TCPIP_NOF_LOCALADDR; i++) {
        if (Config->Config.LocalAddrList[i].AddressId == addressId) {
            ret = i;
            break;
        }
    }
    return ret;
}

static void TcpIp_SwitchEthIfCtrlState(uint8 CtrlIdx, TcpIp_StateType TcpIpState)
{
    SchM_Enter_TcpIp_EA_0();
    if(TcpIp_CtrlAdmin[CtrlIdx].tcpipEthIfCtrlState != TcpIpState){
        TcpIp_CtrlAdmin[CtrlIdx].tcpipEthIfCtrlState = TcpIpState;
        /** @req 4.2.2/SWS_TCPIP_00084 */
        /* After each transition the TcpIp module shall report the new
        state to EthSM via EthSM_TcpIpModeIndication() */
        (void)EthSM_TcpIpModeIndication(CtrlIdx, TcpIpState); /* no return check , nothing can do about it */
    }
    SchM_Exit_TcpIp_EA_0();
}

static Std_ReturnType TcpIp_GetFirstFreeSocket(TcpIp_SocketIdType *SocketIdPtr){
    Std_ReturnType result = E_NOT_OK;
    SchM_Enter_TcpIp_EA_0();
    for(uint32 i=0; i < TCPIP_MAX_NOF_SOCKETS; i++){
        if( (TcpIp_SocketAdmin[i].socketState == TCPIP_SOCKET_INIT) &&
                (TcpIp_SocketAdmin[i].socketHandle == -1))
        {
            *SocketIdPtr = i;
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
    SchM_Exit_TcpIp_EA_0();
}

static void TcpIp_SocketStatusCheck(TcpIp_SocketIdType socketId)
{
    sint32 sockErr;
    boolean closeSock = FALSE;
    TcpIp_EventType event;

    /* Dont use getsockopt before better handling in lwip as it render in sleep behaviour
     * uint32 sockErrLen = sizeof(sockErr);
     * (void)getsockopt(TcpIp_SocketAdmin[socketId].socketHandle, SOL_SOCKET, SO_ERROR, &sockErr, &sockErrLen);
     */
    sockErr = lwip_geterr(TcpIp_SocketAdmin[socketId].socketHandle);
    switch(sockErr){
        /* Close socket on fatal errors */
        case ERR_ABRT:/** Connection aborted.      */
        case ERR_RST: /** Connection reset.        */
        case ERR_CLSD: /** Connection closed.       */
        case ERR_ARG: /** Illegal argument.        */
            closeSock = TRUE;
            break;
        case 0:/* OK */
        case ERR_INPROGRESS:/** Operation in progress    */
        case ERR_WOULDBLOCK:/** Operation would block.   */
            break;
        case ERR_MEM:/** Out of memory error.     */
        case ERR_BUF:/** Buffer error.            */
        case ERR_TIMEOUT:/** Timeout.                 */
        case ERR_RTE:/** Routing problem.         */
        case ERR_VAL:/** Illegal value.           */
        case ERR_USE:/** Address in use.          */
        case ERR_ALREADY:/** Already connecting.      */
        case ERR_ISCONN:/** Conn already established.*/
        case ERR_CONN:/** Not connected.           */
        case ERR_IF:/** Low-level netif error    */
        default:
            TcpIp_SocketAdmin[socketId].errCnt++;
#ifdef CFG_TCPIP_DEBUG
            if(TcpIp_SocketAdmin[socketId].errCnt > TcpIp_SocketAdmin[socketId].errCntMax){
                TcpIp_SocketAdmin[socketId].errCntMax = TcpIp_SocketAdmin[socketId].errCnt;
            }
            TcpIp_SocketAdmin[socketId].sockErrorsArr[TcpIp_SocketAdmin[socketId].errCnt] = sockErr;
#endif
            if(TcpIp_SocketAdmin[socketId].errCnt >= TCPIP_NOF_ERR_BEFORE_RESET){
                closeSock = TRUE;
                TcpIp_SocketAdmin[socketId].errCnt = 0;
            }
            break;
    }

    if(closeSock == TRUE){
#ifdef CFG_TCPIP_DEBUG
        TcpIp_SocketAdmin[socketId].nofCloseSock++;
#endif
        (void)closesocket(TcpIp_SocketAdmin[socketId].socketHandle);
        if(TRUE == TcpIp_SocketAdmin[socketId].socketProtocolIsTcp){
            event = TCPIP_TCP_CLOSED;
        }else{
            event = TCPIP_UDP_CLOSED;
        }
        /** @req 4.2.2/SWS_TCPIP_00144 */
        /** @req 4.2.2/SWS_TCPIP_00224 */
        /* The TcpIp module shall indicate events related to sockets to
        the upper layer module by using the Up_TcpIpEvent API and the following events:
        TCPIP_TCP_RESET, TCPIP_TCP_CLOSED, TCPIP_TCP_FIN_RECEIVED and
        TCPIP_UDP_CLOSED. */
        if(Config->Config.SocketOwnerConfig.SocketOwnerList[TcpIp_SocketAdmin[socketId].socketOwnerId].SocketOwnerTcpIpEventFncPtr != NULL){
            Config->Config.SocketOwnerConfig.SocketOwnerList[TcpIp_SocketAdmin[socketId].socketOwnerId].SocketOwnerTcpIpEventFncPtr(socketId, event);
        }
        TcpIp_FreeUpSocket(socketId);
    }
}


static uint16 TcpIp_SendIpMessage(sint32 SocketHandle, const uint8* DataPtr, const TcpIp_SockAddrType* RemoteAddrPtr, uint16 TotalLength)
{
    uint16 bytesSent = 0;
    sint16 response = 0;

    if (RemoteAddrPtr == NULL) {
        /* TCP */
        response = send(SocketHandle, DataPtr, TotalLength, 0);

    } else {
        /* UDP */
        struct sockaddr_in toAddr;
        uint32 toAddrLen = sizeof(toAddr);
        toAddr.sin_family = RemoteAddrPtr->domain;
        toAddr.sin_len = sizeof(toAddr);
        //uint8 length = (RemoteAddrPtr->domain == TCPIP_AF_INET6 ? TCPIP_SA_DATA_SIZE_IPV6:TCPIP_SA_DATA_SIZE_IPV4);
        TcpIp_IpAddr8to32(RemoteAddrPtr->addr, &toAddr.sin_addr.s_addr);

        toAddr.sin_port = htons(RemoteAddrPtr->port);
        response = sendto(SocketHandle, DataPtr, TotalLength, 0, (struct sockaddr *)&toAddr, toAddrLen);/*lint !e929 !e740 LWIP No harm */
    }
    if(response >= 0){
        bytesSent = (uint16)response;
    }
    return bytesSent;
}

static inline boolean TcpIp_BufferGet(uint32 size, uint8** buffPtr)
{
    boolean res;

    *buffPtr = mem_malloc(size);
    res = (*buffPtr != NULL) ? TRUE : FALSE;

    return res;
}

static inline void TcpIp_BufferFree(uint8* buffPtr)
{
    mem_free(buffPtr);
}

static Std_ReturnType TcpIp_ChangePhysAddrFilterForMulticast(uint8 CtrlIdx, uint32 ipaddr, Eth_FilterActionType action)
{
    uint8 mcastaddr[6];
    uint8 mcastipaddr[4];

    TcpIp_IpAddr32To8(ipaddr, mcastipaddr);

    mcastaddr[0] = 0x01u;
    mcastaddr[1] = 0x00u;
    mcastaddr[2] = 0x5eu;
    mcastaddr[3] = mcastipaddr[1] & 0x7fu;
    mcastaddr[4] = mcastipaddr[2];
    mcastaddr[5] = mcastipaddr[3];
    return EthIf_UpdatePhysAddrFilter(CtrlIdx, mcastaddr, action);
}

static Std_ReturnType verifyLocalAddrId(TcpIp_LocalAddrIdType LocalAddrId){
    boolean result = E_NOT_OK;
    for(uint8 i = 0; i < TCPIP_NOF_LOCALADDR; i++){
        if(Config->Config.LocalAddrList[i].AddressId == LocalAddrId){
            /* Id found in the config */
            result = E_OK;
            break;
        }
    }
    return result;
}


/**
 * @brief This API creates/allocates a new socket , needed to be global for TcpIp configuration file to use this
 * @param SocketOwnerId - Socket Id identifying the socket owner
 * @param Domain - IP address family.
 * @param protocol - Socket protocol as sub-family of parameter type.
 * @param SocketIdPtr - Pointer to socket identifier representing the requested socket.
 * This socket identifier must be provided for all further API calls which requires a SocketId.
 * @return E_OK or E_NOT_OK
 */
Std_ReturnType TcpIp_GetSocket(uint8 SocketOwnerId,TcpIp_DomainType Domain, TcpIp_ProtocolType Protocol, TcpIp_SocketIdType* SocketIdPtr)
{
    sint32 sockFd;
    sint32 sockType;
    Std_ReturnType result = E_OK;

    /* @req 4.2.2/SWS_TCPIP_00128 */
    VALIDATE( ((Domain == TCPIP_AF_INET) || (Domain == TCPIP_AF_INET6)), TCPIP_GETSOCKET_SERVICE_ID, TCPIP_E_AFNOSUPPORT)
    // No specified autosar error for protocol check, use TCPIP_E_NOPROTOOPT as it seems most logical
    VALIDATE( (TCPIP_IPPROTO_UDP == Protocol) || (TCPIP_IPPROTO_TCP == Protocol), TCPIP_GETSOCKET_SERVICE_ID, TCPIP_E_NOPROTOOPT)

    if (TCPIP_IPPROTO_TCP == Protocol) {
        sockType = SOCK_STREAM;
    } else {
        sockType = SOCK_DGRAM;
    }

    sockFd = socket(Domain, sockType, 0);
    if(sockFd < 0){
        result = E_NOT_OK;
    }else{
        TcpIp_SocketIdType socketId;
        sint32 on = 1;
        if(E_OK == TcpIp_GetFirstFreeSocket(&socketId)){
            /* Socket assigned, get first free socket index */
            TcpIp_SocketAdmin[socketId].socketHandle = sockFd;
            TcpIp_SocketAdmin[socketId].socketOwnerId = SocketOwnerId;
            TcpIp_SocketAdmin[socketId].socketState = TCPIP_SOCKET_BIND;
            TcpIp_SocketAdmin[socketId].socketProtocolIsTcp = ((TCPIP_IPPROTO_TCP == Protocol) ? TRUE:FALSE );
            *SocketIdPtr = socketId;
            if((err_t)ERR_OK != ioctlsocket(sockFd, FIONBIO, &on)){  /*lint !e569 !e970 !e9053 LWIP */ // Set socket to non block mode
                result = E_NOT_OK;
            }

        }else{
            result = E_NOT_OK;
            (void)closesocket(sockFd);
        }
     }
    return result;
}


/**
 * @brief This service initializes the TCP/IP Stack
 * @param SocketId - Socket handle identifying the local socket resource ConfigPtr - Configuration data ptr
 * @return void
 */
/** @req 4.2.2/SWS_TCPIP_00002 */
void TcpIp_Init(const TcpIp_ConfigType* ConfigPtr){ /*lint !e9046 Typographical ambiguity */
    if(ConfigPtr != NULL){
        Config = ConfigPtr;
        for(uint32 i=0; i < TCPIP_NOF_CONTROLLERS; i++){
            TcpIp_CtrlAdmin[i].tcpipEthIfCtrlState = TCPIP_STATE_OFFLINE;
            TcpIp_CtrlAdmin[i].tcpipEthIfRequestedCtrlState = TCPIP_STATE_OFFLINE;
            TcpIp_CtrlAdmin[i].ptrLocalUnicastAddr = NULL;
            TcpIp_CtrlAdmin[i].linkUp = FALSE;
        }

        for(uint32 i=0; i < TCPIP_MAX_NOF_SOCKETS; i++){
            TcpIp_SocketAdmin[i].socketState = TCPIP_SOCKET_INIT;
            TcpIp_SocketAdmin[i].socketHandle = -1;
            TcpIp_SocketAdmin[i].ptrLocalAddr = NULL;
            TcpIp_SocketAdmin[i].socketProtocolIsTcp = FALSE;
            TcpIp_SocketAdmin[i].errCnt = 0;
        }
        LwIP_Init();
    }
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
    Std_ReturnType result = E_NOT_OK;
    TcpIp_EventType event;
    (void)Abort;

    /** !req 4.2.2/SWS_TCPIP_00109 *//* Using sockets can not handle TCP abort */
    /** @req 4.2.2/SWS_TCPIP_00110 */
    /* Ignore abort parameter as closesocket always perform a FIN/ACK handshake. Using
     * shutdown does not solve this either */
    if((err_t)ERR_OK == closesocket(TcpIp_SocketAdmin[SocketId].socketHandle)){
      if(TRUE == TcpIp_SocketAdmin[SocketId].socketProtocolIsTcp){
            event = TCPIP_TCP_CLOSED;
        }else{
            event = TCPIP_UDP_CLOSED;
        }
        if(Config->Config.SocketOwnerConfig.SocketOwnerList[TcpIp_SocketAdmin[SocketId].socketOwnerId].SocketOwnerTcpIpEventFncPtr != NULL){
            Config->Config.SocketOwnerConfig.SocketOwnerList[TcpIp_SocketAdmin[SocketId].socketOwnerId].SocketOwnerTcpIpEventFncPtr(SocketId, event);
        }
        TcpIp_FreeUpSocket(SocketId);
        /* Update to upper layer via callback - Pending */
        result = E_OK;
    }

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
    /* TcpIp_LocalAddrIdType  as TCPIP_LOCALADDRID_ANY to bind to any EthIf controller - pending */
    const TcpIp_LocalAddrType *ptrLocalAddr;
    struct sockaddr_in sLocalAddr; /* only ipv4 */

    Std_ReturnType result;
    socklen_t addrlen;

    result = E_OK;
    if(verifyLocalAddrId(LocalAddrId) != E_OK){
        result = E_NOT_OK;
    }

    if (E_OK == result) {
        ptrLocalAddr = &Config->Config.LocalAddrList[getLocalAddrListConfigIdx(LocalAddrId)]; /* This could be ANY - to listen to any IP address*/
        /** @req 4.2.2/SWS_TCPIP_00111 */
        /** @req 4.2.2/SWS_TCPIP_00130 */
        /** @req 4.2.2/SWS_TCPIP_00015 */
        /** !req 4.2.2/SWS_TCPIP_00146 */
        /** !req 4.2.2/SWS_TCPIP_00147 */

        memset((uint8 *)&sLocalAddr, 0, sizeof(sLocalAddr));/*lint !e928 pointer to pointer no harm */

        /*Source*/
        sLocalAddr.sin_family = ptrLocalAddr->Domain;
        sLocalAddr.sin_len = sizeof(sLocalAddr);
        sLocalAddr.sin_addr.s_addr = ptrLocalAddr->StaticIpAddrConfig->StaticIpAddress;
        sLocalAddr.sin_port = htons(*PortPtr);

        if(bind(SocketId, (struct sockaddr *)&sLocalAddr, sizeof(sLocalAddr)) < 0) { /*lint !e929 !e740 */ /* pointer to pointer no harm */
            result = E_NOT_OK;
        }else{
            if(ptrLocalAddr->AddressType == TCPIP_MULTICAST){
                struct ip_mreq mreq;
                mreq.imr_multiaddr.s_addr = sLocalAddr.sin_addr.s_addr;
                /** @req 4.2.2/SWS_TCPIP_00178 */
                if(TcpIp_CtrlAdmin[ptrLocalAddr->TcpIpCtrlRef].ptrLocalUnicastAddr != NULL){
                    if(TRUE == TcpIp_CtrlAdmin[ptrLocalAddr->TcpIpCtrlRef].ptrLocalUnicastAddr->StaticIpAddrConfig->ArcValid){
                        mreq.imr_interface.s_addr = TcpIp_CtrlAdmin[ptrLocalAddr->TcpIpCtrlRef].ptrLocalUnicastAddr->StaticIpAddrConfig->StaticIpAddress;
                    }
                    else{
                        mreq.imr_interface.s_addr = htonl(INADDR_ANY);
                    }
                }

                if (setsockopt(SocketId, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0)
                {
                    result = E_NOT_OK;
                    /* IMPROVMENT, remove this when know working */
                }
            }
            if(result == E_OK){
                /* Find out what port was assigned */
                if((err_t)ERR_OK == getsockname(SocketId, (struct sockaddr *)&sLocalAddr, &addrlen)){ /*lint !e929 !e740 */ /* pointer to pointer no harm */
                    *PortPtr = htons(sLocalAddr.sin_port);
                }else{
                    TCPIP_DET_REPORTERROR(TCPIP_BIND_SERVICE_ID, TCPIP_E_ARC_GENERAL_FAILURE_TCPIP_STACK);
                }

                TcpIp_SocketAdmin[SocketId].socketHandle = SocketId;
                TcpIp_SocketAdmin[SocketId].ptrLocalAddr = ptrLocalAddr;
                if(TRUE == TcpIp_SocketAdmin[SocketId].socketProtocolIsTcp){
                    TcpIp_SocketAdmin[SocketId].socketState = TCPIP_SOCKET_TCP_CONNECT_LISTEN;
                }else{
                    TcpIp_SocketAdmin[SocketId].socketState = TCPIP_SOCKET_UDP_READY;
                }
            }
        }
    }

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
    Std_ReturnType result = E_OK;
    struct sockaddr_in socketAddress;

    /** @req 4.2.2/SWS_TCPIP_00129 */
    VALIDATE( (NULL != RemoteAddrPtr), TCPIP_TCPCONNECT_SERVICE_ID, TCPIP_E_PARAM_POINTER)
    memset((uint8 *)&socketAddress, 0, sizeof(socketAddress));/*lint !e928 pointer to pointer no harm */
    socketAddress.sin_family = RemoteAddrPtr->domain;
    socketAddress.sin_port = htons(RemoteAddrPtr->port);

    //length = (RemoteAddrPtr->domain == TCPIP_AF_INET6 ? TCPIP_SA_DATA_SIZE_IPV6:TCPIP_SA_DATA_SIZE_IPV4);
    TcpIp_IpAddr8to32(RemoteAddrPtr->addr, &socketAddress.sin_addr.s_addr);

    /** @req 4.2.2/SWS_TCPIP_00112 */
    (void)connect(TcpIp_SocketAdmin[SocketId].socketHandle, (struct sockaddr *)&socketAddress, sizeof(struct sockaddr)); /*lint !e929 !e740 */ /* pointer to pointer no harm */
    /* Non blocking connect call will always return error so need to check response */
    TcpIp_SocketAdmin[SocketId].socketState = TCPIP_SOCKET_TCP_CONNECTING;

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
    Std_ReturnType result = E_NOT_OK;

    /** @req 4.2.2/SWS_TCPIP_00113 */
    /** @req 4.2.2/SWS_TCPIP_00114 */
    if((err_t)ERR_OK == listen(TcpIp_SocketAdmin[SocketId].socketHandle, MaxChannels)){ /* BSD style */
        TcpIp_SocketAdmin[SocketId].socketState = TCPIP_SOCKET_TCP_LISTENING;
        result = E_OK;
    }
    return result;
}

/**
 * @brief By this API service the reception of socket data is confirmed to the TCP/IP stack.
 * @param SocketId - Socket handle identifying the local socket resource
 * @param Length - Number of bytes finally consumed by the upper layer.
 * @return result of the function.
 */
/** !req 4.2.2/SWS_TCPIP_00024 */
/** !req 4.2.2/SWS_TCPIP_00115 */
Std_ReturnType TcpIp_TcpReceived(TcpIp_SocketIdType SocketId, uint32 Length){
    Std_ReturnType result = E_OK;
    (void)SocketId;
    (void)Length;

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

    VALIDATE(CtrlIdx < TCPIP_NOF_CONTROLLERS, TCPIP_REQUESTCOMMODE_SERVICE_ID, TCPIP_E_INV_ARG);
    /** @req 4.2.2/SWS_TCPIP_00089 */
    VALIDATE(State != TCPIP_STATE_STARTUP,  TCPIP_REQUESTCOMMODE_SERVICE_ID, TCPIP_E_INV_ARG);
    VALIDATE(State != TCPIP_STATE_SHUTDOWN, TCPIP_REQUESTCOMMODE_SERVICE_ID, TCPIP_E_INV_ARG);

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
                TCPIP_DET_REPORTERROR(TCPIP_REQUESTCOMMODE_SERVICE_ID, TCPIP_E_INV_ARG);
                break;
        }
    }else{
        /* IMPROVMENT shall we respond E_OK when a request for the already active state is made? */
        result = E_OK;
    }

    if(result == E_OK){
        TcpIp_CtrlAdmin[CtrlIdx].tcpipEthIfRequestedCtrlState = State;
    }

    return result;
}

/**
 * @brief By this API service the local IP address assignment for the IP address specified by
 *        LocalAddrId shall be initiated.
 * @param LocalAddrId - IP address index specifying the IP address for which an
 *                      assignment shall be initiated.
 * @param Type - Type of IP address assignment which shall be initiated
 * @param LocalIpAddrPtr - Pointer to structure containing the IP address which shall be
 *                        assigned to the EthIf controller indirectly specified via LocalAddrId.
 * @param Netmask - Network mask of IPv4 address or address prefix of IPv6 address in CIDR Notation.
 * @param DefaultRouterPtr - Pointer to structure containing the IP address of the default router
 *                           (gateway) which shall be assigned.
 * @return result of the function.
 */
/** @req 4.2.2/SWS_TCPIP_00037 */
Std_ReturnType TcpIp_RequestIpAddrAssignment(TcpIp_LocalAddrIdType LocalAddrId, TcpIp_IpAddrAssignmentType Type,
        const TcpIp_SockAddrType* LocalIpAddrPtr, uint8 Netmask, const TcpIp_SockAddrType* DefaultRouterPtr){
    const TcpIp_LocalAddrType *ptrLocalAddr;
    struct netif *netif;
    ip_addr_t ipaddr;
    ip_addr_t netmask; /*lint !e9046  No Typographical ambiguity */
    ip_addr_t gw;
    Std_ReturnType result;

    ipaddr.addr = 0;
    netmask.addr = 0;
    gw.addr = 0;

    result = E_OK;
    if(verifyLocalAddrId(LocalAddrId) != E_OK){
        result = E_NOT_OK;
    }

    if (E_OK == result) {
        ptrLocalAddr = &Config->Config.LocalAddrList[getLocalAddrListConfigIdx(LocalAddrId)];
        netif = &TcpIp_CtrlAdmin[ptrLocalAddr->TcpIpCtrlRef].tcpipNetif;
        /** @req 4.2.2/SWS_TCPIP_00195 */
        /** @req 4.2.2/SWS_TCPIP_00198 */
        if(TCPIP_AUTOMATIC == ptrLocalAddr->AddressAssignment->AssignmentTrigger){
            result = E_NOT_OK;
        }
        else
        {
            /** @req 4.2.2/SWS_TCPIP_00116 */
            if(Type == TCPIP_IPADDR_ASSIGNMENT_STATIC) { /* this is left to the user , not verified IpAddress table with LocalAddrId */
                /** @req 4.2.2/SWS_TCPIP_00079 */
                if(TRUE == ptrLocalAddr->StaticIpAddrConfig->ArcValid){
                    ipaddr.addr = ptrLocalAddr->StaticIpAddrConfig->StaticIpAddress;
                    gw.addr = ptrLocalAddr->StaticIpAddrConfig->DefaultRouter;
                    IP4NetmaskToIpAddr(&netmask, ptrLocalAddr->StaticIpAddrConfig->Netmask);
                }else{
                    /** @req 4.2.2/SWS_TCPIP_00100 */
                    TcpIp_IpAddr8to32(LocalIpAddrPtr->addr,&ipaddr.addr);
                    //  IP4_ADDR(&ipaddr, LocalIpAddrPtr->addr[0],LocalIpAddrPtr->addr[1],LocalIpAddrPtr->addr[2],LocalIpAddrPtr->addr[3]);/*lint !e9032 !e9033 LWIP No harm */
                    IP4NetmaskToIpAddr(&netmask, Netmask);
                    //  IP4_ADDR(&gw, DefaultRouterPtr->addr[0],DefaultRouterPtr->addr[1],DefaultRouterPtr->addr[2],DefaultRouterPtr->addr[3]);/*lint !e9032 !e9033 LWIP No harm */
                    TcpIp_IpAddr8to32(DefaultRouterPtr->addr,&gw.addr);

                    ptrLocalAddr->StaticIpAddrConfig->StaticIpAddress = ipaddr.addr;
                    ptrLocalAddr->StaticIpAddrConfig->DefaultRouter = gw.addr;
                    ptrLocalAddr->StaticIpAddrConfig->Netmask = Netmask;
                    //ptrLocalAddr->StaticIpAddrConfig->ArcValid = TRUE;
                }
            }

            if(ip_addr_ismulticast(&ipaddr)){/* Multicast address */ /*lint !e845 !e778 !e572 LWIP  */ /*lint !e644 wrong lint */
                /* Join group here or in bind ? How will multicast be added ??? */

                /** @req 4.2.2/SWS_TCPIP_00080 */
                /* In case a multicast address is assigned, TcpIp shall derive
                the related physical address from the multicast IP address and add the derived
                address to the Eth MAC address filter by calling EthIf_UpdatePhys-
                AddrFilter() with action set to ETH_ADD_TO_FILTER. */
                result = TcpIp_ChangePhysAddrFilterForMulticast(ptrLocalAddr->TcpIpCtrlRef, ipaddr.addr, ETH_ADD_TO_FILTER);
            }
            else{ /* Unicast address */
                if( TRUE == LwIP_AddNetIf(&ipaddr, &netmask, &gw, ethernetif_init, tcpip_input, netif)){//IMPROVEMENT Assume there is only one unicast and netif per controller and hardcoded to one here... */

                    TcpIp_CtrlAdmin[ptrLocalAddr->TcpIpCtrlRef].ptrLocalUnicastAddr = ptrLocalAddr;

                    if( (Type == TCPIP_IPADDR_ASSIGNMENT_DHCP) || (Type == TCPIP_IPADDR_ASSIGNMENT_LINKLOCAL_DOIP) || (Type == TCPIP_IPADDR_ASSIGNMENT_LINKLOCAL) ){
                        if(Type == TCPIP_IPADDR_ASSIGNMENT_DHCP){
                            /* start dhcp search */
    #if LWIP_DHCP
                            if(ERR_OK != dhcp_start(netif)){
                                result = E_NOT_OK;
                            }
    #endif
                        }

                        if ((Type == TCPIP_IPADDR_ASSIGNMENT_LINKLOCAL_DOIP) || (Type == TCPIP_IPADDR_ASSIGNMENT_LINKLOCAL)){
                            if (LWIP_DHCP_AUTOIP_COOP == 0u){ /* dhcp_start will call autoip_start *//*lint !e774 !e506 use of macro */
                                /* start autoip configuration */
    #if LWIP_AUTOIP
                                if(ERR_OK != autoip_start(netif)){
                                    result = E_NOT_OK;
                                }
    #endif
                            }
                        }
                    }
                    else if(Type == TCPIP_IPADDR_ASSIGNMENT_STATIC) {
                        /* Hardcoded addresses, just bring them up */
                        netif_set_up(netif);
                    }
                    else{
                        result = E_NOT_OK;
                    }
                }else{
                    result = E_NOT_OK;
                }
            }
        }
    }

    return result;
}

static Std_ReturnType TcpIp_AutomaticIpAddrAssignment(TcpIp_LocalAddrIdType LocalAddrId, TcpIp_IpAddrAssignmentType Type){
    Std_ReturnType result = E_OK;
    const TcpIp_LocalAddrType *ptrLocalAddr = &Config->Config.LocalAddrList[getLocalAddrListConfigIdx(LocalAddrId)];
    struct netif *netif = &TcpIp_CtrlAdmin[ptrLocalAddr->TcpIpCtrlRef].tcpipNetif;
    ip_addr_t ipaddr;
    ip_addr_t netmask;
    ip_addr_t gw;

    ipaddr.addr = 0;
    netmask.addr = 0;
    gw.addr = 0;

    if(Type == TCPIP_IPADDR_ASSIGNMENT_STATIC) {
        if(TRUE == ptrLocalAddr->StaticIpAddrConfig->ArcValid){
            ipaddr.addr = ptrLocalAddr->StaticIpAddrConfig->StaticIpAddress;
            gw.addr = ptrLocalAddr->StaticIpAddrConfig->DefaultRouter;
            IP4NetmaskToIpAddr(&netmask, ptrLocalAddr->StaticIpAddrConfig->Netmask);
        }else{
            result = E_NOT_OK;
        }
    }

    if(result == E_OK){
        if(ip_addr_ismulticast(&ipaddr)){/* Multicast address */ /*lint !e845 !e778 !e572 LWIP */
            /* Join group here or in bind ? How will multicast be added ??? */

            /** @req 4.2.2/SWS_TCPIP_00080 */
            /* In case a multicast address is assigned, TcpIp shall derive
            the related physical address from the multicast IP address and add the derived
            address to the Eth MAC address filter by calling EthIf_UpdatePhys-
            AddrFilter() with action set to ETH_ADD_TO_FILTER. */
            result = TcpIp_ChangePhysAddrFilterForMulticast(ptrLocalAddr->TcpIpCtrlRef, ipaddr.addr, ETH_ADD_TO_FILTER);
        }
        else{ /* Unicast address */
            if(TRUE == LwIP_AddNetIf(&ipaddr, &netmask, &gw, ethernetif_init, tcpip_input, netif)){ //IMPROVEMENT Assume there is only one unicast and netif per controller and hardcoded to one here... */

                TcpIp_CtrlAdmin[ptrLocalAddr->TcpIpCtrlRef].ptrLocalUnicastAddr = ptrLocalAddr;

                if( (Type == TCPIP_IPADDR_ASSIGNMENT_DHCP) || (Type == TCPIP_IPADDR_ASSIGNMENT_LINKLOCAL_DOIP) || (Type == TCPIP_IPADDR_ASSIGNMENT_LINKLOCAL) ){
                    if(Type == TCPIP_IPADDR_ASSIGNMENT_DHCP){
                        /* start dhcp search */
#if LWIP_DHCP
                        if(ERR_OK != dhcp_start(netif)){
                            result = E_NOT_OK;
                        }
#endif
                    }

                    if ((Type == TCPIP_IPADDR_ASSIGNMENT_LINKLOCAL_DOIP) || (Type == TCPIP_IPADDR_ASSIGNMENT_LINKLOCAL)){
                        if (LWIP_DHCP_AUTOIP_COOP == 0u){ /* dhcp_start will call autoip_start *//*lint !e774 !e506  use of macro */
                            /* start autoip configuration */
#if LWIP_AUTOIP
                            if(ERR_OK != autoip_start(netif)){
                                result = E_NOT_OK;
                            }
#endif
                        }
                    }
                }
                else if(Type == TCPIP_IPADDR_ASSIGNMENT_STATIC) {
                    /* Hardcoded addresses, just bring them up */
                    netif_set_up(netif);
                }
                else{
                    result = E_NOT_OK;
                }
            }else{
                result = E_NOT_OK;
            }
        }
    }

    return result;
}


static Std_ReturnType TcpIp_RelIpAddrAssignment(TcpIp_LocalAddrIdType LocalAddrId){
    Std_ReturnType result = E_OK;

    const TcpIp_LocalAddrType *ptrLocalAddr = &Config->Config.LocalAddrList[getLocalAddrListConfigIdx(LocalAddrId)];
    struct netif *netif = &TcpIp_CtrlAdmin[ptrLocalAddr->TcpIpCtrlRef].tcpipNetif;
    TcpIp_IpAddrAssignmentType Type = ptrLocalAddr->AddressAssignment->AssignmentMethod;
    ip_addr_t ipaddr;
    // if(TRUE == ptrLocalAddr->StaticIpAddrConfig->ArcValid){
    // ptrLocalAddr->StaticIpAddrConfig->ArcValid = FALSE;
    ipaddr.addr = ptrLocalAddr->StaticIpAddrConfig->StaticIpAddress;

    if(ip_addr_ismulticast(&ipaddr)){/* Multicast address *//*lint !e845 !e778 !e572  LWIP interface */
        /* In case a multicast address is assigned, TcpIp shall derive
        the related physical address from the multicast IP address and add the derived
        address to the Eth MAC address filter by calling EthIf_UpdatePhys-
        AddrFilter() with action set to ETH_REMOVE_FROM_FILTER. */
        result = TcpIp_ChangePhysAddrFilterForMulticast(ptrLocalAddr->TcpIpCtrlRef, ipaddr.addr, ETH_REMOVE_FROM_FILTER);
    }
    else{ /* Unicast address */
        if( (Type == TCPIP_IPADDR_ASSIGNMENT_DHCP) || (Type == TCPIP_IPADDR_ASSIGNMENT_LINKLOCAL_DOIP) || (Type == TCPIP_IPADDR_ASSIGNMENT_LINKLOCAL) ){
            if(Type == TCPIP_IPADDR_ASSIGNMENT_DHCP){
                /* stop dhcp search */
#if LWIP_DHCP
                (void)dhcp_release(netif);
                dhcp_stop(netif);
#endif
            }

            if ((Type == TCPIP_IPADDR_ASSIGNMENT_LINKLOCAL_DOIP) || (Type == TCPIP_IPADDR_ASSIGNMENT_LINKLOCAL)){
                if (LWIP_DHCP_AUTOIP_COOP == 0u){ /* dhcp_start will call autoip_start *//*lint !e774 !e506  use of macro */
                    /* stop autoip configuration */
#if LWIP_AUTOIP
                    (void)autoip_stop(netif);
#endif
                }
            }
        }else if (Type == TCPIP_IPADDR_ASSIGNMENT_STATIC) {
            netif_remove(netif); /* netif will be made down before removal */
        }else{
            result = E_NOT_OK;
        }
    }

  //  }else{
  //      result = E_NOT_OK;
  //  }

    return result;
}

/**
 * @brief By this API service the local IP address assignment for the IP address specified by
 *        LocalAddrId shall be released.
 * @param LocalAddrId - IP address index specifying the IP address for which an
 *                      assignment shall be released.
 * @return result of the function.
 */
/** @req 4.2.2/SWS_TCPIP_00078 */
/** @req 4.2.2/SWS_TCPIP_00117 */
Std_ReturnType TcpIp_ReleaseIpAddrAssignment(TcpIp_LocalAddrIdType LocalAddrId){
    const TcpIp_LocalAddrType *ptrLocalAddr ;
    Std_ReturnType result;

    result = E_OK;
    if(verifyLocalAddrId(LocalAddrId) != E_OK){
        result = E_NOT_OK;
    }

    if (E_OK == result) {
        ptrLocalAddr = &Config->Config.LocalAddrList[getLocalAddrListConfigIdx(LocalAddrId)];

        /** @req 4.2.2/SWS_TCPIP_000199 */
        if(TCPIP_AUTOMATIC == ptrLocalAddr->AddressAssignment->AssignmentTrigger){
            result = E_NOT_OK;
        }else{
            result = TcpIp_RelIpAddrAssignment(LocalAddrId);
        }
    }

    return result;
}

/**
 * @brief Resets all learned IP-addresses to invalid values.
 * @param void
 * @return result of the function.
 */
/** !req 4.2.2/SWS_TCPIP_00215 */
Std_ReturnType TcpIp_ResetIpAssignment(void){
    Std_ReturnType result = E_NOT_OK;

    return result;
}

/**
 * @brief By this API service the TCP/IP stack sends an ICMP message according to the
 *        specified parameters.
 * @param LocalIpAddrId - IP address identifier representing the local IP address and EthIf
 *                        controller which shall be used for transmission of the ICMP message.
 * @param RemoteAddrPtr -  pointer to struct representing the remote address
 * @param Ttl - Time to live value to be used for the ICMP message. If 0 is
 *              specified the default value shall be used.
 * @param Type - type field value to be used in the ICMP message
 * @param Code - code field value to be used in the ICMP message
 * @param DataLength - length of ICMP message
 * @param DataPtr - Pointer to data which shall be sent as ICMP message data
 * @return result of the function.
 */
/** !req 4.2.2/SWS_TCPIP_00039 */
Std_ReturnType TcpIp_IcmpTransmit(TcpIp_LocalAddrIdType LocalIpAddrId, const TcpIp_SockAddrType* RemoteAddrPtr,
        uint8 Ttl, uint8 Type, uint8 Code, uint16 DataLength, const uint8* DataPtr){
    Std_ReturnType result = E_NOT_OK;

    /** !req 4.2.2/SWS_TCPIP_00118 */

    return result;
}/*lint !e715 unused parameters */

/**
 * @brief By this API service the TCP/IP stack sends an ICMPv6 message according to the
 *        specified parameters.
 * @param LocalIpAddrId - IP address identifier representing the local IP address and EthIf
 *                        controller which shall be used for transmission of the ICMPv6 message.
 * @param RemoteAddrPtr -  pointer to struct representing the remote address
 * @param HopLimit - Hop Limit value to be used for the ICMPv6 message. If 0 is
 *                   specified the default value shall be used.
 * @param Type - type field value to be used in the ICMPv6 message
 * @param Code - code field value to be used in the ICMPv6 message
 * @param DataLength - length of ICMPv6 message
 * @param DataPtr - Pointer to data which shall be sent as ICMPv6 message data
 * @return result of the function.
 */
/** !req 4.2.2/SWS_TCPIP_00187 */
//Std_ReturnType TcpIp_IcmpV6Transmit(TcpIp_LocalAddrIdType LocalIpAddrId, const TcpIp_SockAddrType* RemoteAddrPtr,
//		uint8 HopLimit, uint8 Type, uint8 Code, uint16 DataLength, const uint8* DataPtr){}

/**
 * @brief By this API service the TCP/IP stack retrieves DHCP option data identified by
 *        parameter option for already received DHCP options.
 * @param LocalIpAddrId - IP address identifier representing the local IP address and EthIf
 *                        controller for which the DHCP option shall be read.
 * @param Option - DHCP option according to IEFT RfC 2132, e.g. hostname
 * @param DataLength - As input parameter, contains the length of the provided data
 *                     buffer. Will be overwritten with the length of the actual data.
 * @param DataPtr - Pointer to memory containing DHCP option data
 * @return result of the function.
 */
/** !req 4.2.2/SWS_TCPIP_00040 */
Std_ReturnType TcpIp_DhcpReadOption(TcpIp_LocalAddrIdType LocalIpAddrId, uint8 Option, uint8* DataLength, uint8* DataPtr){
    Std_ReturnType result = E_NOT_OK;

    /** !req 4.2.2/SWS_TCPIP_00233 */
    /** !req 4.2.2/SWS_TCPIP_00234 */
    /** !req 4.2.2/SWS_TCPIP_00235 */
    /** !req 4.2.2/SWS_TCPIP_00236 */
    /** !req 4.2.2/SWS_TCPIP_00237 */
    return result;
}/*lint !e715 unused parameters */ /*lint !e818 ASR spec */

/**
 * @brief By this API service the TCP/IP stack retrieves DHCPv6 option data identified by
 *        parameter option for already received DHCPv6 options.
 * @param LocalIpAddrId - IP address identifier representing the local IP address and EthIf
 *                        controller for which the DHCPv6 option shall be read.
 * @param Option - DHCP option according to IEFT RfC 3315, e.g. hostname
 * @param DataLength - As input parameter, contains the length of the provided data
 *                     buffer. Will be overwritten with the length of the actual data.
 * @param DataPtr - Pointer to memory containing DHCPv6 option data
 * @return result of the function.
 */
/** !req 4.2.2/SWS_TCPIP_00189 */
Std_ReturnType TcpIp_DhcpV6ReadOption(TcpIp_LocalAddrIdType LocalIpAddrId, uint16 Option, uint16* DataLength, uint8* DataPtr){
    Std_ReturnType result = E_NOT_OK;

    /** !req 4.2.2/SWS_TCPIP_00238 */
    /** !req 4.2.2/SWS_TCPIP_00239 */
    /** !req 4.2.2/SWS_TCPIP_00240 */
    /** !req 4.2.2/SWS_TCPIP_00241 */
    /** !req 4.2.2/SWS_TCPIP_00242 */

    return result;
}/*lint !e715 unused parameters */ /*lint !e818 ASR spec */

/**
 * @brief By this API service the TCP/IP stack writes the DHCP option data identified by
parameter option
 * @param LocalIpAddrId - IP address identifier representing the local IP address and EthIf
 *                        controller for which the DHCP option shall be written.
 * @param Option - DHCP option according to IEFT RfC 2132, e.g. hostname
 * @param DataLength - Length of DHCP option data
 * @param DataPtr - Pointer to memory containing DHCP option data
 * @return result of the function.
 */
/** !req 4.2.2/SWS_TCPIP_00020 */
Std_ReturnType TcpIp_DhcpWriteOption(TcpIp_LocalAddrIdType LocalIpAddrId, uint8 Option, uint8 DataLength, const uint8* DataPtr){
    Std_ReturnType result = E_NOT_OK;

    /** !req 4.2.2/SWS_TCPIP_00243 */
    /** !req 4.2.2/SWS_TCPIP_00244 */
    /** !req 4.2.2/SWS_TCPIP_00245 */
    /** !req 4.2.2/SWS_TCPIP_00246 */
    /** !req 4.2.2/SWS_TCPIP_00247 */

    return result;
}/*lint !e715 unused parameters */

/**
 * @brief By this API service the TCP/IP stack writes the DHCPv6 option data identified by
 *        parameter option.
 * @param LocalIpAddrId - IP address identifier representing the local IP address and EthIf
 *                        controller for which the DHCPv6 option shall be written.
 * @param Option - DHCP option according to IEFT RfC 3315, e.g. hostname
 * @param DataLength - length of DHCPv6 option data.
 * @param DataPtr - Pointer to memory containing DHCPv6 option data
 * @return result of the function.
 */
/** !req 4.2.2/SWS_TCPIP_00190 */
Std_ReturnType TcpIp_DhcpV6WriteOption(TcpIp_LocalAddrIdType LocalIpAddrId,uint16 Option,uint16 DataLength,const uint8* DataPtr){
    Std_ReturnType result = E_NOT_OK;

    /** !req 4.2.2/SWS_TCPIP_00248 */
    /** !req 4.2.2/SWS_TCPIP_00249 */
    /** !req 4.2.2/SWS_TCPIP_00250 */
    /** !req 4.2.2/SWS_TCPIP_00251 */
    /** !req 4.2.2/SWS_TCPIP_00252 */

    return result;
}/*lint !e715 unused parameters */

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

    /** !req 4.2.2/SWS_TCPIP_00119 */ /* Partial support */
    switch (ParameterId)
    {
        case TCPIP_PARAMID_TCP_RXWND_MAX:
            break;
        case TCPIP_PARAMID_FRAMEPRIO:
            break;
        case TCPIP_PARAMID_TCP_NAGLE:
            {
                sint32 on = 1;
                sint32 status;
                status  = ioctlsocket(TcpIp_SocketAdmin[SocketId].socketHandle, FIONBIO, &on);/*lint !e569 !e970 !e9053 LWIP */
                status |= setsockopt(TcpIp_SocketAdmin[SocketId].socketHandle, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(sint32)); // Set socket to no delay
                if(status != (err_t)ERR_OK){
                    result = E_NOT_OK;
                }
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

    return result;
}/*lint !e715 ParamterValue is currently unused */

/**
 * @brief Obtains the local IP address actually used by LocalAddrId, the netmask and default router
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
    const TcpIp_LocalAddrType *ptrLocalAddr;
    Std_ReturnType result;

    result = E_OK;
    if(verifyLocalAddrId(LocalAddrId) != E_OK){
        result =  E_NOT_OK;
    }

    if (E_OK == result) {
        ptrLocalAddr = &Config->Config.LocalAddrList[getLocalAddrListConfigIdx(LocalAddrId)];

        /** @req 4.2.2/SWS_TCPIP_00205 */
        VALIDATE(IpAddrPtr->domain == ptrLocalAddr->Domain, TCPIP_GETIPADDR_SERVICE_ID, TCPIP_E_INV_ARG);
        /** @req 4.2.2/SWS_TCPIP_00206 */
        VALIDATE(DefaultRouterPtr->domain == IpAddrPtr->domain, TCPIP_GETIPADDR_SERVICE_ID, TCPIP_E_INV_ARG);

        TcpIp_IpAddr32To8(ptrLocalAddr->StaticIpAddrConfig->StaticIpAddress, IpAddrPtr->addr);
        TcpIp_IpAddr32To8(ptrLocalAddr->StaticIpAddrConfig->DefaultRouter, DefaultRouterPtr->addr);
        *NetmaskPtr = ptrLocalAddr->StaticIpAddrConfig->Netmask;
    }
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
    Std_ReturnType result;
    result = verifyLocalAddrId(LocalAddrId);

    if(result == E_OK){
        const TcpIp_LocalAddrType *ptrLocalAddr = &Config->Config.LocalAddrList[getLocalAddrListConfigIdx(LocalAddrId)];

        EthIf_GetPhysAddr(ptrLocalAddr->TcpIpCtrlRef, PhysAddrPtr);
    }
    return result;
}

/**
 * @brief TcpIp_GetRemotePhysAddr queries the IP/physical address translation table
 * specified by CtrlIdx and returns the physical address related to the IP address
 * specified by IpAddrPtr. In case no physical address can be retrieved and
 * parameter initRes is TRUE, address resolution for the specified IP address is
 * initiated on the local network.
 * @param CtrlIdx - EthIf controller index to identify the related ARP/NDP table.
 * @param IpAddrPtr - specifies the IP address for which the physical address shall be retrieved
 * @param PhysAddrPtr - Pointer to the memory where the physical address (MAC
 *                      address) related to the specified IP address is stored in
 *                      network byte order.
 * @param initRes - specifies if the address resolution shall be initiated (TRUE) or
 *                  not (FALSE) in case the physical address related to the
 *                  specified IP address is currently unknown.
 * @return result of the function.
 */
/** @req 4.2.2/SWS_TCPIP_00137 */
TcpIp_ReturnType TcpIp_GetRemotePhysAddr(uint8 CtrlIdx, const TcpIp_SockAddrType* IpAddrPtr, uint8* PhysAddrPtr,
        boolean initRes){
    TcpIp_ReturnType result = TCPIP_OK;
    struct eth_addr* ethaddr_ret;
    const ip_addr_t* ipaddr_ret;
    ip_addr_t ip;
    struct netif *netif;

    TcpIp_IpAddr8to32(IpAddrPtr->addr, &ip.addr);
    netif = &TcpIp_CtrlAdmin[CtrlIdx].tcpipNetif;

    /** !req 4.2.2/SWS_TCPIP_00139 */

    /** @req 4.2.2/SWS_TCPIP_00138 */
    if (etharp_find_addr(netif, &ip, &ethaddr_ret, &ipaddr_ret) > -1)
    {
        /* If the physical address is already known, PhysAddrPtr shall be set to the related
            physical address and the function shall return with TCPIP_E_OK */
        memcpy(PhysAddrPtr,ethaddr_ret->addr, ETHARP_HWADDR_LEN);
    }
    else{
        /* Otherwise it shall (a) initiate an address resolution if parameter initRes is set to
           TRUE and (b) return with TCPIP_E_PHYS_ADDR_MISS. PhysAddrPtr is not updated in this case. */
        result = TCPIP_E_PHYS_ADDR_MISS;
        if(TRUE == initRes){
            /* Initate address resolution */
            if((err_t)ERR_OK != etharp_query(netif, &ip, NULL)){
                TCPIP_DET_REPORTERROR(TCPIP_GETREMOTEPHYSADDR_SERVICE_ID, TCPIP_E_ARC_GENERAL_FAILURE_TCPIP_STACK);
            }
        }
    }

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
    Std_ReturnType result;

    /** @req 4.2.2/SWS_TCPIP_00141 */
    result = verifyLocalAddrId(LocalAddrId);

    if(result == E_OK){
        const TcpIp_LocalAddrType *ptrLocalAddr = &Config->Config.LocalAddrList[getLocalAddrListConfigIdx(LocalAddrId)];
        *CtrlIdxPtr = ptrLocalAddr->TcpIpCtrlRef;
    }
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
/** @req 4.2.2/SWS_TCPIP_00225 */
Std_ReturnType TcpIp_UdpTransmit(TcpIp_SocketIdType SocketId, const uint8* DataPtr, const TcpIp_SockAddrType* RemoteAddrPtr, uint16 TotalLength){
    Std_ReturnType result = E_OK;
    BufReq_ReturnType  bufRes;
    uint8*  bufPtr;
    uint8  ctrlIdx;
    uint16 len = TotalLength;
    uint8 physAddr[TCPIP_ETH_MAC_ADDR_SIZE];
    uint16 bytesSent;

#if defined(TCPIP_TX_BUF_SINGLE_COPY)
    uint32 baseAddress;
    Eth_BufIdxType  bufIdx;
    uint8 priority = 0;
    Eth_FrameType  frameType = TCPIP_FRAME_TYPE_IPV4;
    uint8 destMacAdrs[TCPIP_ETH_MAC_ADDR_SIZE];
#endif

    /* Added some protection even though not specified */
    VALIDATE( (SocketId < TCPIP_MAX_NOF_SOCKETS), TCPIP_UDPTRANSMIT_SERVICE_ID, TCPIP_E_INV_ARG)
    VALIDATE( (TcpIp_SocketAdmin[SocketId].socketHandle >= 0), TCPIP_UDPTRANSMIT_SERVICE_ID, TCPIP_E_NOTCONN)
    VALIDATE( (TcpIp_SocketAdmin[SocketId].socketState  == TCPIP_SOCKET_UDP_READY), TCPIP_UDPTRANSMIT_SERVICE_ID, TCPIP_E_ARC_ILLEGAL_STATE);
    ctrlIdx = TcpIp_SocketAdmin[SocketId].ptrLocalAddr->TcpIpCtrlRef;
    /** @req 4.2.2/SWS_TCPIP_00074 */
    VALIDATE( (TcpIp_CtrlAdmin[ctrlIdx].tcpipEthIfCtrlState  == TCPIP_STATE_ONLINE), TCPIP_UDPTRANSMIT_SERVICE_ID, TCPIP_E_ARC_ILLEGAL_STATE);
    /** !req 4.2.2/SWS_TCPIP_00122 */
    /*
    if(TcpIp_SocketAdmin[SocketId].socketState == TCPIP_SOCKET_BIND){ */
        /* IMPROVEMENT Do we need to do anything here???
         * Not been bind to an address, use local ip address and port */
    /* } */



    /** @req 4.2.2/SWS_TCPIP_00120 */
    EthIf_GetPhysAddr(ctrlIdx,physAddr);

#if defined(TCPIP_TX_BUF_SINGLE_COPY)
    bufRes = EthIf_ProvideTxBuffer(ctrlIdx,frameType,priority,&bufIdx,(Eth_DataType **) &baseAddress, &len);
    if(bufRes == BUFREQ_OK){
        bufPtr = (uint8*)(baseAddress);
    }
#else
    bufRes = BUFREQ_NOT_OK;
       if(TRUE == TcpIp_BufferGet(len,&bufPtr)){
           bufRes = BUFREQ_OK;
       }
#endif

    /** @req 4.2.2/SWS_TCPIP_00121 */
    if(bufRes == BUFREQ_OK){
        if(DataPtr == NULL){
            /* Copy data callback to upper layer  */
            if(Config->Config.SocketOwnerConfig.SocketOwnerList[TcpIp_SocketAdmin[SocketId].socketOwnerId].SocketOwnerCopyTxDataFncPtr != NULL){
                if(BUFREQ_OK != Config->Config.SocketOwnerConfig.SocketOwnerList[TcpIp_SocketAdmin[SocketId].socketOwnerId].SocketOwnerCopyTxDataFncPtr(SocketId, bufPtr, len)){ /* no Tp Handling */
                    TCPIP_DET_REPORTERROR(TCPIP_UDPTRANSMIT_SERVICE_ID, TCPIP_E_ARC_GENERAL_FAILURE_TCPIP);
                }
            }
        }else{
            memcpy(bufPtr,DataPtr,len);
        }

#if defined(TCPIP_TX_BUF_SINGLE_COPY)
        /* Find out how to grab destination MAC address */
        /* Add padding bytes if any before dispatch to OSI layer 2 */
        /** !req 4.2.2/SWS_TCPIP_00131 */
        result = EthIf_Transmit(ctrlIdx,bufIdx,frameType,TRUE/*txconf*/,len,destMacAdrs);
        /* SocketIdInTransfer = SocketId; */
#else
        bytesSent = TcpIp_SendIpMessage(TcpIp_SocketAdmin[SocketId].socketHandle, bufPtr, RemoteAddrPtr, TotalLength);
        TcpIp_BufferFree(bufPtr);
        if(bytesSent == 0){
            result = E_NOT_OK;
        }
#endif
        if(Config->Config.SocketOwnerConfig.SocketOwnerList[TcpIp_SocketAdmin[SocketId].socketOwnerId].SocketOwnerTxConfirmationFncPtr != NULL){
            Config->Config.SocketOwnerConfig.SocketOwnerList[TcpIp_SocketAdmin[SocketId].socketOwnerId].SocketOwnerTxConfirmationFncPtr(SocketId,bytesSent);
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
/** @req 4.2.2/SWS_TCPIP_00050 */
/** @req 4.2.2/SWS_TCPIP_00123 */
/** @req 4.2.2/SWS_TCPIP_00124 */
/** !req 4.2.2/SWS_TCPIP_00125 */
Std_ReturnType TcpIp_TcpTransmit(TcpIp_SocketIdType SocketId, const uint8* DataPtr, uint32 AvailableLength,
        boolean ForceRetrieve){
    Std_ReturnType result = E_OK;
    BufReq_ReturnType  bufRes;
    uint8*  bufPtr;
    uint8  ctrlIdx;
    uint16 len = AvailableLength;
    uint8 physAddr[TCPIP_ETH_MAC_ADDR_SIZE];
    uint16 bytesSent;
    (void)ForceRetrieve;

#if defined(TCPIP_TX_BUF_SINGLE_COPY)
    uint32 baseAddress;
    uint8 priority = 0;
    Eth_FrameType  frameType = TCPIP_FRAME_TYPE_IPV4;
    Eth_BufIdxType  bufIdx;
    uint8 destMacAdrs[TCPIP_ETH_MAC_ADDR_SIZE];
#endif

    /* Added some protection even though not specified */
    VALIDATE( (SocketId < TCPIP_MAX_NOF_SOCKETS), TCPIP_TCPTRANSMIT_SERVICE_ID, TCPIP_E_INV_ARG)
    VALIDATE( (TcpIp_SocketAdmin[SocketId].socketHandle >= 0), TCPIP_TCPTRANSMIT_SERVICE_ID, TCPIP_E_NOTCONN)

    ctrlIdx = TcpIp_SocketAdmin[SocketId].ptrLocalAddr->TcpIpCtrlRef;
    /** @req 4.2.2/SWS_TCPIP_00074 */
    VALIDATE( (TcpIp_CtrlAdmin[ctrlIdx].tcpipEthIfCtrlState  == TCPIP_STATE_ONLINE), TCPIP_TCPTRANSMIT_SERVICE_ID, TCPIP_E_ARC_ILLEGAL_STATE);

    /** @req 4.2.2/SWS_TCPIP_00120 */
    EthIf_GetPhysAddr(ctrlIdx,physAddr);

#if defined(TCPIP_TX_BUF_SINGLE_COPY)
    bufRes = EthIf_ProvideTxBuffer(ctrlIdx,frameType,priority,&bufIdx,(Eth_DataType **) &baseAddress, &len);
    if(bufRes == BUFREQ_OK){
        bufPtr = (uint8*)(baseAddress);
    }
#else
    bufRes = BUFREQ_NOT_OK;
    if(TcpIp_BufferGet(len,&bufPtr) == TRUE){
        bufRes = BUFREQ_OK;
    }
#endif

    /** @req 4.2.2/SWS_TCPIP_00121 */
    /** @req 4.2.2/SWS_TCPIP_00228 */
    if(bufRes == BUFREQ_OK){
        if(DataPtr == NULL){
            /* Copy data callback to upper layer  */
            if(Config->Config.SocketOwnerConfig.SocketOwnerList[TcpIp_SocketAdmin[SocketId].socketOwnerId].SocketOwnerCopyTxDataFncPtr != NULL){
                if(BUFREQ_OK != Config->Config.SocketOwnerConfig.SocketOwnerList[TcpIp_SocketAdmin[SocketId].socketOwnerId].SocketOwnerCopyTxDataFncPtr(SocketId, bufPtr, len)){/* no Tp Handling */
                    TCPIP_DET_REPORTERROR(TCPIP_TCPTRANSMIT_SERVICE_ID, TCPIP_E_ARC_GENERAL_FAILURE_TCPIP);
                }
            }
        }else{
            memcpy(bufPtr,DataPtr,len);
        }

#if defined(TCPIP_TX_BUF_SINGLE_COPY)
        /* Find out how to grab destination MAC address */
        /* Add padding bytes if any before dispatch to OSI layer 2 */
        /** !req 4.2.2/SWS_TCPIP_00131 */
        result = EthIf_Transmit(ctrlIdx,bufIdx,frameType,TRUE/*txconf*/,len,destMacAdrs);
#else
        /* SocketIdInTransfer = SocketId; */
        bytesSent = TcpIp_SendIpMessage(TcpIp_SocketAdmin[SocketId].socketHandle, bufPtr, NULL, AvailableLength);
        TcpIp_BufferFree(bufPtr);
        if(bytesSent == 0){
            result = E_NOT_OK;
        }
#endif
        if(Config->Config.SocketOwnerConfig.SocketOwnerList[TcpIp_SocketAdmin[SocketId].socketOwnerId].SocketOwnerTxConfirmationFncPtr != NULL){
            Config->Config.SocketOwnerConfig.SocketOwnerList[TcpIp_SocketAdmin[SocketId].socketOwnerId].SocketOwnerTxConfirmationFncPtr(SocketId,bytesSent);
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
/** @req 4.2.2/SWS_TCPIP_00029 */
void TcpIp_RxIndication(uint8 CtrlIdx, Eth_FrameType FrameType, boolean IsBroadcast, const uint8* PhysAddrPtr,
        uint8* DataPtr, uint16 LenByte){
    if(tcpip_initialized == TRUE)
    {
#if !defined(USE_NO_ETHIF)
        LwIp_RxIndication(CtrlIdx,FrameType,IsBroadcast,PhysAddrPtr,DataPtr,LenByte);
#endif
    }
}/*lint !e818 ASR specification */

static void TcpIp_NotifyLocalIpAddrAssignmentChg(uint8 CtrlIdx, TcpIp_IpAddrStateType ipAddrState)
{
    /* Call upper layer */
    const TcpIp_LocalAddrType *ptrLocalAddr;
    for(uint32 LocalAddrId=0; LocalAddrId < TCPIP_NOF_LOCALADDR; LocalAddrId++){
        ptrLocalAddr = &Config->Config.LocalAddrList[getLocalAddrListConfigIdx(LocalAddrId)];
        if(ptrLocalAddr->TcpIpCtrlRef == CtrlIdx)
        {
            for(uint32 Idx=0; Idx < TCPIP_NOF_SOCKETOWNERS; Idx++){
                if(Config->Config.SocketOwnerConfig.SocketOwnerList[Idx].SocketOwnerLocalIpAddrAssignmentChgFncPtr != NULL){
                    Config->Config.SocketOwnerConfig.SocketOwnerList[Idx].SocketOwnerLocalIpAddrAssignmentChgFncPtr(ptrLocalAddr->AddressId, ipAddrState);
                }
            }
        }
    }
}

static void TcpIp_EnableAllIpAddrAssignments(uint8 CtrlIdx){
    const TcpIp_LocalAddrType *ptrLocalAddr;
    /* We initiate all unicast first and then any multicast. We assume there is only one */
    for(uint32 i=0; i<TCPIP_NOF_LOCALADDR;i++){
        ptrLocalAddr = &Config->Config.LocalAddrList[i];

        if((ptrLocalAddr->TcpIpCtrlRef == CtrlIdx) && (ptrLocalAddr->AddressAssignment->AssignmentTrigger == TCPIP_AUTOMATIC) && (ptrLocalAddr->AddressType == TCPIP_UNICAST)){
            if(E_NOT_OK == TcpIp_AutomaticIpAddrAssignment(ptrLocalAddr->AddressId, ptrLocalAddr->AddressAssignment->AssignmentMethod)){
                TCPIP_DET_REPORTERROR(TCPIP_MAINFUNCTION_SERVICE_ID, TCPIP_E_ARC_ILLEGAL_STATE);
            }
        }
    }
    for(uint32 i=0; i<TCPIP_NOF_LOCALADDR;i++){
        ptrLocalAddr =  &Config->Config.LocalAddrList[i];

        if((ptrLocalAddr->TcpIpCtrlRef == CtrlIdx) && (ptrLocalAddr->AddressAssignment->AssignmentTrigger == TCPIP_AUTOMATIC) && (ptrLocalAddr->AddressType == TCPIP_MULTICAST)){
            if(E_NOT_OK == TcpIp_AutomaticIpAddrAssignment(ptrLocalAddr->AddressId, ptrLocalAddr->AddressAssignment->AssignmentMethod)){
                TCPIP_DET_REPORTERROR(TCPIP_MAINFUNCTION_SERVICE_ID, TCPIP_E_ARC_ILLEGAL_STATE);
            }
        }
    }
}

static void TcpIp_DisableAllIpAddrAssignments(uint8 CtrlIdx){
    const TcpIp_LocalAddrType *ptrLocalAddr;
    /* We initiate all unicast first and then any multicast. We assume there is only one */
    for(uint32 i=0; i<TCPIP_NOF_LOCALADDR;i++){
        ptrLocalAddr = &Config->Config.LocalAddrList[i];

        if((ptrLocalAddr->TcpIpCtrlRef == CtrlIdx) && (ptrLocalAddr->AddressAssignment->AssignmentTrigger == TCPIP_AUTOMATIC) && (ptrLocalAddr->AddressType == TCPIP_UNICAST)){
            if(E_NOT_OK == TcpIp_RelIpAddrAssignment(ptrLocalAddr->AddressId)){
                TCPIP_DET_REPORTERROR(TCPIP_MAINFUNCTION_SERVICE_ID, TCPIP_E_ARC_ILLEGAL_STATE);
            }
        }
    }
    for(uint32 i=0; i<TCPIP_NOF_LOCALADDR;i++){
        ptrLocalAddr =  &Config->Config.LocalAddrList[i];

        if((ptrLocalAddr->TcpIpCtrlRef == CtrlIdx) && (ptrLocalAddr->AddressAssignment->AssignmentTrigger == TCPIP_AUTOMATIC) && (ptrLocalAddr->AddressType == TCPIP_MULTICAST)){
            if(E_NOT_OK == TcpIp_RelIpAddrAssignment(ptrLocalAddr->AddressId)){
                TCPIP_DET_REPORTERROR(TCPIP_MAINFUNCTION_SERVICE_ID, TCPIP_E_ARC_ILLEGAL_STATE);
            }
        }
    }
}

static void TcpIp_HandleStateOffline(uint8 CtrlIdx)
{
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
    else{
        /* Do nothing */
    }
}

/** @req 4.2.2/SWS_TCPIP_00229 */
static void TcpIp_EnterOfflineState(uint8 CtrlIdx){
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
    for(uint32 SocketId=0; SocketId < TCPIP_MAX_NOF_SOCKETS; SocketId++){
        if((TcpIp_SocketAdmin[SocketId].ptrLocalAddr != NULL) && (TcpIp_SocketAdmin[SocketId].ptrLocalAddr->TcpIpCtrlRef == CtrlIdx))
        {
            (void)TcpIp_Close(SocketId,FALSE);
        }
    }
    /* (d) in case the no EthIf controller is assigned any more, all unbound sockets shall
    be released as well */
    boolean otherCtrls = FALSE;
    for(uint32 SocketId=0; SocketId < TCPIP_MAX_NOF_SOCKETS; SocketId++){
       if(TcpIp_SocketAdmin[SocketId].ptrLocalAddr != NULL){
           /* There are sockets assigned to other ctrl's */
           otherCtrls = TRUE;
           break;
       }
    }
    if(otherCtrls == FALSE){
        /* There are no other assigned controllers, release all unbound sockets */
        for(uint32 SocketId=0; SocketId < TCPIP_MAX_NOF_SOCKETS; SocketId++){
            if(TcpIp_SocketAdmin[SocketId].socketHandle >= 0){
                (void)TcpIp_Close(SocketId,FALSE);
            }
        }
    }

    TcpIp_DisableAllIpAddrAssignments(CtrlIdx);

    /* (e) enter the state TCPIP_STATE_SHUTDOWN for the EthIf controller. */
    TcpIp_SwitchEthIfCtrlState(CtrlIdx, TCPIP_STATE_SHUTDOWN);
}

static void TcpIp_HandleStateOnline(uint8 CtrlIdx)
{
    if(TcpIp_CtrlAdmin[CtrlIdx].tcpipEthIfRequestedCtrlState == TCPIP_STATE_OFFLINE){
        TcpIp_EnterOfflineState(CtrlIdx);
    }
    else if(TcpIp_CtrlAdmin[CtrlIdx].tcpipEthIfRequestedCtrlState == TCPIP_STATE_ONHOLD){
        /** @req 4.2.2/SWS_TCPIP_00076 */
        /* If TCPIP_STATE_ONHOLD is requested for an EthIf
        controller and the current state is TCPIP_STATE_ONLINE for that EthIf controller,
        the TcpIp module shall
        (a) notify the upper layer via Up_LocalIpAddrAssignmentChg() with State
        TCPIP_IPADDR_STATE_ONHOLD for all assigned IP addresses of the
        related EthIf controller, and */
        netif_set_down(&TcpIp_CtrlAdmin[CtrlIdx].tcpipNetif);
        /* (b) deactivate the communication within the TcpIp module for the related EthIf
        controller, and */
        TcpIp_NotifyLocalIpAddrAssignmentChg(CtrlIdx, TCPIP_IPADDR_STATE_ONHOLD);

        /* (c) enter the state TCPIP_STATE_ONHOLD for the EthIf controller.*/
        TcpIp_SwitchEthIfCtrlState(CtrlIdx, TCPIP_STATE_ONHOLD);
    }
    else{
        /* Do nothing */
    }
}

static void TcpIp_HandleStateOnhold(uint8 CtrlIdx)
{

    if(TcpIp_CtrlAdmin[CtrlIdx].tcpipEthIfRequestedCtrlState == TCPIP_STATE_OFFLINE){
        /** @req 4.2.2/SWS_TCPIP_00077 */
        TcpIp_EnterOfflineState(CtrlIdx);
    }
    else if(TcpIp_CtrlAdmin[CtrlIdx].tcpipEthIfRequestedCtrlState == TCPIP_STATE_ONLINE){
        /** @req 4.2.2/SWS_TCPIP_00086 */
       /* If TCPIP_STATE_ONLINE is requested for an EthIf
        controller and the current state is TCPIP_STATE_ONHOLD for that EthIf controller,
        the TcpIp module shall
        (a) reactivate the communication within the TcpIp module for the related EthIf
        controller, */
        netif_set_up(&TcpIp_CtrlAdmin[CtrlIdx].tcpipNetif);

        /* (b) call Up_LocalIpAddrAssignmentChg() with State
        TCPIP_IPADDR_STATE_ASSIGNED for all assigned IP addresses of the
        related EthIf controller, and */
        TcpIp_NotifyLocalIpAddrAssignmentChg(CtrlIdx, TCPIP_IPADDR_STATE_ASSIGNED);

        /* (c) enter the state TCPIP_STATE_ONLINE for the EthIf controller. */
        TcpIp_SwitchEthIfCtrlState(CtrlIdx, TCPIP_STATE_ONLINE);
    }
    else{
        /* Do nothing */
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
#if 0
#if LWIP_AUTOIP
        autoip_stop(&TcpIp_CtrlAdmin[CtrlIdx].tcpipNetif);
#endif
#if LWIP_DHCP
        dhcp_stop(&TcpIp_CtrlAdmin[CtrlIdx].tcpipNetif);
#endif
        netif_set_down(&TcpIp_CtrlAdmin[CtrlIdx].tcpipNetif); /* needs netif_set_up to come back */
#endif

        TcpIp_DisableAllIpAddrAssignments(CtrlIdx);

        /* (b) enter the state TCPIP_STATE_OFFLINE for the EthIf controller. */
        TcpIp_SwitchEthIfCtrlState(CtrlIdx, TCPIP_STATE_OFFLINE);
    }
    else if(TcpIp_CtrlAdmin[CtrlIdx].tcpipEthIfRequestedCtrlState == TCPIP_STATE_ONLINE){

        /** @req 4.2.2/SWS_TCPIP_00085 */
        /* If at least one IP address has been successfully assigned to
        an EthIf controller and the current state is TCPIP_STATE_STARTUP for that EthIf
        controller, the TcpIp module shall enter the state TCPIP_STATE_ONLINE for the
        EthIf controller.*/
        if(TRUE == TcpIp_CtrlAdmin[CtrlIdx].linkUp){
            /* (b) enter the state TCPIP_STATE_OFFLINE for the EthIf controller. */
            TcpIp_SwitchEthIfCtrlState(CtrlIdx, TCPIP_STATE_ONLINE);
            TcpIp_NotifyLocalIpAddrAssignmentChg(CtrlIdx, TCPIP_IPADDR_STATE_ASSIGNED);
        }
    }
    else{
        /* Do nothing */
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

/** @req 4.2.2/SWS_TCPIP_00226*/
static void TcpIp_HandleSocketStateListening(TcpIp_SocketIdType SocketId)
{
    sint32 clientFd;
    struct sockaddr_in clientAddr;
    uint32 addrlen = sizeof(clientAddr);

    clientFd = accept(TcpIp_SocketAdmin[SocketId].socketHandle, (struct sockaddr *)&clientAddr, (uint32 *)&addrlen);/*lint !e929 !e740 */ /* pointer to pointer no harm */

    if( clientFd != (-1))
    {
        Std_ReturnType accepted = E_NOT_OK;
        sint32 on = 1;
        /** @req 4.2.2/SWS_TCPIP_00114 */
        /* Derive a separate socket */
        TcpIp_SocketIdType socketIdConn;
        if(E_OK == TcpIp_GetFirstFreeSocket(&socketIdConn)){
            TcpIp_SocketAdmin[socketIdConn].socketHandle = clientFd;
            TcpIp_SocketAdmin[socketIdConn].remoteAddr.domain = clientAddr.sin_family;
            TcpIp_SocketAdmin[socketIdConn].remoteAddr.port = ntohs(clientAddr.sin_port);
            //uint8 length = (clientAddr.sin_family == TCPIP_AF_INET6 ? TCPIP_SA_DATA_SIZE_IPV6:TCPIP_SA_DATA_SIZE_IPV4);
            TcpIp_IpAddr32To8(clientAddr.sin_addr.s_addr, TcpIp_SocketAdmin[socketIdConn].remoteAddr.addr);

            TcpIp_SocketAdmin[socketIdConn].socketOwnerId = TcpIp_SocketAdmin[SocketId].socketOwnerId;

            /* Call upper layer for acceptance  */
            if(Config->Config.SocketOwnerConfig.SocketOwnerList[TcpIp_SocketAdmin[socketIdConn].socketOwnerId].SocketOwnerTcpAcceptedFncPtr != NULL){
                accepted = Config->Config.SocketOwnerConfig.SocketOwnerList[TcpIp_SocketAdmin[socketIdConn].socketOwnerId].SocketOwnerTcpAcceptedFncPtr(SocketId, socketIdConn, &TcpIp_SocketAdmin[socketIdConn].remoteAddr);
            }

            if(E_OK == accepted){
                TcpIp_SocketAdmin[socketIdConn].socketState = TCPIP_SOCKET_TCP_READY;
                TcpIp_SocketAdmin[socketIdConn].socketProtocolIsTcp = TRUE;
                /* Get local addr from listening socket */
                TcpIp_SocketAdmin[socketIdConn].ptrLocalAddr = TcpIp_SocketAdmin[SocketId].ptrLocalAddr;

                if((err_t)ERR_OK != ioctlsocket(clientFd, FIONBIO, &on)){  /*lint !e970 !e569 !e9053 LWIP */ // Set socket to non block mode
                    TCPIP_DET_REPORTERROR(TCPIP_MAINFUNCTION_SERVICE_ID, TCPIP_E_ARC_GENERAL_FAILURE_TCPIP_STACK);
                }
            }else{
                (void)closesocket(clientFd);
                TcpIp_FreeUpSocket(socketIdConn);
            }
        }
        else{
            (void)closesocket(clientFd);
        }

    }
}

static void TcpIp_HandleSocketStateConnecting(TcpIp_SocketIdType SocketId)
{
    /* Just wait and check if socket is connected */
    boolean connected = FALSE;
    boolean closeSock = FALSE;
    sint32 sockErr;

    sockErr = lwip_geterr(TcpIp_SocketAdmin[SocketId].socketHandle);
    switch(sockErr){
        case 0:
            connected = TRUE;
            break;
        case ERR_INPROGRESS:
            /* Trying to connect*/
            break;

        case ERR_ABRT:
        case ERR_RST:
        case ERR_CLSD:
            closeSock = TRUE;
            break;
        default:
           break;
    }

    if(closeSock == TRUE){
        (void)closesocket(TcpIp_SocketAdmin[SocketId].socketHandle);
        /** @req 4.2.2/SWS_TCPIP_00144 */
        /** @req 4.2.2/SWS_TCPIP_00224 */
        /* The TcpIp module shall indicate events related to sockets to
        the upper layer module by using the Up_TcpIpEvent API and the following events:
        TCPIP_TCP_RESET, TCPIP_TCP_CLOSED, TCPIP_TCP_FIN_RECEIVED and
        TCPIP_UDP_CLOSED. */
        if(Config->Config.SocketOwnerConfig.SocketOwnerList[TcpIp_SocketAdmin[SocketId].socketOwnerId].SocketOwnerTcpIpEventFncPtr != NULL){
            Config->Config.SocketOwnerConfig.SocketOwnerList[TcpIp_SocketAdmin[SocketId].socketOwnerId].SocketOwnerTcpIpEventFncPtr(SocketId, TCPIP_TCP_CLOSED);
        }
        TcpIp_FreeUpSocket(SocketId);
    }else if(connected == TRUE){
        TcpIp_SocketAdmin[SocketId].socketState = TCPIP_SOCKET_TCP_READY;
        /* Call upper layer */
        /** @req 4.2.2/SWS_TCPIP_00227 */
        if(Config->Config.SocketOwnerConfig.SocketOwnerList[TcpIp_SocketAdmin[SocketId].socketOwnerId].SocketOwnerTcpConnectedFncPtr != NULL){
            Config->Config.SocketOwnerConfig.SocketOwnerList[TcpIp_SocketAdmin[SocketId].socketOwnerId].SocketOwnerTcpConnectedFncPtr(SocketId);
        }
    }else{
        /* Do nothing but keep Lint happy */
    }
}

static void TcpIp_HandleSocketStateTcpReady(TcpIp_SocketIdType SocketId)
{
    sint32 nBytes = 1;
    uint8 *dataPtr;

    /* Note: Even it is not shown in the sequence diagram of section 9.3, TcpIp may
    decouple the data reception if required. E.g. for reassembling of incoming IP
    datagrams that are fragmented, TcpIp shall copy the received data to a TcpIp buffer
    and decouple TcpIp_RxIndication() from SoAd_RxIndication() */
    while(nBytes > 0){
        if (TRUE == TcpIp_BufferGet(TCPIP_RX_BUFFER_SIZE, &dataPtr)) {
            nBytes = recv(TcpIp_SocketAdmin[SocketId].socketHandle, dataPtr, TCPIP_RX_BUFFER_SIZE, 0);
            if (nBytes > 0){
                /* Call upper layer */
                /** @req 4.2.2/SWS_TCPIP_00223 */
                if(Config->Config.SocketOwnerConfig.SocketOwnerList[TcpIp_SocketAdmin[SocketId].socketOwnerId].SocketOwnerRxIndicationFncPtr != NULL){
                    Config->Config.SocketOwnerConfig.SocketOwnerList[TcpIp_SocketAdmin[SocketId].socketOwnerId].SocketOwnerRxIndicationFncPtr(SocketId, &TcpIp_SocketAdmin[SocketId].remoteAddr, dataPtr, nBytes);
                }
            }else if(nBytes == 0){
            	/* connection closed */
                (void)closesocket(TcpIp_SocketAdmin[SocketId].socketHandle);
                /** @req 4.2.2/SWS_TCPIP_00144 */
                /** @req 4.2.2/SWS_TCPIP_00224 */
                /* The TcpIp module shall indicate events related to sockets to
                the upper layer module by using the Up_TcpIpEvent API and the following events:
                TCPIP_TCP_RESET, TCPIP_TCP_CLOSED, TCPIP_TCP_FIN_RECEIVED and
                TCPIP_UDP_CLOSED. */
                if(Config->Config.SocketOwnerConfig.SocketOwnerList[TcpIp_SocketAdmin[SocketId].socketOwnerId].SocketOwnerTcpIpEventFncPtr != NULL){
                    Config->Config.SocketOwnerConfig.SocketOwnerList[TcpIp_SocketAdmin[SocketId].socketOwnerId].SocketOwnerTcpIpEventFncPtr(SocketId, TCPIP_TCP_CLOSED);
                }
                TcpIp_FreeUpSocket(SocketId);
            }else{
                /* Nothing received */
            }
            TcpIp_BufferFree(dataPtr);
        }else{
            TCPIP_DET_REPORTERROR(TCPIP_MAINFUNCTION_SERVICE_ID, TCPIP_E_NOBUFS);
            nBytes = 0;
        }
    }
}

static void TcpIp_HandleSocketStateUdpReady(TcpIp_SocketIdType SocketId)
{
    sint32 nBytes = 1;
    uint8 *dataPtr;
    struct sockaddr_in fromAddr;
    uint32 fromAddrLen = sizeof(fromAddr);
    TcpIp_SockAddrType remoteAddr;

    /* Note: Even it is not shown in the sequence diagram of section 9.3, TcpIp may
    decouple the data reception if required. E.g. for reassembling of incoming IP
    datagrams that are fragmented, TcpIp shall copy the received data to a TcpIp buffer
    and decouple TcpIp_RxIndication() from SoAd_RxIndication() */
    while(nBytes > 0){
        if (TRUE == TcpIp_BufferGet(TCPIP_RX_BUFFER_SIZE, &dataPtr)) {
            nBytes = recvfrom(TcpIp_SocketAdmin[SocketId].socketHandle, dataPtr, TCPIP_RX_BUFFER_SIZE, 0, (struct sockaddr*)&fromAddr, &fromAddrLen);/*lint !e929 !e740 */ /* pointer to pointer no harm */
            if (nBytes > 0){
                remoteAddr.domain = fromAddr.sin_family;
                remoteAddr.port = ntohs(fromAddr.sin_port);
                TcpIp_IpAddr32To8(fromAddr.sin_addr.s_addr, remoteAddr.addr);

                /* Call upper layer */
                /** @req 4.2.2/SWS_TCPIP_00223 */
                if(Config->Config.SocketOwnerConfig.SocketOwnerList[TcpIp_SocketAdmin[SocketId].socketOwnerId].SocketOwnerRxIndicationFncPtr != NULL){
                  Config->Config.SocketOwnerConfig.SocketOwnerList[TcpIp_SocketAdmin[SocketId].socketOwnerId].SocketOwnerRxIndicationFncPtr(SocketId, &remoteAddr, dataPtr, nBytes);
                }
            }
            TcpIp_BufferFree(dataPtr);
        }else{
            /** @req 4.2.2/SWS_TCPIP_00089 */
            TCPIP_DET_REPORTERROR(TCPIP_MAINFUNCTION_SERVICE_ID, TCPIP_E_NOBUFS);
            nBytes = 0;
        }
    }
}

/**
 * @brief TcpIp main function.
 * @param  void
 * @return void
 */
/** @req 4.2.2/SWS_TCPIP_00026 */
/** !req 4.2.2/SWS_TCPIP_00094 */
void TcpIp_MainFunction(void){
    for(uint32 CtrlIdx=0; CtrlIdx < TCPIP_NOF_CONTROLLERS; CtrlIdx++)
    {
        /** @req 4.2.2/SWS_TCPIP_00026 */
        switch (TcpIp_CtrlAdmin[CtrlIdx].tcpipEthIfCtrlState) {
            case TCPIP_STATE_OFFLINE:
                TcpIp_HandleStateOffline(CtrlIdx);
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

    for (uint32 i = 0; i < TCPIP_MAX_NOF_SOCKETS; i++) {
        switch (TcpIp_SocketAdmin[i].socketState) {

        case TCPIP_SOCKET_TCP_LISTENING:
            TcpIp_HandleSocketStateListening(i);
            if(TcpIp_SocketAdmin[i].socketHandle != -1){
                TcpIp_SocketStatusCheck(i);
            }
            break;

        case TCPIP_SOCKET_TCP_CONNECTING:
            TcpIp_HandleSocketStateConnecting(i);
            /* Status is checked withing state */
            break;

        case TCPIP_SOCKET_TCP_READY:
            TcpIp_HandleSocketStateTcpReady(i);
            if(TcpIp_SocketAdmin[i].socketHandle != -1){
            	TcpIp_SocketStatusCheck(i);
            }
            break;

        case TCPIP_SOCKET_UDP_READY:
            TcpIp_HandleSocketStateUdpReady(i);
            if(TcpIp_SocketAdmin[i].socketHandle != -1){
            	TcpIp_SocketStatusCheck(i);
            }
            break;

        default:
            /* Do nothing */
            break;
        }
    }
}

/*lint -restore*/ /* end for 9027 734 9045 */
