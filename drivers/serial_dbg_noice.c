/* -------------------------------- Arctic Core ------------------------------
 * Arctic Core - the open source AUTOSAR platform http://arccore.com
 *
 * Copyright (C) 2009  ArcCore AB <contact@arccore.com>
 *
 * This source code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by the
 * Free Software Foundation; See <http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 * -------------------------------- Arctic Core ------------------------------*/

/*
 * DESCRIPTION
 *   NoICE (debugger) terminal
 */

/* ----------------------------[includes]------------------------------------*/
#include <stdint.h>
#include "device_serial.h"
#include "sys/queue.h"
/* ----------------------------[private define]------------------------------*/
/* ----------------------------[private macro]-------------------------------*/
/* ----------------------------[private typedef]-----------------------------*/
/* ----------------------------[private function prototypes]-----------------*/
static int noice_Write(  uint8_t *data, size_t nbytes);
static int noice_Read( uint8_t *data, size_t nbytes );
static int noice_Open( const char *path, int oflag, int mode );

/* ----------------------------[private variables]---------------------------*/
/* Location MUST match NoICE configuration */
static volatile char VUART_TX __attribute__ ((section (".noice_port")));
static volatile char VUART_RX __attribute__ ((section (".noice_port")));
volatile unsigned char START_VUART = 0;

DeviceSerialType NoICE_Device = {
	.name = "serial_noice",
//	.init = noice_Init,
	.read = noice_Read,
	.write = noice_Write,
	.open = noice_Open,
};


/* ----------------------------[public functions]----------------------------*/

void  noice_Init( void ) {
	/* Nothing to do */
}

/**
 * Write data to the NOICE terminal
 *
 * @param fd     File number
 * @param _buf
 * @param nbytes
 * @return
 */
static int noice_Write(  uint8_t *data, size_t nbytes)
{
	char *buf1 = (char *) data;
	if (START_VUART)
	{
   	   for (int i = 0; i < nbytes; i++) {
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
}

/**
 * Read characters from terminal
 *
 * @param data	  Where to save the data to.
 * @param nbytes  The maximum bytes to read
 * @return The number of bytes read.
 */
static int noice_Read( uint8_t *data, size_t nbytes )
{
	//TBD
	return 0;
}

static int noice_Open( const char *path, int oflag, int mode ) {
	return 0;
}
