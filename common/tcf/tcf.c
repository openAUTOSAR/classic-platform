/*
 * tcf.c
 *
 *  Created on: 15 sep 2010
 *      Author: jcar
 */

#define USE_LDEBUG_PRINTF
#include "debug.h"

#include "Std_Types.h"
#include <string.h>
#ifdef USE_LWIP
#include "lwip/tcp.h"
#include "lwip/udp.h"
#include "tcp_handler.h"
#endif
#include "tcf.h"
#include "Tcf_Cfg.h"
#include "xtoa.h"
#include "streams.h"


#define TCF_TCP_PORT	1534

const char TCF_getChildren[] = "getChildren";
const char TCF_getContext[] = "getContext";
const char TCF_ID[] = "\"ID\":";
const char TCF_File[] = "\"File\":";
const char TCF_Service[] = "\"Service\":";
const char TCF_Code[] = "\"Code\":";


const char JSON_ObjStart[] = "{";
const char JSON_ObjEnd[] = "}";
const char JSON_null[] = "null";
const char JSON_Separator[] = ",";

const char JSON_Stringify[] = "\"";
const char JSON_ListStart[] = "[";
const char JSON_ListEnd[] = "]";

struct tcf_tcp_state
{
    int num;
};

char *mystrcat(char *s1, const char *s2)
{
  char *s;

  for(s = s1;*s;++s)
    ;

  while(*s2)
    *s++ = *s2++;
  *s = '\0';

  return s1;
}


uint16_t message_length(const char* msg, uint16_t max_length) {
	uint16_t i;
	for (i = 0; i < max_length; ++i) {
		if (msg[i] == '\x01') {
			return i+1;
		}
	}
	return max_length;
}

void start_tcf_field(char* start, char* field) {
	strcpy(start, field);
	mystrcat(start, TCF_S_EOFIELD_MARKER);
}

void append_tcf_field(char* start, char* field) {
	mystrcat(start, field);
	mystrcat(start, TCF_S_EOFIELD_MARKER);
}

void convert_to_tcf_message(char* start) {
	mystrcat(start, TCF_S_EOM);
	size_t length = strlen(start);
	int i;
	for (i = 0; i < length; ++i) {
		if (start[i] == TCF_C_EOFIELD_MARKER) {
			start[i] = TCF_C_EOFIELD;
		}
	}
}

char* get_next_tcf_field(char* chars, uint16_t maxlen) {
	int i;
	for(i = 0; i < maxlen; i++) {
		if (chars[i] == TCF_FIELD_DELIMITER) {
			return &chars[i+1];
		}
	}
	return (char*) NULL;
}

char* get_next_tcf_msg(char* chars, uint16_t *len) {
	int i;
	for(i = 0; i < *len-1; i++) {
		if (chars[i] == '\x03' && chars[i+1] == '\x01') {
			*len -= (i + 1);
			return &chars[i+2];
		}
	}
	*len= 0;
	return (char*) NULL;
}

static Std_ReturnType parse_command(char* msg, TCF_Command* command, uint16_t len){
	char* curr = msg;

	curr = get_next_tcf_field(curr, len);
	if (curr == NULL) return E_NOT_OK;
	command->token = curr;

	curr = get_next_tcf_field(curr, len);
	if (curr == NULL) return E_NOT_OK;
	command->serviceName = curr;

	curr = get_next_tcf_field(curr, len);
	if (curr == NULL) return E_NOT_OK;
	command->commandName = curr;

	curr = get_next_tcf_field(curr, len);
	if (curr == NULL) return E_NOT_OK;
	command->arguments = curr;

	command->args_len = len -((uint32_t)curr - (uint32_t)msg);
	return E_OK;
}

Std_ReturnType parse_event(char* msg, TCF_Event* event){
	char* curr = msg;

	curr = get_next_tcf_field(curr, TCF_MAX_FIELD_LENGTH);
	if (curr == NULL) return E_NOT_OK;
	event->serviceName = curr;

	curr = get_next_tcf_field(curr, TCF_MAX_FIELD_LENGTH);
	if (curr == NULL) return E_NOT_OK;
	event->eventName = curr;

	curr = get_next_tcf_field(curr, TCF_MAX_FIELD_LENGTH);
	if (curr == NULL) return E_NOT_OK;
	event->eventData = curr;

	return E_OK;
}

