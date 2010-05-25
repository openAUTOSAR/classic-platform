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
#define PORT_PIN_DIRECTION_CHANGES_ALLOWED	STD_ON

typedef uint16 Port_PinType;

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

#define   WPS_BIT     BIT15
#define   WPE_BIT     BIT14
#define   SRC1_BIT    BIT13
#define   SRC0_BIT    BIT12
#define   HYS_BIT     BIT11
#define   ODE_BIT     BIT10
#define   DSC1_BIT    BIT9
#define   DSC0_BIT    BIT8
#define   IBE_BIT     BIT7
#define   OBE_BIT     BIT6
#define   PA2_BIT     BIT5
#define   PA1_BIT     BIT4
#define   PA0_BIT     BIT3

#define   PULL_UP             (WPE_BIT|WPS_BIT)
#define   PULL_DOWN           (WPE_BIT)
#define   PULL_NONE           (0)
#define   SLEW_RATE_MIN       (0)
#define   SLEW_RATE_MED       (SRC1_BIT)
#define   SLEW_RATE_MAX       (SRC1_BIT|SRC0_BIT)
#define   HYS_ENABLE          (HYS_BIT)
#define   ODE_ENABLE          (ODE_BIT)
#define   IBE_ENABLE          (IBE_BIT)
#define   OBE_ENABLE          (OBE_BIT)

#define   PA_IO               (0)
#define   PA_PRIMARY_FUNC     (PA2_BIT)
#define   PA_ALTERNATE_FUNC1  (PA1_BIT)
#define   PA_PRIMARY_FUNC1    (PA1_BIT|PA2_BIT)
#define   PA_ALTERNATE_FUNC2  (PA0_BIT)

#define   PCR_IO_INPUT        (IBE_ENABLE|PULL_DOWN)
#define   PCR_IO_OUTPUT       (OBE_ENABLE)

// Should be this out of reset
#define   PCR_RESET           (0)
#define   PCR_BOOTCFG         (PCR_IO_INPUT)
#define   GPDO_RESET          (0)

typedef struct
{
  uint16_t padCnt;
  const uint16_t *padConfig;
  uint16_t outCnt;
  const uint8_t *outConfig;
} Port_ConfigType;

extern const Port_ConfigType PortConfigData;

#endif /*PORT_CFG_H_*/
