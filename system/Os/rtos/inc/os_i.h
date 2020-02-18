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
#ifndef OS_I_H_
#define OS_I_H_

#define NO_TASK_OWNER       (TaskType)(~0U)
#define STACK_PATTERN       0x42u        /* Pattern that the stack that unused stack space is filled with */


#include "arc_assert.h"
#include <stdlib.h>
#include <string.h>
#include <sys/queue.h>


/* Forward declare most object, so we don't have to care later
 * Rule: In all files use the name without the "Type" and with "struct"
 * */
struct OsApplication;
struct OsCounter;
struct OsAlarm;
struct OsResource;
struct OsTask;
struct OsTaskVar;
struct Os_Sys;
struct OsIsrVar;

#if (OS_STATUS_EXTENDED == STD_ON )

#define OS_VALIDATE(_exp,_err) \
		if( !(_exp) ) { \
			Os_CallErrorHook(_err); \
			return _err; \
		}

#define OS_VALIDATE_STD(_exp,_err,_id) \
        if( !(_exp) ) { \
          rv = _err; \
          OS_STD_ERR(_id);\
        }

#define OS_VALIDATE_STD_1(_exp,_err,_id,_p1) \
        if( !(_exp) ) { \
          rv = _err; \
          OS_STD_ERR_1(_id,_p1);\
        }

#define OS_VALIDATE_STD_2(_exp,_err,_id,_p1,_p2) \
        if( !(_exp) ) { \
          rv = _err; \
          OS_STD_ERR_2(_id,_p1,_p2);\
        }

#define OS_VALIDATE_STD_3(_exp,_err,_id,_p1,_p2,_p3) \
        if( !(_exp) ) { \
          rv = _err; \
          OS_STD_ERR_3(_id,_p1,_p2,_p3);\
        }
#else
#define OS_VALIDATE(_exp,_err)
#define OS_VALIDATE_STD(_exp,_err,_id)
#define OS_VALIDATE_STD_1(_exp,_err,_id,_p1)
#define OS_VALIDATE_STD_2(_exp,_err,_id,_p1,_p2)
#define OS_VALIDATE_STD_3(_exp,_err,_id,_p1,_p2,_p3)
#endif // OS_STATUS_EXTENDED

#include "Std_Types.h"
#include "Os.h" /* @req SWS_BSW_00005 */
#include "os_internal.h"
#include "os_sys.h"
#include "debug.h"
#include "irq_types.h"

/* "Normal" includes */
#include "os_config_macros.h"
#include "alist_i.h"
#include "os_resource_i.h"
#include "os_application_i.h"
#include "os_task_i.h"
#include "os_counter_i.h"
#include "os_alarm_i.h"
#include "os_sched_table_i.h"
#if (OS_NUM_CORES > 1)
#include "os_spinlock_i.h"
#endif
#include "isr.h"
#include "arch.h"
#include "os_multicore_i.h"
#include "arc.h"
#include "os_mm.h"
#include "os_trap.h"
#if (OS_SC3 == STD_ON) || (OS_SC4 == STD_ON)
#include "os_peripheral_i.h"
#endif



/**
 * Save Os internal mode flags
 * @return The
 */
static inline uint32 Os_SystemFlagsSave( uint32 newFlags ) {
    uint32 old = OS_SYS_PTR->osFlags;
    OS_SYS_PTR->osFlags = newFlags;
    return old;
}

static inline void Os_SystemFlagsRestore( uint32 oldFlags) {
    OS_SYS_PTR->osFlags = oldFlags;
}

/* Called for sequence of error hook calls in case a service
 * does not return with E_OK. Note that in this case the general error hook and the OS-
 * Application specific error hook are called.
 */
/** @req SWS_Os_00246  @req SWS_Os_00540  Application error hook to be called after system error hook */
static inline void Os_CallErrorHook(StatusType error) {
#if (OS_ERROR_HOOK == STD_ON)
    uint32 flags = Os_SystemFlagsSave(SYSTEM_FLAGS_IN_OS);
    ErrorHook(error);
    Os_SystemFlagsRestore(flags);
#endif

    OS_APP_CALL_ERRORHOOKS(error);
}

#if (OS_STARTUP_HOOK == STD_ON)
static inline void Os_CallStartupHook( void ) {
    StartupHook();
}
#else
#define Os_CallStartupHook()
#endif

#if (OS_SHUTDOWN_HOOK == STD_ON)

/* @CODECOV:OTHER_TEST_EXIST: Tested with SIL */
__CODE_COVERAGE_IGNORE__
static inline void Os_CallShutdownHook( StatusType error ) {
    ShutdownHook( error );
}
#else
#define Os_CallShutdownHook(_x)
#endif


