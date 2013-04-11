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
