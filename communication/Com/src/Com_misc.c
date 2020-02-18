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

#include <string.h>
#include "arc_assert.h"

#include "Com_Arc_Types.h"
#include "Com.h"
#include "Com_Internal.h"
#include "Com_misc.h"
#include "debug.h"
#include "Cpu.h"
#include "SchM_Com.h"

#if defined(USE_LINOS)
#include "linos_logger.h" /* Logger functions */
#endif

/* General tagging  */
/* @req COM221 */
/* @req COM353 */
/* @req COM007 *//* Endianness conversion of integer types */
/* @req COM008 *//* Sign extension */
/* @req COM674 *//* Endianness conversion of signed data types */
/* @req COM723 *//* The AUTOSAR COM module shall extend the init value (ComSignalInitValue) of a signal to the size of its ComSignalType. */


/*lint -esym(9003, ComNotificationCallouts)*/ /* Defined in Com_Cfg.c */
extern const ComNotificationCalloutType ComNotificationCallouts[];
/*lint -esym(9003, ComTxIPduCallouts)*/ /* Defined in Com_Cfg.c */
extern const ComTxIPduCalloutType ComTxIPduCallouts[];

#if(COM_IPDU_COUNTING_ENABLE ==  STD_ON )
/* Com IPdu counters
 * Next counter value to be transmitted or received */
#define COM_START_SEC_VAR_CLEARED_UNSPECIFIED
#include "Com_MemMap.h"  /*lint !e9019 OTHER [MISRA 2012 Rule 20.1, advisory] OTHER AUTOSAR specified way of using MemMap*/
static boolean ComIPduCountRxStart[COM_MAX_N_SUPPORTED_IPDU_COUNTERS];
#define COM_STOP_SEC_VAR_CLEARED_UNSPECIFIED
#include "Com_MemMap.h"  /*lint !e9019 OTHER [MISRA 2012 Rule 20.1, advisory] OTHER AUTOSAR specified way of using MemMap*/

#define COM_START_SEC_VAR_CLEARED_UNSPECIFIED
#include "Com_MemMap.h"  /*lint !e9019 OTHER [MISRA 2012 Rule 20.1, advisory] OTHER AUTOSAR specified way of using MemMap*/
static uint8 Com_IPdu_Counters[COM_MAX_N_SUPPORTED_IPDU_COUNTERS];
#define COM_STOP_SEC_VAR_CLEARED_UNSPECIFIED
#include "Com_MemMap.h"  /*lint !e9019 OTHER [MISRA 2012 Rule 20.1, advisory] OTHER AUTOSAR specified way of using MemMap*/

#if defined (HOST_TEST)
uint8 ReadInternalCouner(uint16 indx){
    return Com_IPdu_Counters[indx];
}
#endif

/* Internal functions used to access IPdu counters*/
void ResetInternalCounter(uint16 indx) {
    Com_IPdu_Counters[indx] = 0;
}

void ReSetInternalRxStartSts(uint16 indx) {
    ComIPduCountRxStart[indx] = FALSE;
}

void ResetInternalCounterRxStartSts(void){

    for (uint16 i = 0; i < COM_MAX_N_SUPPORTED_IPDU_COUNTERS ; i++){
        Com_IPdu_Counters[i] = 0;
        ComIPduCountRxStart[i] = FALSE;
    }
}
static uint8 updateIPduCntr(const ComIPduCounter_type *CounterCfgRef, uint8 * sduPtr, uint16 * cntrIdx){

    uint16 bytPos, bitPos, startPos, bitMask, cntrRng;
    uint8 cntrVal, val;

    startPos= CounterCfgRef->ComIPduCntrStartPos ;
    bytPos  = startPos / 8 ;
    bitPos  = startPos % 8 ;
    cntrRng = CounterCfgRef->ComIPduCntrRange ;
    /*lint -e{734} bit-pos is limited to value 7, result will not loose precision */
    bitMask = ~((cntrRng - 1u) << bitPos) ;

    *cntrIdx= CounterCfgRef->ComIPduCntrIndex ; /* Find the appropriate counter index */
    /*lint -e{734} cntrRng can have range value 256 hence declared as uint16*/
    cntrVal = (Com_IPdu_Counters[*cntrIdx] + 1) % (cntrRng); /* calculate updated counter value */

    /* update the counter value in the IPdu */
    val     = (sduPtr[bytPos]) ;
    /*lint -e{734} bit-pos is limited to value 7, result will not loose precision */
    /*lint -e{701} cntrVal is unsigned*/
    (sduPtr[bytPos]) = (val & (uint8) bitMask) | (cntrVal << bitPos) ;

    return cntrVal;
}

static uint8 extractIPduCntr(const ComIPduCounter_type *CounterCfgRef, const uint8 * sduPtr){

    uint16 bytPos, bitPos, startPos,cntrRng;
    uint8 val, bitMask;

    startPos= CounterCfgRef->ComIPduCntrStartPos ;
    bytPos  = startPos / 8 ;
    bitPos  = startPos % 8 ;
    cntrRng = (CounterCfgRef->ComIPduCntrRange -1);
    bitMask = (uint8)(cntrRng) ;

    /* Extract IPdu counter from appropriate byte positions */
    val = (sduPtr[bytPos]) ;
    val = (val >> bitPos) & bitMask;
    return val;
}

boolean Com_Misc_validateIPduCounter(const ComIPduCounter_type *CounterCfgRef, const uint8 * sduPtr) {
    uint16 cntrIdx,cntrRng;
    uint8 rxCntrVal,diff;
    boolean ret = FALSE;

    cntrIdx = CounterCfgRef->ComIPduCntrIndex ;
    cntrRng = CounterCfgRef->ComIPduCntrRange ;

    if (TRUE == ComIPduCountRxStart[cntrIdx]){

        rxCntrVal = extractIPduCntr(CounterCfgRef, sduPtr);

        /*lint -e{734} cntrRng can have range value 256 hence declared as uint16*/
        diff = (rxCntrVal >= Com_IPdu_Counters[cntrIdx]) ? (rxCntrVal - Com_IPdu_Counters[cntrIdx]): (rxCntrVal + cntrRng - Com_IPdu_Counters[cntrIdx]);

        /* @req COM590 */
        if (diff <= CounterCfgRef->ComIPduCntrThrshldStep){
            ret = TRUE; /*Accept IPdu only when criterion is fulfilled */
        }

    } else{

        rxCntrVal = extractIPduCntr(CounterCfgRef, sduPtr);
        ComIPduCountRxStart[cntrIdx] = TRUE;
        /* @req COM587 */
        ret = TRUE;
    }

    /* @req COM588 */
    /*lint -e{734} cntrRng can have range value 256 hence declared as uint16*/
    Com_IPdu_Counters[cntrIdx] = (rxCntrVal + 1) % cntrRng ;/* Set the next expected value*/

    return ret;
}
#endif

/**
 * Routine to handle new repetitions based on Transfer property if some transmit repetitions are already in progress
 * correspondingly tx deadline counter is embedded with repetitions
 * @param iPdu
 * @param arcIPdu
 * @param dataChanged
 * @param transferProperty
 * @return TRUE: Operation sucessful, FALSE: operation failed
 */
