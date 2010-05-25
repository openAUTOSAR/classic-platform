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
#define CHANNEL_PTR		(&DioChannelConfigData)

// Port
extern const Dio_PortType DioPortConfigData[];
#define PORT_PTR		(&DioPortConfigData)

// Channel group
extern const Dio_ChannelGroupType DioConfigData[];
#define CHANNEL_GRP_PTR	(&DioConfigData)

// Mapping of logical ports to physical

// Channels
#define LED_K2		(125)

// Port
// No ports available for MPC5567

// Channel group
#define LED_GRP_PTR		(&DioConfigData[0])

//
// Channels
//
#define CONTROL_BUS_EN
#define CONTROL_BUS_STB
#define CONTROL_BUS_ERR
#define CONTROL_BUS_S_PLUS_ST
#define CONTROL_BUS_S_MINUS_ST
#define CONTROL_BUS_S_PLUS_IN

#define MULTILINK_SO_IN
#define MULTILINK_SO_ST

#define DATALINK_CANERR
#define DATALINK_CANEN
#define DATALINK_CANSTB

#define RF_PWM
#define RF_C1
#define RF_C2

#define IO_SOUND_EN
#define IO_SOUND

//
// Channel groups
//
#define CONTROL_BUS			(DioConfigData)
#define MULTILINK_BUS		(DioConfigData)
#define RF					(DioConfigData)
#define IO					(DioConfigData)

//
// Port mapping
//

// Hmm, We have no need for any.. we group with channel group instead

#endif /*DIO_CFG_H_*/
