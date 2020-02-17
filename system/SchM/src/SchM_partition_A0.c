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
/** @fileSafetyClassification ASIL **/ /* SchM_Partition_A0 contains SchM task for SP A0 partition  */

#include "SchM_SP.h"
#include "SchM_cfg.h"

#if defined(HOST_TEST)
#include "schm_devtest_stubs.h"
#endif

#if defined(USE_KERNEL)
#include "Os.h"
#endif

#if defined(USE_ECUM_FIXED)
#include "EcuM.h"
#include "SchM_EcuM.h"
/* @req SWS_BSW_00006 include the BSW Memory mapping header */
#define SCHM_START_SEC_VAR_CLEARED_ASIL_UNSPECIFIED
#include "SchM_MemMap.h"    /*lint !e451 !e9019 MISRA:STANDARDIZED_INTERFACE:AUTOSAR Require Inclusion:[MISRA 2012 Directive 4.10, required] [MISRA 2012 Rule 20.1, advisory] */
SCHM_DECLARE(ECUM_A0);
#define SCHM_STOP_SEC_VAR_CLEARED_ASIL_UNSPECIFIED
#include "SchM_MemMap.h"    /*lint !e451 !e9019 MISRA:STANDARDIZED_INTERFACE:AUTOSAR Require Inclusion:[MISRA 2012 Directive 4.10, required] [MISRA 2012 Rule 20.1, advisory] */

#else
#define SCHM_MAINFUNCTION_ECUM_A0()
#endif

#if defined(USE_RTM)
#include "Rtm.h"
#include "SchM_Rtm.h"

#define SCHM_START_SEC_VAR_CLEARED_ASIL_UNSPECIFIED
#include "SchM_MemMap.h"    /*lint !e451 !e9019 MISRA:STANDARDIZED_INTERFACE:AUTOSAR Require Inclusion:[MISRA 2012 Directive 4.10, required] [MISRA 2012 Rule 20.1, advisory] */
SCHM_DECLARE(RTM);
#define SCHM_STOP_SEC_VAR_CLEARED_ASIL_UNSPECIFIED
#include "SchM_MemMap.h"    /*lint !e451 !e9019 MISRA:STANDARDIZED_INTERFACE:AUTOSAR Require Inclusion:[MISRA 2012 Directive 4.10, required] [MISRA 2012 Rule 20.1, advisory] */

#else
#define SCHM_MAINFUNCTION_RTM()
#endif

#if defined(USE_WDGM)
#include "WdgM.h"
#include "SchM_WdgM.h"

#define SCHM_START_SEC_VAR_CLEARED_ASIL_UNSPECIFIED
#include "SchM_MemMap.h"    /*lint !e451 !e9019 MISRA:STANDARDIZED_INTERFACE:AUTOSAR Require Inclusion:[MISRA 2012 Directive 4.10, required] [MISRA 2012 Rule 20.1, advisory] */
SCHM_DECLARE(WDGM);
#define SCHM_STOP_SEC_VAR_CLEARED_ASIL_UNSPECIFIED
#include "SchM_MemMap.h"    /*lint !e451 !e9019 MISRA:STANDARDIZED_INTERFACE:AUTOSAR Require Inclusion:[MISRA 2012 Directive 4.10, required] [MISRA 2012 Rule 20.1, advisory] */

#else
#define SCHM_MAINFUNCTION_WDGM()
#endif

#if defined(USE_SAFEIOHWAB)
#include "SafeIoHwAb.h"
#include "SchM_SafeIoHwAb.h"

#define SCHM_START_SEC_VAR_CLEARED_ASIL_UNSPECIFIED
#include "SchM_MemMap.h"    /*lint !e451 !e9019 MISRA:STANDARDIZED_INTERFACE:AUTOSAR Require Inclusion:[MISRA 2012 Directive 4.10, required] [MISRA 2012 Rule 20.1, advisory] */
SCHM_DECLARE(SAFEIOHWAB);
#define SCHM_STOP_SEC_VAR_CLEARED_ASIL_UNSPECIFIED
#include "SchM_MemMap.h"    /*lint !e451 !e9019 MISRA:STANDARDIZED_INTERFACE:AUTOSAR Require Inclusion:[MISRA 2012 Directive 4.10, required] [MISRA 2012 Rule 20.1, advisory] */

