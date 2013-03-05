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




#ifndef MM_H_
#define MM_H_

/* [address: XXXXXX/auto]
 * 
 * application: xx
 * allocation: [auto/manual]
 *   - allocates text,data,bsss
 * [segment: text]
 * size: rolldown with sizes... auto??
 *
 * application: xx
 * segment: special
 *   address: [auto/manual]
 *   permissions: 
 * size: 
 * 
 * CPU dependent
 * -------------------------------
 * ENUM [
 *   PU,
 *   MMU,
 * ] MM_TYPE
 *
 * ENUM [
 *  PSIZE_PU_????
 * 
 * ] MM_PU_PSIZE;
 *  
 * ENUM [
 *   PSIZE_1K,
 *   PSIZE_4K,
 *   PSIZE_64K,
 *   PSIZE_1M, 
 * ] MM_MMU_PSIZE; 
 * 
 * ENUM [
 *   PERM_S_R,
 * 	 PERM_S_RW,
 *   ....
 * ] MM_PERMISSIONS
 * 
 * NOT CPU dependent
 * -------------------------------
 * 
 * For each application.
 * 
 * ENUM [
 *   AUTO {
 *   }
 *   MANUAL {
 *     ADDRESS,
 *     PERMISSION,
 *     SIZE,  
 *   }
 * ] ALLOCATION;
 */





#if !defined(_ASSEMBLER_)

/* C-code */

struct TlbEntry {
	uint32_t entry;
	uint32_t mas0;
	uint32_t mas1;
	uint32_t mas2;
	uint32_t mas3;
};

void MM_TlbSetup( const struct TlbEntry *tblTable  );
/* "naked" funciton written in assembler */
void MM_TlbSetupOne( uint32_t mas0, uint32_t mas1, uint32_t mas2, uint32_t mas3 );

#endif



#endif /*MM_H_*/
