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

/* ----------------------------[includes]------------------------------------*/

#include "Std_Types.h"
#include "shell.h"
#include "timer.h"
#include <string.h>
#include <stdio.h>

#define _LOG_NAME_
#include "log.h"
#include "xtoa.h"
/* ----------------------------[private define]------------------------------*/

/* Max size of _LOG_NAME_ */
#define LOG_NAME_SIZE       10

#define LOG_LEFT            (LOG_MAX_STR - LOG_NAME_SIZE)
#define LOG_POS1            LOG_NAME_SIZE
#define LOG_POS2            (LOG_NAME_SIZE + LOG_LEFT/2)
#define LOG_STR             LOG_LEFT/2

#if !defined(LOG_SIZE)
#define LOG_MAX_STR             50
#endif
#if !defined(LOG_SIZE)
#define LOG_SIZE                40
#endif

/* ----------------------------[private macro]-------------------------------*/
/* ----------------------------[private typedef]-----------------------------*/


typedef struct LogEntryS {
    char    str[LOG_MAX_STR];
    uint32  timestamp;
} LogEntryT;

struct LogH {
    uint32      cnt;
    LogEntryT   log[LOG_SIZE];
};

/* ----------------------------[private function prototypes]-----------------*/
/* ----------------------------[private variables]---------------------------*/


#define LOG_START_SEC_VAR_CLEARED_UNSPECIFIED
#include "MemMap.h"  /*lint !e9019 OTHER [MISRA 2012 Rule 20.1, advisory] OTHER AUTOSAR specified way of using MemMap*/
struct LogH Log;
#define LOG_STOP_SEC_VAR_CLEARED_UNSPECIFIED
#include "MemMap.h"  /*lint !e9019 OTHER [MISRA 2012 Rule 20.1, advisory] OTHER AUTOSAR specified way of using MemMap*/

#define LOG_START_SEC_VAR_INIT_UNSPECIFIED
#include "MemMap.h"  /*lint !e9019 OTHER [MISRA 2012 Rule 20.1, advisory] OTHER AUTOSAR specified way of using MemMap*/
struct LogH *logPtr = &Log;
#define LOG_STOP_SEC_VAR_INIT_UNSPECIFIED
#include "MemMap.h"  /*lint !e9019 OTHER [MISRA 2012 Rule 20.1, advisory] OTHER AUTOSAR specified way of using MemMap*/

/* ----------------------------[private functions]---------------------------*/
/* ----------------------------[public functions]----------------------------*/


/**
 * strncpy that fill space from where src ends to n with space
 *
 * @param dst Pointer to destination
 * @param src Pointer to source
 * @param n How many characters to copy.
 *
 * @return Pointer to the destination.
 */
static char *strncpy_space(char *dst, const char *src, size_t n)
{
    char *d = dst;

    /* copy first part */
    while (n > 0 && *src != '\0') {
        *d++ = *src++;
        --n;
    }

    /* fill with space */
    while (n > 0 ) {
        *d++ = ' ';
        --n;
    }
    return dst;
}

/**
 * @brief   Log a string
 *
 * @param[in] lh        handle
 * @param[in] str       Pointer to a string
 */

void log_s( const char *name , const char *str ) {

    struct LogH *lh = logPtr;
    LogEntryT *le = &lh->log[lh->cnt];

    strncpy_space(le->str,name,LOG_NAME_SIZE);
    strncpy_space(&le->str[LOG_POS1],str,LOG_LEFT);
    le->timestamp = TIMER_TICK2US(Timer_GetTicks());
    lh->cnt = (lh->cnt + 1) % LOG_SIZE;

}

/**
 * @brief   Log a string and a string, will use half of the buffer for string
 *          str and the rest for str2
 *
 * @param[in] lh        handle
 * @param[in] str       Pointer to string
 * @param[in] str2      Pointer to string
 */

void log_s_s( const char *name , const char *str, const char *str2 ) {
    struct LogH *lh = logPtr;
    LogEntryT *le = &lh->log[lh->cnt];

    strncpy_space(le->str,name,LOG_NAME_SIZE);
    strncpy_space(&le->str[LOG_POS1],str,LOG_STR);
	strncpy_space(&le->str[LOG_POS2],str2,LOG_STR);
    le->timestamp = TIMER_TICK2US(Timer_GetTicks());
    lh->cnt = (lh->cnt + 1) % LOG_SIZE;
}

/**
 * @brief   Log a string and a uint32
 *
 * @param[in] lh        handle
 * @param[in] str       Pointer to a string
 * @param[in] val       hex value
 */

void log_s_u32( const char *name , const char *str, uint32 val) {
    struct LogH *lh = logPtr;
    char _str[10] = {"0x"};
    LogEntryT *le = &lh->log[lh->cnt];

    strncpy_space(le->str,name,LOG_NAME_SIZE);
    strncpy_space(&le->str[LOG_POS1],str,LOG_STR);
    xtoa(val,&_str[2],16,0);
    strncpy_space(&le->str[LOG_POS2],_str,LOG_STR);
    le->timestamp = TIMER_TICK2US(Timer_GetTicks());
    lh->cnt = (lh->cnt + 1) % LOG_SIZE;
}

/**
 * @brief  Log an u8 array
 *
 * @param[in] lh        handle
 * @param[in] str       Pointer to string, 4 chars only
 * @param[in] data      Pointer to data
 * @param[in] num       Number of data in the array.
 */

void log_s_a8( const char *name , const char *str, uint8 *data, uint8 num ) {
    struct LogH *lh = logPtr;
    LogEntryT *le = &lh->log[lh->cnt];
    uint32 j =LOG_NAME_SIZE;
    uint32 off;

    strncpy_space(le->str,name,LOG_NAME_SIZE);
    strncpy_space(&le->str[j],str,4);
    j += 4;
    num = MIN(num, ((LOG_LEFT - 4)/3));

    for (uint32 i=0;(i<num);i++,j+=3) {
        if( data[i] < 0x10) {
            le->str[j] = '0';
            off = 1;
        } else {
            off = 0;
        }
        xtoa(data[i],&le->str[j+off],16,0);
        le->str[j+2] = ' ';

    }

    le->timestamp = TIMER_TICK2US(Timer_GetTicks());
    lh->cnt = (lh->cnt + 1) % LOG_SIZE;
}


void Log_Init( void ) {
#if defined(CFG_SHELL)
    SHELL_AddCmd(&logCmdInfo);
#endif
}

#if defined(CFG_SHELL)
/**
 * @brief  Function that prints the current log to console
 *
 * @param   argc    Argument count
 * @param   argv    Argument vector
 * @return
 */
static int shellCmdLog(int argc, char *argv[] ) {
    uint32 pr;
    uint32 us;
    uint32 us_cut;
    uint32 sec;

    if(argc == 1 ) {
        pr = MIN(logPtr->cnt,LOG_SIZE);
        for(uint32 i = 0;i<pr; i++ ) {
            us = TIMER_TICK2US(logPtr->log[i].timestamp);
            sec = us/1000000;
            us_cut = us - (sec * 1000000);

            printf("[%0d.%06d] [%03d] %s\n", sec, us_cut ,0, logPtr->log[i].str);
        }
    } else {
    }

    return 0;
}
#endif


