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
#include "Eth.h"
#include "EthIf.h"
#include "SchM_EthIf.h"
#if (ETHIF_TRCV_SUPPORT == STD_ON)
 #include "EthTrcv.h"
#endif
#if (ETHIF_SWITCH_SUPPORT == STD_ON)
#include "EthSwt.h"
#endif
#if (ETHIF_DEV_ERROR_DETECT == STD_ON)
#if defined(USE_DET)
#include "Det.h"
#else
#error "EthIf: DET must be used when Default error detect is enabled"
#endif
#endif
#include "EthIf_Cbk.h"
#include "EthIf_Cfg.h"

//lint -emacro(904,ETHIF_DET_REPORTERROR) //904 PC-Lint exception to MISRA 14.7 (validate DET macros).

/* Globally fulfilled requirements */
/* @req 4.2.2/SWS_EthIf_00010 */
/* @req 4.2.2/SWS_EthIf_00112 */
/* @req 4.2.2/SWS_EthIf_00003 */

#define ETH_FRAME_TYPE_VLAN      0x8100u
#define ETH_FRAME_VLAN_HI        0x81u
#define ETH_FRAME_VLAN_LOW       0x00u
#define PCP_SHIFT_BITS_5         5u
#define PCP_SHIFT_BITS_4         4u
#define VLAN_MASK_NIBBLE         0x0F00u
#define VLAN_MASK_BYTE           0x00FFu
#define SHIFT_EIGHT_BITS         8u
#define FRAME_MASK_HI            0xFF00u
#define FRAME_MASK_LOW           0x00FFu
#define VLAN_TAG_SIZE            4u
#define PRIORITY_SIZE            8u
#define ETHIF_VID_MASK           0x0FFFu
#define ETHIF_SHIFT_BYTE1        8u
#define ETHIF_BYTE_MASK          0xFFu

/*lint -esym(9003,EthIf_ConfigPointer )*/
const EthIf_ConfigType* EthIf_ConfigPointer;

/** Static declarations */

typedef struct {
    uint32 frameType;
    uint32  bufferIdx;
}EthIfRunTimeType;

typedef struct {
    EthIf_StateType initStatus;  /* var to hold EthIf module status */
    EthIfRunTimeType ethIfRunTime[ETHIF_MAX_FRAME_OWNER_CFG]; /* Mapping from buffer index to Frame type*/
}EthIf_InternalType;

/*lint -esym(9003,EthIf_Internal )*//* @req 4.2.2/SWS_EthIf_00146*/
EthIf_InternalType EthIf_Internal = {
        .initStatus = ETHIF_STATE_UNINIT,
};


/* @req 4.2.2/SWS_EthIf_00008 */
#if (ETHIF_DEV_ERROR_DETECT == STD_ON)
#define ETHIF_DET_REPORTERROR(_cond ,_api, _error, ...) \
    if (!_cond) { \
        (void)Det_ReportError(ETHIF_MODULE_ID, 0, _api, _error); \
        return __VA_ARGS__; \
    }
#else
#define ETHIF_DET_REPORTERROR(_cond ,_api, _error, ...) \
    if (!_cond) { \
        return __VA_ARGS__; \
    }
#endif

#define INVALID_BUFFER_INDEX    0xFFFFFFFFUL

/** AUTOSAR APIs */
/**
 * Init function for EthIf
 * @param CfgPtr
 */
/* @req 4.2.2/SWS_EthIf_00024 */
void EthIf_Init( const EthIf_ConfigType* CfgPtr )
{
    uint8 i;

    /* @req 4.2.2/SWS_EthIf_00116 */
    ETHIF_DET_REPORTERROR((NULL != CfgPtr),ETHIF_SERVICE_ID_INIT, ETHIF_E_INIT_FAILED);

    /* @req 4.2.2/SWS_EthIf_00025 */ /* @req 4.2.2/SWS_EthIf_00014 */
    EthIf_ConfigPointer = CfgPtr;

    for (i=0; i < EthIf_ConfigPointer->EthIfOwnersCount; i++) {
        EthIf_Internal.ethIfRunTime[i].frameType = EthIf_ConfigPointer->EthIfOwnerCfg[i].EthIfFrameType;
        EthIf_Internal.ethIfRunTime[i].bufferIdx = INVALID_BUFFER_INDEX;
    }

    /* @req 4.2.2/SWS_EthIf_00114 */
    EthIf_Internal.initStatus = ETHIF_STATE_INIT;
}

#if((ETHIF_VERSION_INFO_API == STD_ON) && (ETHIF_VERSION_INFO_API_MACRO == STD_OFF))
/**
 * This service puts out the version information of this module
 * @param VersionInfoPtr
 */
