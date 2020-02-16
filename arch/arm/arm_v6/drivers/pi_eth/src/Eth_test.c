/*
 * eth_test.c
 *  this is a eth test file. God will help me on the way!
 *  I believe that.
 *  Created on: 2014-1-13
 *      Author: Zhang Shuzhou
 */

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "pi_eth.h"
#include "usb_memory.h"
#include "Platform_Types.h"
#include "usb_std_defs.h"
#include "Uart.h"
#include "bcm2835.h"

//temporary define
#define hs2net(x) (unsigned) ((((x)>>8) &0xff) | (((x) & 0xff)<<8))

#define ETH_ADDR_LEN 6
#define MAX_PAYLOAD  1516
#define ETH_TYPE_ARP 0x0806

#define ETH_PACKET   30



/* Simple ethernet packet structure */
struct etherGram
{
    char dst[ETH_ADDR_LEN];     /* Destination MAC */
    char src[ETH_ADDR_LEN];     /* Source MAC */
    ushort type_len;            /* EthernetII type/Ethernet length */
    char payload[1];            /* Payload data */
};


int Eth_test(void)
{
    boolean passed = TRUE;
    boolean subpass;
    uint memsize;
    int i, value, len;
    struct etherGram *inpkt;
    struct etherGram *outpkt;
    char *payload;
    char mymac[ETH_ADDR_LEN];
    char str[80];
    struct ether *peth = &ethertab[0];
    int result;
    //int dev = peth->dev->num;

    pi_printf("infor: ether test\r\n");
    /* memget */
    memsize = sizeof(struct etherGram) + MAX_PAYLOAD - 1;
    inpkt = memget(memsize);
    outpkt = memget(memsize);

    payload = &(outpkt->payload[0]);

    pi_printf("infor: get mac address\r\n");
    //control(dev, ETH_CTRL_GET_MAC, (long)mymac, 0);
    Eth_Control(ETH_CTRL_GET_MAC, (long)mymac, 0);
    memcpy(outpkt->dst, mymac, ETH_ADDR_LEN);
    memcpy(outpkt->src, mymac, ETH_ADDR_LEN);
    outpkt->type_len = hs2net(ETH_TYPE_ARP);

    pi_printf("infor: generate payload content\r\n");
    /* generate payload content */
    for (i = 0; i < MAX_PAYLOAD; i++)
    {
        /* Cycle through 0x20 to 0x7d (range of 0x5e) */
        value = (i % 0x5e) + 0x20;
        payload[i] = value;
    }

    /* place ether in loopback mode */
    pi_printf("infor: ether in loopback mode\r\n");
    result = Eth_Control(ETH_CTRL_SET_LOOPBK, TRUE, 0);
    /* flush any packets already received */
    while (peth->icount > 0)
    {
        Eth_Read(inpkt, memsize);
    }

    /* small packet (payload 16 bytes + 14 byte header) */
    pi_printf("infor: ether write data\r\n");
    for(i = 0; i < ETH_PACKET; i++){
    	pi_print(outpkt->payload[i],1);
    }
    len = Eth_Write(outpkt, ETH_PACKET);
    if ((len < ETH_PACKET)) {

		pi_printf("error: fail to write data\r\n");
		passed = 0;
	} else {
		pi_printf("infor: success to write data\r\n");
	}
//    for(int m = 0; m < 10; m ++){
//    	 bcm2835_delayMicroseconds(1000);
//    }


    bzero(inpkt, memsize);
    pi_printf("infor: ether read data\r\n");
    len = Eth_Read(inpkt, ETH_PACKET);
	if ((0 != memcmp(outpkt, inpkt, ETH_PACKET))) {

		pi_printf("error: fail to read data\r\n");
		passed = 0;
	} else {

		pi_printf("infor: succes to read data\r\n");
	}

	for(i = 0; i < ETH_PACKET; i++){
		pi_print(inpkt->payload[i],1);
	}
    /* ether out of loopback mode */
	//Eth_Control(ETH_CTRL_SET_LOOPBK, FALSE, 0);
    /* memfree */
    memfree(outpkt, memsize);
    memfree(inpkt, memsize);
    pi_printf("infor: ether test done\r\n");
    return passed;
}


