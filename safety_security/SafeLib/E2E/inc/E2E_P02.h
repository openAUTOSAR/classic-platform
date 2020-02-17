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

/** @file E2E_P02.h
 *
 *  The E2E Profile 2 provides a consistent set of data protection mechanisms,
 *  designed to protecting against the faults considered in the fault model.
 */

#ifndef E2E_P02_H_
#define E2E_P02_H_

/* @req SWS_E2E_00115 E2E library files (i.e. E2E_*.*) shall not include any RTE files */
/* @req SWS_E2E_00217 The implementation of the E2E Library shall provide at least one of the E2E Profiles. */

/* According to AUTOSAR_SWS_E2ELibrary.pdf, Figure5-1: File dependencies:
 * E2E.h is included from E2E_P01.h files
 */
#include "E2E.h"

/* AUTOSAR_SWS_E2ELibrary.pdf, Figure5-1: File dependencies,
 * does not show any requirement on E2E_SM.* files. But as the API
 * is defined for E2E_P01, it is necessary to include E2E_SM.h here
 */
#include "E2E_SM.h"

/* According to AUTOSAR_SWS_E2ELibrary.pdf, Figure5-1: File dependencies:
 * E2E_MemMap.h is included from all E2E files
 */
#include "E2E_MemMap.h"

#define E2E_P02_AR_RELEASE_MAJOR_VERSION        4u
#define E2E_P02_AR_RELEASE_MINOR_VERSION        3u
#define E2E_P02_AR_RELEASE_REVISION_VERSION     0u

#define E2E_P02_SW_MAJOR_VERSION        2
#define E2E_P02_SW_MINOR_VERSION        0

/* @req SWS_E2E_00327
 * The implementer of the E2E Library shall avoid the integration of incompatible files. Minimum implementation is the version check of the header files.
 */
#if (E2E_AR_RELEASE_MAJOR_VERSION != E2E_P02_AR_RELEASE_MAJOR_VERSION) || (E2E_AR_RELEASE_MINOR_VERSION != E2E_P02_AR_RELEASE_MINOR_VERSION)
#error "SW Version mismatch between E2E.h and E2E_P02.h"
#endif

#if (E2E_SM_AR_RELEASE_MAJOR_VERSION != E2E_P02_AR_RELEASE_MAJOR_VERSION) || (E2E_SM_AR_RELEASE_MINOR_VERSION != E2E_P02_AR_RELEASE_MINOR_VERSION)
#error "SW Version mismatch between E2E_SM.h and E1E_P02.h"
#endif

/* @req SWS_E2E_00214 */
/**
 * Result of the verification of the Data in E2E Profile 2, determined by the Check function.
 * As the enum data are not defined in the AUTOSAR_SWS_E2ELibrary.pdf, use UC_E2E_0302 for definition.
 */
typedef enum {
    /**
     * OK: The new data has been received according to communication medium, the CRC is correct,
     * the Counter is incremented by 1 with respect to the most recent Data received with Status _INITIAL, _OK, or _OKSOMELOST.
     * This means that no Data has been lost since the last correct data reception.
     */
    E2E_P02STATUS_OK = 0x00,
    /**
     * Error: the Check function has been invoked but no new Data is not available since the last call,
     * according to communication medium (e.g. RTE, COM). As a result, no E2E checks of Data have been consequently executed.
     */
    E2E_P02STATUS_NONEWDATA = 0x01,
    /**
     * Error: The data has been received according to communication medium, but
     */
    E2E_P02STATUS_WRONGCRC =  0x02,
    /**
     * NOT VALID: The new data has been received after detection of an unexpected behavior of counter.
     * The data has a correct CRC and a counter within the expected range with respect to the most recent Data received,
     * but the determined continuity check for the counter is not finalized yet.
     */
    E2E_P02STATUS_SYNC = 0x03,
     /**
      * Initial: The new data has been received according to communication medium, the CRC is correct,
      * but this is the first Data since the receiver's initialization or reinitialization, so the Counter cannot be verified yet.
      */
    E2E_P02STATUS_INITIAL = 0x04,
    /**
     * Error: The new data has been received according to communication medium, the CRC is correct,
     * but the Counter is identical to the most recent Data received with Status _INITIAL, _OK, or _OKSOMELOST.
     */
    E2E_P02STATUS_REPEATED = 0x08,
    /**
     * OK: The new data has been received according to communication medium, the CRC is correct,
     * the Counter is incremented by DeltaCounter (1 < DeltaCounter = MaxDeltaCounter) with respect to
     * the most recent Data received with Status _INITIAL, _OK, or _OKSOMELOST.
     * This means that some Data in the sequence have been probably lost since the last correct/initial reception,
     * but this is within the configured tolerance range.
     */
    E2E_P02STATUS_OKSOMELOST = 0x20,
    /**
     * Error: The new data has been received according to communication medium, the CRC is correct,
     * but the Counter Delta is too big (DeltaCounter > MaxDeltaCounter) with respect to
     * the most recent Data received with Status _INITIAL, _OK, or _OKSOMELOST.
     * This means that too many Data in the sequence have been probably lost since the last correct/initial reception.
     */
    E2E_P02STATUS_WRONGSEQUENCE = 0x40
} E2E_P02CheckStatusType;