/* @req 4.2.2/SWS_EthIf_00082 */
void EthIf_GetVersionInfo( Std_VersionInfoType* VersionInfoPtr )
{
    /* @req 4.2.2/SWS_EthIf_00127 */
    ETHIF_DET_REPORTERROR((NULL != VersionInfoPtr),ETHIF_SERVICE_ID_GET_VERSION_INFO,ETHIF_E_PARAM_POINTER);

    VersionInfoPtr->moduleID = ETHIF_MODULE_ID;  /* Module ID of ETHIF */
    VersionInfoPtr->vendorID = ETHIF_VENDOR_ID;  /* Vendor Id (ARCCORE) */

    /* return the Software Version numbers */
    VersionInfoPtr->sw_major_version = ETHIF_SW_MAJOR_VERSION;
    VersionInfoPtr->sw_minor_version = ETHIF_SW_MINOR_VERSION;
    VersionInfoPtr->sw_patch_version = ETHIF_SW_PATCH_VERSION;
}
#endif

/**
 *
 * @param CtrlIdx
 * @param CtrlMode
 * @return
 */
/* @req 4.2.2/SWS_EthIf_00034 */
Std_ReturnType EthIf_SetControllerMode( uint8 CtrlIdx, Eth_ModeType CtrlMode )
{
    Std_ReturnType ret;

    /* @req 4.2.2/SWS_EthIf_00036 */
    ETHIF_DET_REPORTERROR((ETHIF_STATE_INIT == EthIf_Internal.initStatus),ETHIF_SERVICE_ID_SET_CTRL_MODE, ETHIF_E_NOT_INITIALIZED, E_NOT_OK);

    /* @req 4.2.2/SWS_EthIf_00037 */
    ETHIF_DET_REPORTERROR((CtrlIdx < EthIf_ConfigPointer->EthIfCtrlCount),ETHIF_SERVICE_ID_SET_CTRL_MODE, ETHIF_E_INV_CTRL_IDX, E_NOT_OK);

    /* @req 4.2.2/SWS_EthIf_00035 */
    ret = Eth_SetControllerMode(EthIf_ConfigPointer->EthIfCtrlCfg[CtrlIdx].EthIfEthCtrlId, CtrlMode);

    return ret;
}

/**
 *
 * @param CtrlIdx
 * @param CtrlModePtr
 * @return
 */
/* @req 4.2.2/SWS_EthIf_00039 */
Std_ReturnType EthIf_GetControllerMode( uint8 CtrlIdx, Eth_ModeType* CtrlModePtr )
{
    Std_ReturnType ret;

    /* @req 4.2.2/SWS_EthIf_00041 */
    ETHIF_DET_REPORTERROR((ETHIF_STATE_INIT == EthIf_Internal.initStatus),ETHIF_SERVICE_ID_GET_CTRL_MODE, ETHIF_E_NOT_INITIALIZED, E_NOT_OK);

    /* @req 4.2.2/SWS_EthIf_00042 */
    ETHIF_DET_REPORTERROR((CtrlIdx < EthIf_ConfigPointer->EthIfCtrlCount),ETHIF_SERVICE_ID_GET_CTRL_MODE, ETHIF_E_INV_CTRL_IDX, E_NOT_OK);

    /* @req 4.2.2/SWS_EthIf_00043 */
    ETHIF_DET_REPORTERROR((NULL != CtrlModePtr),ETHIF_SERVICE_ID_GET_CTRL_MODE, ETHIF_E_PARAM_POINTER, E_NOT_OK);

    /* @req 4.2.2/SWS_EthIf_00040 */
    ret = Eth_GetControllerMode(EthIf_ConfigPointer->EthIfCtrlCfg[CtrlIdx].EthIfEthCtrlId, CtrlModePtr);

    return ret;
}
/**
 *
 * @param CtrlIdx
 * @param PhysAddrPtr
 */
/* @req 4.2.2/SWS_EthIf_00132 */
void EthIf_SetPhysAddr( uint8 CtrlIdx, const uint8* PhysAddrPtr )
{
    /* @req 4.2.2/SWS_EthIf_00135 */
    ETHIF_DET_REPORTERROR((ETHIF_STATE_INIT == EthIf_Internal.initStatus),ETHIF_SERVICE_ID_SET_PHY_ADDR, ETHIF_E_NOT_INITIALIZED);

    /* @req 4.2.2/SWS_EthIf_00136 */
    ETHIF_DET_REPORTERROR((CtrlIdx < EthIf_ConfigPointer->EthIfCtrlCount),ETHIF_SERVICE_ID_SET_PHY_ADDR, ETHIF_E_INV_CTRL_IDX);

    /* @req 4.2.2/SWS_EthIf_00137 */
    ETHIF_DET_REPORTERROR((NULL != PhysAddrPtr),ETHIF_SERVICE_ID_SET_PHY_ADDR, ETHIF_E_PARAM_POINTER);

    /* @req 4.2.2/SWS_EthIf_00134 */
    Eth_SetPhysAddr(EthIf_ConfigPointer->EthIfCtrlCfg[CtrlIdx].EthIfEthCtrlId, PhysAddrPtr);
}
/**
 *
 * @param CtrlIdx
 * @param PhysAddrPtr
 */
