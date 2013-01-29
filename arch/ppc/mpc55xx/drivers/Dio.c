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








/*
 * ports: PBxx to PJxx with xx ranging from 0 to 15
 *
 */

#include "Std_Types.h"
#include "Dio.h"
#if defined(USE_DET)
#include "Det.h"
#endif
#include <string.h>
#include "mpc55xx.h"

#define CHANNEL_PTR		(&DioChannelConfigData)
#define CHANNEL_GRP_PTR	(&DioConfigData)
#define PORT_PTR		(&DioPortConfigData)


#if ( DIO_DEV_ERROR_DETECT == STD_ON )
static int Channel_Config_Contains(Dio_ChannelType channelId)
{
  Dio_ChannelType* ch_ptr=(Dio_ChannelType*)CHANNEL_PTR;
  int rv=0;
  while (DIO_END_OF_LIST!=*ch_ptr)
  {
    if (*ch_ptr==channelId)
    { rv=1; break;}
    ch_ptr++;
  }
  return rv;
}

static int Port_Config_Contains(Dio_PortType portId)
{
  Dio_PortType* port_ptr=(Dio_PortType*)PORT_PTR;
  int rv=0;
  while (DIO_END_OF_LIST!=*port_ptr)
  {
    if (*port_ptr==portId)
    { rv=1; break;}
    port_ptr++;
  }
  return rv;
}

static int Channel_Group_Config_Contains(const Dio_ChannelGroupType* _channelGroupIdPtr)
{
  Dio_ChannelGroupType* chGrp_ptr=(Dio_ChannelGroupType*)CHANNEL_GRP_PTR;
  int rv=0;

  while (DIO_END_OF_LIST!=chGrp_ptr->port)
  {
    if (chGrp_ptr->port==_channelGroupIdPtr->port&&
        chGrp_ptr->offset==_channelGroupIdPtr->offset&&
        chGrp_ptr->mask==_channelGroupIdPtr->mask)
    { rv=1; break;}
    chGrp_ptr++;
  }
  return rv;
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
  Dio_LevelType level = STD_LOW;
  VALIDATE_CHANNEL(channelId, DIO_READCHANNEL_ID);
  if (SIU.PCR[channelId].B.IBE) {
    // Read level from SIU.
    if (SIU.GPDI [channelId].R) {
      level = STD_HIGH;
    }
  } else if(SIU.PCR[channelId].B.OBE) {
    // Read level from SIU.
    if (SIU.GPDO [channelId].R) {
      level = STD_HIGH;
    }
  }
#if ( DIO_DEV_ERROR_DETECT == STD_ON )
  cleanup:
#endif
  return (level);
}

void Dio_WriteChannel(Dio_ChannelType channelId, Dio_LevelType level)
{
  VALIDATE_CHANNEL(channelId, DIO_WRITECHANNEL_ID);
  // Write level to SIU.
  if(SIU.PCR[channelId].B.OBE) {
    SIU.GPDO [channelId].R = level;
  }
#if ( DIO_DEV_ERROR_DETECT == STD_ON )
  cleanup:
#endif
  return;
}

Dio_PortLevelType Dio_ReadPort(Dio_PortType portId)
{
  Dio_LevelType level;
  VALIDATE_PORT(portId, DIO_READPORT_ID);

#if defined(CFG_MPC5554)||defined(CFG_MPC5567) || defined(CFG_MPC563XM)
  vuint16_t *ptr = (vuint16_t *)&SIU.GPDI;
#elif defined(CFG_MPC560X)
  vuint16_t *ptr = (vuint16_t *)&SIU.PGPDI;
#else
  vuint16_t *ptr = (vuint16_t *)&SIU.PGPDI0; // The GPDI 0-3 is organized in 32bit chunks but we want to step them in 16bit port-widths
#endif
  level = ptr[portId]; // Read the bit pattern (16bits) to the port
#if ( DIO_DEV_ERROR_DETECT == STD_ON )
  cleanup:
#endif
  return level;
}

