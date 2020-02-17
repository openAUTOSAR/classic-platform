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

#include "Std_Types.h"
#include "shell.h"
#include "timer.h"
#include <string.h>
#include <stdio.h>

#define LOG_MAX_STR 20
#define LOG_SIZE    100

#ifndef MIN
#define MIN(_x,_y) (((_x) < (_y)) ? (_x) : (_y))
#endif


#if defined(CFG_SHELL)
static int shellCmdLog(int argc, char *argv[] );
#endif

#if defined(CFG_SHELL)
static ShellCmdT logCmdInfo = {
    shellCmdLog,
    0,1,
    "log",
    "log",
    "Display the logs collected by the Shell logger.\n",
    {   NULL,NULL}
};
#endif

typedef struct Log {
    char        str[LOG_MAX_STR];
    uint32      logCnt;
    TimerTick   timestamp;
} LogType;

uint32 logCnt = 0;
LogType Log[LOG_SIZE];

void Log_Add( const char *str ) {
    strncpy(Log[logCnt].str,str,LOG_MAX_STR);
    Log[logCnt].timestamp = Timer_GetTicks();
    Log[logCnt].logCnt = logCnt;
    logCnt = (logCnt + 1) % LOG_SIZE;
}

void Log_Add2( const char *str, const char *str2 ) {
	Log[logCnt].str[4] = ' ';
	Log[logCnt].str[5] = ' ';
	Log[logCnt].str[6] = ' ';
	Log[logCnt].str[7] = ' ';
	Log[logCnt].str[8] = ' ';
	Log[logCnt].str[9] = ' ';
	int s = strlen(str);
	memcpy(Log[logCnt].str,str,s);
	for(int a=s;a<LOG_MAX_STR/2;a++) {
		Log[logCnt].str[a] = ' ';
	}
    strncpy(&Log[logCnt].str[LOG_MAX_STR/2],str2,LOG_MAX_STR/2);
    Log[logCnt].timestamp = Timer_GetTicks();
    Log[logCnt].logCnt = logCnt;
    logCnt = (logCnt + 1) % LOG_SIZE;
}



void Log_Init( void ) {
#if defined(CFG_SHELL)
    SHELL_AddCmd(&logCmdInfo);
#endif
}

#if defined(CFG_SHELL)
/**
 *
 * @param argc
 * @param argv
 * @return
 */
static int shellCmdLog(int argc, char *argv[] ) {
    uint32 pr;
    uint32 us;
    uint32 us_cut;
    uint32 sec;

    if(argc == 1 ) {
        pr = MIN(logCnt,LOG_SIZE);
        for(uint32 i = 0;i<pr; i++ ) {
            us = TIMER_TICK2US(Log[i].timestamp);
            sec = us/1000000;
            us_cut = us - (sec * 1000000);

            printf("[%0d.%06d] [%03d] %s\n", sec, us_cut ,Log[i].logCnt, Log[i].str);
        }
    } else {
    }

    return 0;
}
#endif


