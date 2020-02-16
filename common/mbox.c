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
 * A message box implementation.
 *
 * Notes:
 * - Uses heap
 * - Do NOT use Arc_MBoxDestroy() call, it's there for testing purposes only.
 */

//#define _TEST_MBOX_

#include "mbox.h"
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>


#ifdef _TEST_MBOX_
#include <stdio.h>
#include <assert.h>
#endif



Arc_MBoxType* Arc_MBoxCreate( size_t size ) {
	Arc_MBoxType *mPtr;

	mPtr = malloc(sizeof(Arc_MBoxType));
	mPtr->cirqPtr = CirqBuffDynCreate(size,sizeof(void *));

	return mPtr;
}


void Arc_MBoxDestroy( Arc_MBoxType *mPtr ) {
	CirqBuffDynDestroy(mPtr->cirqPtr);
	free(mPtr);
}

/**
 * Post a message to a box, non-blocking.
 *
 */
int Arc_MBoxPost( Arc_MBoxType *mPtr, void *msg ) {
	int rv;
	rv = CirqBuffPush(mPtr->cirqPtr,msg);
	if( rv != 0) {
		return 1;
	}

	return 0;
}

/**
 *
 */
int Arc_MBoxFetch(Arc_MBoxType *mPtr, void *msg)
{
	int rv;
	rv = CirqBuffPop(mPtr->cirqPtr,msg);
	if(rv != 0) {
		return 1;
	}
	return 0;
}

#ifdef _TEST_MBOX_
int main( void ) {
	Arc_MBoxType *myBoxes[10];
	uint8_t *dataPtr;

	printf("Hej\n");

	myBoxes[0] = Arc_MBoxCreate(2);
	myBoxes[1] = Arc_MBoxCreate(4);

	dataPtr = malloc(10);
	dataPtr[0] = 1;
	Arc_MBoxPost(myBoxes[0],&dataPtr);

	dataPtr = malloc(20);
	dataPtr[0] = 2;
	Arc_MBoxPost(myBoxes[0],&dataPtr);

	Arc_MBoxFetch(myBoxes[0],&dataPtr);
    assert(dataPtr[0] == 1);
	free(dataPtr);

	Arc_MBoxFetch(myBoxes[0],&dataPtr);
	assert(dataPtr[0] == 2);
	free(dataPtr);

	Arc_MBoxDestroy(myBoxes[0]);
	Arc_MBoxDestroy(myBoxes[1]);
	return 0;
}

#endif
