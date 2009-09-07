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









static uint8_tos_pool[20000];

typedef struct  {
	void *free;	
}os_memory_t;

k_memory_tos_mem = { .free = NULL };

void *k_malloc( int size ) {
	void *t;
	if(os_mem.free != NULL ) {
		t =os_mem.free;
		(uint8)k_mem.free += size;
		return t;
	}
	return NULL;
}
