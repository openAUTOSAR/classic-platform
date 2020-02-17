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

#include "LinuxOs_StbM.h"

#include "Std_Types.h"

#include "linos_logger.h" /* Logger functions */

#include "linos_time.h"

#include "Os.h"

#ifndef _WIN32
/* Not POSIX, but needed for debugging using pthread_setname_np */
#define __USE_GNU
#include <pthread.h>
#include <math.h>
#endif

typedef enum {
    NEVER_SYNCED, SYNCED
} synced_flag;

/* Copied from core/system/StbM/StbM.c */
#define StbM_SET_BIT(byte,mask) ((byte) |= (mask))
#define StbM_CLR_BIT(byte,mask) ((byte) &= (~mask))
#define StbM_GET_BIT(byte,mask) (byte & (mask))

/* Copied from core/system/StbM/StbM.c */
#define STBM_TIMEOUT_MASK            0x01u
#define STBM_TIMELEAP_MASK           0x02u
#define STBM_SYNC_TO_GATEWAY_MASK    0x04u
#define STBM_GLOBAL_TIME_BASE_MASK   0x08u

/* @req 4.2.2/SWS_StbM_00241 */
#define STMB_MAX_NSECS 0x3B9AC9FF
#define STMB_MAX_SECS_HI 0xFFFF

/* Synchronization for: */
/* @req 4.2.2/SWS_StbM_00052 */
/* @req 4.2.2/SWS_StbM_00195 */
/* @req 4.2.2/SWS_StbM_00233 */
#ifndef _WIN32
pthread_mutex_t mutex_linos_StbM_Init;
#endif
/* @req 4.2.2/SWS_StbM_00100 */
static int StbM_Initialized = 0;

/* @req 4.2.2/ECUC_StbM_00028 */
typedef float EcucFloatParamDef;

/* @req 4.2.2/ECUC_StbM_00028 */
/* Unit: seconds.
 * Set to a 2 seconds to allow a maximum of one missed update to the slowest
 * version of the gptp, IEEE 1588-2002, which sends synchronization messages
 * every 1 second.
 */
const EcucFloatParamDef StbMSyncLossTimeout = 2; /* Read explanation above. */

/* Synchronization for: */
/* @req 4.2.2/SWS_StbM_00052 */
/* @req 4.2.2/SWS_StbM_00195 */
/* @req 4.2.2/SWS_StbM_00233 */
#ifndef _WIN32
pthread_mutex_t mutex_linos_StbM_Status;
#endif

typedef struct collective_linos_StbM_Status {
#ifndef _WIN32
    struct timespec last_StbM_BusSetGlobalTime;
#endif
    int ptp_in_sync;
    float ptp_offset;
    synced_flag is_ever_synced;
    StbM_TimeBaseStatusType timeBaseStatus;
} collective_linos_StbM_Status;
collective_linos_StbM_Status linos_StbM_Status;

#ifndef _WIN32
const int sizeof__time_t = sizeof(__time_t);
#endif

#define LINOS_STBM_LOG_PREFIX "linos_StbM: "

/* Maximum offset from PTP master is based on OsTickFreq */
#define MSEC_TICK 1000/OsTickFreq
#define NSEC_TICK MSEC_TICK*1000000


/* deviation from @req 4.2.2/SWS_StbM_00250, no parameter to StbM_Init.
Anyway, it is alwyas null an not used. */
/* @req 4.2.2/SWS_StbM_00052 */
void Linos_StbM_Init() {
#ifndef _WIN32

    /**
     * Design note: two lock to allow other StbM functions to do tasks between
     * the check that StbM is initialized and doing status updates.
     *
     * However, we use the two lock in order in all functions.
     */

    /* Setting the initialization flag. */
    pthread_mutex_lock(&mutex_linos_StbM_Init);
    /* Those are the only two sizeof(__time_t) that we can handle. */
    if (sizeof__time_t == 4 || sizeof__time_t == 8) {
        /* @req 4.2.2/SWS_StbM_00121 */
        StbM_Initialized = 1;
    } else {
        logger(LOG_ERR, "%s StbM_Init failed, "
                "can only handle __time_t of sizes 4 and 8.",
                LINOS_STBM_LOG_PREFIX);
        // Do not return, see implemented requirements and Design note above.
    }
    pthread_mutex_unlock(&mutex_linos_StbM_Init);

    /* Anyway, we have to set timeBaseStatus to 0*/
    /* @req 4.2.2/SWS_StbM_00170*/
    pthread_mutex_lock(&mutex_linos_StbM_Status);
    linos_StbM_Status.last_StbM_BusSetGlobalTime.tv_nsec = 0;
    linos_StbM_Status.last_StbM_BusSetGlobalTime.tv_sec = 0;
    linos_StbM_Status.ptp_in_sync = 0;
    linos_StbM_Status.ptp_offset = 0xFFFFFFFF;
    linos_StbM_Status.is_ever_synced = NEVER_SYNCED;
    linos_StbM_Status.timeBaseStatus = 0;
    pthread_mutex_unlock(&mutex_linos_StbM_Status);
#endif
}

