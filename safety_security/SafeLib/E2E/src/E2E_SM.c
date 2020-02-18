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
/* @req SWS_E2E_00049 The E2E library shall not contain library-internal mechanisms for error detection to be traced as development errors. */

#include "E2E_SM.h"

#define E2E_SM_AR_RELEASE_MAJOR_VERSION_INT     4u
#define E2E_SM_AR_RELEASE_MINOR_VERSION_INT     3u
#define E2E_SM_AR_RELEASE_REVISION_VERSION_INT  0u

#define E2E_SM_SW_MAJOR_VERSION_INT        2
#define E2E_SM_SW_MINOR_VERSION_INT        0

/* @req SWS_E2E_00327
 * The implementer of the E2E Library shall avoid the integration of incompatible files. Minimum implementation is the version check of the header files.
 */
#if (E2E_SM_SW_MAJOR_VERSION != E2E_SM_SW_MAJOR_VERSION_INT) || (E2E_SM_SW_MINOR_VERSION != E2E_SM_SW_MINOR_VERSION_INT)
#error "SW Version mismatch between E2E_SM.c and E2E_SM.h"
#endif

#if (E2E_SM_AR_RELEASE_MAJOR_VERSION != E2E_SM_AR_RELEASE_MAJOR_VERSION_INT) || (E2E_SM_AR_RELEASE_MINOR_VERSION != E2E_SM_AR_RELEASE_MINOR_VERSION_INT) || \
    (E2E_SM_AR_RELEASE_REVISION_VERSION != E2E_SM_AR_RELEASE_REVISION_VERSION_INT)
#error "AR Version mismatch between E2E_SM.c and E2E_SM.h"
#endif


/* @req SWS_E2E_00467 The step E2E_SMClearStatus(ProfileStatus, State) in E2E_SMCheck()
 * shall have the following behavior: */

/* Clear all in the observed window.
 */
static INLINE void E2E_SMClearStatus(E2E_SMCheckStateType* StatePtr, uint8 windowSize) {

    uint8 i;

    for (i=0; i<windowSize; i++) {
        StatePtr->ProfileStatusWindow[i] = (uint8)E2E_P_NOTAVAILABLE;
    }

    StatePtr->OkCount = 0;
    StatePtr->ErrorCount = 0;
    StatePtr->WindowTopIndex = 0;
}


/* @req SWS_E2E_00466 The step E2E_SMAddStatus(ProfileStatus, State) in E2E_SMCheck()
 * shall have the following behavior: */

/* Adds the new status to the observed window
 */
static INLINE void E2E_SMAddStatus(E2E_PCheckStatusType ProfileStatus, E2E_SMCheckStateType* StatePtr, uint8 windowSize) {

    uint8 i;
    uint8 count;

    StatePtr->ProfileStatusWindow[StatePtr->WindowTopIndex] = (uint8)ProfileStatus;

    count=0;
    for (i=0; i<windowSize; i++) {
        if (StatePtr->ProfileStatusWindow[i] == (uint8)E2E_P_OK) {
            count++;
        }
    }

    StatePtr->OkCount = count;

    count=0;
    for (i=0; i<windowSize; i++) {
        if (StatePtr->ProfileStatusWindow[i] == (uint8)E2E_P_ERROR) {
            count++;
        }
    }

    StatePtr->ErrorCount = count;

    if (StatePtr->WindowTopIndex >= (windowSize-1)) {
        StatePtr->WindowTopIndex = 0;
    }
    else {
        StatePtr->WindowTopIndex++;
    }
}


/* Checks and Set status for state NODATA
 */
static INLINE void E2E_SMCheck_NODATA(E2E_PCheckStatusType ProfileStatus, E2E_SMCheckStateType* StatePtr) {

    if ((ProfileStatus != E2E_P_ERROR) && (ProfileStatus != E2E_P_NONEWDATA)) {
        StatePtr->SMState = E2E_SM_INIT;
    }
    else {
        StatePtr->SMState = E2E_SM_NODATA;
    }
}


/* Checks and Set status for state INIT
 */
static INLINE void E2E_SMCheck_INIT(const E2E_SMConfigType* ConfigPtr, E2E_SMCheckStateType* StatePtr) {

    if ((StatePtr->ErrorCount <= ConfigPtr->MaxErrorStateInit) && (StatePtr->OkCount >= ConfigPtr->MinOkStateInit)) {
        StatePtr->SMState = E2E_SM_VALID;
    }
    else if (StatePtr->ErrorCount > ConfigPtr->MaxErrorStateInit) {
        StatePtr->SMState = E2E_SM_INVALID;
    }
    else {
        StatePtr->SMState = E2E_SM_INIT;
    }
}


/* Checks and Set status for state VALID
 */
