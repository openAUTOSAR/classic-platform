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

/* Generally implemented requirements */
/* @req PDUR221 *//* Check of PDU id */
/* @req PDUR328 *//* Shall perform routing when ONLINE. See PDUR_VALIDATE_INITIALIZED */
/* @req PDUR330 *//* No routing when uninitialized. Satisfied by PDUR_VALIDATE_INITIALIZED */
/* @req PDUR332 *//* If error detected (when DevErroDetect is enabled), report to Det and return error */
/* @req PDUR0780 *//* Generic service ids for upper layer modules */
/* @req PDUR0781 *//* Generic service ids for lower layer interface modules */
/* @req PDUR0782 *//* Generic service ids for lower layer transport protocol modules */

/*#include <stdlib.h>*/
#include <string.h>
#include "PduR.h"
#if (PDUR_DEV_ERROR_DETECT == STD_ON)
#include "Det.h"
#endif

/* @req PDUR232 */
#if defined(USE_DEM)
#include "Dem.h"
#endif
#include "debug.h"


#if !(((PDUR_SW_MAJOR_VERSION == 3) && (PDUR_SW_MINOR_VERSION == 0)) )
#error PduR: Expected BSW module version to be 3.0.*
#endif


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

    uint32 i;

    /* !req PDUR106 */

    // Make sure the PDU Router is uninitialized.
    // Otherwise raise an error.
    if (PduRState != PDUR_UNINIT) {
        // Raise error and return.
        PDUR_DET_REPORTERROR(PDUR_MODULE_ID, PDUR_INSTANCE_ID, 0x00, PDUR_E_INVALID_REQUEST);
    }else if (ConfigPtr == NULL) {/* @req PDUR0776 */
        PDUR_DET_REPORTERROR(PDUR_MODULE_ID, PDUR_INSTANCE_ID, 0x00, PDUR_E_NULL_POINTER);
    } else {
        PduRConfig = ConfigPtr;
        // Start initialization!
        DEBUG(DEBUG_LOW,"--Initialization of PDU router--\n");

        // Initialize buffers
        /* @req PDUR308 */
        if((PduRConfig->DefaultValues != NULL) && (PduRConfig->DefaultValueLengths != NULL)) {
            for (i = 0u; i < PduR_RamBufCfg.NTxBuffers; i++) {
                if (*PduRConfig->DefaultValueLengths[i] > 0u) {
                    memcpy(PduR_RamBufCfg.TxBuffers[i].DataPtr, PduRConfig->DefaultValues[i], *PduRConfig->DefaultValueLengths[i]);
                }
            }
        }

        for (i=0; i < PduR_RamBufCfg.NTpBuffers; i++) {
            PduR_RamBufCfg.TpBuffers[i].status= PDUR_BUFFER_FREE; //Reset Tp buffer status
        }
#if (PDUR_MAX_NOF_ROUTING_PATH_GROUPS > 0)
        /* Init routing path groups */
        /* @req PDUR0709 */
        for(  i = 0; i < PduRConfig->NofRoutingPathGroups; i++ ) {
            PdurSetRoutingPathEnabled((PduR_RoutingPathGroupIdType)i, PduRConfig->RoutingPathGroups[i].EnabledAtInit);
        }
#endif
        /* @req PDUR326 */
        PduRState = PDUR_ONLINE;
        DEBUG(DEBUG_LOW,"--Initialization of PDU router completed --\n");
    }

}



#if PDUR_VERSION_INFO_API == STD_ON

/* @req PDUR217  */
/* @req PDUR338 */
void PduR_GetVersionInfo (Std_VersionInfoType* versionInfo){
    versionInfo->moduleID = (uint16)PDUR_MODULE_ID;
    versionInfo->vendorID = 60u;
    versionInfo->sw_major_version = PDUR_SW_MAJOR_VERSION;
    versionInfo->sw_minor_version = PDUR_SW_MINOR_VERSION;
    versionInfo->sw_patch_version = PDUR_SW_PATCH_VERSION;
}
#endif

/* @req PDUR341 Change return type */
PduR_PBConfigIdType PduR_GetConfigurationId (void) {
    /* @req PDUR119 */
    /* @req PDUR280 */
    if( PDUR_UNINIT == PduRState ) {
       PDUR_DET_REPORTERROR(PDUR_MODULE_ID, PDUR_INSTANCE_ID, PDUR_SERVICEID_GETCONFIGURATIONID, PDUR_E_INVALID_REQUEST);
       /*lint -e{904} Return statement is necessary in case of reporting a DET error */
       return PDUR_INVALID_CONFIGID;
    }
    return PduRConfig->PduRConfigurationId;
}

/* @req PDUR615 */
void PduR_EnableRouting(PduR_RoutingPathGroupIdType id) {
    /* @req PDUR119 */
    if( PDUR_UNINIT == PduRState ) {
        PDUR_DET_REPORTERROR(PDUR_MODULE_ID, PDUR_INSTANCE_ID, PDUR_SERVICEID_ENABLEROUTING, PDUR_E_INVALID_REQUEST);
        /*lint -e{904} Return statement is necessary in case of reporting a DET error */
        return;
    }

    if( id < PduRConfig->NofRoutingPathGroups ) {
        /* @req PDUR715 */
#if (PDUR_MAX_NOF_ROUTING_PATH_GROUPS > 0)
        PdurSetRoutingPathEnabled(id, TRUE);
#endif
    } else {
        /* @req PDUR647 */
        /* @req PDUR648 */
        PDUR_DET_REPORTERROR(PDUR_MODULE_ID, PDUR_INSTANCE_ID, PDUR_SERVICEID_ENABLEROUTING, PDUR_E_ROUTING_PATH_GROUP_ID_INVALID);
    }
}

/* @req PDUR617 */
void PduR_DisableRouting(PduR_RoutingPathGroupIdType id) {
    /* @req PDUR119 */
    /* !req PDUR646 *//* Only transmit request disabled. */
    if( PDUR_UNINIT == PduRState ) {
        PDUR_DET_REPORTERROR(PDUR_MODULE_ID, PDUR_INSTANCE_ID, PDUR_SERVICEID_DISABLEROUTING, PDUR_E_INVALID_REQUEST);
        /*lint -e{904} Return statement is necessary in case of reporting a DET error */
        return;
    }

    if(id < PduRConfig->NofRoutingPathGroups) {
#if (PDUR_MAX_NOF_ROUTING_PATH_GROUPS > 0)
        PdurSetRoutingPathEnabled(id, FALSE);
        /* Clear all buffers */
        const PduR_RoutingPathGroupType *groupPtr = &PduRConfig->RoutingPathGroups[id];
        const PduRDestPdu_type *destPtr;
        for( uint32 i = 0; i < groupPtr->NofDestinations; i++ ) {
            destPtr = groupPtr->Destinations[i];
            /* !req PDUR663 *//* TP buffers not cleared */
            if( (PDUR_TRIGGER_TRANSMIT == destPtr->DataProvision) && (PDUR_NO_BUFFER != destPtr->TxBufferId) ) {
                memset(PduR_RamBufCfg.TxBuffers[destPtr->TxBufferId].DataPtr, 0, PduR_RamBufCfg.TxBuffers[destPtr->TxBufferId].BufSize);
            }
        }
#endif
    } else {
        /* @req PDUR716 */
        /* @req PDUR649 */
        PDUR_DET_REPORTERROR(PDUR_MODULE_ID, PDUR_INSTANCE_ID, PDUR_SERVICEID_DISABLEROUTING, PDUR_E_ROUTING_PATH_GROUP_ID_INVALID);
    }
}


#endif
