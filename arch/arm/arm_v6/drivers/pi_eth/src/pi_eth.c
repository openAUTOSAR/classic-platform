/*
 * pi_eth.c
 *
 *  Created on: 2013-10-17
 *      Author: Zhang Shuzhou
 */
#include <string.h>
#include "Os.h"
#include "Cpu.h"
#include "Std_Types.h"
#include "bcm2835.h"
#include "usb_semaphore.h"
#include "usb_core_driver.h"
#include "usb_memory.h"
#include "smsc9512.h"
#include "pi_eth.h"
#include "Uart.h"

#include "ethernetif.h"

uint32_t nr_copies = 0; //temp
uint32_t frameLengthGlobal = 0; //Temp
uint32_t packetLength = 0; //temp
boolean ETH_READ_FLAG = FALSE;
boolean ETH_ATTACHED_FLAG = false;
boolean ETH_OPEN_FLAG = false;

/* Global table of Ethernet devices.  */
struct ether ethertab[NETHER];

/**
 * Semaphores that indicate whether each given Ethernet device has actually been
 * connected to the system yet (here, attached to the USB).  Could be moved into
 * <code>struct ::ether</code> if other drivers needed it...
 */
static semaphore smsc9512_attached[NETHER];

/**
 * Try to bind the SMSC LAN9512 driver to a specific USB device.  This the @ref
 * usb_device_driver::bind_device "bind_device" implementation for the SMSC
 * LAN9512 driver and therefore complies with its documented behavior.
 */
static usb_status_t smsc9512_bind_device(struct usb_device *udev) {
	struct ether *ethptr;
	usb_status_t status;
//	pi_printf("infor: start smsc9512 bind\r\n");
	/* Check if this is actually a SMSC LAN9512 by checking the USB device's
	 * standard device descriptor, which the USB core already read into memory.
	 * Also check to make sure the expected endpoints for sending/receiving
	 * packets are present and that the device is operating at high speed.  */
	if (udev->descriptor.idVendor != SMSC9512_VENDOR_ID
			|| udev->descriptor.idProduct != SMSC9512_PRODUCT_ID
			|| udev->interfaces[0]->bNumEndpoints < 2
			|| (udev->endpoints[0][0]->bmAttributes & 0x3)
					!= USB_TRANSFER_TYPE_BULK
			|| (udev->endpoints[0][1]->bmAttributes & 0x3)
					!= USB_TRANSFER_TYPE_BULK
			|| (udev->endpoints[0][0]->bEndpointAddress >> 7)
					!= USB_DIRECTION_IN
			|| (udev->endpoints[0][1]->bEndpointAddress >> 7)
					!= USB_DIRECTION_OUT || udev->speed != USB_SPEED_HIGH) {
		//pi_printf("error: fail to ready smsc9512\r\n");
		return USB_STATUS_DEVICE_UNSUPPORTED;
	}

	/* Make sure this driver isn't already bound to a SMSC LAN9512.
	 * TODO: Support multiple devices of this type concurrently.  */
	ethptr = &ethertab[0];
	STATIC_ASSERT(NETHER == 1);
	if (ethptr->csr != NULL ) {
		pi_printf("error: fail to support smsc9512\r\n");
		return USB_STATUS_DEVICE_UNSUPPORTED;
	}

	/* The rest of this function is responsible for making the SMSC LAN9512
	 * ready to use, but not actually enabling Rx and Tx (which is done in
	 * etherOpen()).  This primarily involves writing to the registers on the
	 * SMSC LAN9512.  But these are not memory mapped registers, as this is a
	 * USB Ethernet Adapter that is attached on the USB!  Instead, registers are
	 * read and written using USB control transfers.  It's somewhat of a pain,
	 * and also unlike memory accesses it is possible for USB control transfers
	 * to fail.  However, here we perform lazy error checking where we just do
	 * all the needed reads and writes, then check at the end if an error
	 * occurred.  */

	udev->last_error = USB_STATUS_SUCCESS;

	/* Resetting the SMSC LAN9512 via its registers should not be necessary
	 * because the USB code already performed a reset on the USB port it's
	 * attached to.  */

	/* Set MAC address.  */
//	pi_printf("infor: set mac address\r\n");
	status = smsc9512_set_mac_address(udev, ethptr->devAddress);
	if (status != USB_STATUS_SUCCESS) {
		pi_printf("error: fail to set smsc9512 mac address\r\n");
	}

	/* Allow multiple Ethernet frames to be received in a single USB transfer.
	 * Also set a couple flags of unknown function.  */
//	pi_printf("infor: set a single usb traf\r\n");
	status = smsc9512_set_reg_bits(udev, HW_CFG, HW_CFG_MEF | HW_CFG_BIR | HW_CFG_BCE);
	if (status != USB_STATUS_SUCCESS) {
		pi_printf("error: fail to set smsc9512\r\n");
	}

	/* Set the maximum USB (not networking!) packets per USB Rx transfer.
	 * Required when HW_CFG_MEF was set.  */
	status = smsc9512_write_reg(udev, BURST_CAP,
			SMSC9512_DEFAULT_HS_BURST_CAP_SIZE / SMSC9512_HS_USB_PKT_SIZE);
//	pi_printf("infor: set the maximum USB packets\r\n");
	if (status != USB_STATUS_SUCCESS) {
		pi_printf("error: fail to write smsc9512\r\n");
	}

	/* Check for error and return.  */
	if (udev->last_error != USB_STATUS_SUCCESS) {
		pi_printf("error: smsc9512 check error\r\n");
		return udev->last_error;
	}

	smsc9512_set_reg_bits(udev, LED_GPIO_CFG, LED_GPIO_CFG_LNK_LED);

	ethptr->csr = udev;
	udev->driver_private = ethptr;
	ETH_ATTACHED_FLAG = true;
	semsignal(smsc9512_attached[ethptr - ethertab]);
	SetEvent(TASK_ID_EthOpenTask, EVENT_MASK_EthOpenEvent);

	pi_printf("infor: smsc9512 bind\r\n");
	return USB_STATUS_SUCCESS;
}

