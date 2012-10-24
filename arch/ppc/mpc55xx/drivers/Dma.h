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


/*
 * Freescale uses two flavors for DMA.
 * 1. eDMA
 * 2. eDMA + DMA_MUX
 *
 * 1. eDMA only (MPC5557, etc)
 *   The "DMA Request Assignments" are used and configured in Dma_Cfg.h using Dma_ChannelType.
 *
 * 2. eDMA + DMA_MUX  (MPC551x , MPC5668, etc )
 *   The eDMA + DMA_MUX the "DMA Request Assignments" are just mappings from the DMA_MUX.
 *   The file Dma.h contains the Dma_MuxChannels
 *
 *
 *  eDMA+DMA_MUX
 *    MPC551x
 *    MPC5668
 *    MPC5605B,MPC5606B,MPC5607B
 *
 *  eDMA
 *    MPC5567
 *
 *  NO DMA
 *    MPC5604B
 *
 */

#ifndef DMA_H_
#define DMA_H_


#include "Std_Types.h"
#include "Dma_Cfg.h"
#include "mpc55xx.h"

#if defined(CFG_MPC5516) || defined(CFG_MPC5517) || (CFG_MPC5606S) || defined(CFG_MPC5668) || (CFG_MPC5606B) || (CFG_MPC5604P)
#if !defined(CFG_DMA_MUX)
#define CFG_DMA_MUX
#endif
#endif

#if defined(CFG_DMA_MUX)

#if defined(CFG_MPC5606S) || (CFG_MPC5606B) || (CFG_MPC5604P)
typedef enum
{
  DMA_CHANNEL_DISABLED,

  DMA_DSPI_0_TX,
  DMA_DSPI_0_RX,
  DMA_DSPI_1_TX,
  DMA_DSPI_1_RX,

  DMA_QuadSPI_0_TFFF,
  DMA_QuadSPI_0_RFDF,

  DMA_I2C_0_TX,
  DMA_I2C_0_RX,
  DMA_I2C_1_TX,
  DMA_I2C_1_RX,
  DMA_I2C_2_TX,
  DMA_I2C_2_RX,
  DMA_I2C_3_TX,
  DMA_I2C_3_RX,

  DMA_EMIOS200_0_FLAG_F0,
  DMA_EMIOS200_0_FLAG_F1,
  DMA_EMIOS200_0_FLAG_F2,
  DMA_EMIOS200_0_FLAG_F3,
  DMA_EMIOS200_0_FLAG_F4,
  DMA_EMIOS200_0_FLAG_F5,
  DMA_EMIOS200_0_FLAG_F6,
  DMA_EMIOS200_0_FLAG_F7,
  DMA_EMIOS200_0_FLAG_F8,
  DMA_EMIOS200_0_FLAG_F9,
  DMA_EMIOS200_0_FLAG_F10,
  DMA_EMIOS200_0_FLAG_F11,
  DMA_EMIOS200_0_FLAG_F12,
  DMA_EMIOS200_0_FLAG_F13,
  DMA_EMIOS200_0_FLAG_F14,
  DMA_EMIOS200_0_FLAG_F15,
  DMA_EMIOS200_1_FLAG_F0,
  DMA_EMIOS200_1_FLAG_F1,
  DMA_EMIOS200_1_FLAG_F2,
  DMA_EMIOS200_1_FLAG_F3,
  DMA_EMIOS200_1_FLAG_F4,
  DMA_EMIOS200_1_FLAG_F5,
  DMA_EMIOS200_1_FLAG_F6,
  DMA_EMIOS200_1_FLAG_F7,

  DMA_RESERVED1,
  DMA_RESERVED2,
  DMA_RESERVED3,
  DMA_RESERVED4,
  DMA_RESERVED5,
  DMA_RESERVED6,
  DMA_RESERVED7,
  DMA_RESERVED8,

  DMA_SIU_EISR_E1F1,
  DMA_SIU_EISR_E1F2,
  DMA_SIU_EISR_E1F3,
  DMA_SIU_EISR_E1F4,

  DMA_ADC,

  DMA_RESERVED9,

  DMA_DCU,

  DMA_RESERVED10,
  DMA_RESERVED11,

  DMA_ALWAYS_REQUESTORS1,
  DMA_ALWAYS_REQUESTORS2,
  DMA_ALWAYS_REQUESTORS3,
  DMA_ALWAYS_REQUESTORS4,
  DMA_ALWAYS_REQUESTORS5,
  DMA_ALWAYS_REQUESTORS6,
  DMA_ALWAYS_REQUESTORS7,
  DMA_ALWAYS_REQUESTORS8
}Dma_MuxChannels;

#elif defined(CFG_MPC5668)