uint16_t handle_LocatorCommand(TCF_Command* command, char* buf) {
	return 0;
}

uint16_t handle_LocatorEvent(TCF_Event* event, char* buf) {
	uint16_t len = 0;
	if(strcmp(event->eventName, "Hello") == 0)
	{
		len = message_length(locator_hello, TCF_MAX_FIELD_LENGTH);
		memcpy(buf,locator_hello,len);
	}else if(strcmp(event->eventName, "peerHeartBeat") == 0){

	}

	return len;
}

uint16_t handle_FileSystemCommand(TCF_Command* command, char* buf) {
	char tmp[20] = "";

	/* Start building return message */
	start_tcf_field(buf, (char *)TCF_S_R); /* Start */
	append_tcf_field(buf, command->token);  /* Token */

	/* Add error field */
	mystrcat(buf, JSON_ObjStart);

	mystrcat(buf, TCF_Code);
	ultoa(TCF_UNSUPPORTED,tmp,10);
	mystrcat(buf, tmp);

	mystrcat(buf, JSON_ObjEnd);
	mystrcat(buf, TCF_S_EOFIELD_MARKER);

	/* Add data field */
	mystrcat(buf, JSON_null);
	mystrcat(buf, TCF_S_EOFIELD_MARKER);

	convert_to_tcf_message(buf);
	uint16_t len = message_length(buf, TCF_MAX_FIELD_LENGTH);

	return len;
}

uint16_t handle_FileSystemEvent(TCF_Event* event, char* buf) {
	return 0;
}

/* Not reentrant so buffers can be static */
char outbuf[TCF_MAX_FIELD_LENGTH] = "";

static void handle_event(char *buf, uint16_t len)
{
	char *msg = (char *)buf;
	uint8 index;
	uint16_t outlen = 0;
	TCF_Event event;

	if (parse_event(msg, &event) == E_OK) {
		/* Find and call the requested agent */
		index = 0;
        while(tcfServiceCfgList[index].eventHandler != NULL){
			if(strcmp(event.serviceName, tcfServiceCfgList[index].name) == 0){
				outlen = tcfServiceCfgList[index].eventHandler(&event, outbuf);
				break;
			}
			index++;
		}
	}

	if(outlen > 0){
#ifdef USE_LWIP
		TcpSendData(outbuf, outlen);
#endif
	}
}

static void handle_command(char *buf, uint16_t len)
{
	char *msg = (char *)buf;
	uint8 index;
	uint16_t outlen = 0;
	TCF_Command command;

	if (parse_command(msg, &command, len) == E_OK) {
		//LDEBUG_PRINTF("C %s\n\r", command.token);
		/* Find and call the requested agent */
		index = 0;
		while(tcfServiceCfgList[index].commandHandler != NULL){
			if(strcmp(command.serviceName, tcfServiceCfgList[index].name) == 0){
				outlen = tcfServiceCfgList[index].commandHandler(&command, outbuf);
				break;
			}
			index++;
		}

		if(outlen > 0){
#ifdef USE_LWIP
			//LDEBUG_PRINTF("R %s\n\r", command.token);
			TcpSendData(outbuf, outlen);
#endif
		}else{
			LDEBUG_PRINTF("TCF: len=0:%s\n\r", command.token);
		}
	}else{
		LDEBUG_PRINTF("TCF: Parse failed:%s\n\r", command.token);
	}
}

static void handle_incoming(const void* buf, uint16_t len) {
	char *msg = (char *)buf;
	if(len > 0){
		do{
			if ((msg[0] == 'C') && (msg[1] == '\0')) {
				handle_command(msg, len);
			} else if ((msg[0] == 'E') && (msg[1] == '\0')) {
				handle_event(msg, len);
			}else{
                len=0;
            }

			/* Check if more than one message in buffer */
            if(len > 0){
                msg = get_next_tcf_msg(msg, &len);
            }
		}while((msg != NULL) && (len > 0));
	}
}

void InitTcf(void){
#ifdef USE_LWIP
    CreateTcpSocket(TCF_TCP_PORT, handle_incoming);
#endif
    init_streams();
}
