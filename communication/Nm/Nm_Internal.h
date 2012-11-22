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


#ifndef NM_INTERNAL_H_
#define NM_INTERNAL_H_


#define NM_BUSNM_GET_USERDATA_AND_NODEID( busnm, channelconf, dataptr, nodeidptr)			\
	{																						\
		Nm_ReturnType rv, globalReturnVal = NM_E_OK;										\
		rv = busnm##_GetUserData(channelconf->BusNmNetworkHandle, dataptr);					\
		if (rv > globalReturnVal) globalReturnVal = rv;										\
		rv = busnm##_GetNodeIdentifier(channelconf->BusNmNetworkHandle, nodeidptr);			\
		if (rv > globalReturnVal) globalReturnVal = rv;										\
		return globalReturnVal;																\
	}


#endif /* NM_INTERNAL_H_ */
