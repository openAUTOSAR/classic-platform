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

/* @req SWS_E2E_00215 Files E2E_P02.c and E2E_P02.h shall contain implementation parts specific of each profile. */
/* @req SWS_E2E_00049 The E2E library shall not contain library-internal mechanisms for error detection to be traced as development errors. */
/* @req SWS_E2E_00221 Each E2E Profile shall use a subset of the data protection mechanisms defined in AUTOSAR_SWS_E2ELibrary.pdf, 7.2. */

/* @req SWS_E2E_00011 The E2E Library shall report errors detected by library-internal mechanisms
 * to callers of E2E functions through return value.
 */
/* @req SWS_E2E_00012 The internal library mechanisms shall detect and report errors
 * shall be implemented according to the pre-defined E2E Profiles specified in sections 7.3 and 7.4.
 */

#include "E2E_P02.h"

//#define USE_LDEBUG_PRINTF // Uncomment this to turn debug statements on.
//#include "debug.h"

/* @req SWS_E2E_00153 the counter is incremented modulo 16. */
#define MAX_P02_COUNTER_VALUE           (15)
/* @req SWS_E2E_00121 Length of data, in bits.
 * E2E checks that Length is ≥ MinDataLength. The value shall be ≥ 2*8.
 * E2E checks that DataLength is ≤ MaxDataLength. The value shall be = 256*8.
 */
#define MAX_P02_DATA_LENGTH_IN_BITS     (8*256)
#define MIN_P02_DATA_LENGTH_IN_BITS     (8*2)

#define E2E_P02_AR_RELEASE_MAJOR_VERSION_INT        4u
#define E2E_P02_AR_RELEASE_MINOR_VERSION_INT        3u
#define E2E_P02_AR_RELEASE_REVISION_VERSION_INT     0u

#define E2E_P02_SW_MAJOR_VERSION_INT        2
#define E2E_P02_SW_MINOR_VERSION_INT        0

/* @req SWS_E2E_00327
 * The implementer of the E2E Library shall avoid the integration of incompatible files. Minimum implementation is the version check of the header files.
 */
#if (E2E_P02_SW_MAJOR_VERSION != E2E_P02_SW_MAJOR_VERSION_INT) || (E2E_P02_SW_MINOR_VERSION != E2E_P02_SW_MINOR_VERSION_INT)
#error "SW Version mismatch between E2E_P02.c and E2E_P02.h"
#endif

#if (E2E_P02_AR_RELEASE_MAJOR_VERSION != E2E_P02_AR_RELEASE_MAJOR_VERSION_INT) || (E2E_P02_AR_RELEASE_MINOR_VERSION != E2E_P02_AR_RELEASE_MINOR_VERSION_INT) || \
    (E2E_P02_AR_RELEASE_REVISION_VERSION != E2E_P02_AR_RELEASE_REVISION_VERSION_INT)
#error "AR Version mismatch between E2E_P02.c and E2E_P02.h"
#endif


/* @req SWS_E2E_00135 In E2E Profile 2, the local variable DeltaCounter shall be calculated
 * by subtracting LastValidCounter from Counter in Data,
 * considering an overflow due to the range of values [0…15].
 */

/* Calculation of the delta counter value taken into account the overflow.
 */
static INLINE uint8 calculateDeltaCounterP02(uint8 receivedCounter, uint8 lastValidCounter) {
    uint8 status;

    if (receivedCounter >= lastValidCounter) {
        status = receivedCounter - lastValidCounter;
    }

    else {
        status = MAX_P02_COUNTER_VALUE + 1 + receivedCounter - lastValidCounter;
    }

    return status;
}


/* @req SWS_E2E_00047 Definition of error flags for all E2E units */
/* @req SWS_E2E_00152 Non-modifiable configuration of the data element sent over an RTE port,
 * for E2E profile 2. The position of the counter and CRC is not configurable in profile 2. */

/* Checks the correctness of the configuration parameter.
 */
