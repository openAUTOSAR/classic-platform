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

#include "Os.h"
#include "arc.h"

#define ENTRY(_x)  [_x] = OS_STRSTR__(_x)

const char *Arc_StatusToString(StatusType err) {

    static char *errToStr[] = {

        ENTRY(E_OK),

        /* STD OSEK */
        ENTRY(E_OS_ACCESS),
        ENTRY(E_OS_CALLEVEL),
        ENTRY(E_OS_ID),
        ENTRY(E_OS_LIMIT),
        ENTRY(E_OS_NOFUNC),
        ENTRY(E_OS_RESOURCE),
        ENTRY(E_OS_STATE),

        ENTRY(E_OS_VALUE),

        /* AUTOSAR, see 7.10 */
        ENTRY(E_OS_SERVICEID),
        ENTRY(E_OS_RATE),
        ENTRY(E_OS_ILLEGAL_ADDRESS ),
        ENTRY(E_OS_MISSINGEND ),
        ENTRY(E_OS_DISABLEDINT ),
        ENTRY(E_OS_STACKFAULT ),
        ENTRY(E_OS_PROTECTION_MEMORY ),
        ENTRY(E_OS_PROTECTION_TIME ),
        ENTRY(E_OS_PROTECTION_LOCKED ),
        ENTRY(E_OS_PROTECTION_EXCEPTION ),
        ENTRY(E_OS_PROTECTION_RATE),

        /* IMPROVMENT: Move it to more appropiate place */
        ENTRY(E_COM_ID),

        /* Implementation specific */

        ENTRY(E_NOT_OK),
    };

    return errToStr[err];
}

