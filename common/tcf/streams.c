/*
 * streams.c
 *
 *  Created on: 15 sep 2010
 *      Author: jcar
 */

#include "Std_Types.h"
#include "streams.h"
#include "tcf.h"
#include <string.h>
#include "internal.h"
#include "xtoa.h"
#include "cirq_buffer.h"

const char Streams_PID[] = "\"PID\":";
const char Streams_Read[] = "read";
const char Streams_Write[] = "write";
const char Streams_Subscribe[] = "subcribe";
const char Streams_Unsubscribe[] = "unsubscribe";
const char Streams_Connect[] = "connect";
const char Streams_Disconnect[] = "disconnect";
const char Streams_EndofStream[] = "eos";

const char Streams_LostSize[] = "\"lost size\":";
const char Streams_EOS[] = "\"EOS\":";
const char Streams_true[] = "true";
const char Streams_false[] = "false";

typedef struct {
	char* id;
	char* size;
	char* data;
} TCF_Streams_Command;

#define TCF_TTY_SIZE 800

static boolean tty_initialized = FALSE;
static CirqBufferType cirqBuf;
char tty_buffer[TCF_TTY_SIZE] = "";

void init_streams()
{
	cirqBuf = CirqBuffStatCreate(tty_buffer, TCF_TTY_SIZE, sizeof(char));
	tty_initialized = TRUE;
}

uint8_t TCF_TTY_SendChar (uint8_t ch)
{
	int rv = 1;
	if(tty_initialized == TRUE){
		rv = CirqBuffPush(&cirqBuf,&ch);
	}

	return rv;
}
uint32_t TCF_TTY_ReadString(char *str, uint16_t max_len)
{
	char ch;
	int rv;
	int len = 0;
	if(tty_initialized == TRUE){
		do{
			rv = CirqBuffPop(&cirqBuf,&ch);
			if(rv==0){
				len++;
				*str++=ch;
				if(ch == '\0'){
					rv = 1;
				}
			}
		}while((rv == 0) && (len < max_len));
	}

	return len;
}


uint16_t handle_StreamsEvent(TCF_Event* event, char* buf){
	return 0;
}

static Std_ReturnType parse_read(char* msg, TCF_Streams_Command* command, uint16_t len){
	char* curr = msg;

	curr = get_next_tcf_field(curr, len);
	if (curr == NULL) return E_NOT_OK;
	command->id = curr;

	curr = get_next_tcf_field(curr, len);
	if (curr == NULL) return E_NOT_OK;
	command->size = curr;

	return E_OK;
}

static Std_ReturnType parse_write(char* msg, TCF_Streams_Command* command, uint16_t len){
	char* curr = msg;

	curr = get_next_tcf_field(curr, len);
	if (curr == NULL) return E_NOT_OK;
	command->id = curr;

	curr = get_next_tcf_field(curr, len);
	if (curr == NULL) return E_NOT_OK;
	command->size = curr;

	curr = get_next_tcf_field(curr, len);
	if (curr == NULL) return E_NOT_OK;
	command->data = curr;

	return E_OK;
}

static Std_ReturnType parse_id(char* msg, TCF_Streams_Command* command, uint16_t len){
	char* curr = msg;

	curr = get_next_tcf_field(curr, len);
	if (curr == NULL) return E_NOT_OK;
	command->id = curr;

	return E_OK;
}

char tmp_stream[200] = "";
uint16_t handle_StreamsCommand(TCF_Command* command, char* buf) {
	TCF_Streams_Command streams_cmd;

	/* Start building return message */
	start_tcf_field(buf, (char *)TCF_S_R); /* Start */
	append_tcf_field(buf, command->token);  /* Token */

	if (strcmp(command->commandName, Streams_Read) == 0) {
		/* C • <token> • Streams • read • <string: stream ID> • <int: size> • */
		/* R • <token> • <string: data> • <error report> • <int: lost size> • <boolean: EOS> */
		if(parse_read(command->arguments,&streams_cmd,command->args_len) != E_OK){
			return 0;
		}

		/* Add data field */
		mystrcat(buf, JSON_Stringify);
		int len = TCF_TTY_ReadString(tmp_stream, 199);
        if(len >= 200){
            return 0;
        }
		tmp_stream[len] = '\0'; /* Terminate to be sure */
		mystrcat(buf, tmp_stream);
		mystrcat(buf, JSON_Stringify);
		mystrcat(buf, TCF_S_EOFIELD_MARKER);

		/* Add error field */
		mystrcat(buf, JSON_null);
		mystrcat(buf, TCF_S_EOFIELD_MARKER);

		//mystrcat(buf, Streams_LostSize);
		ultoa(0,tmp_stream,10);
		mystrcat(buf,tmp_stream);
		mystrcat(buf, TCF_S_EOFIELD_MARKER);

		//mystrcat(buf, Streams_EOS);
		mystrcat(buf, Streams_false);
		mystrcat(buf, TCF_S_EOFIELD_MARKER);
	} else if (strcmp(command->commandName, Streams_Write) == 0) {
		/* C • <token> • Streams • write • <string: stream ID> • <int: size> • <string: data> • */
		/* R • <token> • <error report> */
		if(parse_write(command->arguments,&streams_cmd,command->args_len) != E_OK){
			return 0;
		}

		/* Add error field */
		mystrcat(buf, JSON_null);
		mystrcat(buf, TCF_S_EOFIELD_MARKER);
	} else if (strcmp(command->commandName, Streams_Subscribe) == 0) {
		/* R • <token> • <error report> */
		if(parse_id(command->arguments,&streams_cmd,command->args_len) != E_OK){
			return 0;
		}
		/* Add error field */
		mystrcat(buf, JSON_null);
		mystrcat(buf, TCF_S_EOFIELD_MARKER);
	}else if (strcmp(command->commandName, Streams_Unsubscribe) == 0) {
		/* R • <token> • <error report> */
		if(parse_id(command->arguments,&streams_cmd,command->args_len) != E_OK){
			return 0;
		}
		/* Add error field */
		mystrcat(buf, JSON_null);
		mystrcat(buf, TCF_S_EOFIELD_MARKER);
	}else if (strcmp(command->commandName, Streams_Connect) == 0) {
		/* R • <token> • <error report> */
		if(parse_id(command->arguments,&streams_cmd,command->args_len) != E_OK){
			return 0;
		}
		/* Add error field */
		mystrcat(buf, JSON_null);
		mystrcat(buf, TCF_S_EOFIELD_MARKER);
	}else if (strcmp(command->commandName, Streams_Disconnect) == 0) {
		/* R • <token> • <error report> */
		if(parse_id(command->arguments,&streams_cmd,command->args_len) != E_OK){
			return 0;
		}
		/* Add error field */
		mystrcat(buf, JSON_null);
		mystrcat(buf, TCF_S_EOFIELD_MARKER);
	}

	convert_to_tcf_message(buf);
	uint16_t len = message_length(buf, TCF_MAX_FIELD_LENGTH);

	return len;
}