static INLINE Std_ReturnType checkConfigP02(const E2E_P02ConfigType* Config) {

    /* Check for NULL pointers */
    Std_ReturnType status;
    status = E2E_E_OK;

    if (Config == NULL_PTR) {
        status = E2E_E_INPUTERR_NULL;
    }

    /* Check input parameters values */
    else if (((Config->DataLength % 8) != 0) || (Config->DataLength < MIN_P02_DATA_LENGTH_IN_BITS) ||
        (Config->DataLength > MAX_P02_DATA_LENGTH_IN_BITS)) {
        status = E2E_E_INPUTERR_WRONG;
    }

    /* Offset of the E2E header in the Data[] array in bits. It shall be: 0 ≤ Offset ≤ MaxDataLength-(2*8). */
    else if (Config->Offset > (MAX_P02_DATA_LENGTH_IN_BITS-MIN_P02_DATA_LENGTH_IN_BITS)) {
        status = E2E_E_INPUTERR_WRONG;
    }

    else {
        status = E2E_E_OK;
    }

    return status;
}


/* @req SWS_E2E_00117 E2E Profile 2 shall use the Crc_CalculateCRC8H2F() function
 * of the SWS CRC Library for calculating CRC checksums. */
/* @req SWS_E2E_00118 E2E Profile 2 shall use 0xFF as the start value CRC_StartValue8 for CRC calculation. */
/* @req SWS_E2E_00119 In E2E Profile 2, the specific Data ID used to calculate a specific CRC shall be of length 8 bit. */
/* @req SWS_E2E_00120 In E2E Profile 2, the specific Data ID used for CRC calculation shall be selected from a
 * pre-defined DataIDList[16] using the value of the Counter as an index. */
/* @req SWS_E2E_00122 In E2E Profile 2, the CRC shall be Data[0]. */
/* @req SWS_E2E_00123 In E2E Profile 2, the Counter shall be the low nibble (Bit 0…Bit 3) of Data[1]. */
/* @req SWS_E2E_00124 In E2E Profile 2, the E2E_P02Protect() function shall not modify any bit of Data
 * except the bits representing the CRC and the Counter. */
/* @req SWS_E2E_00126 In E2E Profile 2, the E2E_P02Protect() function shall perform the activities
 * as specified in Figure 7-8 and Figure 7-9 in AUTOSAR_SWS_E2ELibrary.pdf. */
/* @req SWS_E2E_00127 In E2E Profile 2, the E2E_P02Protect() function shall increment the Counter
 * of the state (E2E_P02ProtectStateType) by 1 on every transmission request from the sending SW-C,
 * i.e. on every call of E2E_P02Protect(). */
/* @req SWS_E2E_00128 In E2E Profile 2, the range of the value of the Counter shall be [0…15]. */
/* @req SWS_E2E_00129 When the Counter has reached its upper bound of 15 (0xF),
 * it shall restart at 0 for the next call of the E2E_P02Protect() from the sending SW-C. */
/* @req SWS_E2E_00130 In E2E Profile 2, the E2E_P02Protect() function shall update the Counter
 * (i.e. low nibble (Bit 0…Bit 3) of Data byte 1) in the data buffer (Data) after incrementing the Counter. */
/* @req SWS_E2E_00132 In E2E Profile 2, after determining the specific Data ID, the E2E_P02Protect() function shall
 * calculate the CRC over Data[1], Data[2], … Data[Config->DataLength/8-1] of the data buffer (Data)
 * extended with the Data ID. */
/* @req SWS_E2E_00133 In E2E Profile 2, the E2E_P02Protect() function shall update the CRC (i.e. Data[0])
 * in the data buffer (Data) after computing the CRC. */
/* @req SWS_E2E_00160 API for E2E_P02Protect */
/* @req SWS_E2E_00219 Profile 2 shall provide the following mechanisms as specified in 7.4 */

/* Protects the array/buffer to be transmitted using the E2E profile 2.
 */