/**
 * Unbinds the SMSC LAN9512 driver from a SMSC LAN9512 that has been detached.
 * This the @ref usb_device_driver::unbind_device "unbind_device" implementation
 * for the SMSC LAN9512 driver and therefore complies with its documented
 * behavior.
 */
static void smsc9512_unbind_device(struct usb_device *udev) {
	struct ether *ethptr = udev->driver_private;

	/* Reset attached semaphore to 0.  */
	wait(smsc9512_attached[ethptr - ethertab]);

	/* Close the device.  */
	//etherClose(ethptr->dev);
}

/**
 * Specification of a USB device driver for the SMSC LAN9512.  This is
 * specifically for the USB core and is not related to Xinu's primary device and
 * driver model, which is static.
 */
static const struct usb_device_driver smsc9512_driver = {
		.name = "LAN9512 Driver",
		.bind_device = smsc9512_bind_device,
		.unbind_device = smsc9512_unbind_device,
};

void randomEthAddr(uint8 addr[ETH_ADDR_LEN]) {
	//TODO: latter on
	//{0xb8,0x27,0xeb,0x4b,0x1f,0xdd} .0.8
//	addr[0] = 0Xb8;
//	addr[1] = 0X27;
//	addr[2] = 0XEB;
//	addr[3] = 0X4b;
//	addr[4] = 0X1f;
//	addr[5] = 0Xdd;

	addr[0] = 0Xb8;
	addr[1] = 0X27;
	addr[2] = 0XEB;
	addr[3] = 0X35;
	addr[4] = 0X10;
	addr[5] = 0Xdf;
	/* Clear multicast bit and set locally assigned bit */
	addr[0] &= 0xfe;
	addr[0] |= 0x02;
}

