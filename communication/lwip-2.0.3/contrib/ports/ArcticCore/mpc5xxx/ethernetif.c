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

#include <string.h>

#include "isr.h"
#include "irq_types.h"
#include "mpc55xx.h"
#include "fec_5xxx.h"
#include "LwIpAdp.h"

/* TCP and ARP timeouts */
volatile int tcp_end_time, arp_end_time;

/* Define those to better describe your network interface. */
#define IFNAME0 's'
#define IFNAME1 't'

#define  ETH_ERROR              ((uint32_t)0)
#define  ETH_SUCCESS            ((uint32_t)1)

/* Forward declarations. */
static err_t low_level_output(struct netif *netif, struct pbuf *p);

static struct netif *netIfPtr = NULL;

/* Eth Isr routine */
static void Eth_Isr(void)
{
    uint32_t res = 0;
    uint32 ievent;

    /* Check events */
    ievent = FEC.EIR.R;

#if 0 /* error handling is not done , disable this piece of code in ISR */
    if (ievent & (BABBLINGTRANSMIT | ETHBUS_ERROR | LATECOLLISION |
                  COLLISIONRETRYLIMIT | TRANSMITFIFOUNDERRUN)) {
        /* transmit errors */
        error = ievent;
    }
    if (ievent & HEARTBEAT_ERROR) {
        /* Heartbeat error */
        error = ievent;
    }
    if (ievent & GRACEFULSTOPACK) {
        /* Graceful stop complete */
        error = ievent;
    }
#endif

    while(fec_is_rx_data_available() &&  (res == 0))
    {
          /* move received packet into a new pbuf */
          struct pbuf *p = low_level_input();

          if(p!=NULL){
              if(netIfPtr != NULL){
                  /* TODO support multiple ip addresses on the same netif? */
                  tcpip_input(p, netIfPtr);
              }
          }
          else{
              res = 1;
          }
    }

    /* Clear events */
    FEC.EIR.R = ievent;
#if defined(CFG_MPC5777M)
    FEC.RDAR.B.RDAR = 1;
#else
    FEC.RDAR.B.R_DES_ACTIVE = 1;
#endif
}

ISR(Eth_Isr_RXF){Eth_Isr();}
ISR(Eth_Isr_TXF){Eth_Isr();}
ISR(Eth_Isr_FEC){Eth_Isr();}

void LwIP_EthernetIfStart(struct netif *netIf)
{
    netIfPtr = netIf;
    fec_enable_reception();
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

    /* Create isr for ethernet interrupt */
    ISR_INSTALL_ISR2( "FecRXInt", Eth_Isr_RXF, FEC_RX, 3, 0 );
    ISR_INSTALL_ISR2( "FecTXInt", Eth_Isr_TXF, FEC_TX, 3, 0 );
    ISR_INSTALL_ISR2( "FecWorldInt", Eth_Isr_FEC, FEC_WORLD, 3, 0 );

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

    fec_init(MACaddr);
}

void ethernetif_pre_tcpip_init(void){
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
    uint32_t cnt = 0;
    struct pbuf *q;
    int l = 0;
    uint8_t *buffer;

    do{
        buffer = fec_get_buffer_to_send();

        /* TXBD_READY bit set. Loop here and wait since frames will not be resend */
        cnt++;
        if(cnt > 1000000)
        {
            /* timeout, notify error and return */
            LWIP_DEBUGF(NETIF_DEBUG, ("fec_send: TXBD_READY set\n"));
            return ERR_MEM;
        }
    }while(buffer == NULL);

#if ETH_PAD_SIZE
    pbuf_header(p, -ETH_PAD_SIZE); /* drop the padding word */
#endif

    for(q = p; q != NULL; q = q->next)
    {
        memcpy((uint8_t*)&buffer[l], q->payload, q->len);
        l = l + q->len;
    }

    /* Try to send */
    while(fec_send(buffer, l) != 0)
    {
        /* TXBD_READY bit set. Loop here and wait since frames will not be resend */
        cnt++;
        if(cnt > 1000000)
        {
            /* timeout, notify error and return */
            LWIP_DEBUGF(NETIF_DEBUG, ("fec_send: TXBD_READY set\n"));
            return ERR_MEM;
        }
    }

#if ETH_PAD_SIZE
    pbuf_header(p, ETH_PAD_SIZE); /* reclaim the padding word */
#endif

    /* Return SUCCESS */
    return ERR_OK;
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

