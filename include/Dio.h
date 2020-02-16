/*-------------------------------- Arctic Core ------------------------------
 * Copyright (C) 2013, ArcCore AB, Sweden, www.arccore.com.
 * Contact: <contact@arccore.com>
 * 
 * You may ONLY use this file:
 * 1)if you have a valid commercial ArcCore license and then in accordance with  
 * the terms contained in the written license agreement between you and ArcCore, 
 * or alternatively
 * 2)if you follow the terms found in GNU General Public License version 2 as 
 * published by the Free Software Foundation and appearing in the file 
 * LICENSE.GPL included in the packaging of this file or here 
 * <http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt>
 *-------------------------------- Arctic Core -----------------------------*/








#ifndef DIO_H_
#define DIO_H_

#include "Modules.h"
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

#if defined(CFG_HC1X) || defined(CFG_TMS570)
typedef uint8 Dio_ChannelType;
typedef uint8 Dio_PortType;

/** @req DIO021 */
/** @req DIO022 */
typedef struct
{
  Dio_PortType port;
  uint8 offset;
  uint8 mask;
} Dio_ChannelGroupType;

/** @req DIO023 */
typedef uint8 Dio_LevelType;

/** @req DIO024 */
typedef uint8 Dio_PortLevelType;

#else	// CFG_PPC, CFG_STM32_STAMP and others
typedef uint32 Dio_ChannelType;
typedef uint32 Dio_PortType;
typedef struct
{
  Dio_PortType port;
  uint8 offset;
  uint32 mask;
} Dio_ChannelGroupType;

typedef uint32 Dio_LevelType;

typedef uint16 Dio_PortLevelType;
#endif


#define DIO_MODULE_ID			MODULE_ID_DIO
#define DIO_VENDOR_ID			VENDOR_ID_ARCCORE

#define DIO_SW_MAJOR_VERSION	1
#define DIO_SW_MINOR_VERSION	0
#define DIO_SW_PATCH_VERSION	0

#define DIO_AR_MAJOR_VERSION	2
#define DIO_AR_MINOR_VERSION	2
#define DIO_AR_PATCH_VERSION	0

#include "Dio_Cfg.h"

/** @req DIO124 */
#if ( DIO_VERSION_INFO_API == STD_ON)
/** @req DIO139 */
#define Dio_GetVersionInfo(_vi) STD_GET_VERSION_INFO(_vi,DIO)
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
