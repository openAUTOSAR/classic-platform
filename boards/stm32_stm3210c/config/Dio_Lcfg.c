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
#include "Dio.h"
#include "Dio_Cfg.h"

const Dio_ChannelType DioChannelConfigData[] =
{
	DIO_CHANNEL_NAME_LED_CHANNEL1,
	DIO_CHANNEL_NAME_LED_CHANNEL2,
	DIO_CHANNEL_NAME_LED_CHANNEL3,
	DIO_CHANNEL_NAME_LED_CHANNEL4,
	DIO_END_OF_LIST,
};

const Dio_PortType DioPortConfigData[] =
{
		DIO_PORT_NAME_LED_PORT,
	DIO_END_OF_LIST
};

const Dio_ChannelGroupType DioConfigData[] =
{
  { .port = DIO_PORT_D, .offset = 0, .mask = 0x1094, },
  { .port = DIO_END_OF_LIST, .offset = DIO_END_OF_LIST, .mask = DIO_END_OF_LIST, },
};

uint32 Dio_GetPortConfigSize(void)
{
  return sizeof(DioConfigData);
}
