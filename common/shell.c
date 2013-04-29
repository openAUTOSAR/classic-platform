/* -------------------------------- Arctic Core ------------------------------
 * Arctic Core - the open source AUTOSAR platform http://arccore.com
 *
 * Copyright (C) 2009  ArcCore AB <contact@arccore.com>
 *
 * This source code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by the
 * Free Software Foundation; See <http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 * -------------------------------- Arctic Core ------------------------------*/

/*
 *  DESCRIPTION
 *    Provides functions to register and run shell commands.
 */

/* ----------------------------[Includes]------------------------------------*/

#include <ctype.h>
#include <stdlib.h>
#include <sys/queue.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include "shell.h"

/* ----------------------------[Private define]------------------------------*/

/* The maximum number of arguments when calling a shell function */
#define MAX_ARGS		10

#define CMDLINE_MAX		40


/* ----------------------------[Private macro]-------------------------------*/
/* ----------------------------[Private typedef]-----------------------------*/


struct shellWord {
	int initialized;
	TAILQ_HEAD(,ShellCmd) cmdHead;
};

/* ----------------------------[Private function prototypes]-----------------*/

static int shellHelp(int argc, char *argv[] );
extern char *strtok_r(char *s1, const char *s2, char **s3);


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

static char cmdBuf[40];


/* ----------------------------[Private functions]---------------------------*/

/**
 * Removes backspace from string s and returns the null
 * terminated string in d.
 *
 * @param d  String without backspace
 * @param s  String with potential backspaces.
 * @return   Returns d.
 */
static char *fix(char *d, char *s) {
	int i = 0;
	int di = 0;

	while(s[i]) {
		if( s[i] == '\b' ) {
			--di;
			if(di<0) {
				di = 0;
			}
		} else {
			d[di++] = s[i];
		}
		i++;
	}
	d[di] = '\0';
	return &d[0];
}

/**
 * Trim initial spaces...
 *
 * @param s
 * @return
 */
static char *trim(char *s)
{
   while(*s && (isspace((int)*s))) {
   	s++;
   }

   return(s);
}


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
//			printf("%-15s - %s\n",iCmd->cmd, iCmd->shortDesc);
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

	/* Remove backspace */
//	cmdArgs = fix((char *)cmdArgs, (char *)cmdArgs);

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
			assert(argc<MAX_ARGS);
			argv[argc++] = arg;
		}

		*cmdRv = runCmd->func(argc, argv);

	} else {
		printf("No such command:\"%s\",strlen=%d\n",cmdStr,(int)strlen(cmdStr));
		return SHELL_E_NO_SUCH_CMD;
	}
	return SHELL_E_OK;
}

static void doPrompt( void ) {
	puts("[ArcCore] $ ");
}


int SHELL_Mainloop( void ) {
	char c;
	static char cmdLine[CMDLINE_MAX];
	int lineIndex = 0;
	int cmdRv;

	puts("ArcCore Shell version 0.1\n");
	doPrompt();

	for(;;) {
		c = fgetc(stdin);
		//printf("Got %c\n",(char)rv);
		if( lineIndex >= CMDLINE_MAX ) {
			lineIndex = 0;
		}

		if( c == '\b') {
			lineIndex--;
			fputc(c,stdout);
			// fputs("\e\x50",stdout);
		} else if( c == '\n' || c == '\r' ) {
			puts("\n");
			cmdLine[lineIndex++] = '\n';
			cmdLine[lineIndex] = '\0';
			//printf("Got Cmd:%s\n",cmdLine);
			SHELL_RunCmd(cmdLine,&cmdRv);
			lineIndex = 0;
			doPrompt();
		} else {
			cmdLine[lineIndex++] = c;
			fputc(c,stdout);
		}

	}
}


