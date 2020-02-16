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








#ifndef MEMIF_TYPES_H_
#define MEMIF_TYPES_H_

typedef enum {

	//	The underlying abstraction module or device driver has
	//	not been initialized (yet).
	MEMIF_UNINIT,
	// The underlying abstraction module or device driver is
	// currently idle.
	MEMIF_IDLE,
	
	// The underlying abstraction module or device driver is
	// currently busy.
	MEMIF_BUSY,
	
	//	The underlying abstraction module is busy with internal
	// management operations. The underlying device driver
	// can be busy or idle.
	MEMIF_BUSY_INTERNAL              
} MemIf_StatusType;

typedef enum {
	//The job has been finished successfully.
	MEMIF_JOB_OK,
	// The job has not been finished successfully.
	MEMIF_JOB_FAILED,
	//	The job has not yet been finished.
	MEMIF_JOB_PENDING,
	//	The job has been canceled.
	MEMIF_JOB_CANCELLED,
	//	The requested block is inconsistent, it may contain
	//	corrupted data.	       
	MEMIF_BLOCK_INCONSISTENT,
	// The requested block has been marked as invalid,
	// the requested operation can not be performed.
	MEMIF_BLOCK_INVALID
	
} MemIf_JobResultType; 


typedef enum {
	//    The underlying memory abstraction modules and
	//    drivers are working in slow mode.
	MEMIF_MODE_SLOW,
	// The underlying memory abstraction modules and
	// drivers are working in fast mode.
    MEMIF_MODE_FAST
} MemIf_ModeType;

// Definition of broadcast device ID
#define MEMIF_BROADCAST_ID	0xff

#endif /*MEMIF_TYPES_H_*/
