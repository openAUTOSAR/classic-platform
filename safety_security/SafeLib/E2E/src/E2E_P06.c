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

/* @req SWS_E2E_00215 Files E2E_P06.c and E2E_P06.h shall contain implementation parts specific of each profile. */
/* @req SWS_E2E_00049 The E2E library shall not contain library-internal mechanisms for error detection to be traced as development errors. */
/* @req SWS_E2E_00221 Each E2E Profile shall use a subset of the data protection mechanisms defined in AUTOSAR_SWS_E2ELibrary.pdf, 7.2. */

/* @req SWS_E2E_00011 The E2E Library shall report errors detected by library-internal mechanisms
 * to callers of E2E functions through return value.
 */
/* @req SWS_E2E_00012 The internal library mechanisms shall detect and report errors
 * shall be implemented according to the pre-defined E2E Profiles specified in sections 7.3 and 7.4.
 */

#include "E2E_P06.h"


/* @req SWS_E2E_00443 Each time E2E_P06Protect() is called, it increments the counter up to 0xFF. */
#define MAX_P06_COUNTER_VALUE           (255)
/* @req SWS_E2E_00441 Maximal length of Data, in bits. E2E checks that DataLength is ≤ MaxDataLength.
 * The value shall be = 4096*8 (4kB). MaxDataLength shall be ≥ MinDataLength. The value shall be ≥ 5*8.
 */
#define MAX_P06_DATA_LENGTH_IN_BITS     (8*4096)
#define MIN_P06_DATA_LENGTH_IN_BITS     (8*5)

#define E2E_P06_AR_RELEASE_MAJOR_VERSION_INT        4u
#define E2E_P06_AR_RELEASE_MINOR_VERSION_INT        3u
#define E2E_P06_AR_RELEASE_REVISION_VERSION_INT     0u

#define E2E_P06_SW_MAJOR_VERSION_INT        2
#define E2E_P06_SW_MINOR_VERSION_INT        0

/* @req SWS_E2E_00327
 * The implementer of the E2E Library shall avoid the integration of incompatible files. Minimum implementation is the version check of the header files.
 */
#if (E2E_P06_SW_MAJOR_VERSION != E2E_P06_SW_MAJOR_VERSION_INT) || (E2E_P06_SW_MINOR_VERSION != E2E_P06_SW_MINOR_VERSION_INT)
#error "SW Version mismatch between E2E_P06.c and E2E_P06.h"
#endif

#if (E2E_P06_AR_RELEASE_MAJOR_VERSION != E2E_P06_AR_RELEASE_MAJOR_VERSION_INT) || (E2E_P06_AR_RELEASE_MINOR_VERSION != E2E_P06_AR_RELEASE_MINOR_VERSION_INT) || \
    (E2E_P06_AR_RELEASE_REVISION_VERSION != E2E_P06_AR_RELEASE_REVISION_VERSION_INT)
#error "AR Version mismatch between E2E_P06.c and E2E_P06.h"
#endif


/* @req SWS_E2E_00047 Definition of error flags for all E2E units */
/* @req SWS_E2E_00424 The step “Verify inputs of the protect function” in E2E_P06Protect() shall have the following behavior: */
/* @req SWS_E2E_00441 Checks Min and Max Length */

/* Verifies the input of the function Protect Input for the Profile 06.
 */
static INLINE Std_ReturnType verifyProtectInputsP06(const E2E_P06ConfigType* ConfigPtr, const E2E_P06ProtectStateType* StatePtr, const uint8* DataPtr, uint16 Length) {

    Std_ReturnType status;
    status = E2E_E_OK;
    /* Check for NULL pointers */
    if ((ConfigPtr == NULL_PTR) || (StatePtr == NULL_PTR) || (DataPtr == NULL_PTR)) {
        status = E2E_E_INPUTERR_NULL;
    }

    /* Check input parameters values */
    else if ((ConfigPtr->MinDataLength < MIN_P06_DATA_LENGTH_IN_BITS) || (ConfigPtr->MaxDataLength < MIN_P06_DATA_LENGTH_IN_BITS) ||
        (ConfigPtr->MinDataLength > MAX_P06_DATA_LENGTH_IN_BITS) || (ConfigPtr->MaxDataLength > MAX_P06_DATA_LENGTH_IN_BITS) ||
        (ConfigPtr->MinDataLength > ConfigPtr->MaxDataLength)) {
        status = E2E_E_INPUTERR_WRONG;
    }
    /* Transmitted length shall be MinDataLength ≤ Length ≤ MaxDataLength */
    else if ( ((8*Length) < ConfigPtr->MinDataLength) || ((8*Length) > ConfigPtr->MaxDataLength) ) {
        status = E2E_E_INPUTERR_WRONG;
    } 
    /* The offset shall be a multiple of 8 and 0 ≤ Offset ≤ (Max)DataLength-(5*8). Compared to transmitted length as E2E header has to fit data */
    else if ( ((ConfigPtr->Offset % 8) != 0) || (ConfigPtr->Offset > ((8*Length)-MIN_P06_DATA_LENGTH_IN_BITS))) {
        status = E2E_E_INPUTERR_WRONG;
    }

    else {
        status = E2E_E_OK;
    }

    return status;
}


