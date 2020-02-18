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

/** @reqSettings DEFAULT_SPECIFICATION_REVISION=4.3.0 */

/* @req SWS_E2E_00115 E2E library files (i.e. E2E_*.*) shall not include any RTE files */
/* @req SWS_E2E_00216
 * The E2E Library shall not call BSW modules for error reporting (in particular DEM and DET), nor for any other purpose.
 * The E2E Library shall not call RTE. */

/* @req SWS_E2E_00215 Files E2E_P01.c and E2E_P01.h shall contain implementation parts specific of each profile. */
/* @req SWS_E2E_00049 The E2E library shall not contain library-internal mechanisms for error detection to be traced as development errors. */
/* @req SWS_E2E_00221 Each E2E Profile shall use a subset of the data protection mechanisms defined in AUTOSAR_SWS_E2ELibrary.pdf, 7.2. */

/* @req SWS_E2E_00011 The E2E Library shall report errors detected by library-internal mechanisms
 * to callers of E2E functions through return value.
 */
/* @req SWS_E2E_00012 The internal library mechanisms shall detect and report errors
 * shall be implemented according to the pre-defined E2E Profiles specified in sections 7.3 and 7.4.
 */

#include "E2E_P01.h"
//#define USE_LDEBUG_PRINTF // Uncomment this to turn debug statements on.
//#include "debug.h"

/* @req SWS_E2E_00018 Length of data, in bits. The value shall be a multiple of 8 and shall be ≤ 240. */
#define MAX_P01_DATA_LENGTH_IN_BITS    (240)
/* @req SWS_E2E_00020 Counter to be used for protecting the next Data.
 * The value 0xF is skipped (after 0xE the next is 0x0), as 0xF value represents the error value.
 */
#define MAX_P01_COUNTER_VALUE          (14)

/* For CRC 8*/
#define Crc_8_Xor                      0xFFU

#define E2E_P01_AR_RELEASE_MAJOR_VERSION_INT        4u
#define E2E_P01_AR_RELEASE_MINOR_VERSION_INT        3u
#define E2E_P01_AR_RELEASE_REVISION_VERSION_INT     0u

#define E2E_P01_SW_MAJOR_VERSION_INT        2
#define E2E_P01_SW_MINOR_VERSION_INT        0

/* @req SWS_E2E_00327
 * The implementer of the E2E Library shall avoid the integration of incompatible files. Minimum implementation is the version check of the header files.
 */
#if (E2E_P01_SW_MAJOR_VERSION != E2E_P01_SW_MAJOR_VERSION_INT) || (E2E_P01_SW_MINOR_VERSION != E2E_P01_SW_MINOR_VERSION_INT)
#error "SW Version mismatch between E2E_P01.c and E2E_P01.h"
#endif

#if (E2E_P01_AR_RELEASE_MAJOR_VERSION != E2E_P01_AR_RELEASE_MAJOR_VERSION_INT) || (E2E_P01_AR_RELEASE_MINOR_VERSION != E2E_P01_AR_RELEASE_MINOR_VERSION_INT) || \
    (E2E_P01_AR_RELEASE_REVISION_VERSION != E2E_P01_AR_RELEASE_REVISION_VERSION_INT)
#error "AR Version mismatch between E2E_P01.c and E2E_P01.h"
#endif


/* Calculation of the delta counter value taken into account the overflow.
 */
static INLINE uint8 calculateDeltaCounterP01(uint8 receivedCounter, uint8 lastValidCounter) {
    uint8 res;
    res = 0;

    if (receivedCounter >= lastValidCounter) {
        res = receivedCounter - lastValidCounter;
    }

    else {
        res = MAX_P01_COUNTER_VALUE + 1 + receivedCounter - lastValidCounter;
    }

    return res;
}


/* @req SWS_E2E_00018 Configuration constraints defined for E2E_P01ConfigType */
/* @req SWS_E2E_00047 Definition of error flags for all E2E units */
/* @req SWS_E2E_00169 In E2E Profile 1, with E2E_P01DataIDMode equal to E2E_P01_DATAID_LOW, the high byte of Data ID shall be set to 0x00. */
/* @req SWS_E2E_00227  Profile variant 1A, No explicit support for variants but the config allows such a configuration */
/* @req SWS_E2E_00228  Profile variant 1B, No explicit support for variants but the config allows such a configuration */
/* @req SWS_E2E_00306 In E2E Profile 1, with E2E_P01DataIDMode equal to E2E_P01_DATAID_NIBBLE, the high nibble of the high byte shall be 0x0. */
/* @req SWS_E2E_00307  Profile variant 1C, No explicit support for variants but the config allows such a configuration */
/* UC_E2E_00053 Any user of E2E Profile 1 shall use whenever possible the defined E2E variants. */

