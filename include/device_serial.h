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
 * serial.h
 *
 *  Created on: 23 aug 2011
 *      Author: mahi
 */

#ifndef DEVICE_SERIAL_H_
#define DEVICE_SERIAL_H_

#include <stdint.h>
#include <stdlib.h>
#include "sys/queue.h"

#define DEVICE_NAME_MAX 	16

/* Device type that maps well to POSIX open/read/write */

typedef struct DeviceSerial {
	char name[DEVICE_NAME_MAX];
	uint32_t data;
	int (*open)( const char *path, int oflag, int mode );
	int (*close)( uint8_t *data, size_t nbytes);
	/* Reads nbytes from device to data.
	 * Non-blocking read */
	int (*read)( uint8_t *data, size_t nbytes);
	/* Write nbytes from data to device
	 * Blocks until nbytes have been written */
	int (*write)( uint8_t *data, size_t nbytes );

	TAILQ_ENTRY(DeviceSerial) nextDevice;
} DeviceSerialType;

#endif /* SERIAL_H_ */
