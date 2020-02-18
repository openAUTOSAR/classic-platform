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
#include "Safety_Queue.h"

/* @req ARC_SWS_SafeQueue_00010 The queue shall use FIFO order*/
/* @req ARC_SWS_SafeQueue_00011 The queue shall handle any type specified by user*/
/* @req ARC_SWS_SafeQueue_00013 The files shall be named Safety_Queue.c and Safety_Queue.h*/
/* @req ARC_SWS_SafeQueue_00014 The safety queue shall not use native C types*/
/* @req ARC_SWS_SafeQueue_00015 The safety queue shall be ready to be invoked at any moment*/
/* @req ARC_SWS_SafeQueue_00016 The safety queue shall not call any BSW modules functions*/
/*
 *  A circular buffer implementation of FIFO queue.*
 */

/* @req ARC_SWS_SafeQueue_00004*/
/*lint -e{9016} MISRA:OTHER:correct arithmetic even if Array index is not used:[MISRA 2012 Rule 18.45, advisory]*/
Queue_ReturnType Safety_Queue_Init(Safety_Queue_t *queue, void *buffer,
        uint8 max_count, size_t dataSize, cmpFunc compare_func) {
    SYS_CALL_SuspendOSInterrupts();

    if ((queue == NULL_PTR) || (buffer == NULL_PTR) || (compare_func == NULL_PTR)) {
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
    queue->bufEnd = (char *) buffer + (max_count * dataSize); /*lint !e960 MISRA:OTHER:correct arithmetic even if Array index is not used:[MISRA 2004 Info, advisory]*/
    queue->head = queue->bufStart;
    queue->tail = queue->bufStart;
    queue->dataSize = dataSize;
    queue->count = 0u;
    queue->max_count = max_count;
    queue->compare_func = *compare_func;
    queue->bufferCrc = Crc_CalculateCRC8(queue->bufStart,
            queue->dataSize * queue->max_count, 0u, 1u);
    queue->isInit = TRUE;

    /*lint -e928 MISRA:PERFORMANCE:Need pointer arithmetic to determine size of struct - crc value:[MISRA 2012 Rule 11.5, advisory]*/
    /*lint -e926 MISRA:PERFORMANCE:Need pointer arithmetic to determine size of struct - crc value:[MISRA 2012 Rule 11.5, advisory]*/
    /*lint -e946 MISRA:PERFORMANCE:Need pointer arithmetic to determine size of struct - crc value:[MISRA 2012 Rule 18.3, required]*/
    /*lint -e960 MISRA:PERFORMANCE:Need pointer arithmetic to determine size of struct - crc value:[MISRA 2004 Info, advisory]*/
    /*lint -e947 MISRA:PERFORMANCE:Need pointer arithmetic to determine size of struct - crc value:[MISRA 2012 Rule 18.3, required]*/
    /*lint -e732 MISRA:PERFORMANCE:Need pointer arithmetic to determine size of struct - crc value:[MISRA 2004 Info, advisory]*/
    size_t without_buffer_size = (char*) &(queue->queueCrc) - (char*) queue;
    queue->queueCrc = Crc_CalculateCRC8((void*) queue, without_buffer_size, 0u,
            0u);
    SYS_CALL_ResumeOSInterrupts();
    return QUEUE_E_OK;
}
/* @req ARC_SWS_SafeQueue_00005*/
Queue_ReturnType Safety_Queue_Add(Safety_Queue_t *queue, void const *dataPtr) {
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

    /* @req ARC_SWS_SafeQueue_00002*/
    uint8 currBufferCrc = Crc_CalculateCRC8(queue->bufStart,
            queue->dataSize * queue->max_count, 0u, 1u);

    size_t without_buffer_size = (char*) &(queue->queueCrc) - (char*) queue; /*lint !e970 MISRA:OTHER:Pointer arithmetic:[MISRA 2012 Directive 4.6, advisory]*/
    uint8 currQueueCrc = Crc_CalculateCRC8((void*) queue, without_buffer_size,
            0u, 0u);
    /* @req ARC_SWS_SafeQueue_00001*/
    if ((queue->bufferCrc != currBufferCrc)
            || (queue->queueCrc != currQueueCrc)) {
        SYS_CALL_ResumeOSInterrupts();
        /* @req ARC_SWS_SafeQueue_00003*/
        /*lint -e904 MISRA:OTHER:Validation of parameters, if failure, function will return.This is not inline with Table 8, ISO26262-6-2011, Req 1a:[MISRA 2012 Rule 15.5, advisory]*/
        return QUEUE_E_CRC_ERR;
    }
    //Queue is full
    if (queue->count == queue->max_count) {
        queue->bufFullFlag = TRUE;

        //Update CRC values
        queue->queueCrc = Crc_CalculateCRC8((void*) queue, without_buffer_size, 0u,
                1u);

        SYS_CALL_ResumeOSInterrupts();
        return QUEUE_E_FULL;
    }
    MEMCPY(queue->head, dataPtr, queue->dataSize);
    /*lint -e{970} MISRA:OTHER:correct arithmetic even if Array index is not used:[MISRA 2012 Directive 4.6, advisory]*/
    queue->head = (char *) queue->head + queue->dataSize; /*lint !e9016 MISRA:OTHER:correct arithmetic even if Array index is not used:[MISRA 2012 Rule 18.4, advisory]*/

    //Wrap-around
    if (queue->head == queue->bufEnd) {
        queue->head = queue->bufStart;
    }
    queue->count++;

    //Update CRC values
    queue->bufferCrc = Crc_CalculateCRC8(queue->bufStart,
            queue->dataSize * queue->max_count, 0u, 1u);
    queue->queueCrc = Crc_CalculateCRC8((void*) queue, without_buffer_size, 0u,
            1u);

    SYS_CALL_ResumeOSInterrupts();
    return QUEUE_E_OK;
}
/* @req ARC_SWS_SafeQueue_00006*/
Queue_ReturnType Safety_Queue_Next(Safety_Queue_t *queue, void *dataPtr) {
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

    if (queue->count == 0u) {
        SYS_CALL_ResumeOSInterrupts();
        return QUEUE_E_NO_DATA;
    }

    //CRC check
    uint8 currBufferCrc = Crc_CalculateCRC8(queue->bufStart,
            queue->dataSize * queue->max_count, 0u, 1u);

    /*lint -e970 MISRA:OTHER:Need pointer arithmetic to determine size of struct - crc value:[MISRA 2012 Directive 4.6, advisory]*/
    /*lint -e928 MISRA:PERFORMANCE:Need pointer arithmetic to determine size of struct - crc value:[MISRA 2012 Rule 11.5, advisory]*/
    /*lint -e926 MISRA:PERFORMANCE:Need pointer arithmetic to determine size of struct - crc value:[MISRA 2012 Rule 11.5, advisory]*/
    /*lint -e946 MISRA:PERFORMANCE:Need pointer arithmetic to determine size of struct - crc value:[MISRA 2012 Rule 18.3, required]*/
    /*lint -e960 MISRA:PERFORMANCE:Need pointer arithmetic to determine size of struct - crc value:[MISRA 2004 Info, advisory]*/
    /*lint -e947 MISRA:PERFORMANCE:Need pointer arithmetic to determine size of struct - crc value:[MISRA 2012 Rule 18.3, required]*/
    /*lint -e732 MISRA:PERFORMANCE:Need pointer arithmetic to determine size of struct - crc value:[MISRA 2004 Info, advisory]*/
    size_t without_buffer_size = (char*) &(queue->queueCrc) - (char*) queue;
    uint8 currQueueCrc = Crc_CalculateCRC8((void*) queue, without_buffer_size,
            0u, 0u);
    if ((queue->bufferCrc != currBufferCrc)
            || (queue->queueCrc != currQueueCrc)) {
        SYS_CALL_ResumeOSInterrupts();
        return QUEUE_E_CRC_ERR;
    }
    MEMCPY((void*) dataPtr, queue->tail, queue->dataSize); /*lint !e9005 MISRA:PERFORMANCE:casting away const is okay:[MISRA 2012 Rule 11.8, required]*/
    /*lint -e970 MISRA:OTHER:Need pointer arithmetic to determine size of struct - crc value:[MISRA 2012 Directive 4.6, advisory]*/
    /*lint -e{9016} MISRA:OTHER:correct arithmetic even if Array index is not used:[MISRA 2004 Info,advisory]*/
    queue->tail = (char *) queue->tail + queue->dataSize; /*lint !e960 MISRA:OTHER:correct arithmetic even if Array index is not used:[MISRA 2004 Info,advisory]*/
    if (queue->tail == queue->bufEnd) {
        queue->tail = queue->bufStart;
    }
    --queue->count;

    if (queue->bufFullFlag == TRUE) {
        queue->bufFullFlag = FALSE;

        //Update CRC values
        queue->queueCrc = Crc_CalculateCRC8((void*) queue, without_buffer_size, 0u,
                1u);

        SYS_CALL_ResumeOSInterrupts();
        return QUEUE_E_LOST_DATA;
    }

    queue->bufferCrc = Crc_CalculateCRC8(queue->bufStart,
            queue->dataSize * queue->max_count, 0u, 1u);
    queue->queueCrc = Crc_CalculateCRC8((void*) queue, without_buffer_size, 0u,
            1u);
    SYS_CALL_ResumeOSInterrupts();
    return QUEUE_E_OK;
}
/* @req ARC_SWS_SafeQueue_00007*/
Queue_ReturnType Safety_Queue_Peek(Safety_Queue_t const *queue, void *dataPtr) {
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

    //CRC check
    uint8 currBufferCrc = Crc_CalculateCRC8(queue->bufStart,
            queue->dataSize * queue->max_count, 0u, 1u);

    /*lint -e9005 MISRA:PERFORMANCE:Need pointer arithmetic to determine size of struct - crc value:[MISRA 2004 Info, advisory]*/
    size_t without_buffer_size = (char*) &(queue->queueCrc) - (char*) queue;
    uint8 currQueueCrc = Crc_CalculateCRC8((void*) queue, without_buffer_size,
            0u, 0u);
    if ((queue->bufferCrc != currBufferCrc)
            || (queue->queueCrc != currQueueCrc)) {
        SYS_CALL_ResumeOSInterrupts();
        return QUEUE_E_CRC_ERR;
    }
    if (queue->count == 0u) {
        SYS_CALL_ResumeOSInterrupts();
        return QUEUE_E_NO_DATA;
    }

    MEMCPY((void*) dataPtr, queue->tail, queue->dataSize); /*lint !e9005 MISRA:PERFORMANCE:casting away const is okay:[MISRA 2012 Rule 11.8, required]*/

    SYS_CALL_ResumeOSInterrupts();
    return QUEUE_E_OK;
}
/* @req ARC_SWS_SafeQueue_00008*/
Queue_ReturnType Safety_Queue_Contains(Safety_Queue_t const *queue,
        void const *dataPtr) {
    uint32 i;
    /*lint !e970 LINT:OTHER:pointer declaration:[MISRA 2012 Directive 4.6, advisory]*/
    char *iter;
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

    //CRC check
    uint8 currBufferCrc = Crc_CalculateCRC8(queue->bufStart,
            queue->dataSize * queue->max_count, 0u, 1u);
    /*lint -e9005 MISRA:PERFORMANCE:Need pointer arithmetic to determine size of struct - crc value:[MISRA 2004 Info, advisory]*/
    size_t without_buffer_size = (char*) &(queue->queueCrc) - (char*) queue;
    uint8 currQueueCrc = Crc_CalculateCRC8((void*) queue, without_buffer_size,
            0u, 0u); /*lint !e9005 MISRA:PERFORMANCE:casting away const is okay:[MISRA 2012 Rule 11.8, required]*/
    if ((queue->bufferCrc != currBufferCrc)
            || (queue->queueCrc != currQueueCrc)) {
        SYS_CALL_ResumeOSInterrupts();
        return QUEUE_E_CRC_ERR;
    }
    if (queue->count == 0u) {
        SYS_CALL_ResumeOSInterrupts();
        return QUEUE_E_NO_DATA;
    }

    iter = queue->tail;
    //Loop through queue
    for (i = 0u; i < queue->count; i++) {
        /*lint !e970 MISRA:OTHER:Need pointer arithmetic to determine size of struct - crc value:[MISRA 2012 Directive 4.6, advisory]*/
        /*lint !e904 MISRA:OTHER:Validation of parameters, if failure, function will return.This is not inline with Table 8, ISO26262-6-2011, Req 1a:[MISRA 2012 Rule 15.5, advisory]*/
        if (queue->compare_func(iter, (void*) dataPtr, queue->dataSize) == 0) { /*lint !e9005 MISRA:PERFORMANCE:casting away const is okay:[MISRA 2012 Rule 11.8, required]*/
            SYS_CALL_ResumeOSInterrupts();
            return QUEUE_E_TRUE;
        }
        iter = iter + queue->dataSize; /*lint !e9016 MISRA:OTHER:correct arithmetic even if Array index is not used:[MISRA 2012 Rule 18.4, advisory]*/

    }
    SYS_CALL_ResumeOSInterrupts();
    return QUEUE_E_FALSE;
}
