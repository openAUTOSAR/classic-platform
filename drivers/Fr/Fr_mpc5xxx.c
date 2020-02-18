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

/** @reqSettings DEFAULT_SPECIFICATION_REVISION=4.0.3 */
/** @tagSettings DEFAULT_ARCHITECTURE=RH850F1H */

// #define _DEBUG_
/* ----------------------------[includes]------------------------------------*/
#include "Fr.h"
#include "Fr_Internal.h"
#if defined(USE_DEM) || defined(CFG_FR_DEM_TEST)
#include "Dem.h"
#endif
#if defined(USE_DET)
#include "Det.h"
#endif

#include "mpc55xx.h"
#include "timer.h"
#include <assert.h>
#include <string.h>
#include "debug.h"

// #define _FR_DEBUG_INIT_

#if defined(_FR_DEBUG_INIT_)
#define _debug_init_(...) printf (__VA_ARGS__);
#else
#define _debug_init_(...)
#endif

#if defined(_FR_DEBUG_TX_)
#define _debug_tx_(...) printf (__VA_ARGS__);
#else
#define _debug_tx_(...)
#endif

#if defined(_FR_DEBUG_RX_)
#define _debug_rx_(...) printf (__VA_ARGS__);
#else
#define _debug_rx_(...)
#endif

#if defined(_FR_DEBUG_TIMER_)
#define _debug_timer_(...) printf (__VA_ARGS__);
#else
#define _debug_timer_(...)
#endif


/* ----------------------------[private define]------------------------------*/

#define POC_STATE_CONFIG    3u
#define POC_STATE_PASSIVE   4u
#define POC_STATE_RUN       5u

#define MAX_WAIT_CYCLES (uint32)10000UL

#ifndef DEM_EVENT_ID_NULL
#define DEM_EVENT_ID_NULL    0u
#endif


#define FR_LENGTH_DIV           2UL

#define V_CRC_SIZE (uint32)11UL
#define V_CRC_POLYNOMIAL (uint32)0x385UL
#define V_CRC_INIT   (uint32)0x1AUL

/* Number of shadow buffers... set this to 4 (don't care about memory) */
#define SHADOW_BUFFER_CNT          4

#define CMD_OK              0u
#define CMD_TIMEOUT         1u


/* ----------------------------[private macro]-------------------------------*/
#if defined(USE_DEM) || defined(CFG_FR_DEM_TEST)
#define DEM_REPORT(_eventId, _status) Fr_Internal_reportDem((_eventId), (_status))
#else
#define DEM_REPORT(_eventId, _status)
#endif


#define ALIGN8(_x)          ((((_x) + 8ul - 1ul) / 8ul) * 8ul)


#define _LOW16(_x)   ((_x) & 0xfffful)
#define _HIGH16(_x)   (((_x)>>16) & 0xfffful)

#define DIAG_SET(_x,_y)     _x = (_y); \
                            if ( (_x) != (_y) ) { errCnt++; }


/* ----------------------------[private typedef]-----------------------------*/

typedef enum {
    HEADER = 0,
    HEADER_AND_DATA,
    DATA,
}MsgRAMReadoutType;

typedef struct Fr_Info {
    uint32 staticSlots;
    uint32 dynamicSlots;
    uint32 bitRate;
    uint32 cycles;
    uint32 macrotick;
    const Fr_ContainerType *cfg;
} Fr_InfoType;

Fr_InfoType Fr_Info;

/* ----------------------------[private function prototypes]-----------------*/
static Std_ReturnType findTrigIdx(const Fr_FrIfCCTriggeringType *FrTrigConf, uint32 searchedId, uint32 *trigIdx);
static uint32 getHeaderCrc(const Fr_ContainerType *Fr_Cfg, uint8 cIdx, uint32 frameTrigIdx, uint32 dynLen);

/* ----------------------------[private variables]---------------------------*/

struct FR_tag * const Fr_HwReg[] = {
        ((struct FR_tag *) (&FR_0)),
};

uint8 Fr_MsgBuffers[128*256] __balign(0x10);        /* IMPROVEMENT, fix size here */

/* ----------------------------[functions]---------------------------*/



/* MBIDXRn index to:
 * - Header    : SADR_MBHF= (FR_MBIDXn.MBIDX*8 + FR_SYMBADR.SMBA)
 * - Frame Data: SADR_MBDF= FR_MBDORn.MBDO + FR_SYMBADR.SMBA
 *
 * Memory Layout:
 *
 * Calc                                                  Description
 * ---------------------------------------------------------------------------
 * FrNbrTrigConfiged * 8                                    Headers for both static and dynamic messages
 * gNumberOfStaticSlots * gPayloadLengthStatic *  2         Static payload slots.
 * FrPPayloadLengthDynMax *
 *
 */


MB_HEADER_t * getHeader( struct FR_tag *hwPtr, uint16 idx ) {
    MB_HEADER_t *h;

    h = (MB_HEADER_t *)(idx * 8u + (((uint32)hwPtr->SYSBADHR.R)<<16) + ((uint32)hwPtr->SYSBADLR.R) );

    return h;
}


uint8 * getPayload( const Fr_FrIfCCTriggeringType *trigCfg, uint32 msgBufIdx ) {
    uint8 *h;

    h = (uint8 *)trigCfg->FrMsgBufferCfg[msgBufIdx].FrDataPartitionAddr;

    return h;
}

boolean slotIsStatic(const Fr_FrIfClusterConfigType *clCfg, uint32 slotId ) {
    boolean rv = FALSE;
    if( slotId < clCfg->FrClusterGNumberOfStaticSlots ) {
        rv =  TRUE;
    }
    return rv;
}


/**
 * Disable Tx
 * @param Fr_Cfg
 * @param cIdx
 * @return
 */
Std_ReturnType Fr_Internal_ClearPendingTx(const Fr_ContainerType *Fr_Cfg, uint8 cIdx) {
    Std_ReturnType retval = E_OK;

    /* IMPROVMENT */

    return retval;
}

/**
 * Disable Rx
 * @param Fr_Cfg
 * @param cIdx
 * @return
 */
Std_ReturnType Fr_Internal_ClearPendingRx(const Fr_ContainerType *Fr_Cfg, uint8 cIdx) {
    Std_ReturnType retval = E_OK;

    /* IMPROVEMENT */

    return retval;
}


/**
 * Compares the parameters written to register to those defined in the config.
 * @param Fr_Cfg
 * @param cIdx
 * @return
 */
Std_ReturnType Fr_Internal_RunCCTest(const Fr_ContainerType *Fr_Cfg, uint8 cIdx) {
    Std_ReturnType retval = E_OK;

    return retval;
}


/**
 * Wait for expression _wait_exp to become true for _us.
 */
static uint32 busyRead_1us( volatile const uint16 *baseAddr, uint16 mask, uint16 match) {

    uint32 rv = CMD_TIMEOUT;

    uint32 s = Timer_GetTicks();
    while ( TIMER_TICK2US(Timer_GetTicks() - s) < (50u) ) {
        if( (*baseAddr & mask) == match ) {
            rv = CMD_OK;
            break;
        }
    }
    return rv;
}


