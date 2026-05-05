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

/*
 * DTC (Diagnostic Trouble Code) Manager
 *
 * Implements DTC storage, status byte management per ISO 14229,
 * aging counter management, snapshot/freeze frame data capture,
 * event-based status transitions, and clearing/reporting functions.
 */

#include <string.h>
#include "Dtc_Manager.h"

/*
 * Module-internal state
 */
static Dtc_EntryType dtcTable[DTC_MAX_COUNT];
static uint16 dtcCount;
static boolean dtcInitialized;

/* --------------------------------------------------------------------
 * Internal helper: find index of a DTC by number.
 * Returns DTC_MAX_COUNT if not found.
 * -------------------------------------------------------------------- */
static uint16 Dtc_FindIndex(uint32 dtcNumber)
{
    uint16 idx;
    for (idx = 0u; idx < dtcCount; idx++) {
        if ((dtcTable[idx].active == TRUE) &&
            (dtcTable[idx].dtcNumber == dtcNumber)) {
            return idx;
        }
    }
    return DTC_MAX_COUNT;
}

/* --------------------------------------------------------------------
 * Internal helper: allocate a new slot for a DTC.
 * Returns DTC_MAX_COUNT if table is full.
 * -------------------------------------------------------------------- */
static uint16 Dtc_AllocateSlot(void)
{
    uint16 idx;
    /* First try to reuse an inactive slot */
    for (idx = 0u; idx < dtcCount; idx++) {
        if (dtcTable[idx].active == FALSE) {
            return idx;
        }
    }
    /* Otherwise append if space available */
    if (dtcCount < DTC_MAX_COUNT) {
        idx = dtcCount;
        dtcCount++;
        return idx;
    }
    return DTC_MAX_COUNT;
}

/* --------------------------------------------------------------------
 * Internal helper: capture snapshot data into a DTC entry.
 * -------------------------------------------------------------------- */
static void Dtc_CaptureSnapshot(Dtc_EntryType *entry, const uint8 *data,
                                uint8 length)
{
    uint8 copyLen;

    if ((entry == NULL) || (data == NULL) || (length == 0u)) {
        return;
    }

    copyLen = (length > DTC_SNAPSHOT_DATA_SIZE)
              ? (uint8)DTC_SNAPSHOT_DATA_SIZE
              : length;

    (void)memcpy(entry->snapshotData, data, (size_t)copyLen);
    entry->snapshotLength = copyLen;
}

/* ====================================================================
 * Public API
 * ==================================================================== */

/* --------------------------------------------------------------------
 * Dtc_Manager_Init
 * Initializes the DTC manager, clearing all stored DTCs.
 * -------------------------------------------------------------------- */
Std_ReturnType Dtc_Manager_Init(void)
{
    uint16 idx;
    for (idx = 0u; idx < DTC_MAX_COUNT; idx++) {
        dtcTable[idx].dtcNumber       = 0u;
        dtcTable[idx].statusByte      = DTC_STATUS_TEST_NOT_COMPLETED_SINCE_LAST_CLEAR
                                      | DTC_STATUS_TEST_NOT_COMPLETED_THIS_OP_CYCLE;
        dtcTable[idx].agingCounter    = 0u;
        dtcTable[idx].occurrenceCounter = 0u;
        dtcTable[idx].active          = FALSE;
        dtcTable[idx].snapshotLength  = 0u;
        (void)memset(dtcTable[idx].snapshotData, 0,
                     (size_t)DTC_SNAPSHOT_DATA_SIZE);
    }
    dtcCount = 0u;
    dtcInitialized = TRUE;
    return DTC_RET_OK;
}

/* --------------------------------------------------------------------
 * Dtc_ReportEvent
 * Reports a test result (PASSED / FAILED) for a given DTC number.
 * On FAILED, creates the DTC if it does not exist, captures snapshot
 * data, and transitions the status byte per ISO 14229.
 * On PASSED, clears the testFailed bit and manages aging.
 * -------------------------------------------------------------------- */