/* @req 4.2.2/SWS_EthIf_00061 */
void EthIf_GetPhysAddr( uint8 CtrlIdx, uint8* PhysAddrPtr )
{
    /* @req 4.2.2/SWS_EthIf_00063 */
    ETHIF_DET_REPORTERROR((ETHIF_STATE_INIT == EthIf_Internal.initStatus),ETHIF_SERVICE_ID_GET_PHY_ADDR, ETHIF_E_NOT_INITIALIZED);

    /* @req 4.2.2/SWS_EthIf_00064 */
    ETHIF_DET_REPORTERROR((CtrlIdx < EthIf_ConfigPointer->EthIfCtrlCount),ETHIF_SERVICE_ID_GET_PHY_ADDR, ETHIF_E_INV_CTRL_IDX);

    /* @req 4.2.2/SWS_EthIf_00065 */
    ETHIF_DET_REPORTERROR((NULL != PhysAddrPtr),ETHIF_SERVICE_ID_GET_PHY_ADDR, ETHIF_E_PARAM_POINTER);

    /* @req 4.2.2/SWS_EthIf_00062 */
    Eth_GetPhysAddr(EthIf_ConfigPointer->EthIfCtrlCfg[CtrlIdx].EthIfEthCtrlId, PhysAddrPtr);

}

/**
 *
 * @param CtrlIdx
 * @param PhysAddrPtr
 * @param Action
 * @return
 */
/* @req 4.2.2/SWS_EthIf_00139 */
Std_ReturnType EthIf_UpdatePhysAddrFilter( uint8 CtrlIdx, const uint8* PhysAddrPtr, Eth_FilterActionType Action )
{
    Std_ReturnType ret;

    /* @req 4.2.2/SWS_EthIf_00141 */
    ETHIF_DET_REPORTERROR((ETHIF_STATE_INIT == EthIf_Internal.initStatus),ETHIF_SERVICE_ID_UPDATE_PHY_ADDR_FILTER, ETHIF_E_NOT_INITIALIZED, E_NOT_OK);

    /* @req 4.2.2/SWS_EthIf_00142 */
    ETHIF_DET_REPORTERROR((CtrlIdx < EthIf_ConfigPointer->EthIfCtrlCount),ETHIF_SERVICE_ID_UPDATE_PHY_ADDR_FILTER, ETHIF_E_INV_CTRL_IDX, E_NOT_OK);

    /* @req 4.2.2/SWS_EthIf_00143 */
    ETHIF_DET_REPORTERROR((NULL != PhysAddrPtr),ETHIF_SERVICE_ID_UPDATE_PHY_ADDR_FILTER, ETHIF_E_PARAM_POINTER, E_NOT_OK );

    /* @req 4.2.2/SWS_EthIf_00140 */
#if (ETH_PHYS_ADRS_FILTER_API == STD_ON)
    ret = Eth_UpdatePhysAddrFilter( EthIf_ConfigPointer->EthIfCtrlCfg[CtrlIdx].EthIfEthCtrlId, PhysAddrPtr, Action);
#else
    (void)Action; /* To avoid PC lint error */
    ret = E_NOT_OK;
#endif
    return ret;
}

#if (ETHIF_GLOBAL_TIME_SUPPORT == STD_ON)
/* @req 4.2.2/SWS_EthIf_00158 */ /* @req 4.2.2/SWS_EthIf_00164 */ /* @req 4.2.2/SWS_EthIf_00170 */ /* @req 4.2.2/SWS_EthIf_00176 */
/* @req 4.2.2/SWS_EthIf_00182 */ /* @req 4.2.2/SWS_EthIf_00188 */
/**
 *
 * @param CtrlIdx
 * @param timeQualPtr
 * @param timeStampPtr
 * @return
 */
/* @req 4.2.2/SWS_EthIf_00154 */
Std_ReturnType EthIf_GetCurrentTime( uint8 CtrlIdx, Eth_TimeStampQualType* timeQualPtr, Eth_TimeStampType* timeStampPtr )
{
    Std_ReturnType ret;

    /* @req 4.2.2/SWS_EthIf_00155 */
    ETHIF_DET_REPORTERROR((ETHIF_STATE_INIT == EthIf_Internal.initStatus),ETHIF_SERVICE_ID_GET_CURRENT_TIME, ETHIF_E_NOT_INITIALIZED, E_NOT_OK);

    /* @req 4.2.2/SWS_EthIf_00156 */
    ETHIF_DET_REPORTERROR((CtrlIdx < EthIf_ConfigPointer->EthIfCtrlCount),ETHIF_SERVICE_ID_GET_CURRENT_TIME, ETHIF_E_INV_CTRL_IDX, E_NOT_OK);

    /* @req 4.2.2/SWS_EthIf_00157 */
    ETHIF_DET_REPORTERROR(((NULL != timeQualPtr) && (NULL != timeStampPtr)) ,ETHIF_SERVICE_ID_GET_CURRENT_TIME, ETHIF_E_PARAM_POINTER, E_NOT_OK );

    ret = Eth_GetCurrentTime(EthIf_ConfigPointer->EthIfCtrlCfg[CtrlIdx].EthIfEthCtrlId, timeQualPtr, timeStampPtr );

    return ret;
}

/**
 *
 * @param CtrlIdx
 * @param BufIdx
 */
