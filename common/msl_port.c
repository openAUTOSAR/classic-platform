/*
 * msl_port.c
 *
 *  Created on: 14 feb 2011
 *      Author: mahi
 */

/*
Methods called by MW MSL libraries to perform console IO:
*/

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
	(void)bytes;
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
		char *pbuf = buf;
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
	UDE_write( 0 ,(char *)buf,cnt);
#endif

#if defined(USE_RAMLOG)
		{
			char *pbuf = buf;
			for (int i = 0; i < cnt; i++) {
				ramlog_chr (*(pbuf + i));
			}
		}
#endif

	(void)buf;
	(void)cnt;
	
	return 0; // No error
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
	 * TODO: Call reset of card?
	 * */
	Irq_Disable();
	while(1){}
}

