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

#ifndef FRIF_H_
#define FRIF_H_

/* @req FrIf05140a */ /* Module interface file */
/* @req FrIf05140b */ /* Type definitions and API declarations provided in this file */
/* @req FrIf05141 */ /* Precompile time config is provided by FrIf_Cfg.h */
/* @req FrIf05143 */ /* No global variables defined in header files */
/**
 * Includes
 */
#include "Std_Types.h"
#include "ComStack_Types.h" /* @req FrIf05076g */
#include "MemMap.h" /* @req FrIf05076q */ /* @req FrIf05088 */
#include "Fr_GeneralTypes.h" /* @req FrIf05076f */
#include "FrIf_Types.h" /* @req FrIf05076r */
#include "Fr.h"

/**
 * Defines and macros
 */

#define IDX_ERROR_UINT8 255
#define PDU_DATA_ENTRIES 256

/* @req FrIf05090 */
#define FRIF_VENDOR_ID      60u
#define FRIF_MODULE_ID      61u
#define FRIF_INSTANCE_ID 	0u
/* vendor specific */
#define FRIF_SW_MAJOR_VERSION 2u
#define FRIF_SW_MINOR_VERSION 0u
#define FRIF_SW_PATCH_VERSION 0u
/* compliance with following autosar version */
#define FRIF_AR_MAJOR_VERSION 4u
#define FRIF_AR_MINOR_VERSION 0u
#define FRIF_AR_PATCH_VERSION 3u

#include "FrIf_Cfg.h" /* @req FrIf05076b */
/* API Service ID */
/* Note: The commented defines are not used at the time of writing. Uncomment them to use them. */
#define FRIF_INIT_API_ID                                0x02u
#define FRIF_CONTROLLER_INIT_API_ID                     0x03u
#define FRIF_START_COMMUNICATION_API_ID                 0x04u
#define FRIF_HALT_COMMUNICATION_API_ID                  0x05u
#define FRIF_ABORT_COMMUNICATION_API_ID                 0x06u
#define FRIF_GET_STATE_API_ID                           0x07u
#define FRIF_SET_STATE_API_ID                           0x08u
#define FRIF_SET_WUP_CHANNEL_API_ID                     0x09u
#define FRIF_SEND_WUP_API_ID                            0x0Au
#define FRIF_GET_POC_STATUS_API_ID                      0x0Du
#define FRIF_GET_GLOBAL_TIME_API_ID                     0x0Eu
#define FRIF_ALLOW_COLD_START_API_ID                    0x10u
#define FRIF_GET_MACROTICKS_PER_CYCLE_API_ID            0x11u
#define FRIF_GET_MACROTICKS_DUR_API_ID                  0x31u
#define FRIF_TRANSMIT_API_ID                            0x12u
#define FRIF_SET_TRCV_MODE_API_ID                       0x13u
#define FRIF_GET_TRCV_MODE_API_ID                       0x14u
#define FRIF_GET_TRCV_WU_REASON_API_ID                  0x15u
#define FRIF_CLR_TRCV_WUP_API_ID                        0x18u
#define FRIF_SET_ABSOLUTE_TIMER_API_ID					0x19u
#define FRIF_CANCEL_ABSOLUTE_TIMER_API_ID               0x1Bu
#define FRIF_ENABLE_ABSOLUTE_TIMER_IRQ_API_ID			0x1Du
#define FRIF_ACK_ABSOLUTE_TIMER_IRQ_API_ID				0x21u
#define FRIF_GET_ABSOLUTE_TIMER_IRQ_STATUS_API_ID       0x1Fu
#define FRIF_DISABLE_ABSOLUTE_TIMER_IRQ_API_ID          0x23u
#define FRIF_GET_CYCLE_LENGTH_API_ID                    0x3Au
#define FRIF_ALL_SLOTS_API_ID                           0x33u
#define FRIF_GET_CHNL_STATUS_API_ID                     0x26u
#define FRIF_GET_CLK_CORRECTION_API_ID                  0x29u
#define FRIF_GET_SYNC_FRAME_LIST_API_ID                 0x2Au
#define FRIF_GET_NUM_STARTUP_FRAMES_API_ID              0x34u
#define FRIF_GET_WUP_RX_STATUS_API_ID                   0x2Bu
#define FRIF_CANCEL_TRANSMIT_API_ID                     0x30u
#define FRIF_DISABLE_LPDU_API_ID                        0x28u
#define FRIF_GET_TRV_ERROR_API_ID                       0x35u
#define FRIF_ENABLE_TRCV_BRANCH_API_ID                  0x36u
#define FRIF_DISABLE_TRCV_BRANCH_API_ID                 0x37u
#define FRIF_RECONFIG_LPDU_API_ID                       0x00u
#define FRIF_GET_NM_VECTOR_API_ID                       0x0Fu
#define FRIF_GET_VERSION_INFO_API_ID                    0x01u
#define FRIF_READ_CC_CONFIG_API_ID                      0x3Bu
#define FRIF_JOB_LIST_EXECUTE_API_ID                    0x32u
#define FRIF_CHECK_WUP_BY_TRCV_API_ID                   0x39u
#define FRIF_MAINFUNCTION_API_ID                        0x27u


