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

/*lint -w1 Only errors in module used during development */

/* ----------------------------[information]----------------------------------*/
/*
 *
 * Description:
 *   Implements terminal for isystems winidea debugger
 *   Assumes JTAG access port is in non-cached area.
 */

/* ----------------------------[includes]------------------------------------*/
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "Std_Types.h"
#include "MemMap.h"
#include "device_serial.h"
#include "sys/queue.h"

/* ----------------------------[private define]------------------------------*/

#define TWBUFF_SIZE 0x100
#define TRBUFF_SIZE 0x100


#define TBUFF_PTR 2

#define TWBUFF_LEN (TWBUFF_SIZE+TBUFF_PTR)
#define TRBUFF_LEN (TRBUFF_SIZE+TBUFF_PTR)
#define TWBUFF_TPTR (g_TWBuffer[TWBUFF_SIZE+0])
#define TWBUFF_CPTR (g_TWBuffer[TWBUFF_SIZE+1])
#define TWBUFF_INC(n) ((n + 1)&(TWBUFF_SIZE-1))
#define TWBUFF_FULL() (TWBUFF_TPTR==((TWBUFF_CPTR-1)&(TWBUFF_SIZE-1)))

/* ----------------------------[private macro]-------------------------------*/

/* ----------------------------[private typedef]-----------------------------*/

/* ----------------------------[private function prototypes]-----------------*/
static int WinIdea_Write(  uint8_t *data, size_t nbytes);
static int WinIdea_Read( uint8_t *data, size_t nbytes );
static int WinIdea_Open( const char *path, int oflag, int mode );

/* ----------------------------[private variables]---------------------------*/
#if defined(MC912DG128A)
static volatile unsigned char g_TWBuffer[TWBUFF_LEN];
static volatile unsigned char g_TRBuffer[TRBUFF_LEN];
SECTION_RAM_NO_CACHE_DATA volatile char g_TConn;

#else
static volatile unsigned char g_TWBuffer[TWBUFF_LEN] __balign(0x100);
static volatile unsigned char g_TRBuffer[TRBUFF_LEN] __balign(0x100);
volatile char g_TConn;

#endif


/* ----------------------------[private functions]---------------------------*/
/* ----------------------------[public functions]----------------------------*/

/**
 *
 * @param buffer
 * @param count
 * @return
 */
static int WinIdea_Write( uint8_t *buffer, size_t nbytes) {
    if (g_TConn)
    {
      char *buf = (char *)buffer;
      unsigned char nCnt,nLen;
      for(nCnt=0; nCnt<nbytes; nCnt++)
        {
        while(TWBUFF_FULL()) ;
        nLen=TWBUFF_TPTR;
        g_TWBuffer[nLen]=buf[nCnt];
        nLen=TWBUFF_INC(nLen);
        TWBUFF_TPTR=nLen;
        }
    }
    return nbytes;
}



/**
 * Read characters from terminal
 *
 * @param buffer	  Where to save the data to.
 * @param nbytes  The maximum bytes to read
 * @return The number of bytes read.
 */
static int WinIdea_Read( uint8_t *buffer, size_t nbytes )
{
    (void)buffer; //lint !e920 MISRA False positive. Allowed to cast pointer to void here.
    (void)nbytes; //lint !e920 MISRA False positive. Allowed to cast pointer to void here.

    (void)g_TRBuffer[0];

    return 0;
}

static int WinIdea_Open( const char *path, int oflag, int mode ) {
    (void)path; //lint !e920 MISRA False positive. Allowed to cast pointer to void here.
    (void)oflag;
    (void)mode;

    return 0;
}

DeviceSerialType WinIdea_Device = {
    .device.type = DEVICE_TYPE_CONSOLE,
    .name = "serial_winidea",
//	.init = T32_Init,
    .read = WinIdea_Read,
    .write = WinIdea_Write,
    .open = WinIdea_Open,
};

