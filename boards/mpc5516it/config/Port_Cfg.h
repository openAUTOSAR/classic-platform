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
/** Build change pin direction API */
#define PORT_PIN_DIRECTION_CHANGES_ALLOWED	STD_ON

/** HW specific symbolic names of pins */
typedef enum
{
  PA0,
  PA1,
  PA2,
  PA3,
  PA4,
  PA5,
  PA6,
  PA7,
  PA8,
  PA9,
  PA10,
  PA11,
  PA12,
  PA13,
  PA14,
  PA15,
  PB0,
  PB1,
  PB2,
  PB3,
  PB4,
  PB5,
  PB6,
  PB7,
  PB8,
  PB9,
  PB10,
  PB11,
  PB12,
  PB13,
  PB14,
  PB15,
  PC0,
  PC1,
  PC2,
  PC3,
  PC4,
  PC5,
  PC6,
  PC7,
  PC8,
  PC9,
  PC10,
  PC11,
  PC12,
  PC13,
  PC14,
  PC15,
  PD0,
  PD1,
  PD2,
  PD3,
  PD4,
  PD5,
  PD6,
  PD7,
  PD8,
  PD9,
  PD10,
  PD11,
  PD12,
  PD13,
  PD14,
  PD15,
  PE0,
  PE1,
  PE2,
  PE3,
  PE4,
  PE5,
  PE6,
  PE7,
  PE8,
  PE9,
  PE10,
  PE11,
  PE12,
  PE13,
  PE14,
  PE15,
  PF0,
  PF1,
  PF2,
  PF3,
  PF4,
  PF5,
  PF6,
  PF7,
  PF8,
  PF9,
  PF10,
  PF11,
  PF12,
  PF13,
  PF14,
  PF15,
  PG0,
  PG1,
  PG2,
  PG3,
  PG4,
  PG5,
  PG6,
  PG7,
  PG8,
  PG9,
  PG10,
  PG11,
  PG12,
  PG13,
  PG14,
  PG15,
  PH0,
  PH1,
  PH2,
  PH3,
  PH4,
  PH5,
  PH6,
  PH7,
  PH8,
  PH9,
  PH10,
  PH11,
  PH12,
  PH13,
  PH14,
  PH15,
  PJ0,
  PJ1,
  PJ2,
  PJ3,
  PJ4,
  PJ5,
  PJ6,
  PJ7,
  PJ8,
  PJ9,
  PJ10,
  PJ11,
  PJ12,
  PJ13,
  PJ14,
  PJ15,
  PK0,
  PK1
} Port_PinType;

/** @name HW specific register bits. */
//@{
#define   BIT0    (1<<15)
#define   BIT1    (1<<14)
#define   BIT2    (1<<13)
#define   BIT3    (1<<12)
#define   BIT4    (1<<11)
#define   BIT5    (1<<10)
#define   BIT6    (1<<9)
#define   BIT7    (1<<8)
#define   BIT8    (1<<7)
#define   BIT9    (1<<6)
#define   BIT10   (1<<5)
#define   BIT11   (1<<4)
#define   BIT12   (1<<3)
#define   BIT13   (1<<2)
#define   BIT14   (1<<1)
#define   BIT15   (1<<0)

#define   WPE_BIT     BIT14
#define   WPS_BIT     BIT15
#define   SRC0    BIT12
#define   SRC1    BIT13

#define   PULL_UP     (WPE_BIT|WPS_BIT)
#define   PULL_DOWN   (WPE_BIT)
#define   PULL_NONE   0
#define   SLEW_RATE_MIN 0
#define   SLEW_RATE_MED BIT13
#define   SLEW_RATE_MAX (BIT12|BIT13)
#define   HYS_ENABLE    BIT11
#define   ODE_ENABLE    BIT10
#define   IBE_ENABLE    BIT7
#define   OBE_ENABLE    BIT6
#define   PA_IO     0
#define   PA_FUNC1    (BIT5)
#define   PA_FUNC2    (BIT4)
#define   PA_FUNC3    (BIT4|BIT5)

#define   NORMAL_INPUT  (BIT15)

// Should be this out of reset
#define    PCR_RESET    (0)
#define PCR_BOOTCFG   (IBE_ENABLE|PULL_DOWN)
//@}

#define EVB_TEST_CONFIG  (&PortConfigData)

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
