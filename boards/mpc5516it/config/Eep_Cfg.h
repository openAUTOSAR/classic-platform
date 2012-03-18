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


#ifndef EEP_CFG_H_
#define EEP_CFG_H_


#define EEP_USES_EXTERNAL_DRIVER

#if defined(USE_SPI)
#include "Spi.h"
#endif
#include "Eep_ConfigTypes.h"


// M95256 or 25LC160B
#define E2_WREN   0x6		// Write Enable 0000 0110
#define E2_WRDI   0x4		// Write Disable 0000 0100
#define E2_RDSR   0x5		// Read Status Register  0000 0101
													// 1 - Read data
#define E2_WRSR   0x1		// Write Status Register  0000 0001
													// 1 - Write data
#define E2_READ   0x3		// Read from Memory Array 0000 0011
													// 1  - Write 16-bit address
													// n  - 8 -bit read data
#define E2_WRITE  0x2		// WRITE  Write to Memory Array  0000 0010
													// 1  Write 16-bit address
													// n  - 8-bit reads

/* EepGeneral */

// Switches to activate or deactivate interrupt controlled job processing. true:
// Interrupt controlled job processing enabled. false: Interrupt controlled job
// processing disabled.
#define EEP_USE_INTERRUPTS			      STD_OFF

// Pre-processor switch to enable and disable development error detection.
// true: Development error detection enabled. false: Development error
// detection disabled.
#define EEP_DEV_ERROR_DETECT		      STD_ON

// Pre-processor switch to enable / disable the API to read out the modules
// version information. true: Version info API enabled. false: Version info API
// disabled.
#define EEP_VERSION_INFO_API		      STD_ON

// ndex of the driver, used by EA.
#define EEP_DRIVER_INDEX			        1

// Switches to activate or deactivate write cycle reduction (EEPROM value is
// read and compared before being overwritten). true: Write cycle reduction
// enabled. false: Write cycle reduction disabled.
#define EEP_WRITE_CYCLE_REDUCTION	  STD_OFF

// Container for runtime configuration parameters of the EEPROM driver.
// Implementation Type: Eep_ConfigType.

/*  EepPublishedInformation
 */

// Total size of EEPROM in bytes. Implementation Type: Eep_LengthType.
#define EEP_TOTAL_SIZE				TBD

// Size of smallest erasable EEPROM data unit in bytes.
#define EEP_ERASE_UNIT_SIZE 		TBD

// EepMinimumLengthType {EEP_MINIMUM_LENGTH_TYPE}
// Minimum expected size of Eep_LengthType.
#define EEP_MINIMUM_LENGTH_TYPE 	TBD

// Minimum expected size of Eep_AddressType.
#define EEP_MINIMUM_ADDRESS_TYPE 	TBD

// Size of smallest writable EEPROM data unit in bytes.
#define EEP_WRITE_UNIT_SIZE 		TBD

// Value of an erased EEPROM cell.
#define EEP_ERASE_VALUE 			0

// Number of erase cycles specified for the EEP device (usually given in the
// device data sheet).
#define EEP_SPECIFIED_ERASE_CYCLES 	TBD

// Size of smallest readable EEPROM data unit in bytes.
#define EEP_READ_UNIT_SIZE 			TBD

// Time for writing one EEPROM data unit.(float)
#define EEP_WRITE_TIME				TBD

// Time for erasing one EEPROM data unit (float)
#define EEP_ERASE_TIME				TBD

// Specified maximum number of write cycles under worst case conditions of
// specific EEPROM hardware (e.g. +90°C)
#define EEP_ALLOWED_WRITE_CYCLES x


extern const Eep_ConfigType EepConfigData[];

#define EEP_DEFAULT_CONFIG EepConfigData[0]

#endif /*EEP_CFG_H_*/
