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

#ifndef INTERNAL_H_
#define INTERNAL_H_

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

/* Error handling and hooks are implemented as Macros for readability and easy expansion */
/*
 * Macros for error handling
 * Registers service id of the erroneous function and the applicable parameters
 * to os_error. Functions that have less than three parameters do not touch
 * os_error.param3. Same rule follows for other parameter counts.
 */

/*lint -emacro(506, OS_STD_ERR_1, OS_STD_ERR_2, OS_STD_ERR_3) MISRA:FALSE_POSITIVE:"err" case is not evaluated at macro "call" Constant value Boolean:[MISRA 2012 Rule 2.1, required] */

/*lint -emacro(774, OS_STD_ERR_1, OS_STD_ERR_2, OS_STD_ERR_3) MISRA:FALSE_POSITIVE:"err" case is not evaluated at macro "call" Constant value Boolean:[MISRA 2012 Rule 14.3, required] */

/*lint -emacro(904, OS_STD_ERR, OS_STD_ERR_1, OS_STD_ERR_2, OS_STD_ERR_3) MISRA:FALSE_POSITIVE:Function returns to handle error condition:[MISRA 2012 Rule 15.5, advisory]*/
/* Error handling for functions that take no arguments */
#define OS_STD_ERR(_service_id) \
        os_error.serviceId=_service_id;\
        Os_CallErrorHook(rv);  \
        return rv // Expecting calling function to provide the ending semicolon

/* Error handling for functions that take one argument */
/*lint -emacro(923, OS_STD_ERR_1) MISRA:FALSE_POSITIVE:Allow any pointer type to integer type conversion used for error handling in this macro:[MISRA 2012 Rule 11.6, required]*/
#define OS_STD_ERR_1(_service_id, _p1) \
		os_error.serviceId=_service_id;\
        os_error.param1 = (uint32_t) _p1; \
        Os_CallErrorHook(rv);  \
        return rv // Expecting calling function to provide the ending semicolon
/*lint -e{9036} MISRA:EXTERNAL_FILE::[MISRA 2012 Rule 14.4, required] */
/* Error handling for functions that take two arguments */
/*lint -emacro(923, OS_STD_ERR_2) MISRA:FALSE_POSITIVE:Allow any pointer type to integer type conversion used for error handling in this macro:[MISRA 2012 Rule 11.6, required]*/
#define OS_STD_ERR_2(_service_id, _p1,_p2) \
        os_error.serviceId=_service_id;\
        os_error.param1 = (uint32_t) _p1; \
        os_error.param2 = (uint32_t) _p2; \
        Os_CallErrorHook(rv);  \
        return rv // Expecting calling function to provide the ending semicolon

/* Error handling for functions that take three arguments */
/*lint -emacro(923, OS_STD_ERR_3) MISRA:FALSE_POSITIVE:Allow any pointer type to integer type conversion used for error handling in this macro:[MISRA 2012 Rule 11.6, required]*/
#define OS_STD_ERR_3(_service_id,_p1,_p2,_p3) \
        os_error.serviceId=_service_id;\
        os_error.param1 = (uint32_t) _p1; \
        os_error.param2 = (uint32_t) _p2; \
        os_error.param3 = (uint32_t) _p3; \
        Os_CallErrorHook(rv);  \
        return rv // Expecting calling function to provide the ending semicolon



#if (OS_SC3 == STD_ON) || (OS_SC4 == STD_ON)
 /*lint -emacro(923, OS_VALIDATE_ADDRESS_RANGE) MISRA:FALSE_POSITIVE:Allow any pointer type to integer type conversion used for error handling in this macro:[MISRA 2012 Rule 11.6, required]*/
#define OS_VALIDATE_ADDRESS_RANGE(_outParam,_size) Os_ValidAddressRange((uint32)_outParam,(uint32)_size)

#endif
#endif /*INTERNAL_H_*/
