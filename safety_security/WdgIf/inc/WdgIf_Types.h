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

/** @reqSettings DEFAULT_SPECIFICATION_REVISION=4.3.0 */

/** @file WdgIf_Types.h
 *
 *  This file contains the types used in Wdg module.
 */


#ifndef WDGIF_TYPES_H_
#define WDGIF_TYPES_H_

/** @req SWS_WdgIf_00010 */ /*The Watchdog Interface’s implementer shall place the type definitions of the Watchdog Interface in the file WdgIf_Types.h*/

/* @req SWS_WdgIf_00041 */
/* @req SWS_BSW_00024 Include AUTOSAR Standard Types Header in implementation header */
#include "Std_Types.h"

/** @enum WdgIf_ModeType
*   @brief   AUTOSAR Specified Watchdog Mode Type Definition.
*   The WdgIf_ModeType values shall be passed as parameters to the watchdog drivers mode switching function (Wdg_SetMode).
*/
/* @req SWS_WdgIf_00061 */
typedef enum
{
    WDGIF_OFF_MODE,
    WDGIF_SLOW_MODE,
    WDGIF_FAST_MODE
}WdgIf_ModeType;

/** @brief   ArcCore Specified Trigger Location Pointer type.
*   It is a function pointer.
*/
typedef void (*Wdg_TriggerLocationPtrType)(uint16 timeout);

/** @brief   ArcCore Specified Set Mode Location Pointer type.
*   It is a function pointer.
*/
typedef Std_ReturnType (*Wdg_SetModeLocationPtrType)(WdgIf_ModeType Mode);

/**
  * @brief   ArcCore Specified Wdg General type.
*/
typedef struct
{
    const uint8 Wdg_Index;
    Wdg_TriggerLocationPtrType Wdg_TriggerLocationPtr;
    Wdg_SetModeLocationPtrType Wdg_SetModeLocationPtr;
}Wdg_GeneralType;

typedef struct
{
    uint8 WdgIf_DeviceIndex;
    const Wdg_GeneralType *WdgRef;
}WdgIf_DeviceType;

typedef struct
{
    uint8 WdgIf_NumberOfDevices;
}WdgIf_GeneralType;

typedef struct
{
    const WdgIf_GeneralType     *WdgIf_General;
    const WdgIf_DeviceType      *WdgIf_Device;
}WdgIf_ConfigType;

#endif /* WDGIF_TYPES_H_ */