boolean Com_Misc_TriggerTxOnConditions(uint16 pduHandleId, boolean dataChanged, ComTransferPropertyType transferProperty)
{
    const ComIPdu_type *IPdu = GET_IPdu(pduHandleId);
    Com_Arc_IPdu_type *Arc_IPdu = GET_ArcIPdu(pduHandleId);
    uint8 nofReps = 0;
    boolean ret = FALSE;
    boolean triggeredNow = FALSE;
    const ComTxMode_type *txModePtr;

    if( TRUE == Arc_IPdu->Com_Arc_IpduStarted ) {
        if( TRUE == Arc_IPdu->Com_Arc_IpduTxMode ) {
            txModePtr = &IPdu->ComTxIPdu.ComTxModeTrue;
        }
        else {
            txModePtr = &IPdu->ComTxIPdu.ComTxModeFalse;
        }
        /* If signal has triggered transmit property, trigger a transmission!*/
        /** Signal Requirements */
        /* @req COM767 */
        /* @req COM734 *//* NOTE: The actual sending is done in the main function */
        /* @req COM768 */
        /* @req COM762 *//* Signal with ComBitSize 0 should never be detected as changed */
        /** Signal Group Requirements */
        /* @req COM769 */
        /* @req COM742 */
        /* !req COM743 */
        /* @req COM770 */
        if ( (COM_TRIGGERED == transferProperty) || ( COM_TRIGGERED_WITHOUT_REPETITION == transferProperty ) ||
             (((COM_TRIGGERED_ON_CHANGE == transferProperty) ||
               (COM_TRIGGERED_ON_CHANGE_WITHOUT_REPETITION == transferProperty )) && (TRUE == dataChanged))) {

            /* @req COM305.1 */
            /* @req COM279 */
            /** Signal Requirements */
            /* @req COM330 */
            /* @req COM467 */ /* Though RetryFailedTransmitRequests not supported. */
            /** Signal Group Requirements */
            /* @req COM741 */

            switch(transferProperty) {
                case COM_TRIGGERED:
                case COM_TRIGGERED_ON_CHANGE:
                    if( 0 == txModePtr->ComTxModeNumberOfRepetitions ) {
                        nofReps = 1;
                    } else {
                        nofReps = txModePtr->ComTxModeNumberOfRepetitions;
                    }
                    break;
                case COM_TRIGGERED_WITHOUT_REPETITION:
                case COM_TRIGGERED_ON_CHANGE_WITHOUT_REPETITION:
                    nofReps = 1;
                    break;
                default:
                    break;
            }
            ComTxModeModeType txMode = txModePtr->ComTxModeMode;
            ComTxTriggerStatusType txTrigSts;
            if ((COM_DIRECT == txMode) || (COM_MIXED == txMode)) {

                if (nofReps > 0) {
                    /* @req COM625 */
                    /* @req COM701 *//* Routing is independent of DM. A new Transmission cycle is started for GW routing request */
                    txTrigSts = Com_Misc_TriggerIPDUSend(pduHandleId);

                    if(COM_TX_TRIGGERED ==  txTrigSts) {
                        /* Transmission triggered */
                        triggeredNow = TRUE;
                        Arc_IPdu->Com_Arc_TxIPduTimers.ComTxModeRepetitionPeriodTimer = txModePtr->ComTxModeRepetitionPeriodFactor;
                        nofReps--;
                    } else if (COM_TX_NOT_TRIGGERED == txTrigSts) {
                        /* Transmission was triggered but failed.*/
                        nofReps--;
                        Arc_IPdu->Com_Arc_TxIPduTimers.ComTxModeRepetitionPeriodTimer = txModePtr->ComTxModeRepetitionPeriodFactor;
                    } else if (COM_TX_MIN_DELAY_NOT_FULFILLED == txTrigSts) {
                        /* Transmission was delayed because Delay timer did not expire */
                        Arc_IPdu->Com_Arc_TxIPduTimers.ComTxModeRepetitionPeriodTimer =0;/*This triggers an IPDU immediately after delay timer expires*/
                    } else {
                        /*else created to avoid lint error*/
                    }
                }

                /* @req COM739 */ /* All new Tx Requests results in resetting DM timer */
                if( Arc_IPdu->Com_Arc_TxIPduTimers.ComTxIPduNumberOfRepetitionsLeft <= nofReps ) {
                    /* All outstanding Transmission requests are kept as it is, if greater than current nofReps
                     * Probable case when first signal with transfer property TRIGGERED starts a cycle with n periodic transmissions
                     * and subsequently signal with transfer property TRIGGERED_WITHOUT_REPITION is requested for send. Total no. of Tx = n.*/
                    Arc_IPdu->Com_Arc_TxIPduTimers.ComTxIPduNumberOfRepetitionsLeft = nofReps;
                    /* * DM RESTART with this new request * */
                    if(0 < Arc_IPdu->Com_Arc_TxDeadlineCounter){
                        Arc_IPdu->Com_Arc_TxIPduTimers.ComTxIPduNumberOfTxConfirmations = nofReps + ((TRUE == triggeredNow) ? 1 : 0);
                        Arc_IPdu->Com_Arc_TxIPduTimers.ComTxDMTimer = Arc_IPdu->Com_Arc_TxDeadlineCounter;
                    }
                } else if((TRUE == triggeredNow) && (0 != Arc_IPdu->Com_Arc_TxIPduTimers.ComTxIPduNumberOfRepetitionsLeft)) {
                        Arc_IPdu->Com_Arc_TxIPduTimers.ComTxIPduNumberOfRepetitionsLeft--;
                } else {
                    /*else created to avoid lint error*/
                }
            }
        }
        ret = TRUE;
    }

    return ret;
}

void Com_Misc_CopySignalGroupDataFromShadowBufferToPdu(const Com_SignalIdType signalGroupId, boolean deferredBufferDestination, boolean *dataChanged) {

    /* Get PDU*/
    const ComSignal_type * Signal = GET_Signal(signalGroupId);
    const ComIPdu_type *IPdu = GET_IPdu(Signal->ComIPduHandleId);
    const Com_Arc_Signal_type * Arc_Signal = GET_ArcSignal(Signal->ComHandleId);

    uint8 *pduDataPtr = NULL;
    if (TRUE == deferredBufferDestination) {
        pduDataPtr = GET_ArcIPdu(Signal->ComIPduHandleId)->ComIPduDeferredDataPtr;
    } else {
        pduDataPtr = GET_ArcIPdu(Signal->ComIPduHandleId)->ComIPduDataPtr;
    }

    /* Aligned opaque data -> straight copy with signalgroup mask*/
    const uint8 *buf = (const uint8*)Arc_Signal->Com_Arc_ShadowBuffer;
    uint8 data = 0;
    *dataChanged = FALSE;
    for(uint16 i= 0; i < IPdu->ComIPduSize; i++){
        data = (~Signal->Com_Arc_ShadowBuffer_Mask[i] & *pduDataPtr) |
               (Signal->Com_Arc_ShadowBuffer_Mask[i] & *buf);
        if(*pduDataPtr != data) {
            *dataChanged = TRUE;
        }
        *pduDataPtr = data;
        buf++;
        pduDataPtr++;
    }

}


