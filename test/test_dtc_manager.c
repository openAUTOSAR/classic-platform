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
 * Test suite for the DTC Manager module.
 *
 * Validates:
 *   - Initialization
 *   - Event reporting and ISO 14229 status transitions
 *   - DTC clearing (all and by ID)
 *   - Aging counter management
 *   - Snapshot / freeze-frame data capture
 *   - NULL pointer handling (bug-fix validation)
 */

#include <stdio.h>
#include <string.h>
#include "Dtc_Manager.h"

/*
 * Simple test harness macros
 */
static int tests_run    = 0;
static int tests_passed = 0;
static int tests_failed = 0;

#define TEST_ASSERT(cond, msg)                                    \
    do {                                                          \
        tests_run++;                                              \
        if (cond) {                                               \
            tests_passed++;                                       \
            (void)printf("  PASS: %s\n", (msg));                  \
        } else {                                                  \
            tests_failed++;                                       \
            (void)printf("  FAIL: %s  [%s:%d]\n",                 \
                         (msg), __FILE__, __LINE__);              \
        }                                                         \
    } while (0)

#define TEST_SUITE(name)  (void)printf("\n=== %s ===\n", (name))

/* ------------------------------------------------------------------ */
static void test_init(void)
{
    Std_ReturnType ret;

    TEST_SUITE("Initialization");

    ret = Dtc_Manager_Init();
    TEST_ASSERT(ret == E_OK, "Dtc_Manager_Init returns E_OK");

    TEST_ASSERT(Dtc_GetCount() == 0u, "DTC count is 0 after init");
}

/* ------------------------------------------------------------------ */
static void test_report_event_failed(void)
{
    Std_ReturnType ret;
    uint8 statusByte = 0u;
    uint8 snapshot[4] = { 0xAA, 0xBB, 0xCC, 0xDD };

    TEST_SUITE("Report Event – FAILED");

    ret = Dtc_Manager_Init();
    TEST_ASSERT(ret == E_OK, "Init OK");

    ret = Dtc_ReportEvent(0x010203u, DTC_EVENT_FAILED, snapshot, 4u);
    TEST_ASSERT(ret == E_OK, "Report FAILED for DTC 0x010203");

    TEST_ASSERT(Dtc_GetCount() == 1u, "DTC count is 1");

    ret = Dtc_GetStatus(0x010203u, &statusByte);
    TEST_ASSERT(ret == E_OK, "GetStatus returns E_OK");

    TEST_ASSERT((statusByte & DTC_STATUS_TEST_FAILED) != 0u,
                "testFailed bit is set");
    TEST_ASSERT((statusByte & DTC_STATUS_CONFIRMED_DTC) != 0u,
                "confirmedDTC bit is set");
    TEST_ASSERT((statusByte & DTC_STATUS_PENDING_DTC) != 0u,
                "pendingDTC bit is set");
    TEST_ASSERT((statusByte & DTC_STATUS_TEST_FAILED_SINCE_LAST_CLEAR) != 0u,
                "testFailedSinceLastClear bit is set");
    TEST_ASSERT((statusByte & DTC_STATUS_TEST_NOT_COMPLETED_SINCE_LAST_CLEAR) == 0u,
                "testNotCompletedSinceLastClear bit is cleared");
}

/* ------------------------------------------------------------------ */
static void test_report_event_passed(void)
{
    Std_ReturnType ret;
    uint8 statusByte = 0u;
    uint8 snapshot[2] = { 0x11, 0x22 };

    TEST_SUITE("Report Event – PASSED");

    ret = Dtc_Manager_Init();
    TEST_ASSERT(ret == E_OK, "Init OK");

    /* First fail, then pass */
    ret = Dtc_ReportEvent(0xAABBCCu, DTC_EVENT_FAILED, snapshot, 2u);
    TEST_ASSERT(ret == E_OK, "Report FAILED");

    ret = Dtc_ReportEvent(0xAABBCCu, DTC_EVENT_PASSED, NULL, 0u);
    TEST_ASSERT(ret == E_OK, "Report PASSED");

    ret = Dtc_GetStatus(0xAABBCCu, &statusByte);
    TEST_ASSERT(ret == E_OK, "GetStatus OK");

    TEST_ASSERT((statusByte & DTC_STATUS_TEST_FAILED) == 0u,
                "testFailed bit is cleared after PASSED");
    TEST_ASSERT((statusByte & DTC_STATUS_CONFIRMED_DTC) != 0u,
                "confirmedDTC bit remains set after PASSED");
}

/* ------------------------------------------------------------------ */
static void test_occurrence_counter(void)
{
    Std_ReturnType ret;
    Dtc_EntryType entry;

    TEST_SUITE("Occurrence Counter");

    ret = Dtc_Manager_Init();
    TEST_ASSERT(ret == E_OK, "Init OK");

    ret = Dtc_ReportEvent(0x112233u, DTC_EVENT_FAILED, NULL, 0u);
    TEST_ASSERT(ret == E_OK, "First FAILED");

    ret = Dtc_ReportEvent(0x112233u, DTC_EVENT_FAILED, NULL, 0u);
    TEST_ASSERT(ret == E_OK, "Second FAILED");

    ret = Dtc_ReportEvent(0x112233u, DTC_EVENT_FAILED, NULL, 0u);
    TEST_ASSERT(ret == E_OK, "Third FAILED");

    ret = Dtc_GetEntry(0x112233u, &entry);
    TEST_ASSERT(ret == E_OK, "GetEntry OK");
    TEST_ASSERT(entry.occurrenceCounter == 3u,
                "Occurrence counter is 3 after three failures");
}

