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

#ifndef PMC_SYNC_STATUS_COMMON_H_
#define PMC_SYNC_STATUS_COMMON_H_

#ifndef _WIN32
#include <sys/socket.h>
#include <sys/un.h>
#endif

typedef struct collective_sync_status {
    int in_sync;
    float offsetFromMaster;
} collective_sync_status;

/* sync_status server UNIX domain socket. */
#define SYNC_STATUS_UDS    "/tmp/ptp_sync_status.uds"

/* Returns 1 on success and 0 on failure. */
int init_uds_skt_addr(
        const char *uds_skt_pathname, struct sockaddr_un *uds_skt_addr);

#endif /* PMC_SYNC_STATUS_COMMON_H_ */
