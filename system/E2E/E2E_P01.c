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

#include "E2E_P01.h"


#define MAX_P01_DATA_LENGTH_IN_BITS    (240)
#define MAX_P01_COUNTER_VALUE          (14)


static INLINE uint8 CalculateDeltaCounter(uint8 receivedCounter, uint8 lastValidCounter)
{
    if (receivedCounter >= lastValidCounter) {
        return receivedCounter - lastValidCounter;
    }
    else {
        return MAX_P01_COUNTER_VALUE + 1 + receivedCounter - lastValidCounter;
    }
}


/* @req 4.0.3/E2E0047 */
static INLINE Std_ReturnType CheckConfig(E2E_P01ConfigType* Config) {

    /* Check for NULL pointers */
    if (Config == NULL) {
        return E2E_E_INPUTERR_NULL; /* MISRA exception */
    }

    /* Check input parameters values */
    if ((Config->DataLength > MAX_P01_DATA_LENGTH_IN_BITS) || (Config->DataLength % 8 != 0)  ||
        (Config->CounterOffset % 4 != 0) || Config->CRCOffset % 8 != 0) {
        return E2E_E_INPUTERR_WRONG; /* MISRA exception */
    }

    /* Check that input parameters do not overlap and fits into the data array. It is enough to
     * check if the CRC and Counter are in the same byte since the CRC is one byte long */
    if ((Config->CRCOffset + 8 > Config->DataLength) || (Config->CounterOffset + 4 > Config->DataLength) ||
        (Config->CRCOffset/8 == Config->CounterOffset/8)) {
        return E2E_E_INPUTERR_WRONG; /* MISRA exception */
    }

    return E2E_E_OK;
	
}


/* @req 4.0.3/E2E0070 */
/* @req 4.0.3/E2E0082 */
/* @req 4.0.3/E2E0083 */
/* @req 4.0.3/E2E0163 */
/* @req 4.0.3/E2E0190 */
/* @req 4.0.3/E2E0218 */

/* The start value is 0x00 for E2E (0xFF is standard in the CRC lib). The final
 * XOR is 0x00 (0xFF is standard) so we must XOR the crc with 0xFF
 */
static uint8 CalculateCrc(E2E_P01ConfigType* Config, uint8 Counter, uint8* Data)
{
    uint8 crc = 0x00;
    uint8 lowerByteId = (uint8)Config->DataID;
    uint8 upperByteId = (uint8)(Config->DataID>>8);

    /* Calculate CRC on the Data ID */
    if (Config->DataIDMode == E2E_P01_DATAID_BOTH)
    {
        crc = Crc_CalculateCRC8(&lowerByteId, 1, crc, FALSE);
        crc = Crc_CalculateCRC8(&upperByteId, 1, crc ^ CRC8_XOR, FALSE);
    }
    else if (Config->DataIDMode == E2E_P01_DATAID_LOW)
    {
        crc = Crc_CalculateCRC8(&lowerByteId, 1, crc, FALSE);
    }
    else if (Counter % 2 == 0)
    {
        crc = Crc_CalculateCRC8(&lowerByteId, 1, crc, FALSE);
    }
    else
    {
        crc = Crc_CalculateCRC8(&upperByteId, 1, crc, FALSE);
    }


    /* Calculate CRC on the data */
    if (Config->CRCOffset >= 8) {
        crc = Crc_CalculateCRC8 (Data, (Config->CRCOffset / 8), crc ^ CRC8_XOR, FALSE);
    }

    if (Config->CRCOffset / 8 < (Config->DataLength / 8) - 1) {
        crc = Crc_CalculateCRC8 (&Data[Config->CRCOffset/8 + 1],
                                (Config->DataLength / 8 - Config->CRCOffset / 8 - 1),
                                crc ^ CRC8_XOR, FALSE);
    }

    return crc ^ CRC8_XOR;

}


