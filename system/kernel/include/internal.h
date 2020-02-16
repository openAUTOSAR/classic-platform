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

#ifndef INTERNAL_H_
#define INTERNAL_H_

#include "Os.h"
#if defined(USE_RAMLOG)
#include "Ramlog.h"
#endif

/* Using internal.h
 *
 * Os.h
 *  |
 *  |--- Std_Types.h
 *  |    |--- Platform_Types.h (std?)
 *  |    |--- Compiler.h       (std?)
 *  |
 *  |--- Os_Cfg.h
 *  |
 *  |--- MemMap.h
 *
 *
 * task.c
 *  |--- Os.h
 *  |--- internal.h
 *
 *
 *
 *           kernel.h (shared types between config and OS source... pointless?)
 *              |
 *     |------------------|
 *  Os_Cfg.c          internal.h
 *
 *
 * API's
 * -------------------------------------
 * Os.h   				- OS API and types
 * os_config_func.h     - Inline API for Os_Cfg.c
 * ext_config.h         - API for os_config_func.h, used by kernel
 * internal.h 			- Internal API for kernel, do NOT expose outside kernel
 *
 * os_types.h           - Internal types for the kernel
 *
 *
 */


/*
 * debug settings for os_debug_mask
 *
 */

// print to STDOUT. If not set it prints to ramlog
#define D_STDOUT					0
#define D_RAMLOG					0
#define D_MASTER_PRINT				0
#define D_ISR_MASTER_PRINT			0
#define D_ISR_RAMLOG				0

#define D_TASK						(1<<0)
#define D_ALARM						(1<<1)
#define D_RESOURCE					(1<<2)
#define D_SCHTBL					(1<<3)
#define D_EVENT						(1<<4)
#define D_MESSAGE					(1<<5)


/*
 * Configuration tree:
 * USE_OS_DEBUG               - Turn on/off all Os_DbgPrintf()
 * SELECT_OS_CONSOLE          - Select console
 * USE_RAMLOG                 - Compile ramlog code...
 *
 *
 * Default is to print to RAMLOG.
 *
 *
 * Use cases:
 * 1. We don't have a RAMLOG (low on RAM) so we want to print to serial console:
 *     #define CFG_OS_DEBUG = STD_ON
 *     #define USE_SERIAL_PORT
 *     #define SELECT_OS_CONSOLE=TTY_SERIAL0
 * 2. We have a RAMLOG but we have a debugger connected and want the OS debug
 *    to go there instead:
 *     #define CFG_OS_DEBUG = STD_ON
 *     #define USE_RAMLOG
 *     #define USE_TTY_T32
 *     #define SELECT_OS_CONSOLE=TTY_T32
 * 3. We have only the ramlog:
 *     #define CFG_OS_DEBUG = STD_ON
 *     #define USE_RAMLOG
 *     #define SELECT_OS_CONSOLE=TTY_RAMLOG
 * 4. We use no debug.
 *    <empty>
  *
 */

#if (CFG_OS_DEBUG == STD_ON)
# if (SELECT_OS_CONSOLE==RAMLOG)
#  ifndef USE_RAMLOG
#  error  USE_RAMLOG must be defined.
#  endif

#  define OS_DEBUG(_mask,...) \
	do { \
		if( os_dbg_mask & (_mask) ) { \
			ramlog_printf("[%08u] : ",(unsigned)GetOsTick()); \
			ramlog_printf(__VA_ARGS__ );	\
		}; \
	} while(0);
# elif (SELECT_OS_CONSOLE==TTY_NONE)
#   define OS_DEBUG(_mask,...)
# else
#  define OS_DEBUG(_mask,...) \
	do { \
		if( os_dbg_mask & (_mask) ) { \
			printf("[%08u] : %s %d ",(unsigned)GetOsTick(), __FUNCTION__, __LINE__ ); \
			printf(__VA_ARGS__ );	\
		}; \
	} while(0);
# endif
#else
# define OS_DEBUG(_mask,...)
#endif

