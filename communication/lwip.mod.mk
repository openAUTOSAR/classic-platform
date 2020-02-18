#LWIP
LWIP_VERSION?=2.0.3

ifeq ($(LWIP_VERSION),1.5.0-beta)

inc-$(USE_LWIP) += $(ROOTDIR)/communication/lwip-$(LWIP_VERSION)/contrib/port/ArcticCore
inc-$(USE_LWIP) += $(ROOTDIR)/communication/lwip-$(LWIP_VERSION)/src/include
inc-$(USE_LWIP) += $(ROOTDIR)/communication/lwip-$(LWIP_VERSION)/src/include/ipv4
inc-$(USE_LWIP) += $(ROOTDIR)/communication/lwip-$(LWIP_VERSION)/src/include/ipv6
inc-$(USE_LWIP) += $(ROOTDIR)/communication/lwip-$(LWIP_VERSION)/contrib/port/ArcticCore/LwipAdp


ifeq ($(CFG_STM32F1X),y)
inc-$(USE_LWIP) += $(ROOTDIR)/communication/lwip-$(LWIP_VERSION)/contrib/port/ArcticCore/arm_cm3
vpath-$(USE_LWIP) += $(ROOTDIR)/communication/lwip-$(LWIP_VERSION)/contrib/port/ArcticCore/arm_cm3
obj-$(USE_LWIP) += ethernetif.o
else ifeq ($(CFG_MPC5748G)$(CFG_MPC5747C)$(CFG_MPC5746C)$(CFG_MPC5777C),y)
inc-$(USE_LWIP) += $(ROOTDIR)/communication/lwip-$(LWIP_VERSION)/contrib/port/ArcticCore/asrIf
vpath-$(USE_LWIP) += $(ROOTDIR)/communication/lwip-$(LWIP_VERSION)/contrib/port/ArcticCore/asrIf
else ifeq ($(CFG_MPC55XX),y) 
inc-$(USE_LWIP) += $(ROOTDIR)/communication/lwip-$(LWIP_VERSION)/contrib/port/ArcticCore/mpc5xxx
vpath-$(USE_LWIP) += $(ROOTDIR)/communication/lwip-$(LWIP_VERSION)/contrib/port/ArcticCore/mpc5xxx
obj-$(USE_LWIP) += fec_5xxx.o
obj-$(USE_LWIP) += ethernetif.o
else ifeq ($(CFG_TC29X),y)
inc-$(USE_LWIP) += $(ROOTDIR)/communication/lwip-$(LWIP_VERSION)/contrib/port/ArcticCore/tc29x
vpath-$(USE_LWIP) += $(ROOTDIR)/communication/lwip-$(LWIP_VERSION)/contrib/port/ArcticCore/tc29x
else ifeq ($(CFG_BRD_LINUX),y) 
inc-$(USE_LWIP) += $(ROOTDIR)/communication/lwip-$(LWIP_VERSION)/contrib/port/ArcticCore/utest
vpath-$(USE_LWIP) += $(ROOTDIR)/communication/lwip-$(LWIP_VERSION)/contrib/port/ArcticCore/utest
else
inc-$(USE_LWIP) += $(ROOTDIR)/communication/lwip-$(LWIP_VERSION)/contrib/port/ArcticCore/asrIf
vpath-$(USE_LWIP) += $(ROOTDIR)/communication/lwip-$(LWIP_VERSION)/contrib/port/ArcticCore/asrIf
#obj-$(USE_LWIP) += memhdl.o
endif
obj-$(USE_LWIP) += sys_arch.o
vpath-$(USE_LWIP) += $(ROOTDIR)/communication/lwip-$(LWIP_VERSION)/contrib/port/ArcticCore
vpath-$(USE_LWIP) += $(ROOTDIR)/communication/lwip-$(LWIP_VERSION)/contrib/port/ArcticCore/LwipAdp
#obj-$(USE_LWIP) += httpd.o
obj-$(USE_LWIP) += mbox.o
obj-$(USE_LWIP) += LwIpAdp.o

vpath-$(USE_LWIP) += $(ROOTDIR)/communication/lwip-$(LWIP_VERSION)/src/core/ipv4
vpath-$(USE_LWIP) += $(ROOTDIR)/communication/lwip-$(LWIP_VERSION)/src/core/ipv6

