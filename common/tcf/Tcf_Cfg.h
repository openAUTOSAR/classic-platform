/*
 * Tcf_Cfg.h
 *
 *  Created on: 15 sep 2010
 *      Author: jcar
 */

#ifndef TCF_CFG_H_
#define TCF_CFG_H_

#include "tcf.h"

typedef struct {
	char name[16];
	TCF_Service_CommandHandler commandHandler;
	TCF_Service_EventHandler eventHandler;
} TCF_Service_Info;

extern const TCF_Service_Info tcfServiceCfgList[];

extern const char locator_hello[];

#endif /* TCF_CFG_H_ */
