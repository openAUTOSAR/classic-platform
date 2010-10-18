/*
 * streams.h
 *
 *  Created on: 15 sep 2010
 *      Author: jcar
 */

#ifndef STREAMS_H_
#define STREAMS_H_

#include "Std_Types.h"
#include "tcf.h"

uint8_t TCF_TTY_SendChar (uint8_t ch);
void init_streams();

uint16_t handle_StreamsCommand(TCF_Command* command, char* buf);
uint16_t handle_StreamsEvent(TCF_Event* event, char* buf);

#endif /* STREAMS_H_ */
