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


/** @reqSettings DEFAULT_SPECIFICATION_REVISION=4.1.2 */
/** @tagSettings DEFAULT_ARCHITECTURE=MPC5645S */

#ifndef CDD_LINSLV_H_
#define CDD_LINSLV_H_

#define CDD_LINSLV_MODULE_ID            255u
#define CDD_LINSLV_VENDOR_ID            60u

#define CDD_LINSLV_SW_MAJOR_VERSION            0u
#define CDD_LINSLV_SW_MINOR_VERSION            1u
#define CDD_LINSLV_SW_PATCH_VERSION            0u
#define CDD_LINSLV_AR_RELEASE_MAJOR_VERSION    4u
#define CDD_LINSLV_AR_RELEASE_MINOR_VERSION    1u
#define CDD_LINSLV_AR_RELEASE_PATCH_VERSION    2u

#include "Std_Types.h"
#include "Lin_GeneralTypes.h"
#include "Lin.h"
#include "LinIf_Types.h"
#include "ComStack_Types.h"

typedef enum {
    LINSLV_BUS_TIMEOUT = 0,
    LINSLV_GO_TO_SLEEP_REQ,
}LinSlv_SleepSrcType;

#include "CDD_LinSlv_Cfg.h"

typedef struct {
    Lin_ChannelConfigType LinSlvChnConfig;
    LinIf_StatusType LinSlvChannelInitState;
    uint8 LinSlvRespErrByteIdx;
    uint8 LinSlvRespErrBitIdxInByte;
    const LinIf_FrameType *LinSlvRespErrorFrameRef;
    void  (*LinSlvSleepCallback)(LinSlv_SleepSrcType src);
    void  (*LinSlvWakeupConfirmation)(boolean success);
    uint32 LinSlvBusTimeOutCnt;
}LinSlv_ChannelConfigType;

typedef struct {
    boolean LinSlvAttribIsConfigured;
    uint8   LinSlvConfiguredNad;
    uint16  LinSlvSupplierId;
    uint16  LinSlvFunctionId;
    uint8   LinSlvVariant;
}LinSlv_NodeAttributesType;

typedef struct {
    uint32 LinSlvTrcvEnableSource;
    uint32 LinSlvTrcvRstnSource;
    uint32 LinSlvTrcvTxDioSource;
    uint16 LinSlvTrcvTxPortSource;
    uint32 LinSlvTrcvTxLowWaitCount;
}LinSlv_TrcvDioAccessType;

typedef struct {
    boolean LinSlvTrcvIsConfigured;
    LinTrcv_TrcvModeType LinSlvTrcvInitState;
    LinSlv_TrcvDioAccessType LinSlvTrcvDioAccess;
}LinSlv_TrcvType;

typedef struct {
    uint8 LinSlvIdentifier;
    void  (*LinSlvResponseCallback)(const uint8 **responseFrame, uint8 *len);
}LinSlv_ReadByIdentifierFrameType;

typedef struct {
    uint32 LinSlvChnlFrameCnt;
    uint8 LinSlvUnconditionalFrameCnt;
    const LinIf_FrameType **LinSlvChnFrameCfg;
    uint32 LinSlvChnReadByIdCnt;
    const LinSlv_ReadByIdentifierFrameType *LinSlvReadByIdFrameCfg;
}LinSlv_FrameSetupType;

typedef struct {
    const LinSlv_ChannelConfigType *LinSlvChannelConfig;
    const LinSlv_TrcvType *LinSlvChnTrcvConfig;
    const uint8 *LinSlv_HwId2ChannelMap;
    const LinSlv_NodeAttributesType *LinSlvAttrib;
    const LinSlv_FrameSetupType *LinSlvChannelFrameConfig;
    const LinIf_FrameType *LinSlvPDUFrameCollection;
} LinSlv_ConfigType;


typedef struct {
    const LinSlv_ConfigType *LinSlvCfgPtr;
    Lin_StatusType LinSlvChannelStatus[LINSLV_CHANNEL_CNT];
    Lin_StatusType LinSlvChannelOrderedStatus[LINSLV_CHANNEL_CNT];
}LinSlv_ContainerType;

typedef enum {
    LINSLV_HEADER,
    LINSLV_TRANSMIT_DONE,
    LINSLV_OTHER,
}LinSlv_TxIsrSourceType;

/* API ID */
#define LINSLV_INIT_SERVICE_ID              0x01u
#define LINSLV_GETVERSIONINFO_SERVICE_ID    0x02u
#define LINSLV_CHECKWAKEUP_SERVICE_ID       0x03u
#define LINSLV_GOTOSLEEPINTERNAL_SERVICE_ID 0x04u
#define LINSLV_WAKEUP_SERVICE_ID            0x05u
#define LINSLV_WAKEUPINTERNAL_SERVICE_ID    0x06u
#define LINSLV_GETSTATUS_SERVICE_ID         0x07u
#define LINSLV_SETTRCVMODE_SERVICE_ID       0x08u
#define LINSLV_GETTRCVMODE_SERVICE_ID       0x09u
#define LINSLV_MAINFUNCTION_SERVICE_ID      0x0Du
#define LINSLV_DEINIT_SERVICE_ID            0x0Eu
#define LINSLV_INTERNAL_ISR_ID              0x0Fu

/* Error codes */
#define LINSLV_E_UNINIT                    0x00u
#define LINSLV_E_INVALID_CHANNEL           0x02u
#define LINSLV_E_INVALID_POINTER           0x03u
#define LINSLV_E_PARAM_POINTER             0x05u
#define LINSLV_E_STATE_TRANSITION          0x04u
#define LINSLV_E_INVALID_CONFIG            0x10u

/* Public */
void LinSlv_Init(const LinSlv_ConfigType* Config);
void LinSlv_DeInit(void);
Std_ReturnType LinSlv_GoToSleepInternal(uint8 Channel);
Std_ReturnType LinSlv_WakeupInternal(uint8 Channel);
Lin_StatusType LinSlv_GetStatus(uint8 Channel);
Std_ReturnType LinSlv_Wakeup(uint8 Channel);
Std_ReturnType LinSlv_CheckWakeup(EcuM_WakeupSourceType WakeupSource);
void LinSlv_GetVersionInfo(Std_VersionInfoType* versioninfo);
void LinSlv_MainFunction(void);
Std_ReturnType LinSlv_SetTrcvMode(uint8 Channel, LinTrcv_TrcvModeType TransceiverMode);
Std_ReturnType LinSlv_GetTrcvMode(uint8 Channel, LinTrcv_TrcvModeType* TransceiverModePtr);

/* Internal */
void LinSlv_Internal_Rx_Isr(uint8 hwChannelId);
void LinSlv_Internal_Tx_Isr(uint8 hwChannelId);
void LinSlv_Internal_Err_Isr(uint8 hwChannelId);

#endif /* CDD_LINSLV_H_ */





