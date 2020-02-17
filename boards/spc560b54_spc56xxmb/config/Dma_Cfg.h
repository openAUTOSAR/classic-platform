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

typedef enum
{
  DMA_DSPI_A_COMMAND_CHANNEL,     /* 0 */
  DMA_DSPI_A_RESULT_CHANNEL,
  DMA_DSPI_B_COMMAND_CHANNEL,     /* 1 */
  DMA_DSPI_B_RESULT_CHANNEL,
  DMA_DSPI_C_COMMAND_CHANNEL,     /* 2 */
  DMA_DSPI_C_RESULT_CHANNEL,
  DMA_DSPI_D_COMMAND_CHANNEL,     /* 3 */
  DMA_DSPI_D_RESULT_CHANNEL,
  DMA_DSPI_E_COMMAND_CHANNEL,     /* 4 */
  DMA_DSPI_E_RESULT_CHANNEL,
  DMA_DSPI_F_COMMAND_CHANNEL,     /* 5 */
  DMA_DSPI_F_RESULT_CHANNEL,

  DMA_ADC_GROUP0_RESULT_CHANNEL,

  DMA_NUMBER_OF_CHANNELS
} Dma_ChannelType;

#define DMA_START_CHANNEL	DMA_DSPI_A_COMMAND_CHANNEL

#endif /* DMA_CFG_H_ */
