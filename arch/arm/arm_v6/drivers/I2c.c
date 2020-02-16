/*
 * I2c.c
 *
 *  Created on: 6 aug 2014
 *      Author: sse
 */

#include <stdint.h>
#include <stdio.h>
#include "bcm2835.h"
#include "I2c.h"
#include "isr.h"


i2cdriver iicdriver;
boolean release = false;
uint32 i2cByteTxTime_ns;
/** @brief nano seconds in a second */
#define NSEC_IN_SEC                 1000000000
/** @brief Clock pulses per I2C byte - 8 bits + ACK */
#define CLOCKS_PER_BYTE             9

void I2c_Isr(void) {
	bscdevice_t *device = iicdriver.device;
	uint32_t status = device->status;
	printf("i2c_isr in\r\n");
	if (status & BSC_CLKT) {   //error happens
		printf("error: clock time out\r\n");
	}else if (status & BSC_ERR) {   //error happens
		// TODO set error flags
		printf("error: slave no ack\r\n");
	}
	else if (status & BSC_DONE)  //tranfer complete
	{
		while ((status & BSC_RXD) && (iicdriver.rxidx < iicdriver.rxbytes))
		{   //if FIFO = 1, contains at least 1 byte && there is data need to read
			iicdriver.rxbuf[iicdriver.rxidx++] = device->dataFifo;
		}
		//disable the bsc
		device->control = 0;
		//clear clkt, err and done of status register by writing 1
		device->status = BSC_CLKT | BSC_ERR | BSC_DONE;
	}
	else if (status & BSC_TXW)  //FIFO less then full
	{
		while ((iicdriver.txidx < iicdriver.txbytes) && (status & BSC_TXD))
		{   //there is data need to be transfered && FIFO can accept more data
			device->dataFifo = iicdriver.txbuf[iicdriver.txidx++];
			printf("i2c_isr w\r\n");
		}

	}
	else if (status & BSC_RXR) { //FIFO is full, needs to read
		while ((iicdriver.rxidx < iicdriver.rxbytes) && (status & BSC_RXD))
		{
			printf("i2c_isr r\r\n");
			iicdriver.rxbuf[iicdriver.rxidx++] = device->dataFifo;
		}
	}
	printf("i2c_isr out\r\n");
}

/**
 * @brief   Low level I2C driver initialization.
 *
 * @notapi
 */
void I2c_Init(void) {

//	printf("I2c_Init starts\r\n");
	iicdriver.device = BSC1_ADDR;
	iicdriver.state = I2C_STOP;
	iicdriver.config = NULL;
//	printf("I2c_Init set io pins\r\n");
	/* Set up GPIO pins for I2C */
	bcm2835_GpioFnSel(2, GPFN_ALT0);
	bcm2835_GpioFnSel(3, GPFN_ALT0);

	iicdriver.device->clockDivider = BSC_CLOCK_FREQ/375; //clk at 400khz
	/* Setup the Control Register.
	 * Enable the BSC Controller.
	 * Clear the FIFO. */
	iicdriver.device->control  = BSC_I2CEN | BSC_CLEAR;
	/* Setup the Status Register
	 * Clear NACK ERR flag.
	 * Clear Clock stretch flag.
	 * Clear Done flag. */
	iicdriver.device->status = BSC_ERR | BSC_CLKT | BSC_DONE;

//	ISR_INSTALL_ISR2("I2C", I2c_Isr, BCM2835_IRQ_ID_I2C, 9, 0);
//	IRQ_ENABLE2 = I2C_IRQ;
}


/**
 * @brief   Master receive.
 *
 * @param[in] addr      slave device address (7 bits) without R/W bit
 * @param[out] rxbuf     receive data buffer pointer
 * @param[in] rxbytes   number of bytes to be received
 */
