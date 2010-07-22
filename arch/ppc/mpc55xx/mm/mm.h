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

#define BIT(x)	(1<<(x))
#define PPC_BITS_32(x,offset)		((x)<<(31-(offset)))

#define MM_PSIZE_4K			PPC_BITS_32(1,20)
#define MM_PSIZE_16K		PPC_BITS_32(2,20)
#define MM_PSIZE_64K		PPC_BITS_32(3,20)
#define MM_PSIZE_256K		PPC_BITS_32(4,20)
#define MM_PSIZE_1M			PPC_BITS_32(5,20)
#define MM_PSIZE_4M			PPC_BITS_32(6,20)
#define MM_PSIZE_16M		PPC_BITS_32(7,20)
#define MM_PSIZE_64M		PPC_BITS_32(8,20)
#define MM_PSIZE_256M		PPC_BITS_32(9,20)


/* Memory and cache attribs 
 * W - Write through, I-cache inhibit, 
 * M -Memory coherent, G-Guarded,
 * E - Endian(big=0) 
 */
#define MM_W			BIT(0)	
#define MM_I			BIT(1)
#define MM_M			BIT(2)
#define MM_G			BIT(3)
#define MM_E			BIT(4)

/* memory size */
#define MM_SIZE_8		BIT(16)
#define MM_SIZE_16		BIT(17)
#define MM_SIZE_32		BIT(18)

/* permissions */
#define MM_SX			BIT(24)
#define MM_SR			BIT(25)
#define MM_SW			BIT(26)
#define MM_UX			BIT(27)
#define MM_UR			BIT(28)
#define MM_UW			BIT(29)

#define MM_PERM_STEXT		(MM_SR|MM_X)
#define MM_PERM_SDATA		(MM_SR|MM_SW)


#endif /*MM_H_*/