static Std_ReturnType PCRInit( const Fr_ContainerType *Fr_Cfg, struct FR_tag *hwPtr, uint8 cIdx ) {

    const Fr_CtrlConfigParametersType *paramPtr = &Fr_Cfg->Fr_ConfigPtr->FrCtrlParam[cIdx];
    const Fr_FrIfClusterConfigType *cCfg = &Fr_Cfg->Fr_ConfigPtr->FrClusterConfig[cIdx];
    const Fr_FrIfCCTriggeringType *FrTrigConf = &Fr_Cfg->Fr_ConfigPtr->FrTrigConfig[cIdx];
    uint32 noiseListenTimeout;
    uint32 microPerCyleMin;
    uint32 microPerCyleMax;
    uint32 errCnt = 0;
    Std_ReturnType rv = E_OK;

    /* More or less a direct copy of
     * "Table 46-13. Protocol Configuration Register (PCR0-30) Fields" from the RM
     */

    /** PCR[0] */
    DIAG_SET( hwPtr->PCR0.B.STATIC_SLOT_LENGTH , cCfg->FrClusterGdStaticSlot );
    _debug_init_("STATIC_SLOT_LENGTH=%d [MT]\n",hwPtr->PCR0.B.STATIC_SLOT_LENGTH);
    DIAG_SET( hwPtr->PCR0.B.ACTION_POINT_OFFSET , cCfg->FrClusterGdActionPointOffset - 1u);

    /** PCR[1] */
    DIAG_SET( hwPtr->PCR1.B.MACRO_AFTER_FIRST_STATIC_SLOT , cCfg->FrClusterGMacroPerCycle - cCfg->FrClusterGdStaticSlot);

    /** PCR[2] */
    DIAG_SET(  hwPtr->PCR2.B.MINISLOT_AFTER_ACTION_POINT , cCfg->FrClusterGdMinislot - cCfg->FrClusterGdMiniSlotActionPointOffset - 1u);
    DIAG_SET(  hwPtr->PCR2.B.NUMBER_OF_STATIC_SLOTS , cCfg->FrClusterGNumberOfStaticSlots);
    _debug_init_("NUMBER_OF_STATIC_SLOTS=%d\n",hwPtr->PCR2.B.NUMBER_OF_STATIC_SLOTS);

    /** PCR[3] */
    DIAG_SET(  hwPtr->PCR3.B.COLDSTART_ATTEMPTS , cCfg->FrClusterGColdStartAttempts);
    DIAG_SET(  hwPtr->PCR3.B.MINISLOT_ACTION_POINT_OFFSET , cCfg->FrClusterGdMiniSlotActionPointOffset - 1u);
    DIAG_SET(  hwPtr->PCR3.B.WAKEUP_SYMBOL_RX_LOW , cCfg->FrClusterGdWakeupRxLow);

    /** PCR[4] */
    DIAG_SET(  hwPtr->PCR4.B.CAS_RX_LOW_MAX , cCfg->FrClusterGdCasRxLowMax - 1u);
    DIAG_SET(  hwPtr->PCR4.B.WAKEUP_SYMBOL_RX_WINDOW , cCfg->FrClusterGdWakeupRxWindow);

    /** PCR[5] */
    DIAG_SET(  hwPtr->PCR5.B.TSS_TRANSMITTER , cCfg->FrClusterGdTSSTransmitter);
    DIAG_SET(  hwPtr->PCR5.B.WAKEUP_SYMBOL_RX_IDLE , cCfg->FrClusterGdWakeupRxIdle);
    DIAG_SET(  hwPtr->PCR5.B.WAKEUP_SYMBOL_TX_LOW  , cCfg->FrClusterGdWakeupTxActive);  /* Maps to gdWakeupSymbolTxLow */

    /** PCR[6] */
    if (cCfg->FrClusterGdSymbolWindow != 0u) {
        DIAG_SET(  hwPtr->PCR6.B.SYMBOL_WINDOW_AFTER_ACTION_POINT , cCfg->FrClusterGdSymbolWindow - cCfg->FrClusterGdActionPointOffset - 1u);
    }
    DIAG_SET(  hwPtr->PCR6.B.MACRO_INITIAL_OFFSET_A , paramPtr->FrPMacroInitialOffsetA);

    /** PCR[7] */
    /* From FR 2.1: pMicroPerMacroNom, renamed in FR 3.0 to aMicroPerMacroNom
     *
     * Calculation (from 2.1) :
     *   pMicroPerMacroNom[µT/MT]= gdMacrotick[µs/MT] / pdMicrotick[µs/µT]= pMicroPerCycle[µT] / gMacroPerCycle[MT]
     */
    DIAG_SET(  hwPtr->PCR7.B.MICRO_PER_MACRO_NOM_HALF , (paramPtr->FrPMicroPerCycle / cCfg->FrClusterGMacroPerCycle) / 2 );     // IMPROVEMENT, so this is floor() and not round(pMicroPerMacroNom / 2)
    _debug_init_("MICRO_PER_MACRO_NOM_HALF=%d\n",hwPtr->PCR7.B.MICRO_PER_MACRO_NOM_HALF);
    DIAG_SET(  hwPtr->PCR7.B.DECODING_CORRECTION_B , paramPtr->FrPDecodingCorrection + paramPtr->FrPDelayCompensationB + 2u);

    /** PCR[8] */
    DIAG_SET(  hwPtr->PCR8.B.WAKEUP_SYMBOL_TX_IDLE  , cCfg->FrClusterGdWakeupTxIdle);
    DIAG_SET(  hwPtr->PCR8.B.MAX_WITHOUT_CLOCK_CORRECTION_FATAL , cCfg->FrClusterGMaxWithoutClockCorrectFatal);
    DIAG_SET(  hwPtr->PCR8.B.MAX_WITHOUT_CLOCK_CORRECTION_PASSIVE , cCfg->FrClusterGMaxWithoutClockCorrectPassive);

    /** PCR[9] */
    DIAG_SET(  hwPtr->PCR9.B.SYMBOL_WINDOW_EXISTS , (cCfg->FrClusterGdSymbolWindow != 0u) ? 1u : 0u);
    DIAG_SET(  hwPtr->PCR9.B.MINISLOT_EXISTS , ( cCfg->FrClusterGNumberOfMinislots != 0u) ? 1u : 0u);
    DIAG_SET(  hwPtr->PCR9.B.OFFSET_CORRECTION_OUT , paramPtr->FrPOffsetCorrectionOut);

    /** PCR[10] */
    DIAG_SET(  hwPtr->PCR10.B.MACRO_PER_CYCLE , cCfg->FrClusterGMacroPerCycle);
    _debug_init_("MACRO_PER_CYCLE=%d\n",hwPtr->PCR10.B.MACRO_PER_CYCLE);
    DIAG_SET(  hwPtr->PCR10.B.SINGLE_SLOT_ENABLED , paramPtr->FrPKeySlotOnlyEnabled);   /* Maps to pSingleSlotEnabled */
    DIAG_SET(  hwPtr->PCR10.B.WAKEUP_CHANNEL , (int)paramPtr->FrPWakeupChannel);

    /** PCR[11] */
    DIAG_SET(  hwPtr->PCR11.B.OFFSET_CORRECTION_START , paramPtr->FrPOffsetCorrectionStart);    /* Named gOffsetCorrectionStart in FlexRay 3.0  */
    DIAG_SET(  hwPtr->PCR11.B.KEY_SLOT_USED_FOR_STARTUP , paramPtr->FrPKeySlotUsedForStartup);
    DIAG_SET(  hwPtr->PCR11.B.KEY_SLOT_USED_FOR_SYNC , paramPtr->FrPKeySlotUsedForSync);

    /** PCR[12] */
    DIAG_SET(  hwPtr->PCR12.B.ALLOW_PASSIVE_TO_ACTIVE , paramPtr->FrPAllowPassiveToActive);

    /* Pre-calc header CRC
     *  - Sync frame ind + Startup Frame Ind + Frame ID + Payload Length -> 20 bits
     * */
    {
        uint32 keySlotIdx;
        Std_ReturnType status;
        uint32 crc;

        status = findTrigIdx(FrTrigConf, paramPtr->FrPKeySlotId, &keySlotIdx);
        if( status != E_OK ) {
            return E_NOT_OK;
        }
        crc = getHeaderCrc(Fr_Cfg, cIdx, keySlotIdx, FrTrigConf->FrTrigConfPtr[keySlotIdx].FrTrigLSduLength);

        DIAG_SET(  hwPtr->PCR12.B.KEY_SLOT_HEADER_CRC , crc);
        _debug_init_("KeySlotId=%d CRC=0x%04x\n", paramPtr->FrPKeySlotId, hwPtr->PCR12.B.KEY_SLOT_HEADER_CRC);
    }

    /** PCR[13] */
    DIAG_SET(  hwPtr->PCR13.B.STATIC_SLOT_AFTER_ACTION_POINT , cCfg->FrClusterGdStaticSlot - cCfg->FrClusterGdActionPointOffset - 1u);
    DIAG_SET(  hwPtr->PCR13.B.FIRST_MINISLOT_ACTION_POINT_OFFSET , MAX( cCfg->FrClusterGdActionPointOffset , cCfg->FrClusterGdMiniSlotActionPointOffset) - 1u);

    /** PCR[14] */
    DIAG_SET(  hwPtr->PCR14.B.LISTEN_TIMEOUT_H , (vuint16_t)_HIGH16(paramPtr->FrPdListenTimeout));
    DIAG_SET(  hwPtr->PCR14.B.RATE_CORRECTION_OUT , paramPtr->FrPRateCorrectionOut);

    /** PCR[15] */
    DIAG_SET(  hwPtr->PCR15.B.LISTEN_TIMEOUT_L , (vuint16_t)_LOW16(paramPtr->FrPdListenTimeout));

    /** PCR[16] */
    noiseListenTimeout = (cCfg->FrClusterGListenNoise * paramPtr->FrPdListenTimeout) - 1u;
    DIAG_SET(  hwPtr->PCR16.B.NOISE_LISTEN_TIMEOUT_H , (vuint16_t)_HIGH16(noiseListenTimeout));       /* (gListenNoise * pdListenTimeout) - 1 */
    DIAG_SET(  hwPtr->PCR16.B.MACRO_INITIAL_OFFSET_B , paramPtr->FrPMacroInitialOffsetB);

    /** PCR[17] */
    DIAG_SET(  hwPtr->PCR17.B.NOISE_LISTEN_TIMEOUT_L , (vuint16_t)_LOW16(noiseListenTimeout));

    /** PCR[18] */
    DIAG_SET(  hwPtr->PCR18.B.KEY_SLOT_ID , paramPtr->FrPKeySlotId);
    DIAG_SET(  hwPtr->PCR18.B.WAKEUP_PATTERN , paramPtr->FrPWakeupPattern);

    /** PCR[19] */
    DIAG_SET(  hwPtr->PCR19.B.PAYLOAD_LENGTH_STATIC , cCfg->FrClusterGPayloadLengthStatic);
    DIAG_SET(  hwPtr->PCR19.B.DECODING_CORRECTION_A , paramPtr->FrPDecodingCorrection + paramPtr->FrPDelayCompensationA + 2u);

    /** PCR[20] */
    DIAG_SET(  hwPtr->PCR20.B.MICRO_INITIAL_OFFSET_A , paramPtr->FrPMicroInitialOffsetA);
    DIAG_SET(  hwPtr->PCR20.B.MICRO_INITIAL_OFFSET_B , paramPtr->FrPMicroInitialOffsetB);
    /** PCR[21] */
    DIAG_SET(  hwPtr->PCR21.B.LATEST_TX , cCfg->FrClusterGNumberOfMinislots - paramPtr->FrPLatestTx);

    /*     *                                FR+RM              Autosar
     * EXTERN_RATE_CORRECTION      pExternRateCorrection    Nothing.....
     *
     * This is cluster sync parameters.. we just leave it at 0 (we are proably not syncing clusters) */

    DIAG_SET(  hwPtr->PCR21.B.EXTERN_RATE_CORRECTION , 0u);    /* pExternRateCorrection not found in Autosar */

    /** PCR[22] */
    DIAG_SET(  hwPtr->PCR22.B.COMP_ACCEPTED_STARTUP_RANGE_A , paramPtr->FrPdAcceptedStartupRange - paramPtr->FrPDelayCompensationA);
    DIAG_SET(  hwPtr->PCR22.B.MICRO_PER_CYCLE_H ,  (vuint16_t)_HIGH16(paramPtr->FrPMicroPerCycle));

    /** PCR[23] */
    DIAG_SET(  hwPtr->PCR23.B.micro_per_cycle_l , (vuint16_t)_LOW16(paramPtr->FrPMicroPerCycle));

    /** PCR[24] */
    DIAG_SET(  hwPtr->PCR24.B.MAX_PAYLOAD_LENGTH_DYNAMIC   , paramPtr->FrPPayloadLengthDynMax);
    DIAG_SET(  hwPtr->PCR24.B.CLUSTER_DRIFT_DAMPING        , paramPtr->FrPClusterDriftDamping);
    microPerCyleMin = paramPtr->FrPMicroPerCycle - paramPtr->FrPRateCorrectionOut;     /* pdMaxDrift was replaced by pRateCorrectionOut in FR3.0 */
    microPerCyleMax = paramPtr->FrPMicroPerCycle + paramPtr->FrPRateCorrectionOut;     /* pdMaxDrift was replaced by pRateCorrectionOut in FR3.0 */

    DIAG_SET(  hwPtr->PCR24.B.MICRO_PER_CYCLE_MIN_H , (vuint16_t)_HIGH16(microPerCyleMin));

    /** PCR[25] */
    DIAG_SET(  hwPtr->PCR25.B.MICRO_PER_CYCLE_MIN_L , (vuint16_t)_LOW16(microPerCyleMin));

    /** PCR[26] */
    DIAG_SET(  hwPtr->PCR26.B.ALLOW_HALT_DUE_TO_CLOCK , paramPtr->FrPAllowHaltDueToClock);
    DIAG_SET(  hwPtr->PCR26.B.COMP_ACCEPTED_STARTUP_RANGE_B , paramPtr->FrPdAcceptedStartupRange - paramPtr->FrPDelayCompensationB);
    DIAG_SET(  hwPtr->PCR26.B.MICRO_PER_CYCLE_MAX_H, (vuint16_t)_HIGH16(microPerCyleMax));

    /** PCR[27] */
    DIAG_SET(  hwPtr->PCR27.B.MICRO_PER_CYCLE_MAX_L , (vuint16_t)_LOW16(microPerCyleMax));

    /** PCR[28] */
    DIAG_SET(  hwPtr->PCR28.B.DYNAMIC_SLOT_IDLE_PHASE , cCfg->FrClusterGdDynamicSlotIdlePhase);
    DIAG_SET(  hwPtr->PCR28.B.MACRO_AFTER_OFFSET_CORRECTION , cCfg->FrClusterGMacroPerCycle - paramPtr->FrPOffsetCorrectionStart);

    /** PCR[29] */
    DIAG_SET(  hwPtr->PCR29.B.MINISLOTS_MAX , (cCfg->FrClusterGNumberOfMinislots - 1u));

    /* pExternOffsetCorrection in FR3.0, but not in recent autosar specs... set to 0 */
    DIAG_SET(  hwPtr->PCR29.B.EXTERN_OFFSET_CORRECTION , 0);

    /** PCR[30] */
    DIAG_SET(  hwPtr->PCR30.B.SYNC_NODE_MAX , cCfg->FrClusterGSyncFrameIDCountMax);

    if ( errCnt != 0u ) {
        rv = E_NOT_OK;
    }

    return rv;
}


