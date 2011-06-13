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


#ifndef SPI_CFG_H_
#define SPI_CFG_H_

#include "Dma.h"
#include "mpc55xx.h"
#include "Mcu.h"

#define DSPI_CTRL_A	0
#define DSPI_CTRL_B	1
#define DSPI_CTRL_C	2
#define DSPI_CTRL_D	3

/*
 * General configuration
 */

// Maximum amount of data that can be written/read in one go.
#define SPI_EB_MAX_LENGTH					64

// Switches the Spi_Cancel function ON or OFF.
#define SPI_CANCEL_API						STD_OFF

// Selects the SPI Handler/Driver Channel Buffers usage allowed and delivered.
// LEVEL 0 - Only Internal buffers
// LEVEL 1 - Only external buffers
// LEVEL 2 - Both internal/external buffers
#define SPI_CHANNEL_BUFFERS_ALLOWED	1

#define SPI_DEV_ERROR_DETECT 			STD_ON
// Switches the Spi_GetHWUnitStatus function ON or OFF.
#define SPI_HW_STATUS_API					STD_ON
// Switches the Interruptible Sequences handling functionality ON or OFF.
#define SPI_INTERRUPTIBLE_SEQ_ALLOWED		STD_OFF

// LEVEL 0 - Simple sync
// LEVEL 1 - Basic async
// LEVEL 2 - Enhanced mode
#define SPI_LEVEL_DELIVERED				2

#define SPI_VERSION_INFO_API				STD_ON

#if 0
#if SPI_LEVEL_DELIVERED>=1
#define SPI_INTERRUPTIBLE_SEQ_ALLOWED	STD_ON
#endif
#endif

typedef enum {
	SPI_EXT_DEVICE_A_E2,
} Spi_ExternalDeviceTypeType;

#define	SPI_CH_E2_CMD 		0
#define	SPI_CH_E2_ADDR 		1
#define	SPI_CH_E2_WREN		2
#define SPI_CH_E2_DATA		3

#define SPI_JOB_E2_CMD		0
#define SPI_JOB_E2_CMD2		1
#define SPI_JOB_E2_DATA		2
#define SPI_JOB_E2_WREN		3

#define SPI_SEQ_E2_CMD		0
#define SPI_SEQ_E2_CMD2		1
#define SPI_SEQ_E2_READ		2
#define SPI_SEQ_E2_WRITE	3

#define SPI_MAX_JOB			4
#define SPI_MAX_CHANNEL		4
#define SPI_MAX_SEQUENCE	4


#endif /*SPI_CFG_H_*/
