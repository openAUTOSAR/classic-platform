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



#ifndef E2E_P01_H_
#define E2E_P01_H_

/* @req 4.0.3/E2E0113 */
/* @req 4.0.3/E2E0115 */
/* @req 4.0.3/E2E0217 */
#include "E2E.h"


/* @req 4.0.3/E2E0200 */
typedef enum {
    E2E_P01_DATAID_BOTH = 0x0,
    E2E_P01_DATAID_ALT = 0x1,
    E2E_P01_DATAID_LOW = 0x2
} E2E_P01DataIDMode;


/* @req 4.0.3/E2E0018  */
/* @req 4.0.3/E2E0085  */
/* @req 4.0.3/E2E0227  No explicit support for variants but the config allows such a configuration */
/* @req 4.0.3/E2E0228  No explicit support for variants but the config allows such a configuration */
typedef struct  {
    uint16 CounterOffset;
    uint16 CRCOffset;
    uint16 DataID;
    E2E_P01DataIDMode DataIDMode;
    uint16 DataLength;
    uint8 MaxDeltaCounterInit;
} E2E_P01ConfigType;


/* @req 4.0.3/E2E0020 */
typedef struct {
    uint8 Counter;
} E2E_P01SenderStateType;


/* @req 4.0.3/E2E0022 */
typedef enum {
        E2E_P01STATUS_OK = 0x0,
        E2E_P01STATUS_NONEWDATA = 0x1,
        E2E_P01STATUS_WRONGCRC = 0x2,
        E2E_P01STATUS_INITAL = 0x4,
        E2E_P01STATUS_REPEATED = 0x8,
        E2E_P01STATUS_OKSOMELOST = 0x20,
        E2E_P01STATUS_WRONGSEQUENCE = 0x40
} E2E_P01ReceiverStatusType;


/* @req 4.0.3/E2E0021 */
typedef struct {
    uint8 LastValidCounter;
    uint8 MaxDeltaCounter;
    boolean WaitForFirstData;
    boolean NewDataAvailable;
    uint8 LostData;
    E2E_P01ReceiverStatusType Status;
} E2E_P01ReceiverStateType;


/* @req 4.0.3/E2E0047 */
/* @req 4.0.3/E2E0166 */
Std_ReturnType E2E_P01Protect( E2E_P01ConfigType* Config, E2E_P01SenderStateType* State, uint8* Data);


/* @req 4.0.3/E2E0047 */
/* @req 4.0.3/E2E0158 */
Std_ReturnType E2E_P01Check( E2E_P01ConfigType* Config, E2E_P01ReceiverStateType* State, uint8* Data);


#endif
