/**
 * @file
 * Ethernet Interface Skeleton
 *
 */

/*
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

/*
 * This file is a skeleton for developing Ethernet network interface
 * drivers for lwIP. Add code to the low_level functions and do a
 * search-and-replace for the word "ethernetif" to replace it with
 * something that better describes your network interface.
 */

#include "lwip/opt.h"
#include "lwip/def.h"
#include "lwip/pbuf.h"
#include "lwip/sys.h"
#include <lwip/stats.h>
#include <lwip/snmp.h>
#include "netif/etharp.h"
#include "netif/ppp_oe.h"
#include "lwip/err.h"
#include "ethernetif.h"
#include "lwip/ethip6.h"
#include "lwip/tcpip.h"

#include "Eth.h"
#include "LwIpAdp.h"
#include <string.h>

#include "isr.h"
#include "irq_types.h"
#if defined(CFG_JAC6)
#include "Eth_jacinto.h"
#endif

#if defined(CFG_AURIX)
#include "Ifx_reg.h"
#if defined(CFG_DRIVE_PX_B00)
#include "Phy_Bcm89610.h"
#endif
#endif

/* TCP and ARP timeouts */
volatile int tcp_end_time, arp_end_time;

static uint8  phyAddress = 32u; /* Invalid address or Phy is not initialised in this context */
static boolean  ethCtrlInitRun = FALSE;
static boolean  ethTrcvInitRun = FALSE;


/* Define those to better describe your network interface. */
#define IFNAME0 's'
#define IFNAME1 't'

#define  ETH_HEADER_SIZE                 14u
#define  ETH_MAC_ADDR_SIZE               6u
#define  READ32(address)                 (*(vuint32_t *)(address))
#define  WRITE32(address, val)           (*(volatile uint32_t*)(address) = val)
#define  ETH_CTRL_INDEX                  0u /* Assign HW_UNIT_0 to this index */
#define  VLAN_HEADER_SIZE                4u


#define BMCR_REGISTER    			     0u
#define BMSR_REGISTER    			     1u
#define AUTONEG_ADVERTISE_REGISTER	     4u
#define PHY_SOFT_RESET                	 0x8000u
#define ENABLE_AUTO_NEGOTIATION          0x1000u
#define ENABLE_100MBPS_DUPLEX            0x2100u
#if defined(CFG_ETH_PHY_DP83848C)
#define PHY_STATUS_REGISTER     	     0x10u
#define PHY_STATUS_LINK                  0x0001u
#define PHY_STATUS_NO_AUTO_100           0x0005u
#define PHY_STATUS_LOOPBACK              0x0008u
#elif defined (CFG_ETH_PHY_TJA1100)
#define PHY_STATUS_REGISTER     	     0x01u
#define PHY_EXTENDED_STATUS_REGISTER     15u
#define PHY_EXTENDED_CONTROL_REGISTER    17u
#define PHY_CONFIG_REGISTER1             18u
#define PHY_STATUS_LINK                  0x0004u
#define PHY_STATUS_NO_AUTO_100           0xE600u /*100BASE-T4,100BASE-X_FD,100BASE-X_HD,100BASE-T2_FD,100BASE-T2_HD*/
#define PHY_EXTENDED_STATUS_NO_AUTO_100  0x0080u
#define PHY_CONFIG_EN_BIT                (2u)
#define PHY_CONFIG_MASTER_SLAVE_BIT      (15u)
#else /* CFG_ETH_PHY_DP83865 */
#if defined(CFG_DRIVE_PX_B00)
#define PHY_STATUS_REGISTER              0x01u
#else
#define PHY_STATUS_REGISTER     	     0x11u
#endif
#define PHY_STATUS_LINK                  0x0004u
#define PHY_STATUS_NO_AUTO_100           0x000Eu
#define PHY_STATUS_LOOPBACK              0x0040u
#define RGMII_INBAND_STATUS_EN           0x0100U
#endif

#ifdef CFG_BRD_ZYNQ_ZC702
#define PORT_MIO_52                      0xF80007D0
#define PORT_MIO_53                      0xF80007D4
#endif

#if defined(CFG_JAC6)
#define GMAC_SW_MDIO_LINKSEL             0x00000080U
#define GMAC_SW_MDIO_LINKINTENB          0x00000040U
#define GMAC_SW_MDIO_PHYADDRMON_MASK     0x0000001FU
#define ETH_CTRL_TX_FRAME_MIN_SIZE       60u
#define ETH_CTRL_PAD

