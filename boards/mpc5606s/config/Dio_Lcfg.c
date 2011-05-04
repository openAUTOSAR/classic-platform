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








#warning "This default file may only be used as an example!"

#include "Dio.h"
#include "Dio_Cfg.h"

const Dio_ChannelType DioChannelConfigData[] =
{
	//DIO_CHANNEL_NAME_LEDS_LED4,
	//DIO_CHANNEL_NAME_LEDS_LED5,
	//DIO_CHANNEL_NAME_LEDS_LED6,
	//DIO_CHANNEL_NAME_LEDS_LED7,
	#if 0
	DIO_CHANNEL_A0,
	DIO_CHANNEL_A1,
	DIO_CHANNEL_A2,
	DIO_CHANNEL_A3,
	#endif
	DIO_CHANNEL_A4,
	DIO_CHANNEL_A5,
	DIO_CHANNEL_A6,
	DIO_CHANNEL_A7,
	#if 0
	DIO_CHANNEL_A8,
	DIO_CHANNEL_A9,
	DIO_CHANNEL_A10,
	DIO_CHANNEL_A11,
	DIO_CHANNEL_A12,
	DIO_CHANNEL_A13,
	DIO_CHANNEL_A14,
	DIO_CHANNEL_A15,
	#endif
	DIO_END_OF_LIST
};

const Dio_PortType DioPortConfigData[] =
{
	DIO_PORT_A,
	/*DIO_PORT_NAME_LED_PORT, */
	DIO_END_OF_LIST
};

const Dio_ChannelGroupType DioConfigData[] =
{
	{ .port = DIO_PORT_A, .offset = 8, .mask = 0x0f00, },
	{ .port = DIO_END_OF_LIST, .offset = DIO_END_OF_LIST, .mask = DIO_END_OF_LIST, },
};



uint32 Dio_GetPortConfigSize(void)
{
	return sizeof(DioConfigData);
}

