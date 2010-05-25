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

#include "Spi.h"

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
 *
 * TODO
 *
 * 3.0 additions?
 */

// Total size of EEPROM in bytes. Implementation Type: Eep_LengthType.
#define EEP_TOTAL_SIZE		TBD

// Size of smallest erasable EEPROM data unit in bytes.
#define EEP_ERASE_UNIT_SIZE TBD

// EepMinimumLengthType {EEP_MINIMUM_LENGTH_TYPE}
// Minimum expected size of Eep_LengthType.
#define EEP_MINIMUM_LENGTH_TYPE TBD

// Minimum expected size of Eep_AddressType.
#define EEP_MINIMUM_ADDRESS_TYPE TBD

// Size of smallest writable EEPROM data unit in bytes.
#define EEP_WRITE_UNIT_SIZE TBD

// Value of an erased EEPROM cell.
#define EEP_ERASE_VALUE 		0

// Number of erase cycles specified for the EEP device (usually given in the
// device data sheet).
#define EEP_SPECIFIED_ERASE_CYCLES TBD

// Size of smallest readable EEPROM data unit in bytes.
#define EEP_READ_UNIT_SIZE TBD

// Time for writing one EEPROM data unit.
#define EEP_WRITE_TIME	TBD

// Time for erasing one EEPROM data unit
#define EEP_ERASE_TIME	TBD

// Specified maximum number of write cycles under worst case conditions of
// specific EEPROM hardware (e.g. +90°C)
#define EEP_ALLOWED_WRITE_CYCLES x


typedef struct {

  /* EEP094 */

  Spi_SequenceType EepCmdSequence;
  Spi_SequenceType EepCmd2Sequence;
  Spi_SequenceType EepReadSequence;
  Spi_SequenceType EepWriteSequence;

  Spi_ChannelType EepAddrChannel;
  Spi_ChannelType EepCmdChannel;
  Spi_ChannelType EepDataChannel;
  Spi_ChannelType EepWrenChannel;

  // number of bytes read within one job processing cycle in normal mode.
  Eep_LengthType EepNormalReadBlockSize;

  // call cycle of the job processing function during write/erase operations. Unit: [s]
  float	 EepJobCallCycle;

  // This parameter is the used size of EEPROM device in bytes.
  Eep_LengthType	EepSize;

  // This parameter is a reference to a callback function for positive job result
  void (*Eep_JobEndNotification)();

  // This parameter is the default EEPROM device mode after initialization.
  MemIf_ModeType EepDefaultMode;

  // This parameter is the number of bytes read within one job processing cycle in fast mode
  Eep_LengthType EepFastReadBlockSize;

  // Number of bytes written within one job processing cycle in normal mode.
  Eep_LengthType EepNormalWriteBlockSize;

  // This parameter is a reference to a callback function for negative job result
  void (*Eep_JobErrorNotification)();

  // This parameter is the number of bytes written within one job processing cycle in fast mode
  Eep_LengthType EepFastWriteBlockSize;

  // This parameter is the EEPROM device base address.
  Eep_AddressType EepBaseAddress;
} Eep_ConfigType;

// This container is present for external EEPROM drivers only. Internal
// EEPROM drivers do not use the parameter listed in this container, hence
// its multiplicity is 0 for internal drivers.
typedef struct {
  // Reference to SPI sequence (required for external EEPROM drivers).
  // TODO: hmmm....
  uint32  SpiReference;
} Eep_ExternalDriverType;

extern const Eep_ConfigType EepConfigData[];

#define EEP_DEFAULT_CONFIG EepConfigData[0]

#endif /*EEP_CFG_H_*/