void I2c_Read(i2caddr_t addr, uint8_t *rxbuf, uint32 rxbytes) {

	iicdriver.addr = addr;
	iicdriver.txbuf = NULL;
	iicdriver.txbytes = 0;
	iicdriver.txidx = 0;
	iicdriver.rxbuf = rxbuf;
	iicdriver.rxbytes = rxbytes;
	iicdriver.rxidx = 0;

	/* Setup device.*/
	bscdevice_t *device = iicdriver.device;
	device->slaveAddress = addr;
	device->dataLength = rxbytes;
	//generate interrupt on RX, while RXR = 1, which means FIFO needs Reading
	//generate interrupt while Done = 1, which means data transfer done
	device->control |= BSC_ST|BSC_CLEAR|BSC_READ;
	// wait for read operation to complete?
    // update index?
	while (!(device->status & BSC_DONE));
	while ((iicdriver.rxidx < iicdriver.rxbytes) && (device->status & BSC_RXD)) {
		iicdriver.rxbuf[iicdriver.rxidx] = device->dataFifo;
        //printf("rxbuf:%x\r\n", iicdriver.rxbuf[iicdriver.rxidx]);
		iicdriver.rxidx++;
	}
	if (device->status & BSC_ERR) {
		device->status |= BSC_ERR;
		printf("infor: Received a NACK.\r\n");
	}
	/* Received Clock Timeout error */
	else if (device->status & BSC_CLKT) {
		device->status |= BSC_CLKT;
		printf("infor: Received a Clock Stretch Timeout.\r\n");
	}
	/* Clear the DONE flag */
	device->status |= BSC_DONE;
}


/**
 * @brief   Master transmission.
 *
 * @param[in] addr      slave device address (7 bits) without R/W bit
 * @param[in] txbuf     transmit data buffer pointer
 * @param[in] txbytes   number of bytes to be transmitted
 *
 * @notapi
 */
i2cstate_t I2c_Write(i2caddr_t addr, const uint8_t *txbuf, uint32 txbytes) {

	i2cstatus_t i2c_state = -1;

	iicdriver.addr = addr;
	iicdriver.txbuf = txbuf;
	iicdriver.txbytes = txbytes;
	iicdriver.txidx = 0;
	iicdriver.rxbuf = NULL;
	iicdriver.rxbytes = 0;
	iicdriver.rxidx = 0;

	bscdevice_t *device = iicdriver.device;
	device->slaveAddress = addr;
	device->dataLength = txbytes;

	//Enable Interrupts and start transfer.
	//generate interrupt while TXW = 1, which means FIFO needs Writing
	//generate interrupt while Done = 1, which means data transfer done
   	device->control |= BSC_CLEAR;
	/* Configure Control for a write */
	device->control &= ~BSC_READ;
	/* Configure Control Register for a Start */
	device->control = START_WRITE;

	while (!(device->status & BSC_DONE)) {
		//there is data need to be transfered && FIFO can accept more data
		while ((iicdriver.txidx < iicdriver.txbytes) && (device->status & BSC_TXD)) {
//			printf("txbuf:%x\r\n", iicdriver.txbuf[iicdriver.txidx]);
			device->dataFifo = iicdriver.txbuf[iicdriver.txidx++];
		}
        if (iicdriver.txidx < iicdriver.txbytes)
        {
        	printf("data left:%d\r\n", iicdriver.txbytes - iicdriver.txidx);
        }
        else
        {
//        	printf("write complete\r\n");
        }

	}
	if (device->status & BSC_ERR) {
		device->status |= BSC_ERR;
		printf("infor: I2c_Write received a NACK.\r\n");
		i2c_state = I2C_TX_ERR;
	}
	/* Received Clock Timeout error */
	else if (device->status & BSC_CLKT) {
		device->status |= BSC_CLKT;
		i2c_state = I2C_TX_ERR;
		printf("infor: I2c_Write received a Clock Stretch Timeout.\r\n");
	}
	else if (iicdriver.txidx < iicdriver.txbytes) {
		printf("infor: I2c_Write BSC signaled done but %d data remained.\r\n",
				iicdriver.txbytes-iicdriver.txidx);
		i2c_state = I2C_TX_ERR;
	}
	else {
        //printf("infor: I2c_Write OK\r\n");
		i2c_state = I2C_TX_DONE;
	}

	/* Clear the DONE flag */
	device->status |= BSC_DONE;
	return i2c_state;

}





