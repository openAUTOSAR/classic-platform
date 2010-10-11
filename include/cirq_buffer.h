/* -------------------------------- Arctic Core ------------------------------
 * Arctic Core - the open source AUTOSAR platform http://arccore.com
 *
 * Copyright (C) 2009  ArcCore AB <contact@arccore.com>
 *
 * This source code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by the
 * Free Software Foundation; See <http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 * -------------------------------- Arctic Core ------------------------------*/

#ifndef CIRQ_BUFFER_H_
#define CIRQ_BUFFER_H_

#include <stddef.h>

typedef struct {
	/* The max number of elements in the list */
	int maxCnt;
	int currCnt;

	/* Size of the elements in the list */
	size_t dataSize;
	/* List head and tail */
	void *head;
	void *tail;

	/* Buffer start/stop */
	void *bufStart;
	void *bufEnd;
} CirqBufferDynType;

CirqBufferDynType *CirqBuffDynCreate( size_t size, size_t dataSize );
int CirqBuffDynDestroy(CirqBufferDynType *cPtr );
int CirqBuffDynPush( CirqBufferDynType *cPtr, void *dataPtr );
int CirqBuffDynPop(CirqBufferDynType *cPtr, void *dataPtr );

/* TODO: Static implementation */




#endif /* CIRQ_BUFFER_H_ */
