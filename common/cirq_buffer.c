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
/*
 * DESCRIPTION
 *  A circular buffer implementation.
 *
 *  This file implements the following versions:
 *  1. CirqBuffDynXXXX
 *     - The size of the data is not known of compile time
 *       (semidynamic, since there is no free() function )
 *  2. CirqBuffXXX
 *     - A static implementation, data is known at compile time.
 *       (implemented in the header file?)
 *
 * Implementation note:
 * - CirqBuffDynXXXX, valgrind: OK
 * - coverage: Not run
 *
 */
//#define _TEST_CIRQ_BUFFER_DYN_
/*lint -w1 */
/*lint -w2  -e526 -e628 */
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "Cpu.h"
#include "cirq_buffer.h"
#include "Os.h"

#ifdef _TEST_CIRQ_BUFFER_DYN_
#include "arc_assert.h"
#endif

#if defined(__GNUC__)
#ifndef MEMCPY
#define MEMCPY(_x,_y,_z)	__builtin_memcpy(_x,_y,_z)
#endif /* MEMCPY */
#else
#ifndef MEMCPY
#define MEMCPY(_x,_y,_z)	memcpy(_x,_y,_z)
#endif /* MEMCPY */
#endif

#if !defined(OS_SC3)
#define OS_SC3 STD_OFF
#endif

#if !defined(OS_SC4)
#define OS_SC4 STD_OFF
#endif

#if ((OS_SC3 == STD_ON) || (OS_SC4 == STD_ON))
#define SchM_Enter_CirqBuff_EA_0()  (void)SYS_CALL_SuspendOSInterrupts()
#define SchM_Exit_CirqBuff_EA_0()   (void)SYS_CALL_ResumeOSInterrupts()
#else
#define SchM_Enter_CirqBuff_EA_0()  SuspendOSInterrupts()
#define SchM_Exit_CirqBuff_EA_0()   ResumeOSInterrupts()
#endif


/* IMPROVMENT: Make it threadsafe, add DisableAllInterrts()/EnableAllInterrupts() */
void CirqBuff_Init(CirqBufferType *cirqbuffer, void *buffer, uint32 maxCnt, uint32 dataSize) {
    cirqbuffer->bufStart = buffer;
    cirqbuffer->maxCnt = maxCnt;
    cirqbuffer->bufEnd = (char *)cirqbuffer->bufStart + dataSize*maxCnt;
    cirqbuffer->head = cirqbuffer->bufStart;
    cirqbuffer->tail = cirqbuffer->bufStart;
    cirqbuffer->dataSize = dataSize;
    cirqbuffer->currCnt = 0;
}

/**
 *
 * @param buffer    pointer to a buffer
 * @param maxCnt    Maximum number of elements of size dataSize
 * @param dataSize  The size of the data.
 * @return
 */
CirqBufferType CirqBuffStatCreate(void *buffer, uint32 maxCnt, uint32 dataSize) {
    CirqBufferType cirqbuffer;
    cirqbuffer.bufStart = buffer;
    cirqbuffer.maxCnt = maxCnt;
    cirqbuffer.bufEnd = (char *)cirqbuffer.bufStart + dataSize*maxCnt;
    cirqbuffer.head = cirqbuffer.bufStart;
    cirqbuffer.tail = cirqbuffer.bufStart;
    cirqbuffer.dataSize = dataSize;
    cirqbuffer.currCnt = 0;
    /*return whole object */
    return cirqbuffer;
}

CirqBufferType *CirqBuffDynCreate( uint32 size, uint32 dataSize ) {
    CirqBufferType *cPtr;
    /*lint -save -e586 */
    cPtr = malloc(sizeof(CirqBufferType));
    /*lint -restore */
    if( cPtr == NULL ) {
        return NULL;
    }
    cPtr->maxCnt = size;
    cPtr->dataSize = dataSize;
    /*lint -save -e586 -e632 STANDARDIZED INTERFACE */
    cPtr->bufStart = malloc(dataSize*size);
    /*lint -restore */
    cPtr->bufEnd = (char *)cPtr->bufStart + dataSize*size;
    cPtr->head = cPtr->bufStart;
    cPtr->tail = cPtr->bufStart;
    cPtr->currCnt = 0;
    return cPtr;
}



int CirqBuffDynDestroy(CirqBufferType *cPtr ) {
    /*lint -save -e586 */
    free(cPtr->bufStart);
    free(cPtr);
    /*lint -restore */
    return 0;
}

/**
 * Push an entry to the buffer.
 *
 * @param cPtr    Pointer to the queue created with CirqBuffStatCreate, etc.
 * @param dataPtr
 * @return 0 - if successfully pushed.
 *         1- nothing pushed (it was full)
 */
int CirqBuffPush( CirqBufferType *cPtr, void *dataPtr ) {
    SchM_Enter_CirqBuff_EA_0();

    if( (cPtr==NULL) || (cPtr->currCnt == cPtr->maxCnt)) {
        SchM_Exit_CirqBuff_EA_0();
        return 1;	/* No more room */
    }
    /*lint -save -e632 STANDARDIZED INTERFACE */
    MEMCPY(cPtr->head,dataPtr,cPtr->dataSize);
    /*lint -restore */
    cPtr->head = (char *)cPtr->head + cPtr->dataSize;
    if( cPtr->head == cPtr->bufEnd) {
        cPtr->head = cPtr->bufStart;
    }
    ++cPtr->currCnt;
    SchM_Exit_CirqBuff_EA_0();

    return 0;
}



