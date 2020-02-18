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
 *
 * Logger functions
 *
 *
 * /etc/rsyslog.conf example config rule ...
 *
 * $umask 0000 # make sure nothing interferes with the following definitions
 * $FileCreateMode 0644
 *
 * $template ArcCoreLogs,"/var/log/modbus-slave.%$DAY%.log"
 *
 * if $programname == 'EthernetSimple' then -?ArcCoreLogs
 * if $programname == 'EthernetSimple' then stop
 *
 * AND then in cron.hourly
 *
 * # Delete /var/log/modbus-slave.*.log-files not changed in more than 24 hours:
 * find /var/log/ -type f -mtime +1  -name "modbus-slave*.log" -exec rm -f '{}' \;
 *
 *
*/


#include "linos_logger.h"
#include <stdio.h> // vsprintf
#include <string.h> // strlen

#include <os_i.h>
#include <os_sys.h>  /* OS_SYS_PTR */


char log_prefix[32] = {'\0'}; // Prefix log messages


/*
 * Regardless of using syslog or not, we define log level strings.
 * It will be mainly used for printing logs to stdout and stdin.
 *
 * We did not use the prioritynames in syslog.h because it appears to require
 * more than a simple switch statement to use.
 */
const char * LOG_LEVEL_STR[8] = {
        "LOG_EMERG",
        "LOG_ALERT",
        "LOG_CRIT",
        "LOG_ERR",
        "LOG_WARNING",
        "LOG_NOTICE",
        "LOG_INFO",
        "LOG_DEBUG"
};

// Bit mask for outputs. bit0 = fprintf, bit 1 = syslog
#ifdef USE_SYSLOG
int logger_output_mask = 2; 
#else
int logger_output_mask = 1;
#endif


extern int DebugMask;

#ifdef USE_SYSLOG
/*
 *
 * Open / close syslog
 *
 * open_close = 1 = Open
 *
*/

void logger_open_close(int open_close, char *progname) {
#ifndef _WIN32
    if ( open_close == 1 ) {
        // Open
        // setlogmask (LOG_UPTO (LOG_DEBUG)); // Only send more important messages than LOG_DEBUG to syslog
        openlog (progname, LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
    } else {
        // Close
        closelog ();
    }
#endif
}
#endif


/*
 *
 * Log message suffix
 *
 *
*/ 

void logger_set_prefix(char *prefix) {
    strncpy(log_prefix,prefix, (sizeof(log_prefix) - 1));
}


/*
 *
 * Set logger output
 * 
 * bit 0 = fprintp
 *
 * bit 1 = syslog ( If defined)
 *
 * RETURNS 0 on success
 *
*/

int logger_set_output(int bitMask) {
    int rc = 0;
    int syslog_enabled = 0;

    #ifdef USE_SYSLOG
    syslog_enabled = 1;
    #endif

    if ( ((bitMask & 1) == 1) && (syslog_enabled == 0) ) {
        rc = -1;
    } else {
        logger_output_mask = bitMask;
    }
    return rc;
}




void logger_mod(uint16_t logmodule, int loglevel, char *format, ... ) {
#ifndef _WIN32
    va_list listPointer; // Pointer to variable arguments
    va_start ( listPointer, format); // Set the pointer to the last fixed argument.

    uint16_t module,submodule;

    module = (( logmodule >> 8 )& 0xff) ;
    submodule = (logmodule & 0xff);

    if ( ((DebugMask >> 8)&0xff) & module) {
        if ( (DebugMask&0xff) & submodule ) {
            logger_va(loglevel, format, listPointer);
        }
    }
    va_end( listPointer );
#endif
}


/*
 * logger ( loglevel, *format, ...)
 *
 * WHERE:
 *
 * loglevel = priorities in syslog.h or hard coded if not using syslog
 * *format = is the main string with formatting. AKA printf format.
 *  ... is an ellipsis. (varying number of args). ie. the varables for the above format.
 *
 *
*/

void logger ( int loglevel, char *format, ... ) {
#ifndef _WIN32
    va_list listPointer; // Pointer to variable arguments

    va_start ( listPointer, format); // Set the pointer to the last fixed argument.

    logger_va (loglevel, format, listPointer);
    va_end( listPointer );
#endif
}

#ifndef _WIN32
void logger_va(int loglevel, char *format, va_list listPointer) {
    char format_buffer[255] = {'\0'};
    char print_buffer[255] = {'\0'}; // Temp string buffer
    char timestamp_buffer[16]  = {'\0'};

    snprintf(timestamp_buffer, (sizeof(timestamp_buffer) -1), "[%u]:", OS_SYS_PTR->tick);

    // log_prefix    
    if ( strlen(log_prefix) > 1 ) {
        strcpy(print_buffer, log_prefix);
        strcat(print_buffer, " : ");
    }

    vsnprintf(format_buffer, (sizeof(format_buffer) -1 ), format,listPointer); // printf -> print_buffer
    strcat( print_buffer,timestamp_buffer);

    strcat( print_buffer,format_buffer);

#ifdef USE_SYSLOG
    if ((logger_output_mask & 2) == 2)
        syslog (loglevel, "%s", print_buffer);
#endif
    if ((logger_output_mask & 1) == 1) {
        if (loglevel > LOG_WARNING)
            fprintf(stdout, "%s:%s\n", LOG_LEVEL_STR[loglevel], print_buffer);
        else
            fprintf(stderr, "%s:%s\n", LOG_LEVEL_STR[loglevel], print_buffer);
    }
}
#endif
/*
 *  logger_format_hex( char* s, int slength)
 *
 *  Format a character array as a hex string.
 *
 *  s is a pointer to a buffer to log as hex
 *
 *  slength is the length in bytes
 *
*/

char* logger_format_hex( char* s, int slength) {
    int bytesleft;
    char *iter = s;
    unsigned char ch;
    static char output_buffer[255] = {'\0'};
    char* pOutputBuffer = &output_buffer[0];
    char* pOutput = pOutputBuffer;

    if ( slength > 255 ) {
        slength = 255;
    }

    bytesleft = slength;
    while (bytesleft > 0) {
        ch = *iter++;
        if ( slength != bytesleft) {
            snprintf(pOutputBuffer, 2, " ");
            pOutputBuffer++;
        }
        bytesleft--;
        if( ',' != ch) {
            snprintf(pOutputBuffer, 3, "%.2x", ch);
            pOutputBuffer += 2;
        } else {
            snprintf(pOutputBuffer, 2, ",");
            pOutputBuffer++;
        }
    }
    return pOutput;
}

