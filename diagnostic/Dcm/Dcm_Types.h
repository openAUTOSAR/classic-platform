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








#ifndef DCM_TYPES_H_
#define DCM_TYPES_H_
#include "Std_Types.h"

/*
 * Dcm_SecLevelType
 */
typedef uint8 Dcm_SecLevelType;
#define DCM_SEC_LEV_LOCKED		0x00
#define DCM_SEC_LEV_L1			0x01
#define DCM_SEC_LEV_ALL			0xFF

/*
 * Dcm_SesCtrlType
 */
typedef uint8 Dcm_SesCtrlType;
#define DCM_DEFAULT_SESSION						0x01
#define DCM_PROGRAMMING_SESSION					0x02
#define DCM_EXTENDED_DIAGNOSTIC_SESSION			0x03
#define DCM_SAFTEY_SYSTEM_DIAGNOSTIC_SESSION	0x04
#define DCM_ALL_SESSION_LEVEL					0xFF

typedef uint8 Dcm_ConfirmationStatusType;
#define DCM_RES_POS_OK							0x00
#define DCM_RES_POS_NOT_OK						0x01
#define DCM_RES_NEG_OK							0x02
#define DECM_RES_NEG_NOT_OK						0x03

/*
 * Dcm_ProtocolType
 */
typedef uint8 Dcm_ProtocolType;
#define DCM_OBD_ON_CAN					0x00
#define DCM_UDS_ON_CAN					0x01
#define DCM_UDS_ON_FLEAXRAY				0x02
#define DCM_ROE_ON_CAN					0x03
#define DCM_ROE_ON_FLEXRAY				0x04
#define DCM_PERIODICTRANS_ON_CAN		0x05
#define DCM_PERIODICTRANS_ON_FLEXRAY	0X06

/*
 * Dcm_NegativeResponseCodeType
 */
typedef uint8 Dcm_NegativeResponseCodeType;
#define DCM_E_GENERALREJECT								0x10
#define DCM_E_BUSYREPEATREQUEST							0x21
#define DCM_E_CONDITIONSNOTCORRECT						0x22
#define DCM_E_REQUESTSEQUENCEERROR						0x24
#define DCM_E_REQUESTOUTOFRANGE							0x31
#define DCM_E_SECUTITYACCESSDENIED						0x33
#define DCM_E_GENERALPROGRAMMINGFAILURE					0x72
#define DCM_E_SUBFUNCTIONNOTSUPPORTEDINACTIVESESSION	0x7E
#define DCM_E_RPMTOOHIGH								0x81
#define DCM_E_RPMTOLOW									0x82
#define DCM_E_ENGINEISRUNNING							0x83
#define DCM_E_ENGINEISNOTRUNNING						0x84
#define DCM_E_ENGINERUNTIMETOOLOW						0x85
#define DCM_E_TEMPERATURETOOHIGH						0x86
#define DCM_E_TEMPERATURETOOLOW							0x87
#define DCM_E_VEHICLESPEEDTOOHIGH						0x88
#define DCM_E_VEHICLESPEEDTOOLOW						0x89
#define DCM_E_THROTTLE_PEDALTOOHIGH						0x8A
#define DCM_E_THROTTLE_PEDALTOOLOW						0x8B
#define DCM_E_TRANSMISSIONRANGENOTINNEUTRAL				0x8C
#define DCM_E_TRANSMISSIONRANGENOTINGEAR				0x8D
#define DCM_E_BRAKESWITCH_NOTCLOSED						0x8F
#define DCM_E_SHIFTERLEVERNOTINPARK						0x90
#define DCM_E_TORQUECONVERTERCLUTCHLOCKED				0x91
#define DCM_E_VOLTAGETOOHIGH							0x92
#define DCM_E_VOLTAGETOOLOW								0x93

#define DCM_E_POSITIVERESPONSE							0x00
#define DCM_E_SERVICENOTSUPPORTED						0x11
#define DCM_E_SUBFUNCTIONNOTSUPPORTED					0x12
#define DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT		0x13
#define DCM_E_RESPONSEPENDING							0x78
#define DCM_E_SERVICENOTSUPPORTEDINACTIVESESSION		0x7F

#endif /*DCM_TYPES_H_*/
