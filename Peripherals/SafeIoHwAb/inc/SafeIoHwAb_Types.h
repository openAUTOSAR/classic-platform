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
/** @reqSettings DEFAULT_SPECIFICATION_REVISION=4.3.0*/

#ifndef SAFEIOHWAB_TYPES_H_
#define SAFEIOHWAB_TYPES_H_

/* @req SWS_BSW_00024 Include AUTOSAR Standard Types Header in implementation header */
#include <Std_Types.h>

/* Digital levels */
typedef uint8  SafeIoHwAb_LevelType;

/* Analog values base type */
typedef sint32 SafeIoHwAb_AnalogValueType;

#define SAFEIOHWAB_DUTY_INTERNAL_MAX 0x8000u 										/* 100% */

/*@req ARC_SWS_SafeIoHwAb_00019 SafeIoHwAb_DutyType have the same resulotion as Autosar (0-0x8000) */
/* Duty cycle type (0x8000 = 100%) */
typedef uint32 SafeIoHwAb_DutyType;
#define SAFEIOHWAB_DUTY_MIN		(SafeIoHwAb_DutyType)0u								/*   0% */
#define SAFEIOHWAB_DUTY_MAX		(SafeIoHwAb_DutyType)SAFEIOHWAB_DUTY_INTERNAL_MAX	/* 100% */

/* Frequency type (Hz) */
typedef uint32 SafeIoHwAb_FrequencyType;

/** Enum literals for SignalQuality */
#ifndef SAFEIOHWAB_INIVAL
#define SAFEIOHWAB_INIVAL 0U
#endif /* SAFEIOHWAB_INIVAL */

#ifndef SAFEIOHWAB_ERR
#define SAFEIOHWAB_ERR 1U
#endif /* SAFEIOHWAB_ERR */

#ifndef SAFEIOHWAB_BAD
#define SAFEIOHWAB_BAD 2U
#endif /* SAFEIOHWAB_BAD */

#ifndef SAFEIOHWAB_GOOD
#define SAFEIOHWAB_GOOD 3U
#endif /* SAFEIOHWAB_GOOD */

#define SAFE_PWM_DUTY 0U
#define SAFE_PWM_DUTYFREQ 1U

#define SAFE_DIO_READ 0U
#define SAFE_DIO_WRITE 1U
#define SAFE_DIO_WRITEREADBACK 2U

#define SAFEIOHWAB_LOW 		(SafeIoHwAb_LevelType)STD_LOW
#define SAFEIOHWAB_HIGH 	(SafeIoHwAb_LevelType)STD_HIGH

typedef struct {
    uint8 quality;
} SafeIoHwAb_StatusType;

typedef uint32 SafeIoHwAb_SignalType;

#endif /* SAFEIOHWAB_TYPES_H_ */
