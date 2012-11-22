/*
 * iar_port.c
 *
 *  Created on: 16 nov 2011
 *      Author: jcar
 */

/*
Methods called by IAR libraries to perform console IO:
*/

#include "Os.h"
#include "Ramlog.h"
#include "stddef.h"

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
SECTION_BALIGN(0x100) volatile unsigned char g_TWBuffer[TWBUFF_LEN]; // Transmit to WinIDEA terminal
SECTION_BALIGN(0x100) volatile unsigned char g_TRBuffer[TRBUFF_LEN];

#endif

#ifdef USE_TTY_NOICE
volatile unsigned char START_VUART = 0;

_Pragma("location=0x1000")
__no_init static volatile char VUART_TX;
__no_init static volatile char VUART_RX;

#endif


size_t __write(int handle, const unsigned char *buf, size_t cnt)
{
#ifdef USE_TTY_NOICE
	char *buf1 = (char *)buf;
	if (START_VUART)
	{
   	   for (int i = 0; i < cnt; i++) {
   		   char c = buf1[i];
   		   if (c == '\n')
   		   {
   	   		   while (VUART_TX != 0)
   	   		   {
   	   		   }

   	   		   VUART_TX = '\r';
   		   }

   		   while (VUART_TX != 0)
   		   {
   		   }

   		   VUART_TX = c;
   	   }
	}
#endif
#ifdef USE_TTY_WINIDEA
	(void)g_TRBuffer[0];
	{
		unsigned char nCnt,nLen;
		for(nCnt = 0; nCnt < cnt; nCnt++)
		{
			while( TWBUFF_FULL() ) {}
			nLen = TWBUFF_TPTR;
			g_TWBuffer[nLen] = buf[nCnt];
			nLen = TWBUFF_INC(nLen);
			TWBUFF_TPTR = nLen;
		}
	}
#endif
#if defined(USE_RAMLOG)
		{
			const unsigned char *pbuf = buf;
			for (int i = 0; i < cnt; i++) {
				ramlog_chr (*(pbuf + i));
			}
		}
#endif

	(void)buf;
	(void)cnt;
	
	return 0; // No error
}

int arc_putchar(int fd, int c) {
	char cc = c;
	__write( fd,(const unsigned char *)&cc,1);

	return 0;
}

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
	DisableAllInterrupts();
	while(1){}
}

