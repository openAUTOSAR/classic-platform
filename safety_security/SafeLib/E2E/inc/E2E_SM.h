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

/** @file E2E_SM.h
 *
 *  A state machine describing the logical algorithm of E2E monitoring independent of the used profile.
 */

#ifndef E2E_SM_H_
#define E2E_SM_H_


/* @req SWS_E2E_00115 E2E library files (i.e. E2E_*.*) shall not include any RTE files */

/* According to AUTOSAR_SWS_E2ELibrary.pdf, Figure5-1: File dependencies:
 * E2E.h is included from E2E_P01.h files
 */
#include "E2E.h"

/* According to AUTOSAR_SWS_E2ELibrary.pdf, Figure5-1: File dependencies:
 * E2E_MemMap.h is included from all E2E files
 */
#include "E2E_MemMap.h"

#define E2E_SM_AR_RELEASE_MAJOR_VERSION        4u
#define E2E_SM_AR_RELEASE_MINOR_VERSION        3u
#define E2E_SM_AR_RELEASE_REVISION_VERSION     0u

#define E2E_SM_SW_MAJOR_VERSION        2
#define E2E_SM_SW_MINOR_VERSION        0

/* @req SWS_E2E_00327
 * The implementer of the E2E Library shall avoid the integration of incompatible files. Minimum implementation is the version check of the header files.
 */
#if (E2E_AR_RELEASE_MAJOR_VERSION != E2E_SM_AR_RELEASE_MAJOR_VERSION) || (E2E_AR_RELEASE_MINOR_VERSION != E2E_SM_AR_RELEASE_MINOR_VERSION)
#error "SW Version mismatch between E2E.h and E1E_SM.h"
#endif


/* @req SWS_E2E_00347 */
/**
 * Profile-independent status of the reception on one single Data in one cycle.
 */
typedef enum {
    /**
     * OK: the checks of the Data in this cycle were successful (including counter check).
     */
    E2E_P_OK = 0x00,
    /**
     * Data has a repeated counter.
     */
    E2E_P_REPEATED = 0x01,
    /**
     * The checks of the Data in this cycle were successful,
     * with the exception of counter jump, which changed more than the allowed delta.
     */
    E2E_P_WRONGSEQUENCE = 0x02,
    /**
     * Error not related to counters occurred (e.g. wrong crc, wrong length, wrong Data ID)
     * or the return of the check function was not OK.
     */
    E2E_P_ERROR =  0x03,
    /**
     * No value has been received yet (e.g. during initialization).
     * This is used as the initialization value for the buffer, it is not returned by any E2E profile.
     */
    E2E_P_NOTAVAILABLE = 0x04,
    /**
     * No new data is available.
     */
    E2E_P_NONEWDATA = 0x05
} E2E_PCheckStatusType;


/* @req SWS_E2E_00344 */
/**
 * Status of the communication channel exchanging the data. If the status is OK, then the data may be used.
 */
typedef enum {
    /**
     * Communication functioning properly according to E2E, data can be used.
     */
    E2E_SM_VALID = 0x00,
    /**
     * State before E2E_SMCheckInit() is invoked, data cannot be used.
     */
    E2E_SM_DEINIT = 0x01,
    /**
     * No data from the sender is available since the initialization, data cannot be used.
     */
    E2E_SM_NODATA = 0x02,
    /**
     * There has been some data received since startup, but it is not yet possible use it, data cannot be used.
     */
    E2E_SM_INIT =  0x03,
    /**
     * Communication not functioning properly, data cannot be used.
     */
    E2E_SM_INVALID = 0x04
} E2E_SMStateType;


/* @req SWS_E2E_00342 */
/**
 * Configuration of a communication channel for exchanging Data.
 */