#if (OS_PRE_TASK_HOOK == STD_ON)
static inline void Os_CallPreTaskHook( void  ) {
    /* @CODECOV:OPERATING_SYSTEM_ASSERT: check for sane values */
    __CODE_COVERAGE_OFF__
    ASSERT( OS_SYS_PTR->currTaskPtr->state & ST_RUNNING );
    ASSERT( OS_SYS_PTR->currTaskPtr->flags == SYS_FLAG_HOOK_STATE_EXPECTING_PRE );
    __CODE_COVERAGE_ON__
    OS_SYS_PTR->currTaskPtr->flags = SYS_FLAG_HOOK_STATE_EXPECTING_POST;
    PreTaskHook();
}
#else
#define Os_CallPreTaskHook()
#endif

#if (OS_POST_TASK_HOOK == STD_ON)
static inline void Os_CallPostTaskHook( void  ) {
    /* @CODECOV:OPERATING_SYSTEM_ASSERT: check for sane values */
    __CODE_COVERAGE_OFF__
    ASSERT( OS_SYS_PTR->currTaskPtr->state & ST_RUNNING );
    ASSERT( OS_SYS_PTR->currTaskPtr->flags == SYS_FLAG_HOOK_STATE_EXPECTING_POST );
    __CODE_COVERAGE_ON__
    OS_SYS_PTR->currTaskPtr->flags = SYS_FLAG_HOOK_STATE_EXPECTING_PRE;
    PostTaskHook();
}
#else
#define Os_CallPostTaskHook()
#endif



/** !req SWS_Os_00553 !req SWS_Os_00554 !req SWS_Os_00555 !req SWS_Os_00475 !req SWS_Os_00243 !req SWS_Os_00244 !req SWS_Os_00557
 * ProtectionHook Return value is not handled */
static inline void Os_CallProtectionHook(StatusType error) {

#if (OS_PROTECTION_HOOK == STD_ON) && ((OS_SC3 == STD_ON) || (OS_SC4 == STD_ON))
    ProtectionReturnType rv = PRO_SHUTDOWN;

    uint32 flags =  Os_SystemFlagsSave(SYSTEM_FLAGS_IN_OS);
    rv = ProtectionHook(error);
    Os_SystemFlagsRestore(flags);

    /* @CODECOV:OTHER_TEST_EXIST: Tested in SIL. In testing we want "ignore" the errors to continue testing */
    __CODE_COVERAGE_IGNORE__
    if( rv != PRO_IGNORE ) {
        /* @req SWS_Os_00556 If the  ProtectionHook()  returns  PRO_SHUTDOWN  the  Operating System module shall call the ShutdownOS() */
        /* @req SWS_Os_00308 If ProtectionHook() returns an invalid value, the Operating System module shall take the same action as
         *                   if no protection hook is configured.*/
        ShutdownOS(error);
    }

#else

    /* @req SWS_Os_00107 If no ProtectionHook() is configured and a protection error occurs, the Operating System module shall
     *                   call ShutdownOS()*/
    /* @req SWS_Os_00068  If a stack fault is detected by stack monitoring AND no ProtectionHook() is configured, the Operating
     *                   System module shall call the ShutdownOS() service with the status E_OS_STACKFAULT.*/
    ShutdownOS(error);
#endif
}

#if (OS_SC3 == STD_ON) || (OS_SC4 == STD_ON)

// to check the address validity of the out parameter supplied to the Os service
static inline boolean Os_ValidAddressRange( uint32 addr, uint32 size ) {
    const OsAppConstType *aConstP = Os_ApplGetConst(GetApplicationID());
    boolean rv = FALSE;

    /* data and bss, check dataAddr */
    if( aConstP->trusted == TRUE ) {
        rv = TRUE;
    } else {
        /* @CODECOV:OTHER_TEST_EXIST: This code is generic but the values that come in here are aligned */
        __CODE_COVERAGE_OFF__
        if( ((addr >= (uint32)aConstP->dataStart ) && ( (addr+size) < (uint32)aConstP->dataEnd )) ||
            ((addr >= (uint32)aConstP->bssStart ) && ( (addr+size) < (uint32)aConstP->bssEnd ) ) ) {
            rv = TRUE;
        }
        __CODE_COVERAGE_ON__
    }
    return rv;
}

#endif

#if	(OS_USE_APPLICATIONS == STD_ON)
static inline ApplicationType Os_GetCurrTaskISROwnerId (void) {
	ApplicationType appId;
	if(OS_SYS_PTR->intNestCnt == 0 ) {
		appId = Os_SysTaskGetCurr()->constPtr->applOwnerId;
	} else {
		appId = Os_SysIsrGetCurr()->constPtr->appOwner;
	}
	return appId;
}
#endif // OS_USE_APPLICATIONS == STD_ON

#endif