void Com_Misc_CopySignalGroupDataFromPduToShadowBuffer(const Com_SignalIdType signalGroupId) {

    /* Get PDU*/
    const ComSignal_type * Signal = GET_Signal(signalGroupId);
    const ComIPdu_type *IPdu = GET_IPdu(Signal->ComIPduHandleId);

    const uint8 *pduDataPtr = NULL;
    if ((IPdu->ComIPduSignalProcessing == COM_DEFERRED) && (IPdu->ComIPduDirection == COM_RECEIVE)) {
        pduDataPtr = GET_ArcIPdu(Signal->ComIPduHandleId)->ComIPduDeferredDataPtr;
    } else {
        pduDataPtr = GET_ArcIPdu(Signal->ComIPduHandleId)->ComIPduDataPtr;
    }


    if ( (FALSE == Signal->ComSignalArcUseUpdateBit) ||
            /*lint -e{9016} -e{9005} -e{926} Array indexing couldn't be implemented, as parameters are of different data types */
        ( (TRUE == Signal->ComSignalArcUseUpdateBit) && (TESTBIT(pduDataPtr, Signal->ComUpdateBitPosition) > 0) ) ) {

		/* Aligned opaque data -> straight copy with signalgroup mask*/
		/*lint -e{9005} Com_Arc_ShadowBuffer value is required to be modified */
		uint8 *buf = (uint8 *)GET_ArcSignal(Signal->ComHandleId)->Com_Arc_ShadowBuffer;
		for(uint16 i= 0; i < IPdu->ComIPduSize; i++){
			/*lint -e{9049} The post increment operation is required*/
			*buf = (*buf & ~Signal->Com_Arc_ShadowBuffer_Mask[i]) | (Signal->Com_Arc_ShadowBuffer_Mask[i] & *pduDataPtr);
			pduDataPtr++;
			buf++;
		}
    }
}

/**
 * Routine to read a 64 bits signal from a PDU
 * @param comIPduDataPtr: Pointer to IPdu pduBuffer
 * @param bitPosition: Signal bit position
 * @param bitSize: Signal bit size
 * @param endian: Signal endianness
 * @param signalType: Signal type
 * @param SignalData: Pointer where to store resulting signal data
 * @return None
 */
static inline void Com_Misc_ReadSignalDataFromPdu64Bits (const uint8 *comIPduDataPtr,
        Com_BitPositionType bitPosition, uint16 bitSize,
        ComSignalEndianess_type endian, Com_SignalType signalType,
        uint8 *SignalData) {
    /* Covers signed and unsigned 64 bits signal types*/
    /* Unaligned data and/or endianness conversion*/
    uint64 pduData = 0ULL;
    if(endian == COM_BIG_ENDIAN) {
        uint32 lsbIndex = (((bitPosition ^ 0x7u) + bitSize - 1) ^ 7u); /* calculate lsb bit index. This could be moved to generator*/
        const uint8 *pduDataPtr = ((&comIPduDataPtr [lsbIndex / 8])-7); /* calculate big endian ptr to data*/
        uint8 bitShift = lsbIndex % 8;
        for(uint32 i = 0; i < 8; i++) {
            pduData = (pduData << 8) | pduDataPtr[i];
        }
        pduData >>= bitShift;
        if((64 - bitShift) < bitSize) {
            pduData |= (uint64)pduDataPtr[-1] << (64u - bitShift);
        }
    } else if (endian == COM_LITTLE_ENDIAN) {
        uint32 lsbIndex = bitPosition;
        const uint8 *pduDataPtr = &comIPduDataPtr[(bitPosition/8)];
        uint8 bitShift = lsbIndex % 8;
        for(sint32 i = 7; i >= 0; i--) {
            pduData = (pduData << 8) | pduDataPtr[i];
        }
        pduData >>= bitShift;
        if((64 - bitShift) < bitSize) {
            pduData |= (uint64)pduDataPtr[8] << (64u - bitShift);
        }
    } else {
        ASSERT(0);
    }
    SchM_Exit_Com_EA_0();
    uint64 mask = (0xFFFFFFFFFFFFFFFFULL >> (64u - bitSize)); /* calculate mask for SigVal*/
    pduData &= mask; /* clear bit out of range*/
    uint64 signmask = ~(mask >> 1u);
    switch(signalType) {
    case COM_SINT64:
        if(0 < (pduData & signmask)) {
            pduData |= signmask; /* add sign bits*/
        }
        /* sign extended data can be written as uint*/
        /*lint -e{826} -e{927} pointer cast is essential */
        *(uint64*)SignalData = pduData;
        break;
    case COM_UINT64:
        /*lint -e{826} -e{927} pointer cast is essential */
        *(uint64*)SignalData = pduData;
        break;
    default:
        /* This function is for 64 bits values only, even if it in theory could handle also other signal types*/
        // IMPROVEMENT: Report error to DET.
        ASSERT(0);
    }
}

