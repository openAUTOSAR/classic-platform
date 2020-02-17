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


#ifndef SHELL_H_
#define SHELL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "sys/queue.h"
#include "arc.h"

#define SHELL_F_

typedef int (*ShellFuncT)(int argc, char *argv[]);

typedef struct ShellCmd {
    ShellFuncT func;
    int   argMin;	/* excluding cmd, from 0 -> */
    int   argMax;   /* excluding cmd, from 0 -> */
    const char *cmd;
    const char *shortDesc;
    const char *longDesc;
    TAILQ_ENTRY(ShellCmd) cmdEntry;
} ShellCmdT;

int SHELL_AddCmd(ShellCmdT *shellCmd);
int SHELL_RunCmd(const char *cmdArgs, int *cmdRv );
int SHELL_Init( void );
int SHELL_Mainloop( void );

#define SHELL_E_OK				0
#define SHELL_E_CMD_TOO_LONG	1
#define SHELL_E_CMD_IS_NULL		2
#define SHELL_E_NO_SUCH_CMD		3

#define _SHELL_MAJOR_        0
#define _SHELL_MINOR_        2
#define _SHELL_PATCHLEVEL_   0


#define SHELL_VERSION        (_SHELL_MAJOR_ * 10000 + _SHELL_MINOR_ * 100 + _SHELL_PATCHLEVEL_)
#define SHELL_VERSION_STR    STRSTR__(_SHELL_MAJOR_) "." STRSTR__(_SHELL_MINOR_) "." STRSTR__(_SHELL_PATCHLEVEL_)

#ifdef __cplusplus
}
#endif

#endif /* SHELL_H_ */