/* @req SWS_E2E_00152 */
/**
 * Non-modifiable configuration of the data element sent over an RTE port, for E2E profile 2.
 * The position of the counter and CRC is not configurable in profile 2.
 */
typedef struct {
    /**
     * Length of Data, in bits. The value shall be a multiple of 8.
     */
    uint16 DataLength;
    /**
     * An array of appropriately chosen Data IDs for protection against masquerading.
     */
    uint8 DataIDList[16];
    /**
     * Initial maximum allowed gap between two counter values of two consecutively received valid Data. For example,
     * if the receiver gets Data with counter 1 and MaxDeltaCounterInit is 1, then at the next reception the receiver
     * can accept Counters with values 2 and 3, but not 4. Note that if the receiver does not receive new Data at a consecutive
     * read, then the receiver increments the tolerance by 1.
     */
    uint8 MaxDeltaCounterInit;
    /**
     * The maximum amount of missing or repeated Data which the receiver does not expect to exceed under normal communication conditions.
     */
    uint8 MaxNoNewOrRepeatedData;
    /**
     * Number of Data required for validating the consistency of the counter that must be received with a valid counter
     * (i.e. counter within the allowed lock-in range) after the detection of an unexpected behavior of a received counter.
     */
    uint8 SyncCounterInit;
    /**
     * Offset of the E2E header in the Data[] array in bits. It shall be: 0 ≤ Offset ≤ MaxDataLength-(2*8).
     */
    uint16 Offset;
} E2E_P02ConfigType;


/* @req SWS_E2E_00153 */
/**
 * State of the sender for a Data protected with E2E Profile 2.
 */
typedef struct {
   /**
    * Counter to be used for protecting the Data.
    * The initial value is 0, which means that the first Data will have the counter 0.
    * After the protection by the counter, the counter is incremented modulo 16.
    */
    uint8 Counter;
} E2E_P02ProtectStateType;


/* @req SWS_E2E_00154 */
/**
   State of the sender for a Data protected with E2E Profile 2.
 */
typedef struct {
    /**
     * Counter of last valid received message.
     */
    uint8 LastValidCounter;
    /**
     * MaxDeltaCounter specifies the maximum allowed difference between two counter values
     * of consecutively received valid messages.
     */
    uint8 MaxDeltaCounter;
    /**
     * If true means that no correct data (with correct Data ID and CRC) has been yet received
     * after the receiver initialization or reinitialization.
     */
    boolean WaitForFirstData;
    /**
     * Indicates to E2E Library that a new data is available for Library to be checked.
     * This attribute is set by the E2E Library caller, and not by the E2E Library.
     */
    boolean NewDataAvailable;
    /**
     * Number of data (messages) lost since reception of last valid one.
     */
    uint8 LostData;
    /**
     * Result of the verification of the Data, determined by the Check function.
     */
    E2E_P02CheckStatusType Status;
    /**
     * Number of Data required for validating the consistency of the counter that must be received with a valid counter
     * (i.e. counter within the allowed lock-in range) after the detection of an unexpected behavior of a received counter.
     */
    uint8 SyncCounter;
    /**
     * Amount of consecutive reception cycles in which either (1) there was no new data, or (2) when the data was repeated.
     */
    uint8 NoNewOrRepeatedDataCounter;
} E2E_P02CheckStateType;  //new name E2E_P02CheckStateType according to ASR4.2.2


