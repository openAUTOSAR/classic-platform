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

/* @req SWS_E2E_00215 Files E2E_P04.c and E2E_P04.h shall contain implementation parts specific of each profile. */
/* @req SWS_E2E_00049 The E2E library shall not contain library-internal mechanisms for error detection to be traced as development errors. */
/* @req SWS_E2E_00221 Each E2E Profile shall use a subset of the data protection mechanisms defined in AUTOSAR_SWS_E2ELibrary.pdf, 7.2. */

/* @req SWS_E2E_00011 The E2E Library shall report errors detected by library-internal mechanisms
 * to callers of E2E functions through return value.
 */
/* @req SWS_E2E_00012 The internal library mechanisms shall detect and report errors
 * shall be implemented according to the pre-defined E2E Profiles specified in sections 7.3 and 7.4.
 */

#include "E2E_P04.h"

/* @req SWS_E2E_00478 Counter maximum value (0xFFFF) */
#define MAX_P04_COUNTER_VALUE           (65535)
/* @req SWS_E2E_00334 Length of Data, in bits.
 * E2E checks that Length is ≥ MinDataLength. The value shall be ≥ 12*8.
 * E2E checks that DataLength is ≤ MaxDataLength. The value shall be = 4096*8 (4kB) */
#define MAX_P04_DATA_LENGTH_IN_BITS     (8*4096)
#define MIN_P04_DATA_LENGTH_IN_BITS     (8*12)

#define E2E_P04_AR_RELEASE_MAJOR_VERSION_INT        4u
#define E2E_P04_AR_RELEASE_MINOR_VERSION_INT        3u
#define E2E_P04_AR_RELEASE_REVISION_VERSION_INT     0u

#define E2E_P04_SW_MAJOR_VERSION_INT        2
#define E2E_P04_SW_MINOR_VERSION_INT        0

/* @req SWS_E2E_00327
 * The implementer of the E2E Library shall avoid the integration of incompatible files. Minimum implementation is the version check of the header files.
 */
#if (E2E_P04_SW_MAJOR_VERSION != E2E_P04_SW_MAJOR_VERSION_INT) || (E2E_P04_SW_MINOR_VERSION != E2E_P04_SW_MINOR_VERSION_INT)
#error "SW Version mismatch between E2E_P04.c and E2E_P04.h"
#endif

#if (E2E_P04_AR_RELEASE_MAJOR_VERSION != E2E_P04_AR_RELEASE_MAJOR_VERSION_INT) || (E2E_P04_AR_RELEASE_MINOR_VERSION != E2E_P04_AR_RELEASE_MINOR_VERSION_INT) || \
    (E2E_P04_AR_RELEASE_REVISION_VERSION != E2E_P04_AR_RELEASE_REVISION_VERSION_INT)
#error "AR Version mismatch between E2E_P04.c and E2E_P04.h"
#endif


/* @req SWS_E2E_00047 Definition of error flags for all E2E units */
/* @req SWS_E2E_00334 Checks Min and Max Length */
/* @req SWS_E2E_00363 The step “Verify inputs of the protect function” in E2E_P04Protect() shall
 * have the following behavior: */

/* Verifies the input of the function Protect Input for the Profile 04.
 */
