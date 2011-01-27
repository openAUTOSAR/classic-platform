/*
 * WdgIf_Types.h
 *
 *  Created on: 14 maj 2010
 *      Author: Fredrik
 */

#ifndef WDGIF_TYPES_H_
#define WDGIF_TYPES_H_

typedef enum
{
	WDGIF_FAST_MODE,
	WDGIF_OFF_MODE,
	WDGIF_SLOW_MODE,
}WdgIf_ModeType;

typedef void (*Wdg_TriggerLocationPtrType)(void);
typedef void (*Wdg_SetModeLocationPtrType)(WdgIf_ModeType Mode);

typedef struct
{
	const uint8 Wdg_Index;
	Wdg_TriggerLocationPtrType Wdg_TriggerLocationPtr;
	Wdg_SetModeLocationPtrType Wdg_SetModeLocationPtr;
}Wdg_GeneralType;

#endif /* WDGIF_TYPES_H_ */