/** @brief Protects the array/buffer to be transmitted using the E2E profile 2.
 *         This includes checksum calculation, handling of sequence counter and Data ID.
 *
 *  @param ConfigPtr Pointer to static configuration.
 *  @param StatePtr Pointer to port/data communication state.
 *  @param DataPtr Pointer to Data to be transmitted.
 *
 *  @return Error code (E2E_E_INPUTERR_NULL || E2E_E_INPUTERR_WRONG || E2E_E_INTERR || E2E_E_OK).
 */
/* @req SWS_E2E_00047 error flags for errors shall be used by all E2E Library functions */
/* @req SWS_E2E_00160 API for E2E_P02Protect */
Std_ReturnType E2E_P02Protect(const E2E_P02ConfigType* ConfigPtr, E2E_P02ProtectStateType* StatePtr, uint8* DataPtr);


/** @brief Check the array/buffer using the E2E profile 2.
 *         This includes checksum calculation, handling of sequence counter and Data ID.
 *
 *  @param ConfigPtr Pointer to static configuration.
 *  @param StatePtr Pointer to port/data communication state.
 *  @param DataPtr Pointer to received data.
 *
 *  @return Error code (E2E_E_INPUTERR_NULL || E2E_E_INPUTERR_WRONG || E2E_E_INTERR || E2E_E_OK).
 */
/* @req SWS_E2E_00047 error flags for errors shall be used by all E2E Library functions */
/* @req SWS_E2E_00161 API for E2E_P02Check*/
Std_ReturnType E2E_P02Check(const E2E_P02ConfigType* ConfigPtr, E2E_P02CheckStateType* StatePtr, const uint8* DataPtr);


/** @brief Initializes the protection state.
 *
 *  @param StatePtr Pointer to port/data communication state.
 *
 *  @return Error code (E2E_E_INPUTERR_NULL - null pointer passed E2E_E_OK).
 */
/* @req SWS_E2E_00047 error flags for errors shall be used by all E2E Library functions */
/* @req SWS_E2E_00387 API for E2E_P02ProtectInit */
Std_ReturnType E2E_P02ProtectInit(E2E_P02ProtectStateType* StatePtr);


/** @brief Initializes the check state.
 *
 *  @param StatePtr Pointer to port/data communication state.
 *
 *  @return Error code (E2E_E_INPUTERR_NULL - null pointer passed E2E_E_OK).
 */
/* @req SWS_E2E_00047 error flags for errors shall be used by all E2E Library functions */
/* @req SWS_E2E_00391 API for E2E_P02CheckInit */
Std_ReturnType E2E_P02CheckInit(E2E_P02CheckStateType* StatePtr);


/** @brief The function maps the check status of Profile 2 to a generic check status,
 *         which can be used by E2E state machine check function.
 *         The E2E Profile 2 delivers a more fine-granular status,
 *         but this is not relevant for the E2E state machine.
 *
 *  @param CheckReturn Return value of the E2E_P02Check function.
 *  @param Status Status determined by E2E_P02Check function.
 *  @param profileBehavior FALSE: check has the legacy behavior, before R4.2
 *                         TRUE: check behaves like new P4/P5/P6 profiles introduced in R4.2
 *  @return Profile-independent status of the reception on one single Data in one cycle.
 */
/* @req SWS_E2E_00047 error flags for errors shall be used by all E2E Library functions */
/* @req SWS_E2E_00379 API for E2E_P02MapStatusToSM */
E2E_PCheckStatusType E2E_P02MapStatusToSM(Std_ReturnType CheckReturn, E2E_P02CheckStatusType Status, boolean profileBehavior);


#endif
