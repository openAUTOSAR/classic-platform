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

#ifndef _WIN32

#include "linos_time.h"

#include "linos_logger.h" /* Logger functions */

Std_ReturnType Linos_GetCurrentTime(struct timespec *tp ) {
    if (clock_gettime(CLOCK_REALTIME, tp)) {
        logger(LOG_ERR, "Linos_GetCurrentTime. problems getting CLOCK_REALTIME");
        return E_NOT_OK;
    }
    return E_OK;
}

#endif /* Not defined _WIN32 */
