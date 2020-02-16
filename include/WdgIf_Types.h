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


#ifndef WDGIF_TYPES_H_
#define WDGIF_TYPES_H_

typedef enum
{
	WDGIF_FAST_MODE,
	WDGIF_OFF_MODE,
	WDGIF_SLOW_MODE
}WdgIf_ModeType;

typedef void (*Wdg_TriggerLocationPtrType)(uint16 timeout);
typedef Std_ReturnType (*Wdg_SetModeLocationPtrType)(WdgIf_ModeType Mode);

typedef struct
{
	const uint8 Wdg_Index;
	Wdg_TriggerLocationPtrType Wdg_TriggerLocationPtr;
	Wdg_SetModeLocationPtrType Wdg_SetModeLocationPtr;
}Wdg_GeneralType;

#endif /* WDGIF_TYPES_H_ */
