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


#ifndef FR_INTERNAL_H_
#define FR_INTERNAL_H_

#include "Fr.h"

#if defined(CFG_RH850)
#include "Fr_rh850f1x.h"
#else
#include "Fr_mpc5xxx.h"
#endif

#define FR_POC_CMD_ALLOW_COLDSTART      POC_CMD_ALLOW_COLDSTART
#define FR_POC_CMD_ALL_SLOTS            POC_CMD_ALL_SLOTS
#define FR_POC_CMD_CONFIG               POC_CMD_CONFIG
#define FR_POC_CMD_FREEZE               POC_CMD_FREEZE
#define FR_POC_CMD_READY                POC_CMD_READY
#define FR_POC_CMD_CONFIG_COMPLETE      POC_CMD_CONFIG_COMPLETE
#define FR_POC_CMD_RUN                  POC_CMD_RUN
#define FR_POC_CMD_DEFAULT_CONFIG       POC_CMD_DEFAULT_CONFIG
#define FR_POC_CMD_HALT                 POC_CMD_HALT
#define FR_POC_CMD_WAKEUP               POC_CMD_WAKEUP

typedef struct {
    const Fr_ConfigType *Fr_ConfigPtr;
    Fr_POCStateType Fr_CCPocState[FR_ARC_CTRL_CONFIG_CNT];
    boolean Fr_HasIntiailized;
#if defined(CFG_RH850)
    volatile struct FLXA_reg *hwPtr[FR_ARC_CTRL_CONFIG_CNT];
#endif
}Fr_ContainerType;

/* @req FR098 */
Std_ReturnType Fr_Internal_ClearPendingTx(const Fr_ContainerType *Fr_Cfg, uint8 Fr_CtrlIdx);
Std_ReturnType Fr_Internal_ClearPendingRx(const Fr_ContainerType *Fr_Cfg, uint8 Fr_CtrlIdx);
Std_ReturnType Fr_Internal_RunAllCCTest(const Fr_ContainerType *Fr_Cfg, uint8 Fr_CtrlIdx);
Std_ReturnType Fr_Internal_RunCCTest(const Fr_ContainerType *Fr_Cfg, uint8 Fr_CtrlIdx);
Std_ReturnType Fr_Internal_SetupCC(const Fr_ContainerType *Fr_Cfg, uint8 Fr_CtrlIdx);
Std_ReturnType Fr_Internal_SetupRxTxResources(const Fr_ContainerType *Fr_Cfg, uint8 Fr_CtrlIdx);
void Fr_Arc_ClearPendingIsr(const Fr_ContainerType *Fr_Cfg, uint8 Fr_CtrlIdx);
void Fr_Internal_DisableAllTimers(const Fr_ContainerType *Fr_Cfg, uint8 Fr_CtrlIdx);
void Fr_Internal_DisableAllFrIsr(const Fr_ContainerType *Fr_Cfg, uint8 Fr_CtrlIdx);
void Fr_Internal_DisableAllLPdu(void);
Std_ReturnType Fr_Internal_SetCtrlChiCmd(const Fr_ContainerType *Fr_Cfg, uint8 Fr_CtrlIdx, uint8 chiCmd);
Std_ReturnType Fr_Internal_EnableCtrl(const Fr_ContainerType *Fr_Cfg, uint8 Fr_CtrlIdx);
Std_ReturnType Fr_Internal_setupAndTestCC(Fr_ContainerType *Fr_Cfg, uint8 Fr_CtrlIdx);
/**
 * Write the data to be transmitted.
 * @param Fr_Cfg
 * @param Fr_CtrlIdx
 * @param Fr_POCStatusPtr
 * @return
 */
Std_ReturnType Fr_Internal_GetChiPocState(const Fr_ContainerType *Fr_Cfg, uint8 Fr_CtrlIdx, Fr_POCStatusType* Fr_POCStatusPtr);
Std_ReturnType Fr_Internal_SetTxData(const Fr_ContainerType *Fr_Cfg, uint8 Fr_CtrlIdx, const uint8* Fr_LSduPtr, uint8 Fr_LSduLength, uint16 Fr_MsgBuffrIdx);
Std_ReturnType Fr_Internal_CheckHeader(const Fr_ContainerType *Fr_Cfg, uint8 Fr_CtrlIdx, uint32 trigIdx);
Std_ReturnType Fr_Internal_CheckNewData(const Fr_ContainerType *Fr_Cfg, uint8 Fr_CtrlIdx, uint16 msgBufferIdx);

