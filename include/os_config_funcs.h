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
/*
 * This file only be use with the Os_Cfg.c file and os_config_macros.h
 */
#include "internal.h"
#include "Os_Cfg.h"

#error os_config_funcs.h is deprecated

#if 0
/** @req OS327 */
#if (OS_SC3 == STD_ON) || (OS_SC4 == STD_ON)
#if (OS_STATUS_STANDARD == STD_ON )
#error req OS327 not fullfilled
#endif
#endif


// COUNTER, RESOURCE, TSAK, must be at least 1

#if (OS_ALARM_CNT!=0)
#endif

OsErrorType os_error;

//-------------------------------------------------------------------

/*-----------------------------------------------------------------*/
	(void)resource;

/*-----------------------------------------------------------------*/

/*-----------------------------------------------------------------*/

	(void)sched_id;

/*-----------------------------------------------------------------*/


/*-----------------------------------------------------------------*/

#if 0
OsMessageType *Os_CfgGetMessage(MessageType message_id) {
#if (OS_MESSAGE_CNT!=0)
	return &message_list[message_id];
#else
	return NULL;
#endif
}

uint32 Os_CfgGetMessageCnt(void ) {
	return OS_MESSAGE_CNT;
}
#endif

/*-----------------------------------------------------------------*/

#if (OS_USE_APPLICATIONS == STD_ON)
uint32 Os_CfgGetServiceCnt( void ) {
	return OS_SERVICE_CNT;
}
#endif
#endif