/* @req 4.2.2/SWS_EthIf_00160 */
void EthIf_EnableEgressTimeStamp( uint8 CtrlIdx, Eth_BufIdxType BufIdx )
{
    /* @req 4.2.2/SWS_EthIf_00161 */
    ETHIF_DET_REPORTERROR((ETHIF_STATE_INIT == EthIf_Internal.initStatus),ETHIF_SERVICE_ID_ENABLE_EGRESS_TIME_STAMP, ETHIF_E_NOT_INITIALIZED);

    /* @req 4.2.2/SWS_EthIf_00162 */
    ETHIF_DET_REPORTERROR((CtrlIdx < EthIf_ConfigPointer->EthIfCtrlCount),ETHIF_SERVICE_ID_ENABLE_EGRESS_TIME_STAMP, ETHIF_E_INV_CTRL_IDX);

    Eth_EnableEgressTimeStamp(EthIf_ConfigPointer->EthIfCtrlCfg[CtrlIdx].EthIfEthCtrlId, BufIdx );
}
/**
 *
 * @param CtrlIdx
 * @param BufIdx
 * @param timeQualPtr
 * @param timeStampPtr
 */
/* @req 4.2.2/SWS_EthIf_00166 */
void EthIf_GetEgressTimeStamp(uint8 CtrlIdx, Eth_BufIdxType BufIdx, Eth_TimeStampQualType* timeQualPtr, Eth_TimeStampType* timeStampPtr )
{
    /* @req 4.2.2/SWS_EthIf_00167 */
    ETHIF_DET_REPORTERROR((ETHIF_STATE_INIT == EthIf_Internal.initStatus),ETHIF_SERVICE_ID_GET_EGRESS_TIME_STAMP, ETHIF_E_NOT_INITIALIZED );

    /* @req 4.2.2/SWS_EthIf_00168 */
    ETHIF_DET_REPORTERROR((CtrlIdx < EthIf_ConfigPointer->EthIfCtrlCount),ETHIF_SERVICE_ID_GET_EGRESS_TIME_STAMP, ETHIF_E_INV_CTRL_IDX );

    /* @req 4.2.2/SWS_EthIf_00169 */
    ETHIF_DET_REPORTERROR(((NULL != timeQualPtr) && (NULL != timeStampPtr)) ,ETHIF_SERVICE_ID_GET_EGRESS_TIME_STAMP, ETHIF_E_PARAM_POINTER );

    Eth_GetEgressTimeStamp(EthIf_ConfigPointer->EthIfCtrlCfg[CtrlIdx].EthIfEthCtrlId, BufIdx, timeQualPtr, timeStampPtr );
}

/**
 *
 * @param CtrlIdx
 * @param DataPtr
 * @param timeQualPtr
 * @param timeStampPtr
 */
/* @req 4.2.2/SWS_EthIf_00172 */
void EthIf_GetIngressTimeStamp( uint8 CtrlIdx, Eth_DataType* DataPtr, Eth_TimeStampQualType* timeQualPtr, Eth_TimeStampType* timeStampPtr )
{
    /* @req 4.2.2/SWS_EthIf_00173 */
    ETHIF_DET_REPORTERROR((ETHIF_STATE_INIT == EthIf_Internal.initStatus),ETHIF_SERVICE_ID_GET_INGRESS_TIME_STAMP, ETHIF_E_NOT_INITIALIZED );

    /* @req 4.2.2/SWS_EthIf_00174 */
    ETHIF_DET_REPORTERROR((CtrlIdx < EthIf_ConfigPointer->EthIfCtrlCount),ETHIF_SERVICE_ID_GET_INGRESS_TIME_STAMP, ETHIF_E_INV_CTRL_IDX );

    /* @req 4.2.2/SWS_EthIf_00175 */
    ETHIF_DET_REPORTERROR(((NULL != timeQualPtr) && (NULL != timeStampPtr) && (NULL != DataPtr)) ,ETHIF_SERVICE_ID_GET_INGRESS_TIME_STAMP, ETHIF_E_PARAM_POINTER );

    Eth_GetIngressTimeStamp(EthIf_ConfigPointer->EthIfCtrlCfg[CtrlIdx].EthIfEthCtrlId, DataPtr, timeQualPtr, timeStampPtr );
}

/**
 *
 * @param CtrlIdx
 * @param timeOffsetPtr
 * @param rateRatioPtr
 */
/* @req 4.2.2/SWS_EthIf_00178 */
void EthIf_SetCorrectionTime( uint8 CtrlIdx, const Eth_TimeIntDiffType* timeOffsetPtr, const Eth_RateRatioType* rateRatioPtr )
{
    /* @req 4.2.2/SWS_EthIf_00179 */
    ETHIF_DET_REPORTERROR((ETHIF_STATE_INIT == EthIf_Internal.initStatus),ETHIF_SERVICE_ID_SET_CORRECTION_TIME, ETHIF_E_NOT_INITIALIZED );

    /* @req 4.2.2/SWS_EthIf_00180 */
    ETHIF_DET_REPORTERROR((CtrlIdx < EthIf_ConfigPointer->EthIfCtrlCount),ETHIF_SERVICE_ID_SET_CORRECTION_TIME, ETHIF_E_INV_CTRL_IDX );

    /* @req 4.2.2/SWS_EthIf_00181 */
    ETHIF_DET_REPORTERROR(((NULL != timeOffsetPtr) && (NULL != rateRatioPtr)) ,ETHIF_SERVICE_ID_SET_CORRECTION_TIME, ETHIF_E_PARAM_POINTER );

    Eth_SetCorrectionTime(EthIf_ConfigPointer->EthIfCtrlCfg[CtrlIdx].EthIfEthCtrlId, timeOffsetPtr, rateRatioPtr );
}

