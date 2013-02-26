/*
 * mmu.c
 *
 *  Created on: 18 feb 2013
 *      Author: mahi
 */
#include <stdint.h>
#include "Cpu.h"
#include "asm_ppc.h"
#include "mm.h"

void MM_TlbSetup( const struct TlbEntry *tblTable  )
{
	int32_t i = 0;

	/* Setup the TLBs */
	while( tblTable[i].entry != (-1UL) ) {
		set_spr(SPR_MAS0, tblTable[i].mas0);
		set_spr(SPR_MAS1, tblTable[i].mas1);
		set_spr(SPR_MAS2, tblTable[i].mas2);
		set_spr(SPR_MAS3, tblTable[i].mas3);
		msync();
		isync();
		tlbwe();
		i++;
	}
}