Std_ReturnType E2E_P02Protect(const E2E_P02ConfigType* ConfigPtr, E2E_P02ProtectStateType* StatePtr, uint8* DataPtr) {

    Std_ReturnType status;
    status = E2E_E_OK;
    uint8 dataId;
    uint8 crc;
    Std_ReturnType returnValue = checkConfigP02(ConfigPtr);

    if (E2E_E_OK != returnValue) {
        status = returnValue;
    }

    else if ((StatePtr == NULL_PTR) || (DataPtr == NULL_PTR)) {
        status = E2E_E_INPUTERR_NULL;
    }

    else {
        /* Increase counter*/
        StatePtr->Counter = (StatePtr->Counter + 1) % (MAX_P02_COUNTER_VALUE + 1);

        /* Add counter to data */
        DataPtr[1] = (DataPtr[1] & 0xF0u) | (StatePtr->Counter & 0x0Fu);

        /* Get the data id */
        dataId = ConfigPtr->DataIDList[StatePtr->Counter];

        /* Calculate CRC over data and then data id */
        crc = Crc_CalculateCRC8H2F(&DataPtr[1],  (ConfigPtr->DataLength/8)-1, 0x00, TRUE);
        crc = Crc_CalculateCRC8H2F(&dataId,  1, crc, FALSE);

        /* Write crc to data */
        DataPtr[0] = crc;
    }

    return status;
}


/* Check if that is the first data since initialisation
 */
static INLINE boolean isFirstDataSinceInitialisationP02(const E2E_P02ConfigType* ConfigPtr, E2E_P02CheckStateType* StatePtr, uint8 receivedCounter) {

    boolean status;
    status = FALSE;

    if (StatePtr->WaitForFirstData == TRUE) {
        StatePtr->WaitForFirstData = FALSE;
        StatePtr->MaxDeltaCounter = ConfigPtr->MaxDeltaCounterInit;
        StatePtr->LastValidCounter = receivedCounter;
        StatePtr->Status= E2E_P02STATUS_INITIAL;

        status = TRUE;
    }

    return status;
}


/* Checks the Data received using the E2E profile 2.
 *  Ok, this is the normal case. Calculate delta counter, Check the counter delta */