void Dio_WritePort(Dio_PortType portId, Dio_PortLevelType level)
{
  VALIDATE_PORT(portId, DIO_WRITEPORT_ID);

  // find address of first port
#if defined(CFG_MPC5554)||defined(CFG_MPC5567) || defined(CFG_MPC563XM)
  vuint16_t *ptr = (vuint16_t *)&SIU.GPDO;
#elif defined(CFG_MPC560X)
  vuint16_t *ptr = (vuint16_t *)&SIU.PGPDO;
#else
  vuint16_t *ptr = (vuint16_t *)&SIU.PGPDO0; // The GPDO 0-3 is organized in 32bit chunks but we want to step them in 16bit port-widths
#endif
  ptr[portId] = level; // Write the bit pattern (16bits) to the port
#if ( DIO_DEV_ERROR_DETECT == STD_ON )
  cleanup:
#endif
  return;
}

Dio_PortLevelType Dio_ReadChannelGroup(
    const Dio_ChannelGroupType *channelGroupIdPtr)
{
  Dio_LevelType level;
  VALIDATE_CHANNELGROUP(channelGroupIdPtr,DIO_READCHANNELGROUP_ID);

  // find address of first port
#if defined(CFG_MPC5554)||defined(CFG_MPC5567) || defined(CFG_MPC563XM)
  vuint16_t *ptr = (vuint16_t *)&SIU.GPDI;
#elif defined(CFG_MPC560X)
  uint32 *ptr = (uint32 *)&SIU.PGPDI;
#else
  uint16 *ptr = (uint16 *)&SIU.PGPDI0; // The GPDI 0-3 is organized in 32bit chunks but we want to step them in 16bit port-widths
#endif

#if defined(CFG_MPC560X)
	if(channelGroupIdPtr->port % 2)
	{
		// Get masked values
		level = ptr[channelGroupIdPtr->port / 2] & channelGroupIdPtr->mask;

		// Shift down
		level>>=channelGroupIdPtr->offset;
	}
	else
	{
		// Get masked values
		level = ptr[channelGroupIdPtr->port / 2] & (channelGroupIdPtr->mask<<16);

		// Shift down
		level>>=(channelGroupIdPtr->offset + 16);
	}
#else
  // Get masked values
  level = ptr[channelGroupIdPtr->port] & channelGroupIdPtr->mask;

  // Shift down
  level<<=channelGroupIdPtr->offset;
#endif

#if ( DIO_DEV_ERROR_DETECT == STD_ON )
  cleanup:
#endif
  return level;
}

void Dio_WriteChannelGroup(const Dio_ChannelGroupType *channelGroupIdPtr,
    Dio_PortLevelType level)
{
#if defined(CFG_MPC560X)
	VALIDATE_CHANNELGROUP(channelGroupIdPtr,DIO_WRITECHANNELGROUP_ID);
	// find address of first port of the masked register
	uint32 *ptr = (uint32 *)&SIU.MPGPDO[0]; // modified by Cobb.The GPDI 0-3 is organized in 32bit chunks but we want to step them in 16bit port-widths

	// Build the 32 bits Mask_Valule, and write to masked output register
	ptr[channelGroupIdPtr->port] = (vuint32_t)((((vuint32_t)channelGroupIdPtr->mask )<< 16)|((((vuint16_t)level)<<channelGroupIdPtr->offset)&0xFFFF));
#if ( DIO_DEV_ERROR_DETECT == STD_ON )
	cleanup:
#endif
	return;
#endif

#if defined(CFG_MPC5516)
  VALIDATE_CHANNELGROUP(channelGroupIdPtr,DIO_WRITECHANNELGROUP_ID);
  // find address of first port of the masked register
  uint32 *ptr = (uint32 *)&SIU.MPGPDO0; // The GPDI 0-3 is organized in 32bit chunks but we want to step them in 16bit port-widths

  // Build the 32 bits Mask_Valule, and write to masked output register
  ptr[channelGroupIdPtr->port] = (channelGroupIdPtr->mask << 16)&((level
      <<channelGroupIdPtr->offset)|0xFFFF);
#if ( DIO_DEV_ERROR_DETECT == STD_ON )
  cleanup:
#endif
  return;
#endif
}

