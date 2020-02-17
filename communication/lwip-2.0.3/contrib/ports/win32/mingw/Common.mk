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

CCDEP=gcc
CC=gcc

CFLAGS=-g -Wall -DLWIP_DEBUG -pedantic -Werror \
	-Wparentheses -Wsequence-point -Wswitch-default \
	-Wextra -Wundef -Wshadow -Wpointer-arith -Wcast-qual \
	-Wc++-compat -Wwrite-strings -Wold-style-definition -Wcast-align \
	-Wmissing-prototypes -Wredundant-decls -Wnested-externs -Wno-address \
	-Wunreachable-code -Wuninitialized -Wlogical-op -Wno-format
# -Wno-format: GCC complains about non-standard 64 bit modifier needed for MSVC runtime
# not used for now but interesting:
# -Wpacked
# -ansi
# -std=c89
LDFLAGS=-L$(PCAP_DIR)/lib -lwpcap -lpacket
CONTRIBDIR=../../..
LWIPARCH=$(CONTRIBDIR)/ports/win32
ARFLAGS=rs

#Set this to where you have the lwip core module checked out from CVS
#default assumes it's a dir named lwip at the same level as the contrib module
LWIPDIR=$(CONTRIBDIR)/../lwip/src

PCAPDIR=$(PCAP_DIR)/Include

CFLAGS+=-I. \
	-I.. \
	-I$(CONTRIBDIR) \
	-I$(PCAPDIR) \
	-I$(LWIPDIR)/include \
	-I$(LWIPARCH)/include

include $(CONTRIBDIR)/ports/Filelists.mk
include $(LWIPDIR)/Filelists.mk

# ARCHFILES: Architecture specific files.
ARCHFILES=$(LWIPARCH)/sys_arch.c $(LWIPARCH)/test.c $(LWIPARCH)/pcapif.c \
	$(LWIPARCH)/pcapif_helper.c $(LWIPARCH)/sio.c

# LWIPFILES: All the above.
LWIPFILES=$(LWIPNOAPPSFILES) $(ARCHFILES)
LWIPOBJS=$(notdir $(LWIPFILES:.c=.o))

LWIPLIBCOMMON=liblwipcommon.a
$(LWIPLIBCOMMON): $(LWIPOBJS)
	$(AR) $(ARFLAGS) $(LWIPLIBCOMMON) $?

APPFILES=$(CONTRIBAPPFILES) $(LWIPAPPFILES)
APPLIB=liblwipapps.a
APPOBJS=$(notdir $(APPFILES:.c=.o))
$(APPLIB): $(APPOBJS)
	$(AR) $(ARFLAGS) $(APPLIB) $?

%.o:
	$(CC) $(CFLAGS) -c $(<:.o=.c)

pcapif.o:
	$(CC) $(CFLAGS) -Wno-error -Wno-redundant-decls -c $(<:.o=.c)
pcapif_helper.o:
	$(CC) $(CFLAGS) -std=c99 -Wno-redundant-decls -c $(<:.o=.c)
