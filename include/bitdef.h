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

#ifndef BITDEF_H_
#define BITDEF_H_

#define BITDEF_ENTRY(_reg, _regbitdef ) \
            [_reg].reg = _reg,  \
            [_reg].defPtr = _regbitdef, \
            [_reg].defSize = sizeof(_regbitdef)/sizeof(BitDefType)


typedef struct {
    uint8 pos;
    uint8 size;
    char *desc;
} BitDefType;

typedef struct {
    uint16 reg;
    BitDefType *defPtr;
    uint16 defSize;
} RegInfoType;

void Bitdef_Print(RegInfoType *regP, uint16 data);

#endif /* BITDEF_H_ */