/**
 * Setup the Message RAM and header for the configured FrIf triggering units.
 * @param Fr_Cfg
 * @param cIdx
 * @return
 */
Std_ReturnType Fr_Internal_SetupRxTxResources(const Fr_ContainerType *Fr_Cfg, uint8 cIdx) {


    /* Protocol Initialization
     * 1. Configure the Protocol Engine
     *   a. issue CONFIG command via Protocol Operation Control Register (FR_POCR)
     *   b. wait for POC:config in Protocol Status Register 0 (FR_PSR0)
     *   c. configure the FR_PCR0,…, FR_PCR30 registers to set all protocol parameters
     * 2. Configure the Message Buffers and FIFOs.
     *   a. set the number of message buffers used and the message buffer
     *       segmentation in the Message Buffer Segment Size and Utilization Register (FR_MBSSUTR)
     *   b. define the message buffer data size in the Message Buffer Data Size Register(FR_MBDSR)
     *   c. configure each message buffer by setting the configuration values in the
     *       Message Buffer Configuration, Control, Status Registers (FR_MBCCSRn),
     *       Message Buffer Cycle Counter Filter Registers (FR_MBCCFRn), Message
     *       Buffer Frame ID Registers (FR_MBFIDRn), Message Buffer Index Registers
     *       (FR_MBIDXRn)
     *   d. configure the FIFOs
     *   e. issue CONFIG_COMPLETE command via Protocol Operation Control Register
     *       (FR_POCR)
     *   f. wait for POC:ready in Protocol Status Register 0 (FR_PSR0)
     */
    const Fr_FrIfCCTriggeringType *trigCfg = &Fr_Cfg->Fr_ConfigPtr->FrTrigConfig[cIdx];
    const Fr_CtrlConfigParametersType *paramPtr = &Fr_Cfg->Fr_ConfigPtr->FrCtrlParam[cIdx];
    const Fr_FrIfTriggeringConfType *trigElem;
    const Fr_FrIfClusterConfigType *clCfg = &Fr_Cfg->Fr_ConfigPtr->FrClusterConfig[cIdx];

    Fr_Info.cfg = Fr_Cfg;

    volatile MSG_BUFF_CCS_t *mbPtr;
    struct FR_tag *hwPtr = Fr_HwReg[cIdx];
    uint32 n;
    uint32 headerIndex = 0;
    uint32 rv;
    Std_ReturnType pcrInitRv;
    uint8 ccErrCnt = 0;
    boolean isStatic;
    uint32 staticCnt = 0u;
    uint32 dynamicCnt = 0u;

    /* 1a) Transition to the POC:config state. */
    Fr_HwReg[cIdx]->POCR.R = (1u << (15u-0u)) |     /* WME */
                             (2u << (15u-15u));     /* POCCMD = CONFIG */

    /* 1b) POCR.B.BSY */
    rv = busyRead_1us( &Fr_HwReg[cIdx]->POCR.R, (1u<<(15u-8u)),0);
    if( rv != CMD_OK ) { return E_NOT_OK; }

    /* Wait until in POC:config */
    rv = busyRead_1us( &Fr_HwReg[cIdx]->PSR0.R, (7u<<(15u-7u)),(1u<<(15u-7u)));  /* PROTSTATE : 1 - POC:config */
    if( rv != CMD_OK ) { return E_NOT_OK; }


    /* 1c) */
#if (FR_CTRL_TEST_COUNT > 0)
    for(n = 0; n < FR_CTRL_TEST_COUNT; n++) {
        /* @req SWS_Fr_00647 */
        pcrInitRv = PCRInit(Fr_Cfg, hwPtr, cIdx);

        /* @req SWS_Fr_00598 */
        if (pcrInitRv == E_OK) {
            DEM_REPORT(Fr_Cfg->Fr_ConfigPtr->FrCtrlParam[cIdx].FrDemEventParamRef, DEM_EVENT_STATUS_PASSED);
            break;
        } else {
            ccErrCnt++;
        }
    }

    /* @req SWS_Fr_00147 */
    if (ccErrCnt == FR_CTRL_TEST_COUNT) {
        DEM_REPORT(Fr_Cfg->Fr_ConfigPtr->FrCtrlParam[cIdx].FrDemEventParamRef, DEM_EVENT_STATUS_FAILED);
        return E_NOT_OK;
    }
#endif


    /* The hardware have:
     * 1. Individual Message Buffers
     * 2. Receive Shadow Buffers
     * 3. Receive FIFO Buffers
     *
     * Of these we setup 1) and 2) and choose NOT to support 3)
     *
     * The FrIfFrameTriggering that holds the slot information = Message Box
     * so we set that up.
     */

    /* 2a) */

    /* Place all static buffers in segment 1, dynamic in segment 2 */
    hwPtr->MBSSUTR.B.LAST_MB_SEG1 = trigCfg->FrNbrTrigStatic - 1u;
    hwPtr->MBSSUTR.B.LAST_MB_UTIL = trigCfg->FrNbrTrigConfiged - 1u;

    /* 2b) */
    hwPtr->MBDSR.B.MBSEG1DS = clCfg->FrClusterGPayloadLengthStatic;
    hwPtr->MBDSR.B.MBSEG2DS = paramPtr->FrPPayloadLengthDynMax;

    _debug_init_("LAST_MB_SEG1 %d\n",hwPtr->MBSSUTR.B.LAST_MB_SEG1);
    _debug_init_("LAST_MB_UTIL %d\n",hwPtr->MBSSUTR.B.LAST_MB_UTIL);

    _debug_init_("MBSEG1DS %d\n",hwPtr->MBDSR.B.MBSEG1DS);
    _debug_init_("MBSEG2DS %d\n",hwPtr->MBDSR.B.MBSEG2DS);

    /** 2c) Configure Individial Message Buffers */

    /* precalc some values */

    uint32 baseAddr = ( (((uint32)hwPtr->SYSBADHR.R)<<16) + ((uint32)hwPtr->SYSBADLR.R) );
    uint32 hdrSize =  ((trigCfg->FrNbrTrigConfiged + SHADOW_BUFFER_CNT) * 8u);   /* Total number of headers  */
    uint32 seg1Size = (clCfg->FrClusterGPayloadLengthStatic * (trigCfg->FrNbrTrigStatic) * 2u );
    uint32 seg2Size = (ALIGN8(paramPtr->FrPPayloadLengthDynMax) * (trigCfg->FrNbrTrigConfiged - trigCfg->FrNbrTrigStatic) * 2u );

    _debug_init_("baseaddr  =0x%08x\n",baseAddr);
    _debug_init_("hdrSize   =0x%08x\n",hdrSize);
    _debug_init_("seg1Size  =0x%08x\n",seg1Size);
    _debug_init_("seg2Size  =0x%08x\n",seg2Size);


    for (n = 0; n < trigCfg->FrNbrTrigConfiged; n++) {
        _debug_init_("==== msg %d ====\n",n);

        /**
         * Add each trigger to a message box
         */
        trigElem = &trigCfg->FrTrigConfPtr[n];
        mbPtr = &hwPtr->MBCCS[n];

        /** MBCCSR */

        /* Check if enabled */
        if( mbPtr->MBCCSR.B.EDS ) {
            mbPtr->MBCCSR.B.EDT = 1u;       /* Trigger Enable->Disable */
        }

        if (trigElem->FrTrigIsTx ) {
            mbPtr->MBCCSR.B.MTD = 1u;       /* 0 - Rx, 1 - Tx */
        }
        _debug_init_("  isTx=%d\n",mbPtr->MBCCSR.B.MTD );

        /** MBCCFR */
        if (trigElem->FrTrigChannel == FR_CHANNEL_AB) {
            mbPtr->MBCCFR.B.CHNLA = 1u;
            mbPtr->MBCCFR.B.CHNLB = 1u;
        } else if (trigElem->FrTrigChannel == FR_CHANNEL_B) {
            mbPtr->MBCCFR.B.CHNLB = 1u;
        } else if (trigElem->FrTrigChannel == FR_CHANNEL_A) {
            mbPtr->MBCCFR.B.CHNLA = 1u;
        }

        /*
         * Event/State transmission.
         * This relates have relation to FrIfAlwaysTransmit...
         *   MTM:  0 - Message transmitted only once
         *   MTM:  1 - Message is transmitted continuously
         *
         * FrIfAlwaysTransmit     MTM
         * --------------------------------------------------------
         *        0                0   ->   Send NULL frame when no new data.
         *        0                1   ->   Send Latest data even if no new data.
         *        1                0   ->   Will send NO Null frames... just latest data.
         *        1                1   ->   Pointless, data will be updated.
         *
         * ---> It seems to be most sane to send only updated data, NULL frames otherwise,
         *      so, we keep MTM at 0.
         */

        mbPtr->MBCCFR.B.MTM  = 0u;   /* See above */

        /* It seems that cycle filtering can't be turned off in Autosar... so let's set it up
         * Logical expression: CYCCNT & MBCCFRn[CCFMSK] = MBCCFRn[CCFVAL] & MBCCFRn[CCFMSK]
         *
         * FrIfCycleRepetition - 1,2,4,8,16,32,64
         * FrIfBaseCycle       - 0 -- 63
         * */

        mbPtr->MBCCFR.B.CCFMSK = trigElem->FrTrigCycleRepetition - 1;
        mbPtr->MBCCFR.B.CCFVAL = trigElem->FrTrigBaseCycle;

        mbPtr->MBCCFR.B.CCFE = 1u;    /* 1 - Cycle counter filtering enabled */

        /** MBFIDRn */

        /* SlotID = FrameID */
        mbPtr->MBFIDR.B.FID = trigElem->FrTrigSlotId;
        _debug_init_("  SlotID=%d\n",mbPtr->MBFIDR.B.FID);

        /** MBIDXRn */

        /* MBIDXRn index to:
         * - Header    : SADR_MBHF= (FR_MBIDXn.MBIDX*8 + FR_SYMBADR.SMBA)
         * - Frame Data: SADR_MBDF= FR_MBDORn.MBDO + FR_SYMBADR.SMBA
         */
        hwPtr->MBCCS[n].MBIDXR.B.MBIDX = n;

        isStatic = slotIsStatic(clCfg, trigElem->FrTrigSlotId );


        uint32 offset;

        /* Calc payload addresses
         * After the headers (hdrSize) we have the payloads. Lets allocate static
         * buffers in one segment and dynamic in a section after that.
         *
         *  Headers
         *  Shadow Headers
         *  Static Payloads
         *  Dynamic Payloads
         * */
        if (isStatic) {
            /* Static (SEG1) starts after the headers */
            offset = hdrSize + (clCfg->FrClusterGPayloadLengthStatic * staticCnt * 2u);
            staticCnt++;

            _debug_init_("  Static\n");
        } else {
            /* Dynamic (SEG2) starts after SEG1 */
            offset = hdrSize +
                    seg1Size +
                    (ALIGN8(paramPtr->FrPPayloadLengthDynMax) * dynamicCnt  * 2u );

            dynamicCnt++;
            _debug_init_("  Dynamic\n");
        }

        trigCfg->FrMsgBufferCfg[n].FrDataPartitionAddr = baseAddr + offset;
        hwPtr->MBDOR[n].B.MBDO = offset;

        _debug_init_("  MBDO=0x%08x (0x%08x) (\n",hwPtr->MBDOR[n].B.MBDO, trigCfg->FrMsgBufferCfg[n].FrDataPartitionAddr );

        /* Not use so put in any data != 0 */
        // trigCfg->FrMsgBufferCfg[n].FrDataPartitionAddr = 1UL;
        /* Trigger Index = Buffer Index*/
        trigCfg->FrMsgBufferCfg[n].FrMsgBufferIdx = n;

        /* Pre-calculate CRC for header for static slots */
        {
            uint32 crc;
            MB_HEADER_t *hPtr;
            hPtr = getHeader(hwPtr,n);

            if (trigElem->FrTrigPayloadPreamble == true){
                hPtr->FRAME_HEADER.B.PPI = 1;
            }else{
                hPtr->FRAME_HEADER.B.PPI = 0;
            }
            if( isStatic ) {
                /* Calculate header from configuration... not on the header itself */
                crc = getHeaderCrc(Fr_Cfg,cIdx, n, 0u );    /* Length "0u" is don't care here since it's static */

                hPtr->FRAME_HEADER.B.HDCRC = crc;
                _debug_init_("  Header: Addr=0x%08x, CRC=0x%08x\n",(uint32)hPtr,crc);

            }
            else{ /*if dynamic frame*/
                /* Calculate header from configuration... not on the header itself */
                crc = getHeaderCrc(Fr_Cfg,cIdx, n, (trigElem->FrTrigLSduLength) );

                hPtr->FRAME_HEADER.B.HDCRC = crc;
                _debug_init_("  Header: Addr=0x%08x, CRC=0x%08x\n",(uint32)hPtr,crc);
            }

            /* Set FID and length */
            hPtr->FRAME_HEADER.B.FID = trigElem->FrTrigSlotId;
            hPtr->FRAME_HEADER.B.PLDLEN = (trigElem->FrTrigLSduLength>>1u);
            _debug_init_("  Header: FID=0x%08x, PLDLEN=0x%08x\n",hPtr->FRAME_HEADER.B.FID,hPtr->FRAME_HEADER.B.PLDLEN);
        }

        /* Enable buffer */
        mbPtr->MBCCSR.B.EDT = 1u;
    }

    headerIndex = trigCfg->FrNbrTrigConfiged;

    /**
     * Configure Receive Shadow Buffers
     *
     * RSBIR controls all the shadow buffers
     */

    Fr_Info.staticSlots = staticCnt;
    Fr_Info.dynamicSlots= dynamicCnt;

    /* Shadow Buffers
     *
     * Allocate the shadow buffers after headers and SEG1 and SEG2 buffers
     *
     * */
    uint32 sdataBuff  =   hdrSize + seg1Size + seg2Size;

    hwPtr->RSBIR.R = (0u << (15u-3u)) | (headerIndex) ;/* A, Seg 1 - SEL */
    hwPtr->MBDOR[headerIndex].B.MBDO = sdataBuff;
    trigCfg->FrMsgBufferCfg[headerIndex].FrDataPartitionAddr = baseAddr + sdataBuff;
    _debug_init_("Shadow A Seg1  MBDO=0x%08x, Index=%d\n",hwPtr->MBDOR[headerIndex].B.MBDO,headerIndex );
    headerIndex++;
    sdataBuff += (clCfg->FrClusterGPayloadLengthStatic * 2u);

    hwPtr->RSBIR.R = (2u << (15u-3u)) | (headerIndex) ;/* B, Seg 1 - SEL */
    hwPtr->MBDOR[headerIndex].B.MBDO = sdataBuff;
    trigCfg->FrMsgBufferCfg[headerIndex].FrDataPartitionAddr = baseAddr + sdataBuff;
    _debug_init_("Shadow B Seg1  MBDO=0x%08x, Index=%d\n",hwPtr->MBDOR[headerIndex].B.MBDO,headerIndex );
    headerIndex++;
    sdataBuff += (clCfg->FrClusterGPayloadLengthStatic * 2u);


    hwPtr->RSBIR.R = (1u << (15u-3u)) | (headerIndex) ;/* A, Seg 2 - SEL */
    hwPtr->MBDOR[headerIndex].B.MBDO = sdataBuff;
    trigCfg->FrMsgBufferCfg[headerIndex].FrDataPartitionAddr = baseAddr + sdataBuff;
    _debug_init_("Shadow A Seg2  MBDO=0x%08x, Index=%d\n",hwPtr->MBDOR[headerIndex].B.MBDO,headerIndex );
    headerIndex++;
    sdataBuff += (ALIGN8(paramPtr->FrPPayloadLengthDynMax) * 2u);

    hwPtr->RSBIR.R = (3u << (15u-3u)) | (headerIndex) ;/* B, Seg 2 - SEL */
    hwPtr->MBDOR[headerIndex].B.MBDO = sdataBuff;
    trigCfg->FrMsgBufferCfg[headerIndex].FrDataPartitionAddr = baseAddr + sdataBuff;
    _debug_init_("Shadow B Seg2  MBDO=0x%08x, Index=%d\n",hwPtr->MBDOR[headerIndex].B.MBDO,headerIndex );


    /* 2d) FIFO.... we don't use them */

    /* 2e) Issue CONFIG_COMPLETE */
    Fr_HwReg[cIdx]->POCR.R = (1u << (15u-0u)) |     /* WME */
                             (4u << (15u-15u));     /* POCCMD = CONFIG_COMPLETE */

    rv = busyRead_1us( &Fr_HwReg[cIdx]->POCR.R, (1u<<(15u-8u)),0u);
    if( rv != CMD_OK ) { return E_NOT_OK; }

    /* Wait until in POC:ready */
    rv = busyRead_1us( &Fr_HwReg[cIdx]->PSR0.R, (7u<<(15u-7u)),(3u<<(15u-7u)));  /* PROTSTATE : 3 - POC:ready */
    if( rv != CMD_OK ) { return E_NOT_OK; }

    return E_OK;
}