#if defined(CFG_ETH_HW_SHARE)
#define VLAN_TCI                         (0x0042u)
#endif

static GmacSw_HwRegType * const hwPtr = (GmacSw_HwRegType*) GMAC_SW_BASE;
#endif

/* Max wait cycles for actions to occur in busy wait***** */
#define ETH_MAX_BUSY_WAIT_CYCLES                      (uint32)1000

#define ETH_MDIO_WAIT_CYCLES             50u

/* Macro for busy waiting for a while */
#define BUSY_WAIT_A_WHILE(_regPtr, _mask, _val, _retPtr)\
    *(_retPtr) = E_NOT_OK;\
    for( uint32 i = 0; i < ETH_MAX_BUSY_WAIT_CYCLES; i++) {\
        if( (*(_regPtr) & (_mask)) == (_val) ) {\
            *(_retPtr) = E_OK;\
            break;\
        }\
    }\

#define ETH_LINK_CHECK  /* internal defines for the internal config and test */

LinkState  linkStatus = NO_LINK;

/* Forward declarations. */
static err_t low_level_output(struct netif *netif, struct pbuf *p);
static uint8 Eth_TransHw_findPhyAdrs(uint8 ctrlIdx,uint8 startPhyAddr, uint8 endPhyAddr);
static Std_ReturnType Eth_TransHw_init(uint8 ctrlIdx);

static struct netif *netIfPtr = NULL;

/**
  * @brief  Mock up function for ETH_IF : Frames reception routine
  * Call recurrently, Disable RX interrupt
  * @param  None
  * @retval None
  */
void Ethernetif_RxFunc(void)
{
    if((ethCtrlInitRun == FALSE) || (ethTrcvInitRun == FALSE)){
        return;
    }
#if defined(CFG_JAC6) || (CFG_AURIX)
    Eth_TransHw_linkUp(ETH_CTRL_INDEX);
#endif
    if(linkStatus == LINK_UP){
#if defined(CFG_RH850)
        Eth_Receive(ETH_CTRL_INDEX);
#else
#if defined(CFG_JAC6) && defined(CFG_ETH_HW_SHARE) /* Interrupt mode is not sharable */
        Eth_TxConfirmation(ETH_CTRL_INDEX);
#endif
        Eth_RxStatusType rxStatus; /* unused */
        Eth_Receive(ETH_CTRL_INDEX,&rxStatus);
        (void)rxStatus;
#endif
    }

}


/**
  * @brief  Mock up function for ETH_IF : Frames reception function
  * @param  --
  * @retval None
  */
void EthIf_RxIndication( uint8 CtrlIdx, Eth_FrameType FrameType, boolean IsBroadcast, uint8* PhysAddrPtr, Eth_DataType* DataPtr, uint16 LenByte )
{
    struct pbuf *p;
    uint8 macAdrs[ETH_MAC_ADDR_SIZE] = ETH_MAC_ADDR;
    uint8 macBroadcastAdrs[ETH_MAC_ADDR_SIZE] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
    uint16 AdaptFrameType;
    uint16 AdaptFrameType1;

#if ETH_PAD_SIZE
    len += ETH_PAD_SIZE; /* allow room for Ethernet padding */
#endif
    p = pbuf_alloc(PBUF_RAW, LenByte+ETH_HEADER_SIZE, PBUF_POOL);
    if (p != NULL){

#if ETH_PAD_SIZE
    pbuf_header(p, -ETH_PAD_SIZE); /* drop the padding word */
#endif
       if(IsBroadcast == TRUE){
          MEMCPY((uint8*)p->payload,macBroadcastAdrs,ETH_MAC_ADDR_SIZE);
       }else{
          MEMCPY((uint8*)p->payload,macAdrs,ETH_MAC_ADDR_SIZE);
       }
       MEMCPY((uint8*)p->payload+ETH_MAC_ADDR_SIZE,PhysAddrPtr,ETH_MAC_ADDR_SIZE);
#if defined(CFG_ZYNQ)  || defined(CFG_JAC6) || defined(CFG_TC29X)
       AdaptFrameType =  (uint16)(FrameType >> 8 )  + (uint16)(FrameType << 8);
       MEMCPY((uint8*)p->payload+(2*ETH_MAC_ADDR_SIZE),&AdaptFrameType,sizeof(uint16));
       if(AdaptFrameType == PP_HTONS(ETHTYPE_VLAN)){
           AdaptFrameType1 =  (uint16)(DataPtr[2]) + (uint16)(DataPtr[3] << 8);
           MEMCPY((uint8*)p->payload+(2*ETH_MAC_ADDR_SIZE),&AdaptFrameType1,sizeof(uint16));
       }
#else
       MEMCPY((uint8*)p->payload+(2*ETH_MAC_ADDR_SIZE),&FrameType,sizeof(uint16));
#endif
       if(AdaptFrameType != PP_HTONS(ETHTYPE_VLAN)){
           MEMCPY((uint8*)p->payload+ETH_HEADER_SIZE,DataPtr,LenByte);
       }else{
           MEMCPY((uint8*)p->payload+ETH_HEADER_SIZE,&DataPtr[VLAN_HEADER_SIZE],(LenByte-VLAN_HEADER_SIZE));
       }
       if(netIfPtr != NULL){
           tcpip_input(p, netIfPtr);
       }
    }else{
        /* Out of memory, Nothing to do but to through away data */
        return;
    }

}

