/*-------------------------------- Arctic Core ------------------------------
 * Copyright (C) 2013, ArcCore AB, Sweden, www.arccore.com.
 * Contact: <contact@arccore.com>
 * 
 * You may ONLY use this file:
 * 1)if you have a valid commercial ArcCore license and then in accordance with  
 * the terms contained in the written license agreement between you and ArcCore, 
 * or alternatively
 * 2)if you follow the terms found in GNU General Public License version 2 as 
 * published by the Free Software Foundation and appearing in the file 
 * LICENSE.GPL included in the packaging of this file or here 
 * <http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt>
 *-------------------------------- Arctic Core -----------------------------*/

#ifndef MCU_CFG_H_
#define MCU_CFG_H_


#define MCU_DEV_ERROR_DETECT               STD_OFF
#define MCU_PERFORM_RESET_API              STD_OFF
#define MCU_VERSION_INFO_API               STD_OFF
#define MCU_NO_PLL                         STD_OFF
#define MCU_INIT_CLOCK                     STD_OFF
#define MCU_GET_RAM_STATE                  STD_OFF
#define MCU_CLOCK_SRC_FAILURE_NOTIFICATION STD_OFF

/* Mcu Modes (Symbolic name) */
#define McuConf_McuModeSettingConf_RUN (Mcu_ModeType)0u
#define McuConf_McuModeSettingConf_SLEEP (Mcu_ModeType)1u
#define McuConf_McuModeSettingConf_NORMAL (Mcu_ModeType)2u

/* Mcu Clock Types (Symbolic name) */
#define McuConf_McuClockSettingConfig_EXT_REF_66MHZ (Mcu_ClockType)0u
#define McuConf_McuClockSettingConfig_EXT_REF_80MHZ (Mcu_ClockType)1u
#define McuConf_McuClockSettingConfig_XOSC_CAN (Mcu_ClockType)2u

#endif /* MCU_CFG_H_ */
