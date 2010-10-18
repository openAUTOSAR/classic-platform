/*
 * tcf_cfg.c
 *
 *  Created on: 15 sep 2010
 *      Author: jcar
 */
#include "Tcf_Cfg.h"

//Service includes
#include "sys_monitor.h"
#include "streams.h"

const char locator_hello[] = "E\0Locator\0Hello\0[\"Locator\",\"SysMonitor\",\"AsciiStreams\",\"FileSystem\"]\0\x03\x01";

const TCF_Service_Info tcfServiceCfgList[] = {
		{"Locator", handle_LocatorCommand, handle_LocatorEvent},
		{"FileSystem", handle_FileSystemCommand, handle_FileSystemEvent},
		{"SysMonitor", handle_SysMonCommand, handle_SysMonEvent},
		{"AsciiStreams", handle_StreamsCommand, handle_StreamsEvent},
		{"NULL", NULL},
};
