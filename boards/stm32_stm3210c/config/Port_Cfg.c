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
#include "Port_Cfg.h"


typedef enum {
  PORTA,
  PORTB,
  PORTC,
  PORTD,
  PORTE,
  NUMBER_OF_PORTS
} Port_PortType;

const u32 remaps[] = {
		GPIO_Remap_ETH,
};

const Port_PortConfigType porta = {
  .port = GPIOA,
  .pinCount = 3,
  .pins = {
		    {
		      .GPIO_Pin = GPIO_Pin_2,
		      .GPIO_Mode = GPIO_Mode_AF_PP,
		      .GPIO_Speed = GPIO_Speed_50MHz
		    },
		    {
		      .GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_3,
		      .GPIO_Mode = GPIO_Mode_IN_FLOATING,
		      .GPIO_Speed = GPIO_Speed_50MHz
		    },
		    {
		      .GPIO_Pin = GPIO_Pin_8,
		      .GPIO_Mode = GPIO_Mode_AF_PP,
		      .GPIO_Speed = GPIO_Speed_50MHz
		    },
  }
};

const Port_PortConfigType portb = {
  .port = GPIOB,
  .pinCount = 2,
  .pins = {
	/* PB8 is CAN1_RX, remapped: */
/*	{
	  .GPIO_Pin = GPIO_Pin_8,
	  .GPIO_Mode = GPIO_Mode_IPU,
	  .GPIO_Speed = GPIO_Speed_10MHz
	},*/
	/* PB9 is CAN1_TX, remapped: */
/*	{
	  .GPIO_Pin = GPIO_Pin_9,
	  .GPIO_Mode = GPIO_Mode_AF_PP,
	  .GPIO_Speed = GPIO_Speed_10MHz
	},*/
	{
	  .GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_8 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13,
	  .GPIO_Mode = GPIO_Mode_AF_PP,
	  .GPIO_Speed = GPIO_Speed_50MHz
	},
	{
	  .GPIO_Pin = GPIO_Pin_10,
	  .GPIO_Mode = GPIO_Mode_IN_FLOATING,
	  .GPIO_Speed = GPIO_Speed_50MHz
	},

  }
};

const Port_PortConfigType portc = {
  .port = GPIOC,
  .pinCount = 3,
  .pins = {
		    {
		      .GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2,
		      .GPIO_Mode = GPIO_Mode_AF_PP,
		      .GPIO_Speed = GPIO_Speed_50MHz,
		    },
		    {
		      .GPIO_Pin = GPIO_Pin_3,
		      .GPIO_Mode = GPIO_Mode_IN_FLOATING,
		      .GPIO_Speed = GPIO_Speed_50MHz,
		    },
		    {
		      .GPIO_Pin = GPIO_Pin_4,
		      .GPIO_Mode = GPIO_Mode_AIN,
		      .GPIO_Speed = GPIO_Speed_10MHz,
		    },

  }
};

const Port_PortConfigType portd = {
  .port = GPIOD,
  .pinCount = 1,
  .pins = {
	{
	  .GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12,
	  .GPIO_Mode = GPIO_Mode_IN_FLOATING,
	  .GPIO_Speed = GPIO_Speed_50MHz,
	},
  }
};

const Port_PortConfigType porte = {
  .port = GPIOE,
  .pinCount = 1,
  .pins = {
	{
	  .GPIO_Pin = 0xffff,
	  .GPIO_Mode = GPIO_Mode_IN_FLOATING,
	  .GPIO_Speed = GPIO_Speed_2MHz,
	},
  }
};

const Port_ConfigType PortConfigData = {
    .portCount = NUMBER_OF_PORTS,
    .ports = {
      &porta,
      &portb,
      &portc,
      &portd,
      &porte
    },

    .remapCount = sizeof(remaps) / sizeof(u32),
    .remaps = &remaps[0]
};