/**
  * @brief  Mock up function for ETH_IF : Transmit conmfirmation
  * Doing nothing currently
  * @param  None
  * @retval None
  */
void EthIf_TxConfirmation( uint8 CtrlIdx, uint8 BufIdx )
{
    /* done */

}

void LwIP_EthernetIfStart(struct netif *netIf)
{
    netIfPtr = netIf;
}

/**
 * In this function, the hardware should be initialized.
 * Called from ethernetif_init().
 *
 * @param netif the already initialized lwip network interface structure
 *        for this ethernetif
 */
static void
low_level_init(struct netif *netif)
{
  /* set MAC hardware address length */
  netif->hwaddr_len = ETHARP_HWADDR_LEN;

  uint8_t MACaddr[6] = ETH_MAC_ADDR;

  netif->hwaddr[0] =  MACaddr[0];
  netif->hwaddr[1] =  MACaddr[1];
  netif->hwaddr[2] =  MACaddr[2];
  netif->hwaddr[3] =  MACaddr[3];
  netif->hwaddr[4] =  MACaddr[4];
  netif->hwaddr[5] =  MACaddr[5];

  /* maximum transfer unit */
  netif->mtu = 1500;

  /* device capabilities */
  /* don't set NETIF_FLAG_ETHARP if this device is not an ethernet one */
  netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP | NETIF_FLAG_IGMP;
}

void ethernetif_pre_tcpip_init(void){
    uint8_t MACaddr[6] = ETH_MAC_ADDR;

#ifdef CFG_BRD_ZYNQ_ZC702
   /* IMPROVE - port driver change is required*/
   WRITE32(PORT_MIO_52,0x00001280);
   WRITE32(PORT_MIO_53,0x00001280);
#endif

#if defined(CFG_DRIVE_PX_B00)
   Phy_Bcm89610_Enable();
#endif
   if(E_OK == Eth_ControllerInit(ETH_CTRL_INDEX,0)){
       ethCtrlInitRun = TRUE;
   }

#if defined(CFG_ZYNQ)  || defined(CFG_JAC6)
#if (ETH_PHYS_ADRS_FILTER_API == STD_ON)
  Eth_UpdatePhysAddrFilter(ETH_CTRL_INDEX,MACaddr,ETH_ADD_TO_FILTER);
#endif
#endif

#ifdef CHECKSUM_BY_HARDWARE
 /* Do nothing at the moment */
#endif
//  Eth_SetControllerMode(ETH_CTRL_INDEX,ETH_MODE_ACTIVE);

#if defined(CFG_DRIVE_PX_B00)
  Phy_Bcm89610_Configure();
  ethTrcvInitRun = TRUE;
#else
#if defined(CFG_JAC6) &&  (LWIP_TRCV_ADRS != 32u)
  phyAddress = LWIP_TRCV_ADRS;
#else
  phyAddress =  Eth_TransHw_findPhyAdrs(ETH_CTRL_INDEX,0,31);
#endif
  if(phyAddress != 32u){
     (void)Eth_TransHw_init(ETH_CTRL_INDEX);
  }
#endif
}
/**
 * Should be called at the beginning of the program to set up the
 * network interface. It calls the function low_level_init() to do the
 * actual setup of the hardware.
 *
 * This function should be passed as a parameter to netif_add().
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return ERR_OK if the loopif is initialized
 *         ERR_MEM if private data couldn't be allocated
 *         any other err_t on error
 */