/* Checks the correctness of the configuration parameter.
 */
static INLINE Std_ReturnType checkConfigP01(const E2E_P01ConfigType* Config) {

    Std_ReturnType status;
    status = E2E_E_OK;

    /* Check for NULL pointers */
    if (Config == NULL_PTR) {
        status = E2E_E_INPUTERR_NULL;
    }

    /* Check input parameters values */
    else if ((Config->DataLength > MAX_P01_DATA_LENGTH_IN_BITS) || ((Config->DataLength % 8) != 0)  ||
        ((Config->CounterOffset % 4) != 0) || ((Config->CRCOffset % 8) != 0)) {
        status = E2E_E_INPUTERR_WRONG;
    }

    /* Check that input parameters do not overlap and fits into the data array. It is enough to
     * check if the CRC and Counter are in the same byte since the CRC is one byte long */
    else if (((Config->CRCOffset + 8) > Config->DataLength) || ((Config->CounterOffset + 4) > Config->DataLength) ||
        ((Config->CRCOffset/8) == (Config->CounterOffset/8))) {
        status = E2E_E_INPUTERR_WRONG;
    }

    /* For DataIDMode different than E2E_P01_DATAID_NIBBLE, DataIDNibbleOffset shall be initialized to 0 (even if it is ignored by E2E Library). */
    else if ((Config->DataIDMode != E2E_P01_DATAID_NIBBLE) && (Config->DataIDNibbleOffset != 0)) {
        status = E2E_E_INPUTERR_WRONG;
    }

    /* In E2E Profile 1, with E2E_P01DataIDMode equal to E2E_P01_DATAID_LOW, the high byte of Data ID shall be set to 0x00. */
    else if ((Config->DataIDMode == E2E_P01_DATAID_LOW) && ((Config->DataID>>8) != 0)) {
        status = E2E_E_INPUTERR_WRONG;
    }

    /* In E2E Profile 1, with E2E_P01DataIDMode equal to E2E_P01_DATAID_NIBBLE, the high nibble of the high byte shall be 0x0. */
    else if ((Config->DataIDMode == E2E_P01_DATAID_NIBBLE) && ((Config->DataID>>12) != 0)) {
        status = E2E_E_INPUTERR_WRONG;
    }

    else {
        /* return E2E_E_OK*/
        status = E2E_E_OK;
    }

    return status;
}


/* @req SWS_E2E_00070 E2E Profile 1 shall use the polynomial of CRC-8-SAE J1850 */
/* @req SWS_E2E_00082 In E2E Profile 1, the CRC is calculated over:
 * 1. First over the one or two bytes of the Data ID (depending on Data ID configuration) and then
 * 2. over all transmitted bytes of a safety-related complex data element/signal group (except the CRC byte). */
/* @req SWS_E2E_00083 E2E Profile 1 shall use CRC-8-SAE J1850 for CRC calculation.
 * It shall use 0x00 as the start value and XOR value.*/
/* @req SWS_E2E_00163 Defines inclusion modes for the two-byte Data ID into the calculation of the one-byte CRC. */
/* @req SWS_E2E_00190 E2E Profile 1 shall use the Crc_CalculateCRC8 () function of the SWS CRC Library for calculating CRC checksums. */
/* @req SWS_E2E_00218 Specification of E2E Profile 1 */

/* Calculate CRC over Data ID and Data
 */
