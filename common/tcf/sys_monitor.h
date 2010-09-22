/*
 * sys_monitor.h
 *
 *  Created on: 15 sep 2010
 *      Author: jcar
 */

#ifndef SYSMONITOR_H_
#define SYSMONITOR_H_

#include "Std_Types.h"
#include "tcf.h"

uint16_t handle_SysMonCommand(TCF_Command* command, char* buf);
uint16_t handle_SysMonEvent(TCF_Event* event, char* buf);

#endif /* SYSMONITOR_H_ */