/**
 * Used to find the trigger index linked to the LPdu
 * @param FrTrigConf
 * @param searchedId
 * @param trigIdx
 * @return
 */
static Std_ReturnType findTrigIdx(const Fr_FrIfCCTriggeringType *FrTrigConf, uint32 searchedId, uint32 *trigIdx) {
    uint32 n;

    for (n = 0; n < FrTrigConf->FrNbrTrigConfiged; n++) {
        if (FrTrigConf->FrTrigConfPtr[n].FrTrigSlotId == searchedId) {
            *trigIdx = n;
            return E_OK;
        }
    }

    return E_NOT_OK;
}


/**
 * Updates the length of a message buffer
 * @param Fr_Cfg
 * @param cIdx
 * @param msgBuffrIdx
 * @return
 */
Std_ReturnType Fr_Internal_UpdateHeaderLength(const Fr_ContainerType *Fr_Cfg, uint8 cIdx, uint8 length, uint16 msgBufferIdx, uint32 frameTrigIdx) {
    Std_ReturnType retval = E_OK;
    struct FR_tag *hwPtr = Fr_HwReg[cIdx];
    MB_HEADER_t *hPtr;
    uint32 crc;

    /* This is for dynamic messages only.
     * Since the length is in the header we need to re-calc the header CRC.
     * 1. Write length is buffer.
     * 2. Re-calc header CRC
     */

    hPtr = getHeader(hwPtr,msgBufferIdx);

    /* 1) Write length */
    hPtr->FRAME_HEADER.B.PLDLEN = (length>>1u);

    /* 2) Re-calc header CRC */
    crc = getHeaderCrc(Fr_Cfg,cIdx, msgBufferIdx, length);

    hPtr->FRAME_HEADER.B.HDCRC = crc;

    return retval;
}