static INLINE void doChecksP02(const E2E_P02ConfigType* ConfigPtr, E2E_P02CheckStateType* StatePtr, uint8 receivedCounter) {

    uint8 delta;

    delta = calculateDeltaCounterP02(receivedCounter, StatePtr->LastValidCounter);
//    LDEBUG_PRINTF("delta= 0x%02x", delta);
//    LDEBUG_PRINTF("\n");

    if (delta == 0) {
//        LDEBUG_PRINTF("delta == 0, %d, %d", StatePtr->NoNewOrRepeatedDataCounter, MAX_P02_COUNTER_VALUE);
//        LDEBUG_PRINTF("\n");
        if (StatePtr->NoNewOrRepeatedDataCounter < MAX_P02_COUNTER_VALUE) {
            StatePtr->NoNewOrRepeatedDataCounter++;
        }

        StatePtr->Status= E2E_P02STATUS_REPEATED;
    }

    else if (delta == 1) {
        //LDEBUG_PRINTF("deltaCounter == 1");
        //LDEBUG_PRINTF("\n");
        StatePtr->MaxDeltaCounter = ConfigPtr->MaxDeltaCounterInit;
        StatePtr->LastValidCounter = receivedCounter;
        StatePtr->LostData = 0;

        if (StatePtr->NoNewOrRepeatedDataCounter <= ConfigPtr->MaxNoNewOrRepeatedData) {
            StatePtr->NoNewOrRepeatedDataCounter = 0;

            if (StatePtr->SyncCounter > 0) {
                StatePtr->SyncCounter--;
                StatePtr->Status = E2E_P02STATUS_SYNC;
            }
            else {
                StatePtr->Status = E2E_P02STATUS_OK;
            }
         }
         else {
             StatePtr->NoNewOrRepeatedDataCounter = 0;
             StatePtr->SyncCounter = ConfigPtr->SyncCounterInit;
             StatePtr->Status = E2E_P02STATUS_SYNC;
         }
    }

    else if (delta > StatePtr->MaxDeltaCounter) {
//        LDEBUG_PRINTF("deltaCounter > StatePtr->MaxDeltaCounter, %d, %d", StatePtr->SyncCounter, ConfigPtr->SyncCounterInit);
//        LDEBUG_PRINTF("\n");

        StatePtr->NoNewOrRepeatedDataCounter= 0;
        StatePtr->SyncCounter = ConfigPtr->SyncCounterInit;

        if (StatePtr->SyncCounter > 0) {
//            LDEBUG_PRINTF("StatePtr->SyncCounter > 0");
//            LDEBUG_PRINTF("\n");
            StatePtr->MaxDeltaCounter = ConfigPtr->MaxDeltaCounterInit;
            StatePtr->LastValidCounter = receivedCounter;
        }

        StatePtr->Status= E2E_P02STATUS_WRONGSEQUENCE;
    }  // delta > MaxDeltaCounter

    else {
//        LDEBUG_PRINTF("delta > MaxDeltaCounter, %d, %d", StatePtr->NoNewOrRepeatedDataCounter, ConfigPtr->MaxNoNewOrRepeatedData);
//        LDEBUG_PRINTF("\n");
        StatePtr->MaxDeltaCounter = ConfigPtr->MaxDeltaCounterInit;
        StatePtr->LastValidCounter = receivedCounter;
        StatePtr->LostData = delta - 1;

        if (StatePtr->NoNewOrRepeatedDataCounter <= ConfigPtr->MaxNoNewOrRepeatedData) {
            StatePtr->NoNewOrRepeatedDataCounter= 0;

//            LDEBUG_PRINTF("StatePtr->NoNewOrRepeatedDataCounter %d", StatePtr->NoNewOrRepeatedDataCounter);
//            LDEBUG_PRINTF("\n");

            if (StatePtr->SyncCounter > 0) {
//                LDEBUG_PRINTF("StatePtr->SyncCounter > 0");
//                LDEBUG_PRINTF("\n");

                StatePtr->SyncCounter--;
                StatePtr->Status = E2E_P02STATUS_SYNC;
            }

            else {
                StatePtr->Status = E2E_P02STATUS_OKSOMELOST;
            }
        }

        else {
            StatePtr->NoNewOrRepeatedDataCounter= 0;
            StatePtr->SyncCounter = ConfigPtr->SyncCounterInit;
            StatePtr->Status = E2E_P02STATUS_SYNC;
        }
    } // 1 < delta <= MaxDeltaCounter
}


/* @req SWS_E2E_00118 E2E Profile 2 shall use 0xFF as the start value CRC_StartValue8 for CRC calculation. */
/* @req SWS_E2E_00119 In E2E Profile 2, the specific Data ID used to calculate a specific CRC shall be of length 8 bit. */
/* @req SWS_E2E_00120 In E2E Profile 2, the specific Data ID used for CRC calculation shall be selected from a
 * pre-defined DataIDList[16] using the value of the Counter as an index. */
/* @req SWS_E2E_00122 In E2E Profile 2, the CRC shall be Data[0]. */
/* @req SWS_E2E_00123 In E2E Profile 2, the Counter shall be the low nibble (Bit 0…Bit 3) of Data[1]. */
/* @req SWS_E2E_00125 In E2E Profile 2, the E2E_P02Check() function shall not modify any bit in Data. */
/* @req SWS_E2E_00134 In E2E Profile 2, the E2E_P02Check() function shall perform the activities as specified
 * in Figure 7-10, Figure 7-11and Figure 7-12 of AUTOSAR_SWS_E2ELibrary. */
