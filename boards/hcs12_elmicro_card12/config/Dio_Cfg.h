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



/** @addtogroup Dio DIO Driver
 *  @{ */

/** @file Dio_Cfg.h
 * Definitions of configuration parameters for the DIO Driver.
 */
#ifndef DIO_CFG_H_
#define DIO_CFG_H_

#define DIO_VERSION_INFO_API    STD_ON
#define DIO_DEV_ERROR_DETECT 	STD_ON
#define DIO_END_OF_LIST  -1

/** HW specific DIO port definitions. */
/** @req DIO018 */
/** @req DIO020 */
typedef enum
{
  DIO_PORT_A = 0,
  DIO_PORT_B,
  DIO_PORT_E,
  DIO_PORT_H,
  DIO_PORT_J,
  DIO_PORT_K,
  DIO_PORT_M,
  DIO_PORT_P,
  DIO_PORT_S,
  DIO_PORT_T
} Dio_PortTypesType;

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

 DIO_CHANNEL_B0,
 DIO_CHANNEL_B1,
 DIO_CHANNEL_B2,
 DIO_CHANNEL_B3,
 DIO_CHANNEL_B4,
 DIO_CHANNEL_B5,
 DIO_CHANNEL_B6,
 DIO_CHANNEL_B7,

 DIO_CHANNEL_E0,
 DIO_CHANNEL_E1,
 DIO_CHANNEL_E2,
 DIO_CHANNEL_E3,
 DIO_CHANNEL_E4,
 DIO_CHANNEL_E5,
 DIO_CHANNEL_E6,
 DIO_CHANNEL_E7,

 DIO_CHANNEL_H0,
 DIO_CHANNEL_H1,
 DIO_CHANNEL_H2,
 DIO_CHANNEL_H3,
 DIO_CHANNEL_H4,
 DIO_CHANNEL_H5,
 DIO_CHANNEL_H6,
 DIO_CHANNEL_H7,

 DIO_CHANNEL_J0,
 DIO_CHANNEL_J1,
 DIO_CHANNEL_J2,
 DIO_CHANNEL_J3,
 DIO_CHANNEL_J4,
 DIO_CHANNEL_J5,
 DIO_CHANNEL_J6,
 DIO_CHANNEL_J7,

 DIO_CHANNEL_K0,
 DIO_CHANNEL_K1,
 DIO_CHANNEL_K2,
 DIO_CHANNEL_K3,
 DIO_CHANNEL_K4,
 DIO_CHANNEL_K5,
 DIO_CHANNEL_K6,
 DIO_CHANNEL_K7,

 DIO_CHANNEL_M0,
 DIO_CHANNEL_M1,
 DIO_CHANNEL_M2,
 DIO_CHANNEL_M3,
 DIO_CHANNEL_M4,
 DIO_CHANNEL_M5,
 DIO_CHANNEL_M6,
 DIO_CHANNEL_M7,

 DIO_CHANNEL_P0,
 DIO_CHANNEL_P1,
 DIO_CHANNEL_P2,
 DIO_CHANNEL_P3,
 DIO_CHANNEL_P4,
 DIO_CHANNEL_P5,
 DIO_CHANNEL_P6,
 DIO_CHANNEL_P7,

 DIO_CHANNEL_S0,
 DIO_CHANNEL_S1,
 DIO_CHANNEL_S2,
 DIO_CHANNEL_S3,
 DIO_CHANNEL_S4,
 DIO_CHANNEL_S5,
 DIO_CHANNEL_S6,
 DIO_CHANNEL_S7,

 DIO_CHANNEL_T0,
 DIO_CHANNEL_T1,
 DIO_CHANNEL_T2,
 DIO_CHANNEL_T3,
 DIO_CHANNEL_T4,
 DIO_CHANNEL_T5,
 DIO_CHANNEL_T6,
 DIO_CHANNEL_T7
} Dio_ChannelTypesType;
//@}

#define DIO_CHANNEL_NAME_LED_CHANNEL 	(DIO_CHANNEL_H7)

#define DIO_PORT_NAME_LED_PORT 		(DIO_PORT_H)

#define DIO_GROUP_NAME_LED_PORT		(&DioConfigData[0])

// Channels
extern const Dio_ChannelType DioChannelConfigData[];

// Port
extern const Dio_PortType DioPortConfigData[];

// Channel group
extern const Dio_ChannelGroupType DioConfigData[];


#endif /*DIO_CFG_H_*/
