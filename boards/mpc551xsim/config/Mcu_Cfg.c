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








#ifndef MCU_CFG_C_
#define MCU_CFG_C_

#include "Mcu.h"

Mcu_RamSectorSettingConfigType Mcu_RamSectorSettingConfigData[] = {
  {
  // This parameter shall represent the Data pre-setting to be initialized
  .McuRamDefaultValue = 0,

  // This parameter shall represent the MCU RAM section base address
  .McuRamSectionBaseAddress = 0,

  // This parameter shall represent the MCU RAM Section size
  .McuRamSectionSize = 0xFF,
  }
};

Mcu_ClockSettingConfigType Mcu_ClockSettingConfigData[] =
{
  {
    .McuClockReferencePointFrequency = 8000000UL,
    .Pll1 = 1,
    .Pll2    = 104,
    .Pll3    = 5,
  },
  {
    .McuClockReferencePointFrequency = 16000000UL,
    .Pll1 = 3,
    .Pll2   = 83,
    .Pll3    = 5,
  }
};


 const Mcu_ConfigType McuConfigData[] = {
  {
  //  Enables/Disables clock failure notification. In case this feature is not supported
  //  by HW the setting should be disabled.
  .McuClockSrcFailureNotification = 0,

  //  This parameter shall represent the number of RAM sectors available for
  //  the MCU. calculationFormula = Number of configured McuRamSectorSet-
  //  tingConf
  .McuRamSectors = 1,

  //  This parameter shall represent the number of clock setting available for
  //  the MCU.
  .McuClockSettings = MCU_NBR_OF_CLOCKS,

  //  This container contains the configuration (parameters) for the
  //  Clock settings of the MCU. Please see MCU031 for more in-
  //  formation on the MCU clock settings.
  .McuClockSettingConfig = &Mcu_ClockSettingConfigData[0],

  //  This container contains the configuration (parameters) for the
  //  RAM Sector setting. Please see MCU030 for more information
  //  on RAM sec-tor settings.
  .McuRamSectorSettingConfig = &Mcu_RamSectorSettingConfigData[0],
  },
};

#endif /*MCU_CFG_C_*/