/* @req SWS_E2E_00136 In E2E Profile 2, MaxDeltaCounter shall specify the maximum allowed difference
 * between two Counter values of two consecutively received valid messages. */
/* @req SWS_E2E_00137 In E2E Profile 2, MaxDeltaCounter shall be incremented by 1 every time the E2E_P02Check()
 * function is called, up to the maximum value of 15 (0xF). */
/* @req SWS_E2E_00138 In E2E Profile 2, the E2E_P02Check() function shall set Status to E2E_P02STATUS_NONEWDATA
 * if the attribute NewDataAvailable is FALSE. */
/* @req SWS_E2E_00139 In E2E Profile 2, the E2E_P02Check() function shall determine the specific Data ID
 * from DataIDList using the Counter of the received Data as index. */
/* @req SWS_E2E_00140 In E2E Profile 2, the E2E_P02Check() function shall calculate CalculatedCRC over
 * Data[1], Data[2], … Data[Config->DataLength/8-1] of the data buffer (Data) extended with the determined Data ID. */
/* @req SWS_E2E_00141 In E2E Profile 2, the E2E_P02Check() function shall set Status to E2E_P02STATUS_WRONGCRC
 * if the calculated CalculatedCRC value differs from the value of the CRC in Data. */
/* @req SWS_E2E_00142 In E2E Profile 2, the E2E_P02Check() function shall set Status to E2E_P02STATUS_INITIAL
 * if the flag WaitForFirstData is TRUE. */
/* @req SWS_E2E_00143 In E2E Profile 2, the E2E_P02Check() function shall clear the flag WaitForFirstData
 * if it returns with Status E2E_P02STATUS_INITIAL. */
/* @req SWS_E2E_00145 The E2E_P02Check() function shall
 *      * set Status to E2E_P02STATUS_WRONGSEQUENCE; and
 *      * re-initialize SyncCounter with SyncCounterInit
 * if the calculated value of DeltaCounter exceeds the value of MaxDeltaCounter. */
/* @req SWS_E2E_00146 The E2E_P02Check() function shall set Status to E2E_P02STATUS_REPEATED
 * if the calculated DeltaCounter equals 0. */
/* @req SWS_E2E_00147 The E2E_P02Check() function shall set Status to E2E_P02STATUS_OK
 * if the following conditions are true:
 *      * the calculated DeltaCounter equals 1; and
 *      * the value of the NoNewOrRepeatedDataCounter is less than or equal to MaxNoNewOrRepeatedData
 *        (i.e. State -> NoNewOrRepeatedDataCounter ≤ Config -> MaxNoNewOrRepeatedData); and
 *      * the SyncCounter equals 0.*/
/* @req SWS_E2E_00148 The E2E_P02Check() function shall set Status to E2E_P02STATUS_OKSOMELOST
 * if the following conditions are true: compare AUTOSAR_SWS_E2ELibrary.pdf. */
/* @req SWS_E2E_00149 The E2E_P02Check() function shall set the value LostData to (DeltaCounter – 1)
 * if the calculated DeltaCounter is greater-than 1 but less-than or equal to MaxDeltaCounter. */
/* @req SWS_E2E_00150 The E2E_P02Check() function shall re-initialize MaxDeltaCounter with MaxDeltaCounterInit
 * if it returns one of the following Status */
/* @req SWS_E2E_00151 The E2E_P02Check() function shall set LastValidCounter to Counter of Data
 * if it returns one of the following Status */
/* @req SWS_E2E_00161 API for E2E_P02Check */
/* @req SWS_E2E_00219 Profile 2 shall provide the following mechanisms as specified in 7.4 */
/* @req SWS_E2E_00298 The E2E_P02Check() function shall
 *      *re-initialize SyncCounter with SyncCounterInit; and
 *      * set Status to E2E_P02STATUS_SYNC;
 * if the following conditions are true: */
