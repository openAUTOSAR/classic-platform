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








#ifndef DIO_CFG_H_
#define DIO_CFG_H_

#define DIO_VERSION_INFO_API    STD_ON
#define DIO_DEV_ERROR_DETECT 	STD_ON
#define DIO_END_OF_LIST  -1

// Channels
extern const Dio_ChannelType DioChannelConfigData[];
// Port
extern const Dio_PortType DioPortConfigData[];
// Channel group
extern const Dio_ChannelGroupType DioConfigData[];

// Mapping of logical ports to physical

// Channels
#define DIO_CHANNEL_NAME_LED_K2		(125)

// Port
// No ports available for MPC5567

// Channel group
#define DIO_GROUP_NAME_LED_GRP_PTR		(&DioConfigData[0])

//
// Channels
//
#define DIO_CONTROL_BUS_EN
#define DIO_CONTROL_BUS_STB
#define DIO_CONTROL_BUS_ERR
#define DIO_CONTROL_BUS_S_PLUS_ST
#define DIO_CONTROL_BUS_S_MINUS_ST
#define DIO_CONTROL_BUS_S_PLUS_IN

#define DIO_MULTILINK_SO_IN
#define DIO_MULTILINK_SO_ST

#define DIO_DATALINK_CANERR
#define DIO_DATALINK_CANEN
#define DIO_DATALINK_CANSTB

#define DIO_RF_PWM
#define DIO_RF_C1
#define DIO_RF_C2

#define DIO_IO_SOUND_EN
#define DIO_IO_SOUND

//
// Channel groups
//
#define DIO_CONTROL_BUS			(DioConfigData)
#define DIO_MULTILINK_BUS		(DioConfigData)
#define DIO_RF					(DioConfigData)
#define DIO_IO					(DioConfigData)

//
// Port mapping
//

// Hmm, We have no need for any.. we group with channel group instead

#endif /*DIO_CFG_H_*/