err_t
ethernetif_init(struct netif *netif)
{
    LWIP_ASSERT("netif != NULL", (netif != NULL));

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
    netif->linkoutput = low_level_output;
#if LWIP_IPV6
    netif->output_ip6 = ethip6_output;
#endif /* LWIP_IPV6 */

    /* initialize the hardware */
    low_level_init(netif);

    ethernetif_pre_tcpip_init();

    LwIP_EthernetIfStart(netif);

    return ERR_OK;
}

/**
 * This function should do the actual transmission of the packet. The packet is
 * contained in the pbuf that is passed to the function. This pbuf
 * might be chained.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @param p the MAC packet to send (e.g. IP packet including MAC addresses and type)
 * @return ERR_OK if the packet could be sent
 *         an err_t value if the packet couldn't be sent
 *
 * @note Returning ERR_MEM here if a DMA queue of your MAC is full can lead to
 *       strange results. You might consider waiting for space in the DMA queue
 *       to become availale since the stack doesn't retry to send a packet
 *       dropped because of memory failure (except for the TCP timers).
 */
static err_t
low_level_output(struct netif *netif, struct pbuf *p)
{
  struct pbuf *q;
  int l = 0;
  uint8 *buffer;
  uint32 baseAddress;
  uint32_t cnt = 0;
  Eth_BufIdxType bufIndex = 0;
  uint16 lenByte = 0;
  Eth_FrameType frametype;
  BufReq_ReturnType bufRes;
  Std_ReturnType txRes;
  bufRes = Eth_ProvideTxBuffer(ETH_CTRL_INDEX, &bufIndex,(Eth_DataType **) &baseAddress, &lenByte);
  if(bufRes != BUFREQ_OK){
      LWIP_DEBUGF(NETIF_DEBUG, ("send: buffer not available\n"));
      /* Try to increase the number of ETH TX buffers */
      return ERR_MEM;
  }

#if ETH_PAD_SIZE
    pbuf_header(p, -ETH_PAD_SIZE); /* drop the padding word */
#endif
    buffer = (u8_t*)(baseAddress-ETH_HEADER_SIZE);

#if defined(CFG_JAC6) && defined(CFG_ETH_HW_SHARE)
    for(q = p; q != NULL; q = q->next)
    {
        if(l == 0u){
            SMEMCPY((u8_t*)buffer, q->payload, (ETH_MAC_ADDR_SIZE*2));
            buffer[12] = (uint8)((ETHTYPE_VLAN >> 8u) & 0xFFu); /* 0x8100 */
            buffer[13] = (uint8) (ETHTYPE_VLAN & 0xFFu);
            buffer[14] = (uint8)((VLAN_TCI >> 8u) & 0xFFu);
            buffer[15] = (uint8) (VLAN_TCI & 0xFFu);
            SMEMCPY((u8_t*)&buffer[(ETH_MAC_ADDR_SIZE*2)+VLAN_HEADER_SIZE], (q->payload+(ETH_MAC_ADDR_SIZE*2)), (q->len-(ETH_MAC_ADDR_SIZE*2)));
            l = ((q->len) + VLAN_HEADER_SIZE);
        }else{
            SMEMCPY((u8_t*)&buffer[l], q->payload, q->len);
            l = l + q->len;
        }
    }
#else

  for(q = p; q != NULL; q = q->next)
  {
    SMEMCPY((u8_t*)&buffer[l], q->payload, q->len);
    l = l + q->len;
  }
#endif
  /* Try to send */
  cnt = 0;
#if defined(CFG_ZYNQ) || defined(CFG_JAC6) || defined(CFG_TC29X)
  frametype = ((buffer[12] << 8) + buffer[13] );
#else
  frametype = ((buffer[13] << 8) + buffer[12] );
#endif

#if defined(ETH_CTRL_PAD)
  if(l < ETH_CTRL_TX_FRAME_MIN_SIZE){
      for(uint8 i = l; i < ETH_CTRL_TX_FRAME_MIN_SIZE; i++){
          buffer[i] = 0u;
      }
      l = ETH_CTRL_TX_FRAME_MIN_SIZE;
  }
#endif
  txRes = Eth_Transmit(ETH_CTRL_INDEX,bufIndex,frametype,TRUE,(l-ETH_HEADER_SIZE),buffer);
  while(txRes == E_PENDING)
  {
    /* TXBD_READY bit set. Loop here and wait since frames will not be resend */
    if(!(cnt % 500)){
        Eth_TxConfirmation(ETH_CTRL_INDEX); /* only in POLLING mode */
        txRes = Eth_Transmit(ETH_CTRL_INDEX,bufIndex,frametype,TRUE,(l-ETH_HEADER_SIZE),buffer);
    }
    cnt++;
    if(cnt > 1000000)
    {
        /* timeout, notify error and return */

        LWIP_DEBUGF(NETIF_DEBUG, ("send: not ready\n"));
        return ERR_TIMEOUT;
    }
  }
#if ETH_PAD_SIZE
      pbuf_header(p, ETH_PAD_SIZE); /* reclaim the padding word */
#endif

  /* Return SUCCESS */
    return ERR_OK;
}