/* Changes the Bit Endianness of a byte value
 */
static INLINE uint8 changeBitEndiannessP06(uint8 value)
{
    uint8 result = 0;

    result |= (value & 0x01u) << 7;
    result |= (value & 0x02u) << 5;
    result |= (value & 0x04u) << 3;
    result |= (value & 0x08u) << 1;
    result |= (value & 0x10u) >> 1;
    result |= (value & 0x20u) >> 3;
    result |= (value & 0x40u) >> 5;
    result |= (value & 0x80u) >> 7;

    return result;
}


/* @req SWS_E2E_00470 The step “Compute offset” in E2E_P06Protect() and
 * E2E_P06Check() shall have the following behavior: */

/* Return local variable uint16 Offset, which is in [byte].
 */
static INLINE uint16 computeOffsetP06(const E2E_P06ConfigType* ConfigPtr) {

    return (ConfigPtr->Offset/8);
}


/* @req SWS_E2E_00425 The step “Write Length” in E2E_P06Protect()
 * shall have the following behavior: */

/* Writes the Length value to the right position within the data byte stream.
 */
static INLINE void writeLengthP06(uint8* DataPtr, uint16 offset, uint16 Length) {

    DataPtr[offset+2] = changeBitEndiannessP06(((Length & 0xFF00u) >> 8) & 0xFFu);
    DataPtr[offset+3] = changeBitEndiannessP06( (Length & 0x00FFu)       & 0xFFu);
}


/* @req SWS_E2E_00426 The step “Write Counter” in E2E_P06Protect()
 * shall have the following behavior: */

/* Writes the Counter value to the right position within the data byte stream.
 */
static INLINE void writeCounterP06(uint8* DataPtr, uint16 offset, const E2E_P06ProtectStateType* StatePtr) {

    DataPtr[offset+4] = changeBitEndiannessP06( (StatePtr->Counter & 0xFFu) & 0xFFu);
}


/* @req SWS_E2E_00427 The step “Compute CRC” in E2E_P06Protect() and E2E_P06Check()
 *  shall have the following behavior: */
/* @req SWS_E2E_00419 In the E2E Profile 6, the Data ID shall be implicitly transmitted,
 * by adding the Data ID after the user data in the CRC calculation. */
/* @req SWS_E2E_00420 E2E Profile 6 shall use the Crc_CalculateCRC16() function of the
 * SWS CRC Library for calculating the CRC (Polynomial: 0x1021; Autosar notation). */
/* @req SWS_E2E_00421 In E2E Profile 6, the CRC shall be calculated over the entire E2E header
 * (excluding the CRC bytes), including the user data extended with the Data ID. */
/* @req SWS_E2E_00479 Profile 6 shall provide the control fields as defined in AUTOSAR_SWS_E2ELibrary, 7.7,
 * transmitted at runtime together with the protected data*/

/* Returns the CRC value for the Protect and Check function of the Profile 06.
 */
static INLINE uint16 computeCRCP06(const uint8* DataPtr, uint16 offset, uint16 dataId, uint16 Length) {

    uint16 lengthBytes = (Length-offset)-2;
    uint16 crc;
    uint8 dataIdNibble;

    if (offset > 0) {
        /* compute CRC over bytes that are before CRC */
        crc = Crc_CalculateCRC16(&DataPtr[0], offset, 0xFFFFu, TRUE);

        /* Compute CRC over bytes that are after CRC (if any) */
        crc = Crc_CalculateCRC16(&DataPtr[offset+2], lengthBytes, crc, FALSE);
    }
    else {

        /* Compute CRC over bytes that are after CRC (if any) */
        crc = Crc_CalculateCRC16(&DataPtr[offset+2], lengthBytes, 0xFFFFu, TRUE);
    }

    dataIdNibble = ((dataId>>8) & 0xFFu);
    crc = Crc_CalculateCRC16(&dataIdNibble, 1, crc, FALSE);

    dataIdNibble = ((dataId) & 0xFFu);
    crc = Crc_CalculateCRC16(&dataIdNibble, 1, crc, FALSE);

    return crc;
}