/**
 * @ingroup etherspecific
 *
 * Wait until the specified Ethernet device has actually been attached.
 *
 * This is necessary because USB is a dynamic bus, but Xinu expects static
 * devices.  Therefore, code opening a static ethernet device must wait until
 * the corresponding USB device has actually been detected and attached.  Fun
 * fact: the USB standard places no constraints on how long this will actually
 * take, even if the device is physically soldered to the board.
 *
 * TODO: Wait for at most a certain amount of time before returning failure.
 *
 * @param minor
 *     Minor number of the Ethernet device to wait for.
 *
 * @return
 *      Currently ::USB_STATUS_SUCCESS.  TODO: implement timeouts and return
 *      ::USB_STATUS_TIMEOUT if timed out.
 */
usb_status_t smsc9512_wait_device_attached(ushort minor) {

	while (ETH_ATTACHED_FLAG == false) {

	}
	wait(smsc9512_attached[minor]);
	semsignal(smsc9512_attached[minor]);
	return USB_STATUS_SUCCESS;
}

/**
 * @ingroup etherspecific
 *
 * Callback function executed with interrupts disabled when an asynchronous USB
 * bulk transfer to the Bulk OUT endpoint of the SMSC LAN9512 USB Ethernet
 * Adapter for the purpose of sending an Ethernet packet has successfully
 * completed or has failed.
 *
 * Currently all this function has to do is return the buffer to its pool.  This
 * may wake up a thread in etherWrite() that is waiting for a free buffer.
 *
 * @param req
 *      USB bulk OUT transfer request that has completed.
 */
void smsc9512_tx_complete(struct usb_xfer_request *req) {
	struct ether *ethptr = req->private;

	ethptr->txirq++;

    //pi_printf("infor: smsc9512 tx complete\r\n");
	//bcm2835_delayMicroseconds(1000);
	buffree(req);
}
uint32 previous = 0;
/**
 * @ingroup etherspecific
 *
 * Callback function executed with interrupts disabled when an asynchronous USB
 * bulk transfer from the Bulk IN endpoint of the SMSC LAN9512 USB Ethernet
 * Adapter for the purpose of receiving one or more Ethernet packets has
 * successfully completed or has failed.
 *
 * This function is responsible for breaking up the raw USB transfer data into
 * the constituent Ethernet packet(s), then pushing them onto the incoming
 * packets queue (which may wake up threads in etherRead() that are waiting for
 * new packets).  It then must re-submit the USB bulk transfer request so that
 * packets can continue to be received.
 *
 * @param req
 *      USB bulk IN transfer request that has completed.
 */
