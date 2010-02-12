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

/** @addtogroup Fls Flash Driver
 *  @{ */

/** @file Fls_Cfg.h
 *  Definitions of configuration parameters for Flash Driver.
 */

#ifndef FLS_CFG_H_
#define FLS_CFG_H_

#include "MemIf_Types.h"


/** Indicate that we are building Post Build, NOT Pre-Compile */
#define FLS_VARIANT_PB	STD_ON

/*
 * Fls General container
 */
/** The flash driver shall load the flash access code to RAM whenever an
 *  erase or write job is started and unload (overwrite) it after that job has
 *  been finished or canceled. true: Flash access code loaded on job start /
 *  unloaded on job end or error. false: Flash access code not loaded to /
 *  unloaded from RAM at all. */
#define FLS_AC_LOAD_ON_JOB_START STD_OFF

/** The flash memory start address (see also FLS118).
 *  FLS169: This parameter defines the lower boundary for read / write /
 *  erase and compare jobs. */
#define FLS_BASE_ADDRESS 			0x00000000

/** Compile switch to enable and disable the Fls_Cancel function. true: API
 *  supported / function provided. false: API not supported / function not pro-
 *  vided */
#define FLS_CANCEL_API				STD_OFF

/** Compile switch to enable and disable the Fls_Compare function. true: API
 *  supported / function provided. false: API not supported / function not pro-
 *  vided */
#define FLS_COMPARE_API			STD_ON

/** Pre-processor switch for enabling the development error detection and
 *  reporting (see FLS077). */
#define FLS_DEV_ERROR_DETECT		STD_ON

/** Index of the driver, used by FEE. */
#define FLS_DRIVER_INDEX			100

/** Compile switch to enable and disable the Fls_GetJobResult function. true:
 *  API supported / function provided. false: API not supported / function not
 * provided */
#define FLS_GET_JOB_RESULT_API		STD_ON

/** Compile switch to enable and disable the Fls_GetStatus function. true: API
 *  supported / function provided. false: API not supported / function not pro-
 *  vided */
#define FLS_GET_STATUS_API			STD_ON

/** Compile switch to enable and disable the Fls_SetMode function. true: API
 *  supported / function provided. false: API not supported / function not pro-
 *  vided */
#define FLS_SET_MODE_API			STD_OFF

/** The total amount of flash memory in bytes (see also FLS118).
 *  FLS170: This parameter in conjunction with FLS_BASE_ADDRESS
 *  defines the upper boundary for read / write / erase and compare jobs */
#define FLS_TOTAL_SIZE				0x180000 // from addr 0x0000_0000 to 0x0018_0000
/** Read page size of 128 bits (4 words) (8 bytes) */
#define FLS_READ_PAGE_SIZE 0x8


// Job processing triggered by hardware interrupt. true: Job processing trig-
// gered by interrupt (hardware controlled). false: Job processing not trig-
// gered by interrupt (software controlled)

/** NOT supported by Freescale hardware */
#define FLS_USE_INTERRUPTS			STD_OFF

#define FLS_VERSION_INFO_API		STD_ON


#if ( FLS_AC_LOAD_ON_JOB_START == STD_ON )
/* Sections that are provided by linker */
extern void __FLS_ERASE_RAM__(void);
extern void __FLS_WRITE_RAM__(void);
extern void __FLS_ERASE_ROM__(void);
extern void __FLS_WRITE_ROM__(void);
extern char __FLS_SIZE__;
#endif

/** Configuration description of a flashable sector */
typedef struct {
  /** Number of continuous sectors with the above characteristics. */
  Fls_LengthType FlsNumberOfSectors;

  /** Size of one page of this sector. Implementation Type: Fls_LengthType. */
  Fls_LengthType FlsPageSize;

  /** Size of this sector. Implementation Type: Fls_LengthTyp */
  Fls_LengthType FlsSectorSize;

  /** Start address of this sector */
  Fls_AddressType FlsSectorStartaddress;

} Fls_SectorType;


/** Container for runtime configuration parameters of the flash driver.
 *  Implementation Type: Fls_ConfigType. */
typedef struct {
  /** Address offset in RAM to which the erase flash access code shall be
   *  loaded. Used as function pointer to access the erase flash access code. */
  void (*FlsAcErase)();

  /** Address offset in RAM to which the write flash access code shall be
   *  loaded. Used as function pointer to access the write flash access code. */
  void (*FlsAcWrite)();
//#if 0
//	// Cycle time of calls of the flash driver's main function.
//	float FlsCallCycle;
//#endif
	/** Mapped to the job end notification routine provided by some upper layer
	 *  module, typically the Fee module. */
	void (*FlsJobEndNotification)();

	/** Mapped to the job error notification routine provided by some upper layer
	 *  module, typically the Fee module. */
	void (*FlsJobErrorNotification)();

	/** The maximum number of bytes to read or compare in one cycle of the
     *  flash driver's job processing function in fast mode. */
	uint32 FlsMaxReadFastMode;

    /** The maximum number of bytes to read or compare in one cycle of the
     *  flash driver's job processing function in normal mode. */
	uint32 FlsMaxReadNormalMode;

	/** The maximum number of bytes to write in one cycle of the flash driver's job
	 *  processing function in fast mode. */
	uint32 FlsMaxWriteFastMode;

	/** The maximum number of bytes to write in one cycle of the flash driver's job
	 *  processing function in normal mode. */
	uint32 FlsMaxWriteNormalMode;

	/** Erase/write protection settings. Only relevant if supported by hardware. */
	uint32 FlsProtection;

  /** List of flash:able sectors and pages */
  const Fls_SectorType *FlsSectorList;

  /** Size of List of the FlsSectorList */
  const uint32 FlsSectorListSize;

  uint8 *FlsBlockToPartitionMap;


} Fls_ConfigType;


extern const Fls_ConfigType FlsConfigSet[];

#if 0
/** N/A since PPC have PIC */
#define FLS_AC_LOCATION_ERASE
#define FLS_AC_LOCATION_WRITE
/** N/A since we have internal flash */
#define FLS_EXPECTED_HW_ID
#endif

#define FLS_AC_SIZE_ERASE							(__FLS_WRITE_ROM__ - __FLS_ERASE_ROM__)
#define FLS_AC_SIZE_WRITE							(__FLS_END_ROM__ - __FLS_ERASE_ROM__)
#define FLS_ERASED_VALUE								0xff

#define FLS_SPECIFIED_ERASE_CYCLES			0 /* TODO */
#define FLS_WRITE_TIME	0 							/* TODO */


#endif /*FLS_CFG_H_*/
/** @} */
