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
    
    StatusType protArg;
    protArg = Os_ArchGetProtectionType(exception);

    Os_CallProtectionHook(protArg);
}


