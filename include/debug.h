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


// PC-Lint Exception to MISRA rule 19.12: stdio ok in debug.h.
//lint -e(829)


#ifndef DEBUG_H_
#define DEBUG_H_

/**
 *
 * NOTE!!!!
 * Do not use this in a header file. Should be used in the *.c file like this.
 *
 * #define USE_DEBUG_PRINTF
 * #include "debug.h"
 *
 * Macro's for debugging and tracing
 *
 * Define USE_LDEBUG_PRINTF and DBG_LEVEL either globally( e.g. a makefile )
 * or in a specific file.  The DBG_LEVEL macro controls the amount
 * of detail you want in the debug printout.
 * There are 3 levels:
 * DEBUG_LOW    - Used mainly by drivers to get very detailed
 * DEBUG_MEDIUM - Medium detail
 * DEBUG_HIGH   - General init
 *
 * Example:
 * #define DEBUG_LVL	DEBUG_HIGH
 * DEBUG(DEBUG_HIGH,"Starting GPT");
 *
 *
 */

#include <stdio.h>

#define DEBUG_LOW       1u
#define DEBUG_MEDIUM    2u
#define DEBUG_HIGH      3u
#define DEBUG_NONE      4u

#ifndef DEBUG_LVL
#define DEBUG_LVL       2u
#endif

#define CH_ISR          0u
#define CH_PROC         1u

#if defined(DBG)
#define dbg(...) printf(__VA_ARGS__ )
#else
#define dbg(...)
#endif

#if defined(_DEBUG_)
#define _debug_(...) printf (__VA_ARGS__);
#else
#define _debug_(...)
#endif

#if defined(USE_DEBUG_PRINTF)
#define DEBUG(_level,...) \
    do { \
        if(_level>=DEBUG_LVL) { \
            printf (__VA_ARGS__); \
        }; \
    } while(0);

#else
#define DEBUG(_level,...)
#endif

#if defined(USE_LDEBUG_PRINTF)
#define debug(...)                  printf(__VA_ARGS__)
#define LDEBUG_PRINTF(format,...) 	printf(format,## __VA_ARGS__ )
#define LDEBUG_FPUTS(_str) 			fputs((_str),stdout)
#else
#define LDEBUG_PRINTF(format,...)
#define LDEBUG_FPUTS(_str)
#endif



#endif /*DEBUG_H_*/
