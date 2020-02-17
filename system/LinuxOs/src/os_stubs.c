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
 *
 * Stubbed functions to get a working system
 *
*/

#include <Os.h>
#include <Com_Types.h>
#include <Rte_EcuM_Type.h>

//#include <os_i.h>       /* OS_SYS_PTR */
#include <time.h>

#include "linos_logger.h" /* Logger functions */
#include "os_stubs.h"


TickType GetOsTick(void) {
    //return OS_SYS_PTR->tick;
    //return time(0);
    return clock();
}

void tcpip_task( void ) {
    logger(LOG_INFO, "tcpip_task called.");
}

#if defined(USE_RTE)
Std_ReturnType Os_GetCounterValue(CounterType counterId, TickType * value) {
	/*IMPROVEMENT :Currently stubbed */
    return E_OK;
}


Std_ReturnType Os_GetElapsedValue(CounterType counterId, TickType * value, TickType * elapsedValue) {
	/*IMPROVEMENT :Currently stubbed */
    return E_OK;
}
#endif /* #if defined(USE_RTE) */ 
