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









#define MM_TYPE_UNKNOWN	0
#define MM_TYPE_MMU		1
#define MM_TYPE_PU		2

struct mm_probe_s {
	int type;
		
} mm_probe_t;

void mm_init( void ) {
		
}

// probe hardware
void mm_probe( mm_probe_t *mm_cfg ) {
		
	// found mmu or pu, etc.
		
}