/* @req SWS_E2E_00428 The step “Write CRC” in E2E_P06Protect()
 * shall have the following behavior: */

/* Writes the CRC value to the right position within the data byte stream.
 */
static INLINE void writeCRCP06(uint8* DataPtr, uint16 offset, uint16 crc) {

    DataPtr[offset  ] = changeBitEndiannessP06(((crc & 0xFF00u) >>  8) & 0xFFu);
    DataPtr[offset+1] = changeBitEndiannessP06(((crc & 0x00FFu)      ) & 0xFFu);
}


/* @req SWS_E2E_00417 In E2E Profile 6, on the sender side, for the first transmission request
 * of a data element the counter shall be initialized with 0 and shall be incremented by 1
 * for every subsequent send request. When the counter reaches the maximum value (0xFF),
 * then it shall restart with 0 for the next send request.
 */
/* @req SWS_E2E_00429 The step “Increment Counter” in E2E_P06Protect()
 * shall have the following behavior: */

/* Increments the Counter Value on the sender side.
 */
static INLINE void incrementCounterP06(E2E_P06ProtectStateType* StatePtr) {

    StatePtr->Counter = (StatePtr->Counter + 1) % (MAX_P06_COUNTER_VALUE + 1);
}


/* @req SWS_E2E_00423 The function E2E_P06Protect() shall have the following overall behavior: */
/* @req SWS_E2E_00393 API for E2E_P06Protect */

/* Protects the array/buffer to be transmitted using the E2E profile 6.
 * This includes checksum calculation, handling of counter.
 */
Std_ReturnType E2E_P06Protect(const E2E_P06ConfigType* ConfigPtr, E2E_P06ProtectStateType* StatePtr, uint8* DataPtr, uint16 Length) {

    uint16 offset;
    uint16 crc;

    Std_ReturnType returnValue = verifyProtectInputsP06(ConfigPtr, StatePtr, DataPtr, Length);

    if (E2E_E_OK == returnValue) {
        offset = computeOffsetP06(ConfigPtr);

        writeLengthP06(DataPtr, offset, Length);

        writeCounterP06(DataPtr, offset, StatePtr);

        crc = computeCRCP06(DataPtr, offset, ConfigPtr->DataID, Length);

        writeCRCP06(DataPtr, offset, crc);

        incrementCounterP06(StatePtr);
    }

    return returnValue;
}


/* @req SWS_E2E_00047 Definition of error flags for all E2E units */
/* @req SWS_E2E_00431 The step “Verify inputs of the check function” in E2E_P06Check()
 * shall have the following behavior: */

/* Verifies the input of the function Check Input for the Profile 06.
 */
static INLINE Std_ReturnType verifyCheckInputsP06(boolean* NewDataAvailable, const E2E_P06ConfigType* ConfigPtr, const E2E_P06CheckStateType* StatePtr, const uint8* DataPtr, uint16 Length) {

    Std_ReturnType status;
    status = E2E_E_OK;
    *NewDataAvailable = FALSE;

    /* Check for NULL pointers */
    if ((ConfigPtr == NULL_PTR) || (StatePtr == NULL_PTR)) {
        status = E2E_E_INPUTERR_NULL;
    }

    /* Check input parameters values */
    else if ((ConfigPtr->MinDataLength < MIN_P06_DATA_LENGTH_IN_BITS) || (ConfigPtr->MaxDataLength < MIN_P06_DATA_LENGTH_IN_BITS) ||
        (ConfigPtr->MinDataLength > MAX_P06_DATA_LENGTH_IN_BITS) || (ConfigPtr->MaxDataLength > MAX_P06_DATA_LENGTH_IN_BITS) ||
        (ConfigPtr->MinDataLength > ConfigPtr->MaxDataLength)) {
        status = E2E_E_INPUTERR_WRONG;
    }

    else if ( !(((DataPtr != NULL_PTR) && (Length != 0)) || ((DataPtr == NULL_PTR) && (Length == 0))) ) {
        status = E2E_E_INPUTERR_WRONG;
    }

    else if (DataPtr == NULL_PTR) {
        status = E2E_E_OK;
    }
    /* Received length shall be MinDataLength ≤ Length ≤ MaxDataLength */
    else if ( ((8*Length) < ConfigPtr->MinDataLength) || ((8*Length) > ConfigPtr->MaxDataLength) ) {
        status = E2E_E_INPUTERR_WRONG;
    } 
    /* The offset shall be a multiple of 8 and 0 ≤ Offset ≤ (Max)DataLength-(5*8). Compared to received length as E2E header has to fit data */
    else if ( ((ConfigPtr->Offset % 8) != 0) || (ConfigPtr->Offset > ((8*Length)-MIN_P06_DATA_LENGTH_IN_BITS))) {
        status = E2E_E_INPUTERR_WRONG;
    }
    else {
        *NewDataAvailable = TRUE;
    }

    return status;
}