static INLINE Std_ReturnType verifyProtectInputsP04(const E2E_P04ConfigType* ConfigPtr, const E2E_P04ProtectStateType* StatePtr, const uint8* DataPtr, uint16 Length) {

    Std_ReturnType status;
    status = E2E_E_OK;

    /* Check for NULL pointers */
    if ((ConfigPtr == NULL_PTR) || (StatePtr == NULL_PTR) || (DataPtr == NULL_PTR)) {
        status = E2E_E_INPUTERR_NULL;
    }
    /* Check input parameters values */
    else if ((ConfigPtr->MinDataLength < MIN_P04_DATA_LENGTH_IN_BITS) || (ConfigPtr->MaxDataLength < MIN_P04_DATA_LENGTH_IN_BITS) ||
        (ConfigPtr->MinDataLength > MAX_P04_DATA_LENGTH_IN_BITS) || (ConfigPtr->MaxDataLength > MAX_P04_DATA_LENGTH_IN_BITS) ||
        (ConfigPtr->MinDataLength > ConfigPtr->MaxDataLength)) {

        status = E2E_E_INPUTERR_WRONG;
    }
    /* Transmitted length shall be MinDataLength ≤ Length ≤ MaxDataLength */
    else if ( ((Length*8) < ConfigPtr->MinDataLength) || ((Length*8) > ConfigPtr->MaxDataLength) ) {

        status = E2E_E_INPUTERR_WRONG;
    }
    /* The offset shall be configured to a multiple of 8 and 0 ≤ Offset ≤ (Max)DataLength-(12*8). Compared to transmitted length as E2E header has to fit data */
    else if ( ((ConfigPtr->Offset % 8) != 0) || (ConfigPtr->Offset > ((Length*8)-MIN_P04_DATA_LENGTH_IN_BITS))) {

        status = E2E_E_INPUTERR_WRONG;
    }
    else {
        status = E2E_E_OK;
    }

    return status;
}

/* @req SWS_E2E_00376 The step “Compute offset” in E2E_P04Protect() and E2E_P04Check() shall
 * have the following behavior: */

/* Return local variable uint16 Offset, which is in [byte].
 */
static INLINE uint16 computeOffsetP04(const E2E_P04ConfigType* ConfigPtr) {

    return (ConfigPtr->Offset/8);
}


/* @req SWS_E2E_00364 The step “Write Length” in E2E_P04Protect() shall
 * have the following behavior: */

/* Writes the Length value to the right position within the data byte stream.
 */
static INLINE void writeLengthP04(uint8* DataPtr, uint16 offset, uint16 Length) {

    DataPtr[offset  ] = ((Length & 0xFF00u) >> 8) & 0xFFu;
    DataPtr[offset+1] =  (Length & 0x00FFu)       & 0xFFu;
}


/* @req SWS_E2E_00365 The step “Write Counter” in E2E_P04Protect() shall
 * have the following behavior: */

/* Writes the Counter value to the right position within the data byte stream.
 */
static INLINE void writeCounterP04(uint8* DataPtr, uint16 offset, const E2E_P04ProtectStateType* StatePtr) {

    DataPtr[offset+2] = ((StatePtr->Counter & 0xFF00u) >> 8) & 0xFFu;
    DataPtr[offset+3] =  (StatePtr->Counter & 0x00FFu)       & 0xFFu;
}


/* @req SWS_E2E_00366 The step “Write DataID” in E2E_P04Protect() shall have the following behavior: */

/* Writes the Data ID value to the right position within the data byte stream.
 */
static INLINE void writeDataIDP04(uint8* DataPtr, uint16 offset, const E2E_P04ConfigType* ConfigPtr) {

    DataPtr[offset+4] = ((ConfigPtr->DataID & 0xFF000000u) >> 24) & 0xFFu;
    DataPtr[offset+5] = ((ConfigPtr->DataID & 0x00FF0000u) >> 16) & 0xFFu;
    DataPtr[offset+6] = ((ConfigPtr->DataID & 0x0000FF00u) >>  8) & 0xFFu;
    DataPtr[offset+7] = ((ConfigPtr->DataID & 0x000000FFu)      ) & 0xFFu;
}


/* @req SWS_E2E_00330 In E2E Profile 4, the CRC shall
 * be calculated over the entire E2E header (excluding the CRC bytes) and over the user data. */
/* @req SWS_E2E_00326 In the E2E Profile 4, the Data ID shall be explicitly transmitted,
 * i.e. it shall be the part of the transmitted E2E header. */
