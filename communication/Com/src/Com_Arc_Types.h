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


#ifndef COM_ARC_TYPES_H_
#define COM_ARC_TYPES_H_

#include "Std_Types.h"
#include "Com_Types.h"


typedef struct {
    uint32 ComFilterArcN;
    uint32 ComFilterArcNewValue;
    uint32 ComFilterArcOldValue;
} Com_Arc_Filter_type;

typedef struct {
    const void *Com_Arc_ShadowBuffer;
    uint32 Com_Arc_DeadlineCounter;
    boolean ComSignalUpdated;
    boolean ComSignalUpdatedGwRouting; /* Indicating signal update for gateway routing. Relevant only to Gateway source signals */
    boolean ComSignalRoutingReq; /* Routing is requested if ComSignalUpdatedGwRouting is set */
} Com_Arc_Signal_type;


typedef struct {
    void *Com_Arc_ShadowBuffer;
    boolean ComSignalUpdated;
    uint8 Com_Arc_EOL;
} Com_Arc_GroupSignal_type;

typedef struct {
    uint32 ComTxModeRepetitionPeriodTimer;
    uint32 ComTxIPduMinimumDelayTimer;
    uint32 ComTxModeTimePeriodTimer;
    uint32 ComTxDMTimer; /* Tx DM  timer */
    uint8  ComTxIPduNumberOfRepetitionsLeft;
    uint8  ComTxIPduNumberOfTxConfirmations; /* Tx confirmations from Pdu for the repeted requests(N times) */
} Com_Arc_TxIPduTimer_type;

typedef struct {
    /** Reference to the actual pdu data storage */
    void *ComIPduDataPtr;
    void *ComIPduDeferredDataPtr;

    Com_Arc_TxIPduTimer_type Com_Arc_TxIPduTimers;
    uint32 Com_Arc_TxDeadlineCounter; /* a separate counter storage for Tx deadline monitor,
                                       no need to parse all the signals everytime for the timeout refilling */
    uint16 Com_Arc_DynSignalLength;
    boolean Com_Arc_IpduStarted;
    boolean Com_Arc_IpduRxDMControl;
    boolean Com_Arc_IpduTxMode; /* @req COM605 */

} Com_Arc_IPdu_type;

typedef struct {
    boolean ComSignalUpdatedGwRouting;
    boolean ComSignalRoutingReq; /* Routing is requested if ComSignalUpdatedGwRouting is set */
}Com_Arc_GwSrcDesc_type;

typedef struct {
    Com_Arc_IPdu_type *ComIPdu; // Only used in PduIdCheck()
    Com_Arc_Signal_type *ComSignal;
    Com_Arc_GroupSignal_type *ComGroupSignal;
    Com_Arc_GwSrcDesc_type * ComGwSrcDescSignal;
} Com_Arc_Config_type;

typedef struct{
    Com_BitPositionType ComBitPosition;
    uint16 ComBitSize;
    ComSignalEndianess_type ComSignalEndianess;
    Com_SignalType ComSignalType;
}Com_Arc_ExtractPduInfo_Type;

#endif
