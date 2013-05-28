/**MARTEN_HEADER**/


#ifndef SHELL_H_
#define SHELL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "sys/queue.h"

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


#ifdef __cplusplus
}
#endif

#endif /* SHELL_H_ */