/* FlexRay Error Codes*/
/* @req FrIf05142 */
/* @req FrIf05145 */
#define FRIF_E_INV_POINTER 			0x01u
#define FRIF_E_INV_CTRL_IDX 		0x02u
#define FRIF_E_INV_CLST_IDX 		0x03u
#define FRIF_E_INV_CHNL_IDX 		0x04u
#define FRIF_E_INV_TIMER_IDX 		0x05u
#define FRIF_E_INV_TXPDUID 			0x06u
#define FRIF_E_INV_LPDU_IDX 		0x07u
#define FRIF_E_NOT_INITIALIZED 		0x08u
#define FRIF_E_JLE_SYNC				0x09u
#define FRIF_E_INV_FRAME_ID		    0x0Bu

/* Channel status information bit mask */
#define CHNL_STATUS_VALID_FRAME                  1u
#define CHNL_STATUS_SYNTAX_ERROR                 2u
#define CHNL_STATUS_CONTENT_ERROR                4u
#define CHNL_STATUS_ADDITIONAL_COMMUNICATION     8u
#define CHNL_STATUS_B_VIOLATION                  16u
#define CHNL_STATUS_TX_CONFLICT                  32u
#define CHNL_ACS_ERROR_MASK                      (CHNL_STATUS_SYNTAX_ERROR|CHNL_STATUS_CONTENT_ERROR|CHNL_STATUS_B_VIOLATION|CHNL_STATUS_TX_CONFLICT)
#define SYMBOL_WINDOW_STATUS_VALID_MTS           256u
#define SYMBOL_WINDOW_STATUS_SYNTAX_ERROR        512u
#define SYMBOL_WINDOW_STATUS_B_VIOLATION         1024u
#define SYMBOL_WINDOW_STATUS_TX_CONFLICT         2048u
#define SW_ERROR_MASK                            (SYMBOL_WINDOW_STATUS_SYNTAX_ERROR | SYMBOL_WINDOW_STATUS_B_VIOLATION |SYMBOL_WINDOW_STATUS_TX_CONFLICT)
#define NIT_STATUS_SYNTAX_ERROR                  4096u
#define NIT_STATUS_B_VIOLATION                   8192u
#define NIT_ERROR_MASK                           (NIT_STATUS_SYNTAX_ERROR | NIT_STATUS_B_VIOLATION)

/* Invalid Dem Event parameter Reference */
#define FRIF_INVALID_DEM_EVENT_ID   0xFFFFu
/**
 * Function declarations
 */

extern const FrIf_ConfigType FrIf_Config;