Std_ReturnType Linos_StbM_BusSetGlobalTime(StbM_SynchronizedTimeBaseType timeBaseId,
        const StbM_TimeStampType* timeStampPtr,
        const StbM_UserDataType* userDataPtr, boolean syncToTimeBase,
        int ptp_in_sync, float ptp_offset) {
#ifndef _WIN32
    /**
     * Design note: two lock to allow other StbM functions to do tasks between
     * the check that StbM is initialized and doing status updates.
     *
     * However, we use the two lock in order in all functions.
     */
    Std_ReturnType retValue = E_OK;

    /* Checking the initialization flag. */
    pthread_mutex_lock(&mutex_linos_StbM_Init);
    retValue = E_OK;
    if (!StbM_Initialized) {
        retValue = E_NOT_OK;
    }
    pthread_mutex_unlock(&mutex_linos_StbM_Init);

    if (retValue != E_OK) {
        logger(LOG_ERR,
                "%s Linos_StbM_BusSetGlobalTime failed, StbM is not initialized.",
                LINOS_STBM_LOG_PREFIX);
        return retValue;
    }

    ptp_offset = fabsf(ptp_offset);
    logger(LOG_DEBUG, "Linos_StbM_BusSetGlobalTime got ptp (in-sync, offset):"
            " (%d, %f)", ptp_in_sync, ptp_offset);

    /* For update timeout measurement, get the time from the monotonic clock. */
    /* We do this outside the mutex locking block below. */
    struct timespec current_time;
    clock_gettime(CLOCK_MONOTONIC, &current_time);

    pthread_mutex_lock(&mutex_linos_StbM_Status);

    linos_StbM_Status.last_StbM_BusSetGlobalTime.tv_nsec = current_time.tv_nsec;
    linos_StbM_Status.last_StbM_BusSetGlobalTime.tv_sec = current_time.tv_sec;

    linos_StbM_Status.ptp_in_sync = ptp_in_sync;
    linos_StbM_Status.ptp_offset = ptp_offset;

    if (ptp_in_sync && linos_StbM_Status.is_ever_synced == NEVER_SYNCED) {
        linos_StbM_Status.is_ever_synced = SYNCED;
        /* @req 4.2.2/SWS_StbM_00185 */
        StbM_SET_BIT(linos_StbM_Status.timeBaseStatus,
                STBM_GLOBAL_TIME_BASE_MASK);
    }

    pthread_mutex_unlock(&mutex_linos_StbM_Status);

    return retValue;
#else
    return E_OK;
#endif
}