#else
#define SCHM_MAINFUNCTION_SAFEIOHWAB()
#endif

/* GetVersionInfo for Safety platform SchM (common for all partitions) */
void SchM_GetVersionInfo( Std_VersionInfoType *versionInfo ) {

    if(versionInfo != NULL_PTR) {
        versionInfo->vendorID = SCHM_VENDOR_ID;
        versionInfo->moduleID = SCHM_MODULE_ID;
        versionInfo->sw_major_version = SCHM_SW_MAJOR_VERSION;
        versionInfo->sw_minor_version = SCHM_SW_MINOR_VERSION;
        versionInfo->sw_patch_version = SCHM_SW_PATCH_VERSION;
    }
}

/* @req ARC_SWS_SchM_00003 */
TASK(SchM_Partition_A0) {
    EcuM_StateType state;
    EventMaskType Event;
    EcuM_SP_RetStatus retStatus = ECUM_SP_OK;

    do {
        /*lint -save -e534 MISRA:HARDWARE_ACCESS::[MISRA 2012 Rule 17.7, required] */
        SYS_CALL_WaitEvent((EVENT_MASK_Alarm_BswServices_Partition_A0 | EVENT_MASK_DetReportError_QM | EVENT_MASK_SynchPartition_A0));
        SYS_CALL_GetEvent(TASK_ID_SchM_Partition_A0, &Event); /*lint !e923 MISRA:FALSE_POSITIVE:SYS_CALL:[MISRA 2012 Rule 11.1, required]*/

        if ((Event & EVENT_MASK_Alarm_BswServices_Partition_A0) != 0) { /*lint !e530 MISRA:FALSE_POSITIVE:Event is initialized in SYS_CALL:[MISRA 2012 Rule 9.1, mandatory] */
            SYS_CALL_ClearEvent(EVENT_MASK_Alarm_BswServices_Partition_A0);
        }

        /* @req ARC_SWS_SchM_00005 */
        if ((Event & EVENT_MASK_DetReportError_QM) != 0) {
            SYS_CALL_ClearEvent(EVENT_MASK_DetReportError_QM);
            SCHM_MAINFUNCTION_RTM();
        }

        /* Synchronize with the QM partition */ /* @req ARC_SWS_SchM_00004 */
        if ((Event & EVENT_MASK_SynchPartition_A0) != 0) {

            /* If we enter here it mean QM partition is done */
            SYS_CALL_ClearEvent(EVENT_MASK_SynchPartition_A0);
            /*lint -restore */

            /* Therefore, we can tell EcuM, SchM is ready for next state */
            EcuM_SP_Sync_UpdateStatus(ECUM_SP_PARTITION_FUN_COMPLETED_QM);
        }

        (void)EcuM_GetState(&state);

        /* ARC_SWS_SchM_00006 The BSW scheduler shall schedule BSW modules by calling their MainFunctions */
        switch (state) {
            case ECUM_STATE_STARTUP_ONE:
            case ECUM_STATE_STARTUP_TWO:

                SCHM_MAINFUNCTION_ECUM_ASIL(state, retStatus);


                break;
            default:

                    SCHM_MAINFUNCTION_ECUM_ASIL(state, retStatus);

                    if (retStatus == ECUM_SP_PARTITION_ERR) {
#if defined(USE_RTM)
                        Rtm_EntryType error;

                        error.moduleId = SCHM_MODULE_ID;
                        error.instanceId = 0;
                        error.apiId = SCHM_SID_A0;
                        error.errorId = SCHM_E_SYNCH;
                        error.errorType = RTM_ERRORTYPE_HOOK;

                        Rtm_ReportFailure(error);
#endif
                    } else {
                        /* Do nothing (Can be used for >2 partitions) */
                    }

                    SCHM_MAINFUNCTION_SAFEIOHWAB();
                    SCHM_MAINFUNCTION_RTM();
                    SCHM_MAINFUNCTION_WDGM();

                break;
        }
    } while (SCHM_TASK_EXTENDED_CONDITION != 0); /*lint !e506 MISRA:STANDARDIZED_INTERFACE:Extended Task shall never terminate:[MISRA 2012 Rule 2.1, required] */
}
