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

#include "arc_assert.h"
#include <string.h>

/* @req 4.0.3/IPDUM148 */
#include "IpduM.h"
#include "IpduM_Cbk.h"
#include "PduR_IpduM.h"
#include "IpduM_Cfg.h"
#include "Det.h"
/*#include "Com.h" */
#include "Cpu.h"
#include "SchM_IpduM.h"

static IpduM_InitState ipdum_state = IPDUM_UNINIT;
static const IpduM_ConfigType *IpduM_Config;

/*lint -emacro(904,VALIDATE_PARAM_POINTER_NO_RV,VALIDATE_NO_RV,VALIDATE_INIT,VALIDATE_PARAM_POINTER,VALIDATE_TX_PART_ID,VALIDATE_TX_PATHWAY_ID)*/ /*904 PC-Lint exception to MISRA 14.7 (validate DET macros)*/


#if (IPDUM_DEV_ERROR_DETECT == STD_ON ) /* @req 4.0.3/IPDUM028*/ /* @req 4.0.3/IPDUM026 */
#define VALIDATE(_exp,_api,_err) \
    if (!(_exp)) { \
        (void)Det_ReportError(IPDUM_MODULE_ID, 0, _api, _err); \
        return E_NOT_OK; \
    }

#define VALIDATE_NO_RV(_exp,_api,_err ) \
    if( !(_exp) ) { \
        (void)Det_ReportError(IPDUM_MODULE_ID, 0, _api, _err); \
        return; \
    }

#define VALIDATE_NO_RETURN(_exp,_api,_err ) \
    if( !(_exp) ) { \
        (void)Det_ReportError(IPDUM_MODULE_ID, 0, _api, _err); \
    }


#define VALIDATE_INIT(_api)								VALIDATE(ipdum_state == IPDUM_INIT, _api, IPDUM_E_UNINIT) /* @req 4.0.3/IPDUM153 */
#define VALIDATE_INIT_NO_RV(_api)						VALIDATE_NO_RV(ipdum_state == IPDUM_INIT, _api, IPDUM_E_UNINIT) /* @req 4.0.3/IPDUM153 */
#define VALIDATE_PARAM_POINTER(_api, _pointer) 			VALIDATE(_pointer != NULL, _api, IPDUM_E_PARAM_POINTER) /* @req 4.0.3/IPDUM162 */
#define VALIDATE_PARAM_POINTER_NO_RV(_api, _pointer)	VALIDATE_NO_RV(_pointer != NULL, _api, IPDUM_E_PARAM_POINTER) /* @req 4.0.3/IPDUM162 */
#define VALIDATE_RX_ID(_api, _id)						VALIDATE(_id < IPDUM_N_RX_PATHWAYS, _api, IPDUM_E_PARAM) /* @req 4.0.3/IPDUM026 */
#define VALIDATE_RX_ID_NO_RV(_api, _id)					VALIDATE_NO_RV(_id < IPDUM_N_RX_PATHWAYS, _api, IPDUM_E_PARAM) /* @req 4.0.3/IPDUM026 */
#if IPDUM_N_TX_PATHWAYS > 0
#define VALIDATE_TX_PART_ID(_api, _id)					VALIDATE(_id < IPDUM_N_TX_PARTS, _api, IPDUM_E_PARAM) /* @req 4.0.3/IPDUM026 */
#define VALIDATE_TX_PATHWAY_ID(_api, _id)				VALIDATE(_id < IPDUM_N_TX_PATHWAYS, _api, IPDUM_E_PARAM) /* @req 4.0.3/IPDUM026 */
#define VALIDATE_TX_PATHWAY_ID_NO_RV(_api, _id)			VALIDATE_NO_RV(_id < IPDUM_N_TX_PATHWAYS, _api, IPDUM_E_PARAM) /* @req 4.0.3/IPDUM026 */
#else
#define VALIDATE_TX_PART_ID(_api, _id)
#define VALIDATE_TX_PATHWAY_ID(_api, _id)
#define VALIDATE_TX_PATHWAY_ID_NO_RV(_api, _id)
#endif

#define DET_REPORTERROR(_x,_y,_z,_q) Det_ReportError(_x, _y, _z, _q)