void Com_Misc_ReadSignalDataFromPdu (
        const uint8 *comIPduDataPtr,
        Com_BitPositionType bitPosition,
        uint16 bitSize,
        ComSignalEndianess_type endian,
        Com_SignalType signalType,
        uint8 *SignalData) {
    SchM_Enter_Com_EA_0();
    if ((endian == COM_OPAQUE) || (signalType == COM_UINT8_N)) {
        /* Aligned opaque data -> straight copy*/
        /* @req COM472 */
        memcpy(SignalData, &comIPduDataPtr[bitPosition/8], bitSize / 8);
        SchM_Exit_Com_EA_0();
    } else if ((signalType != COM_UINT64) && (signalType != COM_SINT64)) {
        /* Unaligned data and/or endian-ness conversion*/
        uint32 pduData = 0ULL;
        if(endian == COM_BIG_ENDIAN) {
            uint32 lsbIndex = ((bitPosition ^ 0x7u) + bitSize - 1) ^ 7u; /* calculate lsb bit index. This could be moved to generator*/
            const uint8 *pduDataPtr = ((&comIPduDataPtr [lsbIndex / 8])-3); /* calculate big endian ptr to data*/
            uint8 bitShift = lsbIndex % 8;
            for(uint32 i = 0; i < 4; i++) {
                pduData = (pduData << 8) | pduDataPtr[i];
            }
            pduData >>= bitShift;
            if((32 - bitShift) < bitSize) {
                pduData |= (uint32)pduDataPtr[-1] << (32u - bitShift);
            }
        } else if (endian == COM_LITTLE_ENDIAN) {
            uint32 lsbIndex = bitPosition;
            const uint8 *pduDataPtr = &comIPduDataPtr[(bitPosition/8)];
            uint8 bitShift = lsbIndex % 8;
            for(sint32 i = 3; i >= 0; i--) {
                pduData = (pduData << 8) | pduDataPtr[i];
            }
            pduData >>= bitShift;
            if((32 - bitShift) < bitSize) {
                pduData |= (uint32)pduDataPtr[4] << (32u - bitShift);
            }
        } else {
            ASSERT(0);
        }
        SchM_Exit_Com_EA_0();
        uint32 mask = 0xFFFFFFFFUL >> (32u - bitSize); /* calculate mask for SigVal */
        pduData &= mask; /* clear bit out of range */
        uint32 signmask = ~(mask >> 1u);
        switch(signalType) {
        case COM_SINT8:
            if(0 < (pduData & signmask)) {
                pduData |= signmask; /* add sign bits */
            }
            /* sign extended data can be written as uint*/
            /*lint -e{734} pointer cast and assigning 32bit pduData to 8bit SignalData is required operation*/
            *SignalData = pduData;
            break;
        case COM_BOOLEAN:
            /*lint -e{734} pointer cast and assigning 32bit pduData to 8bit SignalData is required operation*/
            *SignalData = pduData;
            break;
        case COM_UINT8:
            /*lint -e{734} pointer cast and assigning 32bit pduData to 8bit SignalData is required operation*/
            *SignalData = pduData;
            break;
        case COM_SINT16:
            if(0 < (pduData & signmask)) {
                pduData |= signmask; /* add sign bits*/
            }
            /* sign extended data can be written as uint*/
            /*lint -e{927} -e{734} -e{826} pointer cast and assigning 32bit pduData to 16bit SignalData is required operation*/
            *(uint16*)SignalData = pduData;
            break;
        case COM_UINT16:
            /*lint -e{927} -e{734} -e{826} pointer cast and assigning 32bit pduData to 16bit SignalData is required operation*/
            *(uint16*)SignalData = pduData;
            break;
        case COM_SINT32:
            if(0 < (pduData & signmask)) {
                pduData |= signmask; /* add sign bits*/
            }
            /*sign extended data can be written as uint */
            /*lint -e{927} -e{826} pointer cast is required operation*/
            *(uint32*)SignalData = pduData;
            break;
        case COM_UINT32:
            /*lint -e{927} -e{826} pointer cast is required operation*/
            *(uint32*)SignalData = pduData;
            break;
        case COM_UINT8_N:
        case COM_UINT8_DYN:
        case COM_SINT64:
        case COM_UINT64:
            ASSERT(0);
            break;
        default : break;
        }
    } else {
        /* Call separate function for 64bits values.*/
        /* Note: SchM_Exit_Com_EA_0 is called from within called function */
        Com_Misc_ReadSignalDataFromPdu64Bits(comIPduDataPtr, bitPosition,
                bitSize, endian, signalType, SignalData);
    }
}
/**
 * Routine to write a 64 bits signal to a PDU
 * @param SignalDataPtr: Pointer to signal data to write
 * @param signalType: Signal type
 * @param comIPduDataPtr: Pointer to IPdu pduBuffer
 * @param bitPosition: Signal bit position
 * @param bitSize: Signal bit size
 * @param endian: Signal endianness
 * @param dataChanged: Pointer where to write data changed indication
 * @return None
 */
static inline void Com_Misc_WriteSignalDataToPdu64Bits(const uint8 *SignalDataPtr,
        Com_SignalType signalType, uint8 *comIPduDataPtr,
        Com_BitPositionType bitPosition, uint16 bitSize, 
        ComSignalEndianess_type endian, boolean *dataChanged) {
    /* Covers signed and unsigned 64 bits signal types*/
    if ((signalType != COM_UINT64) && (signalType != COM_SINT64)) {
        /* This function is for 64 bits values only, even if it in theory could handle also other signal types*/
        // IMPROVEMENT: Report error to DET.
        ASSERT(0);
    }
    /*lint -e{927} -e{826} pointer cast is required operation*/

    uint64 sigVal = *((const uint64*)SignalDataPtr);
    uint64 mask = 0xFFFFFFFFFFFFFFFFu >> (64u - bitSize); /* calculate mask for SigVal*/
    sigVal &= mask; // mask sigVal;
    SchM_Enter_Com_EA_0();
    if(endian == COM_BIG_ENDIAN) {
        uint32 lsbIndex = ((bitPosition ^ 0x7u) + bitSize - 1) ^ 7u; /* calculate lsb bit index. This could be moved to generator*/
        uint8 *pduDataPtr = ((&comIPduDataPtr [lsbIndex / 8])-7); /* calculate big endian ptr to data*/
        uint64 pduData = 0;
        for(uint32 i = 0; i < 8; i++) {
            pduData = (pduData << 8) | pduDataPtr[i];
        }
        uint8 bitShift = lsbIndex % 8;
        uint64 sigLo = sigVal << bitShift;
        uint64 maskLo = ~(mask  << bitShift);
        uint64 newPduData = (pduData & maskLo) | sigLo;
        *dataChanged = (newPduData != pduData);
        for(sint16 i = 7; i >= 0; i--) {
            pduDataPtr[i] = (uint8)newPduData;
            newPduData >>= 8;
        }
        uint8 maxBitsWritten = 64 - bitShift;
        if(maxBitsWritten < bitSize) {
            pduDataPtr--;
            pduData = *pduDataPtr;
            uint64 maskHi = ~(mask  >> maxBitsWritten);
            uint64 sigHi = sigVal >> maxBitsWritten;
            newPduData = (pduData & maskHi) | sigHi;
            *dataChanged |= (newPduData != pduData) ? TRUE : *dataChanged;
            *pduDataPtr = (uint8)newPduData;
        }
    } else if (endian == COM_LITTLE_ENDIAN) {
        uint32 lsbIndex = bitPosition; /* calculate lsb bit index.*/
        uint8 *pduDataPtr = (&comIPduDataPtr[lsbIndex / 8]); /* calculate big endian ptr to data*/
        uint64 pduData = 0;
        for(sint32 i = 7; i >= 0; i--) {
            pduData = (pduData << 8) | pduDataPtr[i];
        }
        uint8 bitShift = lsbIndex % 8;
        uint64 sigLo = sigVal << bitShift;
        uint64 maskLo = ~(mask  << bitShift);
        uint64 newPduData = (pduData & maskLo) | sigLo;
        *dataChanged = (newPduData != pduData);
        for(uint32 i = 0; i < 8; i++) {
            pduDataPtr[i] = (uint8)newPduData;
            newPduData >>= 8;
        }
        uint8 maxBitsWritten = 64u - bitShift;
        if(maxBitsWritten < bitSize) {
            pduDataPtr = &pduDataPtr[8];
            pduData = *pduDataPtr;
            uint64 maskHi = ~(mask >> maxBitsWritten);
            uint64 sigHi = sigVal >> maxBitsWritten;
            newPduData = (pduData & maskHi) | sigHi;
            *dataChanged = (newPduData != pduData) ? TRUE : *dataChanged;
            *pduDataPtr = (uint8)newPduData;
        }
    } else {
        ASSERT(0);
    }
}

