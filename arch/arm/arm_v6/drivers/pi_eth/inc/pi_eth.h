/*
 * pi_eth.h
 *
 *  Created on: 2013-10-17
 *      Author: Zhang Shuzhou
 */

#ifndef PI_ETH_H_
#define PI_ETH_H_

#include "usb_util.h"
#include "Std_Types.h"
#include "usb_semaphore.h"

/*
 * network define
 *
 */


#define NETADDR_ETHERNET    1

#define NET_GET_MTU         200
#define NET_GET_LINKHDRLEN  201
#define NET_GET_HWADDR      203
#define NET_GET_HWBRC       204

#define NET_MAX_ALEN    6

struct netaddr
{
    ushort type;                      /**< Address type (NETADDR_*)     */
    uchar len;                        /**< Address length               */
    uchar addr[NET_MAX_ALEN];         /**< Address                      */
};

///////////////////////////////////////////////////////////////////////////////
#define ETH_HDR_LEN		    14
#define ETH_ADDR_LEN   		6   /*Length of ethernet address         */
#define NETHER              1

/* ETH Buffer lengths */
#define ETH_IBLEN           1024 /**< input buffer size                 */

/* Ethernet DMA buffer sizes */
#define ETH_MTU             1500 /**< Maximum transmission units        */
#define ETH_HEADER_LEN      ETH_HDR_LEN  /**< Length of Ethernet header */
#define ETH_VLAN_LEN        4   /**< Length of Ethernet vlan tag        */
#define ETH_CRC_LEN         4   /**< Length of Ethernet CRC             */
#define ETH_MAX_PKT_LEN     ( ETH_HEADER_LEN + ETH_VLAN_LEN + ETH_MTU )

/* ETH states */
#define ETH_STATE_FREE      0
#define ETH_STATE_DOWN      1
#define ETH_STATE_UP        2


/* ETH control codes */
#define ETH_CTRL_CLEAR_STATS 1  /**< Clear Ethernet Statistics          */
#define ETH_CTRL_SET_MAC     2  /**< Set the MAC for this device        */
#define ETH_CTRL_GET_MAC     3  /**< Get the MAC for this device        */
#define ETH_CTRL_SET_LOOPBK  4  /**< Set Loopback Mode                  */
#define ETH_CTRL_RESET       5  /**< Reset the Ethernet device          */
#define ETH_CTRL_DISABLE     6  /**< Disable the Ethernet device        */

/**
 * Ethernet packet buffer
 */
struct ethPktBuffer
{
    uchar *buf;                 /**< Pointer to buffer space            */
    uchar *data;                /**< Start of data within buffer        */
    int length;                 /**< Length of packet data              */
};

/************************************************************************
 * Ethernet control block
 ************************************************************************/
struct ether
{
    uint8 state;                /**< ETH_STATE_*above                   */
    //device *phy;                /**< physical eth device for Tx DMA     */
    /* Pointers to associated structures */
    //device *dev;                /**< eth device structure               */
    void *csr;                  /**< control and status registers       */

    uint32 interruptMask;        /**< interrupt mask                     */
    uint32 interruptStatus;      /**< interrupt status                   */

//    struct dmaDescriptor *rxRing; /**< array of receiving ring descs.   */
    struct ethPktBuffer **rxBufs; /**< Rx ring array                    */
    uint32 rxHead;               /**< Rx ring head index                 */
    uint32 rxTail;               /**< Rx ring tail index                 */
    uint32 rxRingSize;           /**< Number of Rx ring descriptors      */
    uint32 rxirq;                /**< Count of Rx interrupt requests     */
    uint32 rxOffset;             /**< Size in bytes of rxHeader          */
    uint32 rxErrors;             /**< Count of Rx errors.                */

//    struct dmaDescriptor *txRing; /**< array of transmit ring descs.    */
    struct ethPktBuffer **txBufs; /**< Tx ring array                    */
    uint32 txHead;               /**< Tx ring head index                 */
    uint32 txTail;               /**< Tx ring tail index                 */
    uint32 txRingSize;           /**< Number of Tx ring descriptors      */
    uint32 txirq;                /**< Count of Tx interrupt requests     */

    uint8 devAddress[ETH_ADDR_LEN];

    uint8 addressLength;        /**< Hardware address length            */
    uint16 mtu;                 /**< Maximum transmission units         */

    uint32 errors;               /**< Number of Ethernet errors          */
    uint16 ovrrun;              /**< Buffer overruns                    */
    semaphore isema;            /**< I/0 sem for eth input              */
    uint16 istart;              /**< Index of first byte                */
    uint16 icount;              /**< Packets in buffer                  */

    struct ethPktBuffer *in[ETH_IBLEN]; /**< Input buffer               */

    int inPool;                 /**< buffer pool id for input           */
    int outPool;                /**< buffer pool id for output          */
};

/* Global table of Ethernet devices.  */
extern struct ether ethertab[];


int Eth_Init(void);

int Eth_Open(void);

/**
 * \ingroup ether
 *
 * Write an Ethernet frame (excluding certain fields) to an Ethernet device.
 * This should be called through write().
 *
 * This function actually only buffers the frame to be sent at some later time.
 * Therefore, there is no guarantee the frame has actually been transmitted on
 * the wire when this function returns.
 *
 * @param devptr
 *      Pointer to the entry in Xinu's device table for the Ethernet device.
 * @param buf
 *      Buffer that contains the Ethernet frame to send.  It must start with the
 *      MAC destination address and end with the payload.
 * @param len
 *      Length, in bytes, of the Ethernet frame to send.
 *
 * @return
 *      ::SYSERR if packet is too small, too large, or the Ethernet device is
 *      not currently up; otherwise @p len, the number of bytes submitted to be
 *      written at some later time.
 */
int Eth_Write(const void *buf, uint len);

/**
 * \ingroup ether
 *
 * Read an Ethernet frame from an Ethernet device.  This should be called
 * through read().
 *
 * This function blocks until a frame has actually been received.  There is no
 * timeout.
 *
 * @param devptr
 *      Pointer to the entry in Xinu's device table for the Ethernet device.
 * @param buf
 *      Buffer in which to receive the Ethernet frame.  The received frame will
 *      start with the MAC destination address and end with the payload.
 * @param len
 *      Maximum length, in bytes, of the Ethernet frame to receive (size of @p
 *      buf).
 *
 * @return
 *      ::SYSERR if the Ethernet device is not currently up; otherwise the
 *      actual length of the Ethernet frame received and written to @p buf.
 */
int Eth_Read(void *buf, uint len);

/**
 * \ingroup ether
 *
 * Execute a control function on an Ethernet device.  This should be called
 * through control().
 *
 * @param req
 *      Control request to execute.
 * @param arg1
 *      First argument (if any) for the control request.
 * @param arg2
 *      Second argument (if any) for the control request.
 *
 * @return
 *      The result of the control request, or ::SYSERR if the control request
 *      @p req was not recognized.
 */
int Eth_Control(int req, long arg1, long arg2);

void randomEthAddr(uint8 addr[ETH_ADDR_LEN]);
//void Eth_Init( const Eth_ConfigType* CfgPtr );

int Eth_test(void);

int Eth_P2P_Sender(void);

int Eth_P2P_Reader(void);

#endif /* PI_ETH_H_ */
