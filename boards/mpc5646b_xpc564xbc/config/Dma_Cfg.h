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

/* enum is used to initialize DMA channels NOT DMA MUX-es 
5646B/C has 32 DMA channels
*/

typedef enum
{
  DMA_CHANNEL_0,     /* 0 */
  DMA_CHANNEL_1,     /* 1 */
  DMA_CHANNEL_2,     /* 2 */
  DMA_CHANNEL_3,     /* 3 */
  DMA_CHANNEL_4,     /* 4 */
  DMA_CHANNEL_5,     /* 5 */
  DMA_CHANNEL_6,     /* 6 */
  DMA_CHANNEL_7,     /* 7 */
  DMA_CHANNEL_8,     /* 8 */
  DMA_CHANNEL_9,     /* 9 */
  DMA_CHANNEL_10,    /* 10 */
  DMA_CHANNEL_11,    /* 11 */
  DMA_CHANNEL_12,    /* 12 */
  DMA_CHANNEL_13,    /* 13 */
  DMA_CHANNEL_14,    /* 14 */
  DMA_CHANNEL_15,    /* 15 */
  DMA_CHANNEL_16,    /* 16 */
  DMA_CHANNEL_17,    /* 17 */
  DMA_CHANNEL_18,    /* 18 */
  DMA_CHANNEL_19,    /* 19 */
  DMA_CHANNEL_20,    /* 20 */
  DMA_CHANNEL_21,    /* 21 */
  DMA_CHANNEL_22,    /* 22 */
  DMA_CHANNEL_23,    /* 23 */
  DMA_CHANNEL_24,    /* 24 */
  DMA_CHANNEL_25,    /* 25 */
  DMA_CHANNEL_26,    /* 26 */
  DMA_CHANNEL_27,    /* 27 */
  DMA_CHANNEL_28,    /* 28 */
  DMA_CHANNEL_29,    /* 29 */
  DMA_CHANNEL_30,    /* 30 */
  DMA_CHANNEL_31,    /* 31 */
  DMA_NUMBER_OF_CHANNELS
} Dma_ChannelType;

#define DMA_START_CHANNEL DMA_CHANNEL_0

#endif /* DMA_CFG_H_ */
