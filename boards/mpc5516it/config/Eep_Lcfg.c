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


/* Configured for:
 *   Microchip 25LC160B (32 bytes pages)
 */


#include "Eep.h"
#include "Spi.h"
#include "debug.h"


static void _JobEndNotify(){
	DEBUG(DEBUG_LOW,"EEP JOB END NOTIFICATION\n");
}
static void _JobErrorNotify(){
	DEBUG(DEBUG_LOW,"EEP JOB ERROR NOTIFICATION\n");
}

/*
 * TODO: probably better to
 */
#define SPI_SEQ_EEP_CMD		SPI_SEQ_CMD
#define SPI_SEQ_EEP_CMD2	SPI_SEQ_CMD2
#define SPI_SEQ_EEP_READ	SPI_SEQ_READ
#define SPI_SEQ_EEP_WRITE	SPI_SEQ_WRITE

#define SPI_CH_EEP_CMD		SPI_CH_CMD
#define SPI_CH_EEP_ADDR		SPI_CH_ADDR
#define SPI_CH_EEP_WREN		SPI_CH_WREN
#define SPI_CH_EEP_DATA		SPI_CH_DATA


const Eep_ExternalDriverType EepExternalDriver = {
	// READ and WRITE sequences and ID's defined in Spi_Cfg.h
	.EepCmdSequence = 	SPI_SEQ_EEP_CMD,
	.EepCmd2Sequence = 	SPI_SEQ_EEP_CMD2,
	.EepReadSequence = 	SPI_SEQ_EEP_READ,
	.EepWriteSequence = SPI_SEQ_EEP_WRITE,

	// Jobs may be left out..

	// Channels used
	.EepCmdChannel	= SPI_CH_EEP_CMD,
	.EepAddrChannel	= SPI_CH_EEP_ADDR,
	.EepWrenChannel	= SPI_CH_EEP_WREN,
	.EepDataChannel	= SPI_CH_EEP_DATA,
};

const Eep_ConfigType EepConfigData[] = {
    {
    // call cycle of the job processing function during write/erase operations. Unit: [s]
//    .EepJobCallCycle = 0.2,
    // This parameter is the EEPROM device base address.
    .EepBaseAddress =  0,

    // This parameter is the default EEPROM device mode after initialization.
    .EepDefaultMode = MEMIF_MODE_FAST,

    // This parameter is the number of bytes read within one job processing cycle in fast mode
    .EepFastReadBlockSize = 64,

    // This parameter is the number of bytes written within one job processing cycle in fast mode
    .EepFastWriteBlockSize = 64,

    // This parameter is a reference to a callback function for positive job result
    .Eep_JobEndNotification = &_JobEndNotify,

    // This parameter is a reference to a callback function for negative job result
    .Eep_JobErrorNotification = &_JobErrorNotify,

    .EepNormalReadBlockSize = 4,

    // Number of bytes written within one job processing cycle in normal mode.
    .EepNormalWriteBlockSize = 1,

    // This parameter is the used size of EEPROM device in bytes.
    .EepSize = 0x1000,	/* 32K bit for M9525, 16K bit 25LC160B*/

    .EepPageSize = 64,	/* 64 for M9525, 32 for 25LC160B, 16 for 25LC160A */

    .externalDriver =  &EepExternalDriver,
    }
};
