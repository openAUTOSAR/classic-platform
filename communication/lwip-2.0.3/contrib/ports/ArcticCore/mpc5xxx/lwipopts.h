#define LWIP_AR_MAJOR_VERSION   1
#define LWIP_AR_MINOR_VERSION   0
#define LWIP_AR_PATCH_VERSION   0

#define LWIP_SW_MAJOR_VERSION   1
#define LWIP_SW_MINOR_VERSION   0
#define LWIP_SW_PATCH_VERSION   0

/* New important lwip1.4.1 defines */
#define LWIP_TCPIP_CORE_LOCKING 1
#define LWIP_CHECKSUM_ON_COPY  1
#define LWIP_COMPAT_MUTEX_ALLOWED 1

/* We want to be notified when the link changes status. */
#define LWIP_NETIF_STATUS_CALLBACK 1

#define TCPIP_MBOX_SIZE        50
#define MEMP_NUM_NETBUF        10
#define MEMP_NUM_TCPIP_MSG_INPKT 20

/* Support IPV6(mandatory for doip) */
#define LWIP_IPV6 1

#include "LwIp_Cfg.h"
