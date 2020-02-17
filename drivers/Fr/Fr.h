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

/* @req FR101 */
#ifndef FR_H_
#define FR_H_

/* @req FR102 */

/*@req FR667 */
#define FR_VENDOR_ID			    60
#define FR_MODULE_ID			    81

#define FR_AR_RELEASE_MAJOR_VERSION     4
#define FR_AR_RELEASE_MINOR_VERSION     0
#define FR_AR_RELEASE_REVISION_VERSION  3

/* @req FR650 */
#define FR_SW_MAJOR_VERSION    1
#define FR_SW_MINOR_VERSION    0
#define FR_SW_PATCH_VERSION    0

/* @req FR460 */
#include "Std_Types.h"
/* @req FR459 */
#include "Fr_GeneralTypes.h"
/* @req FR461 */
#include "Fr_Cfg.h"
/* @req FR112 */
#include "MemMap.h"

/* --- Error / Event IDs --- */
/* @req FR125 */
/* @req FR078 */
/* @req FR025 */
#define FR_E_INV_TIMER_IDX      (uint8)0x01
/* @req FR488 */
#define FR_E_INV_POINTER        (uint8)0x02
/* @req FR489 */
#define FR_E_INV_OFFSET         (uint8)0x03
/* @req FR490 */
#define FR_E_INV_CTRL_IDX       (uint8)0x04
/* @req FR491 */
#define FR_E_INV_CHNL_IDX       (uint8)0x05
/* @req FR492 */
#define FR_E_INV_CYCLE          (uint8)0x06
/* @req FR494 */
#define FR_E_NOT_INITIALIZED    (uint8)0x08
/* @req FR495 */
#define FR_E_INV_POCSTATE       (uint8)0x09
/* @req FR496 */
#define FR_E_INV_LENGTH         (uint8)0x0A
/* @req FR497 */
#define FR_E_INV_LPDU_IDX       (uint8)0x0B
/* @req FR633 */
#define FR_E_INV_HEADERCRC      (uint8)0x0C
#define FR_E_INV_CONFIG_IDX     (uint8)0x0D
#define FR_E_ARC_DATA_PARTITION (uint8)0x0E

/* @req FR078 */
#define FR_CTRLINIT_SERVICE_ID      (uint8)0x00
#define FR_STARTCOMMUNICATION_ID    (uint8)0x03
#define FR_HALTCOMMUNICATION_ID     (uint8)0x04
#define FR_ABORTCOMMUNICATION_ID    (uint8)0x05
#define FR_SENDWUP_ID               (uint8)0x06
#define FR_SETWAKEUPCHANNEL_ID      (uint8)0x07
#define FR_GETPOCSTATUS_ID          (uint8)0x0A
#define FR_TRANSMITTXLPDU_ID        (uint8)0x0B
#define FR_RECEIVERXLPDU_ID         (uint8)0x0C
#define FR_CHECKTXLPDUSTATUS_ID     (uint8)0x0D
#define FR_GETGLOBALTIME_ID         (uint8)0x10
#define FR_SETABSOLUTETIMER_ID      (uint8)0x11
#define FR_CANCELABSOLUTETIMER_ID   (uint8)0x13
#define FR_ENABLEABSTIMERIRQ_ID     (uint8)0x15
#define FR_ACKABSOLUTETIMERIRQ_ID   (uint8)0x17
#define FR_DISABLEABSTIMERIRQ_ID    (uint8)0x19
#define FR_GETVERSIONINFO_ID        (uint8)0x1B
#define FR_INIT_SERVICE_ID          (uint8)0x1C
#define FR_GETABSTIMERIRQSTATUS_ID  (uint8)0x20
#define FR_ALLOWCOLDSTART_ID        (uint8)0x23
#define FR_CANCELTXLPDU_ID          (uint8)0x2D

/* @req FR464 */

/* @req FR501 */
/* @req FR646 */
/* !req FR648 */
typedef struct {
    const Fr_CtrlConfigParametersType *FrCtrlParam;
    const Fr_FrIfCCTriggeringType *FrTrigConfig;
    const Fr_FrIfClusterConfigType *FrClusterConfig;
    const Fr_FrIfLPduContainerType *Fr_LpduConf;
}Fr_ConfigType;

extern const Fr_ConfigType FrConfigData;