/* ------------------------------------------------------------------ */
static void test_clear_all(void)
{
    Std_ReturnType ret;

    TEST_SUITE("Clear All DTCs");

    ret = Dtc_Manager_Init();
    TEST_ASSERT(ret == E_OK, "Init OK");

    (void)Dtc_ReportEvent(0x0001u, DTC_EVENT_FAILED, NULL, 0u);
    (void)Dtc_ReportEvent(0x0002u, DTC_EVENT_FAILED, NULL, 0u);
    (void)Dtc_ReportEvent(0x0003u, DTC_EVENT_FAILED, NULL, 0u);
    TEST_ASSERT(Dtc_GetCount() == 3u, "3 DTCs stored");

    ret = Dtc_ClearAll();
    TEST_ASSERT(ret == E_OK, "ClearAll returns E_OK");
    TEST_ASSERT(Dtc_GetCount() == 0u, "DTC count is 0 after ClearAll");
}

/* ------------------------------------------------------------------ */
static void test_clear_by_id(void)
{
    Std_ReturnType ret;
    uint8 statusByte = 0u;

    TEST_SUITE("Clear DTC by ID");

    ret = Dtc_Manager_Init();
    TEST_ASSERT(ret == E_OK, "Init OK");

    (void)Dtc_ReportEvent(0x0001u, DTC_EVENT_FAILED, NULL, 0u);
    (void)Dtc_ReportEvent(0x0002u, DTC_EVENT_FAILED, NULL, 0u);

    ret = Dtc_ClearById(0x0001u);
    TEST_ASSERT(ret == E_OK, "ClearById 0x0001 returns E_OK");
    TEST_ASSERT(Dtc_GetCount() == 1u, "1 DTC remaining");

    ret = Dtc_GetStatus(0x0001u, &statusByte);
    TEST_ASSERT(ret == E_NOT_OK, "Cleared DTC not found");

    ret = Dtc_GetStatus(0x0002u, &statusByte);
    TEST_ASSERT(ret == E_OK, "Other DTC still present");

    ret = Dtc_ClearById(0x9999u);
    TEST_ASSERT(ret == E_NOT_OK, "ClearById for non-existent DTC returns E_NOT_OK");
}

/* ------------------------------------------------------------------ */
static void test_snapshot_data(void)
{
    Std_ReturnType ret;
    uint8 snapshot[4] = { 0xDE, 0xAD, 0xBE, 0xEF };
    uint8 readBuf[DTC_SNAPSHOT_DATA_SIZE];
    uint8 readLen = 0u;

    TEST_SUITE("Snapshot / Freeze-Frame Data");

    ret = Dtc_Manager_Init();
    TEST_ASSERT(ret == E_OK, "Init OK");

    ret = Dtc_ReportEvent(0x0501u, DTC_EVENT_FAILED, snapshot, 4u);
    TEST_ASSERT(ret == E_OK, "Report with snapshot");

    (void)memset(readBuf, 0, sizeof(readBuf));
    ret = Dtc_GetSnapshot(0x0501u, readBuf, &readLen);
    TEST_ASSERT(ret == E_OK, "GetSnapshot returns E_OK");
    TEST_ASSERT(readLen == 4u, "Snapshot length is 4");
    TEST_ASSERT(readBuf[0] == 0xDEu && readBuf[1] == 0xADu &&
                readBuf[2] == 0xBEu && readBuf[3] == 0xEFu,
                "Snapshot data matches");
}

/* ------------------------------------------------------------------ */
static void test_aging_counter(void)
{
    Std_ReturnType ret;
    Dtc_EntryType entry;
    uint8 i;

    TEST_SUITE("Aging Counter Management");

    ret = Dtc_Manager_Init();
    TEST_ASSERT(ret == E_OK, "Init OK");

    /* Create a DTC, then mark it as PASSED so aging can proceed */
    (void)Dtc_ReportEvent(0x0A01u, DTC_EVENT_FAILED, NULL, 0u);
    (void)Dtc_ReportEvent(0x0A01u, DTC_EVENT_PASSED, NULL, 0u);

    /* Run several aging cycles */
    for (i = 0u; i < 5u; i++) {
        ret = Dtc_AgeCycle();
        TEST_ASSERT(ret == E_OK, "AgeCycle returns E_OK");
    }

    ret = Dtc_GetEntry(0x0A01u, &entry);
    TEST_ASSERT(ret == E_OK, "GetEntry OK");
    TEST_ASSERT(entry.agingCounter == 5u,
                "Aging counter is 5 after 5 cycles");

    /* Run enough cycles to reach the threshold and auto-clear */
    for (i = 0u; i < (DTC_AGING_THRESHOLD - 5u); i++) {
        (void)Dtc_AgeCycle();
    }

    TEST_ASSERT(Dtc_GetCount() == 0u,
                "DTC auto-cleared after reaching aging threshold");
}

