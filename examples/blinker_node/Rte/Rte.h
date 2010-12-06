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


#ifndef RTE_H_
#define RTE_H_
#include "RTE_blinker.h"

// Select DIO channel
#if defined(CFG_BRD_MPC5516IT)
#define LED_CHANNEL		DIO_CHANNEL_NAME_LEDS_LED5

#elif defined(CFG_BRD_MPC5567QRTECH)
#define LED_CHANNEL		DIO_CHANNEL_NAME_LED_K2

#elif defined(CFG_BRD_HCS12X_ELMICRO_TBOARD)
#define LED_CHANNEL             DIO_CHANNEL_NAME_DO_PB0

#elif defined(CFG_BRD_HCS12_ELMICRO_CARD12)
#define LED_CHANNEL             DIO_CHANNEL_NAME_LED_CHANNEL

#elif defined(CFG_BRD_STM32_STM3210C)
#define LED_CHANNEL             LED_CHANNEL1

#elif defined(CFG_BRD_STM32_MCBSTM32)
#define LED_CHANNEL             LED_CHANNEL1

#else
#warning "Unknown board or CFG_BRD_* undefined"

#endif

#endif /* RTE_H_ */