/* Table 22-4. DMA Source Configuration */

typedef enum
{
	DMA_CHANNEL_DISABLED,	/* 0 */
	DMA_CHANNEL_RESERVED,
	DMA_SCI_A_COMBTX,
	DMA_SCI_A_COMBRX,
	DMA_SCI_B_COMBTX,
	DMA_SCI_B_COMBRX,
	DMA_SCI_C_COMBTX,
	DMA_SCI_C_COMBRX,
	DMA_SCI_D_COMBTX,
	DMA_SCI_D_COMBRX,
	DMA_SCI_E_COMBTX,
	DMA_SCI_E_COMBRX,
	DMA_SCI_F_COMBTX,
	DMA_SCI_F_COMBRX,
	DMA_SCI_G_COMBTX,
	DMA_SCI_G_COMBRX,
	DMA_SCI_H_COMBTX,
	DMA_SCI_H_COMBRX,

	DMA_DSPI_A_SR_TFFF,  /* 0x12 */
	DMA_DSPI_A_SR_RFRD,
	DMA_DSPI_B_SR_TFFF,
	DMA_DSPI_B_SR_RFRD,
	DMA_DSPI_E_SR_TFFF,
	DMA_DSPI_E_SR_RFRD,
	DMA_DSPI_F_SR_TFFF,
	DMA_DSPI_F_SR_RFRD,

	DMA_EMIOS200_FLAG_F0, /* 0x1a */
	DMA_EMIOS200_FLAG_F1,
	DMA_EMIOS200_FLAG_F2,
	DMA_EMIOS200_FLAG_F3,
	DMA_EMIOS200_FLAG_F4,
	DMA_EMIOS200_FLAG_F5,
	DMA_EMIOS200_FLAG_F6,
	DMA_EMIOS200_FLAG_F7,
	DMA_EMIOS200_FLAG_F8,
	DMA_EMIOS200_FLAG_F9,
	DMA_EMIOS200_FLAG_F10,
	DMA_EMIOS200_FLAG_F11,
	DMA_EMIOS200_FLAG_F12,
	DMA_EMIOS200_FLAG_F13,
	DMA_EMIOS200_FLAG_F14,
	DMA_EMIOS200_FLAG_F15,

	DMA_IIC_A_TX,  /* 0x2a */
	DMA_IIC_A_RX,
	DMA_IIC_B_TX,
	DMA_IIC_B_RX,

	DMA_SIU_EISR_EIF0, /* 0x2e */
	DMA_SIU_EISR_EIF1,

	DMA_IIC_C_TX, /* 0x30 */
	DMA_IIC_C_RX,

	DMA_ADC_A,

	DMA_IIC_D_TX, /* 0x33 */
	DMA_IIC_D_RX,

	DMA_SCI_J_COMBTX, /* 0x35 */
	DMA_SCI_J_COMBRX,
	DMA_SCI_K_COMBTX,
	DMA_SCI_K_COMBRX,
	DMA_SCI_L_COMBTX,
	DMA_SCI_L_COMBRX,
	DMA_SCI_M_COMBTX,
	DMA_SCI_M_COMBRX,

	DMA_ALWAYS_ENABLED_0, /* 0x3d */
	DMA_ALWAYS_ENABLED_1,
	DMA_ALWAYS_ENABLED_2,
} Dma_MuxChannels;

#elif  defined(CFG_MPC5516) || defined(CFG_MPC5517)
/* MPC551x "Table 13-4. DMA Source Configuration" */