void Com_Misc_WriteSignalDataToPdu(const uint8 *SignalDataPtr, Com_SignalType signalType,
        uint8 *comIPduDataPtr, Com_BitPositionType bitPosition,
        uint16 bitSize, ComSignalEndianess_type endian,
        boolean *dataChanged) {
    if ((endian == COM_OPAQUE) || (signalType == COM_UINT8_N)) {
        /* @req COM472 */
        uint8 *pduBufferBytes = &(comIPduDataPtr[bitPosition / 8]);
        uint16 signalLength = bitSize / 8;
        SchM_Enter_Com_EA_0();
        *dataChanged = ( 0 != memcmp(pduBufferBytes, SignalDataPtr, signalLength) );
        memcpy(pduBufferBytes, SignalDataPtr, signalLength);
#if defined(USE_LINOS)
        logger(LOG_INFO, "Com_Misc_WriteSignalDataToPdu pduBufferBytes                                        [%s]", 
            logger_format_hex(pduBufferBytes, signalLength));
#endif
    } else if ((signalType != COM_UINT64) && (signalType != COM_SINT64)) {
        uint32 sigVal = 0;
        switch(signalType) {
        case COM_BOOLEAN:
        case COM_UINT8:
        case COM_SINT8:
            sigVal = *(SignalDataPtr);
            break;
        case COM_UINT16:
        case COM_SINT16:
            /*lint -e{826} -e{927} uint8* to uint16*  casting is required */
            sigVal = *((const uint16*)SignalDataPtr);
            break;
        case COM_UINT32:
        case COM_SINT32:
            /*lint -e{826} -e{927} uint8* to uint16*  casting is required */
            sigVal = *((const uint32*)SignalDataPtr);
            break;
        case COM_UINT8_N:
        case COM_UINT8_DYN:
        case COM_UINT64:
        case COM_SINT64:
            ASSERT(0);
            break;
        default: break;
        }
        uint32 mask = 0xFFFFFFFFu >> (32u - bitSize); /* calculate mask for SigVal*/
        sigVal &= mask; // mask sigVal;
        SchM_Enter_Com_EA_0();
        if(endian == COM_BIG_ENDIAN) {
            uint32 lsbIndex = ((bitPosition ^ 0x7u) + bitSize - 1) ^ 7u; /* calculate lsb bit index. This could be moved to generator*/
            uint8 *pduDataPtr = ((&comIPduDataPtr [lsbIndex / 8])-3); /* calculate big endian ptr to data*/
            uint32 pduData = 0;
            for(uint32 i = 0; i < 4; i++) {
                pduData = (pduData << 8) | pduDataPtr[i];
            }
            uint8 bitShift = lsbIndex % 8;
            uint32 sigLo = sigVal << bitShift;
            uint32 maskLo = ~(mask  << bitShift);
            uint32 newPduData = (pduData & maskLo) | sigLo;
            *dataChanged = (newPduData != pduData);
            for(sint16 i = 3; i >= 0; i--) {
                pduDataPtr[i] = (uint8)newPduData;
                newPduData >>= 8;
            }
            uint8 maxBitsWritten = 32 - bitShift;
            if(maxBitsWritten < bitSize) {
                pduDataPtr--;
                pduData = *pduDataPtr;
                uint32 maskHi = ~(mask  >> maxBitsWritten);
                uint32 sigHi = sigVal >> maxBitsWritten;
                newPduData = (pduData & maskHi) | sigHi;
                *dataChanged |= (newPduData != pduData) ? TRUE : *dataChanged;
                *pduDataPtr = (uint8)newPduData;
            }
        } else if (endian == COM_LITTLE_ENDIAN) {
            uint32 lsbIndex = bitPosition; /* calculate lsb bit index.*/
            uint8 *pduDataPtr = (&comIPduDataPtr[lsbIndex / 8]); /* calculate big endian ptr to data*/
            uint32 pduData = 0;
            for(sint32 i = 3; i >= 0; i--) {
                pduData = (pduData << 8) | pduDataPtr[i];
            }
            uint8 bitShift = lsbIndex % 8;
            uint32 sigLo = sigVal << bitShift;
            uint32 maskLo = ~(mask  << bitShift);
            uint32 newPduData = (pduData & maskLo) | sigLo;
            *dataChanged = (newPduData != pduData);
            for(uint32 i = 0; i < 4; i++) {
                pduDataPtr[i] = (uint8)newPduData;
                newPduData >>= 8;
            }
            uint8 maxBitsWritten = 32 - bitShift;
            if(maxBitsWritten < bitSize) {
                pduDataPtr = &pduDataPtr[4];
                pduData = *pduDataPtr;
                uint32 maskHi = ~(mask >> maxBitsWritten);
                uint32 sigHi = sigVal >> maxBitsWritten;
                newPduData = (pduData & maskHi) | sigHi;
                *dataChanged = (newPduData != pduData) ? TRUE : *dataChanged;
                *pduDataPtr = (uint8)newPduData;
            }
        } else {
            ASSERT(0);
        }
    } else {
        /* Separate function for 64bits values*/
        /* Note: SchM_Enter_Com_EA_0 is called from within called function*/
        Com_Misc_WriteSignalDataToPdu64Bits(SignalDataPtr, signalType,
                comIPduDataPtr, bitPosition, bitSize, endian, dataChanged);
    }
    SchM_Exit_Com_EA_0();
}


