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








#ifndef PORT_CFG_H_
#define PORT_CFG_H_

#include "Std_Types.h"

#define PORT_VERSION_INFO_API				STD_ON
#define PORT_DEV_ERROR_DETECT				STD_ON
#define PORT_SET_PIN_MODE_API				STD_ON
#define PORT_SET_PIN_DIRECTION_API			STD_ON

typedef uint16 Port_PinType;

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

#define   PORT_WPS_BIT     PORT_BIT15
#define   PORT_WPE_BIT     PORT_BIT14
#define   PORT_SRC1_BIT    PORT_BIT13
#define   PORT_SRC0_BIT    PORT_BIT12
#define   PORT_HYS_BIT     PORT_BIT11
#define   PORT_ODE_BIT     PORT_BIT10
#define   PORT_DSC1_BIT    PORT_BIT9
#define   PORT_DSC0_BIT    PORT_BIT8
#define   PORT_IBE_BIT     PORT_BIT7
#define   PORT_OBE_BIT     PORT_BIT6
#define   PORT_PA2_BIT     PORT_BIT5
#define   PORT_PA1_BIT     PORT_BIT4
#define   PORT_PA0_BIT     PORT_BIT3

#define   PORT_PULL_UP             (PORT_WPE_BIT|PORT_WPS_BIT)
#define   PORT_PULL_DOWN           (PORT_WPE_BIT)
#define   PORT_PULL_NONE           (0)
#define   PORT_SLEW_RATE_MIN       (0)
#define   PORT_SLEW_RATE_MED       (PORT_SRC1_BIT)
#define   PORT_SLEW_RATE_MAX       (PORT_SRC1_BIT|PORT_SRC0_BIT)
#define   PORT_HYS_ENABLE          (PORT_HYS_BIT)
#define   PORT_ODE_ENABLE          (PORT_ODE_BIT)
#define   PORT_IBE_ENABLE          (PORT_IBE_BIT)
#define   PORT_OBE_ENABLE          (PORT_OBE_BIT)

#define   PORT_PA_IO               (0)
#define   PORT_PA_PRIMARY_FUNC     (PORT_PA2_BIT)
#define   PORT_PA_ALTERNATE_FUNC1  (PORT_PA1_BIT)
#define   PORT_PA_PRIMARY_FUNC1    (PORT_PA1_BIT|PORT_PA2_BIT)
#define   PORT_PA_ALTERNATE_FUNC2  (PORT_PA0_BIT)

#define   PORT_PCR_IO_INPUT        (PORT_IBE_ENABLE|PORT_PULL_DOWN)
#define   PORT_PCR_IO_OUTPUT       (PORT_OBE_ENABLE)

// Should be this out of reset
#define   PORT_PCR_RESET           (0)
#define   PORT_PCR_BOOTCFG         (PORT_PCR_IO_INPUT)
#define   PORT_GPDO_RESET          (0)

typedef struct
{
  uint16_t padCnt;
  const uint16_t *padConfig;
  uint16_t outCnt;
  const uint8_t *outConfig;
} Port_ConfigType;

extern const Port_ConfigType PortConfigData;

#endif /*PORT_CFG_H_*/