/**
 *
 * @param CtrlIdx
 * @param timeStampPtr
 * @return
 */
/* @req 4.2.2/SWS_EthIf_00184 */
Std_ReturnType EthIf_SetGlobalTime( uint8 CtrlIdx, const Eth_TimeStampType* timeStampPtr ) {

    Std_ReturnType ret;

    /* @req 4.2.2/SWS_EthIf_000185 */
    ETHIF_DET_REPORTERROR((ETHIF_STATE_INIT == EthIf_Internal.initStatus),ETHIF_SERVICE_ID_SET_GLOBAL_TIME, ETHIF_E_NOT_INITIALIZED, E_NOT_OK);

    /* @req 4.2.2/SWS_EthIf_000186 */
    ETHIF_DET_REPORTERROR((CtrlIdx < EthIf_ConfigPointer->EthIfCtrlCount),ETHIF_SERVICE_ID_SET_GLOBAL_TIME, ETHIF_E_INV_CTRL_IDX, E_NOT_OK);

    /* @req 4.2.2/SWS_EthIf_000187 */
    ETHIF_DET_REPORTERROR((NULL != timeStampPtr),ETHIF_SERVICE_ID_SET_GLOBAL_TIME, ETHIF_E_PARAM_POINTER, E_NOT_OK);

    ret = Eth_SetGlobalTime(EthIf_ConfigPointer->EthIfCtrlCfg[CtrlIdx].EthIfEthCtrlId, timeStampPtr);

    return ret;
}
#endif

/**
 *
 * @param CtrlIdx
 * @param BufIdx
 */
/* @req 4.2.2/SWS_EthIf_00091 */ /* @req 4.2.2/SWS_EthIf_00096 */
void EthIf_TxConfirmation( uint8 CtrlIdx, Eth_BufIdxType BufIdx ) {

    uint8 i;
    uint8 txhandle;

    /* @req 4.2.2/SWS_EthIf_00092 */
    ETHIF_DET_REPORTERROR((ETHIF_STATE_INIT == EthIf_Internal.initStatus),ETHIF_SERVICE_ID_TX_CONFIRMATION, ETHIF_E_NOT_INITIALIZED);

    /* @req 4.2.2/SWS_EthIf_00093 */
    ETHIF_DET_REPORTERROR((CtrlIdx < EthIf_ConfigPointer->EthIfCtrlCount),ETHIF_SERVICE_ID_TX_CONFIRMATION, ETHIF_E_INV_CTRL_IDX);

    /* @req 4.2.2/SWS_EthIf_00094 */
    ETHIF_DET_REPORTERROR((BufIdx < EthIf_ConfigPointer->EthIfCtrlCfg[CtrlIdx].EthIfMaxTxBufsTotal),ETHIF_SERVICE_ID_TX_CONFIRMATION, ETHIF_E_INV_PARAM);

    /* TX Confirmation function to UL */
    if(NULL != EthIf_ConfigPointer->EthIfULTxConfirmation)
    {
        for (i=0; i < EthIf_ConfigPointer->EthIfOwnersCount; i++) {
            if (BufIdx == EthIf_Internal.ethIfRunTime[i].bufferIdx ) {
                txhandle = EthIf_ConfigPointer->EthIfOwnerCfg[i].EthIfTxConfirmationHandle;

                if (INVALID_ETHIF_HANDLE != txhandle) {
                    /* @req 4.2.2/SWS_EthIf_00125 */
                    EthIf_ConfigPointer->EthIfULTxConfirmation[txhandle]( CtrlIdx, BufIdx );
                }
                break;
            }
        }
    }
}
/**
 *
 * @param CtrlIdx
 * @param CtrlMode
 */
/* @req 4.2.2/SWS_EthIf_00231 */
void EthIf_CtrlModeIndication( uint8 CtrlIdx, Eth_ModeType CtrlMode ) {

    /* @req 4.2.2/SWS_EthIf_00017 */
    ETHIF_DET_REPORTERROR((ETHIF_STATE_INIT == EthIf_Internal.initStatus),ETHIF_SERVICE_ID_CTRL_MODE_INDICATION, ETHIF_E_NOT_INITIALIZED);

    /* @req 4.2.2/SWS_EthIf_00017 */
    ETHIF_DET_REPORTERROR((CtrlIdx < EthIf_ConfigPointer->EthIfCtrlCount),ETHIF_SERVICE_ID_CTRL_MODE_INDICATION, ETHIF_E_INV_CTRL_IDX);

    /* @req 4.2.2/SWS_EthIf_00252 */
    EthSM_CtrlModeIndication(EthIf_ConfigPointer->EthIfCtrlCfg[CtrlIdx].EthIfCtrlId, CtrlMode);
}

