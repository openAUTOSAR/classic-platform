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

/* @req SWS_E2E_00215 Files E2E_P05.c and E2E_P05.h shall contain implementation parts specific of each profile. */
/* @req SWS_E2E_00049 The E2E library shall not contain library-internal mechanisms for error detection to be traced as development errors. */
/* @req SWS_E2E_00221 Each E2E Profile shall use a subset of the data protection mechanisms defined in AUTOSAR_SWS_E2ELibrary.pdf, 7.2. */

/* @req SWS_E2E_00011 The E2E Library shall report errors detected by library-internal mechanisms
 * to callers of E2E functions through return value.
 */
/* @req SWS_E2E_00012 The internal library mechanisms shall detect and report errors
 * shall be implemented according to the pre-defined E2E Profiles specified in sections 7.3 and 7.4.
 */

#include "E2E_P05.h"


/* @req SWS_E2E_00397 Counter maximum value (0xFF) */
#define MAX_P05_COUNTER_VALUE           (255)
/* @req SWS_E2E_00437 Length of Data, in bits.
 * The maximum value shall be = 4096*8 (4kB) and shall be ≥ 3*8
 */
#define MAX_P05_DATA_LENGTH_IN_BITS     (8*4096)
#define MIN_P05_DATA_LENGTH_IN_BITS     (8*3)

#define E2E_P05_AR_RELEASE_MAJOR_VERSION_INT        4u
#define E2E_P05_AR_RELEASE_MINOR_VERSION_INT        3u
#define E2E_P05_AR_RELEASE_REVISION_VERSION_INT     0u

#define E2E_P05_SW_MAJOR_VERSION_INT        2
#define E2E_P05_SW_MINOR_VERSION_INT        0

/* @req SWS_E2E_00327
 * The implementer of the E2E Library shall avoid the integration of incompatible files. Minimum implementation is the version check of the header files.
 */
#if (E2E_P05_SW_MAJOR_VERSION != E2E_P05_SW_MAJOR_VERSION_INT) || (E2E_P05_SW_MINOR_VERSION != E2E_P05_SW_MINOR_VERSION_INT)
#error "SW Version mismatch between E2E_P05.c and E2E_P05.h"
#endif

#if (E2E_P05_AR_RELEASE_MAJOR_VERSION != E2E_P05_AR_RELEASE_MAJOR_VERSION_INT) || (E2E_P05_AR_RELEASE_MINOR_VERSION != E2E_P05_AR_RELEASE_MINOR_VERSION_INT) || \
    (E2E_P05_AR_RELEASE_REVISION_VERSION != E2E_P05_AR_RELEASE_REVISION_VERSION_INT)
#error "AR Version mismatch between E2E_P05.c and E2E_P05.h"
#endif


/* @req SWS_E2E_00047 Definition of error flags for all E2E units */
/* @req SWS_E2E_00404 The step “Verify inputs of the protect function” in E2E_P05Protect()
 * shall have the following behavior: */
/* @req SWS_E2E_00437 Checks Min and Max Length */

/* Verifies the input of the function Protect Input for the Profile 05.
 */
static INLINE Std_ReturnType verifyProtectInputsP05(const E2E_P05ConfigType* ConfigPtr, const E2E_P05ProtectStateType* StatePtr, const uint8* DataPtr, uint16 Length) {

    Std_ReturnType status;
    status = E2E_E_OK;

    /* Check for NULL pointers */
    if ((ConfigPtr == NULL_PTR) || (StatePtr == NULL_PTR) || (DataPtr == NULL_PTR)) {
        status = E2E_E_INPUTERR_NULL;
    }
    /* Transmitted length shall be 3*8 ≤ Length ≤ 4KB. It must also be equal to the configured length. */
    else if ( ((8*Length) < MIN_P05_DATA_LENGTH_IN_BITS) || ((8*Length) > MAX_P05_DATA_LENGTH_IN_BITS) || ((8*Length) != ConfigPtr->DataLength) ) {
        status = E2E_E_INPUTERR_WRONG;
    } 
    /* The offset shall be a multiple of 8 and 0 ≤ Offset ≤ DataLength-(3*8). */
    else if ( ((ConfigPtr->Offset % 8) != 0) || (ConfigPtr->Offset > ((8*Length)-MIN_P05_DATA_LENGTH_IN_BITS))) {
        status = E2E_E_INPUTERR_WRONG;
    }
    else {
        status = E2E_E_OK;
    }

    return status;
}


/* @req SWS_E2E_00469 The step “Compute offset” in E2E_P05Protect() and E2E_P05Check()
 * shall have the following behavior: */

/* Return local variable uint16 Offset, which is in [byte].
 */
