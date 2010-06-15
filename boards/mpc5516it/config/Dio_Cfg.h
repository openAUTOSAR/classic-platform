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
typedef enum
{
  DIO_PORT_A = 0,
  DIO_PORT_B,
  DIO_PORT_C,
  DIO_PORT_D,
  DIO_PORT_E,
  DIO_PORT_F,
  DIO_PORT_G,
  DIO_PORT_H,
  DIO_PORT_J,
  DIO_PORT_K
} Dio_PortTypesType;

/** @name DIO channels
 *  HW specific dio channels.
 */
//	Pin Name	GPIO(PCR)Num
//@{
#define DIO_CHANNEL_A0 		0
#define DIO_CHANNEL_A1		1
#define DIO_CHANNEL_A2		2
#define DIO_CHANNEL_A3		3
#define DIO_CHANNEL_A4		4
#define DIO_CHANNEL_A5		5
#define DIO_CHANNEL_A6		6
#define DIO_CHANNEL_A7		7
#define DIO_CHANNEL_A8		8
#define DIO_CHANNEL_A9		9
#define DIO_CHANNEL_A10		10
#define DIO_CHANNEL_A11		11
#define DIO_CHANNEL_A12		12
#define DIO_CHANNEL_A13		13
#define DIO_CHANNEL_A14		14
#define DIO_CHANNEL_A15		15

#define DIO_CHANNEL_B0		16
#define DIO_CHANNEL_B1		17
#define DIO_CHANNEL_B2		18
#define DIO_CHANNEL_B3		19
#define DIO_CHANNEL_B4		20
#define DIO_CHANNEL_B5		21
#define DIO_CHANNEL_B6		22
#define DIO_CHANNEL_B7		23
#define DIO_CHANNEL_B8		24
#define DIO_CHANNEL_B9		25
#define DIO_CHANNEL_B10		26
#define DIO_CHANNEL_B11		27
#define DIO_CHANNEL_B12		28
#define DIO_CHANNEL_B13		29
#define DIO_CHANNEL_B14		30
#define DIO_CHANNEL_B15		31

#define DIO_CHANNEL_C0		32
#define DIO_CHANNEL_C1		33
#define DIO_CHANNEL_C2		34
#define DIO_CHANNEL_C3		35
#define DIO_CHANNEL_C4		36
#define DIO_CHANNEL_C5		37
#define DIO_CHANNEL_C6		38
#define DIO_CHANNEL_C7		39
#define DIO_CHANNEL_C8		40
#define DIO_CHANNEL_C9		41
#define DIO_CHANNEL_C10		42
#define DIO_CHANNEL_C11		43
#define DIO_CHANNEL_C12		44
#define DIO_CHANNEL_C13		45
#define DIO_CHANNEL_C14		46
#define DIO_CHANNEL_C15		47

#define DIO_CHANNEL_D0		48
#define DIO_CHANNEL_D1		49
#define DIO_CHANNEL_D2		50
#define DIO_CHANNEL_D3		51
#define DIO_CHANNEL_D4		52
#define DIO_CHANNEL_D5		53
#define DIO_CHANNEL_D6		54
#define DIO_CHANNEL_D7		55
#define DIO_CHANNEL_D8		56
#define DIO_CHANNEL_D9		57
#define DIO_CHANNEL_D10		58
#define DIO_CHANNEL_D11		59
#define DIO_CHANNEL_D12		60
#define DIO_CHANNEL_D13		61
#define DIO_CHANNEL_D14		62
#define DIO_CHANNEL_D15		63

#define DIO_CHANNEL_E0		64
#define DIO_CHANNEL_E1		65
#define DIO_CHANNEL_E2		66
#define DIO_CHANNEL_E3		67
#define DIO_CHANNEL_E4		68
#define DIO_CHANNEL_E5		69
#define DIO_CHANNEL_E6		70
#define DIO_CHANNEL_E7		71
#define DIO_CHANNEL_E8		72
#define DIO_CHANNEL_E9		73
#define DIO_CHANNEL_E10		74
#define DIO_CHANNEL_E11		75
#define DIO_CHANNEL_E12		76
#define DIO_CHANNEL_E13		77
#define DIO_CHANNEL_E14		78
#define DIO_CHANNEL_E15		79

