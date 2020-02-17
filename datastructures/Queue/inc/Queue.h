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

#ifndef QUEUE_H_
#define QUEUE_H_

#include <stddef.h>

#include "Os.h"
#include "Std_Types.h"

/*
 * Used to guard against multiple queue imports
 */
#ifndef QUEUE_DEFS

typedef uint8 Queue_ReturnType;

/* @req ARC_SWS_Queue_00009*/
#define QUEUE_E_OK                             (Queue_ReturnType)0u
#define QUEUE_E_TRUE                           (Queue_ReturnType)1u
#define QUEUE_E_NO_INIT                        (Queue_ReturnType)2u
#define QUEUE_E_FULL                           (Queue_ReturnType)130u
#define QUEUE_E_NO_DATA                        (Queue_ReturnType)131u
#define QUEUE_E_LOST_DATA                      (Queue_ReturnType)5u
#define QUEUE_E_NO_CONTAINS                    (Queue_ReturnType)7u
#define QUEUE_E_ALREADY_INIT                   (Queue_ReturnType)8u
#define QUEUE_E_FALSE                          (Queue_ReturnType)9u
#define QUEUE_E_NULL                           (Queue_ReturnType)10u

/* Compare function for contains */
/* @req ARC_SWS_Queue_00008*/
typedef int (*cmpFunc)(void *, void *, size_t);

#define QUEUE_DEFS
#endif

#ifndef MEMCPY
#if defined(__GNUC__)
#define MEMCPY(_x,_y,_z)    __builtin_memcpy(_x,_y,_z)
#else
#include <string.h>
#define MEMCPY(_x,_y,_z)    memcpy(_x,_y,_z)
#define MEMCPY
#endif
#endif /* MEMCPY*/

typedef struct queue {
    /* The max number of elements in the list */
    uint8 max_count;
    uint8 count;

    /* Error flag */
    boolean bufFullFlag;

    /* If queue is initiated */
    boolean isInit;

    /* Size of the elements in the list */
    size_t dataSize;

    /* List head and tail */
    void *head;
    void *tail;

    /* Buffer start/stop */
    void *bufStart;
    void *bufEnd;

    /* Compare function */
    cmpFunc compare_func;
} Queue_t;

/**
 * @brief Initiates the queue
 *
 * @param queue         Address of the queue to be initialized with Queue_Init()
 *                       The queue must be declared as a reference, not just a pointer.
 * @param buffer        Char[] allocated data where the actual contents of the queue should reside.
 * @param max_count     Maximum number of elements that the queue can contain
 * @param dataSize      The size of each element in the queue.
 * @param compare_func  Pointer to a function that has the signature int (*cmpFunc)(void *, void *, size_t).
 *                       Used for comparing elements in the queue
 *
 * @return      QUEUE_E_OK - if successfully initialized.
 * @return      QUEUE_E_ALREADY_INIT - queue already initiated
 */
Queue_ReturnType Queue_Init(Queue_t *queue, void *buffer, uint8 max_count,
        size_t dataSize, cmpFunc compare_func);
/**
 * @brief Add an element to the queue
 *
 * @param queue         Address of the queue that has been initialized with Queue_Init()
 * @param dataPtr       Pointer to the data that is to be added
 *
 * @return      QUEUE_E_OK - if successfully added.
 * @return      QUEUE_E_NO_INIT - the queue pointed to has not been initiated by Queue_Init()
 * @return      QUEUE_E_FULL    - Queue is full.
 */
Queue_ReturnType Queue_Add(Queue_t *queue, void const *dataPtr);
/**
 * @brief Get next entry from the queue. This removes the item from the queue.
 *
 * @param queue    Pointer to the queue that has been initialized with Queue_Init()
 * @param dataPtr  Reference to a variable which will hold the result data
 *
 * @return      QUEUE_E_OK - if successfully popped.
 * @return      QUEUE_E_NO_INIT - the queue pointed to has not been initiated by Queue_Init()
 * @return      QUEUE_E_NO_DATA - nothing popped (it was empty)
 * @return      QUEUE_E_LOST_DATA - if a buffer overflow has occurred previously
 */
Queue_ReturnType Queue_Next(Queue_t *queue, void *dataPtr);
/**
 * @brief Peek (i.e lookup) the next element in the queue. This does not alter the queue.
 *
 * @param queue    Pointer to the queue that has been initialized with Queue_Init()
 * @param dataPtr  Reference to a variable which will hold the result data
 *
 * @return      QUEUE_E_OK - if successfully peeked.
 * @return      QUEUE_E_NO_INIT - the queue pointed to has not been initiated by Queue_Init()
 * @return      QUEUE_E_NO_DATA - nothing to peek at (it was empty)
 * @return      QUEUE_E_LOST_DATA - if a buffer overflow has occurred previously
 */
Queue_ReturnType Queue_Peek(Queue_t const *queue, void *dataPtr);
/**
 * @brief Looks through the queue after element dataPtr points to using its compare_func.
 *
 * @param queue    Pointer to the queue that has been initialized with Queue_Init()
 * @param dataPtr  Reference to a variable which will hold the element it will look for using its compare_func
 *
 * @return      QUEUE_E_TRUE - if queue contains the element
 * @return      QUEUE_E_NO_INIT - the queue pointed to has not been initiated by Queue_Init()
 * @return      QUEUE_E_NO_DATA - nothing to peek at (it was empty)
 * @return      QUEUE_E_FALSE - if the element could not be found in the queue
 *
 */
Queue_ReturnType Queue_Contains(Queue_t const *queue, void const *dataPtr);

#endif /* QUEUE_H_ */