#if (ETHIF_ENABLE_RX_INTERRUPT == STD_OFF)
/* @req 4.2.2/SWS_EthIf_00099 */
/* @req 4.2.2/SWS_EthIf_00097 */
/* @req 4.2.2/SWS_EthIf_00004 */
void EthIf_MainFunctionRx( void ) {

    Eth_RxStatusType RxStatusPtr;
    RxStatusPtr = ETH_NOT_RECEIVED;
    uint8 i;
    uint8 CtrlIdx;

    /* @req 4.2.2/SWS_EthIf_00098 */
    ETHIF_DET_REPORTERROR((ETHIF_STATE_INIT == EthIf_Internal.initStatus),ETHIF_SERVICE_ID_MAIN_FUNCTION_RX, ETHIF_E_NOT_INITIALIZED);

    for(CtrlIdx = 0u; CtrlIdx < ETHIF_CTRLS_CNT; CtrlIdx++)
    {
        Eth_Receive(EthIf_ConfigPointer->EthIfCtrlCfg[CtrlIdx].EthIfEthCtrlId, &RxStatusPtr);

        for(i= 0; i < ETHIF_RX_INDICATION_ITERATIONS; i++)
        {
            if(RxStatusPtr == ETH_RECEIVED_MORE_DATA_AVAILABLE)
            {
                Eth_Receive(EthIf_ConfigPointer->EthIfCtrlCfg[CtrlIdx].EthIfEthCtrlId, &RxStatusPtr);
            } else {
                break;
            }


        }
    }
}

#endif

#if (ETHIF_ENABLE_TX_INTERRUPT == STD_OFF)
/* @req 4.2.2/SWS_EthIf_00100 */
/* @req 4.2.2/SWS_EthIf_00004 */
/**
 *
 */
/* @req 4.2.2/SWS_EthIf_00113 */
void EthIf_MainFunctionTx( void ){

    uint8 CtrlIdx;

    /* @req 4.2.2/SWS_EthIf_00124 */
    ETHIF_DET_REPORTERROR((ETHIF_STATE_INIT == EthIf_Internal.initStatus),ETHIF_SERVICE_ID_MAIN_FUNCTION_TX, ETHIF_E_NOT_INITIALIZED);

    for(CtrlIdx=0; CtrlIdx < ETHIF_CTRLS_CNT; CtrlIdx++)
    {
        /* @req 4.2.2/SWS_EthIf_00115 */
        Eth_TxConfirmation( EthIf_ConfigPointer->EthIfCtrlCfg[CtrlIdx].EthIfEthCtrlId );
    }
}
#endif

/**
 * @param CtrlIdx
 * @param FrameType
 * @param Priority
 * @param BufIdxPtr
 * @param BufPtr
 * @param LenBytePtr
 * @return
 */
/* @req 4.2.2/SWS_EthIf_00067 */
BufReq_ReturnType EthIf_ProvideTxBuffer( uint8 CtrlIdx, Eth_FrameType FrameType, uint8 Priority, Eth_BufIdxType* BufIdxPtr, uint8** BufPtr, uint16* LenBytePtr ) {

    BufReq_ReturnType ret;
    uint8 * IntrnlBuf;
    (void)FrameType;

    /* @req 4.2.2/SWS_EthIf_00069 */
    ETHIF_DET_REPORTERROR((ETHIF_STATE_INIT == EthIf_Internal.initStatus),ETHIF_SERVICE_ID_PROVIDE_TX_BUFFER, ETHIF_E_NOT_INITIALIZED, BUFREQ_NOT_OK);

    /* @req 4.2.2/SWS_EthIf_00070 */
    ETHIF_DET_REPORTERROR((CtrlIdx < EthIf_ConfigPointer->EthIfCtrlCount),ETHIF_SERVICE_ID_PROVIDE_TX_BUFFER, ETHIF_E_INV_CTRL_IDX, BUFREQ_NOT_OK);

    /* @req 4.2.2/SWS_EthIf_00071 */
    ETHIF_DET_REPORTERROR((NULL != BufIdxPtr),ETHIF_SERVICE_ID_PROVIDE_TX_BUFFER, ETHIF_E_PARAM_POINTER, BUFREQ_NOT_OK);

    /* @req 4.2.2/SWS_EthIf_00072 */
    ETHIF_DET_REPORTERROR((NULL != BufPtr),ETHIF_SERVICE_ID_PROVIDE_TX_BUFFER, ETHIF_E_PARAM_POINTER, BUFREQ_NOT_OK);

    /* @req 4.2.2/SWS_EthIf_00073 */
    ETHIF_DET_REPORTERROR((NULL != LenBytePtr),ETHIF_SERVICE_ID_PROVIDE_TX_BUFFER, ETHIF_E_PARAM_POINTER, BUFREQ_NOT_OK);

    /* @req 4.2.2/SWS_EthIf_00146*/
    if(INVALID_VLAN_ID == EthIf_ConfigPointer->EthIfCtrlCfg[CtrlIdx].EthIfVlanId)
    {
        if(EthIf_ConfigPointer->EthIfCtrlCfg[CtrlIdx].EthIfCtrlMtu >= *LenBytePtr)
        {
            /*@req 4.2.2/SWS_EthIf_00068 */
            ret = Eth_ProvideTxBuffer(EthIf_ConfigPointer->EthIfCtrlCfg[CtrlIdx].EthIfEthCtrlId, BufIdxPtr, BufPtr, LenBytePtr);
        } else {
            ret = BUFREQ_NOT_OK;
        }
    }
    /* @req 4.2.2/SWS_EthIf_00147*/ /* @req 4.2.2/SWS_EthIf_00128 */
    else
    {
        /* @req 4.2.2/SWS_EthIf_00129 */ /* @req 4.2.2/SWS_EthIf_00130 */
        if((EthIf_ConfigPointer->EthIfCtrlCfg[CtrlIdx].EthIfCtrlMtu >= *LenBytePtr) && (Priority < PRIORITY_SIZE))
        {
            *LenBytePtr= (*LenBytePtr + VLAN_TAG_SIZE);
            /*@req 4.2.2/SWS_EthIf_00068 */
            ret = Eth_ProvideTxBuffer(EthIf_ConfigPointer->EthIfCtrlCfg[CtrlIdx].EthIfEthCtrlId, BufIdxPtr, BufPtr, LenBytePtr);

            IntrnlBuf = *BufPtr;
            if (BUFREQ_OK == ret) {
                IntrnlBuf[0]  = (uint8)( Priority << PCP_SHIFT_BITS_5);
                IntrnlBuf[0] &= (uint8)((~((uint8)1) << (PCP_SHIFT_BITS_4)));
                IntrnlBuf[0] |= (uint8)((EthIf_ConfigPointer->EthIfCtrlCfg[CtrlIdx].EthIfVlanId & VLAN_MASK_NIBBLE) >> SHIFT_EIGHT_BITS);
                IntrnlBuf[1]  = (uint8)(EthIf_ConfigPointer->EthIfCtrlCfg[CtrlIdx].EthIfVlanId & VLAN_MASK_BYTE);
                IntrnlBuf[2]  = (uint8)((FrameType >> ETHIF_SHIFT_BYTE1) & ETHIF_BYTE_MASK);
                IntrnlBuf[3]  = (uint8)( FrameType & ETHIF_BYTE_MASK);
            }

            /*lint -e{438} */
            *BufPtr = &IntrnlBuf[VLAN_TAG_SIZE];
            *LenBytePtr = (*LenBytePtr - VLAN_TAG_SIZE);
        }else {
            ret = BUFREQ_NOT_OK;
        }
    }
    return ret;
}

