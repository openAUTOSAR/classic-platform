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








#ifndef SPI_H_
#define SPI_H_

#include "Std_Types.h"

/* Standard info */
#define SPI_VENDOR_ID             1
#define SPI_MODULE_ID         		 MODULE_ID_SPI
#define SPI_SW_MAJOR_VERSION      1
#define SPI_SW_MINOR_VERSION      0
#define SPI_SW_PATCH_VERSION      2
#define SPI_AR_MAJOR_VERSION      2
#define SPI_AR_MINOR_VERSION      2
#define SPI_AR_PATCH_VERSION      2


/* --- Service IDs --- */
#define SPI_INIT_SERVICE_ID               0x00
#define SPI_DEINIT_SERVICE_ID             0x01
#define SPI_WRITEIB_SERVICE_ID            0x02
#define SPI_ASYNCTRANSMIT_SERVICE_ID      0x03
#define SPI_READIB_SERVICE_ID             0x04
#define SPI_SETUPEB_SERVICE_ID            0x05
#define SPI_GETSTATUS_SERVICE_ID          0x06
#define SPI_GETJOBRESULT_SERVICE_ID       0x07
#define SPI_GETSEQUENCERESULT_SERVICE_ID  0x08
#define SPI_GETVERSIONINFO_SERVICE_ID     0x09
#define SPI_SYNCTRANSMIT_SERVICE_ID       0x0A
#define SPI_GETHWUNITSTATUS_SERVICE_ID    0x0B
#define SPI_CANCEL_SERVICE_ID             0x0C
#define SPI_SETASYNCMODE_SERVICE_ID       0x0D

/* --- Error codes --- */
#define SPI_E_PARAM_CHANNEL               0x0A
#define SPI_E_PARAM_JOB                   0x0B
#define SPI_E_PARAM_SEQ                   0x0C
#define SPI_E_PARAM_LENGTH                0x0D
#define SPI_E_PARAM_UNIT                  0x0E
#define SPI_E_UNINIT                      0x1A
#define SPI_E_SEQ_PENDING                 0x2A
#define SPI_E_SEQ_IN_PROCESS              0x3A
#define SPI_E_ALREADY_INITIALIZED         0x4A

typedef enum {
	SPI_UNINIT=0, // The SPI Handler/Driver is not initialized or not usable.
				// SPI011: This shall be the default value after reset. This
    			// status shall have the value 0.

	SPI_IDLE,	// The SPI Handler/Driver is not currently transmitting any
    			// Job.

	SPI_BUSY, 	// The SPI Handler/Driver is performing a SPI Job( transmit )
} Spi_StatusType;

typedef enum {
	SPI_JOB_OK=0, 		// The last transmission of the Job has been finished
    					// successfully.
    					// SPI012: This shall be the default value after reset.
    					// This status shall have the value 0.

	SPI_JOB_PENDING,	// The SPI Handler/Driver is performing a SPI Job.
						// The meaning of this status is equal to SPI_BUSY.

	SPI_JOB_FAILED, 	// The last transmission of the Job has failed.
} Spi_JobResultType;

typedef enum {

	SPI_SEQ_OK, 	//	The last transmission of the Sequence has been
					//    finished successfully.
					//    SPI017: This shall be the default value after reset.
					//    This status shall have the value 0.

	SPI_SEQ_PENDING, // The SPI Handler/Driver is performing a SPI
    			// Sequence. The meaning of this status is equal to
    			// SPI_BUSY.

	SPI_SEQ_FAILED, 	// The last transmission of the Sequence has failed.

	SPI_SEQ_CANCELLED, 	// The last transmission of the Sequence has been
					//    cancelled by user.
} Spi_SeqResultType;


// Type for defining the number of data elements of the type Spi_DataType to
// send and / or receive by Channel
typedef uint16 Spi_NumberOfDataType;


// Specifies the asynchronous mechanism mode for SPI busses handled
// asynchronously in LEVEL 2.
// SPI150: This type is available or not accordint to the pre compile time parameter:
// SPI_LEVEL_DELIVERED. This is only relevant for LEVEL 2.

typedef enum {

	//	The asynchronous mechanism is ensured by
	//    polling, so interrupts related to SPI busses
	//    handled asynchronously are disabled.
	SPI_POLLING_MODE,

	//    The asynchronous mechanism is ensured by
	//    interrupt, so interrupts related to SPI busses
	//    handled asynchronously are enabled.

	SPI_INTERRUPT_MODE,
} Spi_AsyncModeType;


#include "Spi_Cfg.h"

void Spi_Init( const Spi_ConfigType *ConfigPtr );
Std_ReturnType Spi_DeInit( void );
Std_ReturnType Spi_WriteIB( Spi_ChannelType Channel, const Spi_DataType *DataBufferPtr );
Std_ReturnType Spi_AsyncTransmit( Spi_SequenceType Sequence );
Std_ReturnType Spi_ReadIB( Spi_ChannelType Channel, Spi_DataType *const DataBufferPtr ) ;
Std_ReturnType Spi_SetupEB( Spi_ChannelType Channel,
							const Spi_DataType*  SrcDataBufferPtr,
	     					Spi_DataType* DesDataBufferPtr,
	     					Spi_NumberOfDataType Length );

Spi_StatusType Spi_GetStatus( void );
Spi_JobResultType Spi_GetJobResult ( Spi_JobType       Job );
Spi_SeqResultType Spi_GetSequenceResult(Spi_SequenceType Sequence );

#if ( SPI_VERSION_INFO_API == STD_ON  )
#define Spi_GetVersionInfo(_vi) STD_GET_VERSION_INFO(_vi,SPI)
#endif

Std_ReturnType Spi_SyncTransmit( Spi_SequenceType        Sequence );
Spi_StatusType Spi_GetHWUnitStatus(Spi_HWUnitType        HWUnit);

#if (SPI_CANCEL_API == STD_ON )
void Spi_Cancel( Spi_SequenceType Sequence );
#endif

Std_ReturnType Spi_SetAsyncMode( Spi_AsyncModeType        Mode );
void Spi_MainFunction_Handling( void );
void Spi_MainFunction_Driving( void );

#endif /*SPI_H_*/
