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

#include "pmc_sync_status_common.h"
#ifndef _WIN32
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

#include "linos_logger.h"



/* Returns 1 on success and 0 on failure. */
int init_uds_skt_addr(
        const char *uds_skt_pathname, struct sockaddr_un *uds_skt_addr) {
    if (uds_skt_pathname == NULL || strlen(uds_skt_pathname) == 0 ||
            uds_skt_addr == NULL) {
        logger(LOG_ERR,
                "failed to initialize address for UNIX domain socket, "
                "invalid parameters.");
        return 0;
    }
    memset(uds_skt_addr, 0, sizeof(*uds_skt_addr));
    uds_skt_addr->sun_family = AF_UNIX;
    strcpy(uds_skt_addr->sun_path, uds_skt_pathname);
    return 1;
}
#endif