/* @req FR098 */
void Fr_Init(const Fr_ConfigType* Fr_ConfigPtr);
Std_ReturnType Fr_ControllerInit(uint8 Fr_CtrlIdx);
Std_ReturnType Fr_AbortCommunication(uint8 Fr_CtrlIdx);
Std_ReturnType Fr_HaltCommunication(uint8 Fr_CtrlIdx);
Std_ReturnType Fr_AllowColdstart(uint8 Fr_CtrlIdx);
Std_ReturnType Fr_GetPOCStatus(uint8 Fr_CtrlIdx, Fr_POCStatusType* Fr_POCStatusPtr);
Std_ReturnType Fr_StartCommunication(uint8 Fr_CtrlIdx);
Std_ReturnType Fr_SendWUP(uint8 Fr_CtrlIdx);
Std_ReturnType Fr_SetWakeupChannel(uint8 Fr_CtrlIdx, Fr_ChannelType Fr_ChnlIdx);
Std_ReturnType Fr_TransmitTxLPdu(uint8 Fr_CtrlIdx, uint16 Fr_LPduIdx, const uint8* Fr_LSduPtr, uint8 Fr_LSduLength);
Std_ReturnType Fr_ReceiveRxLPdu(uint8 Fr_CtrlIdx, uint16 Fr_LPduIdx, uint8* Fr_LSduPtr, Fr_RxLPduStatusType* Fr_LPduStatusPtr, uint8* Fr_LSduLengthPtr );
Std_ReturnType Fr_CheckTxLPduStatus(uint8 Fr_CtrlIdx, uint16 Fr_LPduIdx, Fr_TxLPduStatusType* Fr_TxLPduStatusPtr);
Std_ReturnType Fr_CancelTxLPdu(uint8 Fr_CtrlIdx, uint16 Fr_LPduIdx);
Std_ReturnType Fr_GetGlobalTime(uint8 Fr_CtrlIdx, uint8* Fr_CyclePtr, uint16* Fr_MacroTickPtr);
Std_ReturnType Fr_SetAbsoluteTimer(uint8 Fr_CtrlIdx, uint8 Fr_AbsTimerIdx, uint8 Fr_Cycle, uint16 Fr_Offset);
Std_ReturnType Fr_CancelAbsoluteTimer(uint8 Fr_CtrlIdx, uint8 Fr_AbsTimerIdx);
Std_ReturnType Fr_GetAbsoluteTimerIRQStatus(uint8 Fr_CtrlIdx, uint8 Fr_AbsTimerIdx, boolean* Fr_IRQStatusPtr);
Std_ReturnType Fr_AckAbsoluteTimerIRQ(uint8 Fr_CtrlIdx, uint8 Fr_AbsTimerIdx);
Std_ReturnType Fr_DisableAbsoluteTimerIRQ(uint8 Fr_CtrlIdx, uint8 Fr_AbsTimerIdx);
Std_ReturnType Fr_EnableAbsoluteTimerIRQ(uint8 Fr_CtrlIdx, uint8 Fr_AbsTimerIdx);
Std_ReturnType Fr_AllSlots( uint8 Fr_CtrlIdx );
Std_ReturnType Fr_GetChannelStatus( uint8 Fr_CtrlIdx, uint16* Fr_ChannelAStatusPtr, uint16* Fr_ChannelBStatusPtr );
Std_ReturnType Fr_GetClockCorrection( uint8 Fr_CtrlIdx, sint16* Fr_RateCorrectionPtr, sint32* Fr_OffsetCorrectionPtr );
Std_ReturnType Fr_GetNumOfStartupFrames( uint8 Fr_CtrlIdx, uint8* Fr_NumOfStartupFramesPtr );
Std_ReturnType Fr_GetWakeupRxStatus( uint8 Fr_CtrlIdx, uint8* Fr_WakeupRxStatusPtr );
Std_ReturnType Fr_ReadCCConfig( uint8 Fr_CtrlIdx, uint8 Fr_ConfigParamIdx,uint32* Fr_ConfigParamValuePtr );
Std_ReturnType Fr_GetSyncFrameList( uint8 Fr_CtrlIdx, uint8 Fr_ListSize, uint16* Fr_ChannelAEvenListPtr, uint16* Fr_ChannelBEvenListPtr, uint16* Fr_ChannelAOddListPtr, uint16* Fr_ChannelBOddListPtr );
Std_ReturnType Fr_DisableLPdu( uint8 Fr_CtrlIdx, uint16 Fr_LPduIdx );
Std_ReturnType Fr_PrepareLPdu( uint8 Fr_CtrlIdx, uint16 Fr_LPduIdx );
Std_ReturnType Fr_ReconfigLPdu( uint8 Fr_CtrlIdx, uint16 Fr_LPduIdx, uint16 Fr_FrameId, Fr_ChannelType Fr_ChnlIdx,
        uint8 Fr_CycleRepetition, uint8 Fr_CycleOffset, uint8 Fr_PayloadLength, uint16 Fr_HeaderCRC );
#if ( FR_VERSION_INFO_API == STD_ON )
void Fr_GetVersionInfo(Std_VersionInfoType* VersioninfoPtr);
#endif

#endif /*FR_H_*/