/* @req SWS_E2E_00432 The step “Read Length” in E2E_P06Check()
 * shall have the following behavior: */

/* Reads the Length value from the right position within the data byte stream.
 */
static INLINE uint16 readLengthP06(const uint8* DataPtr, uint16 offset) {

    uint16 receivedLength = 0;

    receivedLength |= (((uint16)changeBitEndiannessP06(DataPtr[offset+2]) << 8) & 0xFF00u);
    receivedLength |= (((uint16)changeBitEndiannessP06(DataPtr[offset+3])     ) & 0x00FFu);

    return receivedLength;
}


/* @req SWS_E2E_00433 The step “Read Counter” in E2E_P06Check()
 * shall have the following behavior: */

/* Reads the Counter value from the right position within the data byte stream.
 */
static INLINE uint8 readCounterP06(const uint8* DataPtr, uint16 offset) {

    uint8 receivedCounter = 0;

    receivedCounter |= ((changeBitEndiannessP06(DataPtr[offset+4])     ) & 0xFFu);

    return receivedCounter;
}


/* @req SWS_E2E_00434 The step “Read CRC” in E2E_P06Check()
 * shall have the following behavior: */

/* Reads the CRC value from the right position within the data byte stream.
 */
static INLINE uint16 readCRCP06(const uint8* DataPtr, uint16 offset) {

    uint16 receivedCRC = 0;

    receivedCRC |= (((uint16)changeBitEndiannessP06(DataPtr[offset  ]) <<  8) & 0xFF00u);
    receivedCRC |= (((uint16)changeBitEndiannessP06(DataPtr[offset+1])      ) & 0x00FFu);

    return receivedCRC;
}


/* Calculates the delta counter value out of received and last valid counter values.
 */
static INLINE uint8 calculateDeltaCounterP06(uint8 receivedCounter, uint8 lastValidCounter)
{
    uint8 status;

    if (receivedCounter >= lastValidCounter) {
        status = receivedCounter - lastValidCounter;
    }
    else {
        status = (uint8)((uint32)MAX_P06_COUNTER_VALUE + 1 + (uint32)receivedCounter - (uint32)lastValidCounter);
    }

    return status;
}


/* @req SWS_E2E_00436 The step “Do Checks” in E2E_P06Check() shall have the following behavior: */

/* Do checks to determine the next status value of the receiver.
 */
static INLINE void doChecksP06(E2E_P06CheckStateType* StatePtr, const boolean* NewDataAvailable, const E2E_P06ConfigType* ConfigPtr, uint16 receivedCRC, uint16 computedCRC, uint16 receivedLength, uint16 Length, uint8 receivedCounter) {

    uint8 deltaCounter;

    if (*NewDataAvailable == FALSE) {
        StatePtr->Status = E2E_P06STATUS_NONEWDATA;
    }

    else if ( (receivedCRC != computedCRC) || (receivedLength != Length)) {
        StatePtr->Status = E2E_P06STATUS_ERROR;
    }

    else {
        deltaCounter = calculateDeltaCounterP06(receivedCounter, StatePtr->Counter);

        if (deltaCounter > ConfigPtr->MaxDeltaCounter) {
            StatePtr->Status = E2E_P06STATUS_WRONGSEQUENCE;
        }
        else if (deltaCounter == 0) {
            StatePtr->Status = E2E_P06STATUS_REPEATED;
        }
        else if (deltaCounter == 1) {
            StatePtr->Status = E2E_P06STATUS_OK;
        }
        else {
            StatePtr->Status = E2E_P06STATUS_OKSOMELOST;
        }

        StatePtr->Counter = receivedCounter;
    }
}


/* @req SWS_E2E_00430 The function E2E_P06Check()
 * shall have the following overall behavior: */
/* @req SWS_E2E_00445 Definition of E2E_P06CheckStatusType */
/* @req SWS_E2E_00457 API for E2E_P06Check */

/* Checks the Data received using the E2E profile 6. This includes CRC calculation, handling of Counter.
 * The function checks only one single data in one cycle,
 * it does not determine/compute the accumulated state of the communication link.
 */