/* @req 4.2.2/SWS_StbM_00195 */
Std_ReturnType Linos_StbM_GetCurrentTime(StbM_SynchronizedTimeBaseType timeBaseId,
        StbM_TimeStampType* timeStampPtr, StbM_UserDataType* userDataPtr ) {
#ifndef _WIN32
    /**
     * Design note: two lock to allow other StbM functions to do tasks between
     * the check that StbM is initialized and doing status updates.
     *
     * However, we use the two lock in order in all functions.
     */
    Std_ReturnType retValue = E_OK;

    /* Checking the initialization flag. */
    pthread_mutex_lock(&mutex_linos_StbM_Init);
    if (!StbM_Initialized) {
        retValue = E_NOT_OK;
        /* Avoid to return while locking the mutex. */
    }
    pthread_mutex_unlock(&mutex_linos_StbM_Init);

    if (retValue != E_OK) {
        logger(LOG_ERR,
                "%s Linos_StbM_GetCurrentTime failed, StbM is not initialized.",
                LINOS_STBM_LOG_PREFIX);
        return retValue ;
    }

    struct timespec tpsys;
    retValue = Linos_GetCurrentTime(&tpsys);
    /* @req 4.2.2/SWS_StbM_00199 */
    /* On error, return without touching the parameters. */
    if (retValue != E_OK) {
        logger(LOG_ERR,
                "%s Linos_StbM_GetCurrentTime failed, Error getting system time.",
                LINOS_STBM_LOG_PREFIX);
        /*Improvement: review AUTOSAR error handling requirements. */
        return retValue;
    }
    /* @req 4.2.2/SWS_StbM_00199 */
    /* On error, return without touching the parameters. */
    if ((tpsys.tv_nsec < 0) || (tpsys.tv_sec < 0)) {
        logger(LOG_ERR,
                "%s Linos_StbM_GetCurrentTime failed, Negative system time.",
                LINOS_STBM_LOG_PREFIX);
        /* Improvement: review AUTOSAR error handling requirements. */
        return E_NOT_OK ;
    }

    /* @req 4.2.2/SWS_StbM_00241 */
    uint32 tpsys_secondsHi = 0;
    if (sizeof__time_t == 8) {
        /* Bit wise shifting must be within variable size. */
        tpsys_secondsHi = (tpsys.tv_sec >> 32) & 0xFFFFFFFF;
    }
    if (tpsys.tv_nsec <= STMB_MAX_NSECS && tpsys_secondsHi <= STMB_MAX_SECS_HI) {
        timeStampPtr->nanoseconds = tpsys.tv_nsec & 0xFFFFFFFF;
        timeStampPtr->seconds = tpsys.tv_sec & 0xFFFFFFFF;
        timeStampPtr->secondsHi = tpsys_secondsHi & 0xFFFF;
    } else {
        /* @req 4.2.2/SWS_StbM_00199 */
        /* On error, return without touching the parameters. */
        logger(LOG_ERR,
                "%s Linos_StbM_GetCurrentTime failed, Invalid system time.",
                LINOS_STBM_LOG_PREFIX);
        /* Improvement review AUTOSAR error handling requirements. */
        return E_NOT_OK ;
    }

    /* For update timeout measurement, get the time from the monotonic clock. */
    /* We do this outside the mutex locking block below. */
    struct timespec current_time;
    clock_gettime(CLOCK_MONOTONIC, &current_time);

    /* @req 4.2.2/SWS_StbM_00195 */
    pthread_mutex_lock(&mutex_linos_StbM_Status);

    /* Starting with TIMEOUT bit as set, covering:
     * 1. StbM_BusSetGlobalTime is never called.
     * 2. Not in_sync with ptp master.
     * 3. An overlapping MONTONIC clock.
     * 4. Timed out since last invocation of StbM_BusSetGlobalTime. */
    StbM_SET_BIT(linos_StbM_Status.timeBaseStatus, STBM_TIMEOUT_MASK);

    struct timespec time_since_last_StbM_BusSetGlobalTime;
    /* StbMSyncLossTimeout is defined in seconds, so we skip the nsec. */
    time_since_last_StbM_BusSetGlobalTime.tv_nsec = 0;
    time_since_last_StbM_BusSetGlobalTime.tv_sec =
            current_time.tv_sec - linos_StbM_Status.last_StbM_BusSetGlobalTime.tv_sec;
    if (linos_StbM_Status.is_ever_synced != NEVER_SYNCED &&
            linos_StbM_Status.ptp_in_sync &&
            time_since_last_StbM_BusSetGlobalTime.tv_sec >= 0 &&
            time_since_last_StbM_BusSetGlobalTime.tv_sec < StbMSyncLossTimeout) {
        /* Clearing the TIMEOUT bit. */
        StbM_CLR_BIT(linos_StbM_Status.timeBaseStatus, STBM_TIMEOUT_MASK);
    }

    /* Starting wit the STBM_TIMELEAP_MASK as set. */
    StbM_SET_BIT(linos_StbM_Status.timeBaseStatus, STBM_TIMELEAP_MASK);
    if (linos_StbM_Status.ptp_offset < NSEC_TICK)
        StbM_CLR_BIT(linos_StbM_Status.timeBaseStatus, STBM_TIMELEAP_MASK);

    timeStampPtr->timeBaseStatus = linos_StbM_Status.timeBaseStatus;
    pthread_mutex_unlock(&mutex_linos_StbM_Status);

    logger(LOG_DEBUG,
            "%s Linos_StbM_GetCurrentTime (timeBaseStatus, lastUpdate): (0x%x, %lld)",
            LINOS_STBM_LOG_PREFIX, timeStampPtr->timeBaseStatus,
            (long long)time_since_last_StbM_BusSetGlobalTime.tv_sec);

    return retValue;
#else
    return E_OK;
#endif
}