#ipv6
obj-$(USE_LWIP) += dhcp6.o
obj-$(USE_LWIP) += icmp6.o
obj-$(USE_LWIP) += ethip6.o
obj-$(USE_LWIP) += inet6.o
obj-$(USE_LWIP) += ip6_addr.o
obj-$(USE_LWIP) += ip6_frag.o
obj-$(USE_LWIP) += ip6.o
obj-$(USE_LWIP) += mld6.o
obj-$(USE_LWIP) += nd6.o
#ipv4
obj-$(USE_LWIP) += autoip.o
obj-$(USE_LWIP) += icmp.o
obj-$(USE_LWIP) += igmp.o
obj-$(USE_LWIP) += ip_frag.o
obj-$(USE_LWIP) += ip4_addr.o
obj-$(USE_LWIP) += ip4.o

vpath-$(USE_LWIP) += $(ROOTDIR)/communication/lwip-$(LWIP_VERSION)/src/core
obj-$(USE_LWIP) += inet_chksum.o
obj-$(USE_LWIP) += init_lwip.o
obj-$(USE_LWIP) += dns.o
obj-$(USE_LWIP) += def.o
obj-$(USE_LWIP) += dhcp.o
obj-$(USE_LWIP) += mem.o
obj-$(USE_LWIP) += memp.o
obj-$(USE_LWIP) += netif.o
obj-$(USE_LWIP) += pbuf.o
obj-$(USE_LWIP) += raw.o
obj-$(USE_LWIP) += stats.o
obj-$(USE_LWIP) += sys.o
obj-$(USE_LWIP) += tcp.o
obj-$(USE_LWIP) += tcp_in.o
obj-$(USE_LWIP) += tcp_out.o
obj-$(USE_LWIP) += udp.o
obj-$(USE_LWIP) += timers.o
obj-$(USE_LWIP) += netbios.o

vpath-$(USE_LWIP) += $(ROOTDIR)/communication/lwip-$(LWIP_VERSION)/src/api
obj-$(USE_LWIP) += api_msg.o
obj-$(USE_LWIP) += api_lib.o
obj-$(USE_LWIP) += err.o
obj-$(USE_LWIP) += netbuf.o
obj-$(USE_LWIP) += netdb.o
obj-$(USE_LWIP) += netifapi.o
obj-$(USE_LWIP) += sockets.o
obj-$(USE_LWIP) += tcpip_lwip.o
vpath-$(USE_LWIP) += $(ROOTDIR)/communication/lwip-$(LWIP_VERSION)/src/netif
obj-$(USE_LWIP) += etharp.o

else

inc-$(USE_LWIP) += $(ROOTDIR)/communication/lwip-$(LWIP_VERSION)/contrib/ports/ArcticCore
inc-$(USE_LWIP) += $(ROOTDIR)/communication/lwip-$(LWIP_VERSION)/src/include
inc-$(USE_LWIP) += $(ROOTDIR)/communication/lwip-$(LWIP_VERSION)/src/include/ipv4
inc-$(USE_LWIP) += $(ROOTDIR)/communication/lwip-$(LWIP_VERSION)/src/include/ipv6
inc-$(USE_LWIP) += $(ROOTDIR)/communication/lwip-$(LWIP_VERSION)/contrib/ports/ArcticCore/LwipAdp


