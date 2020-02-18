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

/**
 * @brief   Message Box
 *
 * @addtogroup OS
 * @details Implements APIs to post and fetch messages to/from a message box.
 *          The message box only supports one datatype, OsMBoxMsg (uint32_t)
 *          .
 *          Design:
 *          Posting a message is a matter of adding the message to the message
 *          box and signal the semaphore.
 *          Fetching will wait for the semaphore to be signaled, then get the message
 *          from the mailbox.
 *          Messages are added/remove in FIFO order and is implemented by a
 *          circular buffer.
 *          .
 *          The API is tuned to make it easy to port LWIP. The LWIP API is:
 *          - sys_mbox_t sys_mbox_new(int size)
 *          - sys_mbox_free(sys_mbox_t mbox):
 *          - sys_mbox_post(sys_mbox_t mbox, void *msg):
 *          - u32_t sys_arch_mbox_fetch(sys_mbox_t mbox, void **msg, u32_t timeout).
 *          - (u32_t sys_arch_mbox_tryfetch(sys_mbox_t mbox, void **msg) )
 *          .
 *
 * @{
 */

/* ----------------------------[includes]------------------------------------*/
#include "os_i.h"
#include "os_sem.h"
#include "os_mbox.h"

#if defined(CFG_LOG) && (LOG_OS_MBOX)
#define _LOG_NAME_ "os_mbox"
#endif
#include "log.h"


/* ----------------------------[private define]------------------------------*/
/* ----------------------------[private macro]-------------------------------*/
/* ----------------------------[private typedef]-----------------------------*/
/* ----------------------------[private function prototypes]-----------------*/
/* ----------------------------[private variables]---------------------------*/
/* ----------------------------[private functions]---------------------------*/
/* ----------------------------[public functions]----------------------------*/

void MBoxInit( OsMBoxType *mbox, OsMBoxMsg *msgBuff, uint32 msgCnt ) {

    LOG_S("Init");

    mbox->msgBuff = msgBuff;
    mbox->maxCnt = msgCnt;
    mbox->putPtr = msgBuff;
    mbox->getPtr = msgBuff;
    mbox->topPtr = msgBuff + msgCnt;
    /* Set current count */
    mbox->cnt = 0UL;

    SemInit(&mbox->sem,SEM_LOCKED);
}

StatusType MBoxPost(OsMBoxType *mbox, OsMBoxMsg msg) {

    StatusType rv = E_OK;
    imask_t flags;
    ASSERT(mbox != NULL);

    LOG_S("Post");

    Irq_Save(flags);

    /* Check if it's full or not */
    if( mbox->cnt >= mbox->maxCnt ) {
        rv = E_OS_FULL;
    } else {
        mbox->cnt++;

        *mbox->putPtr = msg;
        mbox->putPtr++;
        if( mbox->putPtr >= mbox->topPtr ) {
            mbox->putPtr = mbox->msgBuff;
        }

        /* Wake waiting thread */
        rv = SemSignal(&mbox->sem);
    }

    Irq_Restore(flags);

    return rv;
}

StatusType MBoxFetchTmo(OsMBoxType *mbox, OsMBoxMsg *msg, uint32 tmo) {

    StatusType rv;
    imask_t flags;

    LOG_S("FetchTmo");

    Irq_Save(flags);
    rv = SemWaitTmo(&mbox->sem,tmo);
    if( rv == E_OK ) {
        *msg = *mbox->getPtr;

        mbox->getPtr++;
        if( mbox->getPtr >= mbox->topPtr ) {
            mbox->getPtr = mbox->msgBuff;
        }
        ASSERT(mbox->cnt > 0 );
        --mbox->cnt;
    }
    Irq_Restore(flags);

    return rv;
}

/** @} */
