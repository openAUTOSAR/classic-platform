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

#ifndef PMC_SYNC_STATUS_READER_H_
#define PMC_SYNC_STATUS_READER_H_
#ifndef _WIN32
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

#include "pmc_sync_status_common.h"

#endif
typedef enum {
    SYNC_STATUS_CONNECTED,
    SYNC_STATUS_TEMP_DISCONNECTED,
    SYNC_STATUS_PERM_DISCONNECTED
} sync_status_reader_conn_state;

#ifndef _WIN32
struct sockaddr_un sync_status_uds_skt_addr;

int reader_uds_skt;

int init_sync_status_reader();

sync_status_reader_conn_state connect_sync_status_reader();

void disconnect_sync_status_reader(int close_permanently);
#endif
#endif /* PMC_SYNC_STATUS_READER_H_ */