void Com_Misc_RxProcessSignals(const ComIPdu_type *IPdu,const Com_Arc_IPdu_type *Arc_IPdu) {
    /* !req COM053 */
    /* @req COM055 */
    /* !req COM396 */ /* Neither invalidation nor filtering supported */
    /* !req COM352 */
    const ComSignal_type *comSignal;

    for (uint16 i = 0; IPdu->ComIPduSignalRef[i] != NULL; i++) {
        comSignal = IPdu->ComIPduSignalRef[i];
        Com_Arc_Signal_type * Arc_Signal = GET_ArcSignal(comSignal->ComHandleId);

        /* If this signal uses an update bit, then it is only considered if this bit is set.*/
        /* @req COM324 */
        /* @req COM067 */
        /* Eligible for gateway routing if update bit is available and set       */
        /* @req COM702 */
        /* @req COM703 */
        /* @req COM705 */
        if ( (FALSE == comSignal->ComSignalArcUseUpdateBit) ||
                /*lint -e{9016} Array indexing couldn't be implemented, as parameters are of different data types */
            ( (TRUE == comSignal->ComSignalArcUseUpdateBit) && (TESTBIT(Arc_IPdu->ComIPduDataPtr, comSignal->ComUpdateBitPosition) > 0) ) ) {

            if (comSignal->ComTimeoutFactor > 0) { /* If reception deadline monitoring is used.*/
                /* Reset the deadline monitoring timer.*/
                /* @req COM715 */
                Arc_Signal->Com_Arc_DeadlineCounter = comSignal->ComTimeoutFactor;
            }

#if (COM_SIG_GATEWAY_ENABLE == STD_ON)
            /* Save the indication of new signal for gateway routing */
            if ((TRUE == IPdu->ComIPduGwRoutingReq) && (TRUE == comSignal->ComSigGwRoutingReq)) {
                Arc_Signal->ComSignalUpdatedGwRouting = TRUE;
            }
#endif

            /* Check the signal processing mode.*/
            if (IPdu->ComIPduSignalProcessing == COM_IMMEDIATE) {
                /* If signal processing mode is IMMEDIATE, notify the signal callback.*/
                /* @req COM300 */
                /* @req COM301 */
                if ((IPdu->ComIPduSignalRef[i]->ComNotification != COM_NO_FUNCTION_CALLOUT) &&
                    (ComNotificationCallouts[IPdu->ComIPduSignalRef[i]->ComNotification] != NULL) ) {
                    ComNotificationCallouts[IPdu->ComIPduSignalRef[i]->ComNotification]();
                }
            } else {
                /* Signal processing mode is DEFERRED, mark the signal as updated.*/
                Arc_Signal->ComSignalUpdated = TRUE;
            }

        } else {
            DEBUG(DEBUG_LOW, "Com_RxIndication: Ignored signal %d of I-PDU %d since its update bit was not set\n", comSignal->ComHandleId, comSignal->ComIPduHandleId);
        }
    }

#if (COM_SIG_GATEWAY_ENABLE == STD_ON)
    const ComGwSrcDesc_type * comGwSrcPtr;
    uint16 srcSigDescHandle;
    /* Save the indication for a new gateway signal description reception */
    if ((TRUE == IPdu->ComIPduGwRoutingReq) && (NULL != IPdu->ComIPduGwMapSigDescHandle)) {

        for (uint8 j=0; IPdu->ComIPduGwMapSigDescHandle[j] != INVALID_GWSIGNAL_DESCRIPTION_HANDLE; j++){

            srcSigDescHandle = IPdu->ComIPduGwMapSigDescHandle[j];
            comGwSrcPtr = GET_GwSrcSigDesc(srcSigDescHandle);
            if ( (FALSE == comGwSrcPtr->ComSignalArcUseUpdateBit) ||
                /*lint -e{9016} Array indexing couldn't be implemented, as parameters are of different data types */
                ( (TRUE == comGwSrcPtr->ComSignalArcUseUpdateBit) && (TESTBIT(Arc_IPdu->ComIPduDataPtr, comGwSrcPtr->ComUpdateBitPosition)> 0) ) ) {

                (GET_ArcGwSrcSigDesc(srcSigDescHandle))->ComSignalUpdatedGwRouting = TRUE;
            }
        }
    }
#endif
}

/**
 * Sevice to handle transmission deadline monitoring logic
 * Internal call
 * Called from reception APIs
 * @param IPdu
 * @param Arc_IPdu
 * @return none
 */
void Com_Misc_TxHandleDM(const ComIPdu_type *IPdu, Com_Arc_IPdu_type *Arc_IPdu)
{
    const ComTxMode_type *txModePtr;

    if((IPdu != NULL) && (Arc_IPdu != NULL) ){
        if( TRUE == Arc_IPdu->Com_Arc_IpduTxMode ) {
            txModePtr = &IPdu->ComTxIPdu.ComTxModeTrue;
        }
        else {
            txModePtr = &IPdu->ComTxIPdu.ComTxModeFalse;
        }
        SchM_Enter_Com_EA_0();
        switch( txModePtr->ComTxModeMode ) {
            /* @req COM308 */
            /* @req COM305.3 */
            /* @req COM305.4 */ /* deviation - but no RTE callback at nth confirmation*/
            case COM_DIRECT:
            case COM_MIXED:
            /* cancel the DM timer at the Nth( even if N = 1) confirmation */
                if(0 < Arc_IPdu->Com_Arc_TxIPduTimers.ComTxIPduNumberOfTxConfirmations) {
                    Arc_IPdu->Com_Arc_TxIPduTimers.ComTxIPduNumberOfTxConfirmations--;
                    if(Arc_IPdu->Com_Arc_TxIPduTimers.ComTxIPduNumberOfTxConfirmations == 0){
                        Arc_IPdu->Com_Arc_TxIPduTimers.ComTxDMTimer = 0;
                    }
                }
                else { /* fall back */
                    Arc_IPdu->Com_Arc_TxIPduTimers.ComTxDMTimer = 0; /* cancel the DM timer at every confirmation */
                }
                break;

            case COM_PERIODIC:
                /* blindly cancel the timer, no check,
                * since if timed out,indication could have gone for the timed out request
                * and  timer would have not started even if there was a new request */
                Arc_IPdu->Com_Arc_TxIPduTimers.ComTxDMTimer = 0;
                break;


            case COM_NONE:
                /* !req COM835 */
                /* @req COM697 */   /* reset timer for each confirmation for TX mode NONE
                * NONE mode is not taken care in the implementations */
                Arc_IPdu->Com_Arc_TxIPduTimers.ComTxDMTimer = Arc_IPdu->Com_Arc_TxDeadlineCounter;
                break;

            default:
                break;

        }
        SchM_Exit_Com_EA_0();
    }
}


void Com_Misc_UnlockTpBuffer(PduIdType PduId) {
    Com_BufferPduState[PduId].locked = false;
    Com_BufferPduState[PduId].currentPosition = 0;
}

void Com_Misc_UpdateShadowSignal(Com_SignalIdType SignalId, const void *SignalDataPtr) {
    const Com_Arc_GroupSignal_type *Arc_GroupSignal = GET_ArcGroupSignal(SignalId);
    const ComGroupSignal_type *GroupSignal = GET_GroupSignal(SignalId);
    /* @req COM632 */
    /* @req COM633 */ /* Sign extension? */
    boolean dataChanged = FALSE;
    Com_Misc_WriteSignalDataToPdu(
            SignalDataPtr,
            GroupSignal->ComSignalType,
            Arc_GroupSignal->Com_Arc_ShadowBuffer,
            GroupSignal->ComBitPosition,
            GroupSignal->ComBitSize,
            GroupSignal->ComSignalEndianess,
            &dataChanged);
}

/* Helpers for getting and setting that a TX PDU confirmation status
 * These function uses the ComSignalUpdated for the first signal within the Pdu. The
 * ComSignalUpdated isn't used for anything else in TxSignals and it is mainly used
 * in Rx signals.
 * The reason is to save RAM.
 */

void Com_Misc_SetTxConfirmationStatus(const ComIPdu_type *IPdu, boolean value) {

    const ComSignal_type *signal = IPdu->ComIPduSignalRef[0];

    if (signal != NULL) {
        Com_Arc_Signal_type * Arc_Signal = GET_ArcSignal(signal->ComHandleId);
        Arc_Signal->ComSignalUpdated = value;
    }
}

boolean Com_Misc_GetTxConfirmationStatus(const ComIPdu_type *IPdu) {

    boolean status;
    status = FALSE;

    if (IPdu != NULL) {
        const ComSignal_type *signal = IPdu->ComIPduSignalRef[0];

        if (signal == NULL) {
            status = FALSE;
        } else {
            const Com_Arc_Signal_type * Arc_Signal = GET_ArcSignal(signal->ComHandleId);
            status = Arc_Signal->ComSignalUpdated;
        }
    }
    return status;
}

/**
 * Implements functionality for Com_TriggerIPDUSend but returns
 * wether transmission was triggered or not
 * @param PduId
 * @return COM_TX_TRIGGERED: Tx was triggered, COM_TX_NOT_TRIGGERED: Not triggered
 */
