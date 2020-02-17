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

/** @fileSafetyClassification ASIL **/

#include "ComM.h"
#include "ComM_Internal.h"
#include "ComM_EcuM.h"

// ECU State Manager Callbacks
// As it is called during startup, it is safety classified.
// ---------------------------
/* @req COMM275 */
void ComM_EcuM_WakeUpIndication( NetworkHandleType Channel ){
    /* @req COMM814 */
    /* Validates parameters / status, and if it fails will call DET and then immediately leave the function with the specified return code */
    /* This is not inline with Table 8, ISO26262-6:2011, Req 1a and 1h */
    COMM_VALIDATE_INIT_NORV(COMM_SERVICEID_ECUM_WAKEUPINDICATION);
    COMM_VALIDATE_CHANNEL_NORV(Channel, COMM_SERVICEID_ECUM_WAKEUPINDICATION);
	if ((ComM_Internal.Channels[Channel].Mode == COMM_NO_COMMUNICATION) ||
		((ComM_Internal.Channels[Channel].SubMode == COMM_SILENT_COM) && /* during silent communication to no communication transition*/
	     (ComM_Internal.Channels[Channel].lastRequestedMode == COMM_NO_COMMUNICATION) && (ComM_Internal.Channels[Channel].requestPending == TRUE))){ /* wake up arrival during a boundary condition where ComM unsync with SM */
		ComM_Internal.Channels[Channel].EcuMWkUpIndication = TRUE;
		ComM_Internal.Channels[Channel].internalRequest = TRUE;
	}
}