Std_ReturnType Dtc_ReportEvent(uint32 dtcNumber, Dtc_EventStatusType eventStatus,
                               const uint8 *snapshotData, uint8 snapshotLength)
{
    uint16 idx;
    Dtc_EntryType *entry;

    if (dtcInitialized != TRUE) {
        return DTC_RET_NOT_OK;
    }

    if (dtcNumber == 0u) {
        return DTC_RET_NOT_OK;
    }

    idx = Dtc_FindIndex(dtcNumber);

    if (eventStatus == DTC_EVENT_FAILED) {
        /* --- FAILED path --- */
        if (idx == DTC_MAX_COUNT) {
            /* DTC not yet stored – allocate */
            idx = Dtc_AllocateSlot();
            if (idx == DTC_MAX_COUNT) {
                return DTC_RET_NOT_OK;   /* table full */
            }
            dtcTable[idx].dtcNumber        = dtcNumber;
            dtcTable[idx].statusByte       = 0u;
            dtcTable[idx].agingCounter     = 0u;
            dtcTable[idx].occurrenceCounter = 0u;
            dtcTable[idx].active           = TRUE;
            dtcTable[idx].snapshotLength   = 0u;
        }

        entry = &dtcTable[idx];

        /* ISO 14229 status transitions on FAILED */
        entry->statusByte |= DTC_STATUS_TEST_FAILED;
        entry->statusByte |= DTC_STATUS_TEST_FAILED_THIS_OP_CYCLE;
        entry->statusByte |= DTC_STATUS_PENDING_DTC;
        entry->statusByte |= DTC_STATUS_CONFIRMED_DTC;
        entry->statusByte |= DTC_STATUS_TEST_FAILED_SINCE_LAST_CLEAR;
        entry->statusByte &= (uint8)~DTC_STATUS_TEST_NOT_COMPLETED_SINCE_LAST_CLEAR;
        entry->statusByte &= (uint8)~DTC_STATUS_TEST_NOT_COMPLETED_THIS_OP_CYCLE;

        /* Reset aging counter on every new failure */
        entry->agingCounter = 0u;

        /* Increment occurrence counter (saturate at 0xFFFF) */
        if (entry->occurrenceCounter < 0xFFFFu) {
            entry->occurrenceCounter++;
        }

        /* Capture freeze-frame / snapshot data (NULL is allowed – skip) */
        if (snapshotData != NULL) {
            Dtc_CaptureSnapshot(entry, snapshotData, snapshotLength);
        }

    } else if (eventStatus == DTC_EVENT_PASSED) {
        /* --- PASSED path --- */
        if (idx < DTC_MAX_COUNT) {
            entry = &dtcTable[idx];
            /* Clear testFailed bit */
            entry->statusByte &= (uint8)~DTC_STATUS_TEST_FAILED;
            /* Clear testNotCompletedThisOpCycle */
            entry->statusByte &= (uint8)~DTC_STATUS_TEST_NOT_COMPLETED_THIS_OP_CYCLE;
            /* Clear testNotCompletedSinceLastClear */
            entry->statusByte &= (uint8)~DTC_STATUS_TEST_NOT_COMPLETED_SINCE_LAST_CLEAR;
        }
        /* If DTC not found, PASSED for unknown DTC is silently ignored */
    } else {
        return DTC_RET_NOT_OK;   /* invalid event status */
    }

    return DTC_RET_OK;
}

/* --------------------------------------------------------------------
 * Dtc_ClearAll
 * Clears all stored DTCs and resets the table.
 * -------------------------------------------------------------------- */
Std_ReturnType Dtc_ClearAll(void)
{
    if (dtcInitialized != TRUE) {
        return DTC_RET_NOT_OK;
    }

    return Dtc_Manager_Init();   /* Re-init clears everything */
}

/* --------------------------------------------------------------------
 * Dtc_ClearById
 * Clears a single DTC by its number.
 * -------------------------------------------------------------------- */
Std_ReturnType Dtc_ClearById(uint32 dtcNumber)
{
    uint16 idx;

    if (dtcInitialized != TRUE) {
        return DTC_RET_NOT_OK;
    }

    if (dtcNumber == 0u) {
        return DTC_RET_NOT_OK;
    }

    idx = Dtc_FindIndex(dtcNumber);
    if (idx == DTC_MAX_COUNT) {
        return DTC_RET_NOT_OK;   /* DTC not found */
    }

    dtcTable[idx].dtcNumber        = 0u;
    dtcTable[idx].statusByte       = DTC_STATUS_TEST_NOT_COMPLETED_SINCE_LAST_CLEAR
                                   | DTC_STATUS_TEST_NOT_COMPLETED_THIS_OP_CYCLE;
    dtcTable[idx].agingCounter     = 0u;
    dtcTable[idx].occurrenceCounter = 0u;
    dtcTable[idx].active           = FALSE;
    dtcTable[idx].snapshotLength   = 0u;
    (void)memset(dtcTable[idx].snapshotData, 0, (size_t)DTC_SNAPSHOT_DATA_SIZE);

    return DTC_RET_OK;
}

/* --------------------------------------------------------------------
 * Dtc_GetStatus
 * Retrieves the ISO 14229 status byte for the given DTC.
 * -------------------------------------------------------------------- */
Std_ReturnType Dtc_GetStatus(uint32 dtcNumber, uint8 *statusByte)
{
    uint16 idx;

    if (dtcInitialized != TRUE) {
        return DTC_RET_NOT_OK;
    }

    if (statusByte == NULL) {
        return DTC_RET_NOT_OK;
    }

    if (dtcNumber == 0u) {
        return DTC_RET_NOT_OK;
    }

    idx = Dtc_FindIndex(dtcNumber);
    if (idx == DTC_MAX_COUNT) {
        return DTC_RET_NOT_OK;
    }

    *statusByte = dtcTable[idx].statusByte;
    return DTC_RET_OK;
}