/**
 * Calculate checksum according to Flexray specification chapter 4.5.
 * @param Fr_Cfg
 * @param Fr_CtrlIdx
 * @param frameTrigIdx
 * @return
 */
/* !@req Flexray specification does not allow CC to calculate checksum */
static uint32 getHeaderCrc(const Fr_ContainerType *Fr_Cfg, uint8 Fr_CtrlIdx, uint32 frameTrigIdx, uint32 dynLen){
    const Fr_FrIfTriggeringConfType *FrTrigConf = &Fr_Cfg->Fr_ConfigPtr->FrTrigConfig[Fr_CtrlIdx].FrTrigConfPtr[frameTrigIdx];
    const Fr_FrIfClusterConfigType *clusterParamPtr = &Fr_Cfg->Fr_ConfigPtr->FrClusterConfig[Fr_CtrlIdx];
    const Fr_CtrlConfigParametersType *paramPtr = &Fr_Cfg->Fr_ConfigPtr->FrCtrlParam[Fr_CtrlIdx];

    // Data setup
    uint32 keySlotId = Fr_Cfg->Fr_ConfigPtr->FrCtrlParam[Fr_CtrlIdx].FrPKeySlotId;
    uint8  slotIsKey = (FrTrigConf->FrTrigSlotId == keySlotId) ? 1 : 0;

    uint32 syncFrameInd  = ((slotIsKey > 0) && paramPtr->FrPKeySlotUsedForSync) ? (uint32)1 : (uint32)0;
    uint32 startFrameInd = ((slotIsKey > 0) && paramPtr->FrPKeySlotUsedForStartup) ? (uint32)1 : (uint32)0;

    uint32 payloadLen = (FrTrigConf->FrTrigSlotId > clusterParamPtr->FrClusterGNumberOfStaticSlots) ? \
            (dynLen/FR_LENGTH_DIV) : clusterParamPtr->FrClusterGPayloadLengthStatic;

    uint32 data = (syncFrameInd << 19u)      |
            (startFrameInd << 18u)           |
            (FrTrigConf->FrTrigSlotId << 7u) |
            (payloadLen);

    _debug_init_("CRC info: TrigSlotId=%d, slotisKey=%d, sync=%d, start=%d, len=%d\n",FrTrigConf->FrTrigSlotId,slotIsKey,syncFrameInd, startFrameInd,payloadLen);

    //Checksum calculation
    uint32 vCrcPolynomial = V_CRC_POLYNOMIAL;
    sint32 vNextBitIdx;
    uint32 vCrcReg = V_CRC_INIT;
    uint32 vCrcNext;
    uint32 nextBit;

    for (vNextBitIdx = 19; vNextBitIdx >= 0; vNextBitIdx--) {

        nextBit =  (data >> (uint32)vNextBitIdx) & (uint32)0x01;
        vCrcNext = nextBit ^ ((vCrcReg >> (V_CRC_SIZE-1)) & (uint32)0x01);

        vCrcReg = vCrcReg << 1u;

        if (vCrcNext > 0) {
            vCrcReg = vCrcReg ^ vCrcPolynomial;
        }
    }

    return (uint32)(vCrcReg & (uint32)0x7FF);
}

/**
 * Clear pending status bits
 *
 * Reference manual chapter 21.3.14, the Host has access to the actual status and error information by reading registers
 * FLXAnFREIR, FLXAnFRSIR, FLXAnFROS, FLXAnFROTS and FLXAnFRITS.
 *
 * @param Fr_Cfg
 * @param cIdx
 */
void Fr_Arc_ClearPendingIsr(const Fr_ContainerType *Fr_Cfg, uint8 cIdx) {
}

/**
 * Disable all absolut timers
 * @param Fr_Cfg
 * @param cIdx
 */
void Fr_Internal_DisableAllTimers(const Fr_ContainerType *Fr_Cfg, uint8 cIdx) {
}

/**
 * Disable all interrupts
 *
 * @param Fr_Cfg
 * @param cIdx
 */
void Fr_Internal_DisableAllFrIsr(const Fr_ContainerType *Fr_Cfg, uint8 cIdx) {
}

/**
 * Run the configuration test n number of times and report status to the DEM.
 * @param cIdx
 * @return
 */
Std_ReturnType Fr_Internal_RunAllCCTest(const Fr_ContainerType *Fr_Cfg, uint8 cIdx) {
    Std_ReturnType retval = E_OK;
    Std_ReturnType status;
    uint8 ccErrCnt = 0;
    uint8 n;

#if (FR_CTRL_TEST_COUNT > 0)
    for(n = 0; n < FR_CTRL_TEST_COUNT; n++) {
        /* @req FR647 */
        status = Fr_Internal_RunCCTest(Fr_Cfg, cIdx);

        /* @req FR598 */
        if (status == E_OK) {
            DEM_REPORT(Fr_Cfg->Fr_ConfigPtr->FrCtrlParam[cIdx].FrDemEventParamRef, DEM_EVENT_STATUS_PASSED);
            break;
        } else {
            ccErrCnt++;
        }
    }

    /* @req FR147 */
    if (ccErrCnt == FR_CTRL_TEST_COUNT) {
        DEM_REPORT(Fr_Cfg->Fr_ConfigPtr->FrCtrlParam[cIdx].FrDemEventParamRef, DEM_EVENT_STATUS_FAILED);
        retval = E_NOT_OK;
    }
#endif

    return retval;
}

/**
 * Set the CHI command according to FLXAnFRSUCC1.CMD page 991 ref manual.
 * @param Fr_Cfg
 * @param cIdx
 * @param chiCmd
 * @return
 */
Std_ReturnType Fr_Internal_SetCtrlChiCmd(const Fr_ContainerType *Fr_Cfg, uint8 cIdx, uint8 chiCmd) {
    Std_ReturnType retval = E_OK;
    uint32 rv;

    Fr_HwReg[cIdx]->POCR.R = (1u << (15u-0u)) |     /* WME */
                             (chiCmd << (15u-15u)); /* POCCMD = chiCmd*/

    /* Wait for write cmd to complete, POCR.B.BSY */
    rv = busyRead_1us( &Fr_HwReg[cIdx]->POCR.R, (1u<<(15u-8u)),0);
    if( rv != CMD_OK ) { return E_NOT_OK; }

    /* Check for illegal transition */
    if( Fr_HwReg[cIdx]->PIFR1.B.IPCIF ) {

        /* @req SWS_Fr_00181, Fr_AllowColdstart */
        /* @req SWS_Fr_00186, Fr_HaltCommunication */
        /* @req SWS_Fr_00190, Fr_AbortCommunication */
        /* @req SWS_Fr_00195, Fr_SendWUP  */
        /* @req SWS_Fr_00201, Fr_SetWakeupChannel*/
        DEM_REPORT(Fr_Cfg->Fr_ConfigPtr->FrCtrlParam[cIdx].FrDemEventParamRef, DEM_EVENT_STATUS_FAILED);
        retval = E_NOT_OK;

        Fr_HwReg[cIdx]->PIFR1.B.IPCIF = 1u;     /* Clear error */
    }
    return retval;
}

/**
 * Will enable controller if its disabled.
 * @param Fr_Cfg
 * @param cIdx
 * @return
 */
Std_ReturnType Fr_Internal_EnableCtrl(const Fr_ContainerType *Fr_Cfg, uint8 cIdx) {


    return E_OK;
}

/**
 * Setup the CC according to all configuration parameters and run the CC test.
 * @param Fr_Cfg
 * @param cIdx
 * @return
 */