#else /* @req 4.0.3/IPDUM027 */
#define VALIDATE(_exp,_api,_err )
#define VALIDATE_NO_RV(_exp,_api,_err )
#define VALIDATE_NO_RETURN(_exp,_api,_err )
#define VALIDATE_INIT(_api)
#define VALIDATE_INIT_NO_RV(_api)
#define VALIDATE_PARAM_POINTER(_api, _pointer)
#define VALIDATE_PARAM_POINTER_NO_RV(_api, _pointer)
#define VALIDATE_RX_ID(_api, _id)
#define VALIDATE_RX_ID_NO_RV(_api, _id)
#define VALIDATE_TX_PART_ID(_api, _id)
#define VALIDATE_TX_PATHWAY_ID(_api, _id)
#define VALIDATE_TX_PATHWAY_ID_NO_RV(_api, _id)
#define DET_REPORTERROR(_x,_y,_z,_q)
#endif

#define GETBITS(source, pos, mask)		(( *( (uint8 *)source  + (pos / 8) ) >> (pos % 8)) & mask)

#define HAS_STATIC_PART(_pathway)		(_pathway->staticPart != NULL)

static void CopySegments(const IpduM_Segment *segments, uint8 nSegments, uint8 *dest, const uint8 *source) {
    uint8 byteNr;
    uint8 mask;
    SchM_Enter_IpduM_EA_0();

    for (uint16 i = 0; i < nSegments; i++) {
        const IpduM_Segment *segment = &(segments[i]);
        byteNr = 0;
        mask = 0;
        while (byteNr < segment->maskLength) {
            mask = segment->mask[byteNr];
            uint8 dest1 = dest[byteNr + segment->startByte];
            uint8 source1 = source[byteNr + segment->startByte];
            dest[byteNr + segment->startByte] = (source1 & mask) | (dest1 & ~mask);
            byteNr++;
        }
    }

    SchM_Exit_IpduM_EA_0();
}


static Std_ReturnType FetchTxPart(const IpduM_TxPart *txPart) {

    Std_ReturnType ret;

    PduInfoType pduInfo = {
        .SduDataPtr = IpduM_Config->txBuffer,
        .SduLength  = txPart->txPathway->pduSize
    };

    SchM_Enter_IpduM_EA_0();

    ret = PduR_IpduMTriggerTransmit(txPart->pduId, &pduInfo);

    CopySegments(txPart->segments, txPart->nSegments, txPart->txPathway->buffer, pduInfo.SduDataPtr);

    SchM_Exit_IpduM_EA_0();

    return ret;

}


/* @req 4.0.3/IPDUM033 */
/* @req 4.0.3/IPDUM083 */
void IpduM_Init(const IpduM_ConfigType *config) {
    /* Com_Init() and PduR_Init() has to be called before IpduM_Init(), see chapter 7.3 of IpduM SRS. */
    VALIDATE_PARAM_POINTER_NO_RV(IPDUM_API_INIT, config);
    IpduM_Config = config;

    for (uint16 i = 0; i < IPDUM_N_TX_PATHWAYS; i++) {
        const IpduM_TxPathway *txPathway = &(IpduM_Config->txPathways[i]);
        /* @req 4.0.3/IPDUM067 */
        memset(txPathway->buffer, txPathway->IpduMIPduUnusedAreasDefault, txPathway->pduSize);
        /* @req 4.0.3/IPDUM068 */
        /* Return value ignored because all PDUs are stopped at initialization, so
         * the PduR_IpduMTriggerTransmit will return an error. (The data is however copied) */
        /*lint -e{534} MISRA:STANDARDIZED_INTERFACE::[MISRA 2012 Rule 17.7, required] */
        FetchTxPart(*(txPathway->activeDynamicPart));

        if (HAS_STATIC_PART(txPathway)) {
            /* @req 4.0.3/IPDUM143 */
            /* Return value ignored because all PDUs are stopped at initialization, so
             * the PduR_IpduMTriggerTransmit will return an error. (The data is however copied) */
            /*lint -e{534} MISRA:STANDARDIZED_INTERFACE::[MISRA 2012 Rule 17.7, required] */
        	FetchTxPart(txPathway->staticPart);
        }
    }

    ipdum_state = IPDUM_INIT;
}

