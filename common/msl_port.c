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

/*
Methods called by MW MSL libraries to perform console IO:
*/

/*lint -w1 Only errors in generic module used during development */

#include "Cpu.h"
#include "Ramlog.h"
#include "../../MSL_Common_Embedded/Include/UART.h"
#include <stddef.h>
#include <stdlib.h>

#if defined(USE_TTY_UDE)
#include "serial_dbg_ude.h"
#endif

#ifdef USE_TTY_WINIDEA

#define TWBUFF_SIZE 0x100
#define TRBUFF_SIZE 0x100
#define TBUFF_PTR 2

#define TWBUFF_LEN		(TWBUFF_SIZE+TBUFF_PTR)
#define TRBUFF_LEN		(TRBUFF_SIZE+TBUFF_PTR)
#define TWBUFF_TPTR		(g_TWBuffer[TWBUFF_SIZE+0])
#define TWBUFF_CPTR		(g_TWBuffer[TWBUFF_SIZE+1])
#define TWBUFF_INC(n)	((n + 1)&(TWBUFF_SIZE-1))
#define TWBUFF_FULL()	(TWBUFF_TPTR==((TWBUFF_CPTR-1)&(TWBUFF_SIZE-1)))

volatile char g_TConn;
volatile unsigned char g_TWBuffer[TWBUFF_LEN] __attribute__ ((aligned (0x100))); // Transmit to WinIDEA terminal
volatile unsigned char g_TRBuffer[TRBUFF_LEN] __attribute__ ((aligned (0x100)));


#endif

UARTError InitializeUART(UARTBaudRate baudRate)
{
    (void)baudRate;
    return 0;
}

UARTError ReadUARTN(void* bytes, unsigned long length)
{
#ifdef USE_TTY_WINIDEA
    (void)g_TRBuffer[0];
#endif
    (void)bytes; //lint !e920 MISRA False positive. Allowed to cast pointer to void here.
    (void)length;
    return 0; // No error
}

UARTError ReadUART1(char* c) {
    return ReadUARTN( c, 1 );
}

UARTError WriteUARTN(const void* buf, unsigned long cnt)
{
#ifdef USE_TTY_WINIDEA
    {
        const char *pbuf = buf;
        if (g_TConn)
        {
            unsigned char nCnt,nLen;
            for(nCnt = 0; nCnt < cnt; nCnt++)
            {
                while( TWBUFF_FULL() ) {}
                nLen = TWBUFF_TPTR;
                g_TWBuffer[nLen] = pbuf[nCnt];
                nLen = TWBUFF_INC(nLen);
                TWBUFF_TPTR = nLen;
            }
        }
    }
#endif
#ifdef USE_TTY_UDE
    UDE_Device.write( buf,cnt);
#endif

#if defined(USE_RAMLOG)
        {
            const char *pbuf = buf;
            for (int i = 0; i < cnt; i++) {
                ramlog_chr (*(pbuf + i));
            }
        }
#endif

    (void)buf; //lint !e920 MISRA False positive. Allowed to cast pointer to void here.
    (void)cnt;
    
    return 0; // No error
}

int write(  int fd, const void *buf, uint32 nbytes) {
    (void)fd;
    (void)buf; //lint !e920 MISRA False positive. Allowed to cast pointer to void here.
    (void)nbytes;
    /* dummy function */
    return 0;
}

UARTError WriteUART1(char c) {
    return WriteUARTN( &c, 1 );
}


#if 0
void __init_hardware(void)
{
}


void __flush_cache(register void *address, register unsigned int size)
{
    (void)address;
    (void)size;

}

void __init_user(void)
{

}
#endif


void exit(int exit ) {
    (void)exit;
    /* !! WARNING !! !! WARNING !! !! WARNING !!
     *
     * ENDING UP HERE CAN BE VERY BAD:
     * 1. You got an assert() (calls exit())
     * 2. For some other reason (deliberately exited main() ?)
     *
     * IMPROVMENT: Call reset of card?
     * */
    Irq_Disable();
    while(1){}
}