extern uint32 os_dbg_mask;

/*
 * Macros for error handling
 * Registers service id of the erroneous function and the applicable parameters
 * to os_error. Functions that have less than three parameters do not touch
 * os_error.param3. Same rule follows for other parameter counts.
 */

/* Error handling for functions that take no arguments */
#define OS_STD_END(_service_id) \
        goto ok;        \
    err:                \
        os_error.serviceId=_service_id;\
        ERRORHOOK(rv);  \
    ok:                 \
        return rv;

/* Error handling for functions that take one argument */
#define OS_STD_END_1(_service_id, _p1) \
        goto ok;        \
    err:                \
    os_error.serviceId=_service_id;\
        os_error.param1 = (uint32_t) _p1; \
        ERRORHOOK(rv);  \
    ok:                 \
        return rv;

/* Error handling for functions that take two arguments */
#define OS_STD_END_2(_service_id, _p1,_p2) \
        goto ok;        \
    err:                \
        os_error.serviceId=_service_id;\
        os_error.param1 = (uint32_t) _p1; \
        os_error.param2 = (uint32_t) _p2; \
        ERRORHOOK(rv);  \
    ok:                 \
        return rv;

/* Error handling for functions that take three arguments */
#define OS_STD_END_3(_service_id,_p1,_p2,_p3) \
        goto ok;        \
    err:                \
        os_error.serviceId=_service_id;\
        os_error.param1 = (uint32_t) _p1; \
        os_error.param2 = (uint32_t) _p2; \
        os_error.param3 = (uint32_t) _p3; \
        ERRORHOOK(rv);  \
    ok:                 \
        return rv;


#define OS_SAVE_PARAM_3(_service_id,_p1,_p2,_p3) \
		 os_error.serviceId=_service_id;\
		 os_error.param1 = (uint32_t) _p1; \
		 os_error.param2 = (uint32_t) _p2; \
		 os_error.param3 = (uint32_t) _p3; \
		 ERRORHOOK(rv);  \


/* Called for sequence of error hook calls in case a service
 * does not return with E_OK. Note that in this case the general error hook and the OS-
 * Application specific error hook are called.
 */

#define ERRORHOOK(x) \
	if( OS_SYS_PTR->hooks->ErrorHook != NULL  ) { \
		OS_SYS_PTR->hooks->ErrorHook(x); \
	} \
	OS_APP_CALL_ERRORHOOKS(x);



#if	(OS_USE_APPLICATIONS == STD_ON)
#define PROTECTIONHOOK(_x) \
	do { \
		if( OS_SYS_PTR->hooks->ProtectionHook != NULL ) { \
			OS_SYS_PTR->hooks->ProtectionHook(_x); \
		} \
    } while(0)

#endif


#define PRETASKHOOK() \
	assert( OS_SYS_PTR->currTaskPtr->state & ST_RUNNING ); \
	assert( OS_SYS_PTR->currTaskPtr->flags == SYS_FLAG_HOOK_STATE_EXPECTING_PRE );  \
	OS_SYS_PTR->currTaskPtr->flags = SYS_FLAG_HOOK_STATE_EXPECTING_POST;   \
	if( OS_SYS_PTR->hooks->PreTaskHook != NULL ) { \
		OS_SYS_PTR->hooks->PreTaskHook(); \
	}

#define POSTTASKHOOK() \
	assert( OS_SYS_PTR->currTaskPtr->state & ST_RUNNING ); \
	assert( OS_SYS_PTR->currTaskPtr->flags == SYS_FLAG_HOOK_STATE_EXPECTING_POST );  \
	OS_SYS_PTR->currTaskPtr->flags = SYS_FLAG_HOOK_STATE_EXPECTING_PRE;   \
	if( OS_SYS_PTR->hooks->PostTaskHook != NULL ) { 	\
		OS_SYS_PTR->hooks->PostTaskHook();			\
	}

#endif /*INTERNAL_H_*/