/* @req 4.0.3/E2E0011 */
/* @req 4.0.3/E2E0012 */
/* @req 4.0.3/E2E0070 */
/* @req 4.0.3/E2E0166 */
/* @req 4.0.3/E2E0169 */
/* @req 4.0.3/E2E0195 */
/* @req 4.0.3/E2E0218 */
Std_ReturnType E2E_P01Protect(E2E_P01ConfigType* Config, E2E_P01SenderStateType* State, uint8* Data) {

    Std_ReturnType returnValue = CheckConfig(Config);

    if (E2E_E_OK != returnValue) {
        return returnValue;
    }

    if ((State == NULL) || (Data == NULL)) {
        return E2E_E_INPUTERR_NULL; /* MISRA exception */
    }

    /* Put counter in data*/
    if (Config->CounterOffset % 8 == 0) {
        *(Data+(Config->CounterOffset/8)) = (*(Data+(Config->CounterOffset/8)) & 0xF0) | (State->Counter & 0x0F);
    }
    else {
        *(Data+(Config->CounterOffset/8)) = (*(Data+(Config->CounterOffset/8)) & 0x0F) | ((State->Counter<<4) & 0xF0);
    }

    /* Calculate CRC */
    *(Data+(Config->CRCOffset/8)) = CalculateCrc(Config, State->Counter, Data);

    /* Update counter */
    State->Counter = E2E_UpdateCounter(State->Counter);

    return E2E_E_OK;
}


/* @req 4.0.3/E2E0011 */
/* @req 4.0.3/E2E0012 */
/* @req 4.0.3/E2E0076 */
/* @req 4.0.3/E2E0196 */
/* @req 4.0.3/E2E0218 */
Std_ReturnType E2E_P01Check(E2E_P01ConfigType* Config, E2E_P01ReceiverStateType* State, uint8* Data) {

    uint8 receivedCounter = 0;
    uint8 receivedCrc = 0;
    uint8 calculatedCrc = 0;
    uint8 delta = 0;
    Std_ReturnType returnValue = CheckConfig(Config);

    if (E2E_E_OK != returnValue) {
        return returnValue;
    }

    if ((State == NULL) || (Data == NULL)) {
        return E2E_E_INPUTERR_NULL; /* MISRA exception */
    }

    /* This is the stated behavior in figure 7-6 but it seems wrong. This means that if more than 15
     * messages are received with wrong CRC, the first message with correct CRC will be have state
     * OKSOMELOST even though it should be WRONGSEQUENCE.
     */
    if (State->MaxDeltaCounter < MAX_P01_COUNTER_VALUE) {
        State->MaxDeltaCounter++;
    }

    if (State->NewDataAvailable == FALSE) {
        State->Status = E2E_P01STATUS_NONEWDATA;
        return E2E_E_OK;  /* MISRA exception */
    }

    /* Counter offset is 4-bit aligned, this check is used to find out if high or low nibble */
    if (Config->CounterOffset % 8 == 0) {
        receivedCounter = *(Data+(Config->CounterOffset/8)) & 0x0F;
    }
    else {
        receivedCounter = (*(Data+(Config->CounterOffset/8)) >> 4) & 0x0F;
    }

    receivedCrc = *(Data+(Config->CRCOffset/8));
    calculatedCrc = CalculateCrc(Config, receivedCounter, Data);

    if (receivedCrc != calculatedCrc) {
        State->Status = E2E_P01STATUS_WRONGCRC;
        return E2E_E_OK;
    }

    /* Check if this is the first data since initialization */
    if (State->WaitForFirstData == TRUE) {
        State->WaitForFirstData = FALSE;
        State->MaxDeltaCounter = Config->MaxDeltaCounterInit;
        State->LastValidCounter = receivedCounter;
        State->Status= E2E_P01STATUS_INITAL;
        return E2E_E_OK;
    }

    /* Ok, this is the normal case. Check the counter delta */
    delta = CalculateDeltaCounter(receivedCounter, State->LastValidCounter);

    if (delta == 1) {
        State->MaxDeltaCounter = Config->MaxDeltaCounterInit;
        State->LastValidCounter = receivedCounter;
        State->LostData = 0;
        State->Status= E2E_P01STATUS_OK;
    }
    else if (delta == 0) {
        State->Status= E2E_P01STATUS_REPEATED;
    }
    else if (delta <= State->MaxDeltaCounter) {
        State->MaxDeltaCounter = Config->MaxDeltaCounterInit;
        State->LastValidCounter = receivedCounter;
        State->LostData = delta - 1;
        State->Status= E2E_P01STATUS_OKSOMELOST;
    }
    else {
        State->Status= E2E_P01STATUS_WRONGSEQUENCE;
    }

    return E2E_E_OK;
}