typedef struct {
    /**
     * Size of the monitoring window for the state machine.
     */
    uint8 WindowSize;
    /**
     * Minimal number of checks in which ProfileStatus equal to E2E_P_OK was determined within the last WindowSize checks
     * (for the state E2E_SM_INIT) required to change to state E2E_SM_VALID.
     */
    uint8 MinOkStateInit;
    /**
     * Maximal number of checks in which ProfileStatus equal to E2E_P_ERROR was determined,
     * within the last WindowSize checks (for the state E2E_SM_INIT).
     */
    uint8 MaxErrorStateInit;
    /**
     * Minimal number of checks in which ProfileStatus equal to E2E_P_OK was determined
     * within the lastWindowSize checks (for the state E2E_SM_VALID) required to keep in state E2E_SM_VALID.
     */
    uint8 MinOkStateValid;
    /**
     * Maximal number of checks in which ProfileStatus equal to E2E_P_ERROR was determined,
     * within the last WindowSize checks (for the state E2E_SM_VALID).
     */
    uint8 MaxErrorStateValid;
    /**
     * Minimum number of checks in which ProfileStatus equal to E2E_P_OK was determined
     * within the last WindowSize checks (for the state E2E_SM_INVALID) required to change to state E2E_SM_VALID.
     */
    uint8 MinOkStateInvalid;
    /**
     * Maximal number of checks in which ProfileStatus equal to E2E_P_ERROR was determined,
     * within the last WindowSize checks (for the state E2E_SM_INVALID).
     */
    uint8 MaxErrorStateInvalid;
} E2E_SMConfigType;


/* @req SWS_E2E_00343 */
/**
 * State of the protection of a communication channel.
 */
typedef struct {
    /**
     * Pointer to an array, in which the ProfileStatus-es of the last E2E-checks are stored. The array size shall be WindowSize.
     */
    uint8* ProfileStatusWindow;
    /**
     * Index in the array, at which the next ProfileStatus is to be written.
     */
    uint8 WindowTopIndex;
    /**
     * Count of checks in which ProfileStatus equal to E2E_P_OK was determined, within the last WindowSize checks.
     */
    uint8 OkCount;
    /**
     * Count of checks in which ProfileStatus equal to E2E_P_ERROR was determined, within the last WindowSize checks.
     */
    uint8 ErrorCount;
    /**
     * The current state in the state machine.
     * The value is not explicitly used in the pseudocode of the state machine,
     * because it is expressed in UML as UML states.
     */
    E2E_SMStateType SMState;
} E2E_SMCheckStateType;


/** @brief Checks the communication channel.
 *         It determines if the data can be used for safety-related application,
 *         based on history of checks performed by a corresponding E2E_P0XCheck() function.
 *
 *  @param ProfileStatus Profile-independent status of the reception on one single Data in one cycle.
 *  @param ConfigPtr Pointer to configuration of the state machine.
 *  @param StatePtr Pointer to port/data communication state.
 *
 *  @return Error code (E2E_E_INPUTERR_NULL || E2E_E_INPUTERR_WRONG || E2E_E_INTERR || E2E_E_OK || E2E_E_WRONGSTATE).
 */
/* @req SWS_E2E_00047 error flags for errors shall be used by all E2E Library functions */
/* @req SWS_E2E_00340 API for E2E_SMCheck */
Std_ReturnType E2E_SMCheck(E2E_PCheckStatusType ProfileStatus, const E2E_SMConfigType* ConfigPtr, E2E_SMCheckStateType* StatePtr);


/** @brief Initializes the state machine.
 *
 *  @param ConfigPtr Pointer to configuration of the state machine.
 *  @param StatePtr Pointer to port/data communication state.
 *
 *  @return Error code (E2E_E_INPUTERR_NULL - null pointer passed E2E_E_OK).
 */
/* @req SWS_E2E_00047 error flags for errors shall be used by all E2E Library functions */
/* @req SWS_E2E_00353 API for E2E_SMCheckInit */
/* Initializes the state machine. */
Std_ReturnType E2E_SMCheckInit(E2E_SMCheckStateType* StatePtr, const E2E_SMConfigType* ConfigPtr);


#endif