ifeq ($(CFG_STM32F1X),y)
inc-$(USE_LWIP) += $(ROOTDIR)/communication/lwip-$(LWIP_VERSION)/contrib/ports/ArcticCore/arm_cm3
vpath-$(USE_LWIP) += $(ROOTDIR)/communication/lwip-$(LWIP_VERSION)/contrib/ports/ArcticCore/arm_cm3
obj-$(USE_LWIP) += ethernetif.o
else ifeq ($(CFG_MPC5748G)$(CFG_MPC5747C)$(CFG_MPC5746C)$(CFG_MPC5777C),y)
inc-$(USE_LWIP) += $(ROOTDIR)/communication/lwip-$(LWIP_VERSION)/contrib/ports/ArcticCore/asrIf
vpath-$(USE_LWIP) += $(ROOTDIR)/communication/lwip-$(LWIP_VERSION)/contrib/ports/ArcticCore/asrIf
else ifeq ($(CFG_MPC55XX),y) 
inc-$(USE_LWIP) += $(ROOTDIR)/communication/lwip-$(LWIP_VERSION)/contrib/ports/ArcticCore/mpc5xxx
vpath-$(USE_LWIP) += $(ROOTDIR)/communication/lwip-$(LWIP_VERSION)/contrib/ports/ArcticCore/mpc5xxx
obj-$(USE_LWIP) += fec_5xxx.o
obj-$(USE_LWIP) += ethernetif.o
else ifeq ($(CFG_TC29X),y)
inc-$(USE_LWIP) += $(ROOTDIR)/communication/lwip-$(LWIP_VERSION)/contrib/ports/ArcticCore/tc29x
vpath-$(USE_LWIP) += $(ROOTDIR)/communication/lwip-$(LWIP_VERSION)/contrib/ports/ArcticCore/tc29x
else ifeq ($(CFG_BRD_LINUX),y) 
inc-$(USE_LWIP) += $(ROOTDIR)/communication/lwip-$(LWIP_VERSION)/contrib/ports/ArcticCore/utest
vpath-$(USE_LWIP) += $(ROOTDIR)/communication/lwip-$(LWIP_VERSION)/contrib/ports/ArcticCore/utest
else
inc-$(USE_LWIP) += $(ROOTDIR)/communication/lwip-$(LWIP_VERSION)/contrib/ports/ArcticCore/asrIf
vpath-$(USE_LWIP) += $(ROOTDIR)/communication/lwip-$(LWIP_VERSION)/contrib/ports/ArcticCore/asrIf
#obj-$(USE_LWIP) += memhdl.o
endif
obj-$(USE_LWIP) += sys_arch.o
vpath-$(USE_LWIP) += $(ROOTDIR)/communication/lwip-$(LWIP_VERSION)/contrib/ports/ArcticCore
vpath-$(USE_LWIP) += $(ROOTDIR)/communication/lwip-$(LWIP_VERSION)/contrib/ports/ArcticCore/LwipAdp
#obj-$(USE_LWIP) += httpd.o
obj-$(USE_LWIP) += mbox.o
obj-$(USE_LWIP) += LwIpAdp.o

vpath-$(USE_LWIP) += $(ROOTDIR)/communication/lwip-$(LWIP_VERSION)/src/core/ipv4
vpath-$(USE_LWIP) += $(ROOTDIR)/communication/lwip-$(LWIP_VERSION)/src/core/ipv6

#ipv6
obj-$(USE_LWIP) += dhcp6.o
obj-$(USE_LWIP) += icmp6.o
obj-$(USE_LWIP) += ethip6.o
obj-$(USE_LWIP) += inet6.o
obj-$(USE_LWIP) += ip6_addr.o
obj-$(USE_LWIP) += ip6_frag.o
obj-$(USE_LWIP) += ip6.o
obj-$(USE_LWIP) += mld6.o
obj-$(USE_LWIP) += nd6.o
#ipv4
obj-$(USE_LWIP) += autoip.o
obj-$(USE_LWIP) += icmp.o
obj-$(USE_LWIP) += igmp.o
obj-$(USE_LWIP) += ip4_frag.o
obj-$(USE_LWIP) += ip4_addr.o
obj-$(USE_LWIP) += ip4.o

vpath-$(USE_LWIP) += $(ROOTDIR)/communication/lwip-$(LWIP_VERSION)/src/core
obj-$(USE_LWIP) += ip.o
obj-$(USE_LWIP) += inet_chksum.o
obj-$(USE_LWIP) += init_lwip.o
obj-$(USE_LWIP) += dns.o
obj-$(USE_LWIP) += def.o
obj-$(USE_LWIP) += dhcp.o
obj-$(USE_LWIP) += mem.o
obj-$(USE_LWIP) += memp.o
obj-$(USE_LWIP) += netif.o
obj-$(USE_LWIP) += pbuf.o
obj-$(USE_LWIP) += raw.o
obj-$(USE_LWIP) += stats.o
obj-$(USE_LWIP) += sys.o
obj-$(USE_LWIP) += tcp.o
obj-$(USE_LWIP) += tcp_in.o
obj-$(USE_LWIP) += tcp_out.o
obj-$(USE_LWIP) += udp.o
obj-$(USE_LWIP) += timeouts.o
obj-$(USE_LWIP) += netbios.o

vpath-$(USE_LWIP) += $(ROOTDIR)/communication/lwip-$(LWIP_VERSION)/src/api
obj-$(USE_LWIP) += api_msg.o
obj-$(USE_LWIP) += api_lib.o
obj-$(USE_LWIP) += err.o
obj-$(USE_LWIP) += netbuf.o
obj-$(USE_LWIP) += netdb.o
obj-$(USE_LWIP) += netifapi.o
obj-$(USE_LWIP) += sockets.o
obj-$(USE_LWIP) += tcpip_lwip.o
vpath-$(USE_LWIP) += $(ROOTDIR)/communication/lwip-$(LWIP_VERSION)/src/netif
obj-$(USE_LWIP) += ethernet.o
obj-$(USE_LWIP) += etharp.o

endif