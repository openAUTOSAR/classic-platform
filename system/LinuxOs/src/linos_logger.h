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

#ifndef _LINOS_LOGGER_H
#define _LINOS_LOGGER_H

#include <stdarg.h> // Optional arguments to functions
#include <stdint.h>

// Use Syslog ?
#ifndef _WIN32
#define USE_SYSLOG
#endif
//


// Log priorities 
//
// If NOT using syslog then hard define them here.
// Taken from /usr/include/syslog.h
#ifdef USE_SYSLOG
#include <syslog.h>
#else

#define LOG_EMERG       0       /* system is unusable */
#define LOG_ALERT       1       /* action must be taken immediately */
#define LOG_CRIT        2       /* critical conditions */
#define LOG_ERR         3       /* error conditions */
#define LOG_WARNING     4       /* warning conditions */
#define LOG_NOTICE      5       /* normal but significant condition */
#define LOG_INFO        6       /* informational */
#define LOG_DEBUG       7       /* debug-level messages */

#endif
//

// Divide logging in to modules and sub-modules. 16 bits.
// Currently used by the DebugMask to filter logging. (-m switch)

// Modules (high) 8 bits
#define LOGGER_MOD_LINOS	1<<8	// 0x100
#define LOGGER_MOD_TCP		1<<9	// 0x200
//

// Sub-modules LINOS 8 bits
#define LOGGER_SUB_OS_DATABASE 	1	// 0x01
#define LOGGER_SUB_OS_MAIN	1<<1	// 0x02
#define LOGGER_SUB_OS_TASK	1<<2	// 0x04
#define LOGGER_SUB_OS_ALARM	1<<3	// 0x08
#define LOGGER_SUB_OS_EVENT	1<<4	// 0x10
//

// Sub-modules TCP 8 bits
#define LOGGER_SUB_TCP_MAIN     1	// 0x01
#define LOGGER_SUB_TCP_SEND	1<<1	// 0x02
#define LOGGER_SUB_TCP_RECV	1<<2	// 0x04
#define LOGGER_SUB_TCP_BIND	1<<3	// 0x08
//


// Declare functions

void logger_open_close(int open_close, char *progname);

void logger_set_prefix(char *prefix);

int logger_set_output(int bitMask );

void logger_va(int loglevel, char *format, va_list listPointer);

void logger(int loglevel, char *format, ... );

void logger_mod(uint16_t logmodule, int loglevel, char *format, ... );

char* logger_format_hex(char* s, int slength);

#endif
