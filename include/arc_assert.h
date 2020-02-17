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


#ifndef ARC_ASSERT_H_
#define ARC_ASSERT_H_

#if defined(CFG_SYSTEM_ASSERT)
#include <assert.h>
#endif

#define QUOTE__(x)     #x
/*lint -save -e666 -e613 -e1776 Expression with side effects passed to repeated parameter 1 in macro 'ASSERT'*/
#if !defined(ASSERT)
#if defined(CFG_SYSTEM_ASSERT)
#  define ASSERT(_x)  assert(_x)
#else
#  if defined(CFG_NDEBUG)
#    define ASSERT(_x)  ((void)0)
#  else
#    define ASSERT(_x)  if (!(_x)) { \
                          Arc_Assert(QUOTE__(_x),__FILE__,__LINE__); \
                        }
#  endif
#endif /* CFG_SYSTEM_ASSERT */
#endif
/*lint restore*/
/**
 * Use this macro if you want to the check the validity of a configuration.
 * This macro should also be used in the init functions.
 * If you are going to check state, etc use ASSERT() instead.
 */
#if defined(CFG_CONFIG_NDEBUG)
#  define CONFIG_ASSERT(_x)  ((void)0)
#else
#  define CONFIG_ASSERT(_x)  if (!(_x)) { \
                               Arc_Config_Assert(QUOTE__(_x),__FILE__,__LINE__); \
                             }
#endif


void Arc_Assert( char *msg, const char *file, int line );
void Arc_Config_Assert( char *msg, const char *file, int line );


#endif /* ARC_ASSERT_H_ */
