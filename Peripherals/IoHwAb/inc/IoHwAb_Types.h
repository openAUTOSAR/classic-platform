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



#ifndef IOHWAB_TYPES_H_
#define IOHWAB_TYPES_H_

#include "Std_Types.h"

/* Digital levels */
typedef uint8  IoHwAb_LevelType;

/* Analog values base type */
typedef sint32 IoHwAb_AnalogValueType;

#if !defined(CFG_IOHWAB_USE_SERVICE_COMPONENT)
typedef uint32 IoHwAb_SignalType;
#endif

/* Duty cycle type */
typedef uint32 IoHwAb_DutyType;
#define IOHWAB_DUTY_MIN		(IoHwAb_DutyType)0u		   /*   0% */
#define IOHWAB_DUTY_MAX		(IoHwAb_DutyType)0x8000u   /* 100% */

/* Frequency type (Hz) */
typedef uint32 IoHwAb_FrequencyType;

/* ISO14229, IoControl */
#define	IOHWAB_RETURNCONTROLTOECU 	0
#define	IOHWAB_RESETTODEFAULT		1
#define	IOHWAB_FREEZECURRENTSTATE	2
#define	IOHWAB_SHORTTERMADJUST		3

/** Enum literals for DigitalLevel */
#ifndef IOHWAB_LOW
#define IOHWAB_LOW 0U
#endif /* IOHWAB_LOW */

#ifndef IOHWAB_HIGH
#define IOHWAB_HIGH 1U
#endif /* IOHWAB_HIGH */

/** Enum literals for SignalQuality */
#ifndef IOHWAB_INIVAL
#define IOHWAB_INIVAL 0U
#endif /* IOHWAB_INIVAL */

#ifndef IOHWAB_ERR
#define IOHWAB_ERR 1U
#endif /* IOHWAB_ERR */

#ifndef IOHWAB_BAD
#define IOHWAB_BAD 2U
#endif /* IOHWAB_BAD */

#ifndef IOHWAB_GOOD
#define IOHWAB_GOOD 3U
#endif /* IOHWAB_GOOD */

typedef uint8 IoHwAb_QualityType;

typedef struct {
    uint8 quality;
} IoHwAb_StatusType;

#endif /* IOHWAB_TYPES_H_ */
