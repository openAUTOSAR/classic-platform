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

#ifndef XCP_BYTESTREAM_H_
#define XCP_BYTESTREAM_H_

#include "Xcp_Internal.h"
#include "SchM_Xcp.h"

#define XCP_UNUSED(x) (void)(x)

#define GET_UINT8(data, offset)  (*((uint8* )(data)+(offset)))
#define GET_UINT16(data, offset) (*(uint16*)((uint8*)(data)+(offset)))
#define GET_UINT32(data, offset) (*(uint32*)((uint8*)(data)+(offset)))

#define SET_UINT8(data, offset, value) do {           \
         (*(uint8* )((uint8*)(data)+(offset))) = (value); \
        } while(0)

#define SET_UINT16(data, offset, value) do {            \
         (*(uint16*)((uint8*)(data)+(offset))) = (value); \
        } while(0)

#define SET_UINT32(data, offset, value) do {            \
         (*(uint32*)((uint8*)(data)+(offset))) = (value); \
        } while(0)

#define FIFO_GET_WRITE(fifo, it) \
    for(Xcp_BufferType* it = Xcp_Fifo_Get(fifo.free); it; Xcp_Fifo_Put(&fifo, it), it = NULL)

#define FIFO_FOR_READ(fifo, it) \
    for(Xcp_BufferType* it = Xcp_Fifo_Get(&fifo); it; Xcp_Fifo_Free(&fifo, it), it = Xcp_Fifo_Get(&fifo))

#define FIFO_ADD_U8(fifo, value) \
    do { SET_UINT8(fifo->data, fifo->len, value); fifo->len+=1; } while(0)

#define FIFO_ADD_U16(fifo, value) \
    do { SET_UINT16(fifo->data, fifo->len, value); fifo->len+=2; } while(0)

#define FIFO_ADD_U32(fifo, value) \
    do { SET_UINT32(fifo->data, fifo->len, value); fifo->len+=4; } while(0)

/* RX/TX FIFO */

typedef struct Xcp_BufferType {
	uint32                 len;
    uint8                  data[XCP_MAX_DTO];
    struct Xcp_BufferType* next;
} Xcp_BufferType;

typedef struct Xcp_FifoType {
    Xcp_BufferType*        front;
    Xcp_BufferType*        back;
    struct Xcp_FifoType*   free;
    void*                  lock;
} Xcp_FifoType;

/* This function is used to enter exclusive area to avoid data correction.*/
static inline void Xcp_Fifo_Lock(void)
{
	SchM_Enter_Xcp(EXCLUSIVE_AREA_0);
}

/* This function is used to exit from exclusive area.*/
static inline void Xcp_Fifo_Unlock(void)
{
	SchM_Exit_Xcp(EXCLUSIVE_AREA_0);
}

/* This function retrieves the fist element from the FIFO q.*/
static inline Xcp_BufferType* Xcp_Fifo_Get(Xcp_FifoType* q)
{
    Xcp_Fifo_Lock();
    Xcp_BufferType* b = q->front;
    if(b == NULL) {

    	/*FIFO is empty*/
        Xcp_Fifo_Unlock();
        return NULL;
    }

    /*retrieve b from the FIFO*/
    q->front = b->next;
    if(q->front == NULL)
        q->back = NULL;
    b->next = NULL;
    Xcp_Fifo_Unlock();
    return b;
}

/* This function add the new element b to the FIFO q.*/
static inline void Xcp_Fifo_Put(Xcp_FifoType* q, Xcp_BufferType* b)
{
    Xcp_Fifo_Lock();
    b->next = NULL;
    if(q->back)
        q->back->next = b; /*add pointer to b from the previous element*/
    else
        q->front = b; /*if there is no previous element add as first element*/
    q->back = b;
    Xcp_Fifo_Unlock();
}

/* This function release one element b by adding it to q->free FIFO.*/
static inline void Xcp_Fifo_Free(Xcp_FifoType* q, Xcp_BufferType* b)
{
    if(b) {
        b->len = 0;
        Xcp_Fifo_Put(q->free, b);
    }
}

/* This function initialize a FIFO q by the content defined by b and e pointer as start
 * and end pointer respectively.*/
static inline void Xcp_Fifo_Init(Xcp_FifoType* q, Xcp_BufferType* b, Xcp_BufferType* e)
{
    q->front = NULL;
    q->back  = NULL;
    q->lock  = NULL;
    for(;b != e; b++)
        Xcp_Fifo_Put(q, b);
}

#endif /* XCP_BYTESTREAM_H_ */