/* @req 4.0.3/IPDUM037 */
/* @req 4.0.3/IPDUM038 */
#if IPDUM_VERSION_INFO_API == STD_ON /* @req 4.0.3/IPDUM039 */
void IpduM_GetVersionInfo( Std_VersionInfoType* versioninfo ){
    VALIDATE_NO_RV((NULL != versioninfo),IPDUM_API_GET_VERSION_INFO,IPDUM_E_PARAM_POINTER);

    versioninfo->moduleID = IPDUM_MODULE_ID;  /* Module ID of IPDUM */
    versioninfo->vendorID = IPDUM_VENDOR_ID;  /* Vendor Id (ARCCORE) */

    /* return the Software Version numbers*/
    versioninfo->sw_major_version = IPDUM_SW_MAJOR_VERSION;
    versioninfo->sw_minor_version = IPDUM_SW_MINOR_VERSION;
    versioninfo->sw_patch_version = IPDUM_SW_PATCH_VERSION;
}
#endif


static boolean shouldTransmit(const IpduM_TxPart *txPart) {
    const IpduM_TxPathway *txPathway = txPart->txPathway;
    return ((txPathway->trigger == IPDUM_STATIC_OR_DYNAMIC_PART_TRIGGER)
        ||  ((txPathway->trigger == IPDUM_STATIC_PART_TRIGGER) && (txPart->type == IPDUM_STATIC))
        ||  ((txPathway->trigger == IPDUM_DYNAMIC_PART_TRIGGER) && (txPart->type == IPDUM_DYNAMIC)));
}

/* @req 4.0.3/IPDUM015 */
/* @req 4.0.3/IPDUM017 */
/* @req 4.0.3/IPDUM021 */
/* @req 4.0.3/IPDUM019 */
/* @req 4.0.3/IPDUM020 */
/* @req 4.0.3/IPDUM152 */
/* @req 4.0.3/IPDUM168 */
/* @req 4.0.3/IPDUM023 */
Std_ReturnType IpduM_Transmit(PduIdType IpduMTxPduId, const PduInfoType *PduInfoPtr) {
    /* Is called with a COM IPdu ID */
    Std_ReturnType status;
    Std_ReturnType ret;
    VALIDATE_INIT(IPDUM_API_TRANSMIT);
    VALIDATE_PARAM_POINTER(IPDUM_API_TRANSMIT, PduInfoPtr);
    VALIDATE_TX_PART_ID(IPDUM_API_TRANSMIT, IpduMTxPduId);
    const IpduM_TxPart *txPart = &(IpduM_Config->txParts[IpduMTxPduId]);
    const IpduM_TxPathway *txPathway = txPart->txPathway;

    /* Do not transmit if we are waiting for a tx confirmation */
    if ((txPathway->IpduMTxConfirmationTimeout > 0) && (*(txPathway->timeoutCounter) > 0)) {
        ret = E_NOT_OK;
    } else {

        /* Copy the part that we received. */
        CopySegments(txPart->segments, txPart->nSegments, txPart->txPathway->buffer, PduInfoPtr->SduDataPtr);

        /* If this is a dynamic part, set to the current active dynamic part. */
        if (txPart->type == IPDUM_DYNAMIC) {
            (*(txPathway->activeDynamicPart)) = txPart;
        }

        /* Should we trigger a transmit? */

        if (FALSE == shouldTransmit(txPart)) {
            ret = E_OK;
        } else {

            /* If this is the static part, then update the dynamic part if it has JIT. */

            if ((txPart->type == IPDUM_STATIC) && (TRUE == ((*(txPathway->activeDynamicPart))->IpduMJitUpdate))) {
                status = FetchTxPart(*(txPathway->activeDynamicPart));
                /* Need to do something with return value - seems appropriate to set DET error */
                VALIDATE_NO_RETURN(status == E_OK, IPDUM_API_TRANSMIT, IPDUM_E_PARAM);
            }

            /* If this is the dynamic part, then update the static part if it has JIT. */
            if ((txPart->type == IPDUM_DYNAMIC) && (HAS_STATIC_PART(txPathway)) && (TRUE == (txPathway->staticPart->IpduMJitUpdate))) {
                status = FetchTxPart(txPathway->staticPart);
                /* Need to do something with return value - seems appropriate to set DET error */
                VALIDATE_NO_RETURN(status == E_OK, IPDUM_API_TRANSMIT, IPDUM_E_PARAM);
            }

            /* Start timeout for TX-confirmation observation */
            if (txPathway->IpduMTxConfirmationTimeout > 0) {
                *(txPathway->timeoutCounter) = txPathway->IpduMTxConfirmationTimeout;
            }

            /* Trigger the transmit */
            PduInfoType pduInfo = {
                .SduLength 		= PduInfoPtr->SduLength,
                .SduDataPtr		= txPathway->buffer
            };
            ret = PduR_IpduMTransmit(txPathway->IpduMOutgoingPduId, &pduInfo);
        }
    }
    return ret;
}


