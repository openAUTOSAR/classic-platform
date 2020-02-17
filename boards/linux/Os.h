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

#ifndef OS_H_
#define OS_H_

#include "Std_Types.h"

typedef uint8 StatusType;
typedef sint16 ISRType;

typedef sint32 ApplicationType;
typedef sint32 AppModeType;

typedef uint16      TaskType;
typedef TaskType*   TaskRefType;
typedef uint64_t        EventMaskType;
typedef uint32      TickType;
typedef TickType *TickRefType;

static inline void DisableAllInterrupts( void ) {
}

static inline void EnableAllInterrupts( void ) {
}

static inline void SuspendAllInterrupts( void ) {
}

static inline void ResumeAllInterrupts( void ) {
}

static inline void SuspendOSInterrupts( void ) {
}

static inline void ResumeOSInterrupts( void ) {
}

#endif /*OS_H_*/