/* @req SWS_E2E_00299 The E2E_P02Check() function shall
 *      * decrement SyncCounter by 1; and
 *      * set Status to E2E_P02STATUS_SYNC
 * if the following conditions are true: */
/* @req SWS_E2E_00300 The E2E_P02Check() function shall reset the NoNewOrRepeatedDataCounter to 0
 * if it returns one of the following status: */
/* @req SWS_E2E_00301 The E2E_P02Check() function shall increment NoNewOrRepeatedDataCounter by 1
 * if it returns the Status E2E_P02STATUS_NONEWDATA or E2E_P02STATUS_REPEATED
 * up to the maximum value of Counter (i.e. 15 or 0xF). */

/* Checks the Data received using the E2E profile 2.
 */
Std_ReturnType E2E_P02Check(const E2E_P02ConfigType* ConfigPtr, E2E_P02CheckStateType* StatePtr, const uint8* DataPtr) {

    Std_ReturnType status;
    status = E2E_E_OK;
    boolean ret;
    ret = TRUE;
    uint8 receivedCounter;
    uint8 dataId;
    uint8 crc;

    Std_ReturnType returnValue = checkConfigP02(ConfigPtr);
    if (E2E_E_OK != returnValue) {
        status = returnValue;
    }

    else if ((StatePtr == NULL_PTR) || (DataPtr == NULL_PTR)) {
            status = E2E_E_INPUTERR_NULL;
    }

    else {

        /* This is the stated behavior in figure 7-10 but it seems wrong. This means that if more than 15
         * messages are received with wrong CRC, the first message with correct CRC will be have state
         * OKSOMELOST even though it should be WRONGSEQUENCE.
         */
        if (StatePtr->MaxDeltaCounter < MAX_P02_COUNTER_VALUE) {
            StatePtr->MaxDeltaCounter++;
        }

        if (StatePtr->NewDataAvailable == FALSE) {

            if (StatePtr->NoNewOrRepeatedDataCounter < MAX_P02_COUNTER_VALUE) {
                StatePtr->NoNewOrRepeatedDataCounter++;
            }

            StatePtr->Status = E2E_P02STATUS_NONEWDATA;
            status = E2E_E_OK;
            ret = FALSE;
        }
        if (ret == TRUE) {
            receivedCounter = DataPtr[1] & 0xFu;

            dataId = ConfigPtr->DataIDList[receivedCounter];

            /* Calculate CRC over data and then data id */
            crc = Crc_CalculateCRC8H2F(&DataPtr[1],  (ConfigPtr->DataLength/8)-1, 0x00, TRUE);
            crc = Crc_CalculateCRC8H2F(&dataId,  1, crc, FALSE);

            /* Compare calculate and received crc */
            if (crc != DataPtr[0]) {
                StatePtr->Status = E2E_P02STATUS_WRONGCRC;
                status = E2E_E_OK;
            }

            /* Check if this is the first data since initialization */
            else if (isFirstDataSinceInitialisationP02(ConfigPtr, StatePtr, receivedCounter) == TRUE) {
                status = E2E_E_OK;

            } else {

                doChecksP02(ConfigPtr, StatePtr, receivedCounter);
            }
        }
    }

    return status;
}


/* @req SWS_E2E_00387 API for E2E_P02ProtectInit */
/* @req SWS_E2E_00388 In case State is NULL, E2E_P02ProtectInit shall return immediately with E2E_E_INPUTERR_NULL.
 * Otherwise, it shall intialize the state structure, setting Counter to 0. */

/* Initializes the protection state.
 */
Std_ReturnType E2E_P02ProtectInit(E2E_P02ProtectStateType* StatePtr) {

    Std_ReturnType status;
    status = E2E_E_OK;

    if (StatePtr == NULL_PTR) {
        status = E2E_E_INPUTERR_NULL;
    } else {
        StatePtr->Counter = 0;
    }

    return status;
}


