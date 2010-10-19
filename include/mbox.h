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

#ifndef MBOX_H_
#define MBOX_H_

#include "cirq_buffer.h"

typedef struct
{
	CirqBufferType *cirqPtr;
} Arc_MBoxType;


typedef enum {
	SOME_ERROR,
} Arc_MBoxErrType;

Arc_MBoxType* Arc_MBoxCreate( size_t size );
void Arc_MBoxDestroy( Arc_MBoxType *mPtr );
int Arc_MBoxPost( Arc_MBoxType *mPtr, void *msg );
int Arc_MBoxFetch(Arc_MBoxType *mPtr, void *msg);

#endif /* MBOX_H_ */
