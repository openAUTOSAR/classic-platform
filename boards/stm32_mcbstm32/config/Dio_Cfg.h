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

#ifndef DIO_CFG_H_
#define DIO_CFG_H_

#define DIO_VERSION_INFO_API    STD_ON
#define DIO_DEV_ERROR_DETECT 	STD_ON
#define DIO_END_OF_LIST  -1

/** @name DIO channels
 *  HW specific dio channels.
 */
//	Pin Name	GPIO(PCR)Num
//@{
//* req DIO015 */
//* req DIO017 */
typedef enum
{
	 DIO_CHANNEL_A0,
	 DIO_CHANNEL_A1,
	 DIO_CHANNEL_A2,
	 DIO_CHANNEL_A3,
	 DIO_CHANNEL_A4,
	 DIO_CHANNEL_A5,
	 DIO_CHANNEL_A6,
	 DIO_CHANNEL_A7,
	 DIO_CHANNEL_A8,
	 DIO_CHANNEL_A9,
	 DIO_CHANNEL_A10,
	 DIO_CHANNEL_A11,
	 DIO_CHANNEL_A12,
	 DIO_CHANNEL_A13,
	 DIO_CHANNEL_A14,
	 DIO_CHANNEL_A15,

	 DIO_CHANNEL_B0,
	 DIO_CHANNEL_B1,
	 DIO_CHANNEL_B2,
	 DIO_CHANNEL_B3,
	 DIO_CHANNEL_B4,
	 DIO_CHANNEL_B5,
	 DIO_CHANNEL_B6,
	 DIO_CHANNEL_B7,
	 DIO_CHANNEL_B8,
	 DIO_CHANNEL_B9,
	 DIO_CHANNEL_B10,
	 DIO_CHANNEL_B11,
	 DIO_CHANNEL_B12,
	 DIO_CHANNEL_B13,
	 DIO_CHANNEL_B14,
	 DIO_CHANNEL_B15,

	 DIO_CHANNEL_C0,
	 DIO_CHANNEL_C1,
	 DIO_CHANNEL_C2,
	 DIO_CHANNEL_C3,
	 DIO_CHANNEL_C4,
	 DIO_CHANNEL_C5,
	 DIO_CHANNEL_C6,
	 DIO_CHANNEL_C7,
	 DIO_CHANNEL_C8,
	 DIO_CHANNEL_C9,
	 DIO_CHANNEL_C10,
	 DIO_CHANNEL_C11,
	 DIO_CHANNEL_C12,
	 DIO_CHANNEL_C13,
	 DIO_CHANNEL_C14,
	 DIO_CHANNEL_C15,

	 DIO_CHANNEL_D0,
	 DIO_CHANNEL_D1,
	 DIO_CHANNEL_D2,
	 DIO_CHANNEL_D3,
	 DIO_CHANNEL_D4,
	 DIO_CHANNEL_D5,
	 DIO_CHANNEL_D6,
	 DIO_CHANNEL_D7,
	 DIO_CHANNEL_D8,
	 DIO_CHANNEL_D9,
	 DIO_CHANNEL_D10,
	 DIO_CHANNEL_D11,
	 DIO_CHANNEL_D12,
	 DIO_CHANNEL_D13,
	 DIO_CHANNEL_D14,
	 DIO_CHANNEL_D15,

	 DIO_CHANNEL_E0,
	 DIO_CHANNEL_E1,
	 DIO_CHANNEL_E2,
	 DIO_CHANNEL_E3,
	 DIO_CHANNEL_E4,
	 DIO_CHANNEL_E5,
	 DIO_CHANNEL_E6,
	 DIO_CHANNEL_E7,
	 DIO_CHANNEL_E8,
	 DIO_CHANNEL_E9,
	 DIO_CHANNEL_E10,
	 DIO_CHANNEL_E11,
	 DIO_CHANNEL_E12,
	 DIO_CHANNEL_E13,
	 DIO_CHANNEL_E14,
	 DIO_CHANNEL_E15,

	 DIO_CHANNEL_F0,
	 DIO_CHANNEL_F1,
	 DIO_CHANNEL_F2,
	 DIO_CHANNEL_F3,
	 DIO_CHANNEL_F4,
	 DIO_CHANNEL_F5,
	 DIO_CHANNEL_F6,
	 DIO_CHANNEL_F7,
	 DIO_CHANNEL_F8,
	 DIO_CHANNEL_F9,
	 DIO_CHANNEL_F10,
	 DIO_CHANNEL_F11,
	 DIO_CHANNEL_F12,
	 DIO_CHANNEL_F13,
	 DIO_CHANNEL_F14,
	 DIO_CHANNEL_F15,

} Dio_Hw_ChannelType;
//@}

/** HW specific DIO port definitions. */
/** @req DIO018 */
/** @req DIO020 */
typedef enum {
	DIO_PORT_A,
	DIO_PORT_B,
	DIO_PORT_C,
	DIO_PORT_D,
	DIO_PORT_E,
	DIO_PORT_F,
} Dio_Hw_PortType;

// Channels
#define DIO_CHANNEL_NAME_LED_CHANNEL1 (DIO_CHANNEL_B13)
#define DIO_CHANNEL_NAME_LED_CHANNEL2 (DIO_CHANNEL_B14)

// Channel group
#define DIO_GROUP_NAME_LED_GROUP			(&DioConfigData[0])

// Ports
#define DIO_PORT_NAME_LED_PORT 		(DIO_PORT_B)



// Pointers for convenience.
// Channels
extern const Dio_ChannelType DioChannelConfigData[];
// Port
extern const Dio_PortType DioPortConfigData[];
// Channel group
extern const Dio_ChannelGroupType DioConfigData[];

#endif /* DIO_CFG_H_ */