Std_ReturnType Fr_Internal_setupAndTestCC(Fr_ContainerType *Fr_Cfg, uint8 cIdx) {
    const Fr_CtrlConfigParametersType *paramPtr = &Fr_Cfg->Fr_ConfigPtr->FrCtrlParam[cIdx];
    const Fr_FrIfClusterConfigType *clusterParamPtr = &Fr_Cfg->Fr_ConfigPtr->FrClusterConfig[cIdx];

    uint32 tmp = 0;
    uint32 rv;
    Std_ReturnType setupRv = E_NOT_OK;

    /*
     * Make sure that we start from a clean slate
     */
    if( Fr_HwReg[cIdx]->MCR.B.MEN == 1ul ) {

        /* To POC:halt (since it's only there we can disable the module */
        Fr_HwReg[cIdx]->POCR.R = (1u << (15u-0u)) |                         /* WME */
                                 (POC_CMD_FREEZE << (15u-15u));     /* POCCMD  */

        /* Wait for write cmd to complete, POCR.B.BSY */
        rv = busyRead_1us( &Fr_HwReg[cIdx]->POCR.R, (1u<<(15u-8u)),0);
        if( rv != CMD_OK ) { return E_NOT_OK; }

        /* Back to POC:default_config (since it's only there we can disable the module */
        Fr_HwReg[cIdx]->POCR.R = (1u << (15u-0u)) |                         /* WME */
                                 (POC_CMD_DEFAULT_CONFIG << (15u-15u));     /* POCCMD  */

        /* Wait for write cmd to complete, POCR.B.BSY */
        rv = busyRead_1us( &Fr_HwReg[cIdx]->POCR.R, (1u<<(15u-8u)),0);
        if( rv != CMD_OK ) { return E_NOT_OK; }

        /* Disable module, for some bizarre reason it does not always bite the first time */
        {
            uint32 s = Timer_GetTicks();
            Fr_HwReg[cIdx]->MCR.B.MEN = 0;

            while( Fr_HwReg[cIdx]->MCR.B.MEN == 1 ) {
                if( TIMER_TICK2US(Timer_GetTicks() - s) > (50u) ) {
                    return E_NOT_OK;
                }

                Fr_HwReg[cIdx]->MCR.B.MEN = 0;
            }
        }

        /* Wait for write cmd to complete, POCR.B.BSY */
        rv = busyRead_1us( &Fr_HwReg[cIdx]->POCR.R, (1u<<(15u-8u)),0);
        if( rv != CMD_OK ) { return E_NOT_OK; }
    }

    /* Init step: (from "Initialization Sequence" in RM)
     *
     * Module Initialization
     * 1. Configure CC
     *  a. configure the control bits in the Module Configuration Register (FR_MCR)
     *  b. configure the system memory base address in System Memory
     *      Base Address Register (FR_SYMBADR)
     * 2.  Enable the CC.
     *  a. write 1 to the module enable bit MEN in the Module Configuration
     *     Register (FR_MCR)
     */


    /**
     * 1a) FR_MSR
     */

    if( Fr_HwReg[cIdx]->MCR.B.MEN == 0ul ) {
        MCR_t mcr;

        mcr.R = 0;

        /* If we would need single channel mode set mcr.B.SCM = 1 */

        /* FR_MCR : CHB and CHA (FlexRay Channel B Enable and FlexRay Channel A Enable) */
        if (paramPtr->FrPChannels == FR_CHANNEL_AB) {
            mcr.B.CHA = 1;
            mcr.B.CHB = 1;
        } else if (paramPtr->FrPChannels == FR_CHANNEL_B) {
            mcr.B.CHB = 1;
        } else if (paramPtr->FrPChannels == FR_CHANNEL_A) {
            mcr.B.CHA = 1;
        } else {
            /* All Channels disabled. */
        }

        /* FR_MCR : SFFE (Synchronization Frame Filter Enable) */
#if 0
        if ( Fr_DriverSpecificConfigPtr->Fr_SynchronizationFrameFiltering) {
            u16Temp |= FR_MCR_SFFE;
        }


        /* FR_MCR : CLKSEL (Protocol Engine Clock Source Select) */
        /* FR_BASE_CLOCK_PLL = The clock source is system clock / 3. This should only be used if the system frequency is 120 MHz. */
        /* Else, the clock source is an on-chip crystal oscillator. */
        if (Fr_DriverSpecificConfigPtr->Fr_ClockSource == FR_BASE_CLOCK_PLL) {
            u16Temp |= FR_MCR_CLKSEL;
        }
#else
        mcr.B.SFFE = 0;         /* 0 - Disable frame filtering
                                 *     (IMPROVEMENT: check this )*/
        mcr.B.CLKSEL = 0;       /* 0 - PE clock source is generated by on-chip crystal oscillator.
                                 *     (IMPROVEMENT: check this ) */
#endif

        /* gdBit defines the bitrate */
        switch(clusterParamPtr->FrClusterGdBit) {
        case FR_T100NS: /* 10  Mbits/s */
            tmp = 0;
            break;
        case FR_T200NS: /* 5   Mbits/s */
            tmp = 1;
            break;
        case FR_T400NS: /* 2.5 Bbits/s */
            tmp = 2;
            break;
        default:
            break;
        }

        mcr.B.BITRATE = tmp;
        _debug_init_("bdBit=%d\n",mcr.B.BITRATE);

        /* Save the FR_MCR configuration into its register. */
        Fr_HwReg[cIdx]->MCR.R = mcr.R;

        /**
         * 1b) configure the system memory base address in System Memory Base Address Register (FR_SYMBADR)
         */
        Fr_HwReg[cIdx]->SYSBADHR.R = (((uint32)(&Fr_MsgBuffers))>>16u);
        Fr_HwReg[cIdx]->SYSBADLR.R = (((uint32)(&Fr_MsgBuffers)) & 0xffffu );

        /**
         * 2a) Enable the FlexRay module.
         * */
        Fr_HwReg[cIdx]->MCR.B.MEN = 1;

        /* Normal Mode: The protocol engine is now in its default
         configuration state (POC:default config). */

        /* Wait for write cmd to complete, POCR.B.BSY */
        rv = busyRead_1us( &Fr_HwReg[cIdx]->POCR.R, (1u<<(15u-8u)),0);
        if( rv != CMD_OK ) { return E_NOT_OK; }


        setupRv = Fr_Internal_SetupRxTxResources(Fr_Cfg,cIdx);

    }

    return setupRv;
}

Fr_POCStateType Fr_Internal_GetProtState( const Fr_ContainerType *Fr_Cfg, uint8 cIdx ) {
    struct FR_tag *hwPtr = Fr_HwReg[cIdx];
    Fr_POCStateType state;

    const Fr_POCStateType ProtStateList[] = {   FR_POCSTATE_DEFAULT_CONFIG,
                                                FR_POCSTATE_CONFIG,
                                                FR_POCSTATE_WAKEUP,
                                                FR_POCSTATE_READY,
                                                FR_POCSTATE_NORMAL_PASSIVE,
                                                FR_POCSTATE_NORMAL_ACTIVE,
                                                FR_POCSTATE_HALT,
                                                FR_POCSTATE_STARTUP };
    if( hwPtr->PSR1.B.FRZ ) {
        state = FR_POCSTATE_HALT;
    } else {
        state = ProtStateList[hwPtr->PSR0.B.PROTSTATE];
    }

    return state;
}


/**
 * Read chi poc status.
 * @param Fr_Cfg
 * @param cIdx
 * @param Fr_POCStatusPtr
 * @return E_OK or E_NOT_OK
 */
Std_ReturnType Fr_Internal_GetChiPocState(const Fr_ContainerType *Fr_Cfg, uint8 cIdx, Fr_POCStatusType* Fr_POCStatusPtr) {

    struct FR_tag *hwPtr = Fr_HwReg[cIdx];

    Fr_POCStatusType pocStatus = {0};

    uint32 errMode =    hwPtr->PSR0.B.ERRMODE;
    uint32 slotMode =   hwPtr->PSR0.B.SLOTMODE;

    pocStatus.CHIHaltRequest =  hwPtr->PSR1.B.HHR ? TRUE : FALSE;
    pocStatus.ColdstartNoise =  hwPtr->PSR1.B.CPN ? TRUE : FALSE;
    pocStatus.Freeze =          hwPtr->PSR1.B.FRZ ? TRUE : FALSE;

    const Fr_SlotModeType SlotModeList[] = {   FR_SLOTMODE_KEYSLOT,
                                               FR_SLOTMODE_ALL_PENDING,
                                               FR_SLOTMODE_ALL,
    };

    if ( slotMode > 2u) {
        DEM_REPORT(Fr_Cfg->Fr_ConfigPtr->FrCtrlParam[cIdx].FrDemEventParamRef, DEM_EVENT_STATUS_FAILED);
        return E_NOT_OK;
    }

    pocStatus.SlotMode = SlotModeList[slotMode];

    const Fr_ErrorModeType ErrModeList[] = {  FR_ERRORMODE_ACTIVE,
                                              FR_ERRORMODE_PASSIVE,
                                              FR_ERRORMODE_COMM_HALT };

    if ( errMode > 2u) {
        DEM_REPORT(Fr_Cfg->Fr_ConfigPtr->FrCtrlParam[cIdx].FrDemEventParamRef, DEM_EVENT_STATUS_FAILED);
        return E_NOT_OK;
    }

    pocStatus.ErrorMode = ErrModeList[errMode];

    pocStatus.State = Fr_Internal_GetProtState(Fr_Cfg,cIdx);

    const Fr_StartupStateType StartupList[] = { FR_STARTUP_UNDEFINED,
                                                FR_STARTUP_UNDEFINED,
                                                FR_STARTUP_COLDSTART_COLLISION_RESOLUTION,
                                                FR_STARTUP_COLDSTART_LISTEN,
                                                FR_STARTUP_INTEGRATION_CONSISTENCY_CHECK,
                                                FR_STARTUP_INTEGRATION_LISTEN,
                                                FR_STARTUP_UNDEFINED,
                                                FR_STARTUP_INITIALIZE_SCHEDULE,
                                                FR_STARTUP_UNDEFINED,
                                                FR_STARTUP_UNDEFINED,
                                                FR_STARTUP_COLDSTART_CONSISTENCY_CHECK,
                                                FR_STARTUP_UNDEFINED,
                                                FR_STARTUP_UNDEFINED,
                                                FR_STARTUP_INTEGRATION_COLDSTART_CHECK,
                                                FR_STARTUP_COLDSTART_GAP,
                                                FR_STARTUP_COLDSTART_JOIN };

    pocStatus.StartupState = StartupList[hwPtr->PSR0.B.STARTUPSTATE];

    const Fr_WakeupStatusType WakeupList[] = { FR_WAKEUP_UNDEFINED,
                                               FR_WAKEUP_RECEIVED_HEADER,
                                               FR_WAKEUP_RECEIVED_WUP,
                                               FR_WAKEUP_COLLISION_HEADER,
                                               FR_WAKEUP_COLLISION_WUP,
                                               FR_WAKEUP_COLLISION_UNKNOWN,
                                               FR_WAKEUP_TRANSMITTED,
                                               FR_WAKEUP_UNDEFINED };

    pocStatus.WakeupStatus = WakeupList[hwPtr->PSR0.B.WAKEUPSTATUS];

    //Successful readout, set output values.
    Fr_POCStatusPtr->CHIHaltRequest = pocStatus.CHIHaltRequest;
    Fr_POCStatusPtr->ColdstartNoise = pocStatus.ColdstartNoise;
    Fr_POCStatusPtr->ErrorMode      = pocStatus.ErrorMode;
    Fr_POCStatusPtr->Freeze         = pocStatus.Freeze;
    Fr_POCStatusPtr->SlotMode       = pocStatus.SlotMode;
    Fr_POCStatusPtr->StartupState   = pocStatus.StartupState;
    Fr_POCStatusPtr->State          = pocStatus.State;
    Fr_POCStatusPtr->WakeupStatus   = pocStatus.WakeupStatus;

    return E_OK;
}

/**
 * Set data to transmit
 * @param Fr_Cfg
 * @param cIdx
 * @param Fr_LSduPtr
 * @param Fr_LSduLength
 * @param Fr_MsgBuffrIdx
 * @return
 */
