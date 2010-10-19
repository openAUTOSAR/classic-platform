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
