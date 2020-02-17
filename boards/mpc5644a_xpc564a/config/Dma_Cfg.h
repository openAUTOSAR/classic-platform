/*-------------------------------- Arctic Core ------------------------------
 * Copyright (C) 2013, ArcCore AB, Sweden, www.arccore.com.
 * Contact: <contact@arccore.com>
 *
 * You may ONLY use this file:
 * 1)if you have a valid commercial ArcCore license and then in accordance with
 * the terms contained in the written license agreement between you and ArcCore,
 * or alternatively
 * 2)if you follow the terms found in GNU General Public License version 2 as
 * published by the Free Software Foundation and appearing in the file
 * LICENSE.GPL included in the packaging of this file or here
 * <http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt>
 *-------------------------------- Arctic Core -----------------------------*/

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

  DMA_DSPI_B_COMMAND_CHANNEL,
  DMA_DSPI_B_RESULT_CHANNEL,

  DMA_DSPI_C_COMMAND_CHANNEL,
  DMA_DSPI_C_RESULT_CHANNEL,

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

#define DMA_START_CHANNEL	DMA_ADC_GROUP0_COMMAND_CHANNEL

#endif /* DMA_CFG_H_ */