static uint8 calculateCrcP01(const E2E_P01ConfigType* Config, uint8 Counter, const uint8* Data)
{
    /* The start value is 0x00 for E2E (0xFF is standard in the CRC lib). The final
     * XOR is 0x00 (0xFF is standard) so we must XOR the crc with 0xFF
     */
    uint8 crc = 0x00u ^ Crc_8_Xor; /* Need to cancel first XOR in CRC */
    uint8 lowerByteId = (uint8)Config->DataID;
    uint8 upperByteId = (uint8)(Config->DataID>>8);

    /* Calculate CRC on the Data ID */
    if (Config->DataIDMode == E2E_P01_DATAID_BOTH)
    {
        crc = Crc_CalculateCRC8(&lowerByteId, 1, crc, FALSE);
        crc = Crc_CalculateCRC8(&upperByteId, 1, crc, FALSE);
    }
    else if (Config->DataIDMode == E2E_P01_DATAID_LOW)
    {
        crc = Crc_CalculateCRC8(&lowerByteId, 1, crc, FALSE);
    }
    else if (Config->DataIDMode == E2E_P01_DATAID_NIBBLE)
    {
        crc = Crc_CalculateCRC8(&lowerByteId, 1, crc, FALSE);
        upperByteId = 0;
        crc = Crc_CalculateCRC8(&upperByteId, 1, crc, FALSE);
    }
    else if ((Counter % 2) == 0) //E2E_P01_DATAID_ALT: even counter
    {
        crc = Crc_CalculateCRC8(&lowerByteId, 1, crc, FALSE);
    }
    else //E2E_P01_DATAID_ALT: odd counter
    {
        crc = Crc_CalculateCRC8(&upperByteId, 1, crc, FALSE);
    }


    /* Calculate CRC on the data */
    if (Config->CRCOffset >= 8) {
        crc = Crc_CalculateCRC8 (Data, (Config->CRCOffset / 8), crc, FALSE);
    }

    if ((Config->CRCOffset / 8) < ((Config->DataLength / 8) - 1)) {
        crc = Crc_CalculateCRC8 (&Data[(Config->CRCOffset/8) + 1],
                                (((Config->DataLength / 8) - (Config->CRCOffset / 8)) - 1),
                                crc, FALSE);
    }

    return crc ^ Crc_8_Xor; /* Need to cancel last XOR in CRC */
}


/* @req SWS_E2E_00070 E2E Profile 1 shall use the polynomial of CRC-8-SAE J1850 */
/* @req SWS_E2E_00075 In E2E Profile 1, on the sender side,
 * for the first transmission request of a data element the counter shall be initialized with 0 and
 * shall be incremented by 1 for every subsequent send request.
 * When the counter reaches the value 14 (0xE),
 * then it shall restart with 0 for the next send request (i.e. value 0xF shall be skipped).
 */
/* @req SWS_E2E_00169 In E2E Profile 1, with E2E_P01DataIDMode equal to E2E_P01_DATAID_LOW,
 * the high byte of Data ID shall be set to 0x00.
 */
/* @req SWS_E2E_00195 The function E2E_P01Protect() shall write data as specified
 * by Figure 7-5 and Figure 7-6 in AUTOSAR_SWS_E2ELibrary.pdf
 */
/* @req SWS_E2E_00218 Profile 1 shall provide the following mechanisms as specified in 7.3 */
/* @req SWS_E2E_00306 In E2E Profile 1, with E2E_P01DataIDMode equal to E2E_P01_DATAID_NIBBLE,
 * the high nibble of the high byte shall be 0x0.
 */
/* @req SWS_E2E_00166 API for E2E_P01Protect */

/* Protects the array/buffer to be transmitted using the E2E profile 1.
 */
Std_ReturnType E2E_P01Protect(const E2E_P01ConfigType* ConfigPtr, E2E_P01ProtectStateType* StatePtr, uint8* DataPtr) {

    Std_ReturnType status;
    status = E2E_E_OK;
    Std_ReturnType returnValue = checkConfigP01(ConfigPtr);

    if (E2E_E_OK != returnValue) {
        status = returnValue;
    }

    else if ((StatePtr == NULL_PTR) || (DataPtr == NULL_PTR)) {
        status = E2E_E_INPUTERR_NULL;
    }

    else {
        /* Put counter in data*/
        if ((ConfigPtr->CounterOffset % 8) == 0) {
            DataPtr[ConfigPtr->CounterOffset/8] = (DataPtr[(ConfigPtr->CounterOffset/8)] & 0xF0u) | (StatePtr->Counter & 0x0Fu);
        }
        else {
            DataPtr[ConfigPtr->CounterOffset/8] = (DataPtr[ConfigPtr->CounterOffset/8] & 0x0Fu) | ((StatePtr->Counter<<4) & 0xF0u);
        }

        /* Put counter in data for E2E_P01_DATAID_NIBBLE */ // ASR4.2.2
        if (ConfigPtr->DataIDMode == E2E_P01_DATAID_NIBBLE) {
            if ((ConfigPtr->DataIDNibbleOffset % 8) == 0) {
                DataPtr[ConfigPtr->DataIDNibbleOffset/8] = (DataPtr[(ConfigPtr->DataIDNibbleOffset/8)] & 0xF0u) | ((uint8)((ConfigPtr->DataID>>8) & 0x0Fu));
            }
            else {
                DataPtr[ConfigPtr->DataIDNibbleOffset/8] = (DataPtr[ConfigPtr->DataIDNibbleOffset/8] & 0x0Fu) | ((uint8)((ConfigPtr->DataID>>4) & 0xF0u));
            }
        }

        /* Calculate CRC */
        DataPtr[(ConfigPtr->CRCOffset/8)] = calculateCrcP01(ConfigPtr, StatePtr->Counter, DataPtr);

        /* Update counter */
        StatePtr->Counter = (StatePtr->Counter+1) % 15;
    }

    return status;
}


