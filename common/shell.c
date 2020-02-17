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
 *  DESCRIPTION
 *    Provides functions to register and run shell commands.
 *
 *    - Will run a cmd on either CR or LF.
 *    - History buffer
 *    - TAB completion.
 *    - Backspace support.
 *    - Supports blocking or non-blocking driver.
 *    - Define configured:
 *      SHELL_CMD_NO_ML    - Define if you don't want a mainloop
 *      SHELL_CMDLINE_MAX  - Maximum characters input for a command
 *      SHELL_HIST_CNT     - Maximum history
 *      SHELL_PROMPT       - The prompt
 */

/* ----------------------------[Includes]------------------------------------*/

#include <ctype.h>
#include <stdlib.h>
#include <sys/queue.h>
#include <string.h>
#include "arc_assert.h"
#include <stdio.h>
#include "shell.h"

/* ----------------------------[Private define]------------------------------*/

/* The maximum number of arguments when calling a shell function */
#define MAX_ARGS		10

#ifndef SHELL_CMDLINE_MAX
#define SHELL_CMDLINE_MAX		40
#endif

/* Number cmds in history */
#ifndef SHELL_HIST_CNT
#define SHELL_HIST_CNT   10
#endif

#ifndef SHELL_PROMPT
#define SHELL_PROMPT    "[Arccore]"
#endif

/* Escape sequence */
#define ESC_H   "\x1b\x5b"

/* ----------------------------[Private macro]-------------------------------*/

static inline int dec(int value, int max) {
    int rv;

    if( value == 0 ) {
         rv = (max - 1);
    } else {
         rv = value - 1;
    }
    return rv;
}

static inline int inc(int value, int max) {
    return ((value+1) % max);
}

/* ----------------------------[Private typedef]-----------------------------*/


struct shellWord {
    int initialized;
    TAILQ_HEAD(,ShellCmd) cmdHead;
};

typedef struct Hist {
    int idx;
    int viewIdx;
    int num;
    char buff[SHELL_HIST_CNT][SHELL_CMDLINE_MAX];
} HistType;




/* ----------------------------[Private function prototypes]-----------------*/

static int shellHelp(int argc, char *argv[] );
static int shellHistory(int argc, char *argv[] );
extern char *strtok_r(char *s1, const char *s2, char **s3);
static void doPrompt( char * );


/* ----------------------------[Private variables]---------------------------*/
struct shellWord shellWorld;

static ShellCmdT helpInfo  = {
        shellHelp,
        0,1,
        "help",
        "help <cmd>",
        "Show all commands all help no a specific command\n",
        {NULL,NULL}
};

static ShellCmdT historyInfo  = {
        shellHistory,
        0,1,
        "history",
        "history",
        "Show command history\n",
        {NULL,NULL}
};


static char cmdBuf[40];

HistType hist;


/* ----------------------------[Private functions]---------------------------*/

/**
 * Split and string into tokens and strip the token from whitespace.
 *
 * @param s1 See strtok_r documentation
 * @param s2 See strtok_r documentation
 * @param s3 See strtok_r documentation
 * @return
 */
static char *strtokAndTrim(char *s1, const char *s2, char **s3)
{
#if defined (WIN32)
  char *str = strtok_s(s1, s2, s3);
#else
  char *str = strtok_r(s1, s2, s3);
#endif
  return str;
  //return trim(str);
}

/**
 * Implements the help command. Either type just "help" to list all available
 * command that is registered or "help <cmd>" to get detailed help on a specific
 * command.
 *
 * @param argc
 * @param argv
 * @return
 */

static int shellHelp(int argc, char *argv[] ) {
    char *cmd = NULL;
    ShellCmdT *iCmd;

    if(argc == 1 ) {
        /* display "help" */
        printf("List of commands:\n");
        TAILQ_FOREACH(iCmd,&shellWorld.cmdHead,cmdEntry ) {
            printf("%-15s - %s\n",iCmd->cmd, iCmd->shortDesc);
        }
    } else {
        cmd = argv[1];
        /* display "help <cmd>" */
        TAILQ_FOREACH(iCmd,&shellWorld.cmdHead,cmdEntry ) {
            if( strcmp(cmd,iCmd->cmd) == 0 ) {
                printf("%s\n",iCmd->longDesc);
            }
        }

    }

    return 0;
}


