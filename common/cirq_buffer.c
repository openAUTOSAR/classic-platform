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
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "Cpu.h"
#include "cirq_buffer.h"

#ifdef _TEST_CIRQ_BUFFER_DYN_
#include <assert.h>
#endif

#if defined(__GNUC__)
#define MEMCPY(_x,_y,_z)	__builtin_memcpy(_x,_y,_z)
#else
#define MEMCPY(_x,_y,_z)	memcpy(_x,_y,_z)
#endif
//#define MEMCPY(_x,_y,_z)	memcpy(_x,_y,_z)


/* TODO: Not threadsafe, add DisableAllInterrts()/EnableAllInterrupts() */

void CirqBuff_Init(CirqBufferType *cirqbuffer, void *buffer, int maxCnt, size_t dataSize) {
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
CirqBufferType CirqBuffStatCreate(void *buffer, int maxCnt, size_t dataSize) {
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

CirqBufferType *CirqBuffDynCreate( size_t size, size_t dataSize ) {
	CirqBufferType *cPtr;
	/*lint -save -e586 */
	cPtr = malloc(sizeof(CirqBufferType));
	/*lint -restore */
	if( cPtr == NULL ) {
		return NULL;
	}
	cPtr->maxCnt = size;
	cPtr->dataSize = dataSize;
	/*lint -save -e586 */
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

int CirqBuffPush( CirqBufferType *cPtr, void *dataPtr ) {
	uint32_t flags;
	Irq_Save(flags);
	if( (cPtr->currCnt == cPtr->maxCnt) || (cPtr==NULL) ) {
		Irq_Restore(flags);
		return 1;	/* No more room */
	}
	MEMCPY(cPtr->head,dataPtr,cPtr->dataSize);
	cPtr->head = (char *)cPtr->head + cPtr->dataSize;
	if( cPtr->head == cPtr->bufEnd) {
		cPtr->head = cPtr->bufStart;
	}
	++cPtr->currCnt;
	Irq_Restore(flags);

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
	uint32_t flags;
	Irq_Save(flags);
	if( (cPtr->currCnt == 0) || (cPtr==NULL) ) {
		Irq_Restore(flags);
		return 1;
	}
	MEMCPY(dataPtr,cPtr->tail,cPtr->dataSize);
	cPtr->tail = (char *)cPtr->tail + cPtr->dataSize;
	if( cPtr->tail == cPtr->bufEnd) {
		cPtr->tail = cPtr->bufStart;
	}
	--cPtr->currCnt;
	Irq_Restore(flags);
	return 0;
}

void *CirqBuff_PushLock( CirqBufferType *cPtr) {
	void *dataPtr;
	if( (cPtr->currCnt == cPtr->maxCnt) || (cPtr==NULL) ) {
		return NULL;	/* No more room */
	}
	dataPtr = cPtr->head;
	cPtr->head = (char *)cPtr->head + cPtr->dataSize;
	if( cPtr->head == cPtr->bufEnd) {
		cPtr->head = cPtr->bufStart;
	}
	return dataPtr;
}


void * CirqBuff_PopLock(CirqBufferType *cPtr ) {
	void *dataPtr;
	if( (cPtr->currCnt == 0) || (cPtr==NULL) ) {
		return NULL;
	}
	dataPtr = cPtr->tail;
	cPtr->tail = (char *)cPtr->tail + cPtr->dataSize;
	if( cPtr->tail == cPtr->bufEnd) {
		cPtr->tail = cPtr->bufStart;
	}
	--cPtr->currCnt;
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
	assert(rv == 0);
	free(dataPtr);

	dataPtr = malloc(DATA_SIZE);
	dataPtr[0] = 2;
	rv  = CirqBuffPush(cPtr,dataPtr);
	assert(rv == 0);
	free(dataPtr);

	dataPtr = malloc(DATA_SIZE);
	rv = CirqBuffPop(cPtr,dataPtr);
	assert( dataPtr[0] == 1);
	assert(rv == 0);
	free(dataPtr);

	dataPtr = malloc(DATA_SIZE);
	rv = CirqBuffPop(cPtr,dataPtr);
	assert( dataPtr[0] == 2);
	assert(rv == 0);
	free(dataPtr);

	CirqBuffDynDestroy(cPtr);
}
#endif