/* Reads the counter value
 */
static INLINE uint8 readCounterP01(const E2E_P01ConfigType* ConfigPtr, const uint8* DataPtr) {

    uint8 receivedCounter = 0;

    /* Counter offset is 4-bit aligned, this check is used to find out if high or low nibble */
    if ((ConfigPtr->CounterOffset % 8) == 0) {
        receivedCounter = DataPtr[ConfigPtr->CounterOffset/8] & 0x0Fu;
    }
    else {
        receivedCounter = (DataPtr[ConfigPtr->CounterOffset/8] >> 4) & 0x0Fu;
    }

    return receivedCounter;
}


/* Reads the data id nibble offset
 */
static INLINE uint8 readDataIDNibbleOffsetP01(const E2E_P01ConfigType* ConfigPtr, const uint8* DataPtr) {

    uint8 receivedDataIDNibble = 0;

    if (ConfigPtr->DataIDMode == E2E_P01_DATAID_NIBBLE) {

        if ((ConfigPtr->DataIDNibbleOffset % 8) == 0) {
            receivedDataIDNibble = DataPtr[(ConfigPtr->DataIDNibbleOffset)/8] & 0x0Fu;
        }
        else {
            receivedDataIDNibble = (DataPtr[(ConfigPtr->DataIDNibbleOffset)/8] >> 4) & 0x0Fu;
        }
    }

    return receivedDataIDNibble;
}


/* Check if that is the first data since initialisation
 */
static INLINE boolean isFirstDataSinceInitialisationP01(const E2E_P01ConfigType* ConfigPtr, E2E_P01CheckStateType* StatePtr, uint8 receivedCounter) {

    boolean status;
    status = FALSE;

    if (StatePtr->WaitForFirstData == TRUE) {
        StatePtr->WaitForFirstData = FALSE;
        StatePtr->MaxDeltaCounter = ConfigPtr->MaxDeltaCounterInit;
        StatePtr->LastValidCounter = receivedCounter;
        StatePtr->Status = E2E_P01STATUS_INITIAL;
        status = TRUE;
    }

    return status;
}


/* Check first if the received CRC and calculated CRC is identical or not
 */
static INLINE boolean isCheckOfReceivedDataNotOkP01(const E2E_P01ConfigType* ConfigPtr, E2E_P01CheckStateType* StatePtr, uint8 receivedCounter, uint8 receivedCrc, uint8 calculatedCrc, uint8 receivedDataIDNibble) {

    boolean status = FALSE;

    if ( (receivedCrc != calculatedCrc) ||
         ( (ConfigPtr->DataIDMode == E2E_P01_DATAID_NIBBLE) && (receivedDataIDNibble != ((ConfigPtr->DataID)>>8)) )
       ) {
//      LDEBUG_PRINTF("receivedCrcreceivedDataIDNibble check");
//      LDEBUG_PRINTF("\n");
        StatePtr->Status = E2E_P01STATUS_WRONGCRC;
        status = TRUE;
    }
    /* Check if this is the first data since initialization, this should only be done if the CRC check above
     * has been done without errors   
     */
    else if (isFirstDataSinceInitialisationP01(ConfigPtr, StatePtr, receivedCounter) == TRUE) {
//      LDEBUG_PRINTF("first data check");
//      LDEBUG_PRINTF("\n");
        status = TRUE;
    }
    /* @CODECOV:DEFAULT_CASE:Else statement is required for defensive programming. */
    __CODE_COVERAGE_IGNORE__
    else {
        /* Do nnothing but needed for MISRA */
    }


    return status;
}