/**
 * Pop an entry from the buffer.
 *
 * @param cPtr    Pointer to the queue created with CirqBuffStatCreate, etc.
 * @param dataPtr
 * @return 0 - if successfully popped.
 *         1- nothing popped (it was empty)
 */
int CirqBuffPop(CirqBufferType *cPtr, void *dataPtr ) {
    SchM_Enter_CirqBuff_EA_0();
    if((cPtr==NULL) || (cPtr->currCnt == 0) ) {
        SchM_Exit_CirqBuff_EA_0();
        return 1;
    }
    /*lint -save -e632 STANDARDIZED INTERFACE */
    MEMCPY(dataPtr,cPtr->tail,cPtr->dataSize);
    /*lint -restore */
    cPtr->tail = (char *)cPtr->tail + cPtr->dataSize;
    if( cPtr->tail == cPtr->bufEnd) {
        cPtr->tail = cPtr->bufStart;
    }
    --cPtr->currCnt;
    SchM_Exit_CirqBuff_EA_0();
    return 0;
}


/**
 * PushLock an entry to the buffer.
 * NOTE: This must be used in combination with CirqBuff_PushRelease when the
 * application has filled the buffer with data. Lock and release must be done
 * in consecutive order i.e lock/release/lock/release. lock/lock/release/release is not
 * allowed.
 *
 * @param cPtr    Pointer to the queue created with CirqBuffStatCreate, etc.
 * @return ptr to buffer - if successfully pushed.
 *         NULL - nothing pushed (it was full)
 */
void *CirqBuff_PushLock( CirqBufferType *cPtr) {
    void *dataPtr;
    if((cPtr==NULL) || (cPtr->currCnt == cPtr->maxCnt)) {
        return NULL;	/* No more room */
    }
    dataPtr = cPtr->head;
    cPtr->head = (char *)cPtr->head + cPtr->dataSize;
    if( cPtr->head == cPtr->bufEnd) {
        cPtr->head = cPtr->bufStart;
    }
    return dataPtr;
}

/**
 * PopLock an entry from the buffer.
 * NOTE: This must be used in combination with CirqBuff_PopRelease when the
 * application has used or copied the buffer data. Lock and release must be done
 * in consecutive order i.e lock/release/lock/release. lock/lock/release/release is not
 * allowed.
 *
 * @param cPtr    Pointer to the queue created with CirqBuffStatCreate, etc.
 * @return ptr to buffer - if successfully pushed.
 *         NULL - nothing pushed (it was full)
 */
void * CirqBuff_PopLock(CirqBufferType *cPtr ) {
    void *dataPtr;
    if((cPtr==NULL) || (cPtr->currCnt == 0) ) {
        return NULL;
    }
    dataPtr = cPtr->tail;
    cPtr->tail = (char *)cPtr->tail + cPtr->dataSize;
    if( cPtr->tail == cPtr->bufEnd) {
        cPtr->tail = cPtr->bufStart;
    }
    return dataPtr;
}

void *CirqBuff_Peek( CirqBufferType *cPtr, uint32 offset ){
    void *dataPtr;
    if( (cPtr==NULL) || (cPtr->currCnt == 0) || (offset > cPtr->currCnt) ) {
        return NULL;
    }
    dataPtr = cPtr->tail;
    for(uint32 i=0;i<offset;i++){
        dataPtr = (char *)dataPtr + cPtr->dataSize;
        if( dataPtr == cPtr->bufEnd) {
            dataPtr = cPtr->bufStart;
        }
    }
    return dataPtr;
}

#ifdef _TEST_CIRQ_BUFFER_DYN_
int main( void ) {
    CirqBufferType *cPtr;
    uint8_t *dataPtr;
    int rv;

#define DATA_SIZE 	4
#define QSIZE		8
    cPtr = CirqBuffDynCreate(QSIZE, DATA_SIZE );

    dataPtr = malloc(DATA_SIZE);
    dataPtr[0] = 1;
    rv  = CirqBuffPush(cPtr,dataPtr);
    ASSERT(rv == 0);
    free(dataPtr);

    dataPtr = malloc(DATA_SIZE);
    dataPtr[0] = 2;
    rv  = CirqBuffPush(cPtr,dataPtr);
    ASSERT(rv == 0);
    free(dataPtr);

    dataPtr = malloc(DATA_SIZE);
    rv = CirqBuffPop(cPtr,dataPtr);
    ASSERT( dataPtr[0] == 1);
    ASSERT(rv == 0);
    free(dataPtr);

    dataPtr = malloc(DATA_SIZE);
    rv = CirqBuffPop(cPtr,dataPtr);
    ASSERT( dataPtr[0] == 2);
    ASSERT(rv == 0);
    free(dataPtr);

    CirqBuffDynDestroy(cPtr);
}
#endif


