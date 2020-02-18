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
#include "Queue.h"

/* @req ARC_SWS_Queue_00001 The queue shall use FIFO (First in, First out) order.*/
/* @req ARC_SWS_Queue_00002 The queue shall support any datatype defined by user.*/
/* @req ARC_SWS_Queue_00010 The files of the queue shall be named: Queue.c and Queue.h */
/* @req ARC_SWS_Queue_00011 The queue shall not use native C types. */
/* @req ARC_SWS_Queue_00012 The queue shall be ready to be invoked at any time */
/* @req ARC_SWS_Queue_00013 The queue shall not call any BSW modules function */
/*
 *  A circular buffer implementation of fifo queue.*
 */
/* @req ARC_SWS_Queue_00003 */
Queue_ReturnType Queue_Init(Queue_t *queue, void *buffer, uint8 max_count,
        size_t dataSize, cmpFunc cmp) {
    SYS_CALL_SuspendOSInterrupts();

    if ((queue == NULL_PTR) || (buffer == NULL_PTR) || (cmp == NULL_PTR)) {
        SYS_CALL_ResumeOSInterrupts();
        /*lint -e904 MISRA:OTHER:Validation of parameters, if failure, function will return.This is not inline with Table 8, ISO26262-6-2011, Req 1a:[MISRA 2012 Rule 15.5, advisory]*/
        return QUEUE_E_NULL;
    }

    if (queue->isInit == TRUE) {
        SYS_CALL_ResumeOSInterrupts();
        return QUEUE_E_ALREADY_INIT;
    }

    queue->bufStart = buffer;
    /*lint -e970 MISRA:OTHER:argument check:[MISRA 2012 Directive 4.6, advisory]*/
    queue->bufEnd = (char *) buffer + (max_count * dataSize); /*lint !e9016 MISRA:OTHER:correct arithmetic even if Array index is not used:[MISRA 2012 Rule 18.4, advisory]*/
    queue->head = queue->bufStart;
    queue->tail = queue->bufStart;
    queue->dataSize = dataSize;
    queue->count = 0u;
    queue->max_count = max_count;
    queue->compare_func = *cmp;
    queue->isInit = TRUE;
    queue->bufFullFlag = FALSE;

    SYS_CALL_ResumeOSInterrupts();
    return QUEUE_E_OK;
}
/* @req ARC_SWS_Queue_00004 */
Queue_ReturnType Queue_Add(Queue_t *queue, void const *dataPtr) {
    SYS_CALL_SuspendOSInterrupts();


    if ((queue == NULL_PTR) || (dataPtr == NULL_PTR)) {
        SYS_CALL_ResumeOSInterrupts();
        /*lint -e904 MISRA:OTHER:Validation of parameters, if failure, function will return.This is not inline with Table 8, ISO26262-6-2011, Req 1a:[MISRA 2012 Rule 15.5, advisory]*/
        return QUEUE_E_NULL;
    }
    //Not initialized
    if (queue->isInit != TRUE) {
        SYS_CALL_ResumeOSInterrupts();
        return QUEUE_E_NO_INIT; /* Faulty pointer into method */
    }
    //Queue is full
    if (queue->count == queue->max_count) {
        queue->bufFullFlag = TRUE;
        SYS_CALL_ResumeOSInterrupts();
        return QUEUE_E_FULL; /* No more room */
    }

    MEMCPY(queue->head, dataPtr, queue->dataSize);
    /*lint -e970 MISRA:OTHER:argument check:[MISRA 2012 Directive 4.6, advisory]*/
    queue->head = (char *) queue->head + queue->dataSize; /*lint !e9016 MISRA:OTHER:correct arithmetic even if Array index is not used:[MISRA 2012 Rule 18.4, advisory]*/

    //Wrap-around
    if (queue->head == queue->bufEnd) {
        queue->head = queue->bufStart;
    }
    queue->count++;

    SYS_CALL_ResumeOSInterrupts();
    return QUEUE_E_OK;
}
/* @req ARC_SWS_Queue_00005 */
Queue_ReturnType Queue_Next(Queue_t *queue, void *dataPtr) {
    SYS_CALL_SuspendOSInterrupts();

    if ((queue == NULL_PTR) || (dataPtr == NULL_PTR)) {
        SYS_CALL_ResumeOSInterrupts();
        /*lint -e904 MISRA:OTHER:Validation of parameters, if failure, function will return.This is not inline with Table 8, ISO26262-6-2011, Req 1a:[MISRA 2012 Rule 15.5, advisory]*/
        return QUEUE_E_NULL;
    }

    if (queue->isInit != TRUE) {
            SYS_CALL_ResumeOSInterrupts();
            return QUEUE_E_NO_INIT; /* Faulty pointer into method */
        }

    if (queue->count == 0) {
        SYS_CALL_ResumeOSInterrupts();
        return QUEUE_E_NO_DATA;
    }

    MEMCPY((void*) dataPtr, queue->tail, queue->dataSize);
    /*lint -e970 MISRA:OTHER:argument check:[MISRA 2012 Directive 4.6, advisory]*/
    queue->tail = (char *) queue->tail + queue->dataSize; /*lint !e9016 MISRA:OTHER:correct arithmetic even if Array index is not used:[MISRA 2012 Rule 18.4, advisory]*/
    if (queue->tail == queue->bufEnd) {
        queue->tail = queue->bufStart;
    }
    --queue->count;

    if (queue->bufFullFlag == TRUE) {
        queue->bufFullFlag = FALSE;
        SYS_CALL_ResumeOSInterrupts();
        return QUEUE_E_LOST_DATA;
    }

    SYS_CALL_ResumeOSInterrupts();
    return QUEUE_E_OK;
}
/* @req ARC_SWS_Queue_00006 */
Queue_ReturnType Queue_Peek(Queue_t const *queue, void *dataPtr) {
    SYS_CALL_SuspendOSInterrupts();

    if ((queue == NULL_PTR) || (dataPtr == NULL_PTR)) {
        SYS_CALL_ResumeOSInterrupts();
        /*lint -e904 MISRA:OTHER:Validation of parameters, if failure, function will return.This is not inline with Table 8, ISO26262-6-2011, Req 1a:[MISRA 2012 Rule 15.5, advisory]*/
        return QUEUE_E_NULL;
    }
    if (queue->isInit != TRUE) {
        SYS_CALL_ResumeOSInterrupts();
        return QUEUE_E_NO_INIT;
    }
    if (queue->count == 0) {
        SYS_CALL_ResumeOSInterrupts();
        return QUEUE_E_NO_DATA;
    }

    MEMCPY((void*) dataPtr, queue->tail, queue->dataSize);

    SYS_CALL_ResumeOSInterrupts();
    return QUEUE_E_OK;
}
/* @req ARC_SWS_Queue_00007 */
Queue_ReturnType Queue_Contains(Queue_t const *queue, void const *dataPtr) {
    uint8 i;

    SYS_CALL_SuspendOSInterrupts();

    if ((queue == NULL_PTR) || (dataPtr == NULL_PTR)) {
        SYS_CALL_ResumeOSInterrupts();
        /*lint -e904 MISRA:OTHER:Validation of parameters, if failure, function will return.This is not inline with Table 8, ISO26262-6-2011, Req 1a:[MISRA 2012 Rule 15.5, advisory]*/
        return QUEUE_E_NULL;
    }

    if (queue->isInit != (boolean)TRUE) {
        SYS_CALL_ResumeOSInterrupts();
        return QUEUE_E_NO_INIT;
    }
    if (queue->count == 0) {
        SYS_CALL_ResumeOSInterrupts();
        return QUEUE_E_NO_DATA;
    }

    /*lint -e970 MISRA:OTHER:argument check:[MISRA 2012 Directive 4.6, advisory]*/
    char *iter = queue->tail;
    //Loop through queue
    for (i = 0; i < queue->count; i++) {
        if (queue->compare_func(iter, (void*) dataPtr, queue->dataSize) == 0) { /*lint !e9005 MISRA:PERFORMANCE:casting away const is okay:[MISRA 2012 Rule 11.8, required]*/
            SYS_CALL_ResumeOSInterrupts();
            return QUEUE_E_TRUE;
        }
        iter = iter + queue->dataSize; /*lint !e9016 MISRA:OTHER:correct arithmetic even if Array index is not used:[MISRA 2012 Rule 18.4, advisory]*/
    }

    SYS_CALL_ResumeOSInterrupts();
    return QUEUE_E_FALSE;
}
