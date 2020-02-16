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
