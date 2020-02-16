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

/** Current mode of the Communication Manager (main state of the state machine). */
/**  @req COMM879 */
typedef enum {
	COMM_NO_COMMUNICATION = 0,
	COMM_SILENT_COMMUNICATION = 1,
	COMM_FULL_COMMUNICATION = 2
} ComM_ModeType; /** @req COMM867 @req COMM868 */

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

/** Inhibition status of ComM. */
typedef uint8 ComM_InhibitionStatusType;

#define COMM_INHIBITION_STATUS_NONE					(0u)
/** Wake Up inhibition active */
#define COMM_INHIBITION_STATUS_WAKE_UP				(1u)
/** Limit to �No Communication� mode active */
#define COMM_INHIBITION_STATUS_NO_COMMUNICATION		(uint8)(1u << 1)

#endif /*COMM_TYPES_H*/