int Eth_P2P_Sender(void) {
	boolean passed = TRUE;
	boolean subpass;
	uint memsize;
	int i, value, len;
	struct etherGram *inpkt;
	struct etherGram *outpkt;
	char *payload;
	char mymac[ETH_ADDR_LEN];
	char str[80];
	struct ether *peth = &ethertab[0];
	int result;
	//int dev = peth->dev->num;

	pi_printf("infor: ether sender test\r\n");
	/* memget */
	memsize = sizeof(struct etherGram) + MAX_PAYLOAD - 1;
	inpkt = memget(memsize);
	outpkt = memget(memsize);

	payload = &(outpkt->payload[0]);

	pi_printf("infor: get mac address\r\n");
	//control(dev, ETH_CTRL_GET_MAC, (long)mymac, 0);
	Eth_Control(ETH_CTRL_GET_MAC, (long) mymac, 0);
	memcpy(outpkt->dst, mymac, ETH_ADDR_LEN);
	memcpy(outpkt->src, mymac, ETH_ADDR_LEN);
	outpkt->type_len = hs2net(ETH_TYPE_ARP);

	pi_printf("infor: generate payload content\r\n");
	/* generate payload content */
	for (i = 0; i < MAX_PAYLOAD; i++) {
		/* Cycle through 0x20 to 0x7d (range of 0x5e) */
		value = (i % 0x5e) + 0x20;
		payload[i] = value;
	}

	/* place ether in loopback mode */
//	pi_printf("infor: ether in loopback mode\r\n");
//	result = Eth_Control(ETH_CTRL_SET_LOOPBK, TRUE, 0);
	/* flush any packets already received */
	while (peth->icount > 0) {
		Eth_Read(inpkt, memsize);
	}

	/* small packet (payload 16 bytes + 14 byte header) */
	pi_printf("infor: ether write data\r\n");
	for (i = 0; i < ETH_PACKET; i++) {
		pi_print(outpkt->payload[i], 1);
	}
	len = Eth_Write(outpkt, ETH_PACKET);
	if ((len < ETH_PACKET)) {

		pi_printf("error: fail to write data\r\n");
		passed = 0;
	} else {
		pi_printf("infor: success to write data\r\n");
	}


	/* ether out of loopback mode */
	//Eth_Control(ETH_CTRL_SET_LOOPBK, FALSE, 0);
	/* memfree */
	memfree(outpkt, memsize);
	memfree(inpkt, memsize);
	pi_printf("infor: ether sender done\r\n");
	return passed;
}


int Eth_P2P_Reader(void){
	boolean passed = TRUE;
	boolean subpass;
	uint memsize;
	int i, value, len;
	struct etherGram *inpkt;
	struct etherGram *outpkt;
	char *payload;
	char mymac[ETH_ADDR_LEN];
	char str[80];
	struct ether *peth = &ethertab[0];
	int result;
	//int dev = peth->dev->num;

	pi_printf("infor: ether reader test\r\n");
	/* memget */
	memsize = sizeof(struct etherGram) + MAX_PAYLOAD - 1;
	inpkt = memget(memsize);
	outpkt = memget(memsize);

	payload = &(outpkt->payload[0]);

	pi_printf("infor: get mac address\r\n");
	//control(dev, ETH_CTRL_GET_MAC, (long)mymac, 0);
	Eth_Control(ETH_CTRL_GET_MAC, (long) mymac, 0);
	memcpy(outpkt->dst, mymac, ETH_ADDR_LEN);
	memcpy(outpkt->src, mymac, ETH_ADDR_LEN);
	outpkt->type_len = hs2net(ETH_TYPE_ARP);

	pi_printf("infor: generate payload content\r\n");
	/* generate payload content */
	for (i = 0; i < MAX_PAYLOAD; i++) {
		/* Cycle through 0x20 to 0x7d (range of 0x5e) */
		value = (i % 0x5e) + 0x20;
		payload[i] = value;
	}

	/* place ether in loopback mode */
	/* flush any packets already received */
	while (peth->icount > 0) {
		Eth_Read(inpkt, memsize);
	}

	bzero(inpkt, memsize);
	pi_printf("infor: ether read data\r\n");
	len = Eth_Read(inpkt, ETH_PACKET);
	if ((0 != memcmp(outpkt, inpkt, ETH_PACKET))) {

		pi_printf("error: fail to read data\r\n");
		passed = 0;
	} else {

		pi_printf("infor: succes to read data\r\n");
	}

	for (i = 0; i < ETH_PACKET; i++) {
		pi_print(inpkt->payload[i], 1);
	}

	memfree(outpkt, memsize);
	memfree(inpkt, memsize);
	pi_printf("infor: ether reader done\r\n");
	return passed;
}