void smsc9512_rx_complete(struct usb_xfer_request *req) {
//	uint32 startTime = Microseconds();
////	printf("interval:%d\r\n",startTime - previous);
//
//	previous = startTime;
	struct ether *ethptr = req->private;
	ethptr->rxirq++;
	if (req->status == USB_STATUS_SUCCESS) {
		const unsigned char *data, *edata;
		uint32_t recv_status;
		uint32_t frame_length;

		/* For each Ethernet frame in the received USB data... */
		for (data = req->recvbuf, edata = req->recvbuf + req->actual_size;
				data + SMSC9512_RX_OVERHEAD + ETH_HDR_LEN + ETH_CRC_LEN <= edata;
				data += SMSC9512_RX_OVERHEAD + ((frame_length + 3) & ~3)) {

			/* Get the Rx status word, which contains information about the next
			 * Ethernet frame.  */
			recv_status = data[0] | data[1] << 8 | data[2] << 16 | data[3] << 24;

			/* Extract frame_length, which specifies the length of the next
			 * Ethernet frame from the MAC destination address to end of the CRC
			 * following the payload.  (This does not include the Rx status
			 * word, which we instead account for in SMSC9512_RX_OVERHEAD.) */
			frame_length = (recv_status & RX_STS_FL) >> 16;

			if ((recv_status & RX_STS_ES)
					|| (frame_length + SMSC9512_RX_OVERHEAD > edata - data)
					|| (frame_length > ETH_MAX_PKT_LEN + ETH_CRC_LEN)
					|| (frame_length < ETH_HDR_LEN + ETH_CRC_LEN)) {
				/* The Ethernet adapter set the error flag to indicate a problem
				 * or the Ethernet frame size it provided was invalid. */
//                usb_dev_debug(req->dev, "SMSC9512: Tallying rx error "
//                              "(recv_status=0x%08x, frame_length=%u)\n",
//                              recv_status, frame_length);
				pi_printf("error: smsc9512 rx error\r\n");
				ethptr->errors++;
			} else if (ethptr->icount == ETH_IBLEN) {
				/* No space to buffer another received packet.  */
				//usb_dev_debug(req->dev, "SMSC9512: Tallying overrun\n");
				pi_printf("error: smsc9512 rx overrun\r\n");
				ethptr->ovrrun++;
			} else {
				/* Buffer the received packet.  */
				struct ethPktBuffer *pkt;

				pkt = bufget(ethptr->inPool);
				pkt->buf = pkt->data = (unsigned char*) (pkt + 1);
				pkt->length = frame_length - ETH_CRC_LEN;

				memcpy(pkt->buf, data + SMSC9512_RX_OVERHEAD, pkt->length);
				ethptr->in[(ethptr->istart + ethptr->icount) % ETH_IBLEN] = pkt;
				ethptr->icount++;

//              usb_dev_debug(req->dev, "SMSC9512: Receiving "
//                              "packet (length=%u, icount=%u)\n",
//                              pkt->length, ethptr->icount);
				//semsignal(ethptr->isema);
				low_level_input();     // this part seems logic wrong.
			}
		}
	} else {
		/* USB transfer failed for some reason.  */
		pi_printf("infor: smsc9512 rx failed\r\n");
		ethptr->errors++;
	}
	//usb_dev_debug(req->dev, "SMSC9512: Re-submitting USB Rx request\n");
	usb_submit_xfer_request(req);
//	uint32 overTime = Microseconds();
//	printf("ETH:%d\r\n", overTime-startTime);
}

int Eth_Init(void) {
	struct ether *ethptr;
	usb_status_t status;
	sem_err err;
	/* Initialize the static `struct ether' for this device.  */
	ethptr = &ethertab[0];
	bzero(ethptr, sizeof(struct ether));  //maybe need do something
	//ethptr->dev = devptr;
	ethptr->state = ETH_STATE_DOWN;
	ethptr->mtu = ETH_MTU;
	ethptr->addressLength = ETH_ADDR_LEN;

	err = usb_sem_new(&ethptr->isema, 0);

	if (err != SEM_OK) {
		pi_printf("error: fail to create ethptr sema\r\n");
		goto err;
	}
	//smsc9512_attached[devptr->minor] = semcreate(0);
	smsc9512_attached[0] = semcreate(0); // i am not sure here,just use 0 to instead.
	if (isbadsem(smsc9512_attached[0])) { //here is the same:smsc9512_attached[devptr->minor]
		goto err_free_isema;
	}
	/* The SMSC LAN9512 on the Raspberry Pi does not have an EEPROM attached.
	 * The EEPROM is normally used to store the MAC address of the adapter,
	 * along with some other information.  As a result, software needs to set
	 * the MAC address to a value of its choosing (such as a random number).  */
	randomEthAddr(ethptr->devAddress);
	/* Register this device driver with the USB core and return.  */
	status = usb_register_device_driver(&smsc9512_driver);
	if (status != USB_STATUS_SUCCESS) {
		goto err_free_attached_sema;
	}
	return E_OK ;

	err_free_attached_sema:
		semfree(smsc9512_attached[0]); //here is the same:smsc9512_attached[devptr->minor] fake
	err_free_isema:
		usb_sem_free(&ethptr->isema);
	err:
		return E_NOT_OK ;
}

/* Implementation of etherOpen() for the smsc9512; see the documentation for
 * this function in ether.h.  */
/**
 * @details
 *
 * SMSC LAN9512-specific notes:  as a work-around to use USB's dynamic device
 * model at the same time as Xinu's static device model, this function will
 * block until the corresponding USB device has actually been attached to the
 * USB.  Strictly speaking, there is no guarantee as to when this will actually
 * occur, even if the device is non-removable.
 */
