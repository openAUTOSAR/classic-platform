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

/** @addtogroup Port Port Driver
 *  @{ */

/** @file Port_Cfg.h
 *  Definitions of configuration parameters for Port Driver.
 */

#ifndef PORT_CFG_H_
#define PORT_CFG_H_

#include "Std_Types.h"

/** Build version info API */
#define PORT_VERSION_INFO_API				STD_ON
/** Enable Development Error Trace */
#define PORT_DEV_ERROR_DETECT				STD_ON
/** Allow Pin mode changes during runtime */
#define PORT_SET_PIN_MODE_API				STD_ON
/** Build change pin direction API */
#define PORT_SET_PIN_DIRECTION_API	STD_ON

/** HW specific symbolic names of pins */
typedef enum
{
  PORT_PA0,
  PORT_PA1,
  PORT_PA2,
  PORT_PA3,
  PORT_PA4,
  PORT_PA5,
  PORT_PA6,
  PORT_PA7,
  PORT_PA8,
  PORT_PA9,
  PORT_PA10,
  PORT_PA11,
  PORT_PA12,
  PORT_PA13,
  PORT_PA14,
  PORT_PA15,
  PORT_PB0,
  PORT_PB1,
  PORT_PB2,
  PORT_PB3,
  PORT_PB4,
  PORT_PB5,
  PORT_PB6,
  PORT_PB7,
  PORT_PB8,
  PORT_PB9,
  PORT_PB10,
  PORT_PB11,
  PORT_PB12,
  PORT_PB13,
  PORT_PB14,
  PORT_PB15,
  PORT_PC0,
  PORT_PC1,
  PORT_PC2,
  PORT_PC3,
  PORT_PC4,
  PORT_PC5,
  PORT_PC6,
  PORT_PC7,
  PORT_PC8,
  PORT_PC9,
  PORT_PC10,
  PORT_PC11,
  PORT_PC12,
  PORT_PC13,
  PORT_PC14,
  PORT_PC15,
  PORT_PD0,
  PORT_PD1,
  PORT_PD2,
  PORT_PD3,
  PORT_PD4,
  PORT_PD5,
  PORT_PD6,
  PORT_PD7,
  PORT_PD8,
  PORT_PD9,
  PORT_PD10,
  PORT_PD11,
  PORT_PD12,
  PORT_PD13,
  PORT_PD14,
  PORT_PD15,
  PORT_PE0,
  PORT_PE1,
  PORT_PE2,
  PORT_PE3,
  PORT_PE4,
  PORT_PE5,
  PORT_PE6,
  PORT_PE7,
  PORT_PE8,
  PORT_PE9,
  PORT_PE10,
  PORT_PE11,
  PORT_PE12,
  PORT_PE13,
  PORT_PE14,
  PORT_PE15,
  PORT_PF0,
  PORT_PF1,
  PORT_PF2,
  PORT_PF3,
  PORT_PF4,
  PORT_PF5,
  PORT_PF6,
  PORT_PF7,
  PORT_PF8,
  PORT_PF9,
  PORT_PF10,
  PORT_PF11,
  PORT_PF12,
  PORT_PF13,
  PORT_PF14,
  PORT_PF15,
  PORT_PG0,
  PORT_PG1,
  PORT_PG2,
  PORT_PG3,
  PORT_PG4,
  PORT_PG5,
  PORT_PG6,
  PORT_PG7,
  PORT_PG8,
  PORT_PG9,
  PORT_PG10,
  PORT_PG11,
  PORT_PG12,
  PORT_PG13,
  PORT_PG14,
  PORT_PG15,
  PORT_PH0,
  PORT_PH1,
  PORT_PH2,
  PORT_PH3,
  PORT_PH4,
  PORT_PH5,
  PORT_PH6,
  PORT_PH7,
  PORT_PH8,
  PORT_PH9,
  PORT_PH10,
  PORT_PH11,
  PORT_PH12,
  PORT_PH13,
  PORT_PH14,
  PORT_PH15,
  PORT_PJ0,
  PORT_PJ1,
  PORT_PJ2,
  PORT_PJ3,
  PORT_PJ4,
  PORT_PJ5,
  PORT_PJ6,
  PORT_PJ7,
  PORT_PJ8,
  PORT_PJ9,
  PORT_PJ10,
  PORT_PJ11,
  PORT_PJ12,
  PORT_PJ13,
  PORT_PJ14,
  PORT_PJ15,
  PORT_PK0,
  PORT_PK1
} Port_PinType;

/** @name HW specific register bits. */
//@{
#define   PORT_BIT0    (1<<15)
#define   PORT_BIT1    (1<<14)
#define   PORT_BIT2    (1<<13)
#define   PORT_BIT3    (1<<12)
#define   PORT_BIT4    (1<<11)
#define   PORT_BIT5    (1<<10)
#define   PORT_BIT6    (1<<9)
#define   PORT_BIT7    (1<<8)
#define   PORT_BIT8    (1<<7)
#define   PORT_BIT9    (1<<6)
#define   PORT_BIT10   (1<<5)
#define   PORT_BIT11   (1<<4)
#define   PORT_BIT12   (1<<3)
#define   PORT_BIT13   (1<<2)
#define   PORT_BIT14   (1<<1)
#define   PORT_BIT15   (1<<0)

#define   PORT_WPE_BIT		PORT_BIT14
#define   PORT_WPS_BIT		PORT_BIT15
#define   PORT_SRC0			PORT_BIT12
#define   PORT_SRC1			PORT_BIT13

#define   PORT_PULL_UP			(PORT_WPE_BIT|PORT_WPS_BIT)
#define   PORT_PULL_DOWN		(PORT_WPE_BIT)
#define   PORT_PULL_NONE		0
#define   PORT_SLEW_RATE_MIN	0
#define   PORT_SLEW_RATE_MED	PORT_BIT13
#define   PORT_SLEW_RATE_MAX	(PORT_BIT12|PORT_BIT13)
#define   PORT_HYS_ENABLE		PORT_BIT11
#define   PORT_ODE_ENABLE		PORT_BIT10
#define   PORT_IBE_ENABLE		PORT_BIT7
#define   PORT_OBE_ENABLE		PORT_BIT6
#define   PORT_PA_IO     		0
#define   PORT_PA_FUNC1			(PORT_BIT5)
#define   PORT_PA_FUNC2			(PORT_BIT4)
#define   PORT_PA_FUNC3			(PORT_BIT4|PORT_BIT5)

#define   PORT_NORMAL_INPUT	(PORT_BIT15)

// Should be this out of reset
#define   PORT_PCR_RESET    (0)
#define   PORT_PCR_BOOTCFG   (PORT_IBE_ENABLE|PORT_PULL_DOWN)
//@}

#define PORT_EVB_TEST_CONFIG  (&PortConfigData)

/** Top level configuration container */
typedef struct
{
  /** Total number of pins */
  uint16_t padCnt;
  /** List of pin configurations */
  const uint16_t *padConfig;
  /** Total number of pin default levels */
  uint16_t outCnt;
  /** List of pin default levels */
  const uint8_t *outConfig;
//  uint16_t inCnt;
//  const uint8_t *inConfig;
} Port_ConfigType;

/** Instance of the top level configuration container */
extern const Port_ConfigType PortConfigData;

#endif /*PORT_CFG_H_*/
/** @} */
