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


const Mcu_PerClockConfigType McuPerClockConfigData =
{
	.AHBClocksEnable = RCC_AHBPeriph_DMA1 | RCC_AHBPeriph_ETH_MAC |
					   RCC_AHBPeriph_ETH_MAC_Tx | RCC_AHBPeriph_ETH_MAC_Rx,
	.APB1ClocksEnable = RCC_APB1Periph_TIM2 | RCC_APB1Periph_CAN1,
	.APB2ClocksEnable = (RCC_APB2Periph_ADC1 | RCC_APB2Periph_GPIOA |
						 RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC |
						 RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE | RCC_APB2Periph_AFIO),
};

Mcu_ClockSettingConfigType Mcu_ClockSettingConfigData[] =
{
  {
    .McuClockReferencePointFrequency = 25000000UL,
    .Pll1 = 9,
    .Pll2 = 8,
    .Pll3 = 0,
  },
};


 const Mcu_ConfigType McuConfigData[] = {
  {
  //  Enables/Disables clock failure notification. In case this feature is not supported
  //  by HW the setting should be disabled.
  .McuClockSrcFailureNotification = 0,

  //  This parameter shall represent the number of Modes available for the
  //  MCU. calculationFormula = Number of configured McuModeSettingConf
//  .McuNumberOfMcuModes = 1, /* NOT USED */

  //  This parameter shall represent the number of RAM sectors available for
  //  the MCU. calculationFormula = Number of configured McuRamSectorSet-
  //  tingConf
  .McuRamSectors = 1,

  //  This parameter shall represent the number of clock setting available for
  //  the MCU.
  .McuClockSettings = MCU_NBR_OF_CLOCKS,

  // Default clock frequency used
  .McuDefaultClockSettings = MCU_CLOCKTYPE_EXT_REF_25MHZ,

  //  This parameter relates to the MCU specific reset configuration. This ap-
  //  plies to the function Mcu_PerformReset, which performs a microcontroller
  //  reset using the hardware feature of the microcontroller.
//  .McuResetSetting = 0, /* NOT USED */

  //  This container contains the configuration (parameters) for the
  //  Clock settings of the MCU. Please see MCU031 for more in-
  //  formation on the MCU clock settings.
  .McuClockSettingConfig = &Mcu_ClockSettingConfigData[0],

  //  This container contains the configuration (parameters) for the
  //  Mode setting of the MCU. Please see MCU035 for more infor-
  //  mation on the MCU mode settings.
//  .McuModeSettingConfig = 0,

  //  This container contains the configuration (parameters) for the
  //  RAM Sector setting. Please see MCU030 for more information
  //  on RAM sec-tor settings.
  .McuRamSectorSettingConfig = &Mcu_RamSectorSettingConfigData[0],
  },
};

#endif /*MCU_CFG_C_*/