#define DIO_CHANNEL_F0		80
#define DIO_CHANNEL_F1		81
#define DIO_CHANNEL_F2		82
#define DIO_CHANNEL_F3		83
#define DIO_CHANNEL_F4		84
#define DIO_CHANNEL_F5		85
#define DIO_CHANNEL_F6		86
#define DIO_CHANNEL_F7		87
#define DIO_CHANNEL_F8		88
#define DIO_CHANNEL_F9		89
#define DIO_CHANNEL_F10		90
#define DIO_CHANNEL_F11		91
#define DIO_CHANNEL_F12		92
#define DIO_CHANNEL_F13		93
#define DIO_CHANNEL_F14		94
#define DIO_CHANNEL_F15		95

#define DIO_CHANNEL_G0		96
#define DIO_CHANNEL_G1		97
#define DIO_CHANNEL_G2		98
#define DIO_CHANNEL_G3		99
#define DIO_CHANNEL_G4		100
#define DIO_CHANNEL_G5		101
#define DIO_CHANNEL_G6		102
#define DIO_CHANNEL_G7		103
#define DIO_CHANNEL_G8		104
#define DIO_CHANNEL_G9		105
#define DIO_CHANNEL_G10		106
#define DIO_CHANNEL_G11		107
#define DIO_CHANNEL_G12		108
#define DIO_CHANNEL_G13		109
#define DIO_CHANNEL_G14		110
#define DIO_CHANNEL_G15		111

#define DIO_CHANNEL_H0		112
#define DIO_CHANNEL_H1		113
#define DIO_CHANNEL_H2		114
#define DIO_CHANNEL_H3		115
#define DIO_CHANNEL_H4		116
#define DIO_CHANNEL_H5		117
#define DIO_CHANNEL_H6		118
#define DIO_CHANNEL_H7		119
#define DIO_CHANNEL_H8		120
#define DIO_CHANNEL_H9		121
#define DIO_CHANNEL_H10		122
#define DIO_CHANNEL_H11		123
#define DIO_CHANNEL_H12		124
#define DIO_CHANNEL_H13		125
#define DIO_CHANNEL_H14		126
#define DIO_CHANNEL_H15		127

#define DIO_CHANNEL_J0		128
#define DIO_CHANNEL_J1		129
#define DIO_CHANNEL_J2		130
#define DIO_CHANNEL_J3		131
#define DIO_CHANNEL_J4		132
#define DIO_CHANNEL_J5		133
#define DIO_CHANNEL_J6		134
#define DIO_CHANNEL_J7		135
#define DIO_CHANNEL_J8		136
#define DIO_CHANNEL_J9		137
#define DIO_CHANNEL_J10		138
#define DIO_CHANNEL_J11		139
#define DIO_CHANNEL_J12		140
#define DIO_CHANNEL_J13		141
#define DIO_CHANNEL_J14		142
#define DIO_CHANNEL_J15		143

#define DIO_CHANNEL_K0		144
#define DIO_CHANNEL_K1		145
//@}

// Channels	
extern const Dio_ChannelType DioChannelConfigData[];
// Port
extern const Dio_PortType DioPortConfigData[];
// Channel group
extern const Dio_ChannelGroupType DioConfigData[];

// Mapping of logical ports to physical 
#ifdef CFG_BRD_MPC5516IT

// Channels	
#define DIO_CHANNEL_NAME_LEDS_LED4		(DIO_CHANNEL_D4)
#define DIO_CHANNEL_NAME_LEDS_LED5		(DIO_CHANNEL_D5)

// Port
#define DIO_PORT_NAME_LED_PORT 			(DIO_PORT_D)

// Channel group
#define DIO_GROUP_NAME_LED_GRP_PTR		(&DioConfigData[0])

#else
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
#endif

#endif /*DIO_CFG_H_*/