/**
 * Fetch PHY address:: mock up function for ETH tranciever
 * @param startPhyAddr
 * @param endPhyAddr
 * @return phyAddr
 */
uint8 Eth_TransHw_findPhyAdrs(uint8 ctrlIdx, uint8 startPhyAddr, uint8 endPhyAddr)
{
    uint8 phy_addr;
      for (phy_addr = 0; phy_addr <= endPhyAddr; phy_addr++) {
#if !defined(CFG_JAC6)
          uint8 Status;
          uint16 Reg1;
          uint16 Reg2;
#if defined (CFG_AURIX)
          Eth_ReadMii(ctrlIdx,phy_addr, 2, &Reg1);
          Eth_ReadMii(ctrlIdx,phy_addr,3, &Reg2); /* change it */
          Status= ETH_OK;
#else
        Status =  Eth_ReadMii(ctrlIdx,phy_addr, 2, &Reg1);
        Status |= Eth_ReadMii(ctrlIdx,phy_addr,3, &Reg2); /* change it */
#endif
          if ((Status == ETH_OK) &&
              (Reg1 > 0x0000) && (Reg1 < 0xffff) &&
              (Reg2 > 0x0000) && (Reg2 < 0xffff)) {
              return phy_addr;
          }
#else
          for( uint32 i = 0; i < ETH_MDIO_WAIT_CYCLES; i++); /* Provide some time for MDIO module to find the PHYs available */
          if(hwPtr->MDIO.MDIO_ALIVE & (0x1u << phy_addr)){
              return phy_addr;
          }
#endif
      }
      return phy_addr;
}

/**
 * Link Setting up - Needed to be called from a cyclic routine
 * precondition - phyAddress is valid
 * @param ctrlIdx
 * @return
 */
void Eth_TransHw_linkUp(uint8 ctrlIdx)
{
    if(phyAddress != 32u){
        uint16 statusValue = 0;
#if defined(CFG_JAC6) || defined(CFG_TC29X)
        Eth_ReadMii(ctrlIdx,phyAddress,PHY_STATUS_REGISTER,&statusValue);
        if((statusValue & PHY_STATUS_LINK ) != 0){
            linkStatus = LINK_UP;
        }else{
            linkStatus = LINK_DOWN;
        }
        LwIpAdp_LinkStateUpdate(linkStatus);
#else
        uint16 regValue;
        if(linkStatus == NO_LINK)
        {
            /* Set auto neg advert register */
            (void)Eth_WriteMii(ctrlIdx,phyAddress, AUTONEG_ADVERTISE_REGISTER, 0x01e1);
            // Enable and start auto-negotiation
            Eth_ReadMii(ctrlIdx,phyAddress,BMCR_REGISTER, &regValue);
            Eth_WriteMii(ctrlIdx,phyAddress,BMCR_REGISTER, (regValue | 0x1200));
            linkStatus = AUTONEGOTIATE_RESTART;
        }
        // Wait for Auto-Neg complete flag
        Eth_ReadMii(ctrlIdx,phyAddress,BMSR_REGISTER,&statusValue);
        if((statusValue & 0x0020 )== 0x0020){
            linkStatus = AUTONEGOTIATE_COMPLETE;
        }
        Eth_ReadMii(ctrlIdx,phyAddress,PHY_STATUS_REGISTER,&statusValue);
        if((statusValue & PHY_STATUS_LINK ) != 0){
            linkStatus = LINK_UP;
        }else{
            linkStatus = LINK_DOWN;
        }
        LwIpAdp_LinkStateUpdate(linkStatus);
#endif
    }else{
        phyAddress =  Eth_TransHw_findPhyAdrs(ETH_CTRL_INDEX,0,31);
        if(phyAddress != 0x00){
             (void)Eth_TransHw_init(ETH_CTRL_INDEX);
        }
    }
}