/* Checks the Data received using the E2E profile 1.
 * This is the normal case. Calculate delta counter, Check the counter delta.
 */
static INLINE void doChecksP01(const E2E_P01ConfigType* ConfigPtr, E2E_P01CheckStateType* StatePtr, uint8 receivedCounter) {

    uint8 delta;

    delta = calculateDeltaCounterP01(receivedCounter, StatePtr->LastValidCounter);

    if (delta == 0) {
        //  LDEBUG_PRINTF("deltaCounter=0");
        //  LDEBUG_PRINTF("\n");

            if (StatePtr->NoNewOrRepeatedDataCounter < MAX_P01_COUNTER_VALUE) {
                StatePtr->NoNewOrRepeatedDataCounter++;
            }

            StatePtr->Status = E2E_P01STATUS_REPEATED;
    }
    else if (delta == 1) {
        //  LDEBUG_PRINTF("deltaCounter=1");
        //  LDEBUG_PRINTF("\n");

            StatePtr->MaxDeltaCounter = ConfigPtr->MaxDeltaCounterInit;
            StatePtr->LastValidCounter = receivedCounter;
            StatePtr->LostData = 0;

            if (StatePtr->NoNewOrRepeatedDataCounter <= ConfigPtr->MaxNoNewOrRepeatedData) {
                StatePtr->NoNewOrRepeatedDataCounter = 0;

                if (StatePtr->SyncCounter > 0) {
                    StatePtr->SyncCounter--;
                    StatePtr->Status = E2E_P01STATUS_SYNC;
                }
                else {
                    StatePtr->Status = E2E_P01STATUS_OK;
                }
            }
            else {
                StatePtr->NoNewOrRepeatedDataCounter = 0;
                StatePtr->SyncCounter = ConfigPtr->SyncCounterInit;
                StatePtr->Status = E2E_P01STATUS_SYNC;
            }
    }
    else if (delta <= StatePtr->MaxDeltaCounter) { // 1 < delta <= MaxDeltaCounter
        //  LDEBUG_PRINTF("1<deltaCounter<=Max");
        //  LDEBUG_PRINTF("\n");

            StatePtr->MaxDeltaCounter = ConfigPtr->MaxDeltaCounterInit;
            StatePtr->LastValidCounter = receivedCounter;
            StatePtr->LostData = delta - 1;

            if (StatePtr->NoNewOrRepeatedDataCounter <= ConfigPtr->MaxNoNewOrRepeatedData) {
                StatePtr->NoNewOrRepeatedDataCounter= 0;

                if (StatePtr->SyncCounter > 0) {
                    StatePtr->SyncCounter--;
                    StatePtr->Status = E2E_P01STATUS_SYNC;
                }
                else {
                    StatePtr->Status = E2E_P01STATUS_OKSOMELOST;
                }
            }
            else {
                StatePtr->NoNewOrRepeatedDataCounter= 0;
                StatePtr->SyncCounter = ConfigPtr->SyncCounterInit;
                StatePtr->Status = E2E_P01STATUS_SYNC;
            }
    }
    else { // delta > MaxDeltaCounter
        //  LDEBUG_PRINTF("deltaCounter>Max");
        //  LDEBUG_PRINTF("\n");

            StatePtr->NoNewOrRepeatedDataCounter= 0;
            StatePtr->SyncCounter = ConfigPtr->SyncCounterInit;

            if (StatePtr->SyncCounter > 0) {
                StatePtr->MaxDeltaCounter = ConfigPtr->MaxDeltaCounterInit;
                StatePtr->LastValidCounter = receivedCounter;
            }

            StatePtr->Status = E2E_P01STATUS_WRONGSEQUENCE;
    }
}


/* @req SWS_E2E_00076 In E2E Profile 1, on the receiver side,
 * by evaluating the counter of received data against the counter of previously received data,
 * the following shall be detected: */
/* @req SWS_E2E_00196 The function E2E_P01Check shall check as specified
 * by Figure 7-7 and Figure 7-6 in AUTOSAR_SWS_E2ELibrary.
 */
/* @req SWS_E2E_00218 Profile 1 shall provide the following mechanisms as specified in 7.3. */
/* @req SWS_E2E_00306 In E2E Profile 1, with E2E_P01DataIDMode equal to E2E_P01_DATAID_NIBBLE,
 * the high nibble of the high byte shall be 0x0. */