static INLINE uint16 computeOffsetP05(const E2E_P05ConfigType* ConfigPtr) {

    return (ConfigPtr->Offset/8);
}


/* @req SWS_E2E_00405 The step “Write Counter” in E2E_P05Protect()
 * shall have the following behavior: */

/* Writes the Counter value to the right position within the data byte stream.
 */
static INLINE void writeCounterP05(uint8* DataPtr, uint16 offset, const E2E_P05ProtectStateType* StatePtr) {

    DataPtr[offset+2] = (StatePtr->Counter & 0xFFu) & 0xFFu;
}


/* @req SWS_E2E_00399 In the E2E Profile 5, the Data ID shall be implicitly transmitted,
 * by adding the Data ID after the user data in the CRC calculation. */
/* @req SWS_E2E_00400 E2E Profile 5 shall use the Crc_CalculateCRC16() function of the SWS CRC Library
 * for calculating the CRC (Polynomial: 0x1021; Autosar notation). */
/* @req SWS_E2E_00401 In E2E Profile 5, the CRC shall be calculated over the entire E2E header
 * (excluding the CRC bytes), including the user data extended at the end with the Data ID. */
/* @req SWS_E2E_00394 Profile 5 shall provide the following control fields as defined in AUTOSAR_SWS_E2ELibrary, 7.6,
 * transmitted at runtime together with the protected data: */

/* @req SWS_E2E_00406 The step “Compute CRC” in E2E_P05Protect() and in E2E_P05Check
 * shall have the following behavior: */

/* Returns the CRC value for the Protect and Check function of the Profile 05.
 */
static INLINE uint16 computeCRCP05(const uint8* DataPtr, uint16 length, uint16 offset, uint16 dataId) {

    uint16 lengthBytes = (length-offset)-2;
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

    dataIdNibble = ((dataId) & 0xFFu);
    crc = Crc_CalculateCRC16(&dataIdNibble, 1, crc, FALSE);

    dataIdNibble = ((dataId>>8) & 0xFFu);
    crc = Crc_CalculateCRC16(&dataIdNibble, 1, crc, FALSE);

    return crc;
}


/* @req SWS_E2E_00407 The step “Write CRC” in E2E_P05Protect()
 * shall have the following behavior: */

/* Writes the CRC value to the right position within the data byte stream.
 */
static INLINE void writeCRCP05(uint8* DataPtr, uint16 offset, uint16 crc) {

    DataPtr[offset  ] = ((crc & 0x00FFu)      ) & 0xFFu;
    DataPtr[offset+1] = ((crc & 0xFF00u) >>  8) & 0xFFu;
}


/* @req SWS_E2E_00397 In E2E Profile 5, on the sender side, for the first transmission
 * request of a data element the counter shall be initialized with 0 and shall be incremented by 1
 * for every subsequent send request. When the counter reaches the maximum value (0xFF),
 * then it shall restart with 0 for the next send request. */
/* @req SWS_E2E_00409 The step “Increment Counter” in E2E_P05Protect()
 * shall have the following behavior: */

/* Increments the Counter Value on the sender side.
 */
static INLINE void incrementCounterP05(E2E_P05ProtectStateType* StatePtr) {

    StatePtr->Counter = (StatePtr->Counter + 1) % (MAX_P05_COUNTER_VALUE + 1);
}



/* @req SWS_E2E_00403 The function E2E_P05Protect() shall have the following overall behavior: */
/* @req SWS_E2E_00446 API for E2E_P05Protect */

/* Protects the array/buffer to be transmitted using the E2E profile 5.
 * This includes checksum calculation, handling of counter.
 */
Std_ReturnType E2E_P05Protect(const E2E_P05ConfigType* ConfigPtr, E2E_P05ProtectStateType* StatePtr, uint8* DataPtr, uint16 Length) {

    uint16 offset;
    uint16 crc;

    Std_ReturnType returnValue = verifyProtectInputsP05(ConfigPtr, StatePtr, DataPtr, Length);

    if (E2E_E_OK == returnValue) {
        offset = computeOffsetP05(ConfigPtr);

        writeCounterP05(DataPtr, offset, StatePtr);

        crc = computeCRCP05(DataPtr, Length, offset, ConfigPtr->DataID);

        writeCRCP05(DataPtr, offset, crc);

        incrementCounterP05(StatePtr);
    }

    return returnValue;
}


/* @req SWS_E2E_00047 Definition of error flags for all E2E units */
/* @req SWS_E2E_00412 The step “Verify inputs of the check function” in E2E_P05Check()
 * shall have the following behavior: */

/* Verifies the input of the function Check Input for the Profile 05.
 */