/**
 *
 * @param CtrlIdx
 * @param BufIdx
 * @param FrameType
 * @param TxConfirmation
 * @param LenByte
 * @param PhysAddrPtr
 * @return
 */
/* @req 4.2.2/SWS_EthIf_00075 */
Std_ReturnType EthIf_Transmit( uint8 CtrlIdx, Eth_BufIdxType BufIdx, Eth_FrameType FrameType, boolean TxConfirmation, uint16 LenByte, const uint8* PhysAddrPtr ) {

    Std_ReturnType ret;
    uint8 i;

    /* @req 4.2.2/SWS_EthIf_00077 */
    ETHIF_DET_REPORTERROR((ETHIF_STATE_INIT == EthIf_Internal.initStatus),ETHIF_SERVICE_ID_TRANSMIT, ETHIF_E_NOT_INITIALIZED, E_NOT_OK);

    /* @req 4.2.2/SWS_EthIf_00078 */
    ETHIF_DET_REPORTERROR((CtrlIdx < EthIf_ConfigPointer->EthIfCtrlCount),ETHIF_SERVICE_ID_TRANSMIT, ETHIF_E_INV_CTRL_IDX, E_NOT_OK);

    /* @req 4.2.2/SWS_EthIf_00079 */
    ETHIF_DET_REPORTERROR((BufIdx < EthIf_ConfigPointer->EthIfCtrlCfg[CtrlIdx].EthIfMaxTxBufsTotal),ETHIF_SERVICE_ID_TRANSMIT, ETHIF_E_INV_PARAM, E_NOT_OK);

    /* @req 4.2.2/SWS_EthIf_00080 */
    ETHIF_DET_REPORTERROR((NULL != PhysAddrPtr),ETHIF_SERVICE_ID_TRANSMIT, ETHIF_E_PARAM_POINTER, E_NOT_OK);

    if(EthIf_ConfigPointer->EthIfCtrlCfg[CtrlIdx].EthIfCtrlMtu >= LenByte)
    {
        /* @req 4.2.2/SWS_EthIf_00250 */
        if(INVALID_VLAN_ID != EthIf_ConfigPointer->EthIfCtrlCfg[CtrlIdx].EthIfVlanId)
        {
            /* @req 4.2.2/SWS_EthIf_00076 */
            ret = Eth_Transmit(EthIf_ConfigPointer->EthIfCtrlCfg[CtrlIdx].EthIfEthCtrlId, BufIdx, ETH_FRAME_TYPE_VLAN, TxConfirmation, (LenByte + VLAN_TAG_SIZE), PhysAddrPtr);
        }
        else
        {
            /* @req 4.2.2/SWS_EthIf_00076 */
            ret = Eth_Transmit(EthIf_ConfigPointer->EthIfCtrlCfg[CtrlIdx].EthIfEthCtrlId, BufIdx, FrameType, TxConfirmation, LenByte, PhysAddrPtr);
        }
        if ((E_OK == ret) && (EthIf_ConfigPointer->EthIfULTxConfirmation != NULL_PTR)){
            for (i=0; i < EthIf_ConfigPointer->EthIfOwnersCount; i++) {
                if ((FrameType == EthIf_Internal.ethIfRunTime[i].frameType)) {
                    if (TRUE == TxConfirmation) {
                        EthIf_Internal.ethIfRunTime[i].bufferIdx = BufIdx; /* Currently used buffer index stored for the requested frame type */
                    } else {
                        EthIf_Internal.ethIfRunTime[i].bufferIdx = INVALID_BUFFER_INDEX; /* Reset the buffer index when no TxConfirmation is requested */
                    }
                    break;
                }
            }
        }
    } else{
        ret = E_NOT_OK;
    }
    return ret;
}

