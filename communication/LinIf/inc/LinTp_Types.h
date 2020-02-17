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

#ifndef LINTP_TYPES_H_
#define LINTP_TYPES_H_

#include "Std_Types.h"


typedef enum {
    LINTP_UNINIT,         /** @req SWS_LinIf_00316 */
    LINTP_INIT,           /** @req SWS_LinIf_00483 */
}LinTp_StatusType;


/* @req SWS_LinIf_00426 */
/* Global configuration */
typedef struct
{
	/* @req ECUC_LinTp_00624 */
	/* Configures the maximum number of allowed response pending frames. */
	uint16 LinTpMaxNumberOfRespPendingFrames;

	/* @req ECUC_LinTp_00635 */
	/* Maximum number of NSdus. This parameter is needed only in case
	 * of post-build loadable implementation using static memory allocation. */
	uint16 LinTpMaxRxNSduCnt;

	/* @req ECUC_LinTp_00636 */
	/*Maximum number of NSdus. This parameter is needed only in case
	 * of post-build loadable implementation using static memory allocation. */
	uint16 LinTpMaxTxNSduCnt;

	/* @req ECUC_LinTp_00622 */
	/* P2*max timeout when a response pending frame is expected in seconds.
       Note that the minimum value of LinTpP2Max shall be more than or equal
       to the value of LinTpP2Timing. */
	uint16 LinTpP2Max;

	/* @req ECUC_LinTp_00625 */
	/* Definition of the P2max timeout observation parameter in seconds. */
	uint16 LinTpP2Timing;


}LinTp_ConfigType;

/* @req SWS_LinIf_00629 */
typedef enum {
    LINTP_APPLICATIVE_SCHEDULE = 0,
    LINTP_DIAG_REQUEST,
    LINTP_DIAG_RESPONSE
}LinTp_Mode;



/* LinTP configuration */
typedef struct
{
    const LinTp_ConfigType *LinTprGlobalConfig;
} LinTp_Type;



#endif
