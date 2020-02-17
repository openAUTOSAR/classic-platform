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

/** @file E2E_P01.h
 *
 *  The E2E Profile 1 provides a consistent set of data protection mechanisms,
 *  designed to protecting against the faults considered in the fault model.
 */

#ifndef E2E_P01_H_
#define E2E_P01_H_

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

#define E2E_P01_AR_RELEASE_MAJOR_VERSION        4u
#define E2E_P01_AR_RELEASE_MINOR_VERSION        3u
#define E2E_P01_AR_RELEASE_REVISION_VERSION     0u

#define E2E_P01_SW_MAJOR_VERSION        2
#define E2E_P01_SW_MINOR_VERSION        0

/* @req SWS_E2E_00327
 * The implementer of the E2E Library shall avoid the integration of incompatible files. Minimum implementation is the version check of the header files.
 */
#if (E2E_AR_RELEASE_MAJOR_VERSION != E2E_P01_AR_RELEASE_MAJOR_VERSION) || (E2E_AR_RELEASE_MINOR_VERSION != E2E_P01_AR_RELEASE_MINOR_VERSION)
#error "SW Version mismatch between E2E.h and E1E_P01.h"
#endif

#if (E2E_SM_AR_RELEASE_MAJOR_VERSION != E2E_P01_AR_RELEASE_MAJOR_VERSION) || (E2E_SM_AR_RELEASE_MINOR_VERSION != E2E_P01_AR_RELEASE_MINOR_VERSION)
#error "SW Version mismatch between E2E_SM.h and E1E_P01.h"
#endif


/* @req SWS_E2E_00200 */
/**
 * The Data ID is two bytes long in E2E Profile 1.
 * There are four inclusion modes how the implicit two-byte Data ID is included in the one-byte CRC.
 */
typedef enum {
    /**
     * Two bytes are included in the CRC (double ID configuration) This is used in E2E variant 1A.
     */
    E2E_P01_DATAID_BOTH = 0x0,
     /**
      * One of the two bytes byte is included,
      * alternating high and low byte, depending on parity of the counter (alternating ID configuration).
      * For an even counter, the low byte is included. For an odd counter, the high byte is included.
      * This is used in E2E variant 1B.
      */
    E2E_P01_DATAID_ALT = 0x1,
     /**
      * Only the low byte is included, the high byte is never used.
      * This is applicable if the IDs in a particular system are 8 bits.
      */
    E2E_P01_DATAID_LOW = 0x2,
    /**
     * The low byte is included in the implicit CRC calculation,
     * the low nibble of the high byte is transmitted along with the data (i.e. it is explicitly included),
     * the high nibble of the high byte is not used.
     * This is applicable for the IDs up to 12 bits. This is used in E2E variant 1C.
     */
    E2E_P01_DATAID_NIBBLE = 0x3
} E2E_P01DataIDMode;


/* @req SWS_E2E_00085
 * In E2E Profile 1, with E2E_P01DataIDMode equal to E2E_P01_DATAID_BOTH or E2E_P01_DATAID_ALT the length of the Data
 * ID shall be 16 bits */
/* @req SWS_E2E_00227  Profile variant 1A, No explicit support for variants but the configuration allows such a configuration
 * The E2E Profile variant 1A is defined as follows:
 * 1. CRC is the 0th byte in the signal group (i.e. starts with bit offset 0)
 * 2. Alive counter is located in lowest 4 bits of 1st byte (i.e. starts with bit offset 8)
 * 3. E2E_P01DataIDMode = E2E_P01_DATAID_BOTH
 * 4. SignalIPdu.unusedBitPattern = 0xFF.
 */
/* @req SWS_E2E_00228  Profile variant 1B, No explicit support for variants but the configuration allows such a configuration
 * The E2E Profile variant 1B is defined as follows:
 * 1. CRC is the 0th byte in the signal group (i.e. starts with bit offset 0)
 * 2. Alive counter is located in lowest 4 bits of 1st byte (i.e. starts with bit offset 8)
 * 3. E2E_P01DataIDMode = E2E_P01_DATAID_ALTERNATING
 * 4. SignalIPdu.unusedBitPattern = 0xFF.
 */
/* @req SWS_E2E_00307  Profile variant 1C, No explicit support for variants but the configuration allows such a configuration
 * The E2E Profile variant 1C is defined as follows:
 * 1. CRC is the 0th byte in the signal group (i.e. starts with bit offset 0)
 * 2. Alive counter is located in lowest 4 bits of 1st byte (i.e. starts with bit offset 8)
 * 3. The Data ID nibble is located in the highest 4 bits of 1st byte (i.e. starts with bit offset 12)
 * 4. E2E_P01DataIDMode = E2E_P01_DATAID_NIBBLE
 * 5. SignalIPdu.unusedBitPattern = 0xFF.
 */
/* @req SWS_E2E_00018 */
/**
 * Configuration of transmitted Data (Data Element or I-PDU), for E2E Profile 1.
 * For each transmitted Data, there is an instance of this typedef.
 */
