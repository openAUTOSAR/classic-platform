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


#error "Status of this TTY: We have not been able to get this implementation to work in Code Composer 6.0"

/* ----------------------------[private define]------------------------------*/

#define _DTOPEN    (0xF0)
#define _DTCLOSE   (0xF1)
#define _DTREAD    (0xF2)
#define _DTWRITE   (0xF3)
#define _DTLSEEK   (0xF4)
#define _DTUNLINK  (0xF5)
#define _DTGETENV  (0xF6)
#define _DTRENAME  (0xF7)
#define _DTGETTIME (0xF8)
#define _DTGETCLK  (0xF9)
#define _DTSYNC    (0xFF)

#define LOADSHORT(x,y,z)  { x[(z)]   = (unsigned short) (y); \
                            x[(z)+1] = (unsigned short) (y) >> 8;  }

#define UNLOADSHORT(x,z) ((short) ( (short) x[(z)] +             \
                   ((short) x[(z)+1] << 8)))

#define PACKCHAR(val, base, byte) ( (base)[(byte)] = (val) )

#define UNPACKCHAR(base, byte)    ( (base)[byte] )

#define CC_BUFSIZ 512
#define CC_BUFFER_SIZE ((CC_BUFSIZ)+32)

/* ----------------------------[private macro]-------------------------------*/

/* ----------------------------[private typedef]-----------------------------*/

/* ----------------------------[private function prototypes]-----------------*/


/* ----------------------------[private variables]---------------------------*/
static unsigned char parmbuf[8];
volatile unsigned int _CIOBUF_[CC_BUFFER_SIZE] __attribute__ ((section (".cio")));
static unsigned char CIOTMPBUF[CC_BUFSIZ];
static uint16 cio_tmp_buf_index = 0;

/* ----------------------------[private functions]---------------------------*/
/***************************************************************************/
/*                                                                         */
/*  WRITEMSG()  -  Sends the passed data and parameters on to the host.    */
/*                                                                         */
/***************************************************************************/
void writemsg(unsigned char  command,
              register const unsigned char *parm,
              register const          char *data,
              unsigned int            length)
{
   volatile unsigned char *p = (volatile unsigned char *)(_CIOBUF_+1);
   unsigned int i;

   /***********************************************************************/
   /* THE LENGTH IS WRITTEN AS A TARGET INT                               */
   /***********************************************************************/
   _CIOBUF_[0] = length;

   /***********************************************************************/
   /* THE COMMAND IS WRITTEN AS A TARGET BYTE                             */
   /***********************************************************************/
   *p++ = command;

   /***********************************************************************/
   /* PACK THE PARAMETERS AND DATA SO THE HOST READS IT AS BYTE STREAM    */
   /***********************************************************************/
   for (i = 0; i < 8; i++)      PACKCHAR(*parm++, p, i);
   for (i = 0; i < length; i++) PACKCHAR(*data++, p, i+8);

   /***********************************************************************/
   /* THE BREAKPOINT THAT SIGNALS THE HOST TO DO DATA TRANSFER            */
   /***********************************************************************/
   __asm("	 .global C$$IO$$");
   __asm("C$$IO$$: nop");
}

/***************************************************************************/
/*                                                                         */
/*  READMSG()   -  Reads the data and parameters passed from the host.     */
/*                                                                         */
/***************************************************************************/
void readmsg(register unsigned char *parm,
         register char          *data)
{
   volatile unsigned char *p = (volatile unsigned char *)(_CIOBUF_+1);
   unsigned int   i;
   unsigned int   length;

   /***********************************************************************/
   /* THE LENGTH IS READ AS A TARGET INT                                  */
   /***********************************************************************/
   length = _CIOBUF_[0];

   /***********************************************************************/
   /* UNPACK THE PARAMETERS AND DATA                                      */
   /***********************************************************************/
   for (i = 0; i < 8; i++) *parm++ = UNPACKCHAR(p, i);
   if (data != NULL)
      for (i = 0; i < length; i++) *data++ = UNPACKCHAR(p, i+8);
}

/****************************************************************************/
/* HOSTWRITE()  -  Pass the write command and its arguments to the host.    */
/****************************************************************************/
int HOSTwrite(int dev_fd, const char *buf, unsigned count)
{
   int result;

   // WARNING. Can only handle count == 1!
   if (count != 1) _exit(1);

   if (count > CC_BUFSIZ) count = CC_BUFSIZ;

   if (cio_tmp_buf_index < CC_BUFSIZ) {
       CIOTMPBUF[cio_tmp_buf_index++] = *buf;

       if (*buf != 0xA) { // Only flush if newline
           return 0;
       }
   }


   LOADSHORT(parmbuf,dev_fd,0);
   LOADSHORT(parmbuf,cio_tmp_buf_index,2);
   writemsg(_DTWRITE,parmbuf,(char *)CIOTMPBUF,cio_tmp_buf_index);
   readmsg(parmbuf,NULL);

   result = UNLOADSHORT(parmbuf,0);

   cio_tmp_buf_index = 0;

   return result;
}

/* ----------------------------[public functions]----------------------------*/
static int CodeComposer_Read( uint8_t *buffer, size_t nbytes )
{
    return 0;
}

static int CodeComposer_Open( const char *path, int oflag, int mode ) {
    return 0;
}
static int CodeComposer_Write( uint8_t *buffer, size_t nbytes) {

    HOSTwrite(0, buffer, nbytes);

    return (nbytes);
}



DeviceSerialType CodeComposer_Device = {
    .name = "serial_code_composer",
//	.init = T32_Init,
    .read = CodeComposer_Read,
    .write = CodeComposer_Write,
    .open = CodeComposer_Open,
};

