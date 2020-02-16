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


#ifndef COMM_CONFIGTYPES_H_
#define COMM_CONFIGTYPES_H_


typedef enum {
	COMM_BUS_TYPE_CAN,
	COMM_BUS_TYPE_FR,
	COMM_BUS_TYPE_INTERNAL,
	COMM_BUS_TYPE_LIN
} ComM_BusTypeType;

typedef enum {
	COMM_NM_VARIANT_NONE,
	COMM_NM_VARIANT_LIGHT,
	COMM_NM_VARIANT_PASSIVE,
	COMM_NM_VARIANT_FULL
} ComM_NmVariantType;

typedef struct {
	const ComM_BusTypeType			BusType;
	const NetworkHandleType			BusSMNetworkHandle;
	const NetworkHandleType			NmChannelHandle;
	const ComM_NmVariantType		NmVariant;
	const uint32					MainFunctionPeriod;
	const uint32					LightTimeout;
	const uint8						Number;
} ComM_ChannelType;


typedef struct {
	const ComM_ChannelType**		ChannelList;
	const uint8						ChannelCount;
} ComM_UserType;

typedef struct {
	const ComM_ChannelType*			Channels;
	const ComM_UserType*			Users;
} ComM_ConfigType;

#endif /* COMM_CONFIGTYPES_H_ */
