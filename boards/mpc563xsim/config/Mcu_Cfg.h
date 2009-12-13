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








#ifndef MCU_CFG_H_
#define MCU_CFG_H_

#include "mpc55xx.h"

#define MCU_DEV_ERROR_DETECT 	STD_ON
// Preprocessor switch to enable / disable the use of the function
// Mcu_PerformReset()
#define MCU_PERFORM_RESET_API 	STD_ON

#define MCU_VERSION_INFO_API 	STD_ON

#include "Std_Types.h"

/* FMPLL modes( atleast in 5553/5554 ) */

typedef enum {
	MCU_FMPLL_BYPASS = 0,
	MCU_FMPLL_EXTERNAL_REF,
	MCU_FMPLL_EXTERNAL_REF_NO_FM,
	MCU_FMPLL_DUAL_CONTROLLER_MODE,
} Mcu_FMPLLmode_t;

typedef enum {
  MCU_CLOCKTYPE_EXT_REF_80MHZ = 0,
  MCU_CLOCKTYPE_EXT_REF_66MHZ,
  MCU_NBR_OF_CLOCKS,
	//MCU_CLOCKTYPE_EXTERNAL_REF,
	//MCU_CLOCKTYPE_EXTERNAL_REF_NO_FM,
	//MCU_CLOCKTYPE_DUAL_CONTROLLER_MODE,
} Mcu_ClockType;

typedef struct {
	// This container defines a reference point in the Mcu Clock tree
	// It defines the frequency which then can be used by other modules
	// as an input value. Lower multiplicity is 1, as even in the
	// simplest case (only one frequency is used), there is one
	// frequency to be defined.
	uint32 McuClockReferencePoint;

  // Phase locked loop configuration parameters for MPC551x.
  uint8 PllEprediv;
  uint8 PllEmfd;
  uint8 PllErfd;
} Mcu_ClockSettingConfigType;

typedef struct {
	// The parameter represents the MCU Mode settings
	uint32 McuMode;
} Mcu_ModeSettingConfigType;

typedef struct {
	// This parameter shall represent the Data pre-setting to be initialized
	uint32 McuRamDefaultValue;

	// This parameter shall represent the MCU RAM section base address
	uint32 McuRamSectionBaseAddress;

	// This parameter shall represent the MCU RAM Section size
	uint32 McuRamSectionSize;

} Mcu_RamSectorSettingConfigType;


// This container defines a reference point in the Mcu Clock tree. It defines
// the frequency which then can be used by other modules as an input value.
// Lower multiplicity is 1, as even in the simplest case (only one frequency is
// used), there is one frequency to be defined.
typedef struct {

	//	This is the frequency for the specific instance of the McuClockReference-
	//	Point container. It shall be givn in Hz.
	uint32 McuClockReferencePointFrequency;

} Mcu_ClockReferencePointType;

typedef struct {
	//	Enables/Disables clock failure notification. In case this feature is not supported
	//	by HW the setting should be disabled.
	uint8	McuClockSrcFailureNotification;

	//	This parameter shall represent the number of Modes available for the
	//	MCU. calculationFormula = Number of configured McuModeSettingConf
	uint8 McuNumberOfMcuModes;

  //  This parameter shall represent the number of RAM sectors available for
  //  the MCU. calculationFormula = Number of configured McuRamSectorSet-
  //  tingConf
  uint8 McuRamSectors;

  //  This parameter shall represent the number of clock setting available for
  //  the MCU.
  uint8 McuClockSettings;

  // This parameter defines the default clock settings that should be used
  // It is an index into the McuClockSettingsConfig
  Mcu_ClockType McuDefaultClockSettings;

	//	This parameter relates to the MCU specific reset configuration. This ap-
	//	plies to the function Mcu_PerformReset, which performs a microcontroller
	//	reset using the hardware feature of the microcontroller.
	uint32 McuResetSetting;

	//	This container contains the configuration (parameters) for the
	//	Clock settings of the MCU. Please see MCU031 for more in-
	//	formation on the MCU clock settings.
	Mcu_ClockSettingConfigType * McuClockSettingConfig;

	//	This container contains the configuration (parameters) for the
	//	Mode setting of the MCU. Please see MCU035 for more information
	//  on the MCU mode settings.
	Mcu_ModeSettingConfigType  *McuModeSettingConfig;

	//	This container contains the configuration (parameters) for the
	//	RAM Sector setting. Please see MCU030 for more information
	//	on RAM sec-tor settings.
	Mcu_RamSectorSettingConfigType *McuRamSectorSettingConfig;

} Mcu_ConfigType;

extern const Mcu_ConfigType McuConfigData[];

#define MCU_DEFAULT_CONFIG McuConfigData[0]

#endif /*MCU_CFG_H_*/