/* @req 4.0.3/IPDUM041 */
/* @req 4.0.3/IPDUM042 */
/* @req 4.0.3/IPDUM086 */
/*lint -e{818} Pointer parameter 'PduInfoPtr' could be declared as pointing to const, this is updated in  version 4.2.2 */
void IpduM_RxIndication(PduIdType RxPduId, PduInfoType *PduInfoPtr) {

    uint8 selectBytePos;
    VALIDATE_INIT_NO_RV(IPDUM_API_RX_INDICATION);
    VALIDATE_PARAM_POINTER_NO_RV(IPDUM_API_RX_INDICATION, PduInfoPtr);
    VALIDATE_RX_ID_NO_RV(IPDUM_API_RX_INDICATION, RxPduId);
    if (PduInfoPtr->SduLength == 0) {
        /*lint -e{904} Return statement is necessary to avoid multiple if loops and hence increase readability in case of argument check */
        return; /* Silently ignore pdus of size 0. See chapter 7.5 of IpduM specification. */
    }
    const IpduM_RxPathway *rxPath = &(IpduM_Config->rxPathways[RxPduId]);

    PduInfoType pduInfo = {
        .SduLength 		= PduInfoPtr->SduLength,
        .SduDataPtr		= rxPath->buffer
    };

    if (HAS_STATIC_PART(rxPath)) {
        memset(rxPath->buffer, 0, rxPath->pduSize);
        CopySegments(rxPath->staticPart->segments, rxPath->staticPart->nSegments, rxPath->buffer, PduInfoPtr->SduDataPtr);
        PduR_IpduMRxIndication(rxPath->staticPart->IpduMOutgoingPduId, &pduInfo);
    }
    const uint8 * SelbytePos;
    uint8 selector,bitPos;
    SelbytePos = &(PduInfoPtr->SduDataPtr[(rxPath->IpduMSelectorFieldPosition/8)]);
    bitPos = (rxPath->IpduMSelectorFieldPosition%8);
    selector = 0;
    if (rxPath->IpduMSelectorMasklength == 2) {
        if (IPDUM_LITTLE_ENDIAN == rxPath->IpduMRxIndicationEndianess) {
            selector = (*SelbytePos & rxPath->IpduMSelectorMask[0]) >> bitPos;
            SelbytePos++;
            selectBytePos = (*SelbytePos & rxPath->IpduMSelectorMask[1]);
            /*lint -e{701} selectBytePos is unsigned quantity and 8-bitPos is designed such that it always gives the positive value.Hence lint exception is made */
            selector |=  (uint8)(selectBytePos << (8u-bitPos));
        } else {
            selector = (*SelbytePos & rxPath->IpduMSelectorMask[1]) >> bitPos;
            SelbytePos--;
            selectBytePos = (*SelbytePos & rxPath->IpduMSelectorMask[0]);
            /*lint -e{701} selectBytePos1 is unsigned quantity and 8-bitPos is designed such that it always gives the positive value.Hence lint exception is made */
            selector |= (uint8)(selectBytePos << (8u-bitPos));
        }
    } else {
            selector = (*SelbytePos & rxPath->IpduMSelectorMask[0]) >> bitPos;
    }
    for (uint8 i = 0; i < rxPath->nDynamicParts; i++) {
        const IpduM_RxPart *dynamicPart = rxPath->dynamicParts[i];
        if (dynamicPart->IpduMRxSelectorValue == selector) {
            memset(rxPath->buffer, 0, rxPath->pduSize);
            CopySegments(dynamicPart->segments, dynamicPart->nSegments, rxPath->buffer, PduInfoPtr->SduDataPtr);
            PduR_IpduMRxIndication(dynamicPart->IpduMOutgoingPduId, &pduInfo);
            break;
        }
    }
}

