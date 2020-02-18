/*-------------------------------- Arctic Core ------------------------------
 * Copyright (C) 2013, ArcCore AB, Sweden, www.arccore.com.
 * Contact: <contact@arccore.com>
 *
 * You may ONLY use this file:
 * 1)if you have a valid commercial ArcCore license and then in accordance with
 * the terms contained in the written license agreement between you and ArcCore,
 * or alternatively
 * 2)if you follow the terms found in GNU General Public License version 2 as
 * published by the Free Software Foundation and appearing in the file
 * LICENSE.GPL included in the packaging of this file or here
 * <http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt>
 *-------------------------------- Arctic Core -----------------------------*/

#include "Std_Types.h"
#include "bitdef.h"
#include <stdio.h>

void Bitdef_Print(RegInfoType *regP, uint16 data) {
    uint16 odata = data;
    uint16 pos;
    uint16 mask;

    printf("reg: %2d , data:%02x\n", regP->reg, data);
    for (uint16 i = 0; i < regP->defSize; i++) {
        pos = regP->defPtr[i].pos;
        mask = ((1u << regP->defPtr[i].size) - 1u);
        //if( data & (1u<<pos) ) {
        printf("  %2x %2d %s\n", ((data >> pos) & mask), pos,
                regP->defPtr[i].desc);
        //}
    }
}