Std_ReturnType Fr_Internal_SetTxData(const Fr_ContainerType *Fr_Cfg, uint8 cIdx, const uint8* Fr_LSduPtr, uint8 Fr_LSduLength, uint16 msgBufIdx) {

    struct FR_tag *hwPtr = Fr_HwReg[cIdx];
    volatile MSG_BUFF_CCS_t *mbPtr = &hwPtr->MBCCS[msgBufIdx];
    uint8 *payloadPtr;
    uint16 data;
    Std_ReturnType rv = E_OK;

    /*
     * Write header and payload data. This is heavily inspired by Freescale unified driver as
     * the RM... is not that good in this area.
     */

    /* Lock MB */
    data = mbPtr->MBCCSR.R;
    data |= (1u<<(15u-6u));       // LCKT = 1
    data &= ~(1u<<(15u-5u));      // EVB = 0;
    mbPtr->MBCCSR.R = data;

    /* Read and check if we got lock */
    if( mbPtr->MBCCSR.B.LCKS ) {

        /* Clear flags */
        mbPtr->MBCCSR.B.MBIF = 1u;

        /* For Tx Frame Header struct active fields are FID, PLDLEN and HDCRC
         * ..however these are already written at startup for static messages.
         */

        const Fr_FrIfClusterConfigType *clCfg = &Fr_Cfg->Fr_ConfigPtr->FrClusterConfig[cIdx];
        const Fr_FrIfCCTriggeringType *trigCfg = &Fr_Cfg->Fr_ConfigPtr->FrTrigConfig[cIdx];

        payloadPtr = getPayload(trigCfg,msgBufIdx);
        uint32 padLength;


        if( slotIsStatic(clCfg, trigCfg->FrTrigConfPtr[msgBufIdx].FrTrigSlotId ) ) {
            /* Note!
             * For static data:
             *  - Seems that additional data should be padded with 0x0000
             *   "if gPayloadLengthStatic > vTCHI!Length then the vTCHI!Length number of two-byte
             *    payload words shall be copied from vTCHI!Message to vTF!Payload. The
             *    remaining  (gPayloadLengthStatic - vTCHI!Length) two-byte payload words in vTF!Payload
             *    shall be set to the padding pattern 0x0000"
             */
            /* Fr_LSduLength should match cluster config */
            assert( (Fr_LSduLength>>1u) <= hwPtr->PCR19.B.PAYLOAD_LENGTH_STATIC );

            padLength = (hwPtr->PCR19.B.PAYLOAD_LENGTH_STATIC * 2u - Fr_LSduLength);
        } else {
            /* Dynamic segment
             * The function Fr_Internal_UpdateHeaderLength() should already
             * setup the header (CRC, etc)
             * */
            assert( (Fr_LSduLength>>1u) <= hwPtr->PCR24.B.MAX_PAYLOAD_LENGTH_DYNAMIC );
            padLength = 0; // (trigCfg->FrTrigConfPtr[msgBufIdx].FrTrigLSduLength - Fr_LSduLength);
        }

        /* Copy data */
        memcpy(payloadPtr,  Fr_LSduPtr, Fr_LSduLength );
        /* pad */
        if( padLength != 0u ) {
            memset(payloadPtr+Fr_LSduLength-padLength,0,padLength);
        }

        _debug_tx_("-----> TX\n");

        /* Commit */
        mbPtr->MBCCSR.B.CMT = 1;

        /* Unlock */
        mbPtr->MBCCSR.B.LCKT = 1u;

    } else {
        /* No lock on the buffer */
        rv = E_NOT_OK;
    }

    return rv;
}

/**
 * Check if the index message buffer have received any new data.
 * @param Fr_Cfg
 * @param cIdx
 * @param msgBufferIdx
 * @return
 */
Std_ReturnType Fr_Internal_CheckNewData(const Fr_ContainerType *Fr_Cfg, uint8 cIdx, uint16 msgBufIdx) {

    struct FR_tag *hwPtr = Fr_HwReg[cIdx];
    volatile MSG_BUFF_CCS_t *mbPtr = &hwPtr->MBCCS[msgBufIdx];
    Std_ReturnType rv = E_NOT_OK;

    /*
     * A table called "Receive Message Buffer Update" describes the status
     * bits to look at.
     */
    if( mbPtr->MBCCSR.B.DUP  ) {
        /* Valid non-null frame */
        rv = E_OK;
    } else {
        /* May have receviced a NULL frame, but we don't really care
         * since Autosar SWS_Fr_00236 "The  function  Fr_ReceiveRxLPdu  shall  ensure  that
         *  FR_RECEIVED is returned only for non-Nullframes".
         *
         *  IMPROVEMENT: Do we need to ack anywhy here
         */
    }

    return rv;
}

/**
 * Copy data from the message ram
 * @param Fr_Cfg
 * @param cIdx
 * @param msgBufferIdx
 * @param Fr_LSduLengthPtr
 * @return
 * See header file.
 */
Std_ReturnType Fr_Internal_GetNewData(  const Fr_ContainerType *Fr_Cfg, uint8 cIdx, uint32 trigIdx, uint16 msgBufIdx,
                                        uint8* Fr_LSduPtr, uint8* Fr_LSduLengthPtr) {

    struct FR_tag *hwPtr = Fr_HwReg[cIdx];
    const Fr_FrIfCCTriggeringType *trigCfg = &Fr_Cfg->Fr_ConfigPtr->FrTrigConfig[cIdx];
    volatile MSG_BUFF_CCS_t *mbPtr = &hwPtr->MBCCS[msgBufIdx];
    MB_HEADER_t *hPtr;
    uint16 newIndex;
    uint8 *payloadPtr;

    /* The natural way would be to read the header directly, however from the RM
     * regarding shadow buffers:
     *
     * "This means that the message buffer area in the FlexRay memory area
     * accessed by the application for reading the received message is different
     * from the initial setting of the message buffer. Therefore, the application
     * must not rely on the index information written initially into the Message
     * Buffer Index Registers (FR_MBIDXRn). Instead, the index of the message
     * buffer header field must be fetched from the Message Buffer Index
     * Registers (FR_MBIDXRn)."
     *
     * --> Status is held in the HW message boxes, get header and payload from index.
     */

    /* Check that it's updated */
    if( (mbPtr->MBCCSR.B.DUP == 1u)  && (mbPtr->MBCCSR.B.MBIF == 1u) ) {

        /* Lock request */
        mbPtr->MBCCSR.B.LCKT = 1u;

        /* Check that we actually locked it
         * (Contradictions?!:  "Received Message Access" state that
         *   receive message buffers must be in state HDis, HDisLck and HLck..
         *   but on the other hand table "Receive Message Buffer States and Accesses"
         *   states that is should be read in HLckCCRx )
         * */
        if( mbPtr->MBCCSR.B.LCKS == 1u ) {

            /*-- Find message header/body  --*/

            /* Get the message box index first */
            newIndex = mbPtr->MBIDXR.B.MBIDX;
            /* Get header for that index */
            hPtr = getHeader(hwPtr,newIndex);

            *Fr_LSduLengthPtr = (hPtr->FRAME_HEADER.B.PLDLEN<<1u);

            payloadPtr = getPayload(trigCfg,newIndex);

            /* Copy SDU */
            memcpy(Fr_LSduPtr, payloadPtr,*Fr_LSduLengthPtr );

            /* -- Do some sanity checks (check valid frame) -- */

#if defined(CFG_FR_CHECK_BAD_SLOT_STATUS)
            /* This check is NOT verified */

            /* get channel assignments, CHA and CHB */
            uint8 chIdx = (((mbPtr->MBCCFR.R & 0x6000u) >> 13u) & 0x3u);

            /* Assign S_STATUS_t values in order:
             * CHA CHB
             *  0   0  Not valid
             *  0   1  B
             *  1   0  A
             *  1   1  Both
             */

            const uint16 slotStatus[4u] = { 0, 0x8000u, 0x0080u, 0x8080u };

            // Check valid frame VFB (bit 0) and VFA (bit 8)
            // In future include to check slot status register
            if ( (hPtr->SLOT_STATUS.R & slotStatus[chIdx]) != slotStatus[chIdx]) {
                while(1) {} /* : bad */
            }
#endif

            /*Clear Interrupt flag w1c*/
            mbPtr->MBCCSR.B.MBIF = 1u;
            /*Unlock buffer*/
            mbPtr->MBCCSR.B.LCKT = 1u;

        }
    }

    /* IMPROVEMENT: lots of checks in the code above. */

    return E_OK;
}

/**
 * Checks the message buffer status
 * From Fr_CheckTxLPduStatus
 *
 * @param Fr_Cfg
 * @param cIdx
 * @param msgBufferIdx
 * @return
 */
Std_ReturnType Fr_Internal_GetTxPending(const Fr_ContainerType *Fr_Cfg, uint8 cIdx, uint32 trigIdx, boolean *txPending) {
    struct FR_tag *hwPtr = Fr_HwReg[cIdx];
    volatile MSG_BUFF_CCS_t *mbPtr = &hwPtr->MBCCS[trigIdx];        /* IMPROVEMENT: Use mailbox here instead * */

    /* Check if something is commited for transmission */
    if(mbPtr->MBCCSR.B.CMT) {
        *txPending = TRUE;
    } else {
        *txPending = FALSE;
    }
    return E_OK;

}

/**
 * Set the FLXAnFRIBCM.STXRH to 0 for the indexed message buffer.
 * @param Fr_Cfg
 * @param cIdx
 * @param msgBufferIdx
 * @return
 */
Std_ReturnType Fr_Internal_CancelTx(const Fr_ContainerType *Fr_Cfg, uint8 cIdx, uint16 msgBufferIdx) {

    return E_OK;
}

/**
 * Set the wakeup channel
 * @param Fr_Cfg
 * @param cIdx
 * @param Fr_ChnlIdx
 * @return
 */
Std_ReturnType Fr_Internal_SetWUPChannel(const Fr_ContainerType *Fr_Cfg, uint8 cIdx, Fr_ChannelType Fr_ChnlIdx) {
    return E_OK;
}

/**
 * Read out the global time from hw register
 * @param Fr_Cfg
 * @param cIdx
 * @param Fr_CyclePtr
 * @param Fr_MacroTickPtr
 * @return
 */
