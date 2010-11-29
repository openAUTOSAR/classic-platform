/*
 * tcf.h
 *
 *  Created on: 15 sep 2010
 *      Author: jcar
 */

#ifndef TCF_H_
#define TCF_H_

#include "Std_Types.h"

#define TCF_C_EOFIELD           '\x00'
#define TCF_C_EOFIELD_MARKER    '\x04'

#define TCF_S_EOFIELD  "\x00"
#define TCF_S_EOFIELD_MARKER "\x04"
#define TCF_S_R "R"
#define TCF_S_EOM  "\x03\x01"

#define TCF_FIELD_DELIMITER		('\x00')
#define TCF_MAX_FIELD_LENGTH	(1400)

extern const char TCF_getChildren[];
extern const char TCF_getContext[];
extern const char TCF_ID[];
extern const char TCF_File[];
extern const char TCF_Service[];
extern const char TCF_Code[];

extern const char JSON_ObjStart[];
extern const char JSON_ObjEnd[];
extern const char JSON_null[];
extern const char JSON_Separator[];

extern const char JSON_Stringify[];
extern const char JSON_ListStart[];
extern const char JSON_ListEnd[];

/* Standard error codes: */

typedef enum{
    TCF_OTHER               = 1,
    TCF_JSON_SYNTAX         = 2,
    TCF_PROTOCOL            = 3,
    TCF_BUFFER_OVERFLOW     = 4,
    TCF_CHANNEL_CLOSED      = 5,
    TCF_COMMAND_CANCELLED   = 6,
    TCF_UNKNOWN_PEER        = 7,
    TCF_BASE64              = 8,
    TCF_EOF                 = 9,
    TCF_ALREADY_STOPPED     = 10,
    TCF_ALREADY_EXITED      = 11,
    TCF_ALREADY_RUNNING     = 12,
    TCF_ALREADY_ATTACHED    = 13,
    TCF_IS_RUNNING          = 14,
    TCF_INV_DATA_SIZE       = 15,
    TCF_INV_CONTEXT         = 16,
    TCF_INV_ADDRESS         = 17,
    TCF_INV_EXPRESSION      = 18,
    TCF_INV_FORMAT          = 19,
    TCF_INV_NUMBER          = 20,
    TCF_INV_DWARF           = 21,
    TCF_SYM_NOT_FOUND       = 22,
    TCF_UNSUPPORTED         = 23,
    TCF_INV_DATA_TYPE       = 24,
    TCF_INV_COMMAND         = 25,
} TCF_ErrorCode_t;


typedef struct {
	char* serviceName;
	char* eventName;
	char* eventData;
} TCF_Event;

typedef struct {
	char* token;
	char* serviceName;
	char* commandName;
	char* arguments;
	uint16_t args_len;
} TCF_Command;

typedef struct {
	char* token;
	char* error;
	char* data;
} TCF_Result;

typedef uint16_t (*TCF_Service_CommandHandler)(TCF_Command* command, char* buf);
typedef uint16_t (*TCF_Service_EventHandler)(TCF_Event* command, char* buf);

void InitTcf(void);
uint16_t message_length(const char* msg, uint16_t max_length);
char* get_next_tcf_field(char* chars, uint16_t maxlen);
void start_tcf_field(char* start, char* field);
void append_tcf_field(char* start, char* field);
void convert_to_tcf_message(char* start);
void tcf_compile_result(TCF_Result* result, char* buffer);

/* Locator and Filesystem handlers are in tcf.c file. To be moved */
uint16_t handle_LocatorCommand(TCF_Command* command, char* buf);
uint16_t handle_LocatorEvent(TCF_Event* event, char* buf);
uint16_t handle_FileSystemCommand(TCF_Command* command, char* buf);
uint16_t handle_FileSystemEvent(TCF_Event* event, char* buf);

char *mystrcat(char *s1, const char *s2);

#endif /* TCF_H_ */