/* @req 4.0.3/IPDUM088 */
/* @req 4.0.3/IPDUM087 */
/* @req 4.0.3/IPDUM022 */
/* @req 4.0.3/IPDUM024 */
void IpduM_TxConfirmation(PduIdType TxPduId) {
    VALIDATE_INIT_NO_RV(IPDUM_API_TX_CONFIRMATION);
    VALIDATE_TX_PATHWAY_ID_NO_RV(IPDUM_API_TX_CONFIRMATION, TxPduId);
    const IpduM_TxPathway *txPathway = &(IpduM_Config->txPathways[TxPduId]);

    /* Ignore unexpected tx confirmations */
    if ((txPathway->IpduMTxConfirmationTimeout > 0) && (*(txPathway->timeoutCounter) == 0)) {
        /*lint -e{904} Return statement is necessary to avoid multiple if loops and hence increase readability in case of argument check */
        return;
    }
    if (TRUE == (*(txPathway->activeDynamicPart))->useConfirmation) {
        PduR_IpduMTxConfirmation((*(txPathway->activeDynamicPart))->pduId);
    }
    if ((HAS_STATIC_PART(txPathway)) && (TRUE == txPathway->staticPart->useConfirmation)) {
        PduR_IpduMTxConfirmation(txPathway->staticPart->pduId);
    }

    /* Rest tx confirmation timeout */
    if (txPathway->IpduMTxConfirmationTimeout > 0) {
        SchM_Enter_IpduM_EA_0();
        *(txPathway->timeoutCounter) = 0;
        SchM_Exit_IpduM_EA_0();
    }
}

/* @req 4.0.3/IPDUM090 */
/* @req 4.0.3/IPDUM091 */
/* @req 4.0.3/IPDUM089 */
/* @req 4.0.3/IPDUM169 */
Std_ReturnType IpduM_TriggerTransmit(PduIdType TxPduId, PduInfoType *PduInfoPtr) {
    Std_ReturnType status;
    VALIDATE_INIT(IPDUM_API_TRIGGER_TRANSMIT);
    VALIDATE_PARAM_POINTER(IPDUM_API_TRIGGER_TRANSMIT, PduInfoPtr);
    VALIDATE_TX_PATHWAY_ID(IPDUM_API_TRIGGER_TRANSMIT, TxPduId);
    const IpduM_TxPathway *txPathway = &(IpduM_Config->txPathways[TxPduId]);

    SchM_Enter_IpduM_EA_0();
    if (TRUE == (*(txPathway->activeDynamicPart))->IpduMJitUpdate) {
        status = FetchTxPart(*(txPathway->activeDynamicPart));
        /* Need to do something with return value - seems appropiate to set DET error */
        VALIDATE_NO_RETURN(status == E_OK, IPDUM_API_TRIGGER_TRANSMIT, IPDUM_E_PARAM);
    }
    if ((HAS_STATIC_PART(txPathway)) && (TRUE == (txPathway->staticPart->IpduMJitUpdate))){
        status = FetchTxPart(txPathway->staticPart);
        /* Need to do something with return value - seems appropiate to set DET error */
        VALIDATE_NO_RETURN(status == E_OK, IPDUM_API_TRIGGER_TRANSMIT, IPDUM_E_PARAM);
    }

    memcpy(PduInfoPtr->SduDataPtr, txPathway->buffer, txPathway->pduSize);
    SchM_Exit_IpduM_EA_0();
    PduInfoPtr->SduLength = txPathway->pduSize;

    return E_OK;
}

/* @req 4.0.3/IPDUM101 */
void IpduM_MainFunction(void) {
    VALIDATE_INIT_NO_RV(IPDUM_API_MAIN);

    for (uint16 i = 0; i < IPDUM_N_TX_PATHWAYS; i++) {
        const IpduM_TxPathway *txPathway = &(IpduM_Config->txPathways[i]);

        SchM_Enter_IpduM_EA_0();
        if ((txPathway->IpduMTxConfirmationTimeout > 0) && (*(txPathway->timeoutCounter) > 0)) {
            (*(txPathway->timeoutCounter))--;
        }
        SchM_Exit_IpduM_EA_0();
    }
}