/* @req SWS_E2E_00158 API for E2E_P01Check */

/* Checks the Data received using the E2E profile 1.
 */
Std_ReturnType E2E_P01Check(const E2E_P01ConfigType* ConfigPtr, E2E_P01CheckStateType* StatePtr, const uint8* DataPtr) {

    Std_ReturnType status;
    status = E2E_E_OK;
    boolean ret;
    ret = TRUE;
    uint8 receivedCounter;
    uint8 receivedCrc;
    uint8 calculatedCrc;
    uint8 receivedDataIDNibble;
    Std_ReturnType returnValue = checkConfigP01(ConfigPtr);

    if (E2E_E_OK != returnValue) {
        status = returnValue;
    }

    else if ((StatePtr == NULL_PTR) || (DataPtr == NULL_PTR)) {
        status = E2E_E_INPUTERR_NULL;
    }

    else {

        /* This is the stated behavior in figure 7-6 but it seems wrong. This means that if more than 15
         * messages are received with wrong CRC, the first message with correct CRC will be have state
         * OKSOMELOST even though it should be WRONGSEQUENCE.
         */
        // changed in ASR 4.2.2, figure is now 7-7
        if (StatePtr->MaxDeltaCounter < MAX_P01_COUNTER_VALUE) {
            StatePtr->MaxDeltaCounter++;
        }

        if (StatePtr->NewDataAvailable == FALSE) {

            if (StatePtr->NoNewOrRepeatedDataCounter < MAX_P01_COUNTER_VALUE) {
                StatePtr->NoNewOrRepeatedDataCounter++;
            }

            StatePtr->Status = E2E_P01STATUS_NONEWDATA;
            status = E2E_E_OK;
            ret = FALSE;
        }

        if (ret == TRUE){
            receivedCounter = readCounterP01(ConfigPtr, DataPtr);

            if (receivedCounter > MAX_P01_COUNTER_VALUE) {
                status = E2E_E_INPUTERR_WRONG;
            } else {
                receivedCrc = DataPtr[(ConfigPtr->CRCOffset/8)];

                receivedDataIDNibble = readDataIDNibbleOffsetP01(ConfigPtr, DataPtr);

                calculatedCrc = calculateCrcP01(ConfigPtr, receivedCounter, DataPtr);

                if (isCheckOfReceivedDataNotOkP01(ConfigPtr, StatePtr, receivedCounter, receivedCrc, calculatedCrc, receivedDataIDNibble) == TRUE) {
                    status = E2E_E_OK;
                } else {
                    doChecksP01(ConfigPtr, StatePtr, receivedCounter);
                }
            }
        }
    }

    return status;
}


/* @req SWS_E2E_00075 In E2E Profile 1, on the sender side,
 * for the first transmission request of a data element the counter shall be initialized with 0
 */
/* @req SWS_E2E_00386 In case State is NULL, E2E_P01ProtectInit shall return immediately with E2E_E_INPUTERR_NULL.
 * Otherwise, it shall intialize the state structure, setting Counter to 0.
 */
/* @req SWS_E2E_00385 API for E2E_P01ProtectInit */

/* Initializes the protection state.
 */
Std_ReturnType E2E_P01ProtectInit(E2E_P01ProtectStateType* StatePtr) {

    Std_ReturnType status;
    status = E2E_E_OK;

    if (StatePtr == NULL_PTR) {
        status = E2E_E_INPUTERR_NULL;
    } else {
        StatePtr->Counter = 0;
    }

    return status;
}


/* @req SWS_E2E_00389 In case State is NULL, E2E_P01CheckInit shall return immediately with E2E_E_INPUTERR_NULL.
 * Otherwise, it shall initialize the state structure */
/* UC_E2E_00208, UC_E2E_00297, SWS_E2E_00324SWS_E2E_00325  overwrites init values  NewDataAvailable */
/* @req SWS_E2E_00390 API for E2E_P01CheckInit */
/* @req ARC_SWS_E2E_00001 */

/* Initializes the check state.
 */
