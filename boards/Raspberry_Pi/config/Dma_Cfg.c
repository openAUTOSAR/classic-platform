/*
 * Dma_Cfg.c
 *
 *  Created on: Sep 16, 2013
 *      Author: Zhang Shuzhou
 */

#include <Dma.h>
#include "bcm2835.h"

const Dma_ChannelConfigType DmaChannelConfig [DMA_NUMBER_OF_CHANNELS] =
{
	{
		.DMA_CHANNEL_PRIORITY = DMA_CHANNEL0,
		.DMA_CHANNEL_PREEMTION_ENABLE = BIT(1)		// Sets the value of the ENABLE register (enabling dma engine 0)
	},

	{
		.DMA_CHANNEL_PRIORITY = DMA_CHANNEL1,
		.DMA_CHANNEL_PREEMTION_ENABLE = BIT(2)		// Sets the value of the ENABLE register (enabling dma engine 1)
	},
};


const Dma_ConfigType DmaConfig []=
{
  {DmaChannelConfig, DMA_FIXED_PRIORITY_ARBITRATION}
};

