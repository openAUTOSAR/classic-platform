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

#ifndef RTE_STBM_TYPE_H_
#define RTE_STBM_TYPE_H_

#ifdef USE_RTE
#warning This file should only be used when not using an RTE with StbM service component.
#include "Rte_Type.h"
#else

/* @req 4.2.2/SWS_StbM_00239 */
typedef uint8 StbM_TimeBaseStatusType;


/* @req 4.2.2/SWS_StbM_00241 */
typedef struct{
    StbM_TimeBaseStatusType timeBaseStatus;  /* Status of the Time Base */
    uint32 nanoseconds;  /* Nanoseconds part of the time */
    uint32 seconds;     /* 32 bit LSB of the 48 bits Seconds part of the time */
    uint16 secondsHi;   /* 16 bit MSB of the 48 bits Seconds part of the time*/
}StbM_TimeStampType;


/* @req 4.2.2/SWS_StbM_00242 */
typedef struct{
    StbM_TimeBaseStatusType timeBaseStatus; /* Status of the Time Base */
    uint32 nanoseconds;   /* Nanoseconds part of the time */
    uint64 seconds;         /* 48 bit Seconds part of the time */
}StbM_TimeStampExtendedType;


/* @req 4.2.2/SWS_StbM_00243 */
typedef struct{
    uint8 userDataLength;   /*  User Data Length in bytes */
    uint8 userByte0;
    uint8 userByte1;
    uint8 userByte2;
}StbM_UserDataType;
#endif
#endif