Std_ReturnType E2E_P01CheckInit(E2E_P01CheckStateType* StatePtr) {

    Std_ReturnType status;
    status = E2E_E_OK;

    if (StatePtr == NULL_PTR) {
        status = E2E_E_INPUTERR_NULL;
    } else {
        StatePtr->LastValidCounter = 0;
        StatePtr->MaxDeltaCounter = 0;
        StatePtr->WaitForFirstData = TRUE;
        StatePtr->NewDataAvailable = FALSE;
        StatePtr->LostData = 0;
        StatePtr->Status = E2E_P01STATUS_NONEWDATA;
        StatePtr->SyncCounter = 0;
        StatePtr->NoNewOrRepeatedDataCounter = 0;
    }

    return status;
}


/* @req SWS_E2E_00383 If CheckReturn == E2E_E_OK and ProfileBehavior == TRUE,
 * then the function E2E_P01MapStatusToSM shall return the values depending on the value of Status.
 */

/* The function maps the check status of Profile 1 to a generic check status in case ProfileBehavior == True
 */
static INLINE E2E_PCheckStatusType doE2E_P01MapStatusToSM_TRUE(E2E_P01CheckStatusType Status) {

    E2E_PCheckStatusType retValue = E2E_P_ERROR;

    switch(Status) {
        case E2E_P01STATUS_OK:
        case E2E_P01STATUS_OKSOMELOST:
        case E2E_P01STATUS_SYNC:
            retValue = E2E_P_OK;
            break;
        case E2E_P01STATUS_REPEATED:
            retValue = E2E_P_REPEATED;
            break;
        case E2E_P01STATUS_NONEWDATA:
            retValue = E2E_P_NONEWDATA;
            break;
        case E2E_P01STATUS_WRONGSEQUENCE:
        case E2E_P01STATUS_INITIAL:
            retValue = E2E_P_WRONGSEQUENCE;
            break;
        /* @CODECOV:DEFAULT_CASE:Default statement is required for defensive programming. If this happens the state will be restored to E2E_P_ERROR */
        __CODE_COVERAGE_IGNORE__
        default:
            retValue = E2E_P_ERROR;
            break;
    }

    return retValue;
}


/* @req SWS_E2E_00476 If CheckReturn == E2E_E_OK and ProfileBehavior == FALSE,
 * then the function E2E_P01MapStatusToSM shall return the values depending on the value of Status.
 */

/* The function maps the check status of Profile 1 to a generic check status in case ProfileBehavior == False
 */
static INLINE E2E_PCheckStatusType doE2E_P01MapStatusToSM_FALSE(E2E_P01CheckStatusType Status) {

    E2E_PCheckStatusType retValue = E2E_P_ERROR;

    switch(Status) {
        case E2E_P01STATUS_OK:
        case E2E_P01STATUS_OKSOMELOST:
        case E2E_P01STATUS_INITIAL:
            retValue = E2E_P_OK;
            break;
        case E2E_P01STATUS_REPEATED:
            retValue = E2E_P_REPEATED;
            break;
        case E2E_P01STATUS_NONEWDATA:
            retValue = E2E_P_NONEWDATA;
            break;
        case E2E_P01STATUS_WRONGSEQUENCE:
        case E2E_P01STATUS_SYNC:
            retValue = E2E_P_WRONGSEQUENCE;
            break;
        /* @CODECOV:DEFAULT_CASE:Default statement is required for defensive programming. If this happens the state will be restored to E2E_P_ERROR */
        __CODE_COVERAGE_IGNORE__
        default:
            retValue = E2E_P_ERROR;
            break;
    }

    return retValue;
}


/* @req SWS_E2E_00382 API for E2E_P01MapStatusToSM */
/* @req SWS_E2E_00384 If CheckReturn != E2E_E_OK,
 * then the function E2E_P01MapStatusToSM() shall return E2E_P_ERROR (regardless of value of Status).
 */

/* The function maps the check status of Profile 1 to a generic check status
 */
E2E_PCheckStatusType E2E_P01MapStatusToSM(Std_ReturnType CheckReturn, E2E_P01CheckStatusType Status, boolean profileBehavior) {

    E2E_PCheckStatusType retValue = E2E_P_ERROR;

    if (CheckReturn != E2E_E_OK) {
        retValue = E2E_P_ERROR;
    }

    else if (profileBehavior == TRUE) { //ASR4.2.2 or higher behavior

        retValue = doE2E_P01MapStatusToSM_TRUE(Status);
    }
    else { //ASR4.2.2 pre-behavior

        retValue = doE2E_P01MapStatusToSM_FALSE(Status);
    }

    return retValue;
}
