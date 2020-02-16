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


#include <stdlib.h>
#include <string.h>

#include "Det.h"

/* @req PDUR232 */
#if defined(USE_DEM)
#include "Dem.h"
#endif
#include "PduR.h"
#include "debug.h"


#if !(((PDUR_SW_MAJOR_VERSION == 1) && (PDUR_SW_MINOR_VERSION == 2)) )
#error PduR: Expected BSW module version to be 1.2.*
#endif


/* @req PDUR0774 */
#if !(((PDUR_AR_RELEASE_MAJOR_VERSION == 4) && (PDUR_AR_RELEASE_MINOR_VERSION == 0)) )
#error PduR: Expected AUTOSAR version to be 4.0.*
#endif


/*
 * The state of the PDU router.
 */

/* @req PDUR644 */
/* @req PDUR325 */
PduR_StateType PduRState = PDUR_UNINIT;


#if PDUR_ZERO_COST_OPERATION == STD_OFF



const PduR_PBConfigType * PduRConfig;


/*
 * Initializes the PDU Router.
 */
void PduR_Init (const PduR_PBConfigType* ConfigPtr) {

	//Enter(0);

	/* !req PDUR106 */
	/* !req PDUR709 */


	// Make sure the PDU Router is uninitialized.
	// Otherwise raise an error.
	if (PduRState != PDUR_UNINIT) {
		// Raise error and return.
		PDUR_DET_REPORTERROR(MODULE_ID_PDUR, PDUR_INSTANCE_ID, 0x00, PDUR_E_INVALID_REQUEST);
	}

	/* @req PDUR0776 */
	else if (ConfigPtr == NULL) {
		PDUR_DET_REPORTERROR(MODULE_ID_PDUR, PDUR_INSTANCE_ID, 0x00, PDUR_E_NULL_POINTER);
	} else {
		PduRConfig = ConfigPtr;

		// Start initialization!
		DEBUG(DEBUG_LOW,"--Initialization of PDU router--\n");

		//uint8 failed = 0;

        // Initialize buffers
        /* @req PDUR308 */
        if (PduRConfig->DefaultValues != NULL && PduRConfig->DefaultValueLengths != NULL) {
            for (uint32 i = 0u; i < PduR_RamBufCfg.NTxBuffers; i++) {
                if (*PduRConfig->DefaultValueLengths[i] > 0u) {
                    memcpy(PduR_RamBufCfg.TxBuffers[i], PduRConfig->DefaultValues[i], *PduRConfig->DefaultValueLengths[i]);
                }
            }
        }


        /*if (failed) {
            // TODO Report PDUR_E_INIT_FAILED to Dem.
            PduRState = PDUR_REDUCED;
            DEBUG(DEBUG_LOW,"--Initialization of PDU router failed--\n");
        }*/

        // The initialization succeeded!

        /* @req PDUR326 */
        PduRState = PDUR_ONLINE;
        DEBUG(DEBUG_LOW,"--Initialization of PDU router completed --\n");
	}

}

/*
void PduR_BufferInc(PduRTxBuffer_type *Buffer, uint8 **ptr) {
	(*ptr) = (*ptr) + Buffer->Length;

	// TODO make more efficient without multiplication.
	//lint -e946 //PC-Lint Exception of MISRA rule 17.3
	if ( *ptr >= ( Buffer->Buffer + (Buffer->Depth * Buffer->Length) ) ) {
		*ptr = Buffer->Buffer;
	}
	//  *val = (*val + 1) % Buffer->Depth;
}

void PduR_BufferQueue(PduRTxBuffer_type *Buffer, const uint8 * SduPtr) {
    imask_t state;
    Irq_Save(state);

	if (PduR_BufferIsFull(Buffer)) { // Buffer is full
		PduR_BufferFlush(Buffer);
#if defined(USE_DEM)
		Dem_ReportErrorStatus(PDUR_E_PDU_INSTANCE_LOST, DEM_EVENT_STATUS_FAILED);
#endif

	} else {
		// Copy data to last place in buffer
		memcpy(Buffer->Last, SduPtr, sizeof(uint8) * Buffer->Length);

		PduR_BufferInc(Buffer, &Buffer->Last);
		Buffer->NrItems++;
		DEBUG(DEBUG_LOW,"\tBuffer %d: Queued data %d. Status: NrItems %d, First %d, Last %d\n", Buffer->BufferId, *SduPtr, Buffer->NrItems, *Buffer->First, *Buffer->Last);

	}
    Irq_Restore(state);
}

void PduR_BufferDeQueue(PduRTxBuffer_type *Buffer, uint8 *SduPtr) {
    imask_t state;
    Irq_Save(state);

	if (Buffer->NrItems > 0) {
		memcpy(SduPtr, Buffer->First, sizeof(uint8) * Buffer->Length);
		PduR_BufferInc(Buffer, &Buffer->First);
		Buffer->NrItems--;
		DEBUG(DEBUG_LOW,"\tBuffer %d: DeQueueing data. Status: NrItems %d, First %d, Last %d\n", Buffer->BufferId, Buffer->NrItems, *Buffer->First, *Buffer->Last);
	} else {
		DEBUG(DEBUG_LOW,"\tBuffer %d: Buffer is empty, nothing to dequeue!\n", Buffer->BufferId);
	}
    Irq_Restore(state);
}

void PduR_BufferFlush(PduRTxBuffer_type *Buffer) {
	DEBUG(DEBUG_LOW,"\tBuffer %d: Flushing!\n", Buffer->BufferId);
    imask_t state;
    Irq_Save(state);
	Buffer->NrItems = 0;
	Buffer->First = Buffer->Buffer;
	Buffer->Last = Buffer->Buffer;
	Buffer->TxConfP = 0;
    Irq_Restore(state);
}

uint8 PduR_BufferIsFull(PduRTxBuffer_type *Buffer) {
    imask_t state;
    Irq_Save(state);
	uint8 rv = 0;
	if (Buffer->NrItems < Buffer->Depth) {
		rv = 0;
	} else {
		rv = 1;
	}
    Irq_Restore(state);
	return rv;
}
*/



#if PDUR_VERSION_INFO_API == STD_ON

/* @req PDUR217  */
/* @req PDUR338 */
void PduR_GetVersionInfo (Std_VersionInfoType* versionInfo){
	versionInfo->moduleID = (uint16)MODULE_ID_PDUR;
	versionInfo->vendorID = 60u;
    versionInfo->sw_major_version = PDUR_SW_MAJOR_VERSION;
    versionInfo->sw_minor_version = PDUR_SW_MINOR_VERSION;
    versionInfo->sw_patch_version = PDUR_SW_PATCH_VERSION;
    versionInfo->ar_major_version = PDUR_AR_RELEASE_MAJOR_VERSION;
    versionInfo->ar_minor_version = PDUR_AR_RELEASE_MINOR_VERSION;
    versionInfo->ar_patch_version = PDUR_AR_RELEASE_REVISION_VERSION;
}
#endif

/* !req PDUR341 Change return type */
uint32 PduR_GetConfigurationId (void) {
	//PduR_DevCheck(0,1,0x18,E_NOT_OK);

	/* @req PDUR280 */
	return PduRConfig->PduRConfigurationId;
}

/* !req PDUR615 */
void PduR_EnableRouting(PduR_RoutingPathGroupIdType id) {
    // TODO: implement
	(void)id;
}

/* !req PDUR617 */
void PduR_DisableRouting(PduR_RoutingPathGroupIdType id) {
    // TODO: implement
	(void)id;
}


#endif