typedef enum
{
  DMA_CHANNEL_DISABLED,
  DMA_SCI_A_COMBTX,
  DMA_SCI_A_COMBRX,
  DMA_SCI_B_COMBTX,
  DMA_SCI_B_COMBRX,
  DMA_SCI_C_COMBTX,
  DMA_SCI_C_COMBRX,
  DMA_SCI_D_COMBTX,
  DMA_SCI_D_COMBRX,
  DMA_SCI_E_COMBTX,
  DMA_SCI_E_COMBRX,
  DMA_SCI_F_COMBTX,
  DMA_SCI_F_COMBRX,
  DMA_SCI_G_COMBTX,
  DMA_SCI_G_COMBRX,
  DMA_SCI_H_COMBTX,
  DMA_SCI_H_COMBRX,

  DMA_DSPI_A_SR_TFFF,
  DMA_DSPI_A_SR_RFRD,
  DMA_DSPI_B_SR_TFFF,
  DMA_DSPI_B_SR_RFRD,
  DMA_DSPI_C_SR_TFFF,
  DMA_DSPI_C_SR_RFRD,
  DMA_DSPI_D_SR_TFFF,
  DMA_DSPI_D_SR_RFRD,

  DMA_EMIOS200_FLAG_F0,
  DMA_EMIOS200_FLAG_F1,
  DMA_EMIOS200_FLAG_F2,
  DMA_EMIOS200_FLAG_F3,
  DMA_EMIOS200_FLAG_F4,
  DMA_EMIOS200_FLAG_F5,
  DMA_EMIOS200_FLAG_F6,
  DMA_EMIOS200_FLAG_F7,
  DMA_EMIOS200_FLAG_F8,
  DMA_EMIOS200_FLAG_F9,
  DMA_EMIOS200_FLAG_F10,
  DMA_EMIOS200_FLAG_F11,
  DMA_EMIOS200_FLAG_F12,
  DMA_EMIOS200_FLAG_F13,
  DMA_EMIOS200_FLAG_F14,
  DMA_EMIOS200_FLAG_F15,

  DMA_IIC_A_TX,
  DMA_IIC_A_RX,

  DMA_RESERVED1,
  DMA_RESERVED2,

  DMA_SIU_EISR_EIF1,
  DMA_SIU_EISR_EIF2,
  DMA_SIU_EISR_EIF3,
  DMA_SIU_EISR_EIF4,

  DMA_EQADC_FISR0_RFDF0,
  DMA_EQADC_FISR0_CFFF0,
  DMA_EQADC_FISR1_RFDF0,
  DMA_EQADC_FISR1_CFFF0,

  DMA_MLB_DMA_REQ,

  DMA_RESERVED3,
  DMA_RESERVED4,

  DMA_ALWAYS_ENABLED1,
  DMA_ALWAYS_ENABLED2,
  DMA_ALWAYS_ENABLED3,
  DMA_ALWAYS_ENABLED4,
  DMA_ALWAYS_ENABLED5,
  DMA_ALWAYS_ENABLED6,
  DMA_ALWAYS_ENABLED7,
  DMA_ALWAYS_ENABLED8
}Dma_MuxChannels;

#endif
#endif


#if defined(CFG_DMA_MUX)
typedef struct
{
  vuint8_t DMA_CHANNEL_ENABLE;
  vuint8_t DMA_CHANNEL_TRIG_ENABLE;
  Dma_MuxChannels DMA_CHANNEL_SOURCE;
} Dma_MuxConfigType;
#endif

typedef struct
{
  vuint8_t DMA_CHANNEL_PRIORITY;
  vuint8_t DMA_CHANNEL_PREEMTION_ENABLE;
}Dma_ChannelConfigType;

typedef enum
{
  DMA_TRANSFER_SIZE_8BITS,
  DMA_TRANSFER_SIZE_16BITS,
  DMA_TRANSFER_SIZE_32BITS,
  DMA_TRANSFER_SIZE_64BITS,
  DMA_TRANSFER_SIZE_16BYTES_BURST,
  DMA_TRANSFER_SIZE_32BYTES_BURST
}Dma_DataTranferSizeType;

typedef enum
{
  DMA_FIXED_PRIORITY_ARBITRATION,
  DMA_ROUND_ROBIN_ARBITRATION
}Dma_ChannelArbitrationType;

typedef struct
{
	// 5567 has no Dma Mux, but maybe this should be left in anyway?
#if defined(CFG_DMA_MUX)
  const Dma_MuxConfigType          *dmaMuxConfigPtr;
#endif
  const Dma_ChannelConfigType      *dmaChannelConfigPtr;
  const Dma_ChannelArbitrationType  dmaChannelArbitration;
}Dma_ConfigType;

extern const Dma_ConfigType DmaConfig [];


void Dma_Init (const Dma_ConfigType *ConfigPtr);
void Dma_DeInit (void );
void Dma_ConfigureChannel (Dma_TcdType *tcd, Dma_ChannelType channel);
void Dma_ConfigureChannelTranferSize (uint32_t nbrOfIterations, Dma_ChannelType channel);
void Dma_ConfigureChannelSourceCorr (uint32_t sourceCorrection, Dma_ChannelType channel);
void Dma_ConfigureChannelDestinationCorr (uint32_t destinationCorrection, Dma_ChannelType channel);
void Dma_ConfigureDestinationAddress (uint32_t destAddr, Dma_ChannelType channel);
void Dma_ConfigureSourceAddress (uint32_t sourceAddr, Dma_ChannelType channel);
void Dma_StartChannel (Dma_ChannelType channel);
void Dma_StopChannel (Dma_ChannelType channel);
Std_ReturnType Dma_ChannelDone (Dma_ChannelType channel);
volatile Dma_TcdType * Dma_GetTcd( Dma_ChannelType channel );
boolean Dma_CheckConfig( void );

#endif /* DMA_H_ */