static INLINE void E2E_SMCheck_VALID(const E2E_SMConfigType* ConfigPtr, E2E_SMCheckStateType* StatePtr) {

    if ((StatePtr->ErrorCount <= ConfigPtr->MaxErrorStateValid) && (StatePtr->OkCount >= ConfigPtr->MinOkStateValid)) {
        StatePtr->SMState = E2E_SM_VALID;
    }
    else {
        StatePtr->SMState = E2E_SM_INVALID;
    }
}


/* Checks and Set status for state INVALID
 */
static INLINE void E2E_SMCheck_INVALID( const E2E_SMConfigType* ConfigPtr, E2E_SMCheckStateType* StatePtr ) {

    if ((StatePtr->ErrorCount <= ConfigPtr->MaxErrorStateInvalid) && (StatePtr->OkCount >= ConfigPtr->MinOkStateInvalid)) {
        StatePtr->SMState = E2E_SM_VALID;
    }
    else {
        StatePtr->SMState = E2E_SM_INVALID;
    }
}


/* @req SWS_E2E_00340 API for E2E_SMCheck */
/* @req SWS_E2E_00345 The E2E State machine shall have the following behavior with respect to the function E2E_SMCheck(): */
/* @req SWS_E2E_00354 The E2E state machine shall be implemented by the functions E2E_SMCheck() and E2E_SMCheckInit() */
/* @req SWS_E2E_00371 In case State is NULL or Config is NULL,
 * the function E2E_SMCheck shall return immediately with E2E_E_INPUTERR_NULL. */

/* Checks the communication channel. It determines if the data can be used for safety-related application,
 * based on history of checks performed by a corresponding E2E_P0XCheck() function.
 */
Std_ReturnType E2E_SMCheck(E2E_PCheckStatusType ProfileStatus, const E2E_SMConfigType* ConfigPtr, E2E_SMCheckStateType* StatePtr) {

    Std_ReturnType status;
    status = E2E_E_OK;

    if ((StatePtr == NULL_PTR) || (ConfigPtr == NULL_PTR)) {
        status = E2E_E_INPUTERR_NULL;
    }

    /* Size of the monitoring window for the state machine can also be 0, check here consistency */
    else if ((StatePtr->ProfileStatusWindow == NULL_PTR) && (ConfigPtr->WindowSize>0)) {
        status = E2E_E_INPUTERR_WRONG;
    }

    else if (StatePtr->SMState == E2E_SM_DEINIT) {
        status = E2E_E_WRONGSTATE;
    }

    else {
        switch (StatePtr->SMState) {

            case E2E_SM_NODATA:
                E2E_SMCheck_NODATA(ProfileStatus, StatePtr);
                break;

            case E2E_SM_INIT:
                E2E_SMAddStatus(ProfileStatus, StatePtr, ConfigPtr->WindowSize);
                E2E_SMCheck_INIT(ConfigPtr, StatePtr );
                break;

            case E2E_SM_VALID:
                E2E_SMAddStatus(ProfileStatus, StatePtr, ConfigPtr->WindowSize);
                E2E_SMCheck_VALID(ConfigPtr, StatePtr);
                break;

            case E2E_SM_INVALID:
                E2E_SMAddStatus(ProfileStatus, StatePtr, ConfigPtr->WindowSize);
                E2E_SMCheck_INVALID(ConfigPtr, StatePtr);
                break;
            /* @CODECOV:DEFAULT_CASE:Default statement is required for defensive programming. If this happens the state will be restored to E2E_SM_DEINIT */
            __CODE_COVERAGE_IGNORE__
            default:
                /*Do nothing*/
                break;
        }
    }

    return status;
}


/* @req SWS_E2E_00353 API for E2E_SMCheckInit */
/* @req SWS_E2E_00354 The E2E state machine shall be implemented by the functions E2E_SMCheck() and E2E_SMCheckInit() */
/* @req SWS_E2E_00370 In case State is NULL or Config is NULL,
 * the function E2E_SMCheckInit shall return immediately with E2E_E_INPUTERR_NULL. */
/* @req SWS_E2E_00375 The E2E State machine shall have the following behavior with
 * respect to the function E2E_SMCheckInit(): */

/* Initializes the state machine.
 */
Std_ReturnType E2E_SMCheckInit(E2E_SMCheckStateType* StatePtr, const E2E_SMConfigType* ConfigPtr) {

    Std_ReturnType status;
    status = E2E_E_OK;

    if ((StatePtr == NULL_PTR) || (ConfigPtr == NULL_PTR)) {
        status = E2E_E_INPUTERR_NULL;
    }

    /* Size of the monitoring window for the state machine can also be 0, check here consistency */
    else if ((StatePtr->ProfileStatusWindow == NULL_PTR) && (ConfigPtr->WindowSize>0)) {
        status = E2E_E_INPUTERR_WRONG;
    }

    else {
        E2E_SMClearStatus(StatePtr, ConfigPtr->WindowSize);
        StatePtr->SMState=E2E_SM_NODATA;
    }

    return status;
}