/**
  * @brief  Mock up function for ETH Tranceiver : Initialise tranciever required
  * @param  ctrlIdx
  * @retval Std_ReturnType
  */
Std_ReturnType Eth_TransHw_init(uint8 ctrlIdx)
{
    if(ethCtrlInitRun != TRUE){ /* Tranciever is initialised after Eth controller initialisation */
        return E_NOT_OK;
    }
#if defined(CFG_ETH_PHY_DP83848C) || defined(CFG_ETH_PHY_DP83865) || defined(CFG_ETH_PHY_TJA1100)
    uint16 phyStatus;
    uint16 timeout;
#if defined(CFG_ETH_PHY_TJA1100)
    uint16 extdPhyStatus;
#endif
    uint16 regValue;
    /* Wait for completion */
    timeout = 10000;
    /*  Reset phy */
    (void)Eth_WriteMii(ctrlIdx,phyAddress, BMCR_REGISTER, PHY_SOFT_RESET);
    for(volatile uint16 i=0; i < 1000; i++){};

#if	defined(CFG_JAC6)  /*  ETH_NO_AUTO_NEG */
    hwPtr->MDIO.MDIO_USERPHYSEL0 |=  GMAC_SW_MDIO_LINKSEL;
    hwPtr->MDIO.MDIO_USERPHYSEL0 |= (GMAC_SW_MDIO_PHYADDRMON_MASK & phyAddress);

    (void)Eth_ReadMii (ctrlIdx,phyAddress, BMCR_REGISTER, &regValue);
    regValue &= ~ENABLE_AUTO_NEGOTIATION; // disable AN
    (void)Eth_WriteMii(ctrlIdx,phyAddress, BMCR_REGISTER, (regValue | ENABLE_100MBPS_DUPLEX));
    for(volatile uint16 i=0; i < ETH_MDIO_WAIT_CYCLES; i++){};
    do
    {
        for(volatile uint16 i=0; i < ETH_MDIO_WAIT_CYCLES; i++){};
        if ((timeout--) == 0) {
            return E_NOT_OK;
        }
        Eth_ReadMii(ctrlIdx,phyAddress, PHY_STATUS_REGISTER, &phyStatus);
#if defined (CFG_ETH_PHY_TJA1100)
        Eth_ReadMii(ctrlIdx,phyAddress, PHY_EXTENDED_STATUS_REGISTER, &extdPhyStatus);
#endif
    }while
#if !defined (CFG_ETH_PHY_TJA1100)
    ((phyStatus & PHY_STATUS_NO_AUTO_100) != PHY_STATUS_NO_AUTO_100);
#else
    (((phyStatus & PHY_STATUS_NO_AUTO_100) == 0u) && ((extdPhyStatus & PHY_EXTENDED_STATUS_NO_AUTO_100) != PHY_EXTENDED_STATUS_NO_AUTO_100));
#endif

#else //CFG_JAC6(ETH_NO_AUTO_NEG) end

#ifdef ETH_LINK_CHECK
    Eth_TransHw_linkUp(ctrlIdx);
#else
    /* Set auto neg advert register */
    (void)Eth_WriteMii(ctrlIdx,phyAddress, AUTONEG_ADVERTISE_REGISTER, 0x01e1);

    /* enable auto-negotiation */
    (void)Eth_ReadMii (ctrlIdx,phyAddress, BMCR_REGISTER, &regValue);
    (void)Eth_WriteMii(ctrlIdx,phyAddress, BMCR_REGISTER, (regValue | 0x1200));


    do {
        for(volatile uint16 i=0; i < 1000; i++){
        }

        if ((timeout--) == 0) {
            return E_NOT_OK;
        }

        if (Eth_ReadMii(ctrlIdx,phyAddress, BMSR_REGISTER, &phyStatus) != 0) {
            return E_NOT_OK;
        }
        if(phyStatus == 0xffff){
            return E_NOT_OK;
        }
    } while (!(phyStatus & 0x0020)); // Should be 0x786D
#endif //ETH_LINK_CHECK end
#endif /* Auto negotiate end */


#ifdef ETH_PHYLOOPBACK /* physical line test */

    (void)Eth_ReadMii (ctrlIdx,phyAddress, BMCR_REGISTER, &regValue);
    (void)Eth_WriteMii(ctrlIdx,phyAddress, BMCR_REGISTER, (regValue | 0x6100)); // loopback with speed 100
    do
    {
        for(volatile uint16 i=0; i < 1000; i++){};
        if ((timeout--) == 0) {
            return E_NOT_OK;
        }
        Eth_ReadMii(ctrlIdx,phyAddress, PHY_STATUS_REGISTER, &phyStatus);
    }while (!(phyStatus & PHY_STATUS_LOOPBACK));
#endif                   /* physical line test */

#else /* other phy */
    /* PHY device - MARVELL-88E1111 */
    #define RGMII_TXRX_CLK_DELAY    	0x0030
    #define ASYMMETRIC_PAUSE    		0x0800
    #define PAUSE   					0x0400
    #define LINKSPEED_1000MBPS		    0x0040
    #define LINKSPEED_100MBPS			0x2000
    #define LINKSPEED_10MBPS			0x0000

    uint16 Val = 0;
    uint32_t i;
    enum{
       ETH_SPEED_10MBPS,
       ETH_SPEED_100MBPS,
       ETH_SPEED_1000MBPS
    };
    uint16 speed = ETH_SPEED_100MBPS;


    Eth_WriteMii(ctrlIdx,phyAddress, 22, 2); //page adres
    Eth_ReadMii(ctrlIdx,phyAddress,21, &Val);
    Val |= RGMII_TXRX_CLK_DELAY;
    Eth_WriteMii(ctrlIdx,phyAddress, 21, Val);

    Eth_WriteMii(ctrlIdx,phyAddress, 22, 0 );// page adrs

    Eth_ReadMii(ctrlIdx,phyAddress, AUTONEG_ADVERTISE_REGISTER, &Val);
    Val |= RGMII_TXRX_CLK_DELAY;/* ASYMMETRIC_PAUSE ?? */
    Val |= PAUSE;
    Eth_WriteMii(ctrlIdx,phyAddress, AUTONEG_ADVERTISE_REGISTER, Val);

    Eth_ReadMii(ctrlIdx, phyAddress, BMCR_REGISTER, &Val);
    Val &= ~LINKSPEED_1000MBPS;
    Val &= ~LINKSPEED_100MBPS;
    Val &= ~LINKSPEED_10MBPS;

    if(speed == ETH_SPEED_100MBPS)	{
        Val |=  LINKSPEED_100MBPS;
    }
    else if (speed == ETH_SPEED_1000MBPS)    {
        Val |=  LINKSPEED_1000MBPS;
    }
    else {
        Val |=  LINKSPEED_10MBPS;
    }

    Eth_WriteMii( ctrlIdx,phyAddress, 9, 0); /* Dont advertise PHY speed of 1000 Mbps */
    Eth_WriteMii(ctrlIdx,phyAddress, AUTONEG_ADVERTISE_REGISTER,  0x0100 |0x0080); // don't advertise 100mbps full duplex and 100mbps half resp

    Eth_WriteMii(ctrlIdx,phyAddress,BMCR_REGISTER, Val | 0x8000); // reset mask added
    for ( i=0; i < 100000; i++);
#endif
#if 0 /* Speed is set statically and configured */
    Eth_Hw_SetSpeed(speed);
#endif
    linkStatus = LINK_UP;
    LwIpAdp_LinkStateUpdate(LINK_UP);
    ethTrcvInitRun = TRUE;
    return E_OK;
}

void EthIf_GetPhysAddr( uint8 CtrlIdx, uint8* PhysAddrPtr ){
    uint8 macAddr[6] = ETH_MAC_ADDR;
    PhysAddrPtr[0] = macAddr[0];
    PhysAddrPtr[1] = macAddr[1];
    PhysAddrPtr[2] = macAddr[2];
    PhysAddrPtr[3] = macAddr[3];
    PhysAddrPtr[4] = macAddr[4];
    PhysAddrPtr[5] = macAddr[5];
}

Std_ReturnType EthIf_UpdatePhysAddrFilter( uint8 CtrlIdx, const uint8* PhysAddrPtr, Eth_FilterActionType Action ){
    return E_NOT_OK;
}