static int shellHistory(int argc, char *argv[] ) {

    if(argc == 1 ) {
        /* display "history" */
        printf("\nHistory:\n");
        if (hist.num == SHELL_HIST_CNT ) {
            int j = hist.idx;

            for(int i=0;i<SHELL_HIST_CNT;i++) {
                printf("%3d %s\n", i, &hist.buff[j][0]);
                j = inc(j,SHELL_HIST_CNT);
            }
        } else {
            for(int i=0;i<hist.num;i++) {
                printf("%3d %s\n", i, &hist.buff[i][0]);
            }
        }
    }
    return 0;
}

/* ----------------------------[Public functions]----------------------------*/

/**
 *
 * @return
 */
int SHELL_Init( void ) {
    shellWorld.initialized = 1;
    TAILQ_INIT(&shellWorld.cmdHead);

    SHELL_AddCmd(&helpInfo);
    SHELL_AddCmd(&historyInfo);

    puts("\nArcCore Shell version: " SHELL_VERSION_STR "\n");
    doPrompt("");

    return 0;
}

/**
 * Add a command to list of commands
 * @param shellCmd
 * @return
 */

int SHELL_AddCmd(ShellCmdT *shellCmd) {
    if(shellWorld.initialized != 1 ) {
        SHELL_Init();
    }

    TAILQ_INSERT_TAIL(&shellWorld.cmdHead, shellCmd, cmdEntry );

    return 0;
}

/**
 * Run a command from current context
 *
 * @param cmdArgs    The command string
 * @param cmdRv		The return value from the command run
 * @return
 */
int SHELL_RunCmd(const char *cmdArgs, int *cmdRv ) {
    char *delim = " \t\n";
    char *token_r;
    ShellCmdT *iCmd;
    size_t len;
    char *cmdStr;
    ShellCmdT *runCmd = NULL;
    int argc = 0;
    char *argv[MAX_ARGS];
    char *arg;

    *cmdRv = 1;

    if (cmdArgs == NULL) {
        return SHELL_E_CMD_IS_NULL;
    }

    len = strlen(cmdArgs) + 1; /* Add termination */
    if (sizeof(cmdBuf) < len) {
        return SHELL_E_CMD_TOO_LONG;
    }

    /* Copy to a buffer we can mess with so that strtok have
     * something to work on */
    memcpy(cmdBuf, cmdArgs, len);
    cmdStr = strtokAndTrim(cmdBuf, delim, &token_r);

    if (cmdStr == NULL) {
        return SHELL_E_CMD_IS_NULL;
    }

    /* Search for the command */
    TAILQ_FOREACH(iCmd,&shellWorld.cmdHead,cmdEntry ) {
        if( strcmp(cmdStr,iCmd->cmd) == 0 ) {
            runCmd = iCmd;
            break;
        }
    }

    /* Check arg count and deliver them into argc and argv */
    if (runCmd != NULL) {
        /* Add the cmd */
        argv[0] = cmdStr;
        argc++;

        while( (arg = strtokAndTrim(NULL, delim, &token_r)) != NULL ) {
            ASSERT(argc<MAX_ARGS);
            argv[argc++] = arg;
        }

        *cmdRv = runCmd->func(argc, argv);

    } else {
        printf("\"%s\" : command not found\n",cmdStr);
        return SHELL_E_NO_SUCH_CMD;
    }
    return SHELL_E_OK;
}

static void doPrompt( char *s ) {
    printf( SHELL_PROMPT " $ %s",s);
}

