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


#ifndef COMM_TYPES_H
#define COMM_TYPES_H

#include "Rte_ComM_Type.h"

typedef enum {
    COMM_NO_COM_NO_PENDING_REQUEST = 0,
    COMM_NO_COM_REQUEST_PENDING,
    COMM_FULL_COM_NETWORK_REQUESTED,
    COMM_FULL_COM_READY_SLEEP,
    COMM_SILENT_COM
} ComM_StateType;

/** Initialization status of ComM. */
typedef enum {
    COMM_UNINIT,
    COMM_INIT
} ComM_InitStatusType;

/** Enum literals for ComM_ModeType */
#ifndef COMM_NO_COMMUNICATION
#define COMM_NO_COMMUNICATION 0U
#endif /* COMM_NO_COMMUNICATION */

#ifndef COMM_SILENT_COMMUNICATION
#define COMM_SILENT_COMMUNICATION 1U
#endif /* COMM_SILENT_COMMUNICATION */

#ifndef COMM_FULL_COMMUNICATION
#define COMM_FULL_COMMUNICATION 2U
#endif /* COMM_FULL_COMMUNICATION */

#ifndef COMM_NOT_USED_USER_ID
#define COMM_NOT_USED_USER_ID 255U
#endif /* COMM_NOT_USED_USER_ID */



/** Inhibition status of ComM. Inhibition status is really a bit struct with bit 0 = inhibit to nocomm and bit 1 = wakeup inhibit */
#define COMM_INHIBITION_STATUS_NONE					(0u)
/** Wake Up inhibition active */
#define COMM_INHIBITION_STATUS_WAKE_UP				(1u)
/** Limit to �No Communication� mode active */
#define COMM_INHIBITION_STATUS_NO_COMMUNICATION		(uint8)(1u << 1)

typedef enum{
    PNC_NO_COMMUNICATION=0,
    /* @req ComM907 */
    PNC_REQUESTED,
    PNC_READY_SLEEP,
    PNC_PREPARE_SLEEP
}ComM_PncModeType;

typedef enum{
    /* @req ComM924 */
    PNC_NO_COMMUNICATION_STATE=0,
    PNC_FULL_COMMUNICATION_STATE
}ComM_PncStateType;

typedef uint8  PNCHandleType;
#endif /*COMM_TYPES_H*/