static INLINE Std_ReturnType verifyCheckInputsP05(boolean* NewDataAvailable, const E2E_P05ConfigType* ConfigPtr, const E2E_P05CheckStateType* StatePtr, const uint8* DataPtr, uint16 Length) {

    Std_ReturnType status;
    status = E2E_E_OK;
    *NewDataAvailable = FALSE;

    /* Check for NULL pointers */
    if ((ConfigPtr == NULL_PTR) || (StatePtr == NULL_PTR)) {
        status = E2E_E_INPUTERR_NULL;
    }

    else if ( !(((DataPtr != NULL_PTR) && (Length != 0)) || ((DataPtr == NULL_PTR) && (Length == 0))) ) {
        status = E2E_E_INPUTERR_WRONG;
    }

    else if (DataPtr == NULL_PTR) {
        status = E2E_E_OK;
    }
    /* Check input parameters values */
    /* Received length shall be 3*8 ≤ Length ≤ 4KB. It must also be equal to the configured length. */
    else if ( ((8*Length) < MIN_P05_DATA_LENGTH_IN_BITS) || ((8*Length) > MAX_P05_DATA_LENGTH_IN_BITS) || ((8*Length) != ConfigPtr->DataLength) ) {
        status = E2E_E_INPUTERR_WRONG;
    }    
    /* The offset shall be a multiple of 8 and 0 ≤ Offset ≤ DataLength-(3*8). */
    else if ( ((ConfigPtr->Offset % 8) != 0) || (ConfigPtr->Offset > ((8*Length)-MIN_P05_DATA_LENGTH_IN_BITS))) {
        status = E2E_E_INPUTERR_WRONG;
    }
    else {
        *NewDataAvailable = TRUE;
    }

    return status;
}


/* @req SWS_E2E_00413 The step “Read Counter” in E2E_P05Check()
 * shall have the following behavior: */

/* Reads the Counter value from the right position within the data byte stream.
 */
static INLINE uint8 readCounterP05(const uint8* DataPtr, uint16 offset) {

    uint8 receivedCounter = 0;

    receivedCounter |= (DataPtr[offset+2] & 0xFFu);

    return receivedCounter;
}


/* @req SWS_E2E_00414 The step “Read CRC” in E2E_P05Check()
 * shall have the following behavior: */

/* Reads the CRC value from the right position within the data byte stream.
 */
static INLINE uint16 readCRCP05(const uint8* DataPtr, uint16 offset) {

    uint16 receivedCRC = 0;

    receivedCRC |= (((uint16)DataPtr[offset ]       ) & 0x00FFu);
    receivedCRC |= (((uint16)DataPtr[offset+1] <<  8) & 0xFF00u);

    return receivedCRC;
}


/* Calculates the delta counter value out of received and last valid counter values.
 */
static INLINE uint8 calculateDeltaCounterP05(uint8 receivedCounter, uint8 lastValidCounter)
{
    uint8 status;

    if (receivedCounter >= lastValidCounter) {
        status = receivedCounter - lastValidCounter;
    }
    else {
        status = (uint8)((uint32)MAX_P05_COUNTER_VALUE + 1 + (uint32)receivedCounter - (uint32)lastValidCounter);
    }

    return status;
}


/* @req SWS_E2E_00416 The step “Do Checks” in E2E_P05Check() shall have the following behavior: */

/* Do checks to determine the next status value of the receiver.
 */
static INLINE void doChecksP05(E2E_P05CheckStateType* StatePtr, const boolean* NewDataAvailable, const E2E_P05ConfigType* ConfigPtr, uint16 receivedCRC, uint16 computedCRC, uint8 receivedCounter) {

    uint8 deltaCounter;

    if (*NewDataAvailable == FALSE) {
        StatePtr->Status = E2E_P05STATUS_NONEWDATA;
    }

    else if (receivedCRC != computedCRC) {
        StatePtr->Status = E2E_P05STATUS_ERROR;
    }

    else {
        deltaCounter = calculateDeltaCounterP05(receivedCounter, StatePtr->Counter);

        if (deltaCounter > ConfigPtr->MaxDeltaCounter) {
            StatePtr->Status = E2E_P05STATUS_WRONGSEQUENCE;
        }
        else if (deltaCounter == 0) {
            StatePtr->Status = E2E_P05STATUS_REPEATED;
        }
        else if (deltaCounter == 1) {
            StatePtr->Status = E2E_P05STATUS_OK;
        }
        else {
            StatePtr->Status = E2E_P05STATUS_OKSOMELOST;
        }

        StatePtr->Counter = receivedCounter;
    }
}


