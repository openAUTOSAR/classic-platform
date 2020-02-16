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


/* @req 4.0.3/E2E0215 */
/* @req 4.0.3/E2E0114 */
/* @req 4.0.3/E2E0115 */
/* @req 4.0.3/E2E0049 */
/* @req 4.0.3/E2E0221 */

#include "E2E_P02.h"


#define MAX_P02_COUNTER_VALUE           (15)
#define MAX_P02_DATA_LENGTH_IN_BITS     (8*256)
#define MIN_P02_DATA_LENGTH_IN_BITS     (8*2)


/* @req 4.0.3/E2E0135*/
static INLINE uint8 CalculateDeltaCounter(uint8 receivedCounter, uint8 lastValidCounter)
{
    if (receivedCounter >= lastValidCounter) {
        return receivedCounter - lastValidCounter;
    }
    else {
        return MAX_P02_COUNTER_VALUE + 1 + receivedCounter - lastValidCounter;
    }
}


/* @req 4.0.3/E2E0047 */
static INLINE Std_ReturnType CheckConfig(E2E_P02ConfigType* Config) {

    /* Check for NULL pointers */
    if (Config == NULL) {
        return E2E_E_INPUTERR_NULL; /* MISRA exception */
    }

    if ((Config->DataLength % 8 != 0) || (Config->DataLength < MIN_P02_DATA_LENGTH_IN_BITS) ||
        (Config->DataLength > MAX_P02_DATA_LENGTH_IN_BITS)) {
        return E2E_E_INPUTERR_WRONG; /* MISRA exception */
    }

    return E2E_E_OK;

}


/* @req 4.0.3/E2E0160 */
/* @req 4.0.3/E2E0127 */
/* @req 4.0.3/E2E0128 */
/* @req 4.0.3/E2E0129 */
/* @req 4.0.3/E2E0130 */
/* @req 4.0.3/E2E0132 */
/* @req 4.0.3/E2E0133 */
/* @req 4.0.3/E2E0117 */
/* @req 4.0.3/E2E0118 */
/* @req 4.0.3/E2E0119 */
/* @req 4.0.3/E2E0120 */
/* @req 4.0.3/E2E0121 */
/* @req 4.0.3/E2E0122 */
/* @req 4.0.3/E2E0123 */
/* @req 4.0.3/E2E0124 */
/* @req 4.0.3/E2E0126 */
/* @req 4.0.3/E2E0219 */
Std_ReturnType E2E_P02Protect(E2E_P02ConfigType* Config, E2E_P02SenderStateType* State, uint8* Data) {

    uint8 dataId = 0;
    uint8 crc = 0;
    Std_ReturnType returnValue = CheckConfig(Config);

    if (E2E_E_OK != returnValue) {
        return returnValue; /* MISRA exception */
    }

    if ((State == NULL) || (Data == NULL)) {
        return E2E_E_INPUTERR_NULL; /* MISRA exception */
    }

    /* Increase counter*/
    State->Counter = (State->Counter + 1) % (MAX_P02_COUNTER_VALUE + 1);

    /* Add counter to data */
    Data[1] = (Data[1] & 0xF0) | (State->Counter & 0x0F);

    /* Get the data id */
    dataId = Config->DataIDList[State->Counter];

    /* Calculate CRC over data and then data id */
    crc = Crc_CalculateCRC8H2F(&Data[1],  (Config->DataLength/8)-1, 0x00, TRUE);
    crc = Crc_CalculateCRC8H2F(&dataId,  1, crc ^ CRC8_H2F_XOR , FALSE);

    /* Write crc to data */
    Data[0] = crc;

    return E2E_E_OK;
}


