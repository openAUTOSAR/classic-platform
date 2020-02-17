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

#ifndef _LOG_H
#define _LOG_H

/**
 * @brief   A logging API
 *
 * @details
 *  A very crude and fast logger that logs in a circular buffer. The logger
 *  information is just a long string that is divided into different
 *  sections.
 *  .
 *  index       _s      _s_s     _s_u32 _s_u8
 *  ---------------------------------------------
 *    0-
 *    | "name"
 *    |-- LOG_NAME_SIZE / LOG_POS1
 *    |         "str"   "str1"  "str"    "str"*)
 *    |-- LOG_POS2
 *    |         "..."   "str2"   "u32"   "data"**)
 *    |-- LOG_MAX_STR
 *
 *    *)  Only 4 bytes
 *    **) Data is concatenated when n is too large.
 * .
 * .
 * Usage:
 *   To use the logger in a file:
 * .
 * in header
 *
 *  #if defined(CFG_LOG) && defined(LOG_<bla>)
 *    #define _LOG_NAME_ "bla"
 *  #endif
 *  #include "log.h"
 * .
 * in code:
 * .
 *   LOG_S("some text");
 *   LOG_S_S("some text", "some more text");
 *   LOG_S_U32("some text", 123);
 *   LOG_S_U8("array", &data, 8);
 */

#if defined(_LOG_NAME_)


#define LOG_S( _a)               log_s( _LOG_NAME_,_a)
#define LOG_S_S( _a, _b )        log_s_s(_LOG_NAME_, _a, _b )
#define LOG_S_U32( _a, _b)       log_s_u32( _LOG_NAME_,_a, _b)
#define LOG_S_A8(  _a, _b, _c )  log_s_a8( _LOG_NAME_, _a, _b, _c )

void log_s( const char *name ,const char *str );
void log_s_s( const char *name ,const char *str, const char *str2 );
void log_s_u32(  const char *name ,const char *str, uint32_t val);
void log_s_a8(  const char *name ,const char *str, uint8_t *data, uint8_t num );
#else
#define LOG_S( _a)
#define LOG_S_S( _a, _b )
#define LOG_S_U32( _a, _b)
#define LOG_S_A8(  _a, _b, _c )
#endif

#endif