/* @req FrIf05003 */
void FrIf_Init(const FrIf_ConfigType* FrIf_ConfigPtr);
/* @req FrIf05004 */
Std_ReturnType FrIf_ControllerInit(uint8 FrIf_CtrlIdx);
/* @req FrIf05021 */
Std_ReturnType FrIf_SetAbsoluteTimer(uint8 FrIf_CtrlIdx, uint8 FrIf_AbsTimerIdx, uint8 FrIf_Cycle, uint16 FrIf_Offset);
/* @req FrIf05025 */
Std_ReturnType FrIf_EnableAbsoluteTimerIRQ(uint8 FrIf_CtrlIdx, uint8 FrIf_AbsTimerIdx);
/* @req FrIf05029 */
Std_ReturnType FrIf_AckAbsoluteTimerIRQ(uint8 FrIf_CtrlIdx, uint8 FrIf_AbsTimerIdx);
/* @req FrIf05005 */
Std_ReturnType FrIf_StartCommunication(uint8 FrIf_CtrlIdx);
/* @req FrIf05006 */
Std_ReturnType FrIf_HaltCommunication( uint8 FrIf_CtrlIdx );
/* @req FrIf05007 */
Std_ReturnType FrIf_AbortCommunication( uint8 FrIf_CtrlIdx );
/* @req FrIf05170 */
Std_ReturnType FrIf_GetState( uint8 FrIf_ClstIdx, FrIf_StateType* FrIf_StatePtr );
/* @req FrIf05174 */
Std_ReturnType FrIf_SetState(uint8 FrIf_ClstIdx, FrIf_StateTransitionType FrIf_StateTransition);
/* @req FrIf05010 */
Std_ReturnType FrIf_SetWakeupChannel( uint8 FrIf_CtrlIdx, Fr_ChannelType FrIf_ChnlIdx );
/* @req FrIf05011 */
Std_ReturnType FrIf_SendWUP( uint8 FrIf_CtrlIdx );
/* @req FrIf05014 */
Std_ReturnType FrIf_GetPOCStatus(uint8 FrIf_CtrlIdx, Fr_POCStatusType *FrIf_POCStatusPtr);
/* @req FrIf05015 */
Std_ReturnType FrIf_GetGlobalTime(uint8 FrIf_CtrlIdx, uint8 *FrIf_CyclePtr, uint16 *FrIf_MacroTickPtr);
/* @req FrIf05017 */
Std_ReturnType FrIf_AllowColdstart(uint8 Fr_CtrlIdx);
/* @req FrIf05018 */
uint16 FrIf_GetMacroticksPerCycle(uint8 FrIf_CtrlIdx);
/* @req FrIf05019 */
uint16 FrIf_GetMacrotickDuration( uint8 FrIf_CtrlIdx );
/* @req FrIf05033 */
Std_ReturnType FrIf_Transmit(PduIdType FrIf_TxPduId, const PduInfoType *FrIf_PduInfoPtr);
/* !req FrIf05034 */
Std_ReturnType FrIf_SetTransceiverMode( uint8 FrIf_CtrlIdx, Fr_ChannelType FrIf_ChnlIdx, FrTrcv_TrcvModeType FrIf_TrcvMode );
/* !req FrIf05035 */
Std_ReturnType FrIf_GetTransceiverMode( uint8 FrIf_CtrlIdx, Fr_ChannelType FrIf_ChnlIdx, FrTrcv_TrcvModeType* FrIf_TrcvModePtr );
/* !req FrIf05036 */
Std_ReturnType FrIf_GetTransceiverWUReason( uint8 FrIf_CtrlIdx, Fr_ChannelType FrIf_ChnlIdx, FrTrcv_TrcvWUReasonType* FrIf_TrcvWUReasonPtr );
/* !req FrIf05039 */
Std_ReturnType FrIf_ClearTransceiverWakeup( uint8 FrIf_CtrlIdx, Fr_ChannelType FrIf_ChnlIdx );
/* @req FrIf05023 */
Std_ReturnType FrIf_CancelAbsoluteTimer( uint8 FrIf_CtrlIdx, uint8 FrIf_AbsTimerIdx );
/* @req FrIf05027 */
Std_ReturnType FrIf_GetAbsoluteTimerIRQStatus( uint8 FrIf_CtrlIdx, uint8 FrIf_AbsTimerIdx, boolean* FrIf_IRQStatusPtr );
/* @req FrIf05031 */
Std_ReturnType FrIf_DisableAbsoluteTimerIRQ(uint8 FrIf_CtrlIdx, uint8 FrIf_AbsTimerIdx);
/* @req FrIf05239 */
uint32 FrIf_GetCycleLength( uint8 FrIf_CtrlIdx );

