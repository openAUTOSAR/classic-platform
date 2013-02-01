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








#include <assert.h>

#include "Std_Types.h"
#include "mpc55xx.h"
#include "Mcu.h"
#include "Dma.h"

void Dma_Init (const Dma_ConfigType *ConfigPtr)
{
  Dma_ChannelType channel;
  for (channel = (Dma_ChannelType)0; channel < DMA_NUMBER_OF_CHANNELS; channel++)
  {
#if defined(CFG_MPC5516) || defined(CFG_MPC5517) || defined(CFG_MPC5606S)
    /* DMA mux configuration. */
    DMAMUX.CHCONFIG[channel].B.ENBL = ConfigPtr->dmaMuxConfigPtr[channel].DMA_CHANNEL_ENABLE;
    DMAMUX.CHCONFIG[channel].B.TRIG = ConfigPtr->dmaMuxConfigPtr[channel].DMA_CHANNEL_TRIG_ENABLE;
    DMAMUX.CHCONFIG[channel].B.SOURCE = ConfigPtr->dmaMuxConfigPtr[channel].DMA_CHANNEL_SOURCE;
#endif

    /* DMA channel configration. */
    EDMA.CPR[channel].B.ECP = ConfigPtr->dmaChannelConfigPtr[channel].DMA_CHANNEL_PREEMTION_ENABLE;
    EDMA.CPR[channel].B.CHPRI = ConfigPtr->dmaChannelConfigPtr[channel].DMA_CHANNEL_PRIORITY;
  }

  /* Write channel arbitration mode. */
  EDMA.CR.B.ERCA = ConfigPtr->dmaChannelArbitration;
}

void Dma_DeInit( void ) {
	Dma_ChannelType channel;
	for (channel = (Dma_ChannelType)0; channel < DMA_NUMBER_OF_CHANNELS; channel++)
	{
		Dma_StopChannel(channel);
	}
}


void Dma_ConfigureChannel (Dma_TcdType *tcd, Dma_ChannelType channel)
{
  /* Copy transfer configuration to correct channel. */
  EDMA.TCD[channel] = *tcd;

  /* Disable channel. */
  EDMA.CERQ.R = channel;

  /* Check configuration. */
  if (EDMA.ES.B.VLD)
  {
    assert(0);
  }
  else
  {
    /* Configuration seems to be OK. Do nothing. */
  }
}

volatile Dma_TcdType * Dma_GetTcd( Dma_ChannelType channel ) {
		return &EDMA.TCD[channel];
}

boolean Dma_CheckConfig( void ) {
  /* Check configuration. */
  if (EDMA.ES.B.VLD)
  {
    assert(0);
  }
  else
  {
    /* Configuration seems to be OK. Do nothing. */
  }
  return TRUE;
}

void Dma_ConfigureChannelTranferSize (uint32_t nbrOfIterations, Dma_ChannelType channel)
{
  EDMA.TCD[channel].BITER = nbrOfIterations;
  EDMA.TCD[channel].CITER = nbrOfIterations;
}

void Dma_ConfigureChannelSourceCorr (uint32_t sourceCorrection, Dma_ChannelType channel)
{
  EDMA.TCD[channel].SLAST = sourceCorrection;
}

void Dma_ConfigureChannelDestinationCorr (uint32_t destinationCorrection, Dma_ChannelType channel)
{
  EDMA.TCD[channel].DLAST_SGA = destinationCorrection;
}

void Dma_ConfigureDestinationAddress (uint32_t destAddr, Dma_ChannelType channel)
{
  EDMA.TCD[channel].DADDR = destAddr;
}

void Dma_ConfigureSourceAddress (uint32_t sourceAddr, Dma_ChannelType channel)
{
  EDMA.TCD[channel].SADDR = sourceAddr;
}

int Dma_Active(Dma_ChannelType channel) {
	return EDMA.TCD[channel].ACTIVE;
}

void Dma_StartChannel (Dma_ChannelType channel)
{
  /* Start the channel... */
  EDMA.SERQ.R = channel;
}

#if 0
void Dma_EnableInterrupt (Dma_ChannelType channel)
{
  /* Start the channel... */
  EDMA.IRQRL.R = (1<<channel);
}
#endif

void Dma_ClearInterrupt (Dma_ChannelType channel)
{
  /* Start the channel... */
  //EDMA.CIRQ.R = channel;
  EDMA.CINT.R = channel;
}



void Dma_StopChannel (Dma_ChannelType channel)
{
  /* Stop the channel... */
  EDMA.CERQ.R = channel;
}

Std_ReturnType Dma_ChannelDone (Dma_ChannelType channel)
{
  return (Std_ReturnType)EDMA.TCD[channel].DONE;
}