/* @req 4.0.3/E2E0161 */
/* @req 4.0.3/E2E0134 */
/* @req 4.0.3/E2E0136 */
/* @req 4.0.3/E2E0137 */
/* @req 4.0.3/E2E0138 */
/* @req 4.0.3/E2E0139 */
/* @req 4.0.3/E2E0140 */
/* @req 4.0.3/E2E0141 */
/* @req 4.0.3/E2E0142 */
/* @req 4.0.3/E2E0143 */
/* @req 4.0.3/E2E0145 */
/* @req 4.0.3/E2E0146 */
/* @req 4.0.3/E2E0147 */
/* @req 4.0.3/E2E0148 */
/* @req 4.0.3/E2E0149 */
/* @req 4.0.3/E2E0150 */
/* @req 4.0.3/E2E0151 */
/* @req 4.0.3/E2E0117 */
/* @req 4.0.3/E2E0118 */
/* @req 4.0.3/E2E0119 */
/* @req 4.0.3/E2E0120 */
/* @req 4.0.3/E2E0121 */
/* @req 4.0.3/E2E0122 */
/* @req 4.0.3/E2E0123 */
/* @req 4.0.3/E2E0125 */
/* @req 4.0.3/E2E0219 */
Std_ReturnType E2E_P02Check(E2E_P02ConfigType* Config, E2E_P02ReceiverStateType* State, uint8* Data) {

    uint8 receivedCounter = 0;
    uint8 dataId = 0;
    uint8 crc = 0;
    uint8 deltaCounter = 0;

    Std_ReturnType returnValue = CheckConfig(Config);
    if (E2E_E_OK != returnValue) {
        return returnValue;  /* MISRA exception */
    }


    if ((State == NULL) || (Data == NULL)) {
        return E2E_E_INPUTERR_NULL; /* MISRA exception */
    }

    /* This is the stated behavior in figure 7-10 but it seems wrong. This means that if more than 15
     * messages are received with wrong CRC, the first message with correct CRC will be have state
     * OKSOMELOST even though it should be WRONGSEQUENCE.
     */
    if (State->MaxDeltaCounter < MAX_P02_COUNTER_VALUE) {
        State->MaxDeltaCounter++;
    }


    if (State->NewDataAvailable == FALSE) {
        State->Status = E2E_P02STATUS_NONEWDATA;
        return E2E_E_OK;
    }

    receivedCounter = Data[1] &0xF;

    dataId = Config->DataIDList[receivedCounter];

    /* Calculate CRC over data and then data id */
    crc = Crc_CalculateCRC8H2F(&Data[1],  Config->DataLength/8-1, 0x00, TRUE);
    crc = Crc_CalculateCRC8H2F(&dataId,  1, crc ^ CRC8_H2F_XOR , FALSE);

    /* Compare calculate and received crc */
    if (crc != Data[0]) {
        State->Status = E2E_P02STATUS_WRONGCRC;
        return E2E_E_OK;
    }


    if (State->WaitForFirstData == TRUE)
    {
        State->WaitForFirstData = FALSE;
        State->MaxDeltaCounter = Config->MaxDeltaCounterInit;
        State->LastValidCounter = receivedCounter;
        State->Status= E2E_P02STATUS_INITIAL;
        return E2E_E_OK;
    }


    /* Calculate delta counter */
    deltaCounter = CalculateDeltaCounter(receivedCounter, State->LastValidCounter);

    if (deltaCounter == 1) {
        State->MaxDeltaCounter = Config->MaxDeltaCounterInit;
        State->LastValidCounter = receivedCounter;
        State->LostData = 0;
        State->Status= E2E_P02STATUS_OK;
    }
    else if (deltaCounter == 0) {
        State->Status= E2E_P02STATUS_REPEATED;

    }
    else if (deltaCounter > State->MaxDeltaCounter) {
        State->Status= E2E_P02STATUS_WRONGSEQUENCE;
    }
    else {
        State->MaxDeltaCounter = Config->MaxDeltaCounterInit;
        State->LastValidCounter = receivedCounter;
        State->LostData = deltaCounter - 1;
        State->Status= E2E_P02STATUS_OKSOMELOST;
    }

    return E2E_E_OK;
}



