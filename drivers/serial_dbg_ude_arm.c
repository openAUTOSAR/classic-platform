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

/* ----------------------------[information]----------------------------------*/
/*
 * COPYRIGHT   :  pls Programmierbare Logik & Systeme GmbH  1999,2011
 *
 * Author: mahi and parts taken from pls
 *
 * Description:
 *   Implements terminal for PLS/UDE debugger.
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

#define ARM_SIMIO_ENCODE_CHANNEL(channel) \
  ((channel<<24) & 0x0F000000)

#define ARM_SIMIO_DECODE_CHANNEL(data) \
    ((data>>24) & 0x0F)

#define ARM_SIMIO_ENCODE_DATA(data) \
  (data & 0x0FF)

#define ARM_SIMIO_DECODE_DATA(data) \
  (data & 0x0FF)

#define ARM_SIMIO_FLAG_DATA1 0x10000000
#define ARM_SIMIO_CHECK_DATA1(data) \
  (data & ARM_SIMIO_FLAG_DATA1)

#define ARM_SIMIO_ENCODE_DATA1(data) \
  (((data<<8) & 0x0FF00) | ARM_SIMIO_FLAG_DATA1)

#define ARM_SIMIO_DECODE_DATA1(data) \
  ((data>>8) & 0x0FF)

#define ARM_SIMIO_FLAG_DATA2 0x20000000
#define ARM_SIMIO_CHECK_DATA2(data) \
  (data & ARM_SIMIO_FLAG_DATA2)

#define ARM_SIMIO_ENCODE_DATA2(data) \
  (((data<<16) & 0x0FF0000) | ARM_SIMIO_FLAG_DATA2)

#define ARM_SIMIO_DECODE_DATA2(data) \
  ((data>>16) & 0x0FF)


#define DEFAULT_SIMIO_CHANNEL   0

/* ----------------------------[private macro]-------------------------------*/

#ifndef MIN
#define MIN(_x,_y) (((_x) < (_y)) ? (_x) : (_y))
#endif

/* ----------------------------[private typedef]-----------------------------*/

typedef struct tagSimioAccess
{
    volatile uint32_t dwCtrl;
    volatile uint32_t adwData[16];
} TJtagSimioAccess;

/* ----------------------------[private function prototypes]-----------------*/
static int UDE_Write(  uint8_t *data, size_t nbytes);
static int UDE_Read( uint8_t *data, size_t nbytes );
static int UDE_Open( const char *path, int oflag, int mode );

/* ----------------------------[private variables]---------------------------*/
DeviceSerialType UDE_Device = {
    .device.type = DEVICE_TYPE_CONSOLE,
    .name = "serial_ude",
//	.init = T32_Init,
    .read = UDE_Read,
    .write = UDE_Write,
    .open = UDE_Open,
};

/* ----------------------------[private functions]---------------------------*/
/* ----------------------------[public functions]----------------------------*/

void SendChar(int ch)
{
  asm volatile ("                 \n\
      1:             \n\
        MRC   %%p14,0,%%R1,%%c0,%%c1   \n\
        TST      %%R1,#0x20000000               \n\
        BNE     1b         \n\
        MCR   %%p14,0,%0,%%c0,%%c5"  : : "r" (ch) : "cc", "1" );
}

void SIMIO_PutByteToHost (unsigned int Channel, unsigned char Data)
{
  int ch=Data;
  ch|=ARM_SIMIO_ENCODE_CHANNEL(Channel);
  SendChar(ch);   /*lint !e522 MISRA:FALSE_POSITIVE:Assembler function:[MISRA 2012 Rule 2.2, required] */
}


int UDE_Write (  uint8_t *data, size_t nbytes)
{
    for ( uint32_t index = 0; index < nbytes; index++, data++) {
        SIMIO_PutByteToHost (DEFAULT_SIMIO_CHANNEL,(unsigned char)*data); /*lint !e522 MISRA:FALSE_POSITIVE:Assembler function:[MISRA 2012 Rule 2.2, required] */
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
static int UDE_Read( uint8_t *buffer, size_t nbytes )
{
    return nbytes;
}

static int UDE_Open( const char *path, int oflag, int mode ) {
    (void)path;
    (void)oflag;
    (void)mode;

    return 0;
}

