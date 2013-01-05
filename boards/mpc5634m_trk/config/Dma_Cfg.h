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

// See section 7.1.2 DMA Request Assignments in MPC5634M RM
typedef enum
{
  DMA_ADC_GROUP0_COMMAND_CHANNEL,
  DMA_ADC_GROUP0_RESULT_CHANNEL,

  DMA_ADC_GROUP1_COMMAND_CHANNEL,
  DMA_ADC_GROUP1_RESULT_CHANNEL,

  DMA_ADC_GROUP2_COMMAND_CHANNEL,
  DMA_ADC_GROUP2_RESULT_CHANNEL,

  DMA_ADC_GROUP3_COMMAND_CHANNEL,
  DMA_ADC_GROUP3_RESULT_CHANNEL,

  DMA_ADC_GROUP4_COMMAND_CHANNEL,
  DMA_ADC_GROUP4_RESULT_CHANNEL,

  DMA_ADC_GROUP5_COMMAND_CHANNEL,
  DMA_ADC_GROUP5_RESULT_CHANNEL,

  DMA_SPI_B_TRANSMIT_CHANNEL,
  DMA_SPI_B_RECEIVE_CHANNEL,

  DMA_SPI_C_TRANSMIT_CHANNEL,
  DMA_SPI_C_RECEIVE_CHANNEL,

  DECFIL_FILL_BUF_CHANNEL,
  DECFIL_DRAIN_BUF_CHANNEL,

  SCI_A_TDRE_TC_TXRDY_CHANNEL,
  SCI_A_RDRF_RXRDY_CHANNEL,

  DMA_EMIOS_0_CHANNEL,
  DMA_EMIOS_1_CHANNEL,
  DMA_EMIOS_2_CHANNEL,
  DMA_EMIOS_3_CHANNEL,
  DMA_EMIOS_4_CHANNEL,
  DMA_EMIOS_8_CHANNEL,
  DMA_EMIOS_9_CHANNEL,

  DMA_TPU_0_CHANNEL,
  DMA_TPU_1_CHANNEL,
  DMA_TPU_2_CHANNEL,
  DMA_TPU_14_CHANNEL,
  DMA_TPU_15_CHANNEL,

  DMA_NUMBER_OF_CHANNELS
} Dma_ChannelType;



#endif /* DMA_CFG_H_ */
