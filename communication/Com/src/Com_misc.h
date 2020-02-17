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


#ifndef COM_MISC_H_
#define COM_MISC_H_

/* Copy Signal group data from PDU to shadowbuffer*/
void Com_Misc_CopySignalGroupDataFromPduToShadowBuffer(
        const Com_SignalIdType signalGroupId);

/* Copy Signal group data from shadowbuffer to Pdu*/
void Com_Misc_CopySignalGroupDataFromShadowBufferToPdu(
        const Com_SignalIdType signalGroupId,
        boolean deferredBufferDestination,
        boolean *dataChanged);

/**
 * Reads signal data from Pdu
 * @param comIPduDataPtr
 * @param bitPosition
 * @param bitSize
 * @param endian
 * @param signalType
 * @param SignalData
 */
void Com_Misc_ReadSignalDataFromPdu (
        const uint8 *comIPduDataPtr,
        Com_BitPositionType bitPosition,
        uint16 bitSize,
        ComSignalEndianess_type endian,
        Com_SignalType signalType,
        uint8 *SignalData);

/**
 * Write signal data to PDU
 * @param SignalDataPtr
 * @param signalType
 * @param comIPduDataPtr
 * @param bitPosition
 * @param bitSize
 * @param endian
 * @param dataChanged
 */
void Com_Misc_WriteSignalDataToPdu(
        const uint8 *SignalDataPtr,
        Com_SignalType signalType,
        uint8 *comIPduDataPtr,
        Com_BitPositionType bitPosition,
        uint16 bitSize,
        ComSignalEndianess_type endian,
        boolean *dataChanged);

void Com_Misc_UpdateShadowSignal(Com_SignalIdType SignalId, const void *SignalDataPtr);

/*
 * This function copies numBits bits of data from Source to Destination with the possibility to offset
 * both the source and destination.
 *
 * Return value: the last bit it copies (sign bit).
 */
//uint8 Com_CopyData(void *Destination, const void *Source, uint8 numBits, uint8 destOffset, uint8 sourceOffset);

//void Com_CopyData2(char *dest, const char *source, uint8 destByteLength, uint8 segmentStartBitOffset, uint8 segmentBitLength);

void Com_Misc_RxProcessSignals(const ComIPdu_type *IPdu,const Com_Arc_IPdu_type *Arc_IPdu);
void Com_Misc_TxHandleDM      (const ComIPdu_type *IPdu, Com_Arc_IPdu_type *Arc_IPdu);
static inline PduIdType getPduId(const ComIPdu_type* IPdu) {
    extern const Com_ConfigType * ComConfig;
    /*lint -e{946} -e{947} pointers are subtracted to get the address
     * which intern type-casted to uint16, the address of PduId is returned, which is the required operation*/
    /*lint -e{9033} addresses are subtracted and casted to uint16, this is required operation*/
    return (PduIdType)(IPdu - ComConfig->ComIPdu);
}

void Com_Misc_UnlockTpBuffer(PduIdType PduId);
#define isPduBufferLocked(id) Com_BufferPduState[id].locked


/* Helpers for getting and setting that a PDU confirmation status */
void Com_Misc_SetTxConfirmationStatus(const ComIPdu_type *IPdu, boolean value);
boolean Com_Misc_GetTxConfirmationStatus(const ComIPdu_type *IPdu);

/* Assign repition cycles for DIRECT and MIXED transmission modes */
boolean Com_Misc_ReAssignReps(const ComIPdu_type *IPdu,Com_Arc_IPdu_type *arcIPdu, boolean dataChanged, ComTransferPropertyType transferProperty);

#if (COM_SIG_GATEWAY_ENABLE == STD_ON)
/* Transmit destination signals/group signals */
void Com_Misc_RouteGwDestnSignals(uint8 gwMapidx, const uint8 * SignalDataPtr,Com_SignalType ComSigType,uint16 ComBitSize);
/* Extract receive signals/group signals */
void Com_Misc_ExtractGwSrcSigData(const void* comSignalSrc, uint16 iPduHandle,uint8 *SigDataPtr, const Com_Arc_ExtractPduInfo_Type *pduInfo ) ;
#endif

#endif /* COM_MISC_H_ */
