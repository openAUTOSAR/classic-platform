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








#ifndef PORT_H_
#define PORT_H_

#define PORT_SW_MAJOR_VERSION    	1 
#define PORT_SW_MINOR_VERSION   	0 
#define PORT_SW_PATCH_VERSION    	0 

#define PORT_AR_MAJOR_VERSION     3
#define PORT_AR_MINOR_VERSION     0 
#define PORT_AR_PATCH_VERSION     2 

#include "Port_Cfg.h"

#if PORT_VERSION_INFO_API == STD_ON
void Port_GetVersionInfo( Std_VersionInfoType *versionInfo );
#endif 

#define PORT_E_PARAM_PIN					       0x0a
#define PORT_E_DIRECTION_UNCHANGEABLE		 0x0b
#define PORT_E_PARAM_CONFIG					     0x0c
#define PORT_E_PARAM_INVALID_MODE			   0x0d
#define PORT_E_MODE_UNCHANGEABLE			   0x0e
#define PORT_E_UNINIT						         0x0f

#define PORT_INIT_ID						         0x00
#define PORT_SET_PIN_DIRECTION_ID			   0x01
#define PORT_REFRESH_PORT_DIRECTION_ID   0x02
#define PORT_GET_VERSION_INFO_ID         0x03
#define PORT_SET_PIN_MODE_ID 			       0x04

/*
 * PORT046: The type Port_PinDirectionType is a type for defining the direction of a Port Pin. 
 * PORT_PIN_IN Sets port pin as input. 
 * PORT_PIN_OUT  Sets port pin as output. 
 */
typedef enum
{
  PORT_PIN_IN = 0,
  PORT_PIN_OUT,
} Port_PinDirectionType;

typedef uint32 Port_PinModeType;
typedef enum
{
  PORT_UNINITIALIZED = 0,
  PORT_INITIALIZED,
} Port_StateType;

void Port_Init( const Port_ConfigType *configType );
void Port_SetPinDirection( Port_PinType pin, Port_PinDirectionType direction );
void Port_RefreshPortDirection( void );
void Port_SetPinMode( Port_PinType Pin, Port_PinModeType Mode );

#endif /*PORT_H_*/
