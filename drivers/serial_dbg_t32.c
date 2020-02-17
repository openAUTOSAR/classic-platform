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
 * DESCRIPTION
 *   Lauterbach T32 (debugger) terminal
 */

/* ----------------------------[includes]------------------------------------*/
#include <stdint.h>
#include "device_serial.h"
#include "sys/queue.h"
#include "MemMap.h"

/* ----------------------------[private define]------------------------------*/
/* ----------------------------[private macro]-------------------------------*/
/* ----------------------------[private typedef]-----------------------------*/
/* ----------------------------[private function prototypes]-----------------*/
static int t32_Write(  uint8_t *data, size_t nbytes);
static int t32_Read( uint8_t *data, size_t nbytes );
static int t32_Open( const char *path, int oflag, int mode );

/* ----------------------------[private variables]---------------------------*/
SECTION_RAM_NO_CACHE_BSS static volatile char t32_outport;
SECTION_RAM_NO_CACHE_BSS static volatile char t32_inport;

DeviceSerialType T32_Device = {
    .device.type = DEVICE_TYPE_CONSOLE,
    .name = "serial_t32",
//	.init = T32_Init,
    .read = t32_Read,
    .write = t32_Write,
    .open = t32_Open,
};

/* ----------------------------[private functions]---------------------------*/

void t32_writebyte(char c)
{
    /* T32 can hang here for several reasons;
     * - term.view e:address.offset(v.address(t32_outport)) e:0
     */

    while (t32_outport != 0 ) ; /* wait until port is free */
    t32_outport = c; /* send character */
}


/* ----------------------------[public functions]----------------------------*/

void  T32_Init( void ) {
    /* Nothing to do */
}

/**
 * Write data to the T32 terminal
 *
 * @param fd     File number
 * @param _buf
 * @param nbytes
 * @return
 */
static int t32_Write(  uint8_t *data, size_t nbytes)
{
    for (int i = 0; i < nbytes; i++) {
        if (*(data + i) == '\n') {
            t32_writebyte ('\r');
        }
        t32_writebyte (*(data + i));
    }
    return nbytes;
}

/**
 * Read characters from terminal
 *
 * @param data	  Where to save the data to.
 * @param nbytes  The maximum bytes to read
 * @return The number of bytes read.
 */
static int t32_Read( uint8_t *data, size_t nbytes )
{
    size_t b = nbytes;
    while (nbytes > 0)
    {
        while (t32_inport== 0) {};	/* wait for ready */
        *data = t32_inport;
        t32_inport = 0;
        data++;
        nbytes--;
    }
    return b;
}

static int t32_Open( const char *path, int oflag, int mode ) {
    return 0;
}