/* --------------------------------------------------------------------
 * Dtc_GetSnapshot
 * Retrieves the stored snapshot / freeze-frame data for a DTC.
 * -------------------------------------------------------------------- */
Std_ReturnType Dtc_GetSnapshot(uint32 dtcNumber, uint8 *destBuffer, uint8 *length)
{
    uint16 idx;

    if (dtcInitialized != TRUE) {
        return DTC_RET_NOT_OK;
    }

    if (destBuffer == NULL) {
        return DTC_RET_NOT_OK;
    }

    if (length == NULL) {
        return DTC_RET_NOT_OK;
    }

    if (dtcNumber == 0u) {
        return DTC_RET_NOT_OK;
    }

    idx = Dtc_FindIndex(dtcNumber);
    if (idx == DTC_MAX_COUNT) {
        return DTC_RET_NOT_OK;
    }

    if (dtcTable[idx].snapshotLength == 0u) {
        *length = 0u;
        return DTC_RET_OK;
    }

    (void)memcpy(destBuffer, dtcTable[idx].snapshotData,
                 (size_t)dtcTable[idx].snapshotLength);
    *length = dtcTable[idx].snapshotLength;

    return DTC_RET_OK;
}

/* --------------------------------------------------------------------
 * Dtc_AgeCycle
 * Advances the aging counter for every confirmed DTC that is no longer
 * testFailed.  When the counter reaches the aging threshold the DTC is
 * automatically cleared.
 * -------------------------------------------------------------------- */
Std_ReturnType Dtc_AgeCycle(void)
{
    uint16 idx;
    Dtc_EntryType *entry;

    if (dtcInitialized != TRUE) {
        return DTC_RET_NOT_OK;
    }

    for (idx = 0u; idx < dtcCount; idx++) {
        entry = &dtcTable[idx];
        if (entry->active != TRUE) {
            continue;
        }

        /* Only age confirmed DTCs where testFailed is NOT currently set */
        if (((entry->statusByte & DTC_STATUS_CONFIRMED_DTC) != 0u) &&
            ((entry->statusByte & DTC_STATUS_TEST_FAILED) == 0u)) {

            if (entry->agingCounter < 0xFFu) {
                entry->agingCounter++;
            }

            /* Threshold reached – auto-clear this DTC */
            if (entry->agingCounter >= DTC_AGING_THRESHOLD) {
                entry->dtcNumber        = 0u;
                entry->statusByte       = DTC_STATUS_TEST_NOT_COMPLETED_SINCE_LAST_CLEAR
                                        | DTC_STATUS_TEST_NOT_COMPLETED_THIS_OP_CYCLE;
                entry->agingCounter     = 0u;
                entry->occurrenceCounter = 0u;
                entry->active           = FALSE;
                entry->snapshotLength   = 0u;
                (void)memset(entry->snapshotData, 0,
                             (size_t)DTC_SNAPSHOT_DATA_SIZE);
            }
        }

        /* Clear per-cycle status bits at end of operation cycle */
        entry->statusByte &= (uint8)~DTC_STATUS_TEST_FAILED_THIS_OP_CYCLE;
        entry->statusByte |= DTC_STATUS_TEST_NOT_COMPLETED_THIS_OP_CYCLE;
    }

    return DTC_RET_OK;
}

/* --------------------------------------------------------------------
 * Dtc_GetCount
 * Returns the number of currently active (stored) DTCs.
 * -------------------------------------------------------------------- */
uint16 Dtc_GetCount(void)
{
    uint16 idx;
    uint16 count = 0u;

    if (dtcInitialized != TRUE) {
        return 0u;
    }

    for (idx = 0u; idx < dtcCount; idx++) {
        if (dtcTable[idx].active == TRUE) {
            count++;
        }
    }
    return count;
}

/* --------------------------------------------------------------------
 * Dtc_GetEntry
 * Copies the full DTC entry structure to the caller-provided buffer.
 * -------------------------------------------------------------------- */
Std_ReturnType Dtc_GetEntry(uint32 dtcNumber, Dtc_EntryType *entry)
{
    uint16 idx;

    if (dtcInitialized != TRUE) {
        return DTC_RET_NOT_OK;
    }

    if (entry == NULL) {
        return DTC_RET_NOT_OK;
    }

    if (dtcNumber == 0u) {
        return DTC_RET_NOT_OK;
    }

    idx = Dtc_FindIndex(dtcNumber);
    if (idx == DTC_MAX_COUNT) {
        return DTC_RET_NOT_OK;
    }

    (void)memcpy(entry, &dtcTable[idx], sizeof(Dtc_EntryType));
    return DTC_RET_OK;
}
