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
#include "stm32f10x_gpio.h"


const uint32 remaps[] = {
		GPIO_Remap1_CAN1,
		GPIO_PartialRemap2_TIM2,
};

const GpioPinCnfMode_Type GPIOConf[] =
{
  /*GPIOA*/
  {
	.GpioPinCnfMode_15 = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
	.GpioPinCnfMode_14 = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
	.GpioPinCnfMode_13 = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
	.GpioPinCnfMode_12 = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
	.GpioPinCnfMode_11 = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
	.GpioPinCnfMode_10 = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
	.GpioPinCnfMode_9  = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
	.GpioPinCnfMode_8  = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
	.GpioPinCnfMode_7  = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
	.GpioPinCnfMode_6  = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
	.GpioPinCnfMode_5  = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
	.GpioPinCnfMode_4  = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
	.GpioPinCnfMode_3  = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
	.GpioPinCnfMode_2  = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
	.GpioPinCnfMode_1  = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
	.GpioPinCnfMode_0  = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
  },
  /*GPIOB*/
  {
  	.GpioPinCnfMode_15 = GPIO_OUTPUT_2MHz_MODE | GPIO_OUTPUT_PUSHPULL_CNF,
  	.GpioPinCnfMode_14 = GPIO_OUTPUT_2MHz_MODE | GPIO_OUTPUT_PUSHPULL_CNF,
  	.GpioPinCnfMode_13 = GPIO_OUTPUT_2MHz_MODE | GPIO_OUTPUT_PUSHPULL_CNF,
  	.GpioPinCnfMode_12 = GPIO_OUTPUT_2MHz_MODE | GPIO_OUTPUT_PUSHPULL_CNF,
  	.GpioPinCnfMode_11 = GPIO_OUTPUT_2MHz_MODE | GPIO_OUTPUT_PUSHPULL_CNF,
  	.GpioPinCnfMode_10 = GPIO_OUTPUT_2MHz_MODE | GPIO_OUTPUT_PUSHPULL_CNF,
  	.GpioPinCnfMode_9  = GPIO_OUTPUT_10MHz_MODE | GPIO_ALT_PUSHPULL_CNF,/* PB9 is CAN1_TX, remapped: */
  	.GpioPinCnfMode_8  = GPIO_INPUT_MODE | GPIO_INPUT_PULLUP_CNF,/* PB8 is CAN1_RX, remapped: */
  	.GpioPinCnfMode_7  = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
  	.GpioPinCnfMode_6  = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
  	.GpioPinCnfMode_5  = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
  	.GpioPinCnfMode_4  = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
  	.GpioPinCnfMode_3  = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
  	.GpioPinCnfMode_2  = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
  	.GpioPinCnfMode_1  = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
  	.GpioPinCnfMode_0  = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
  },
  /*GPIOC*/
  {
	.GpioPinCnfMode_15 = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
	.GpioPinCnfMode_14 = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
	.GpioPinCnfMode_13 = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
	.GpioPinCnfMode_12 = GPIO_OUTPUT_2MHz_MODE | GPIO_OUTPUT_PUSHPULL_CNF,
	.GpioPinCnfMode_11 = GPIO_OUTPUT_2MHz_MODE | GPIO_OUTPUT_PUSHPULL_CNF,
	.GpioPinCnfMode_10 = GPIO_OUTPUT_2MHz_MODE | GPIO_OUTPUT_PUSHPULL_CNF,
	.GpioPinCnfMode_9  = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
	.GpioPinCnfMode_8  = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
	.GpioPinCnfMode_7  = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
	.GpioPinCnfMode_6  = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
	.GpioPinCnfMode_5  = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
	.GpioPinCnfMode_4  = GPIO_OUTPUT_2MHz_MODE | GPIO_INPUT_PULLUP_CNF,
	.GpioPinCnfMode_3  = GPIO_OUTPUT_2MHz_MODE | GPIO_OUTPUT_PUSHPULL_CNF,
	.GpioPinCnfMode_2  = GPIO_OUTPUT_2MHz_MODE | GPIO_OUTPUT_PUSHPULL_CNF,
	.GpioPinCnfMode_1  = GPIO_OUTPUT_2MHz_MODE | GPIO_OUTPUT_PUSHPULL_CNF,
	.GpioPinCnfMode_0  = GPIO_OUTPUT_2MHz_MODE | GPIO_OUTPUT_PUSHPULL_CNF,
   },
   /*GPIOD*/
     {
   	.GpioPinCnfMode_15 = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
   	.GpioPinCnfMode_14 = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
   	.GpioPinCnfMode_13 = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
   	.GpioPinCnfMode_12 = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
   	.GpioPinCnfMode_11 = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
   	.GpioPinCnfMode_10 = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
   	.GpioPinCnfMode_9  = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
   	.GpioPinCnfMode_8  = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
   	.GpioPinCnfMode_7  = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
   	.GpioPinCnfMode_6  = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
   	.GpioPinCnfMode_5  = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
   	.GpioPinCnfMode_4  = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
   	.GpioPinCnfMode_3  = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
   	.GpioPinCnfMode_2  = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
   	.GpioPinCnfMode_1  = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
   	.GpioPinCnfMode_0  = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
   },
   /*GPIOE*/
   {
	.GpioPinCnfMode_15 = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
	.GpioPinCnfMode_14 = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
	.GpioPinCnfMode_13 = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
	.GpioPinCnfMode_12 = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
	.GpioPinCnfMode_11 = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
	.GpioPinCnfMode_10 = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
	.GpioPinCnfMode_9  = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
	.GpioPinCnfMode_8  = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
	.GpioPinCnfMode_7  = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
	.GpioPinCnfMode_6  = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
	.GpioPinCnfMode_5  = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
	.GpioPinCnfMode_4  = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
	.GpioPinCnfMode_3  = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
	.GpioPinCnfMode_2  = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
	.GpioPinCnfMode_1  = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
	.GpioPinCnfMode_0  = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
  },
  /*GPIOF*/
  {
	.GpioPinCnfMode_15 = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
	.GpioPinCnfMode_14 = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
	.GpioPinCnfMode_13 = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
	.GpioPinCnfMode_12 = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
	.GpioPinCnfMode_11 = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
	.GpioPinCnfMode_10 = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
	.GpioPinCnfMode_9  = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
	.GpioPinCnfMode_8  = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
	.GpioPinCnfMode_7  = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
	.GpioPinCnfMode_6  = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
	.GpioPinCnfMode_5  = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
	.GpioPinCnfMode_4  = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
	.GpioPinCnfMode_3  = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
	.GpioPinCnfMode_2  = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
	.GpioPinCnfMode_1  = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
	.GpioPinCnfMode_0  = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
  },
  /*GPIOG*/
  {
	.GpioPinCnfMode_15 = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
	.GpioPinCnfMode_14 = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
	.GpioPinCnfMode_13 = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
	.GpioPinCnfMode_12 = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
	.GpioPinCnfMode_11 = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
	.GpioPinCnfMode_10 = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
	.GpioPinCnfMode_9  = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
	.GpioPinCnfMode_8  = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
	.GpioPinCnfMode_7  = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
	.GpioPinCnfMode_6  = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
	.GpioPinCnfMode_5  = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
	.GpioPinCnfMode_4  = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
	.GpioPinCnfMode_3  = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
	.GpioPinCnfMode_2  = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
	.GpioPinCnfMode_1  = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
	.GpioPinCnfMode_0  = GPIO_INPUT_MODE | GPIO_FLOATING_INPUT_CNF,
  }
};