/* @req SWS_E2E_00329 E2E Profile 4 shall use the Crc_CalculateCRC32P4 () function of the SWS CRC Library
 * for calculating the CRC.*/
/* @req SWS_E2E_00372 Profile 4 shall provide the control fields as defined in AUTOSAR_SWS_E2ELibrary, 7.5,
 * transmitted at runtime together with the protected data: */
/* @req SWS_E2E_00367 The step “ComputeCRC” in E2E_P04Protect() and in E2E_P04Check()
 * shall have the following behavior: */

/* Returns the CRC value for the Protect and Check function of the Profile 04.
 */
static INLINE uint32 computeCRCP04(const uint8* DataPtr, uint16 offset, uint16 Length) {

    uint32 offsetBytes = (offset+8);
    uint32 crc;

    /* compute CRC over bytes that are before CRC */
    crc = Crc_CalculateCRC32P4(&DataPtr[0], offsetBytes, 0xFFFFFFFFu, TRUE);

    /* Compute CRC over bytes that are after CRC (if any) */
    if ((offset + 12) < Length) {
        offsetBytes = ((Length-offset)-12);
        crc = Crc_CalculateCRC32P4(&DataPtr[offset+12], offsetBytes, crc, FALSE);
    }

    return crc;
}


/* @req SWS_E2E_00368 The step “Write CRC” in E2E_P04Protect() shall have the following behavior: */

/* Writes the CRC value to the right position within the data byte stream.
 */
static INLINE void writeCRCP04(uint8* DataPtr, uint16 offset, uint32 crc) {

    DataPtr[offset+ 8] = ((crc & 0xFF000000u) >> 24) & 0xFFu;
    DataPtr[offset+ 9] = ((crc & 0x00FF0000u) >> 16) & 0xFFu;
    DataPtr[offset+10] = ((crc & 0x0000FF00u) >>  8) & 0xFFu;
    DataPtr[offset+11] = ((crc & 0x000000FFu)      ) & 0xFFu;
}


/* @req SWS_E2E_00478 In E2E Profile 4, on the sender side, for the first transmission request
 * of a data element the counter shall be initialized with 0 and shall be incremented by 1
 * for every subsequent send request. When the counter reaches the maximum value (0xFF’FF),
 * then it shall restart with 0 for the next send request. */
/* @req SWS_E2E_00369 The step “Increment Counter” in E2E_P04Protect() shall have the following behavior: */

/* Increments the Counter Value on the sender side.
 */
static INLINE void incrementCounterP04(E2E_P04ProtectStateType* StatePtr) {

    StatePtr->Counter = (StatePtr->Counter + 1) % (MAX_P04_COUNTER_VALUE + 1);
}


/* @req SWS_E2E_00338 API for E2E_P04Protect */
/* @req SWS_E2E_00362 The function E2E_P04Protect() shall have the following overall behavior: */

/* Protects the array/buffer to be transmitted using the E2E profile 4.
 * This includes checksum calculation, handling of counter and Data ID.
 */

Std_ReturnType E2E_P04Protect(const E2E_P04ConfigType* ConfigPtr, E2E_P04ProtectStateType* StatePtr, uint8* DataPtr, uint16 Length) {

    uint16 offset;
    uint32 crc;

    Std_ReturnType returnValue = verifyProtectInputsP04(ConfigPtr, StatePtr, DataPtr, Length);

    if (E2E_E_OK == returnValue) {
        offset = computeOffsetP04(ConfigPtr);

        writeLengthP04(DataPtr, offset, Length);

        writeCounterP04(DataPtr, offset, StatePtr);

        writeDataIDP04(DataPtr, offset, ConfigPtr);

        crc = computeCRCP04(DataPtr, offset, Length);

        writeCRCP04(DataPtr, offset, crc);

        incrementCounterP04(StatePtr);
    }

    return returnValue;
}


/* @req SWS_E2E_00047 Definition of error flags for all E2E units */
/* @req SWS_E2E_00356 The step “Verify inputs of the check function” in E2E_P04Check()
 * shall have the following behavior: */