Std_ReturnType  Fr_Internal_GetGlobalTime(const Fr_ContainerType *Fr_Cfg, uint8 cIdx, uint8* Fr_CyclePtr, uint16* Fr_MacroTickPtr) {
    Std_ReturnType rv = E_NOT_OK;

    struct FR_tag *hwPtr = Fr_HwReg[cIdx];

    if( Fr_Internal_IsSynchronous(Fr_Cfg, cIdx)  == E_OK )
    {
        *Fr_CyclePtr = hwPtr->CYCTR.R;      /* Cycle counter */
        *Fr_MacroTickPtr = hwPtr->MTCTR.R;  /* Macro counter */
        rv = E_OK;
    }

    return rv;
}

/**
 * Checks error status register if the unit is synchronous or not.
 * @param Fr_Cfg
 * @param cIdx
 * @return
 */
Std_ReturnType Fr_Internal_IsSynchronous(const Fr_ContainerType *Fr_Cfg, uint8 cIdx) {

    Fr_POCStateType state = Fr_Internal_GetProtState(Fr_Cfg,cIdx);
    Std_ReturnType rv = E_NOT_OK;

    /* From Autosar FR spec:
     * A FlexRay CC is considered synchronized, to the FlexRay cluster connected to, as
     * long as the following condition holds true:
     * ((!vPOC!Freeze) && (vPOC!State == NORMAL_ACTIVE) || (vPOC!State == NORMAL_PASSIVE))
     */
    if( ((state != FR_POCSTATE_HALT) && ( state == FR_POCSTATE_NORMAL_ACTIVE )) ||
         ( state == FR_POCSTATE_NORMAL_PASSIVE ) )
    {
        rv= E_OK;
    }

    return rv;
}

/**
 * Absolute timer setup
 *
 * @param Fr_Cfg
 * @param cIdx
 * @param Fr_AbsTimerIdx
 * @param Fr_Cycle
 * @param Fr_Offset
 * @return
 */
void Fr_Internal_SetupAbsTimer(const Fr_ContainerType *Fr_Cfg, uint8 cIdx, uint8 Fr_AbsTimerIdx, uint8 Fr_Cycle, uint16 Fr_Offset) {
    struct FR_tag *hwPtr = Fr_HwReg[cIdx];

    /* We have 2 timers, set them up:
     *
     * The HW have more capabilities (CYCMSK and Repeat) than the API in this case
     * so set the TI1CYCMSK to exact match (0x3f)
     */
    if( Fr_AbsTimerIdx == 0U) {
        hwPtr->TICCR.B.T1SP = 1u;   /* Stop Timer */

        hwPtr->TI1CYSR.B.TI1CYCVAL = Fr_Cycle;
        hwPtr->TI1CYSR.B.TI1CYCMSK = 0x3FU;

        hwPtr->T1MTOR.R=Fr_Offset; /*Timer 1 Macrotic Offset*/
        hwPtr->TICCR.B.T1REP = 0u;  /* Set Repeat */
        hwPtr->TICCR.B.T1TR = 1u;   /* Start Timer */

    } else if( Fr_AbsTimerIdx == 1U) {

        hwPtr->TICCR.B.T2SP = 1u;   /* Stop Timer */

        hwPtr->TI2CR0.B.TI2CYCVAL = Fr_Cycle;
        hwPtr->TI2CR0.B.TI2CYCMSK = 0x3FU;

        hwPtr->TI2CR1.R = Fr_Offset;  /* IMPROVEMENT, some offset not in timer1 ?? */

        hwPtr->TICCR.B.T2REP = 0u;  /* Set Repeat */
        hwPtr->TICCR.B.T2TR = 1u;   /* Start Timer */
    } else {
        assert(0);
    }
}

/**
 * Set the timer to halt.
 * From Fr_CancelAbsoluteTimer
 *
 * @param Fr_Cfg
 * @param cIdx
 * @param Fr_AbsTimerIdx
 */
void Fr_Internal_DisableAbsTimer(const Fr_ContainerType *Fr_Cfg, uint8 cIdx, uint8 Fr_AbsTimerIdx) {
    struct FR_tag *hwPtr = Fr_HwReg[cIdx];

    if( Fr_AbsTimerIdx == 0U) {
        hwPtr->TICCR.B.T1SP = 1u;   /* Stop Timer */
    } else if( Fr_AbsTimerIdx == 1U) {
        hwPtr->TICCR.B.T2SP = 1u;   /* Stop Timer */
    } else {
        assert(0);
    }
}

/**
 * Returns the absolute timer status
 * From Fr_GetAbsoluteTimerIRQStatus
 *
 * @param Fr_Cfg
 * @param cIdx
 * @param Fr_AbsTimerIdx
 * @return
 */
boolean Fr_Internal_GetAbsTimerIrqStatus(const Fr_ContainerType *Fr_Cfg, uint8 cIdx, uint8 Fr_AbsTimerIdx) {

    boolean rv = FALSE;
    struct FR_tag *hwPtr = Fr_HwReg[cIdx];

    if( Fr_AbsTimerIdx == 0U) {
         if( hwPtr->PIFR0.B.TI1IF ) {
             rv = TRUE;
         }
     } else if (Fr_AbsTimerIdx == 1U) {
         if( hwPtr->PIFR0.B.TI2IF  ) {
             rv = TRUE;
         }
     } else {
         assert(0);
     }

    return rv;
}

/**
 * Clear the interrupt flag for the selected absolute timer
 * Comes from Fr_AckAbsoluteTimerIRQ
 *
 * @param Fr_Cfg
 * @param cIdx
 * @param Fr_AbsTimerIdx
 */
void Fr_Internal_ResetAbsTimerIsrFlag(const Fr_ContainerType *Fr_Cfg,
        uint8 cIdx, uint8 Fr_AbsTimerIdx) {

    struct FR_tag *hwPtr = Fr_HwReg[cIdx];

    if (Fr_AbsTimerIdx == 0U) {
        hwPtr->PIFR0.B.TI1IF = 1u;
        while (hwPtr->PIFR0.B.TI1IF == 1u) {};

    } else if (Fr_AbsTimerIdx == 1U) {
        hwPtr->PIFR0.B.TI2IF = 1u;
        while (hwPtr->PIFR0.B.TI2IF == 1u) {};
    } else {
        assert(0);
    }
}

/**
 * Disables the interrupt line to the absolute timer.
 * @param Fr_Cfg
 * @param cIdx
 * @param Fr_AbsTimerIdx
 */
void Fr_Internal_DisableAbsTimerIrq(const Fr_ContainerType *Fr_Cfg, uint8 cIdx, uint8 Fr_AbsTimerIdx) {

    struct FR_tag *hwPtr = Fr_HwReg[cIdx];

    if( Fr_AbsTimerIdx == 0U) {
        hwPtr->PIER0.B.TI1IE = 0u;
    } else if (Fr_AbsTimerIdx == 1U) {
        hwPtr->PIER0.B.TI2IE = 0u;
    } else {
        assert(0);
    }
}

/**
 * Enables the interrupt line to the absolute timer.
 * @param Fr_Cfg
 * @param cIdx
 * @param Fr_AbsTimerIdx
 */
void Fr_Internal_EnableAbsTimerIrq(const Fr_ContainerType *Fr_Cfg, uint8 cIdx, uint8 Fr_AbsTimerIdx) {

    struct FR_tag *hwPtr = Fr_HwReg[cIdx];

    if( Fr_AbsTimerIdx == 0U) {
        hwPtr->PIER0.B.TI1IE = 1u;
    } else if (Fr_AbsTimerIdx == 1U) {
        hwPtr->PIER0.B.TI2IE = 1u;
    } else {
        assert(0);
    }

    /* All FR_PIER0 flags routed through this gate, enable */
    hwPtr->GIFER.B.PRIE = 1u;
}

/**
 * Used to wrap the call to DEM.
 * @param eventId
 * @param eventStatus
 */
#if defined(USE_DEM) || defined(CFG_FR_DEM_TEST)
static inline void Fr_Internal_reportDem(Dem_EventIdType eventId, Dem_EventStatusType eventStatus) {

    /* @req FR628 */
    /* @req FR630 */
    if (eventId != DEM_EVENT_ID_NULL) {
        /* @req FR028 */
        Dem_ReportErrorStatus(eventId, eventStatus);
    }
}
#endif




void Fr_PrintInfo( void ) {
    uint32 cIdx = 0;
    const Fr_FrIfClusterConfigType *cCfg = &Fr_Info.cfg->Fr_ConfigPtr->FrClusterConfig[cIdx];
    const Fr_CtrlConfigParametersType *pPtr = &Fr_Info.cfg->Fr_ConfigPtr->FrCtrlParam[cIdx];
    const Fr_FrIfClusterConfigType *clCfg = &Fr_Info.cfg->Fr_ConfigPtr->FrClusterConfig[cIdx];


    printf("bitRate:         %d\n",111);
    printf("cycle:           %d\n",111);
    printf("gMacroPerCycle:  %d\n", cCfg->FrClusterGMacroPerCycle);
    printf("pMicroPerCycle:  %d\n", pPtr->FrPMicroPerCycle);
//    printf("gdBit:           %d\n", cCfg->FrClusterGdBit);
    printf("gdCycle:         %4d [us] Duration of the cycle \n", (uint32)(cCfg->FrClusterGdCycle * 1000000));
    printf("gdMacrotick:     %4d [us] Duration of macrotick \n", (uint32)(cCfg->FrClusterGdMacrotick * 1000000));
    printf("pdMicrotick:     %d (25ns for 10Mbit)...0 in config\n", pPtr->FrPdMicrotick);
    printf("static Slots:    %d (%d)\n",Fr_Info.staticSlots, clCfg->FrClusterGNumberOfStaticSlots );
    printf("macrotick/static slot:    %d\n",cCfg->FrClusterGdStaticSlot);
    printf("payload length       :    %d\n",cCfg->FrClusterGPayloadLengthStatic);
    printf("dynamic Slots:   %d (%d)\n",Fr_Info.dynamicSlots, cCfg->FrClusterGNumberOfMinislots);
    printf("macrotick/dynamic slot:   %d\n",cCfg->FrClusterGdMinislot);
    printf("\n\n");

    printf("pMicroPerMacroNom:   %d\n",(pPtr->FrPMicroPerCycle / (cCfg->FrClusterGMacroPerCycle )));

}


