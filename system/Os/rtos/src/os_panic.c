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

/** @reqSettings DEFAULT_SPECIFICATION_REVISION=4.3.0 */

#include "os_i.h"

/**
 * Interpret the exception and call ProtectionHook/Shutdown hook
 * with a E_OS_xx as argument
 *
 * @param exception The exception number
 * @param pData
 */
void Os_Arc_Panic(uint32 exception, void *pData) {

    (void)pData; /*lint !e920 MISRA:FALSE_POSITIVE:Allowed to cast pointer to void here:[MISRA 2012 Rule 1.3, required]*/
	ProtectionReturnType rv = PRO_SHUTDOWN;

    if ( (OS_SYS_PTR->hooks != NULL_PTR) && (OS_SYS_PTR->hooks->ProtectionHook != NULL_PTR) ) {

        StatusType protArg;
        protArg = Os_ArchGetProtectionType(exception);

        /*  Supported inputs:
         * - E_OS_PROTECTION_MEMORY
         * - E_OS_PROTECTION_EXCEPTION
         */
        if( protArg != E_OS_ILLEGAL) {
            rv = OS_SYS_PTR->hooks->ProtectionHook(protArg);
        }
    } else {
        /* No protection hook */
        /* @req SWS_Os_00107 */ /* If not protection hook, call ShutdownOS() */
    }

    /* if no protection hook is called system calls ShutdownOS */
    if( rv != PRO_IGNORE) {
		/* The rest will be handled PRO_SHUTDOWN */
		/* @req SWS_Os_00556 */ /* for rv == PRO_SHUTDOWN */
		/* @req SWS_Os_00308
		 * Requirement: If ProtectionHook() returns an invalid value, the Operating System module shall take the same action as if no protection hook is configured.
		 * Reference: Requirement OS107 says if no ProtectionHook() configured then OS shall call ShutdownOS().
		 * Note: Though invalid value is specifically checked, but if invalid value comes then requirement OS308 is handled here
		 */
        ShutdownOS(E_OS_PANIC);
    } else  {
        /* Ignore the error and return */
        /* @req SWS_Os_00106 */ /* for protArg == E_OS_PROTECTION_ARRIVAL */
    }

}