void EthOpenTask(void) {
	//irqmask im;
	struct ether *ethptr;
	struct usb_device *udev;
	uint i;
	int retval = SYSERR;

	for (;;) {

		WaitEvent(EVENT_MASK_EthOpenEvent);
		boolean err = false;
//		ClearEvent(EVENT_MASK_EthOpenEvent);
		if (ETH_ATTACHED_FLAG == true) {

//			printf("infor: ether open\r\n");
			/* Fail if device is not down.  */
			ethptr = &ethertab[0];
			if (ethptr->state != ETH_STATE_DOWN) {
				pi_printf("error: ether state down\r\n");
				goto out_restore;
			}

			/* Create buffer pool for Tx transfers.  */
			ethptr->outPool = bfpalloc(
					sizeof(struct usb_xfer_request) + ETH_MAX_PKT_LEN
							+ SMSC9512_TX_OVERHEAD, SMSC9512_MAX_TX_REQUESTS); //

			if (ethptr->outPool == SYSERR) {
				pi_printf("error: fail to allocate outpool\r\n");
				goto out_restore;
			}else{
//				pi_printf("infor: ethptr->outPool = "); mini_uart_sendDec(ethptr->outPool);pi_printf("\r\n");
			}

			/* Create buffer pool for Rx packets (not the actual USB transfers, which
			 * are allocated separately).  */
			ethptr->inPool = bfpalloc(
					sizeof(struct ethPktBuffer) + ETH_MAX_PKT_LEN, ETH_IBLEN);
			if (ethptr->inPool == SYSERR) {
				pi_printf("error: fail to allocate inpool\r\n");
				goto out_free_out_pool;
			}else{
//				pi_printf("infor: ethptr->inPool = "); mini_uart_sendDec(ethptr->inPool);pi_printf("\r\n");
			}

			/* We're abusing the csr field to store a pointer to the USB device
			 * structure.  At least it's somewhat equivalent, since it's what we need to
			 * actually communicate with the device hardware.  */

			udev = ethptr->csr;

//			pi_printf("infor: smsc9512 set mac address\r\n");
			/* Set MAC address */
			if (smsc9512_set_mac_address(udev, ethptr->devAddress)
					!= USB_STATUS_SUCCESS) {
				pi_printf("error: fail to smsc9512 set mac address\r\n");
				goto out_free_in_pool;
			}

			/* Initialize the Tx requests.  */
//			pi_printf("infor: initialize the tx requests.\r\n");
			{

				struct usb_xfer_request *reqs[SMSC9512_MAX_TX_REQUESTS];
				for (i = 0; i < SMSC9512_MAX_TX_REQUESTS; i++) {
					struct usb_xfer_request *req;

					req = bufget(ethptr->outPool);
					usb_init_xfer_request(req);
					req->dev = udev;
					/* Assign Tx endpoint, checked in smsc9512_bind_device() */
					req->endpoint_desc = udev->endpoints[0][1];
					req->sendbuf = (unsigned char*) req	+ sizeof(struct usb_xfer_request);
					req->completion_cb_func = smsc9512_tx_complete;
					req->private = ethptr;
					reqs[i] = req;
				}
				for (i = 0; i < SMSC9512_MAX_TX_REQUESTS; i++) {
					buffree(reqs[i]);
				}
			}

			/* Allocate and submit the Rx requests.  TODO: these aren't freed anywhere.
			 * */
//			pi_printf("infor: initialize the rx requests.\r\n");
			for (i = 0; i < SMSC9512_MAX_RX_REQUESTS; i++) {
				struct usb_xfer_request *req;

				req = usb_alloc_xfer_request(SMSC9512_DEFAULT_HS_BURST_CAP_SIZE); //18944
				if (req == NULL ) {
					goto out_free_in_pool;
				}
				req->dev = udev;
				/* Assign Rx endpoint, checked in smsc9512_bind_device() */
				req->endpoint_desc = udev->endpoints[0][0];
				req->completion_cb_func = smsc9512_rx_complete;
				req->private = ethptr;
				usb_submit_xfer_request(req);
			}

			/* Enable transmit and receive on the actual hardware.  After doing this and
			 * restoring interrupts, the Rx transfers can complete at any time due to
			 * incoming packets.  */
			udev->last_error = USB_STATUS_SUCCESS;
//			pi_printf("infor: smsc9512 set reg bits.\r\n");
			smsc9512_set_reg_bits(udev, MAC_CR, MAC_CR_TXEN | MAC_CR_RXEN);
//			pi_printf("infor: smsc9512 write reg bits.\r\n");
			smsc9512_write_reg(udev, TX_CFG, TX_CFG_ON);

			if (udev->last_error != USB_STATUS_SUCCESS) {

				pi_printf("error: usb error in ethopen\r\n");
				goto out_free_in_pool;
			}

			/* Success!  Set the device to ETH_STATE_UP. */
			ethptr->state = ETH_STATE_UP;

			goto out_restore;

			out_free_in_pool:
				bfpfree(ethptr->inPool);
                err = true;
			out_free_out_pool:
				bfpfree(ethptr->outPool);
				err = true;
			out_restore:
			    if(err == false){
			    	ETH_ATTACHED_FLAG = false;
			    	pi_printf("infor: ether open done\r\n");
			   		ETH_OPEN_FLAG = true;
			    }
			//restore(im);
		}
		ClearEvent(EVENT_MASK_EthOpenEvent);
	}

}

