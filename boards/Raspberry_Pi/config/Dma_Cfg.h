/*
 * Dma_Cfg.h
 *
 *  Created on: Sep 16, 2013
 *      Author: Zhang Shuzhou
 */

#ifndef DMA_CFG_H_
#define DMA_CFG_H_

/**
 * Up to 16 DMA channels can be defined for BCM2835.
 * However, the last channel is physically separated from the others in
 * the address space and will need special treatment.
 */
typedef enum
 {
	DMA_CHANNEL0,
	DMA_CHANNEL1,
	DMA_NUMBER_OF_CHANNELS
} Dma_ChannelType;

#endif /* DMA_CFG_H_ */
