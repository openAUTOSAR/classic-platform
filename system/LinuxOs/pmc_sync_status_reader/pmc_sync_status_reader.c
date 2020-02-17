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

#include "pmc_sync_status_reader.h"

#include "linos_logger.h"

#ifndef _WIN32
/* Not POSIX, but needed for debugging using pthread_setname_np */
#define __USE_GNU
#include <pthread.h>
#endif

int init_sync_status_reader() {
#ifndef _WIN32
    if (!init_uds_skt_addr((const char *) SYNC_STATUS_UDS,
            &sync_status_uds_skt_addr)) {
        logger(LOG_ERR, "failed to initialize UNIX domain socket address.");
        return 0;
    }
#endif
    return 1;
}

#ifndef _WIN32
pthread_mutex_t mutex_sync_status_reader_conn;
#endif

sync_status_reader_conn_state conn_state = SYNC_STATUS_TEMP_DISCONNECTED;

sync_status_reader_conn_state connect_sync_status_reader() {
    sync_status_reader_conn_state ret_state;
#ifndef _WIN32
    pthread_mutex_lock(&mutex_sync_status_reader_conn);
    if (conn_state != SYNC_STATUS_PERM_DISCONNECTED) {
        reader_uds_skt = socket(AF_UNIX, SOCK_STREAM, 0);
        int conn_rslt = connect(reader_uds_skt,
                (const struct sockaddr *) &sync_status_uds_skt_addr,
                (socklen_t) sizeof(sync_status_uds_skt_addr));
        if (conn_rslt == -1) {
            conn_state = SYNC_STATUS_TEMP_DISCONNECTED;
            close(reader_uds_skt);
            logger(LOG_ERR,
                    "failed to connect to sync_status UNIX domain socket %s"
                            ", error: %d\n", (const char *) SYNC_STATUS_UDS,
                    errno);
        } else {
            conn_state = SYNC_STATUS_CONNECTED;
            logger(LOG_INFO, "sync_status reader connected\n");
        }
    }
    ret_state = conn_state;
    pthread_mutex_unlock(&mutex_sync_status_reader_conn);
#endif
    return ret_state;
}

void disconnect_sync_status_reader(int close_permanently) {
#ifndef _WIN32
    pthread_mutex_lock(&mutex_sync_status_reader_conn);
    /* Design note:
     * ------------
     * To ensure the correctness of the system, disconnect_sync_status_reader
     * may only set conn_state to SYNC_STATUS_PERM_DISCONNECTED and never
     * allows setting it to SYNC_STATUS_TEMP_DISCONNECTED.
     */
    if (close_permanently)
        conn_state = SYNC_STATUS_PERM_DISCONNECTED;
    /* The socket is closed for both temporary and permanent closure. */
    close(reader_uds_skt);
    pthread_mutex_unlock(&mutex_sync_status_reader_conn);
#endif
}
