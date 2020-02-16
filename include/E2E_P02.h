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



#ifndef E2E_P02_H_
#define E2E_P02_H_

/* @req 4.0.3/E2E0113 */
/* @req 4.0.3/E2E0115 */
/* @req 4.0.3/E2E0217 */
#include "E2E.h"


/* @req 4.0.3/E2E0214*/
typedef enum {
    E2E_P02STATUS_OK = 0x0,
    E2E_P02STATUS_NONEWDATA = 0x01,
    E2E_P02STATUS_WRONGCRC =  0x02,
    E2E_P02STATUS_INITIAL = 0x04,
    E2E_P02STATUS_REPEATED = 0x08,
    E2E_P02STATUS_OKSOMELOST = 0x20,
    E2E_P02STATUS_WRONGSEQUENCE = 0x40
} E2E_P02ReceiverStatusType;


/* @req 4.0.3/E2E0152 */
typedef struct {
    uint16 DataLength;
    uint8 DataIDList[16];
    uint8 MaxDeltaCounterInit;
} E2E_P02ConfigType;


/* @req 4.0.3/E2E0153 */
typedef struct {
    uint8 Counter;
} E2E_P02SenderStateType;


/* @req 4.0.3/E2E0154 */
typedef struct {
    uint8 LastValidCounter;
    uint8 MaxDeltaCounter;
    boolean WaitForFirstData;
    boolean NewDataAvailable;
    uint8 LostData;
    E2E_P02ReceiverStatusType Status;
} E2E_P02ReceiverStateType;


/* @req 4.0.3/E2E0160 */
Std_ReturnType E2E_P02Protect(E2E_P02ConfigType* Config, E2E_P02SenderStateType* State, uint8* Data);


/* @req 4.0.3/E2E0161 */
Std_ReturnType E2E_P02Check(E2E_P02ConfigType* Config, E2E_P02ReceiverStateType* State, uint8* Data);


#endif
