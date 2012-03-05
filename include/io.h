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

#ifndef IO_H_
#define IO_H_

#define WRITE8(address, value)      (*(uint8_t*)(address) = (value))
#define READ8(address)              ((uint8_t)(*(uint8_t*)(address)))

#define WRITE16(address, value)     (*(vuint16_t*)(address) = (value))
#define READ16(address)             ((uint16_t)(*(vuint16_t*)(address)))

#define WRITE32(address, value)     (*(vuint32_t*)(address) = (value))
#define READ32(address)             ((uint32_t)(*(vuint32_t*)(address)))

#define WRITE64(address, value)     (*(vuint64_t*)(address) = (value))
#define READ64(address)             ((uint64_t)(*(vuint64_t*)(address)))


#endif /* IO_H_ */
