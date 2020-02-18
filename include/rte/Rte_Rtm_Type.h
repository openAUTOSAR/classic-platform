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

#ifndef RTE_RTM_TYPE_H_
#define RTE_RTM_TYPE_H_

#define RTM_NOT_SERVICE_COMPONENT
#ifdef USE_RTE
#warning This file should only be used when NOT using an Rtm Service Component.
#include "Rte_Type.h"
#else
/* Std_Types.h is normally included by Rte_Type.h */
#include "Std_Types.h"

/* Structure type Rtm_BswErrorType */
typedef struct _Rtm_BswErrorType_ Rtm_BswErrorType;
struct _Rtm_BswErrorType_ {
    uint8 moduleId;
    uint8 errorId;
};

/* Structure type Rtm_DetErrorType */
typedef struct _Rtm_DetErrorType_ Rtm_DetErrorType;
struct _Rtm_DetErrorType_ {
    uint16 moduleId;
    uint8 instanceId;
    uint8 apiId;
    uint8 errorId;
};

/* Structure type Rtm_SmalErrorType */
/* Structure type Rtm_SmalErrorType */
typedef struct _Rtm_SmalErrorType_ Rtm_SmalErrorType;
struct _Rtm_SmalErrorType_ {
    uint32 addr;
    uint32 flags;
    uint32 dump[4];
};

/* Redefinition type Rtm_ErrorType */
typedef uint8 Rtm_ErrorType;

/* Structure type Rtm_HookErrorType */
typedef struct _Rtm_HookErrorType_ Rtm_HookErrorType;
struct _Rtm_HookErrorType_ {
    uint32 flags;
    uint32 vector;
};

/* Redefinition type Rtm_StatusType */
typedef uint8 Rtm_StatusType;


/* Union type Rtm_ErrorUnionType */
typedef union {
    Rtm_SmalErrorType smal;
    Rtm_DetErrorType det;
    Rtm_HookErrorType hook;
    Rtm_BswErrorType bsw;
} Rtm_ErrorUnionType;

/* Structure type Rtm_EntryType */
typedef struct _Rtm_EntryType_ Rtm_EntryType;
struct _Rtm_EntryType_ {
    Rtm_ErrorType errorType;
    Rtm_ErrorUnionType error;
};

/** --- ENUMERATION DATA TYPES ------------------------------------------------------------------ */

/** Enum literals for Rtm_ErrorType */
#ifndef RTM_ERRORTYPE_SMAL
#define RTM_ERRORTYPE_SMAL 0U
#endif /* RTM_ERRORTYPE_SMAL */

#ifndef RTM_ERRORTYPE_DET
#define RTM_ERRORTYPE_DET 1U
#endif /* RTM_ERRORTYPE_DET */

#ifndef RTM_ERRORTYPE_HOOK
#define RTM_ERRORTYPE_HOOK 2U
#endif /* RTM_ERRORTYPE_HOOK */

#ifndef RTM_ERRORTYPE_BSW
#define RTM_ERRORTYPE_BSW 3U
#endif /* RTM_ERRORTYPE_BSW */

/** Enum literals for Rtm_StatusType */
#ifndef RTM_ALARM_STATUS_OK
#define RTM_ALARM_STATUS_OK 0U
#endif /* RTM_ALARM_STATUS_OK */

#ifndef RTM_ALARM_STATUS_NOK
#define RTM_ALARM_STATUS_NOK 1U
#endif /* RTM_ALARM_STATUS_NOK */

#ifndef RTM_ALARM_STATUS_INVALID
#define RTM_ALARM_STATUS_INVALID 2U
#endif /* RTM_ALARM_STATUS_INVALID */

#endif

#endif /* RTE_RTM_TYPE_H_ */