/* Implementation of etherWrite() for the SMSC LAN9512; see the documentation
 * for this function in ether.h.  */
int Eth_Write(const void *buf, uint len) {
	struct ether *ethptr;
	struct usb_xfer_request *req;
	unsigned char *sendbuf;
	uint32_t tx_cmd_a, tx_cmd_b;

	ethptr = &ethertab[0];
	if (ethptr->state != ETH_STATE_UP || len < ETH_HEADER_LEN || len > ETH_HDR_LEN + ETH_MTU) {
		pi_printf("error: fail to eth write\r\n");
		return SYSERR;
	}
//	pi_printf("infor:bufget out pool"); mini_uart_sendDec(ethptr->outPool);pi_printf("\r\n");
//	bcm2835_delayMicroseconds(1000);
	/* Get a buffer for the packet.  (This may block.)  */
	req = bufget(ethptr->outPool);
	/* Copy the packet's data into the buffer, but also include two words at the
	 * beginning that contain device-specific flags.  These two fields are
	 * required, although we essentially just use them to tell the hardware we
	 * are transmitting one (1) packet with no extra bells and whistles.  */
	sendbuf = req->sendbuf;
	tx_cmd_a = len | TX_CMD_A_FIRST_SEG | TX_CMD_A_LAST_SEG;
	sendbuf[0] = (tx_cmd_a >> 0) & 0xff;
	sendbuf[1] = (tx_cmd_a >> 8) & 0xff;
	sendbuf[2] = (tx_cmd_a >> 16) & 0xff;
	sendbuf[3] = (tx_cmd_a >> 24) & 0xff;
	tx_cmd_b = len;
	sendbuf[4] = (tx_cmd_b >> 0) & 0xff;
	sendbuf[5] = (tx_cmd_b >> 8) & 0xff;
	sendbuf[6] = (tx_cmd_b >> 16) & 0xff;
	sendbuf[7] = (tx_cmd_b >> 24) & 0xff;
	STATIC_ASSERT(SMSC9512_TX_OVERHEAD == 8);
	memcpy(sendbuf + SMSC9512_TX_OVERHEAD, buf, len);  //des src len

	/* Set total size of the data to send over the USB.  */
	req->size = len + SMSC9512_TX_OVERHEAD;
	/* Submit the data as an asynchronous bulk USB transfer.  In other words,
	 * this tells the USB subsystem to send begin sending the data over the USB
	 * to the SMSC LAN9512 USB Ethernet Adapter.  At some later time when all
	 * the data has been transferred over the USB, smsc9512_tx_complete() will
	 * be called by the USB subsystem.  */

	usb_submit_xfer_request(req);
	/* Return the length of the packet written (not including the
	 * device-specific fields that were added). */

	return len;
}

