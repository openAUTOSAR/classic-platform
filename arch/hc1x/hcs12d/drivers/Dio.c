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

#include "Std_Types.h"
#include "Dio.h"
#include "Det.h"
#include <string.h>
#include "regs.h"

#if ( DIO_VERSION_INFO_API == STD_ON )
static Std_VersionInfoType _Dio_VersionInfo =
{
  .vendorID   = (uint16)1,
  .moduleID   = (uint16)1,
  .instanceID = (uint8)1,
  .sw_major_version = (uint8)DIO_SW_MAJOR_VERSION,
  .sw_minor_version = (uint8)DIO_SW_MINOR_VERSION,
  .sw_patch_version = (uint8)DIO_SW_PATCH_VERSION,
  .ar_major_version = (uint8)DIO_AR_MAJOR_VERSION,
  .ar_minor_version = (uint8)DIO_AR_MINOR_VERSION,
  .ar_patch_version = (uint8)DIO_AR_PATCH_VERSION,
};
#endif

#if ( DIO_DEV_ERROR_DETECT == STD_ON )
static int Channel_Config_Contains(Dio_ChannelType channelId)
{
  return 1;
}

static int Port_Config_Contains(Dio_PortType portId)
{
  return 1;
}

static int Channel_Group_Config_Contains(const Dio_ChannelGroupType* _channelGroupIdPtr)
{
  return 1;
}

#define VALIDATE_CHANNEL(_channelId, _api) \
	if(0==Channel_Config_Contains(channelId)) {	\
		Det_ReportError(MODULE_ID_DIO,0,_api,DIO_E_PARAM_INVALID_CHANNEL_ID ); \
		level = 0;	\
		goto cleanup;	\
		}
#define VALIDATE_PORT(_portId, _api)\
	if(0==Port_Config_Contains(_portId)) {\
		Det_ReportError(MODULE_ID_DIO,0,_api,DIO_E_PARAM_INVALID_PORT_ID ); \
		level = STD_LOW;\
		goto cleanup;\
	}
#define VALIDATE_CHANNELGROUP(_channelGroupIdPtr, _api)\
	if(0==Channel_Group_Config_Contains(_channelGroupIdPtr)) {\
		Det_ReportError(MODULE_ID_DIO,0,_api,DIO_E_PARAM_INVALID_GROUP_ID ); \
		level = STD_LOW;\
		goto cleanup;\
	}
#else
#define VALIDATE_CHANNEL(_channelId, _api)
#define VALIDATE_PORT(_portId, _api)
#define VALIDATE_CHANNELGROUP(_channelGroupIdPtr, _api)
#endif

Dio_LevelType Dio_ReadChannel(Dio_ChannelType channelId)
{
  Dio_LevelType level;
  VALIDATE_CHANNEL(channelId, DIO_READCHANNEL_ID);

  cleanup: return (level);
}

void Dio_WriteChannel(Dio_ChannelType channelId, Dio_LevelType level)
{
  VALIDATE_CHANNEL(channelId, DIO_WRITECHANNEL_ID);

  cleanup: return;
}

Dio_PortLevelType Dio_ReadPort(Dio_PortType portId)
{
  Dio_LevelType level;
  VALIDATE_PORT(portId, DIO_READPORT_ID);


  cleanup: return level;
}

void Dio_WritePort(Dio_PortType portId, Dio_PortLevelType level)
{
  VALIDATE_PORT(portId, DIO_WRITEPORT_ID);

  cleanup: return;
}

Dio_PortLevelType Dio_ReadChannelGroup(
    const Dio_ChannelGroupType *channelGroupIdPtr)
{
  Dio_LevelType level;
  VALIDATE_CHANNELGROUP(channelGroupIdPtr,DIO_READCHANNELGROUP_ID);

  cleanup: return level;
}

void Dio_WriteChannelGroup(const Dio_ChannelGroupType *channelGroupIdPtr,
    Dio_PortLevelType level)
{
  return;
}

#if (DIO_VERSION_INFO_API == STD_ON)
void Dio_GetVersionInfo(Std_VersionInfoType *versionInfo)
{
  memcpy(versionInfo, &_Dio_VersionInfo, sizeof(Std_VersionInfoType));
}
#endif

