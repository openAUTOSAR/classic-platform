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

/** @file E2E_P06.h
 *
 *  The E2E Profile 6 provides a consistent set of data protection mechanisms,
 *  designed to protecting against the faults considered in the fault model.
 */

#ifndef E2E_P06_H_
#define E2E_P06_H_

/* @req SWS_E2E_00115 E2E library files (i.e. E2E_*.*) shall not include any RTE files */
/* @req SWS_E2E_00217 The implementation of the E2E Library shall provide at least one of the E2E Profiles. */

/* AUTOSAR_SWS_E2ELibrary.pdf, Figure5-1: File dependencies,
 * does not show any requirement on E2E_SM.* files. But as the API
 * is defined for E2E_P01, it is necessary to include E2E_SM.h here
 */
#include "E2E_SM.h"

/* According to AUTOSAR_SWS_E2ELibrary.pdf, Figure5-1: File dependencies:
 * E2E_MemMap.h is included from all E2E files
 */
#include "E2E_MemMap.h"

#define E2E_P06_AR_RELEASE_MAJOR_VERSION        4u
#define E2E_P06_AR_RELEASE_MINOR_VERSION        3u
#define E2E_P06_AR_RELEASE_REVISION_VERSION     0u

#define E2E_P06_SW_MAJOR_VERSION        2
#define E2E_P06_SW_MINOR_VERSION        0

/* @req SWS_E2E_00327
 * The implementer of the E2E Library shall avoid the integration of incompatible files. Minimum implementation is the version check of the header files.
 */
#if (E2E_SM_AR_RELEASE_MAJOR_VERSION != E2E_P06_AR_RELEASE_MAJOR_VERSION) || (E2E_SM_AR_RELEASE_MINOR_VERSION != E2E_P06_AR_RELEASE_MINOR_VERSION)
#error "SW Version mismatch between E2E_SM.h and E1E_P06.h"
#endif


/* @req SWS_E2E_00445 */
/**
 * Status of the reception on one single Data in one cycle, protected with E2E Profile 6.
 */
typedef enum {
    /**
     * OK: the checks of the Data in this cycle were successful
     * (including counter check, which was incremented by 1).
     */
    E2E_P06STATUS_OK = 0x00,
    /**
     * Error: the Check function has been invoked but no new Data is not available since the last call,
     * according to communication medium (e.g. RTE, COM). As a result, no E2E checks of Data have been consequently executed.
     * This may be considered similar to E2E_P06STATUS_REPEATED.
     */
    E2E_P06STATUS_NONEWDATA = 0x01,
    /**
     * Error: error not related to counters occurred (e.g. wrong crc, wrong length, wrong options, wrong Data ID).
     */
    E2E_P06STATUS_ERROR =  0x07,
    /**
     * Error: the checks of the Data in this cycle were successful, with the exception of the repetition.
     */
    E2E_P06STATUS_REPEATED = 0x08,
    /**
     * OK: the checks of the Data in this cycle were successful (including counter check,
     * which was incremented within the allowed configured delta).
     */
    E2E_P06STATUS_OKSOMELOST = 0x20,
    /**
     * Error: the checks of the Data in this cycle were successful,
     * with the exception of counter jump, which changed more than the allowed delta.
     */
    E2E_P06STATUS_WRONGSEQUENCE = 0x40
} E2E_P06CheckStatusType;


/* @req SWS_E2E_00419
 * In the E2E Profile 6, the Data ID shall be implicitly transmitted,
 * by adding the Data ID after the user data in the CRC calculation.
 */
/* @req SWS_E2E_00479
 * Profile 6 shall provide the following control fields, transmitted at runtime together with the protected data:
 */
/* @req SWS_E2E_00441 */
/**
 * Configuration of transmitted Data (Data Element or I-PDU), for E2E Profile 6.
 * For each transmitted Data, there is an instance of this typedef.
 */
typedef struct {
    /**
     * Bit offset of the first bit of the E2E headerfrom the beginning of the Data (bit numbering: bit 0 is the least important).
     * The offset shall be a multiple of 8 and 0 ≤ Offset ≤ MaxDataLength-(5*8).
     * Example: If Offset equals 8, then the high byte of the E2E Crc (16 bit) is written to Byte 1, the low Byte is written to Byte 2.
     */
    uint16 Offset;
    /**
     * Minimal length of Data, in bits (i.e. the length of the E2E-protected message, without counting the field Length itself).
     * E2E checks that Length is ≥ MinDataLength. The value shall be = 4096*8 (4kB) and shall be ≥ 5*8
     */
    uint16 MinDataLength;
    /**
     * Maximal length of Data, in bits. E2E checks that DataLength is ≤ MaxDataLength.
     * The value shall be = 4096*8 (4kB). MaxDataLength shall be ≥ MinDataLength.
     */
    uint16 MaxDataLength;
    /**
     * A system-unique identifier of the Data.
     */
    uint16 DataID;
    /**
     * Maximum allowed gap between two counter values of two consecutively received valid Data. For example,
     * if the receiver gets Data with counter 1 and MaxDeltaCounter is 3,
     * then at the next reception the receiver can accept Counters with values 2, 3 or 4.
     */
    uint8 MaxDeltaCounter;
} E2E_P06ConfigType;