typedef struct  {
    /**
     * Bit offset of Counter in MSB first order. In variants 1A and 1B, CounterOffset is 8.
     * The offset shall be a multiple of 4.
     */
    uint16 CounterOffset;
    /**
     * Bit offset of CRC (i.e. since *Data) in MSB first order. In variants 1A and 1B, CRCOffset is 0.
     * The offset shall be a multiple of 8.
     */
    uint16 CRCOffset;
    /**
     * A unique identifier, for protection against masquerading. There are some constraints on the selection of ID values,
     * described in section "Configuration constraints on Data IDs".
     */
    uint16 DataID;
    /**
     * Bit offset of the low nibble of the high byte of Data ID. This parameter is used by E2E Library only if
     * DataIDMode = E2E_P01_DATAID_NIBBLE (otherwise it is ignored by E2E Library).
     * For DataIDMode different than E2E_P01_DATAID_NIBBLE, DataIDNibbleOffset shall be initialized to 0 (even if it is ignored by E2E Library).
     */
    uint16 DataIDNibbleOffset;
    /**
     * Inclusion mode of ID in CRC computation (both bytes, alternating, or low byte only of ID included).
     */
    E2E_P01DataIDMode DataIDMode;
    /**
     * Length of data, in bits. The value shall be a multiple of 8 and shall be ≤ 240.
     */
    uint16 DataLength;
    /**
     * Initial maximum allowed gap between two counter values of two consecutively received valid Data.
     * For example, if the receiver gets Data with counter 1 and MaxDeltaCounterInit is 1,
     * then at the next reception the receiver can accept Counters with values 2 and 3, but not 4.
     * Note that if the receiver does not receive new Data at a consecutive read, then the receiver increments the tolerance by 1.
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
} E2E_P01ConfigType;


/* @req SWS_E2E_00020 */
/**
 * State of the sender for a Data protected with E2E Profile 1.
 */
typedef struct {
    /**
     * Counter to be used for protecting the next Data.
     * The initial value is 0, which means that the first Data will have the counter 0.
     * After the protection by the Counter, the Counter is incremented modulo 0xF.
     * The value 0xF is skipped (after 0xE the next is 0x0), as 0xF value represents the error value.
     * The four high bits are always 0.
     */
    uint8 Counter;
} E2E_P01ProtectStateType;


/* @req SWS_E2E_00022 */
/**
 * Result of the verification of the Data in E2E Profile 1, determined by the Check function.
 */
typedef enum {
    /**
     * OK: The new data has been received according to communication medium, the CRC is correct,
     * the Counter is incremented by 1 with respect to the most recent Data received with Status _INITIAL, _OK, or _OKSOMELOST.
     * This means that no Data has been lost since the last correct data reception.
     */
     E2E_P01STATUS_OK = 0x00,
     /**
      * Error: the Check function has been invoked but no new Data is not available since the last call,
      * according to communication medium (e.g. RTE, COM). As a result, no E2E checks of Data have been consequently executed.
      */
     E2E_P01STATUS_NONEWDATA = 0x1,
     /**
      * Error: The data has been received according to communication medium, but
      * 1. the CRC is incorrect (applicable for all E2E Profile 1 configurations) or
      * 2. the low nibble of the high byte of Data ID is incorrect (applicable only for E2E Profile 1 with E2E_P01DataIDMode = E2E_P01_DATAID_NIBBLE).
      * The two above errors can be a result of corruption, incorrect addressing or masquerade.
      */
     E2E_P01STATUS_WRONGCRC = 0x2,
     /**
      * NOT VALID: The new data has been received after detection of an unexpected behavior of counter.
      * The data has a correct CRC and a counter within the expected range with respect to the most recent Data received,
      * but the determined continuity check for the counter is not finalized yet.
      */
     E2E_P01STATUS_SYNC = 0x03,
     /**
      * Initial: The new data has been received according to communication medium, the CRC is correct,
      * but this is the first Data since the receiver's initialization or reinitialization, so the Counter cannot be verified yet.
      */
     E2E_P01STATUS_INITIAL = 0x4,
     /**
      * Error: The new data has been received according to communication medium, the CRC is correct,
      * but the Counter is identical to the most recent Data received with Status _INITIAL, _OK, or _OKSOMELOST.
      */
     E2E_P01STATUS_REPEATED = 0x8,
     /**
      * OK: The new data has been received according to communication medium, the CRC is correct,
      * the Counter is incremented by DeltaCounter (1 < DeltaCounter = MaxDeltaCounter) with respect to
      * the most recent Data received with Status _INITIAL, _OK, or _OKSOMELOST.
      * This means that some Data in the sequence have been probably lost since the last correct/initial reception,
      * but this is within the configured tolerance range.
      */
     E2E_P01STATUS_OKSOMELOST = 0x20,
     /**
      * Error: The new data has been received according to communication medium, the CRC is correct,
      * but the Counter Delta is too big (DeltaCounter > MaxDeltaCounter) with respect to
      * the most recent Data received with Status _INITIAL, _OK, or _OKSOMELOST.
      * This means that too many Data in the sequence have been probably lost since the last correct/initial reception.
      */
     E2E_P01STATUS_WRONGSEQUENCE = 0x40
} E2E_P01CheckStatusType;


/* @req SWS_E2E_00021 */
/**
 * State of the receiver for a Data protected with E2E Profile 1.
 */
typedef struct {
    /**
     * Counter value most recently received. If no data has been yet received, then the value is 0x0.
     * After each reception, the counter is updated with the value received.
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
     * Number of data (messages) lost since reception of last valid one. This attribute is set only if Status equals
     * E2E_P01STATUS_OK or E2E_P01STATUS_OKSOMELOST. For other values of Status, the value of LostData is undefined.
     * E2E_P01CheckStatusType Status Result of the verification of the Data, determined by the Check function.
     */
    uint8 LostData;
    /**
     * Result of the verification of the Data, determined by the Check function.
     */
    E2E_P01CheckStatusType Status;
    /**
     * Number of Data required for validating the consistency of the counter that must be received with a valid counter
     * (i.e. counter within the allowed lock-in range) after the detection of an unexpected behavior of a received counter.
     */
    uint8 SyncCounter;
    /**
     * Amount of consecutive reception cycles in which either (1) there was no new data, or (2) when the data was repeated.
     */
    uint8 NoNewOrRepeatedDataCounter;
} E2E_P01CheckStateType;  //new name E2E_P01CheckStateType according to ASR4.2.2


/** @brief Protects the array/buffer to be transmitted using the E2E profile 1.
 *         This includes checksum calculation, handling of counter and Data ID.
 *
 *  @param ConfigPtr Pointer to static configuration.
 *  @param StatePtr Pointer to port/data communication state.
 *  @param DataPtr Pointer to Data to be transmitted.
 *
 *  @return Error code (E2E_E_INPUTERR_NULL || E2E_E_INPUTERR_WRONG || E2E_E_INTERR || E2E_E_OK).
 */
/* @req SWS_E2E_00047 error flags for errors shall be used by all E2E Library functions */
/* @req SWS_E2E_00166 API for E2E_P01Protect */
Std_ReturnType E2E_P01Protect(const E2E_P01ConfigType* ConfigPtr, E2E_P01ProtectStateType* StatePtr, uint8* DataPtr);


/** @brief Checks the Data received using the E2E profile 1.
 *
 *  @param ConfigPtr Pointer to static configuration.
 *  @param StatePtr Pointer to port/data communication state.
 *  @param DataPtr Pointer to received data.
 *
 *  @return Error code (E2E_E_INPUTERR_NULL || E2E_E_INPUTERR_WRONG || E2E_E_INTERR || E2E_E_OK).
 */
/* @req SWS_E2E_00047 error flags for errors shall be used by all E2E Library functions */
/* @req SWS_E2E_00158 API for E2E_P01Check */
Std_ReturnType E2E_P01Check(const E2E_P01ConfigType* ConfigPtr, E2E_P01CheckStateType* StatePtr, const uint8* DataPtr);


/** @brief Initializes the protection state.
 *
 *  @param StatePtr Pointer to port/data communication state.
 *
 *  @return Error code (E2E_E_INPUTERR_NULL - null pointer passed E2E_E_OK).
 */
/* @req SWS_E2E_00047 error flags for errors shall be used by all E2E Library functions */
/* @req SWS_E2E_00385 API for E2E_P01ProtectInit */
Std_ReturnType E2E_P01ProtectInit(E2E_P01ProtectStateType* StatePtr);


/** @brief Initializes the check state.
 *
 *  @param StatePtr Pointer to port/data communication state.
 *
 *  @return Error code (E2E_E_INPUTERR_NULL - null pointer passed E2E_E_OK).
 */
/* @req SWS_E2E_00047 error flags for errors shall be used by all E2E Library functions */
/* @req SWS_E2E_00390 API for E2E_P01CheckInit */
Std_ReturnType E2E_P01CheckInit(E2E_P01CheckStateType* StatePtr);


/** @brief The function maps the check status of Profile 1 to a generic check status,
 *         which can be used by E2E state machine check function.
 *         The E2E Profile 1 delivers a more fine-granular status,
 *         but this is not relevant for the E2E state machine.
 *
 *  @param CheckReturn Return value of the E2E_P01Check function.
 *  @param Status Status determined by E2E_P01Check function.
 *  @param profileBehavior FALSE: check has the legacy behavior, before R4.2
 *                         TRUE: check behaves like new P4/P5/P6 profiles introduced in R4.2
 *  @return Profile-independent status of the reception on one single Data in one cycle.
 */
/* @req SWS_E2E_00047 error flags for errors shall be used by all E2E Library functions */
/* @req SWS_E2E_00382 API for E2E_P01MapStatusToSM */
E2E_PCheckStatusType E2E_P01MapStatusToSM(Std_ReturnType CheckReturn, E2E_P01CheckStatusType Status, boolean profileBehavior);


#endif
