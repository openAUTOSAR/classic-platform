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


#ifndef OS_MBOX_
#define OS_MBOX_

#include "Std_Types.h"

/**
 * Datatype used by mbox
 */
typedef uint32 OsMBoxMsg;

typedef struct {
    OsMBoxMsg       *msgBuff;   /**< @brief Pointer to mbox data        */

    OsMBoxMsg       *putPtr;    /**< @brief Put data pointer            */
    OsMBoxMsg       *getPtr;    /**< @brief Get data pointer            */
    OsMBoxMsg       *topPtr;    /**< @brief Top of the data             */

    uint32          maxCnt;     /**< @brief Maximum number of elements  */
    uint32          cnt;        /**< @brief Current count of the number
                                            of elements in the mbox     */

    OsSemType       sem;        /**< @brief The semaphore to used to
                                            signal/wait on the mbox.    */
} OsMBoxType;


/**
 * @breif  Initialize a message box.
 *
 * @param mbox      Pointer to a message box.
 * @param msgBuff   Pointer to a buffer to hold the messages.
 * @param msgCnt    Number of messages that the mailbox holds
 */
void MBoxInit( OsMBoxType *mbox, OsMBoxMsg *msgBuff, uint32 msgCnt );

/**
 * @brief   Post a message to a message box.
 *
 * @param[in]  mbox     Pointer to message box.
 * @param[in]  msg      Message to post to box.
 *                      .
 * @retval E_OS_FULL        The message box is full
 */
StatusType MBoxPost(OsMBoxType *mbox, OsMBoxMsg msg);

/**
 * @brief    Fetch a message from a mesasge box.
 * @details  Will fetch a message, msg, from message box, mbox.
 *           The timeout, tmo, set for how long we should
 *           wait for the message.
 *
 * @param[in]  mbox     Pointer to mbox
 * @param[out] msg      The message to fetch
 * @param[in]  tmo      Timeout in ticks.
 *                      TMO_MIN
 *                      .
 *                      Just check the box. If no message,
 *                      just return E_OS_TIMEOUT
 *                      .
 *                      TMO_INFINITE
 *                      Wait forever.
 * @retval E_OK           OK
 * @retval E_OS_TIMEOUT   Timeout
 *
 */
StatusType MBoxFetchTmo(OsMBoxType *mbox, OsMBoxMsg *msg, uint32 tmo);


#endif /*OS_MBOX_*/
