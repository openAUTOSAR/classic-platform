/*
 * Dma.c
 *
 *  Created on:  Sep 4, 2013
 *      Author:  Zhang Shuzhou
 *  Reviewed on: Nov 7, 2014
 *     Reviewer: Avenir Kobetski
 *
 * This class implements Direct Memory Access (DMA) functionality.
 */

#include "Dma.h"
#include "bcm2835.h"

/**
 * DMA initialization. Basically each defined DMA channel is enabled.
 *
 * @param ConfigPtr			-----	pointer to global configuration data
 */
void Dma_Init(const Dma_ConfigType *ConfigPtr)
{
	Dma_ChannelType channel;
	for (channel = (Dma_ChannelType)0; channel < DMA_NUMBER_OF_CHANNELS; channel++)
	{
	    /* Enable each defined DMA channel in the GLOBAL register */
		DMA_GLOBAL_ENABLE |= ConfigPtr->dmaChannelConfigPtr[channel].DMA_CHANNEL_PREEMTION_ENABLE;
	}
}

/**
 * This function disables all DMA channels
 */
void Dma_DeInit(void)
{
	/* Reset the global DMA ENABLE register */
	DMA_GLOBAL_ENABLE = 0x00;
}

/**
 * This function rounds up the control block pointer to a 256-bit aligned memory address
 *
 * @param cb_address		-----	control block address
 * @return alignedAddress	----- 	256-bit aligned control block address, i.e. rounded up
 * 									so that the last 5 bits are 0
 */
Dma_CBType* Dma_AlignCB(Dma_CBType* cb_address)
{
	uint32 aligned_address = ALIGN((uint32)cb_address, 32);
	return (Dma_CBType*)aligned_address;
}

/**
 * This function resets and reconfigures a DMA channel.
 * Appropriate control block address is written to the control block register.
 *
 * @param cb_address		-----	control block address
 * @param channel			-----	DMA channel to be configured
 */
void Dma_ConfigureChannel (Dma_CBType* cb_address, Dma_ChannelType channel)
{
	/* Reset this DMA channel */
	DMA_CS(channel) = DMA_CS_RESET;
	bcm2835_Sleep(4);

	/* Clear end-of-transfer and interrupt statuses from previous transfer on this channel */
	DMA_CS(channel) = DMA_CS_END |
					  DMA_CS_INT;

	/* Clear all debug error flags */
	DMA_DEBUG(channel) = DMA_CLR_NOT_SET_ERROR |
						 DMA_CLR_FIFO_ERROR	|
						 DMA_CLR_READ_ERROR;

	/* Set the control block address */
	DMA_CONBLK_AD(channel) = (uint32) cb_address;

	/* Activate the channel, set priorities to mid-level, wait for outstanding writes */
	DMA_CS(channel) = DMA_CS_ACTIVE |
					  DMA_CS_MIDPRIO |
					  DMA_CS_PANICMIDPRIO |
					  DMA_CS_WAIT_WRITES;
}

/**
 * This function configures a DMA control block(see p.40 in BCM2835-ARM-Peripherals.pdf).
 *
 * @param transferInfo		----- 	transfer information (TI), i.e. how the actual transfer should be configured
 * @param srcAddress		-----	source address (SOURCE_AD), i.e. where the actual data is stored
 * @param destAddress		-----	destination address (DEST_AD), i.e. which to which register the data will be sent
 * @param transferLength	-----	transfer length (TXFR_LEN), i.e. the number of bytes per DMA package
 * @param stride			-----	2D mode stride (STRIDE), there are two ways of configuring DMA transfer length,
 * 									normal and 2D, we use the normal mode, i.e. stride = 0.
 * @param nextCB			-----	next control block address (NEXTCONBK),
 * 									if non-null this pointer triggers consecutive DMA transfer
 */
void Dma_ConfigureCB(Dma_CBType* cb,
  					 uint32 transferInfo, uint32 srcAddress,
					 uint32 destAddress, uint32 transferLength,
					 uint32 stride, uint32 nextCB)
{
	cb->info = transferInfo;
	cb->src = srcAddress;
	cb->dst = destAddress;
	cb->length = transferLength;
	cb->stride = stride;
	cb->next = nextCB;
}
