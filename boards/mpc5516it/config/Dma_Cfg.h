/* -------------------------------- Arctic Core ------------------------------
 * Arctic Core - the open source AUTOSAR platform http://arccore.com
 *
 * Copyright (C) 2009  ArcCore AB <contact@arccore.com>
 *
 * This source code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by the
 * Free Software Foundation; See <http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 * -------------------------------- Arctic Core ------------------------------*/








#ifndef DMA_CFG_H_
#define DMA_CFG_H_

typedef enum
{
  DMA_ADC_GROUP0_RESULT_CHANNEL,
  DMA_ADC_GROUP0_COMMAND_CHANNEL,
  DMA_ADC_GROUP1_RESULT_CHANNEL,
  DMA_ADC_GROUP1_COMMAND_CHANNEL,
  DMA_DSPI_A_COMMAND_CHANNEL,
  DMA_DSPI_A_RESULT_CHANNEL,
  DMA_DSPI_B_COMMAND_CHANNEL,
  DMA_DSPI_B_RESULT_CHANNEL,
  /*DMA_CHANNEL8,
  DMA_CHANNEL9,
  DMA_CHANNEL10,
  DMA_CHANNEL11,
  DMA_CHANNEL12,
  DMA_CHANNEL13,
  DMA_CHANNEL14,
  DMA_CHANNEL15,*/
  DMA_NUMBER_OF_CHANNELS
} Dma_ChannelType;

#endif /* DMA_CFG_H_ */