/* @req SWS_E2E_00411 The function E2E_P05Check() shall have the following overall behavior: */
/* @req SWS_E2E_00440 Definition of E2E_P05CheckStatusType */
/* @req SWS_E2E_00449 API for E2E_P05Check */

/* Checks the Data received using the E2E profile 5. This includes CRC calculation, handling of Counter.
 * The function checks only one single data in one cycle, it does not determine/compute the accumulated
 * state of the communication link.
 */
Std_ReturnType E2E_P05Check(const E2E_P05ConfigType* ConfigPtr, E2E_P05CheckStateType* StatePtr, const uint8* DataPtr, uint16 Length) {

    boolean NewDataAvailable;
    uint16 offset;
    uint8 receivedCounter = 0;
    uint16 receivedCRC = 0;
    uint16 computedCRC = 0;

    Std_ReturnType returnValue = verifyCheckInputsP05(&NewDataAvailable, ConfigPtr, StatePtr, DataPtr, Length);

    if (E2E_E_OK == returnValue) {
        if (NewDataAvailable == TRUE) {

            offset = computeOffsetP05(ConfigPtr);

            receivedCounter = readCounterP05(DataPtr, offset);

            receivedCRC = readCRCP05(DataPtr, offset);

            computedCRC = computeCRCP05(DataPtr, Length, offset, ConfigPtr->DataID);
        }

        doChecksP05(StatePtr, &NewDataAvailable, ConfigPtr, receivedCRC, computedCRC, receivedCounter);
    }

    return returnValue;
}


/* @req SWS_E2E_00438 Definition of E2E_P05ProtectStateType */
/* @req SWS_E2E_00447 API for E2E_P05ProtectInit */
/* @req SWS_E2E_00448 In case State is NULL, E2E_P05ProtectInit shall return immediately with E2E_E_INPUTERR_NULL.
 * Otherwise, it shall intialize the state structure, setting Counter to 0. */

/* Initializes the protection state.
 */
Std_ReturnType E2E_P05ProtectInit(E2E_P05ProtectStateType* StatePtr) {

    Std_ReturnType status;
    status = E2E_E_OK;

    if (StatePtr == NULL_PTR) {
        status =  E2E_E_INPUTERR_NULL;
    }
    else {
        StatePtr->Counter = 0;
    }

    return status;
}


/* @req SWS_E2E_00439 Definition of the State of the reception on one single Data protected with E2E Profile 5. */
/* @req SWS_E2E_00450 API for E2E_P05CheckInit */
/* @req SWS_E2E_00451 In case State is NULL, E2E_P05CheckInit shall return immediately with E2E_E_INPUTERR_NULL.
 * Otherwise, it shall initialize the state structure, setting:
 *      1. Counter to 0xFF
 *      2. Status to E2E_P05STATUS_ERROR.
 */

/* Initializes the check state.
 */
Std_ReturnType E2E_P05CheckInit(E2E_P05CheckStateType* StatePtr) {

    Std_ReturnType status;
    status = E2E_E_OK;

    if (StatePtr == NULL_PTR) {
        status =  E2E_E_INPUTERR_NULL;
    }
    else {
        StatePtr->Status = E2E_P05STATUS_ERROR;
        StatePtr->Counter = 0xFF;
    }

    return status;
}


/* @req SWS_E2E_00452 API for E2E_P05MapStatusToSM */
/* @req SWS_E2E_00453 If CheckReturn = E2E_E_OK, then the function E2E_P05MapStatusToSM shall
 * return the values depending on the value of Status: */
/* @req SWS_E2E_00454 If CheckReturn != E2E_E_OK, then the function E2E_P05MapStatusToSM() shall
 * return E2E_P_ERROR (regardless of value of Status). */

/* The function maps the check status of Profile 5 to a generic check status,
 * which can be used by E2E state machine check function. The E2E Profile 5 delivers a more fine-granular status,
 * but this is not relevant for the E2E state machine.
 */
E2E_PCheckStatusType E2E_P05MapStatusToSM(Std_ReturnType CheckReturn, E2E_P05CheckStatusType Status) {

    E2E_PCheckStatusType retValue = E2E_P_ERROR;

    if (CheckReturn == E2E_E_OK) {
        switch( Status ) {
            case E2E_P05STATUS_OK:
            case E2E_P05STATUS_OKSOMELOST:
                retValue = E2E_P_OK;
                break;
            case E2E_P05STATUS_REPEATED:
                retValue = E2E_P_REPEATED;
                break;
            case E2E_P05STATUS_NONEWDATA:
                retValue = E2E_P_NONEWDATA;
                break;
            case E2E_P05STATUS_WRONGSEQUENCE:
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