Std_ReturnType E2E_P06Check(const E2E_P06ConfigType* ConfigPtr, E2E_P06CheckStateType* StatePtr, const uint8* DataPtr, uint16 Length) {

    boolean NewDataAvailable;
    uint16 offset;
    uint16 receivedLength = 0;
    uint8 receivedCounter = 0;
    uint16 receivedCRC = 0;
    uint16 computedCRC = 0;

    Std_ReturnType returnValue = verifyCheckInputsP06(&NewDataAvailable, ConfigPtr, StatePtr, DataPtr, Length);

    if (E2E_E_OK == returnValue) {
        if (NewDataAvailable == TRUE) {

            offset = computeOffsetP06(ConfigPtr);

            receivedLength = readLengthP06(DataPtr, offset);

            receivedCounter = readCounterP06(DataPtr, offset);

            receivedCRC = readCRCP06(DataPtr, offset);

            computedCRC = computeCRCP06(DataPtr, offset, ConfigPtr->DataID, Length);
        }

        doChecksP06(StatePtr, &NewDataAvailable, ConfigPtr, receivedCRC, computedCRC, receivedLength, Length, receivedCounter);
    }

    return returnValue;
}

/* @req SWS_E2E_00443 Definition of E2E_P06ProtectStateType */
/* @req SWS_E2E_00455 API for E2E_P06ProtectInit */
/* @req SWS_E2E_00456  In case State is NULL, E2E_P06ProtectInit shall return immediately with E2E_E_INPUTERR_NULL.
 * Otherwise, it shall intialize the state structure, setting Counter to 0. */

/* Initializes the protection state.
 */
Std_ReturnType E2E_P06ProtectInit(E2E_P06ProtectStateType* StatePtr) {

    Std_ReturnType status;
    status = E2E_E_OK;

    if (StatePtr == NULL_PTR) {
        status = E2E_E_INPUTERR_NULL;
    }
    else {
        StatePtr->Counter = 0;
    }

    return status;
}


/* @req SWS_E2E_00444 Definition of the State of the reception on one single Data protected with E2E Profile 6. */
/* @req SWS_E2E_00458 API for E2E_P06CheckInit */
/* @req SWS_E2E_00459 In case State is NULL, E2E_P06CheckInit shall return immediately with E2E_E_INPUTERR_NULL.
 * Otherwise, it shall initialize the state structure, setting:
 *      1. Counter to 0xFF
 *      2. Status to E2E_P06STATUS_ERROR.
 */

/* Initializes the check state.
 */
Std_ReturnType E2E_P06CheckInit(E2E_P06CheckStateType* StatePtr) {

    Std_ReturnType status;
    status = E2E_E_OK;

    if (StatePtr == NULL_PTR) {
        status = E2E_E_INPUTERR_NULL;
    }
    else {
        StatePtr->Status = E2E_P06STATUS_ERROR;
        StatePtr->Counter = 0xFF;
    }

    return status;
}


/* @req SWS_E2E_00460 API for E2E_P06MapStatusToSM */
/* @req SWS_E2E_00461 If CheckReturn = E2E_E_OK, then the function E2E_P06MapStatusToSM shall
 * return the values depending on the value of Status: */
/* @req SWS_E2E_00462 If CheckReturn != E2E_E_OK, then the function E2E_P06MapStatusToSM() shall
 * return E2E_P_ERROR (regardless of value of Status). */

/* The function maps the check status of Profile 6 to a generic check status,
 * which can be used by E2E state machine check function. The E2E Profile 6 delivers a more fine-granular status,
 * but this is not relevant for the E2E state machine.
 */
E2E_PCheckStatusType E2E_P06MapStatusToSM(Std_ReturnType CheckReturn, E2E_P06CheckStatusType Status) {

    E2E_PCheckStatusType retValue = E2E_P_ERROR;

    if (CheckReturn == E2E_E_OK) {
        switch( Status ) {
            case E2E_P06STATUS_OK:
            case E2E_P06STATUS_OKSOMELOST:
                retValue = E2E_P_OK;
                break;
            case E2E_P06STATUS_REPEATED:
                retValue = E2E_P_REPEATED;
                break;
            case E2E_P06STATUS_NONEWDATA:
                retValue = E2E_P_NONEWDATA;
                break;
            case E2E_P06STATUS_WRONGSEQUENCE:
                retValue = E2E_P_WRONGSEQUENCE;
                break;
            /* @CODECOV:DEFAULT_CASE:Default statement is required for defensive programming. If this happens the state will be restored to E2E_P_ERROR */
            __CODE_COVERAGE_IGNORE__
            default:
                retValue = E2E_P_ERROR;
                break;
        }
    }

    return retValue;
}