/**
 * @brief Get new RX data. Received data is copied to Fr_LSduPtr and Fr_LSduLengthPtr is updated
 *        with the length.
 * @param Fr_Cfg			Pointer to the flexray configuration.
 * @param Fr_CtrlIdx		Index of flexray controllers.
 * @param trigIdx			Index into the trigger configuration.
 * @param msgBufferIdx      Index into the message buffers.
 * @param Fr_LSduPtr		Pointer to data to data to receive.
 * @param Fr_LSduLengthPtr  Pointer to lengh of the data. Will be update by function.
 * @return
 */
Std_ReturnType Fr_Internal_GetNewData(const Fr_ContainerType *Fr_Cfg, uint8 Fr_CtrlIdx, uint32 trigIdx,
                                      uint16 msgBufferIdx, uint8* Fr_LSduPtr, uint8* Fr_LSduLengthPtr);

/**
 * Checks the message status
 * @param Fr_Cfg
 * @param Fr_CtrlIdx
 * @param msgBufferIdx
 * @return
 */
Std_ReturnType Fr_Internal_GetTxPending(const Fr_ContainerType *Fr_Cfg, uint8 Fr_CtrlIdx, uint32 trigIdx, boolean *txPending);
Std_ReturnType Fr_Internal_CancelTx(const Fr_ContainerType *Fr_Cfg, uint8 Fr_CtrlIdx, uint16 msgBufferIdx);
Std_ReturnType Fr_Internal_SetWUPChannel(const Fr_ContainerType *Fr_Cfg, uint8 Fr_CtrlIdx, Fr_ChannelType Fr_ChnlIdx);
Std_ReturnType  Fr_Internal_GetGlobalTime(const Fr_ContainerType *Fr_Cfg, uint8 Fr_CtrlIdx, uint8* Fr_CyclePtr, uint16* Fr_MacroTickPtr);
/**
 *
 * @param Fr_Cfg
 * @param Fr_CtrlIdx
 * @return
 */
Std_ReturnType Fr_Internal_IsSynchronous(const Fr_ContainerType *Fr_Cfg, uint8 Fr_CtrlIdx);
void Fr_Internal_SetupAbsTimer(const Fr_ContainerType *Fr_Cfg, uint8 Fr_CtrlIdx, uint8 Fr_AbsTimerIdx, uint8 Fr_Cycle, uint16 Fr_Offset);
void Fr_Internal_DisableAbsTimer(const Fr_ContainerType *Fr_Cfg, uint8 Fr_CtrlIdx, uint8 Fr_AbsTimerIdx);
/**
 * Gets IRQ status of a timer.
 *
 * @param Fr_Cfg
 * @param Fr_CtrlIdx
 * @param Fr_AbsTimerIdx
 * @return TRUE  - If interrupt is pending
 *         FALSE - Not pending
 */
boolean Fr_Internal_GetAbsTimerIrqStatus(const Fr_ContainerType *Fr_Cfg, uint8 Fr_CtrlIdx, uint8 Fr_AbsTimerIdx);
void Fr_Internal_ResetAbsTimerIsrFlag(const Fr_ContainerType *Fr_Cfg, uint8 Fr_CtrlIdx, uint8 Fr_AbsTimerIdx);
void Fr_Internal_DisableAbsTimerIrq(const Fr_ContainerType *Fr_Cfg, uint8 Fr_CtrlIdx, uint8 Fr_AbsTimerIdx);
void Fr_Internal_EnableAbsTimerIrq(const Fr_ContainerType *Fr_Cfg, uint8 Fr_CtrlIdx, uint8 Fr_AbsTimerIdx);
Std_ReturnType Fr_Internal_UpdateHeaderLength(const Fr_ContainerType *Fr_Cfg, uint8 Fr_CtrlIdx, uint8 length, uint16 msgBuffrIdx, uint32 frameTrigIdx);
#if defined(USE_DEM) || defined(CFG_FR_DEM_TEST)
static inline void Fr_Internal_reportDem(Dem_EventIdType eventId, Dem_EventStatusType eventStatus);
#endif


Fr_POCStateType Fr_Internal_GetProtState( const Fr_ContainerType *Fr_Cfg, uint8 cIdx );

#endif /*FR_INTERNAL_H_*/
