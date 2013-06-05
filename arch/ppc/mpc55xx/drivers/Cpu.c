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

/* ----------------------------[information]----------------------------------*/
/*
 * Author: mahi
 *
 * Part of Release:
 *   Non-Autosar
 *
 * Description:
 *   Implements cache routines.
 */

/* ----------------------------[includes]------------------------------------*/
#include "Cpu.h"

/* ----------------------------[private define]------------------------------*/
/* ----------------------------[private macro]-------------------------------*/
/* ----------------------------[private typedef]-----------------------------*/
/* ----------------------------[private function prototypes]-----------------*/
/* ----------------------------[private variables]---------------------------*/
/* ----------------------------[private functions]---------------------------*/
/* ----------------------------[public functions]----------------------------*/



void Cache_Invalidate( void ) {

	/* Wait for it to clear */
	while( get_spr(SPR_L1CSR0) & L1CSR0_CINV ) {}

	msync();
	isync();
	set_spr(SPR_L1CSR0, L1CSR0_CINV );

	/* Wait for it to clear */
	while( get_spr(SPR_L1CSR0) & L1CSR0_CINV ) {}
}

/**
 * Unified cache enable.
 * MPC5668
 */
void Cache_EnableU( void ) {
	uint32 l1csr0;

	/* Reset  does  not  invalidate  the  cache  lines;  therefore,  the  cache  should  be  invalidated
     * explicitly after a hardware reset.
     */
	Cache_Invalidate();

	l1csr0 = get_spr(SPR_L1CSR0);
	l1csr0 |= L1CSR0_CE;		/* enable cache */
	msync();
	isync();
	set_spr(SPR_L1CSR0,l1csr0);
}

void Cache_EnableD( void ) {

}

void Cache_EnableI( void ) {

}