static int getInput( void ) {

    char c;
    static char cmdLine[SHELL_CMDLINE_MAX];
    static int lineIndex = 0;
    static int cmdRv;
    static int escState = 0;

    c = fgetc(stdin);
    // printf("Got 0x%x %d\n",c,c);
    if( lineIndex >= SHELL_CMDLINE_MAX ) {
        lineIndex = 0;
    }

//    printf("%d",lineIndex);

    if( escState != 0) {

        if( escState == 1 ) {
            /* Check for "[" (0x5b) */
            if( c == 0x5b ) {
                escState = 2;
            } else {
                /* Ops, something is wrong return out */
                escState = 0;
            }
        } else if( escState == 2 ) {
            /* Update prompt with history. Example
             * 11
             * 22
             * 33
             * - up key will go up here
             * - down key will go down
             * */
            if( c == 'A' ) {
                /* Go back to prompt and erase to the right */
                if( lineIndex != 0) {
                    printf(ESC_H "%dD" ESC_H "K",(lineIndex));
                }

                if( hist.num != 0 ) {
                    /* Go up one in history buffer */
                    hist.viewIdx = dec( hist.viewIdx, SHELL_HIST_CNT);
                    puts(&hist.buff[hist.viewIdx][0]);
                    lineIndex = strlen(&hist.buff[hist.viewIdx][0]);
                    memcpy(cmdLine,&hist.buff[hist.viewIdx][0],lineIndex);
                }
            } else if( c == 'B' ) {
                if( hist.viewIdx != hist.idx) {
                    /* Go back to prompt and erase to the right */
                    if( lineIndex != 0) {
                        printf(ESC_H "%dD" ESC_H "K",(lineIndex));
                    }

                    hist.viewIdx = inc( hist.viewIdx, SHELL_HIST_CNT);
                    puts(&hist.buff[hist.viewIdx][0]);
                    lineIndex = strlen(&hist.buff[hist.viewIdx][0]);
                    memcpy(cmdLine,&hist.buff[hist.viewIdx][0],lineIndex);
                }
            }
            escState = 0;
        }

        return SHELL_E_OK;
    }

    if( c == '\x1b' ) {
        /* Escape sequence
         * http://ascii-table.com/ansi-escape-sequences-vt-100.php
         * http://ascii-table.com/documents/vt100/chapter3.php#S3.3.3
         */
        escState = 1;
    } else if( c == '\b') {
        /* backspace */
        if( lineIndex != 0 ) {
            lineIndex--;
            puts(ESC_H "D" ESC_H "K");
        }

        // fputs("\e\x50",stdout);
    } else if( c == '\n' || c == '\r' ) {
        puts("\n");
        cmdLine[lineIndex++] = '\n';
        cmdLine[lineIndex] = '\0';

        if( lineIndex > 1 ) {
            /* Copy to history buffer */
            memcpy(&hist.buff[hist.idx][0],cmdLine,lineIndex);
            hist.buff[hist.idx][lineIndex-1] = '\0';

            hist.idx = inc(hist.idx,SHELL_HIST_CNT);
            hist.num++;
            hist.num = MIN(hist.num, SHELL_HIST_CNT);
            /* Reset history view */
            hist.viewIdx = hist.idx;

            SHELL_RunCmd(cmdLine,&cmdRv);
        }

        /* Add prompt again */
        lineIndex = 0;

        doPrompt("");
    }  else if( c == '\t' ) {
        /* tab completion */
        if( lineIndex > 0 ) {
            /* Find all commands that match */

            ShellCmdT *itCmd;            ShellCmdT *sCmd = NULL;
            uint16 cnt = 0;
            uint16 first = 0;

            /* Search for the command */
            TAILQ_FOREACH(itCmd,&shellWorld.cmdHead,cmdEntry ) {
                if( strncmp(itCmd->cmd,cmdLine,lineIndex) == 0 ) {
                    if( sCmd == NULL ) {
                        /* First time, just save, no print */
                        sCmd = itCmd;
                    } else {
                        if (first == 0 ) {
                            printf("\n %s\n",sCmd->cmd);
                            first = 1;
                        }
                        printf(" %s\n",itCmd->cmd);
                    }
                    cnt++;
                }
            }
            if( cnt == 0u ) {
                /* We do nothing.. */
            } else  if( cnt == 1u ) {
                /* Only one match, add to cmd line */
                puts(&sCmd->cmd[lineIndex]);
                lineIndex = strlen(sCmd->cmd);
                strcpy(cmdLine, sCmd->cmd);
            } else {
                cmdLine[lineIndex] = '\0';
                doPrompt(cmdLine);
            }
        }
    } else if ( c != 0 ) {
        cmdLine[lineIndex++] = c;
        fputc(c,stdout);
    }
    return SHELL_E_OK;
}



int SHELL_Mainloop( void ) {

#if defined(SHELL_CMD_NO_ML)
    getInput();
#else
    for(;;) {
        getInput();
    }
#endif

    return SHELL_E_OK;
}