/**
 *
 * @param CtrlIdx
 * @param FrameType
 * @param IsBroadcast
 * @param PhysAddrPtr
 * @param DataPtr
 * @param LenByte
 */
/* @req 4.2.2/SWS_EthIf_00085 */ /* @req 4.2.2/SWS_EthIf_00151 */
/* @req 4.2.2/SWS_EthIf_00090 */ /* EthIf_RxIndication function shall be callable on interrupt level.*/
void EthIf_RxIndication( uint8 CtrlIdx, Eth_FrameType FrameType, boolean IsBroadcast, const uint8* PhysAddrPtr, Eth_DataType* DataPtr, uint16 LenByte ) {

    uint8 i;
    uint8 handle;
    Eth_FrameType adaptFrameType;
    adaptFrameType = 0u;
    Eth_DataType* adaptDataPtr;
    adaptDataPtr = NULL_PTR;
    uint16 adaptLenByte;
    adaptLenByte = 0u;
    boolean status;
    status = TRUE;

    /* @req 4.2.2/SWS_EthIf_00086 */
    ETHIF_DET_REPORTERROR((ETHIF_STATE_INIT == EthIf_Internal.initStatus),ETHIF_SERVICE_ID_RX_INDICATION, ETHIF_E_NOT_INITIALIZED);

    /* @req 4.2.2/SWS_EthIf_00087 */
    ETHIF_DET_REPORTERROR((CtrlIdx < EthIf_ConfigPointer->EthIfCtrlCount),ETHIF_SERVICE_ID_RX_INDICATION, ETHIF_E_INV_CTRL_IDX);

    /* @req 4.2.2/SWS_EthIf_00088 */
    ETHIF_DET_REPORTERROR((NULL != DataPtr),ETHIF_SERVICE_ID_RX_INDICATION, ETHIF_E_PARAM_POINTER);

    /* @req 4.2.2/SWS_EthIf_00145 */
    if(INVALID_VLAN_ID == EthIf_ConfigPointer->EthIfCtrlCfg[CtrlIdx].EthIfVlanId){
        adaptFrameType = FrameType;
        adaptDataPtr   = DataPtr;
        adaptLenByte   = LenByte;
    }else {
        if(FrameType == ETH_FRAME_TYPE_VLAN){
            uint16 tCI = 0u;
            tCI |= (uint16) DataPtr[0] << ETHIF_SHIFT_BYTE1;/*lint !e9033 no harm in shifting */
            tCI |= (uint16) DataPtr[1];/*lint !e9033 no harm in taking the byte pointer*/

            if((tCI & ETHIF_VID_MASK) != EthIf_ConfigPointer->EthIfCtrlCfg[CtrlIdx].EthIfVlanId ){
                status = FALSE;
                /* Error- no processing involved */
            } else {
                adaptFrameType = 0u;
                adaptDataPtr   = &DataPtr[VLAN_TAG_SIZE];
                adaptLenByte   = LenByte-VLAN_TAG_SIZE;
                adaptFrameType |=  (Eth_FrameType) DataPtr[2] << ETHIF_SHIFT_BYTE1; /*lint !e9033 no harm in shifting */
                adaptFrameType |=  (Eth_FrameType) DataPtr[3];/*lint !e9033 no harm in taking the byte pointer*/
            }
        }else{
            status = FALSE;
            /* Error- no processing involved */
        }
    }
    if (status == TRUE) {
        for(i =0 ; i <  EthIf_ConfigPointer->EthIfOwnersCount; i ++){
           if(adaptFrameType == EthIf_ConfigPointer->EthIfOwnerCfg[i].EthIfFrameType){
               handle = EthIf_ConfigPointer->EthIfOwnerCfg[i].EthIfRxIndicationHandle;
               if( handle != INVALID_ETHIF_HANDLE){
                   EthIf_ConfigPointer->EthIfULRxIndication[handle](CtrlIdx, adaptFrameType, IsBroadcast, PhysAddrPtr, adaptDataPtr, adaptLenByte);
               }
           }
        }
    }
}


#ifdef HOST_TEST
EthIf_StateType readinternal_ethif_status(void );
EthIf_StateType readinternal_ethif_status(void)
{
    return EthIf_Internal.initStatus;
}
#endif
