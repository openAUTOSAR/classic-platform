/*
 * Adc_conversion.c
 *
 *  Created on: 8 sep 2014
 *      Author: sse
 */
#include "Platform_Types.h"

#include "Spi.h"
#include "bcm2835.h"
#include "Sensors.h"

/**
 * Read ADC-converted data (analog input voltage) from one of MCP3008's channels
 *
 * @param channel		    ----- the channel nr to read from (0-7)
 * @param channel		    ----- pointer for storing the output data (analog input voltage)
 */
void Mcp3008_Read(uint8 channel, uint32 *data)
{
	/* SPI receive buffer */
	uint8 rxbuf[2];

	/* SPI transmit buffers */
	uint8 START = BIT(0);										// Set MCP3008's start-of-transmission bit
    uint8 CONFIG = BIT(7) | (((channel & 7) << 4));				// Choose single-ended conversion and set channel id

    /* Setup necessary external buffers for the SPI read sequence (see Spi_Lcfg.c) */
    Spi_SetupEB(SPI_CH_CMD, &START, NULL, 1);					// Set up the start bit
	Spi_SetupEB(SPI_CH_ADDR, &CONFIG, &rxbuf[0], 1);			// Configure the ADC device and prepare for the response to be placed in rx-buffer
 	Spi_SetupEB(SPI_CH_DATA, NULL, &rxbuf[1], 1);				// On this channel, wait for the next response byte

 	/* Transmit all jobs and channels in the SPI read sequence and collect the response data */
 	Spi_SyncTransmit(SPI_SEQ_READ);

 	/* The relevant output from MCP3008 is 10 bit long, convert it to decimal */
 	*data = ((rxbuf[0] & 3) << 8) + rxbuf[1];
}