ComTxTriggerStatusType Com_Misc_TriggerIPDUSend(PduIdType PduId) {
#if(COM_IPDU_COUNTING_ENABLE ==  STD_ON )
    uint16 cntrIdx = 0;
    uint8 cntrVal = 0;
#endif
    boolean status;
    status = TRUE;
    ComTxTriggerStatusType txTriggerStatus = COM_TX_NOT_TRIGGERED;
    const ComIPdu_type *IPdu = GET_IPdu(PduId);
    Com_Arc_IPdu_type *Arc_IPdu = GET_ArcIPdu(PduId);
    SchM_Enter_Com_EA_0();

    /* Is the IPdu ready for transmission?*/
    /* @req COM388 */
    if (Arc_IPdu->Com_Arc_TxIPduTimers.ComTxIPduMinimumDelayTimer == 0) {

        /* Check callout status*/
        /* @req COM492 */
        /* @req COM346 */
        /* !req COM381 */
        /* IMPROVEMENT: No other COM API than Com_TriggerIPDUSend, Com_SendSignal or Com_SendSignalGroup
         * can be called from an an I-PDU callout.*/
        /* !req COM780 */
        /* !req COM781 */
        /* @req COM719 */
        if ((IPdu->ComTxIPduCallout != COM_NO_FUNCTION_CALLOUT) && (ComTxIPduCallouts[IPdu->ComTxIPduCallout] != NULL) ) {
            if (FALSE == ComTxIPduCallouts[IPdu->ComTxIPduCallout](PduId, Arc_IPdu->ComIPduDataPtr)) {
                // IMPROVEMENT: Report error to DET.
                // Det_ReportError();
                txTriggerStatus = COM_TX_NOT_TRIGGERED;
                status = FALSE;
            }
        }
        if (status == TRUE) {
            PduInfoType PduInfoPackage;
            PduInfoPackage.SduDataPtr = (uint8 *)Arc_IPdu->ComIPduDataPtr;
            if (IPdu->ComIPduDynSignalRef != 0) {
                /* !req COM757 */ /*Length of I-PDU?*/
                uint16 sizeWithoutDynSignal = IPdu->ComIPduSize - (IPdu->ComIPduDynSignalRef->ComBitSize/8);
                PduInfoPackage.SduLength = sizeWithoutDynSignal + Arc_IPdu->Com_Arc_DynSignalLength;
            } else {
                PduInfoPackage.SduLength = IPdu->ComIPduSize;
            }

    #if(COM_IPDU_COUNTING_ENABLE ==  STD_ON )
            if (ComConfig->ComIPdu[PduId].ComIpduCounterRef != NULL){
                cntrVal = updateIPduCntr(ComConfig->ComIPdu[PduId].ComIpduCounterRef, PduInfoPackage.SduDataPtr, &cntrIdx) ;
            }
    #endif
            /* Send IPdu!*/
            /* @req COM138 */
            if (PduR_ComTransmit(IPdu->ArcIPduOutgoingId, &PduInfoPackage) == E_OK) {
                txTriggerStatus = COM_TX_TRIGGERED;
                if (IPdu->ComTxIPdu.ComTxIPduClearUpdateBit == TRANSMIT) {
                    /* Clear all update bits for the contained signals*/
                    /* @req COM062 */
                    for (uint16 i = 0; (IPdu->ComIPduSignalRef != NULL) && (IPdu->ComIPduSignalRef[i] != NULL); i++) {
                        if (TRUE == IPdu->ComIPduSignalRef[i]->ComSignalArcUseUpdateBit) {
                            /*lint -e{9016} Array indexing couldn't be implemented, as parameters are of different data types */
                            CLEARBIT(Arc_IPdu->ComIPduDataPtr, IPdu->ComIPduSignalRef[i]->ComUpdateBitPosition);
                        }
                    }
                }
    #if (COM_SIG_GATEWAY_ENABLE == STD_ON)
                const ComGwDestnDesc_type * gwSigDesc;
                /* Reset the update bit for destination gateway signal description */
                /* @req COM702 */
                /* @req COM706 */
                for (uint8 i = 0; (IPdu->ComIPduGwMapSigDescHandle != NULL) && (IPdu->ComIPduGwMapSigDescHandle[i] != INVALID_GWSIGNAL_DESCRIPTION_HANDLE); i++){
                    gwSigDesc = GET_GwDestnSigDesc(IPdu->ComIPduGwMapSigDescHandle[i]);
                    if (TRUE == gwSigDesc->ComSignalArcUseUpdateBit) {
                        /*lint -e{9016} Array indexing couldn't be implemented, as parameters are of different data types */
                        CLEARBIT(Arc_IPdu->ComIPduDataPtr, gwSigDesc->ComUpdateBitPosition);
                    }
                }
    #endif
    #if(COM_IPDU_COUNTING_ENABLE ==  STD_ON )
                /* @req COM688 */
                if (ComConfig->ComIPdu[PduId].ComIpduCounterRef != NULL){
                    //lint -save -e644 //Warning:cntrVal & cntrIdx may not have been initialized. These are computed above
                    Com_IPdu_Counters[cntrIdx] = cntrVal ; /* Update counter value*/
                    //lint -restore
                }
    #endif
            } else {
                Com_Misc_UnlockTpBuffer(getPduId(IPdu));
            }

            /* Reset miminum delay timer.*/
            /* @req COM471 */
            /* @req COM698 */
            Arc_IPdu->Com_Arc_TxIPduTimers.ComTxIPduMinimumDelayTimer = IPdu->ComTxIPdu.ComTxIPduMinimumDelayFactor;
        }
    } else {
        /* Not time for transmission */
        txTriggerStatus = COM_TX_MIN_DELAY_NOT_FULFILLED;
    }

    SchM_Exit_Com_EA_0();
    return txTriggerStatus;
}

