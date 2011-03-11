/*
 * sys_monitor.c
 *
 *  Created on: 15 sep 2010
 *      Author: jcar
 */

#include "Std_Types.h"
#include "sys_monitor.h"
#include "tcf.h"
#include <string.h>
#include "internal.h"
#include "xtoa.h"

const char SysMon_PID[] = "\"PID\":";
const char SysMon_Prio[] = "\"Priority\":";
const char SysMon_Stackstart[] = "\"StackStart\":";
const char SysMon_Stackend[] = "\"StackEnd\":";
const char SysMon_Stackcurr[] = "\"StackCurr\":";
const char SysMon_State[] = "\"State\":";

static void AddState(char *buf, state_t state)
{
	/* "RSDZTW" where
	R is running,
	S is sleeping in an interruptible wait,
	D is waiting in uninterruptible disk sleep,
	Z is zombie,
	T is traced or stopped (on a signal), and
	W is paging */
	switch(state)
	{
	case ST_READY:
		mystrcat(buf,"\"D\"");
		break;
	case ST_WAITING:
		mystrcat(buf,"\"S\"");
		break;
	case ST_SUSPENDED:
		mystrcat(buf,"\"T\"");
		break;
	case ST_RUNNING:
		mystrcat(buf,"\"R\"");
		break;
	case ST_NOT_STARTED:
		mystrcat(buf,"\"T\"");
		break;
	}
}

uint16_t handle_SysMonEvent(TCF_Event* event, char* buf){
	return 0;
}


uint16_t handle_SysMonCommand(TCF_Command* command, char* buf) {
	OsTaskVarType *iterPcbPtr;

	/* Start building return message */
	start_tcf_field(buf, (char *)TCF_S_R); /* Start */
	append_tcf_field(buf, command->token);  /* Token */

	if (strcmp(command->commandName, TCF_getChildren) == 0) {
		boolean first = TRUE;

		/* Add error field */
		mystrcat(buf, JSON_null);
		mystrcat(buf, TCF_S_EOFIELD_MARKER);

		/* Add data field */
		mystrcat(buf, JSON_ListStart);
		TAILQ_FOREACH(iterPcbPtr,& Os_Sys.pcb_head,pcb_list) {
			if(iterPcbPtr->proc_type < 4)
			{
				if(first){
					first = FALSE;
				}else{
					mystrcat(buf, JSON_Separator);
				}
				mystrcat(buf, JSON_Stringify);
				mystrcat(buf, iterPcbPtr->name);
				mystrcat(buf, JSON_Stringify);
			}
		}
		mystrcat(buf, JSON_ListEnd);
		mystrcat(buf, TCF_S_EOFIELD_MARKER);

	} else if (strcmp(command->commandName, TCF_getContext) == 0) {
		char tmp[20] = "";

		/* Add error field */
		mystrcat(buf, JSON_null);
		mystrcat(buf, TCF_S_EOFIELD_MARKER);

		/* Add data field */
		char *arg = command->arguments + 1; /* add 1 for " */
		TAILQ_FOREACH(iterPcbPtr,& Os_Sys.pcb_head,pcb_list) {
			if(strncmp(arg, iterPcbPtr->name, strlen(command->arguments)-2) == 0)
			{
				break;
			}
		}
		mystrcat(buf, JSON_ObjStart);
	    mystrcat(buf, TCF_ID);
		mystrcat(buf, command->arguments);

		mystrcat(buf, JSON_Separator);
		mystrcat(buf, TCF_File);
		mystrcat(buf, command->arguments);

		mystrcat(buf, JSON_Separator);
		mystrcat(buf, SysMon_PID);
		ultoa(iterPcbPtr->pid,tmp,10);
		mystrcat(buf, tmp);

		mystrcat(buf, JSON_Separator);
		mystrcat(buf, SysMon_Prio);
		ultoa(iterPcbPtr->prio,tmp,10);
		mystrcat(buf, tmp);

		mystrcat(buf, JSON_Separator);
		mystrcat(buf, SysMon_Stackstart);
		ultoa((uint32_t)iterPcbPtr->stack.top,tmp,10);
		mystrcat(buf, tmp);

/*		mystrcat(buf, JSON_Separator);
		mystrcat(buf, SysMon_Stackend);
		ultoa((uint32_t)iterPcbPtr->stack.top + iterPcbPtr->stack.size,tmp,10);
		mystrcat(buf, tmp);

		mystrcat(buf, JSON_Separator);
		mystrcat(buf, SysMon_Stackcurr);
		ultoa((uint32_t)iterPcbPtr->stack.curr,tmp,10);
		mystrcat(buf, tmp);
*/
		mystrcat(buf, JSON_Separator);
		mystrcat(buf, SysMon_State);
		AddState(buf,iterPcbPtr->state);

		mystrcat(buf, JSON_ObjEnd);
		mystrcat(buf, TCF_S_EOFIELD_MARKER);
	}

	convert_to_tcf_message(buf);
	uint16_t len = message_length(buf, TCF_MAX_FIELD_LENGTH);

	return len;
}
