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
