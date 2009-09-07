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









#ifndef COM_MISC_H_
#define COM_MISC_H_

#include "Com.h"
/*
 * The following function are exported only for testing purposes.
 */
uint8 Com_Filter(ComSignal_type *signal);
void Com_CopyFromSignal(const ComSignal_type *signal, void *Destination);
void Com_CopyToSignal(ComSignal_type *signal, const void *Source);

/*
 * This function copies numBits bits of data from Source to Destination with the possibility to offset
 * both the source and destination.
 *
 * Return value: the last bit it copies (sign bit).
 */
uint8 Com_CopyData(void *Destination, const void *Source, uint8 numBits, uint8 destOffset, uint8 sourceOffset);
#endif /* COM_MISC_H_ */
