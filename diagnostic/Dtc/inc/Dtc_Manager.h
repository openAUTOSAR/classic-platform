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

#ifndef DTC_MANAGER_H_
#define DTC_MANAGER_H_

#include "Std_Types.h"

/*
 * DTC Manager Configuration
 */
#define DTC_MAX_COUNT               64u
#define DTC_SNAPSHOT_DATA_SIZE      32u
#define DTC_AGING_THRESHOLD         40u

/*
 * ISO 14229 UDS Status Byte Bit Definitions
 */
#define DTC_STATUS_TEST_FAILED                          0x01u
#define DTC_STATUS_TEST_FAILED_THIS_OP_CYCLE            0x02u
#define DTC_STATUS_PENDING_DTC                          0x04u
#define DTC_STATUS_CONFIRMED_DTC                        0x08u
#define DTC_STATUS_TEST_NOT_COMPLETED_SINCE_LAST_CLEAR  0x10u
#define DTC_STATUS_TEST_FAILED_SINCE_LAST_CLEAR         0x20u
#define DTC_STATUS_TEST_NOT_COMPLETED_THIS_OP_CYCLE     0x40u
#define DTC_STATUS_WARNING_INDICATOR_REQUESTED          0x80u

/*
 * DTC Event Types
 */
typedef uint8 Dtc_EventStatusType;
#define DTC_EVENT_PASSED     ((Dtc_EventStatusType)0x00u)
#define DTC_EVENT_FAILED     ((Dtc_EventStatusType)0x01u)

/*
 * DTC Return Types
 */
#define DTC_RET_OK           ((Std_ReturnType)E_OK)
#define DTC_RET_NOT_OK       ((Std_ReturnType)E_NOT_OK)

/*
 * DTC Entry Structure
 */
typedef struct {
    uint32  dtcNumber;
    uint8   statusByte;
    uint8   agingCounter;
    uint16  occurrenceCounter;
    boolean active;
    uint8   snapshotData[DTC_SNAPSHOT_DATA_SIZE];
    uint8   snapshotLength;
} Dtc_EntryType;

/*
 * Public API Functions
 */
Std_ReturnType Dtc_Manager_Init(void);
Std_ReturnType Dtc_ReportEvent(uint32 dtcNumber, Dtc_EventStatusType eventStatus,
                               const uint8 *snapshotData, uint8 snapshotLength);
Std_ReturnType Dtc_ClearAll(void);
Std_ReturnType Dtc_ClearById(uint32 dtcNumber);
Std_ReturnType Dtc_GetStatus(uint32 dtcNumber, uint8 *statusByte);
Std_ReturnType Dtc_GetSnapshot(uint32 dtcNumber, uint8 *destBuffer,
                               uint8 *length);
Std_ReturnType Dtc_AgeCycle(void);
uint16         Dtc_GetCount(void);
Std_ReturnType Dtc_GetEntry(uint32 dtcNumber, Dtc_EntryType *entry);

#endif /* DTC_MANAGER_H_ */
