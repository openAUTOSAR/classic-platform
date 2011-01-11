/* -------------------------------- Arctic Core ------------------------------
 * Arctic Core - the open source AUTOSAR platform http://arccore.com
 *
 * Copyright (C) 2009  ArcCore AB <contact@arccore.com>
 *
 * This source code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by the
 * Free Software Foundation; See <http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 * -------------------------------- Arctic Core ------------------------------*/








#ifndef COMM_TYPES_H
#define COMM_TYPES_H

/** Current mode of the Communication Manager (main state of the state machine). */
/** @req COMM484  @req COMM190  @req COMM248 */
typedef enum {
	COMM_NO_COMMUNICATION = 0,
	COMM_SILENT_COMMUNICATION = 1,
	COMM_FULL_COMMUNICATION = 2
} ComM_ModeType;

/** Initialization status of ComM. */
typedef enum {
	COMM_UNINIT,
	COMM_INIT
} ComM_InitStatusType;  /**< @req COMM494 */

/** Inhibition status of ComM. */
typedef uint8 ComM_InhibitionStatusType;  /**< @req COMM496 */

#define COMM_INHIBITION_STATUS_NONE					(0u)
/** Wake Up inhibition active */
#define COMM_INHIBITION_STATUS_WAKE_UP				(1u)
/** Limit to “No Communication” mode active */
#define COMM_INHIBITION_STATUS_NO_COMMUNICATION		(uint8)(1u << 1)

#endif /*COMM_TYPES_H*/
