/*
 * Copyright (c) 2001-2003 Swedish Institute of Computer Science.
 * All rights reserved. 
 * 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 * 
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

#include "lwip/debug.h"

#include <unistd.h>


#include "lwip/mem.h"
#include "lwip/memp.h"
#include "lwip/sys.h"
#include "lwip/tcp.h"
#include "lwip/priv/tcp_priv.h" /* for tcp_debug_print_pcbs() */
#include "lwip/timeouts.h"

#include "lwip/stats.h"


#include "lwip/tcpip.h"


#include "netif/unixif.h"
#include "netif/dropif.h"

#include "netif/tcpdump.h"

#include "lwip/ip_addr.h"

#include "arch/perf.h"

#include "lwip/apps/httpd.h"
#include "apps/udpecho/udpecho.h"
#include "apps/tcpecho/tcpecho.h"
#include "apps/shell/shell.h"

#if LWIP_IPV4 /* @todo: IPv6 */

/* nonstatic debug cmd option, exported in lwipopts.h */
unsigned char debug_flags;

/*-----------------------------------------------------------------------------------*/
static void
tcp_timeout(void *data)
{
  LWIP_UNUSED_ARG(data);
#if TCP_DEBUG && LWIP_TCP
  tcp_debug_print_pcbs();
#endif /* TCP_DEBUG */
  sys_timeout(5000, tcp_timeout, NULL);
}
/*-----------------------------------------------------------------------------------*/
struct netif netif_unix;
/*-----------------------------------------------------------------------------------*/
static void
tcpip_init_done(void *arg)
{
  ip4_addr_t ipaddr, netmask, gw;
  sys_sem_t *sem;
  sem = (sys_sem_t *)arg;

  IP4_ADDR(&gw, 192,168,1,1);
  IP4_ADDR(&ipaddr, 192,168,1,2);
  IP4_ADDR(&netmask, 255,255,255,0);

  netif_set_default(netif_add(&netif_unix, &ipaddr, &netmask, &gw, NULL, unixif_init_client,
			      tcpip_input));
  netif_set_up(&netif_unix);
#if LWIP_IPV6
  netif_create_ip6_linklocal_address(&netif_unix, 1);
#endif
  /*  netif_set_default(netif_add(&ipaddr, &netmask, &gw, NULL, sioslipif_init1,
			      tcpip_input)); */

#if LWIP_NETCONN
  tcpecho_init();
  shell_init();
#if LWIP_IPV4 && LWIP_TCP
  httpd_init();
#endif
  udpecho_init();
#endif

  printf("Applications started.\n");

  sys_timeout(5000, tcp_timeout, NULL);

  sys_sem_signal(sem);
}
/*-----------------------------------------------------------------------------------*/

static void
main_thread(void *arg)
{
  sys_sem_t sem;
  LWIP_UNUSED_ARG(arg);

  if(sys_sem_new(&sem, 0) != ERR_OK) {
    LWIP_ASSERT("Failed to create semaphore", 0);
  }
  tcpip_init(tcpip_init_done, &sem);
  sys_sem_wait(&sem);
  printf("TCP/IP initialized.\n");

#ifdef MEM_PERF
  mem_perf_init("/tmp/memstats.client");
#endif /* MEM_PERF */

  /* Block forever. */
  sys_sem_wait(&sem);
}
/*-----------------------------------------------------------------------------------*/
int
main(void)
{
#ifdef PERF
  perf_init("/tmp/client.perf");
#endif /* PERF */

#if LWIP_IPV4 && LWIP_TCP
  tcpdump_init();
#endif
  
  printf("System initialized.\n");

  sys_thread_new("main_thread", main_thread, NULL, DEFAULT_THREAD_STACKSIZE, DEFAULT_THREAD_PRIO);
  pause();
  return 0;
}

#else /* LWIP_IPV4 */

int
main(int argc, char **argv)
{
  LWIP_UNUSED_ARG(argc);
  LWIP_UNUSED_ARG(argv);

  printf("simnode only works with IPv4\n");

  return 0;
}

#endif /* LWIP_IPV4 */

/* dummy, because SNTP is pulled in via LWIP_DHCP_GET_NTP_SRV */
void
sntp_set_system_time(u32_t sec)
{
  LWIP_UNUSED_ARG(sec);
}

/*-----------------------------------------------------------------------------------*/
