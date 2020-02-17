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

#ifndef STREAMS_H_
#define STREAMS_H_

#include "Std_Types.h"
#include "tcf.h"

uint8_t TCF_TTY_SendChar (uint8_t ch);
void init_streams();

uint16_t handle_StreamsCommand(TCF_Command* command, char* buf);
uint16_t handle_StreamsEvent(TCF_Event* event, char* buf);

#endif /* STREAMS_H_ */