/* Verifies the input of the function Check Input for the Profile 04.
 */
static INLINE Std_ReturnType verifyCheckInputsP04(boolean* NewDataAvailable, const E2E_P04ConfigType* ConfigPtr, const E2E_P04CheckStateType* StatePtr, const uint8* DataPtr, uint16 Length) {

    Std_ReturnType status;
    status = E2E_E_OK;
    *NewDataAvailable = FALSE;

    /* Check for NULL pointers */
    if ((ConfigPtr == NULL_PTR) || (StatePtr == NULL_PTR)) {
        status =  E2E_E_INPUTERR_NULL;
    }

    /* Check input parameters values */
    else if ((ConfigPtr->MinDataLength < MIN_P04_DATA_LENGTH_IN_BITS) || (ConfigPtr->MaxDataLength < MIN_P04_DATA_LENGTH_IN_BITS) ||
        (ConfigPtr->MinDataLength > MAX_P04_DATA_LENGTH_IN_BITS) || (ConfigPtr->MaxDataLength > MAX_P04_DATA_LENGTH_IN_BITS) ||
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
    else if ( ((Length*8) < ConfigPtr->MinDataLength) || ((Length*8) > ConfigPtr->MaxDataLength) ) {
        status = E2E_E_INPUTERR_WRONG;
    } 
    /* The offset shall be configured to a multiple of 8 and 0 ≤ Offset ≤ (Max)DataLength-(12*8). Compared to received length as E2E header has to fit data  */
    else if ( ((ConfigPtr->Offset % 8) != 0) || (ConfigPtr->Offset > ((Length*8)-MIN_P04_DATA_LENGTH_IN_BITS))) {
        status = E2E_E_INPUTERR_WRONG;
    }
    else {

        *NewDataAvailable = TRUE;
    }

    return status;
}


/* @req SWS_E2E_00357 The step “Read Length” in E2E_P04Check()
 * shall have the following behavior: */

/* Reads the Length value from the right position within the data byte stream.
 */
static INLINE uint16 readLengthP04(const uint8* DataPtr, uint16 offset) {

    uint16 receivedLength = 0;

    receivedLength |= (((uint16)DataPtr[offset  ]) << 8) & 0xFF00u;
    receivedLength |= (((uint16)DataPtr[offset+1])     ) & 0x00FFu;

    return receivedLength;
}


/* @req SWS_E2E_00358 The step “Read Counter” in E2E_P04Check()
 * shall have the following behavior: */

/* Reads the Counter value from the right position within the data byte stream.
 */
static INLINE uint16 readCounterP04(const uint8* DataPtr, uint16 offset) {

    uint16 receivedCounter = 0;

    receivedCounter |= (((uint16)DataPtr[offset+2]) << 8) & 0xFF00u;
    receivedCounter |= (((uint16)DataPtr[offset+3])     ) & 0x00FFu;

    return receivedCounter;
}


/* @req SWS_E2E_00359 The step “Read DataID” in E2E_P04Check()
 * shall have the following behavior: */

/* Reads the Data ID value from the right position within the data byte stream.
 */
static INLINE uint32 readDataIDP04(const uint8* DataPtr, uint16 offset) {

    uint32 receivedDataID = 0;

    receivedDataID |= (((uint32)DataPtr[offset+4]) << 24) & 0xFF000000u;
    receivedDataID |= (((uint32)DataPtr[offset+5]) << 16) & 0x00FF0000u;
    receivedDataID |= (((uint32)DataPtr[offset+6]) <<  8) & 0x0000FF00u;
    receivedDataID |= (((uint32)DataPtr[offset+7])      ) & 0x000000FFu;

    return receivedDataID;
}


/* @req SWS_E2E_00360 The step “Read CRC” in E2E_P04Check() shall have the following behavior: */

/* Reads the CRC value from the right position within the data byte stream.
 */
static INLINE uint32 readCRCP04(const uint8* DataPtr, uint16 offset) {

    uint32 receivedCRC = 0;

    receivedCRC |= (((uint32)DataPtr[offset+ 8]) << 24) & 0xFF000000u;
    receivedCRC |= (((uint32)DataPtr[offset+ 9]) << 16) & 0x00FF0000u;
    receivedCRC |= (((uint32)DataPtr[offset+10]) <<  8) & 0x0000FF00u;
    receivedCRC |= (((uint32)DataPtr[offset+11])      ) & 0x000000FFu;

    return receivedCRC;
}


/* Calculates the delta counter value out of received and last valid counter values.
 */
static INLINE uint16 calculateDeltaCounterP04(uint16 receivedCounter, uint16 lastValidCounter)
{
    uint16 status;

    if (receivedCounter >= lastValidCounter) {
        status = receivedCounter - lastValidCounter;
    }
    else {
        status = (uint16)((uint32)MAX_P04_COUNTER_VALUE + 1 + (uint32)receivedCounter - (uint32)lastValidCounter);
    }

    return status;
}


/* @req SWS_E2E_00361 The step “Do Checks” in E2E_P04Check() shall have the following behavior: */

/* Do checks to determine the next status value of the receiver.
 */
static INLINE void doChecksP04(E2E_P04CheckStateType* StatePtr, const boolean* NewDataAvailable, const E2E_P04ConfigType* ConfigPtr, uint32 receivedCRC, uint32 computedCRC, uint32 receivedDataID, uint16 receivedLength, uint16 Length, uint16 receivedCounter) {

    uint16 deltaCounter;

    if (*NewDataAvailable == FALSE) {
        StatePtr->Status = E2E_P04STATUS_NONEWDATA;
    }

    else if ( (receivedCRC != computedCRC) || (receivedDataID != ConfigPtr->DataID)
    /** @CODECOV PARAMETER_VALIDATION_PRIVATE_FUNCTION:CRC check will always fail if lengths are different, length check will never occur */
    __CODE_COVERAGE_IGNORE__
    || (receivedLength != Length)) {
        StatePtr->Status = E2E_P04STATUS_ERROR;
    }

    else {
        deltaCounter = calculateDeltaCounterP04(receivedCounter, StatePtr->Counter);

        if (deltaCounter > ConfigPtr->MaxDeltaCounter) {
            StatePtr->Status = E2E_P04STATUS_WRONGSEQUENCE;
        }
        else if (deltaCounter == 0) {
            StatePtr->Status = E2E_P04STATUS_REPEATED;
        }
        else if (deltaCounter == 1) {
            StatePtr->Status = E2E_P04STATUS_OK;
        }
        else {
            StatePtr->Status = E2E_P04STATUS_OKSOMELOST;
        }

        StatePtr->Counter = receivedCounter;
    }
}


/* @req SWS_E2E_00339 API for E2E_P04Check */
/* @req SWS_E2E_00337 Definition of E2E_P04CheckStatusType */
/* @req SWS_E2E_00355 The function E2E_P04Check() shall have the following overall behavior: */

/* Checks the Data received using the E2E profile 4. This includes CRC calculation, handling of Counter and Data ID.
 * The function checks only one single data in one cycle, it does not determine/compute the accumulated
 * state of the communication link.
 */
Std_ReturnType E2E_P04Check(const E2E_P04ConfigType* ConfigPtr, E2E_P04CheckStateType* StatePtr, const uint8* DataPtr, uint16 Length) {

    boolean NewDataAvailable;
    uint16 offset;
    uint16 receivedLength = 0;
    uint16 receivedCounter = 0;
    uint32 receivedDataID = 0;
    uint32 receivedCRC = 0;
    uint32 computedCRC = 0;

    Std_ReturnType returnValue = verifyCheckInputsP04(&NewDataAvailable, ConfigPtr, StatePtr, DataPtr, Length);

    if (E2E_E_OK == returnValue) {
        if (NewDataAvailable == TRUE) {

            offset = computeOffsetP04(ConfigPtr);

            receivedLength = readLengthP04(DataPtr, offset);

            receivedCounter = readCounterP04(DataPtr, offset);

            receivedDataID = readDataIDP04(DataPtr, offset);

            receivedCRC = readCRCP04(DataPtr, offset);

            computedCRC = computeCRCP04(DataPtr, offset, Length);
        }

        doChecksP04(StatePtr, &NewDataAvailable, ConfigPtr, receivedCRC, computedCRC, receivedDataID, receivedLength, Length, receivedCounter);
    }

    return returnValue;
}


/* @req SWS_E2E_00335 Definition of E2E_P04ProtectStateType */
/* @req SWS_E2E_00373 API for E2E_P04ProtectInit */
/* @req SWS_E2E_00377 In case State is NULL, E2E_P04ProtectInit shall return immediately with E2E_E_INPUTERR_NULL.
 * Otherwise, it shall intialize the state structure, setting Counter to 0. */

/* Initializes the protection state.
 */
Std_ReturnType E2E_P04ProtectInit(E2E_P04ProtectStateType* StatePtr) {

    Std_ReturnType status;
    status = E2E_E_OK;

    if (StatePtr == NULL_PTR) {
        status = E2E_E_INPUTERR_NULL;
    } else {
        StatePtr->Counter = 0;
    }

    return status;
}


/* @req SWS_E2E_00336 Definition of the State of the reception on one single Data protected with E2E Profile 4. */
/* @req SWS_E2E_00350 API for E2E_P04CheckInit */
/* @req SWS_E2E_00378 In case State is NULL, E2E_P04CheckInit shall return immediately with E2E_E_INPUTERR_NULL.
 * Otherwise, it shall initialize the state structure, setting:
 *      1. Counter to 0xFF’FF.
 *      2. Status to E2E_P04STATUS_ERROR.
 */

/* Initializes the check state.
 */
Std_ReturnType E2E_P04CheckInit(E2E_P04CheckStateType* StatePtr) {

    Std_ReturnType status;
    status = E2E_E_OK;

    if (StatePtr == NULL_PTR) {
        status = E2E_E_INPUTERR_NULL;
    } else {
        StatePtr->Status = E2E_P04STATUS_ERROR;
        StatePtr->Counter = 0xFFFF;
    }

    return status;
}


/* @req SWS_E2E_00349 API for E2E_P04MapStatusToSM */
/* @req SWS_E2E_00351 If CheckReturn = E2E_E_OK, then the function E2E_P04MapStatusToSM shall
 * return the values depending on the value of Status: */
/* @req SWS_E2E_00352 If CheckReturn != E2E_E_OK, then the function E2E_P04MapStatusToSM() shall
 * return E2E_P_ERROR (regardless of value of Status). */

/* The function maps the check status of Profile 4 to a generic check status,
 * which can be used by E2E state machine check function.
 * The E2E Profile 4 delivers a more fine-granular status, but this is not relevant for the E2E state machine.
 */
E2E_PCheckStatusType E2E_P04MapStatusToSM(Std_ReturnType CheckReturn, E2E_P04CheckStatusType Status) {

    E2E_PCheckStatusType retValue = E2E_P_ERROR;

    if (CheckReturn == E2E_E_OK) {
        switch( Status ) {
            case E2E_P04STATUS_OK:
            case E2E_P04STATUS_OKSOMELOST:
                retValue = E2E_P_OK;
                break;
            case E2E_P04STATUS_REPEATED:
                retValue = E2E_P_REPEATED;
                break;
            case E2E_P04STATUS_NONEWDATA:
                retValue = E2E_P_NONEWDATA;
                break;
            case E2E_P04STATUS_WRONGSEQUENCE:
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