/* Implementation of etherRead() for the smsc9512; see the documentation for
 * this function in ether.h.  */
int Eth_Read(void *buf, uint len) {
	struct ether *ethptr;
	struct ethPktBuffer *pkt;

	irqmask im;
	im = disable();

	/* Make sure device is actually up.  */
	ethptr = &ethertab[0];
	if (ethptr->state != ETH_STATE_UP) {
		restore(im);
		pi_printf("infor: ether not up\r\n");
		return SYSERR;
	}

	/* Wait for received packet to be available in the ethptr->in circular
	 * queue.  */
//	usb_sem_wait(&ethptr->isema, 0);

	/* Remove the received packet from the circular queue.  */
	pkt = ethptr->in[ethptr->istart];
	ethptr->istart = (ethptr->istart + 1) % ETH_IBLEN;
	ethptr->icount--;

	/* TODO: we'd like to restore interrupts here (before the memcpy()), but
	 * this doesn't work yet because smsc9512_rx_complete() expects a buffer to
	 * be available if icount < ETH_IBLEN; therefore, since we decremented
	 * icount, we can't restore interrupts until we actually release the
	 * corresponding buffer.  */

	/* Copy the data from the packet buffer, being careful to copy at most the
	 * number of bytes requested. */
	if (pkt->length < len) {
		len = pkt->length;
	}
	memcpy(buf, pkt->buf, len);
/*	New_Data_Flag = true;*/
//	pi_printf("frame: ");
//    for (int i = 0; i < len; i+=4) {
//    	uint32_t temp = pkt->buf[i] | pkt->buf[i+1] << 8 | pkt->buf[i+2] << 16 | pkt->buf[i+3] << 24;
//    	pi_print(temp,0);
//    }
//    pi_printf("\r\n");
	/* Return the packet buffer to the pool, then return the length of the
	 * packet received.  */
	buffree(pkt);
	restore(im);
	return len;
}

/* Implementation of etherControl() for the smsc9512; see the documentation for
 * this function in ether.h.  */
int Eth_Control(int req, long arg1, long arg2) {
	struct usb_device *udev;
	usb_status_t status;
	struct netaddr *addr;
	struct ether *ethptr;

	ethptr = &ethertab[0];
	udev = ethptr->csr;
	if (udev == NULL ) {
		return SYSERR;
	}

	status = USB_STATUS_SUCCESS;

	switch (req) {
	/* Program MAC address into device. */
	case ETH_CTRL_SET_MAC:
		status = smsc9512_set_mac_address(udev, (const uchar*) arg1);
		break;

		/* Get MAC address from device. */
	case ETH_CTRL_GET_MAC:
		status = smsc9512_get_mac_address(udev, (uchar*) arg1);
		break;

		/* Enable or disable loopback mode.  */
	case ETH_CTRL_SET_LOOPBK:
		status = smsc9512_modify_reg(udev, MAC_CR, ~MAC_CR_LOOPBK,
				((boolean) arg1 == TRUE ) ? MAC_CR_LOOPBK : 0);
		break;

		/* Get link header length. */
	case NET_GET_LINKHDRLEN:
		return ETH_HDR_LEN;

		/* Get MTU. */
	case NET_GET_MTU:
		return ETH_MTU;

		/* Get hardware address.  */
	case NET_GET_HWADDR:
		addr = (struct netaddr *) arg1;
		addr->type = NETADDR_ETHERNET;
		addr->len = ETH_ADDR_LEN;
		return Eth_Control(ETH_CTRL_GET_MAC, (long) addr->addr, 0);

		/* Get broadcast hardware address. */
	case NET_GET_HWBRC:
		addr = (struct netaddr *) arg1;
		addr->type = NETADDR_ETHERNET;
		addr->len = ETH_ADDR_LEN;
		memset(addr->addr, 0xFF, ETH_ADDR_LEN);
		break;

	default:
		return SYSERR;
	}

	if (status != USB_STATUS_SUCCESS) {
		return SYSERR;
	}

	return OK;
}

