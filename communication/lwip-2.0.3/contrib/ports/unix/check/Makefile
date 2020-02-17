#
# Copyright (c) 2001, 2002 Swedish Institute of Computer Science.
# All rights reserved. 
# 
# Redistribution and use in source and binary forms, with or without modification, 
# are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice,
#    this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
# 3. The name of the author may not be used to endorse or promote products
#    derived from this software without specific prior written permission. 
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED 
# WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
# MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
# SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
# OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
# IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
# OF SUCH DAMAGE.
#
# This file is part of the lwIP TCP/IP stack.
# 
# Author: Adam Dunkels <adam@sics.se>
#
 
all compile: lwip_unittests
.PHONY: all clean check

include ../Common.mk

LDFLAGS:=$(LDFLAGS) -lcheck -lm -lsubunit
CFLAGS:=$(CFLAGS) -DLWIP_NOASSERT_ON_ERROR -I/usr/include/check -I$(LWIPDIR)/../test/unit

CODEDIR=../../../../lwip
TESTDIR=$(CODEDIR)/test/unit
TESTFILES=$(TESTDIR)/lwip_unittests.c $(TESTDIR)/udp/test_udp.c $(TESTDIR)/etharp/test_etharp.c $(TESTDIR)/tcp/tcp_helper.c \
	$(TESTDIR)/tcp/test_tcp_oos.c $(TESTDIR)/tcp/test_tcp.c $(TESTDIR)/core/test_mem.c $(TESTDIR)/dhcp/test_dhcp.c \
	$(TESTDIR)/core/test_pbuf.c $(TESTDIR)/mdns/test_mdns.c 
TESTOBJS=$(notdir $(TESTFILES:.c=.o))

clean:
	@rm -f *.o $(LWIPLIBCOMMON) $(APPLIB) lwip_unittests *.s .depend* *.core core

depend dep: .depend

include .depend

.depend: $(LWIPFILES) $(APPFILES) $(TESTFILES)
	$(CCDEP) $(CFLAGS) -MM $^ > .depend || rm -f .depend

lwip_unittests: .depend $(TESTOBJS) $(LWIPLIBCOMMON) $(APPLIB)
	$(CC) $(CFLAGS) -o lwip_unittests $(TESTOBJS)  -Wl,--start-group $(LWIPLIBCOMMON) $(APPLIB) $(LDFLAGS) -Wl,--end-group

check: lwip_unittests
	@./lwip_unittests
