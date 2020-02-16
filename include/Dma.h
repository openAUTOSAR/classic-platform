/*
 * Dma.h
 *
 *  Created on: Sep 4, 2013
 *      Author: Zhang Shuzhou
 */

#ifndef DMA_H_
#define DMA_H_

#include "Dma_Cfg.h"
#include "Std_Types.h"

typedef struct
{
	volatile uint32 DMA_CHANNEL_PRIORITY;
	volatile uint32 DMA_CHANNEL_PREEMTION_ENABLE;
} Dma_ChannelConfigType;

typedef enum
{
  DMA_FIXED_PRIORITY_ARBITRATION,
  DMA_ROUND_ROBIN_ARBITRATION
} Dma_ChannelArbitrationType;

typedef struct
{
  const Dma_ChannelConfigType      *dmaChannelConfigPtr;
  const Dma_ChannelArbitrationType  dmaChannelArbitration;
} Dma_ConfigType;

/* DMA Control Block Data Structure (p.40 in BCM2835-ARM-Peripherals.pdf) */
typedef struct {
	uint32		info;	// TI: transfer information
	uint32  	src; 	// SOURCE_AD
	uint32  	dst; 	// DEST_AD
	uint32  	length; // TXFR_LEN: transfer length
	uint32  	stride; // 2D stride mode
	uint32  	next; 	// NEXTCONBK
	uint32  	pad[2]; // _reserved_
} Dma_CBType;

extern const Dma_ConfigType DmaConfig[];

/**
 * DMA initialization. Basically each defined DMA channel is enabled.
 *
 * @param ConfigPtr			-----	pointer to global configuration data
 */
void Dma_Init (const Dma_ConfigType *ConfigPtr);

/**
 * This function disables all DMA channels
 */
void Dma_DeInit(void);

/**
 * This function rounds up the control block pointer to a 256-bit aligned memory address
 *
 * @param cb_address		-----	control block address
 * @return alignedAddress	----- 	256-bit aligned control block address, i.e. rounded up
 * 									so that the last 5 bits are 0
 */
Dma_CBType* Dma_AlignCB(Dma_CBType* cb_address);

/**
 * This function resets and reconfigures a DMA channel.
 * Appropriate control block address is written to the control block register.
 *
 * @param cb_address		-----	control block address
 * @param channel			-----	DMA channel to be configured
 */
void Dma_ConfigureChannel (Dma_CBType* cb_address, Dma_ChannelType channel);

/**
 * This function configures a DMA control block (see p.40 in BCM2835-ARM-Peripherals.pdf).
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
					 uint32 stride, uint32 nextCB);

#endif /* DMA_H_ */