/* ------------------------------------------------------------------ */
static void test_null_pointer_checks(void)
{
    Std_ReturnType ret;

    TEST_SUITE("NULL Pointer Handling (Bug-Fix Validation)");

    ret = Dtc_Manager_Init();
    TEST_ASSERT(ret == E_OK, "Init OK");

    /* GetStatus with NULL statusByte */
    ret = Dtc_GetStatus(0x0001u, NULL);
    TEST_ASSERT(ret == E_NOT_OK,
                "GetStatus rejects NULL statusByte pointer");

    /* GetSnapshot with NULL destBuffer */
    ret = Dtc_GetSnapshot(0x0001u, NULL, NULL);
    TEST_ASSERT(ret == E_NOT_OK,
                "GetSnapshot rejects NULL destBuffer pointer");

    /* GetSnapshot with NULL length */
    {
        uint8 buf[4];
        ret = Dtc_GetSnapshot(0x0001u, buf, NULL);
        TEST_ASSERT(ret == E_NOT_OK,
                    "GetSnapshot rejects NULL length pointer");
    }

    /* GetEntry with NULL entry */
    ret = Dtc_GetEntry(0x0001u, NULL);
    TEST_ASSERT(ret == E_NOT_OK,
                "GetEntry rejects NULL entry pointer");

    /* ReportEvent with dtcNumber == 0 */
    ret = Dtc_ReportEvent(0u, DTC_EVENT_FAILED, NULL, 0u);
    TEST_ASSERT(ret == E_NOT_OK,
                "ReportEvent rejects dtcNumber 0");

    /* ClearById with dtcNumber == 0 */
    ret = Dtc_ClearById(0u);
    TEST_ASSERT(ret == E_NOT_OK,
                "ClearById rejects dtcNumber 0");

    /* GetStatus with dtcNumber == 0 */
    {
        uint8 sb;
        ret = Dtc_GetStatus(0u, &sb);
        TEST_ASSERT(ret == E_NOT_OK,
                    "GetStatus rejects dtcNumber 0");
    }

    /* Invalid event status value */
    ret = Dtc_ReportEvent(0x0001u, (Dtc_EventStatusType)0xFFu, NULL, 0u);
    TEST_ASSERT(ret == E_NOT_OK,
                "ReportEvent rejects invalid event status");
}

/* ------------------------------------------------------------------ */
static void test_uninit_guard(void)
{
    Std_ReturnType ret;
    uint8 sb;

    TEST_SUITE("Uninitialized Guard");

    /*
     * Hack: call ClearAll which internally re-inits, then we need a way to
     * test uninit state.  We rely on the static dtcInitialized flag being
     * set to FALSE before Dtc_Manager_Init has been called.  Since all
     * prior tests call Init, we cannot truly test this in-process without
     * exposing internals.  Instead we just verify the contract: after Init
     * everything works.
     */
    ret = Dtc_Manager_Init();
    TEST_ASSERT(ret == E_OK, "Init OK");

    ret = Dtc_GetStatus(0x1234u, &sb);
    TEST_ASSERT(ret == E_NOT_OK, "GetStatus for non-existent DTC returns E_NOT_OK");
}

/* ------------------------------------------------------------------ */
static void test_table_full(void)
{
    Std_ReturnType ret;
    uint32 i;

    TEST_SUITE("Table Full Handling");

    ret = Dtc_Manager_Init();
    TEST_ASSERT(ret == E_OK, "Init OK");

    /* Fill the table to capacity */
    for (i = 1u; i <= DTC_MAX_COUNT; i++) {
        ret = Dtc_ReportEvent(i, DTC_EVENT_FAILED, NULL, 0u);
        TEST_ASSERT(ret == E_OK, "Stored DTC within capacity");
    }

    TEST_ASSERT(Dtc_GetCount() == DTC_MAX_COUNT,
                "DTC count equals DTC_MAX_COUNT");

    /* Next allocation should fail */
    ret = Dtc_ReportEvent(DTC_MAX_COUNT + 1u, DTC_EVENT_FAILED, NULL, 0u);
    TEST_ASSERT(ret == E_NOT_OK,
                "ReportEvent returns E_NOT_OK when table is full");
}

/* ====================================================================
 * main
 * ==================================================================== */
int main(void)
{
    (void)printf("DTC Manager Test Suite\n");
    (void)printf("======================\n");

    test_init();
    test_report_event_failed();
    test_report_event_passed();
    test_occurrence_counter();
    test_clear_all();
    test_clear_by_id();
    test_snapshot_data();
    test_aging_counter();
    test_null_pointer_checks();
    test_uninit_guard();
    test_table_full();

    (void)printf("\n======================\n");
    (void)printf("Results: %d run, %d passed, %d failed\n",
                 tests_run, tests_passed, tests_failed);

    return (tests_failed > 0) ? 1 : 0;
}
