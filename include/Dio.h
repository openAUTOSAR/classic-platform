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

#ifndef DIO_H_
#define DIO_H_

#include "Std_Types.h" /** @req DIO131 */

// API Service ID's
#define DIO_READCHANNEL_ID			0x00
#define DIO_WRITECHANNEL_ID			0x01
#define DIO_READPORT_ID				0x02
#define DIO_WRITEPORT_ID			0x03
#define DIO_READCHANNELGROUP_ID		0x04
#define DIO_WRITECHANNELGROUP_ID	0x05
#define DIO_GETVERSIONINFO_ID		0x12

#define DIO_E_PARAM_INVALID_CHANNEL_ID 		10
#define DIO_E_PARAM_INVALID_PORT_ID 		20
#define DIO_E_PARAM_INVALID_GROUP_ID 		31

#define DIO_SW_MAJOR_VERSION	1
#define DIO_SW_MINOR_VERSION	0
#define DIO_SW_PATCH_VERSION	0

#define DIO_AR_MAJOR_VERSION     2
#define DIO_AR_MINOR_VERSION     2 
#define DIO_AR_PATCH_VERSION     1 

#include "Dio_Cfg.h"

/** @req DIO124 */
#if ( DIO_VERSION_INFO_API == STD_ON)
/** @req DIO139 */
void Dio_GetVersionInfo( Std_VersionInfoType *versionInfo );
#endif

/** @req DIO133 */
/** @req DIO027 */
Dio_LevelType Dio_ReadChannel(Dio_ChannelType channelId);
/** @req DIO134 */
void Dio_WriteChannel(Dio_ChannelType channelId, Dio_LevelType level);

/** @req DIO135 */
/** @req DIO031 */
Dio_PortLevelType Dio_ReadPort(Dio_PortType portId);
/** @req DIO136 */
void Dio_WritePort(Dio_PortType portId, Dio_PortLevelType level);

/** @req DIO137 */
Dio_PortLevelType Dio_ReadChannelGroup( const Dio_ChannelGroupType *channelGroupIdPtr );
/** @req DIO138 */
void Dio_WriteChannelGroup(const Dio_ChannelGroupType *channelGroupIdPtr, Dio_PortLevelType level);

#endif /*DIO_H_*/