/* @req SWS_E2E_00391 API for E2E_P02CheckInit */
/* @req SWS_E2E_00392 In case State is NULL, E2E_P02CheckInit shall return immediately with E2E_E_INPUTERR_NULL.
 * Otherwise, it shall initialize the state structure, setting: */
/* UC_E2E_00208, UC_E2E_00297, SWS_E2E_00324, SWS_E2E_00325 overwrites init values NewDataAvailable */
/* @req ARC_SWS_E2E_00001 */
/* Initializes the check state.
 */
Std_ReturnType E2E_P02CheckInit(E2E_P02CheckStateType* StatePtr) {

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
        StatePtr->Status = E2E_P02STATUS_NONEWDATA;
        StatePtr->SyncCounter = 0;
        StatePtr->NoNewOrRepeatedDataCounter = 0;
    }

    return status;
}


/* @req SWS_E2E_00380 If CheckReturn == E2E_E_OK and ProfileBehavior == 1,
 * then the function E2E_P02MapStatusToSM shall return the values depending on the value of Status: */

/* The function maps the check status of Profile 2 to a generic check status in case ProfileBehavior == True
 */
static INLINE E2E_PCheckStatusType doE2E_P02MapStatusToSM_TRUE(E2E_P02CheckStatusType Status) {

    E2E_PCheckStatusType retValue = E2E_P_ERROR;

    switch(Status) {
        case E2E_P02STATUS_OK:
        case E2E_P02STATUS_OKSOMELOST:
        case E2E_P02STATUS_SYNC:
            retValue = E2E_P_OK;
            break;
        case E2E_P02STATUS_REPEATED:
            retValue = E2E_P_REPEATED;
            break;
        case E2E_P02STATUS_NONEWDATA:
            retValue = E2E_P_NONEWDATA;
            break;
        case E2E_P02STATUS_WRONGSEQUENCE:
        case E2E_P02STATUS_INITIAL:
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


/* @req SWS_E2E_00477 If CheckReturn == E2E_E_OK and ProfileBehavior == 0,
 * then the function E2E_P02MapStatusToSM shall return the values depending on the value of Status: */

/* The function maps the check status of Profile 2 to a generic check status in case ProfileBehavior == False
 */
static INLINE E2E_PCheckStatusType doE2E_P02MapStatusToSM_FALSE(E2E_P02CheckStatusType Status) {

    E2E_PCheckStatusType retValue = E2E_P_ERROR;

    switch(Status) {
        case E2E_P02STATUS_OK:
        case E2E_P02STATUS_OKSOMELOST:
        case E2E_P02STATUS_INITIAL:
            retValue = E2E_P_OK;
            break;
        case E2E_P02STATUS_REPEATED:
            retValue = E2E_P_REPEATED;
            break;
        case E2E_P02STATUS_NONEWDATA:
            retValue = E2E_P_NONEWDATA;
            break;
        case E2E_P02STATUS_WRONGSEQUENCE:
        case E2E_P02STATUS_SYNC:
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


/* @req SWS_E2E_00379 API for E2E_P02MapStatusToSM */
/* @req SWS_E2E_00381 If CheckReturn != E2E_E_OK, then the function E2E_P02MapStatusToSM()
 * shall return E2E_P_ERROR (regardless of value of Status). */

/* The function maps the check status of Profile 2 to a generic check status
 */
E2E_PCheckStatusType E2E_P02MapStatusToSM(Std_ReturnType CheckReturn, E2E_P02CheckStatusType Status, boolean profileBehavior) {

    E2E_PCheckStatusType retValue = E2E_P_ERROR;

    if (CheckReturn != E2E_E_OK) {
        retValue = E2E_P_ERROR;
    }

    else if (profileBehavior == TRUE) { //ASR4.2.2 or higher behavior

        retValue = doE2E_P02MapStatusToSM_TRUE(Status);
    }
    else { //ASR4.2.2 pre-behavior

        retValue = doE2E_P02MapStatusToSM_FALSE(Status);
    }

    return retValue;
}
