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

#ifndef ARC_T1_INT_H
#define ARC_T1_INT_H

#include "arc.h"

typedef struct {
    uint16_t 		id;
    char 		    name[OS_ARC_PCB_NAME_SIZE];
    uint8_t			core;
    uint8_t			priority;
    int16_t			vector;
    int16_t 		type;
    void 			(*entry)( void );
} ArcT1Info_t;

ArcT1Info_t * Arc_T1_GetInfoAboutIsr(uint16_t *len);
void Arc_T1_Init(void);

void Arc_T1_MainFunction(void);

void Arc_T1_BackgroundFunction(void);

#endif /* ARC_T1_INT_H */