/* @req SWS_E2E_00443 */
/**
 * State of the sender for a Data protected with E2E Profile 6.
 */
typedef struct {
    /**
     * Counter to be used for protecting the next Data. The initial value is 0, which means that in the first cycle, Counter is 0.
     * Each time E2E_P06Protect() is called, it increments the counter up to 0xFF. After the maximum value is reached,
     * the next value is 0x0. The overflow is not reported to the caller.
     */
    uint8 Counter;
} E2E_P06ProtectStateType;


/* @req SWS_E2E_00444 */
/**
 * State of the reception on one single Data protected with E2E Profile 6.
 */
typedef struct {
    /**
     * Result of the verification of the Data in this cycle, determined by the Check function.
     */
    E2E_P06CheckStatusType Status;
    /**
     * Counter of the data in previous cycle.
     */
    uint8 Counter;
} E2E_P06CheckStateType;


/** @brief Protects the array/buffer to be transmitted using the E2E profile 6.
 *         This includes checksum calculation, handling of counter.
 *
 *  @param ConfigPtr Pointer to static configuration.
 *  @param StatePtr Pointer to port/data communication state.
 *  @param DataPtr Pointer to Data to be transmitted.
 *  @param Length Length of the data in bytes.
 *
 *  @return Error code (E2E_E_INPUTERR_NULL || E2E_E_INPUTERR_WRONG || E2E_E_INTERR || E2E_E_OK).
 */
/* @req SWS_E2E_00047 error flags for errors shall be used by all E2E Library functions */
/* @req SWS_E2E_00393 API for E2E_P06Protect */
Std_ReturnType E2E_P06Protect(const E2E_P06ConfigType* ConfigPtr, E2E_P06ProtectStateType* StatePtr, uint8* DataPtr, uint16 Length);


/** @brief Checks the Data received using the E2E profile 6.
 *         This includes CRC calculation, handling of Counter.
 *         The function checks only one single data in one cycle,
 *         it does not determine/compute the accumulated state of the communication link.
 *
 *  @param ConfigPtr Pointer to static configuration.
 *  @param StatePtr Pointer to port/data communication state.
 *  @param DataPtr Pointer to received data.
 *  @param Length Length of the data in bytes.
 *
 *  @return Error code (E2E_E_INPUTERR_NULL || E2E_E_INPUTERR_WRONG || E2E_E_INTERR || E2E_E_OK).
 */
/* @req SWS_E2E_00047 error flags for errors shall be used by all E2E Library functions */
/* @req SWS_E2E_00457 API for E2E_P06Check */
Std_ReturnType E2E_P06Check(const E2E_P06ConfigType* ConfigPtr, E2E_P06CheckStateType* StatePtr, const uint8* DataPtr, uint16 Length);


/** @brief Initializes the protection state.
 *
 *  @param StatePtr Pointer to port/data communication state.
 *
 *  @return Error code (E2E_E_INPUTERR_NULL - null pointer passed E2E_E_OK).
 */
/* @req SWS_E2E_00047 error flags for errors shall be used by all E2E Library functions */
/* @req SWS_E2E_00455 API for E2E_P06ProtectInit */
Std_ReturnType E2E_P06ProtectInit(E2E_P06ProtectStateType* StatePtr);


/** @brief Initializes the check state.
 *
 *  @param StatePtr Pointer to port/data communication state.
 *
 *  @return Error code (E2E_E_INPUTERR_NULL - null pointer passed E2E_E_OK).
 */
/* @req SWS_E2E_00047 error flags for errors shall be used by all E2E Library functions */
/* @req SWS_E2E_00458 API for E2E_P06CheckInit */
Std_ReturnType E2E_P06CheckInit(E2E_P06CheckStateType* StatePtr);


/** @brief The function maps the check status of Profile 6 to a generic check status,
 *         which can be used by E2E state machine check function.
 *         The E2E Profile 6 delivers a more fine-granular status,
 *         but this is not relevant for the E2E state machine.
 *
 *  @param CheckReturn Return value of the E2E_P06Check function.
 *  @param Status Status determined by E2E_P06Check function.

 *  @return Profile-independent status of the reception on one single Data in one cycle.
 */
/* @req SWS_E2E_00047 error flags for errors shall be used by all E2E Library functions */
/* @req SWS_E2E_00460 API for E2E_P06MapStatusToSM */
E2E_PCheckStatusType E2E_P06MapStatusToSM(Std_ReturnType CheckReturn, E2E_P06CheckStatusType Status);


#endif