#if (COM_SIG_GATEWAY_ENABLE == STD_ON)
/* Rout destination signals and group signals */
void Com_Misc_RouteGwDestnSignals(uint8 gwMapidx, const uint8 * SignalDataPtr,Com_SignalType ComSigType,uint16 ComBitSize){

    uint16 j,sigHandle, bitpos,ubitPos, iPduHandle;
    ComGwSignalRef_type gwSignalRef;
    boolean dataChanged;
    const void * comSignalDest;
    const Com_Arc_IPdu_type  *arcIPduDest;
    uint8 *comIPduDataPtr;
    const Com_Arc_Signal_type * arcSigDest;
    ComSignalEndianess_type endian;
    boolean updateBitUsed;
    const ComSignal_type* comSigGrp;
    ComTransferPropertyType transferProperty;
    boolean isValid = FALSE;

    /* Copy gated signals to target IPdus and Evaluate runtime transmission properties */
    /* @req COM466 */
    for (j=0;j < ComConfig->ComGwMappingRef[gwMapidx].ComGwNoOfDesitnationRoutes;j++) {
        /* Loop over all destination mappings */
        gwSignalRef = ComConfig->ComGwMappingRef[gwMapidx].ComGwDestinationRef[j].ComGwDestinationSignalRef;
        sigHandle   = ComConfig->ComGwMappingRef[gwMapidx].ComGwDestinationRef[j].ComGwDestinationSignalHandle;
        dataChanged = FALSE;
        isValid = FALSE;

        /* Type of destination signal */
        switch (gwSignalRef) {
            /*lint -save -e644 -e645 */
            case COM_SIGNAL_REFERENCE:
                /*lint -e{929} pointer cast is required*/
                comSignalDest   = (const ComSignal_type *)GET_Signal(sigHandle) ;
                iPduHandle      = ((const ComSignal_type *)comSignalDest)->ComIPduHandleId;
                arcIPduDest     = GET_ArcIPdu(iPduHandle);
                comIPduDataPtr  = arcIPduDest->ComIPduDataPtr;
                bitpos          = ((const ComSignal_type *)comSignalDest)->ComBitPosition;
                endian          = ((const ComSignal_type *)comSignalDest)->ComSignalEndianess;
                ubitPos         = ((const ComSignal_type *)comSignalDest)->ComUpdateBitPosition;
                updateBitUsed   = ((const ComSignal_type *)comSignalDest)->ComSignalArcUseUpdateBit;
                transferProperty    = ((const ComSignal_type *)comSignalDest)->ComTransferProperty;
                isValid         = TRUE;
                break;

            case COM_GROUP_SIGNAL_REFERENCE:
                /*lint -e{929} pointer cast is required*/
                comSignalDest   = (const ComGroupSignal_type *)GET_GroupSignal(sigHandle) ;
                sigHandle       = ((const ComGroupSignal_type *)comSignalDest)->ComSigGrpHandleId;
                comSigGrp       = GET_Signal(sigHandle);
                iPduHandle      = comSigGrp->ComIPduHandleId;
                arcIPduDest     = GET_ArcIPdu(iPduHandle);
                arcSigDest      = GET_ArcSignal(sigHandle);
                /*lint -e{9005} GET_ArcSignal(sigHandle) handled via comIPduDataPtr pointer*/
                comIPduDataPtr  = (uint8*)arcSigDest->Com_Arc_ShadowBuffer;
                bitpos          = ((const ComGroupSignal_type *)comSignalDest)->ComBitPosition;
                endian          = ((const ComGroupSignal_type *)comSignalDest)->ComSignalEndianess;
                ubitPos         = comSigGrp->ComUpdateBitPosition;
                updateBitUsed   = comSigGrp->ComSignalArcUseUpdateBit;
                transferProperty    = comSigGrp->ComTransferProperty;
                isValid         = TRUE;
                break;

            case GATEWAY_SIGNAL_DESCRIPTION:
                /*lint -e{929} pointer cast is required*/
                comSignalDest   = (const ComGwDestnDesc_type *)GET_GwDestnSigDesc(sigHandle) ;
                iPduHandle      = ((const ComGwDestnDesc_type *)comSignalDest)->ComIPduHandleId;
                arcIPduDest     = GET_ArcIPdu(iPduHandle);
                comIPduDataPtr  = arcIPduDest->ComIPduDataPtr;
                bitpos          = ((const ComGwDestnDesc_type *)comSignalDest)->ComBitPosition;
                endian          = ((const ComGwDestnDesc_type *)comSignalDest)->ComSignalEndianess;
                ubitPos         = ((const ComGwDestnDesc_type *)comSignalDest)->ComUpdateBitPosition;
                updateBitUsed   = ((const ComGwDestnDesc_type *)comSignalDest)->ComSignalArcUseUpdateBit;
                transferProperty    = ((const ComGwDestnDesc_type *)comSignalDest)->ComTransferProperty;
                isValid         = TRUE;
                break;

            case COM_SIGNAL_GROUP_REFERENCE:/*This condition is erroneous*/
                break;
            default:/*This condition is erroneous*/
                break;
        }

        if (FALSE == isValid) {
            break;
        }

        /* Endianness conversion is handled by Com_Misc_WriteSignalDataToPdu() */
        /* @req COM360 */
        /* @req COM362 */
        Com_Misc_WriteSignalDataToPdu(
                SignalDataPtr,
                ComSigType,
                comIPduDataPtr,
                bitpos,
                ComBitSize,
                endian,
                &dataChanged);
        /* Only if Update bit available it is set */
        /* @req COM704 */
        /* @req COM706 */
        if (TRUE == updateBitUsed) {
            /*lint -e{926} pointer cast is essential since SETBIT parameters are of different pointer data type*/
            /*lint -e{9016} Array indexing couldn't be implemented, as parameters are of different data types */
            SETBIT(comIPduDataPtr, ubitPos);
        }

        SchM_Enter_Com_EA_0();

        if (COM_GROUP_SIGNAL_REFERENCE == gwSignalRef) {
            /* Copy from shadow buffer to IPdu ram buffer */
            Com_Misc_CopySignalGroupDataFromShadowBufferToPdu(sigHandle,FALSE,&dataChanged);
            /* Only if Update bit available it is set */
            /* @req COM704 */
            /* @req COM706 */
            if (TRUE == updateBitUsed) {
                /*lint -e{9016} Array indexing couldn't be implemented, as parameters are of different data types */
                SETBIT(arcIPduDest->ComIPduDataPtr, ubitPos);
            }
        }
        /* Assign the number of repetitions based on Transfer property */
        (void)Com_Misc_TriggerTxOnConditions(iPduHandle, dataChanged, transferProperty);
        SchM_Exit_Com_EA_0();
        /*lint -restore */
    }
}

/* Extract source signal for gateway routing */
void Com_Misc_ExtractGwSrcSigData(const void* comSignalSrc, uint16 iPduHandle,uint8 *SigDataPtr, const Com_Arc_ExtractPduInfo_Type *pduInfo ) {

    /*lint -e{920} comSignalSrc not used */
    (void)comSignalSrc;
    const Com_Arc_IPdu_type *arcIPduSrc;
    const ComIPdu_type *iPdu;
    const uint8* pduDataPtr = NULL;
    uint16 startFromPduByte;

    arcIPduSrc = GET_ArcIPdu(iPduHandle);
    iPdu = GET_IPdu(iPduHandle);

    if (iPdu->ComIPduSignalProcessing == COM_DEFERRED) {
        pduDataPtr = arcIPduSrc->ComIPduDeferredDataPtr;
    }else {
        pduDataPtr = arcIPduSrc->ComIPduDataPtr;
    }

    if ((COM_UINT8_N != pduInfo->ComSignalType) && (COM_UINT8_DYN != pduInfo->ComSignalType)) {

        Com_Misc_ReadSignalDataFromPdu(
                pduDataPtr,
                pduInfo->ComBitPosition,
                pduInfo->ComBitSize,
                pduInfo->ComSignalEndianess,
                pduInfo->ComSignalType,
                SigDataPtr);
    } else {

        startFromPduByte = (pduInfo->ComBitPosition) / 8;
        SchM_Enter_Com_EA_0();
        memcpy(SigDataPtr, (&pduDataPtr[startFromPduByte]), (pduInfo->ComBitSize)/8);
        SchM_Exit_Com_EA_0();
    }

}
#endif
