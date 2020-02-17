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
/*lint -save -e9045 */ /* non-hidden definition of type  ..inherited from other files */

#include "mbox.h"
#include <stdlib.h>
#include <string.h>



#ifdef _TEST_MBOX_
#include <stdio.h>
#include "arc_assert.h"
#endif



Arc_MBoxType* Arc_MBoxCreate( uint32 size ) {
    Arc_MBoxType *mPtr;

    mPtr = malloc(sizeof(Arc_MBoxType));/*lint !e586  Intended malloc */
    mPtr->cirqPtr = CirqBuffDynCreate(size,sizeof(void *)); /*lint !e613  Intended pointer usage */

    return mPtr;
}


void Arc_MBoxDestroy( Arc_MBoxType *mPtr ) {
    (void)CirqBuffDynDestroy(mPtr->cirqPtr);
    free(mPtr); /*lint !e586  Intended free */
}

/**
 * Post a message to a box, non-blocking.
 *
 */
sint32 Arc_MBoxPost( const Arc_MBoxType *mPtr, void *msg ) {
    sint32 rv;
    sint32 status;
    status = 0;
    rv = CirqBuffPush(mPtr->cirqPtr,msg);
    if( rv != 0) {
        status = 1;
    }

    return status;
}

/**
 *
 */
sint32 Arc_MBoxFetch(const Arc_MBoxType *mPtr, void *msg)
{
    sint32 rv;
    sint32 status;
    status = 0;
    rv = CirqBuffPop(mPtr->cirqPtr,msg);
    if(rv != 0) {
        status = 1;
    }
    return status;
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
    ASSERT(dataPtr[0] == 1);
    free(dataPtr);

    Arc_MBoxFetch(myBoxes[0],&dataPtr);
    ASSERT(dataPtr[0] == 2);
    free(dataPtr);

    Arc_MBoxDestroy(myBoxes[0]);
    Arc_MBoxDestroy(myBoxes[1]);
    return 0;
}

#endif