const GpioPinOutLevel_Type GPIOOutConf[] =
{
  /* GPIOA */
  {
	.GpioPinOutLevel_0 = GPIO_OUTPUT_LOW,
	.GpioPinOutLevel_1 = GPIO_OUTPUT_LOW,
	.GpioPinOutLevel_2 = GPIO_OUTPUT_LOW,
	.GpioPinOutLevel_3 = GPIO_OUTPUT_LOW,
	.GpioPinOutLevel_4 = GPIO_OUTPUT_LOW,
	.GpioPinOutLevel_5 = GPIO_OUTPUT_LOW,
	.GpioPinOutLevel_6 = GPIO_OUTPUT_LOW,
	.GpioPinOutLevel_7 = GPIO_OUTPUT_LOW,
	.GpioPinOutLevel_8 = GPIO_OUTPUT_LOW,
	.GpioPinOutLevel_9 = GPIO_OUTPUT_LOW,
    .GpioPinOutLevel_10 = GPIO_OUTPUT_LOW,
	.GpioPinOutLevel_11 = GPIO_OUTPUT_LOW,
	.GpioPinOutLevel_12 = GPIO_OUTPUT_LOW,
	.GpioPinOutLevel_13 = GPIO_OUTPUT_LOW,
	.GpioPinOutLevel_14 = GPIO_OUTPUT_LOW,
	.GpioPinOutLevel_15 = GPIO_OUTPUT_LOW,
  },
  /* GPIOB */
  {
	.GpioPinOutLevel_0 = GPIO_OUTPUT_LOW,
	.GpioPinOutLevel_1 = GPIO_OUTPUT_LOW,
	.GpioPinOutLevel_2 = GPIO_OUTPUT_LOW,
	.GpioPinOutLevel_3 = GPIO_OUTPUT_LOW,
	.GpioPinOutLevel_4 = GPIO_OUTPUT_LOW,
	.GpioPinOutLevel_5 = GPIO_OUTPUT_LOW,
	.GpioPinOutLevel_6 = GPIO_OUTPUT_LOW,
	.GpioPinOutLevel_7 = GPIO_OUTPUT_LOW,
	.GpioPinOutLevel_8 = GPIO_OUTPUT_HIGH,
	.GpioPinOutLevel_9 = GPIO_OUTPUT_LOW,
    .GpioPinOutLevel_10 = GPIO_OUTPUT_HIGH,
	.GpioPinOutLevel_11 = GPIO_OUTPUT_LOW,
	.GpioPinOutLevel_12 = GPIO_OUTPUT_HIGH,
	.GpioPinOutLevel_13 = GPIO_OUTPUT_LOW,
	.GpioPinOutLevel_14 = GPIO_OUTPUT_HIGH,
	.GpioPinOutLevel_15 = GPIO_OUTPUT_LOW,
  },
  /* GPIOC */
  {
	.GpioPinOutLevel_0 = GPIO_OUTPUT_LOW,
	.GpioPinOutLevel_1 = GPIO_OUTPUT_LOW,
	.GpioPinOutLevel_2 = GPIO_OUTPUT_LOW,
	.GpioPinOutLevel_3 = GPIO_OUTPUT_LOW,
	.GpioPinOutLevel_4 = GPIO_OUTPUT_LOW,
	.GpioPinOutLevel_5 = GPIO_OUTPUT_LOW,
	.GpioPinOutLevel_6 = GPIO_OUTPUT_LOW,
	.GpioPinOutLevel_7 = GPIO_OUTPUT_LOW,
	.GpioPinOutLevel_8 = GPIO_OUTPUT_LOW,
	.GpioPinOutLevel_9 = GPIO_OUTPUT_LOW,
    .GpioPinOutLevel_10 = GPIO_OUTPUT_LOW,
	.GpioPinOutLevel_11 = GPIO_OUTPUT_LOW,
	.GpioPinOutLevel_12 = GPIO_OUTPUT_LOW,
	.GpioPinOutLevel_13 = GPIO_OUTPUT_LOW,
	.GpioPinOutLevel_14 = GPIO_OUTPUT_LOW,
	.GpioPinOutLevel_15 = GPIO_OUTPUT_LOW,
  },
  /* GPIOD */
  {
	.GpioPinOutLevel_0 = GPIO_OUTPUT_LOW,
	.GpioPinOutLevel_1 = GPIO_OUTPUT_LOW,
	.GpioPinOutLevel_2 = GPIO_OUTPUT_LOW,
	.GpioPinOutLevel_3 = GPIO_OUTPUT_LOW,
	.GpioPinOutLevel_4 = GPIO_OUTPUT_LOW,
	.GpioPinOutLevel_5 = GPIO_OUTPUT_LOW,
	.GpioPinOutLevel_6 = GPIO_OUTPUT_LOW,
	.GpioPinOutLevel_7 = GPIO_OUTPUT_LOW,
	.GpioPinOutLevel_8 = GPIO_OUTPUT_LOW,
	.GpioPinOutLevel_9 = GPIO_OUTPUT_LOW,
    .GpioPinOutLevel_10 = GPIO_OUTPUT_LOW,
	.GpioPinOutLevel_11 = GPIO_OUTPUT_LOW,
	.GpioPinOutLevel_12 = GPIO_OUTPUT_LOW,
	.GpioPinOutLevel_13 = GPIO_OUTPUT_LOW,
	.GpioPinOutLevel_14 = GPIO_OUTPUT_LOW,
	.GpioPinOutLevel_15 = GPIO_OUTPUT_LOW,
  },
  /* GPIOE */
  {
	.GpioPinOutLevel_0 = GPIO_OUTPUT_LOW,
	.GpioPinOutLevel_1 = GPIO_OUTPUT_LOW,
	.GpioPinOutLevel_2 = GPIO_OUTPUT_LOW,
	.GpioPinOutLevel_3 = GPIO_OUTPUT_LOW,
	.GpioPinOutLevel_4 = GPIO_OUTPUT_LOW,
	.GpioPinOutLevel_5 = GPIO_OUTPUT_LOW,
	.GpioPinOutLevel_6 = GPIO_OUTPUT_LOW,
	.GpioPinOutLevel_7 = GPIO_OUTPUT_LOW,
	.GpioPinOutLevel_8 = GPIO_OUTPUT_LOW,
	.GpioPinOutLevel_9 = GPIO_OUTPUT_LOW,
    .GpioPinOutLevel_10 = GPIO_OUTPUT_LOW,
	.GpioPinOutLevel_11 = GPIO_OUTPUT_LOW,
	.GpioPinOutLevel_12 = GPIO_OUTPUT_LOW,
	.GpioPinOutLevel_13 = GPIO_OUTPUT_LOW,
	.GpioPinOutLevel_14 = GPIO_OUTPUT_LOW,
	.GpioPinOutLevel_15 = GPIO_OUTPUT_LOW,
  },
  /* GPIOF */
  {
	.GpioPinOutLevel_0 = GPIO_OUTPUT_LOW,
	.GpioPinOutLevel_1 = GPIO_OUTPUT_LOW,
	.GpioPinOutLevel_2 = GPIO_OUTPUT_LOW,
	.GpioPinOutLevel_3 = GPIO_OUTPUT_LOW,
	.GpioPinOutLevel_4 = GPIO_OUTPUT_LOW,
	.GpioPinOutLevel_5 = GPIO_OUTPUT_LOW,
	.GpioPinOutLevel_6 = GPIO_OUTPUT_LOW,
	.GpioPinOutLevel_7 = GPIO_OUTPUT_LOW,
	.GpioPinOutLevel_8 = GPIO_OUTPUT_LOW,
	.GpioPinOutLevel_9 = GPIO_OUTPUT_LOW,
    .GpioPinOutLevel_10 = GPIO_OUTPUT_LOW,
	.GpioPinOutLevel_11 = GPIO_OUTPUT_LOW,
	.GpioPinOutLevel_12 = GPIO_OUTPUT_LOW,
	.GpioPinOutLevel_13 = GPIO_OUTPUT_LOW,
	.GpioPinOutLevel_14 = GPIO_OUTPUT_LOW,
	.GpioPinOutLevel_15 = GPIO_OUTPUT_LOW,
  },
  /* GPIOG */
  {
	.GpioPinOutLevel_0 = GPIO_OUTPUT_LOW,
	.GpioPinOutLevel_1 = GPIO_OUTPUT_LOW,
	.GpioPinOutLevel_2 = GPIO_OUTPUT_LOW,
	.GpioPinOutLevel_3 = GPIO_OUTPUT_LOW,
	.GpioPinOutLevel_4 = GPIO_OUTPUT_LOW,
	.GpioPinOutLevel_5 = GPIO_OUTPUT_LOW,
	.GpioPinOutLevel_6 = GPIO_OUTPUT_LOW,
	.GpioPinOutLevel_7 = GPIO_OUTPUT_LOW,
	.GpioPinOutLevel_8 = GPIO_OUTPUT_LOW,
	.GpioPinOutLevel_9 = GPIO_OUTPUT_LOW,
    .GpioPinOutLevel_10 = GPIO_OUTPUT_LOW,
	.GpioPinOutLevel_11 = GPIO_OUTPUT_LOW,
	.GpioPinOutLevel_12 = GPIO_OUTPUT_LOW,
	.GpioPinOutLevel_13 = GPIO_OUTPUT_LOW,
	.GpioPinOutLevel_14 = GPIO_OUTPUT_LOW,
	.GpioPinOutLevel_15 = GPIO_OUTPUT_LOW,
  },
};

const Port_ConfigType PortConfigData =
{
	.padCnt = 7,  // TODO Replace with sizeof expression
	.padConfig = GPIOConf,
	.outConfig = GPIOOutConf,

    .remapCount = sizeof(remaps) / sizeof(uint32),
    .remaps = &remaps[0]
};