/** Optional APIs */
/* @req FrIf05412 */
#if (FRIF_ALL_SLOTS_SUPPORT == STD_ON)
/* @req FrIf05020 */
Std_ReturnType FrIf_AllSlots( uint8 FrIf_CtrlIdx );
#endif
/* @req FrIf05413 */
#if (FRIF_GET_CHNL_STATUS_SUPPORT == STD_ON)
/* @req FrIf05030 */
Std_ReturnType FrIf_GetChannelStatus( uint8 FrIf_CtrlIdx, uint16* FrIf_ChannelAStatusPtr, uint16* FrIf_ChannelBStatusPtr );
#endif
/* @req FrIf05414 */
#if (FRIF_GET_CLK_CORRECTION_SUPPORT == STD_ON)
/* @req FrIf05071 */
Std_ReturnType FrIf_GetClockCorrection( uint8 FrIf_CtrlIdx, sint16* FrIf_RateCorrectionPtr, sint32* FrIf_OffsetCorrectionPtr );
#endif
/* @req FrIf05415 */
#if (FRIF_GET_SYNC_FRAME_LIST_SUPPORT == STD_ON)
/* @req FrIf05072 */
Std_ReturnType FrIf_GetSyncFrameList( uint8 FrIf_CtrlIdx, uint8 FrIf_ListSize, uint16* FrIf_ChannelAEvenListPtr, uint16* FrIf_ChannelBEvenListPtr, uint16* FrIf_ChannelAOddListPtr, uint16* FrIf_ChannelBOddListPtr );
#endif
/* @req FrIf05416 */
#if (FRIF_GET_NUM_STARTUP_FRAMES_SUPPORT == STD_ON)
/* @req FrIf05073 */
Std_ReturnType FrIf_GetNumOfStartupFrames( uint8 FrIf_CtrlIdx, uint8* FrIf_NumOfStartupFramesPtr );
#endif
/* @req FrIf05417 */
#if (FRIF_GET_WUP_RX_STATUS_SUPPORT == STD_ON)
/* @req FrIf05102 */
Std_ReturnType FrIf_GetWakeupRxStatus( uint8 FrIf_CtrlIdx, uint8* FrIf_WakeupRxStatusPtr );
#endif
/* @req FrIf05713 */
#if (FRIF_CANCEL_TRANSMIT_SUPPORT == STD_ON)
/* !req FrIf05070 */
Std_ReturnType FrIf_CancelTransmit( PduIdType FrIf_TxPduId );
#endif
/* @req FrIf05418 */
#if (FRIF_DISABLE_LPDU_SUPPORT == STD_ON)
/* @req FrIf05710 */
Std_ReturnType FrIf_DisableLPdu( uint8 FrIf_CtrlIdx, uint16 FrIf_LPduIdx );
#endif
/* @req FrIf05419 */
#if (FRIF_GET_TRCV_ERROR_SUPPORT == STD_ON)
/* !req FrIf05032 */
Std_ReturnType FrIf_GetTransceiverError( uint8 FrIf_CtrlIdx, Fr_ChannelType FrIf_ChnlIdx, uint8 FrIf_BranchIdx, uint32* FrIf_BusErrorState );
#endif
/* @req FrIf05420 */
#if (FRIF_ENABLE_TRCV_BRANCH_SUPPORT == STD_ON)
/* !req FrIf05085 */
Std_ReturnType FrIf_EnableTransceiverBranch( uint8 FrIf_CtrlIdx, Fr_ChannelType FrIf_ChnlIdx, uint8 FrIf_BranchIdx );
#endif
/* @req FrIf05421 */
/* @req FrIf05425 */
#if (FRIF_DIABLE_TRCV_BRANCH_SUPPORT == STD_ON)
/* !req FrIf05028 */
Std_ReturnType FrIf_DisableTransceiverBranch( uint8 FrIf_CtrlIdx, Fr_ChannelType FrIf_ChnlIdx, uint8 FrIf_BranchIdx );
#endif
/* @req FrIf05422 */
#if (FRIF_RECONFIG_LPDU_SUPPORT == STD_ON)
/* @req FrIf05048 */
Std_ReturnType FrIf_ReconfigLPdu( uint8 FrIf_CtrlIdx, uint16 FrIf_LPduIdx, uint16 FrIf_FrameId, Fr_ChannelType FrIf_ChnlIdx,
        uint8 FrIf_CycleRepetition, uint8 FrIf_CycleOffset, uint8 FrIf_PayloadLength, uint16 FrIf_HeaderCRC);
#endif
/* @req FrIf05423 */
#if (FRIF_GET_NM_VECTOR_SUPPORT == STD_ON)
/* !req FrIf05016 */
Std_ReturnType FrIf_GetNmVector( uint8 FrIf_CtrlIdx, uint8* FrIf_NmVectorPtr );
#endif
/* @req FrIf05153 */
#if (FRIF_VERSION_INFO_API == STD_ON)
/* @req FrIf05002 */
void FrIf_GetVersionInfo( Std_VersionInfoType* FrIf_VersionInfoPtr );
#endif
/* @req FrIf05313 */
Std_ReturnType FrIf_ReadCCConfig( uint8 FrIf_CtrlIdx, uint8 FrIf_ConfigParamIdx, uint32* FrIf_ConfigParamValuePtr );

/* !req FrIf05041 */
void FrIf_CheckWakeupByTransceiver( uint8 FrIf_CtrlIdx, Fr_ChannelType FrIf_ChnlIdx );

void FrIf_JobListExec(uint8 clstrIdx);
void FrIf_MainFunction(uint8 clstrIdx);





#endif /* FRIF_H_ */
