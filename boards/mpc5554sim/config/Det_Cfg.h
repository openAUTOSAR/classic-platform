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
 * Development Error Tracer driver
 *
 * Specification: Autosar v2.0.1, Final
 *
 */
#warning "This default file may only be used as an example!"

#ifndef DET_CFG_H
#define DET_CFG_H

#define DET_ENABLE_CALLBACKS STD_ON  // Enable to use callback on errors
#define DET_USE_RAMLOG       STD_ON  // Enable to log DET errors to ramlog
#define DET_WRAP_RAMLOG      STD_ON  // The ramlog wraps around when reaching the end
#define DET_USE_STDERR       STD_OFF // Enable to get DET errors on stderr

#define DET_DEINIT_API       STD_ON // Enable/Disable the Det_DeInit function

#define DET_RAMLOG_SIZE (32)        // Number of entries in ramlog
#define DET_NUMBER_OF_CALLBACKS (5) // Number of callbacks

#endif /*DET_CFG_H*/
