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


/* macros to remove unavailable BusNm calls */
#if defined(USE_CANNM)
#define NM_CALL_BUSNM_CASE_CAN( service, channelconf, args... )	\
		case NM_BUSNM_CANNM:							\
			return CanNm_##service(channelconf->BusNmNetworkHandle , ## args);
#else
#define NM_CALL_BUSNM_CASE_CAN( service, channelconf, args...)
#endif

#if defined(USE_FRNM)
#define NM_CALL_BUSNM_CASE_FR( service, channelconf, args...  )	\
		case NM_BUSNM_FRNM:							\
			return FrNm_##service(channelconf->BusNmNetworkHandle , ## args);
#else
#define NM_CALL_BUSNM_CASE_FR( service, channelconf, args...)
#endif

#if defined(USE_LINNM)
#define NM_CALL_BUSNM_CASE_LIN( service, channelconf, args...  )	\
		case NM_BUSNM_LINNM:							\
			return LinNm_##service(channelconf->BusNmNetworkHandle , ## args);
#else
#define NM_CALL_BUSNM_CASE_LIN( service, channelconf, args...)
#endif

/* complete switch for BusNm_<service>(...) calls */
#define NM_CALL_BUSNM( service, channelconf, extra...)							\
	switch(channelconf->BusType) {												\
		NM_CALL_BUSNM_CASE_CAN( service, channelconf, ## extra)					\
		NM_CALL_BUSNM_CASE_FR( service, channelconf, ## extra)					\
		NM_CALL_BUSNM_CASE_LIN( service, channelconf, ## extra)					\
		default: return NM_E_NOT_OK;											\
	}

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
