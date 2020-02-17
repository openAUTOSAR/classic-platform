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


/*
 * General requirements
 * */


/* ----------------------------[includes]------------------------------------*/
#include "Cpu.h"
#include "Mcu.h"
#include "Can.h"
#include "isr.h"
#include "irq.h"
#include "Dio.h"
#include "Spi.h"

/* @req SWS_CanTrcv_00067 */
#include "CanTrcv.h"
#include "CanTrcv_Internal.h"

#if defined(CANTRCV_DEVICE_TYPE) && defined(TJA1145)
#if (CANTRCV_DEVICE_TYPE != TJA1145)
#error CANTRCV: CanTrcvDevice is not properly configured
#endif
#endif

#include "CanTrcv_TJA1145.h"


#if (CAN_DEV_ERROR_DETECT == STD_ON)
#include "Det.h"
#endif
#include "EcuM.h"
#include "CanIf.h"



/* ----------------------------[private define]------------------------------*/

/* ----------------------------[private typedef]-----------------------------*/



/* ----------------------------[private function prototypes]-----------------*/


/* ----------------------------[private variables]---------------------------*/

Spi_SeqResultType SeqResult;
Std_ReturnType ApiReturn;
static CanTrcv_InternalType CanTrcv_Internal;
const CanTrcv_ConfigType * CanTrcv_ConfigPtr;

/* ----------------------------[private macro]-------------------------------*/

#if  ( CANTRCV_DEV_ERROR_DETECT == STD_ON )
#define DET_REPORT_ERROR(_api,_err) Det_ReportError(CANTRCV_MODULE_ID, 0, _api, _err)

#define VALIDATE(_exp,_api,_err,...) \
  if( !(_exp) ) { \
      (void)DET_REPORT_ERROR(_api, _err); \
      return __VA_ARGS__; \
  }

#else
#define DET_REPORT_ERROR(_api,_err)
#define VALIDATE(_exp,_api,_err,...) \
        if( !(_exp) ) { \
            return __VA_ARGS__; \
        }
#endif

/* ----------------------------[private functions]---------------------------*/
static Std_ReturnType initializeTransceiver(uint8 trcv,CanTrcv_TrcvWakeupReasonType* wakeupReason);
#if (CANTRCV_WAKE_UP_SUPPORTED == STD_ON)
static Std_ReturnType initializeSelectiveWakeup(uint8 trcv,CanTrcv_TrcvWakeupReasonType* wakeupReason,uint8 apiId);
#endif
static Std_ReturnType canTrcvSetMode(uint8 trcv,CanTrcv_TrcvModeType currMode,const CanTrcv_SpiSequenceConfigType *spiSeq,CanTrcv_TrcvModeType OpMode);

/* Initialization sequence for transceivers */
static Std_ReturnType initializeTransceiver(uint8 trcv,CanTrcv_TrcvWakeupReasonType* wakeupReason) {

	const CanTrcv_SpiSequenceConfigType *spiSeq;
    CanTrcv_TrcvModeType mode;
    Std_ReturnType ret;
    ret = E_OK;
    spiSeq = CanTrcv_ConfigPtr->CanTrcvChannel[trcv].CanTrcvSpiSequenceConfig;

#if (CANTRCV_WAKE_UP_SUPPORTED == STD_ON)
    if (TRUE == CanTrcv_ConfigPtr->CanTrcvChannel[trcv].CanTrcvWakeupByBus)
    {
        /* Selective wakeup - Partial networking */
        if (E_NOT_OK == initializeSelectiveWakeup(trcv,wakeupReason,CANTRCV_INIT_ID))
        {
            ret = E_NOT_OK;
        } else {
            /* Do nothing */
        }
        if (E_OK == ret) {
            /* @req SWS_CanTrcv_00113 */ /* Clear all wake up events otherwise subsequent mode transition to sleep is impossible */
            VALIDATE((E_OK == CanTrcv_Hw_ClearWakeUpEventStatus(spiSeq)),CANTRCV_INIT_ID, CANTRCV_E_NO_TRCV_CONTROL,E_NOT_OK);
            CanTrcv_Internal.CanTrcv_RuntimeData[trcv].CanTrcv_CurWakeupFlag = FALSE;
            /* @req SWS_CanTrcv_00113 */
            /* Enable selective or standard ISO 11898 -2 wakeup pattern based on PN support*/
            VALIDATE((E_OK == CanTrcv_Hw_EnableWakeUpEvent(spiSeq)),
                    CANTRCV_INIT_ID, CANTRCV_E_NO_TRCV_CONTROL,E_NOT_OK);
            CanTrcv_Internal.CanTrcv_RuntimeData[trcv].CanTrcv_CurWakeupMode = CANTRCV_WUMODE_ENABLE;
        }
    }
#endif

    if (E_OK == ret) {
        /* Set Initial mode */
        /* @req SWS_CanTrcv_00100 */
        /* @req SWS_CanTrcv_00113 */
        mode =CanTrcv_ConfigPtr->CanTrcvChannel[trcv].CanTrcvInitState;
        VALIDATE((E_OK == CanTrcv_Hw_SetupMode(mode,spiSeq)),
                CANTRCV_INIT_ID, CANTRCV_E_NO_TRCV_CONTROL,E_NOT_OK);
        CanTrcv_Internal.CanTrcv_RuntimeData[trcv].CanTrcv_TrcvCurMode = mode;
    }

    return ret;
}

#if (CANTRCV_WAKE_UP_SUPPORTED == STD_ON)
/* Initialization for selective wakeup */
static Std_ReturnType initializeSelectiveWakeup(uint8 trcv,CanTrcv_TrcvWakeupReasonType* wakeupReason,uint8 apiId)
{
    const CanTrcv_SpiSequenceConfigType *spiSeq;
    boolean reset;
    boolean failSts;
    boolean configErrSts;
    boolean wakeupFlag;

    reset = FALSE;
    failSts = FALSE;
    configErrSts = FALSE;
    spiSeq = CanTrcv_ConfigPtr->CanTrcvChannel[trcv].CanTrcvSpiSequenceConfig;

    if (TRUE == CanTrcv_ConfigPtr->CanTrcvChannel[trcv].CanTrcvPnSupport)
    {
        /* @req SWS_CanTrcv_00181 */
        /* Read Power on state */
        /* @req SWS_CanTrcv_00113 */

        VALIDATE((E_OK == CanTrcv_Hw_ReadWakeupEventStatus(wakeupReason,&wakeupFlag,spiSeq)), apiId, CANTRCV_E_NO_TRCV_CONTROL,E_NOT_OK);

        if (CANTRCV_WU_POWER_ON == * wakeupReason) {
            reset =TRUE;
        } else if (CANTRCV_WU_BY_SYSERR ==  * wakeupReason) {
            failSts = TRUE;
        } else {
            /* Do nothing */
        }

        /* Read PN configuration status if PN is supported*/
        /* @req SWS_CanTrcv_00113 */
        VALIDATE((E_OK == CanTrcv_Hw_ReadPNConfigurationStatus(&configErrSts,spiSeq)), apiId, CANTRCV_E_NO_TRCV_CONTROL,E_NOT_OK);

        /* In case of reset or error, run initialization of hardware */
        /* @req SWS_CanTrcv_00182 */
        if ((TRUE == reset) || (TRUE == configErrSts) || (TRUE == failSts))
        {
            /* Setup Baud rate */
            /* @req SWS_CanTrcv_00168 */
            VALIDATE((E_OK == CanTrcv_Hw_SetBaudRate(CanTrcv_ConfigPtr->CanTrcvChannel[trcv].CanTrcvPartialNetworkConfig->CanTrcvBaudRate,spiSeq)),
                    apiId, CANTRCV_E_BAUDRATE_NOT_SUPPORTED,E_NOT_OK);

            /* Setup Partial networking registers for selective wakeup ISO 11898 -6 */
            /* @req SWS_CanTrcv_00113 */
            VALIDATE((E_OK == CanTrcv_Hw_SetupPN(CanTrcv_ConfigPtr->CanTrcvChannel[trcv].CanTrcvPartialNetworkConfig,spiSeq)),
                    apiId, CANTRCV_E_NO_TRCV_CONTROL,E_NOT_OK);

            if (CanTrcv_ConfigPtr->CanTrcvChannel[trcv].CanTrcvPartialNetworkConfig->CanTrcvPnEnabled)
            {
                /* @req SWS_CanTrcv_00113 */
                VALIDATE((E_OK == CanTrcv_Hw_EnablePN(spiSeq)),apiId, CANTRCV_E_NO_TRCV_CONTROL,E_NOT_OK);
            } else {
                /* Do nothing */
            }
        } else {
            /* Do nothing */
        }
    }
    return E_OK;
}
#endif

/* Set transceiver mode */
static Std_ReturnType canTrcvSetMode(uint8 trcv,CanTrcv_TrcvModeType currMode,const CanTrcv_SpiSequenceConfigType *spiSeq,CanTrcv_TrcvModeType OpMode)
{
    Std_ReturnType ret;
    ret = E_OK;
    switch (currMode){
        /* @req SWS_CanTrcv_00103 */
        case CANTRCV_TRCVMODE_NORMAL:
            if (OpMode == CANTRCV_TRCVMODE_STANDBY){
                /* Transition from Normal mode to Standby Mode */
                /* @req SWS_CanTrcv_00114 */
                VALIDATE((E_OK == CanTrcv_Hw_SetupMode(OpMode,spiSeq)),
                        CANTRCV_SET_OPMODE_ID, CANTRCV_E_NO_TRCV_CONTROL,E_NOT_OK);
            }
            else if (OpMode == CANTRCV_TRCVMODE_SLEEP){
                /* Transition from Normal mode to Sleep Mode is invalid*/
                ret =E_NOT_OK;
            }
            else if (OpMode == CANTRCV_TRCVMODE_NORMAL){ /* Do nothing */
            } else {
                /* Do nothing */
            }
            break;

            /* @req SWS_CanTrcv_00104 */
        case CANTRCV_TRCVMODE_STANDBY:
            if (OpMode == CANTRCV_TRCVMODE_NORMAL){
                /* Transition from Standby mode to Normal Mode */
                /* @req SWS_CanTrcv_00114 */
                VALIDATE((E_OK == CanTrcv_Hw_SetupMode(OpMode,spiSeq)),
                        CANTRCV_SET_OPMODE_ID, CANTRCV_E_NO_TRCV_CONTROL,E_NOT_OK);
            }
            else if (OpMode == CANTRCV_TRCVMODE_SLEEP){
#if (CANTRCV_WAKE_UP_SUPPORTED == STD_ON)
                if (TRUE == CanTrcv_ConfigPtr->CanTrcvChannel[trcv].CanTrcvWakeupByBus)
                {
                    /* Transition from Standby mode to Sleep Mode */
                    /* clear event flags otherwise sleep mode is not entered successfully*/
                    VALIDATE((E_OK == CanTrcv_Hw_ClearWakeUpEventStatus(spiSeq)),
                                            CANTRCV_SET_OPMODE_ID, CANTRCV_E_NO_TRCV_CONTROL,E_NOT_OK);
                    CanTrcv_Internal.CanTrcv_RuntimeData[trcv].CanTrcv_CurWakeupFlag = FALSE;
                }
#else
                (void)trcv;
#endif
                /* @req SWS_CanTrcv_00114 */
                VALIDATE((E_OK == CanTrcv_Hw_SetupMode(OpMode,spiSeq)),
                        CANTRCV_SET_OPMODE_ID, CANTRCV_E_NO_TRCV_CONTROL,E_NOT_OK);
            }
            else if (OpMode == CANTRCV_TRCVMODE_STANDBY){
                /* Do nothing */
            } else {
                /* Do nothing */
            }
            break;

        case CANTRCV_TRCVMODE_SLEEP:
            if (OpMode == CANTRCV_TRCVMODE_NORMAL){
                /* Transition from Sleep mode to Normal Mode */
                /* @req SWS_CanTrcv_00114 */
                VALIDATE((E_OK == CanTrcv_Hw_SetupMode(OpMode,spiSeq)),
                        CANTRCV_SET_OPMODE_ID, CANTRCV_E_NO_TRCV_CONTROL,E_NOT_OK);
            }
            else if (OpMode == CANTRCV_TRCVMODE_STANDBY){
                /* Transition from Sleep mode to Standby Mode is invalid*/
                ret =E_NOT_OK;
            }
            else if (OpMode == CANTRCV_TRCVMODE_SLEEP){
                /* Do nothing */
            }  else {
                /* Do nothing */
            }
            break;

        default:
            ret = E_NOT_OK;
            break;
    }
    return ret;
}


/* ----------------------------[API]---------------------------*/

/** @req SWS_CanTrcv_00001 */
/** Initializes the CanTrcv module. **/
void CanTrcv_Init(const CanTrcv_ConfigType *ConfigPtr)
{

    CanTrcv_TrcvWakeupReasonType wakeUpReason;

    CanTrcv_Internal.initRun = FALSE;

    /* @req SWS_CanTrcv_00185 */
    VALIDATE(( NULL != ConfigPtr ), CANTRCV_INIT_ID, CANTRCV_E_PARAM_POINTER);

    CanTrcv_ConfigPtr = ConfigPtr;

    for(uint8 trcv=0;trcv<CANTRCV_CHANNEL_COUNT ;trcv++)
    {
//        CanTrcv_Internal.CanTrcv_RuntimeData[trcv].CanTrcv_TrcvCurMode = CANTRCV_TRCVMODE_UNITIALIZED;

        if (TRUE == CanTrcv_ConfigPtr->CanTrcvChannel[trcv].CanTrcvEnableStatus) {

            wakeUpReason = CANTRCV_WU_NOT_SUPPORTED;
            /* @req  SWS_CanTrcv_00180  */
            if (E_OK == initializeTransceiver(trcv,&wakeUpReason)){
#if (CANTRCV_WAKE_UP_SUPPORTED == STD_ON)
                /* Clear wake up flags as there is no requirement to remember power on or error wake up events */
                CanTrcv_Internal.CanTrcv_RuntimeData[trcv].CanTrcv_CurWakeupFlag = FALSE;
                CanTrcv_Internal.CanTrcv_RuntimeData[trcv].CanTrcv_TrcvWakeupReason = CANTRCV_WU_NOT_SUPPORTED;
                if (TRUE == CanTrcv_ConfigPtr->CanTrcvChannel[trcv].CanTrcvWakeupByBus)
                {
                    /* @req SWS_CanTrcv_00167 */
                    if (CANTRCV_WU_POWER_ON == wakeUpReason)
                    {
                        /* @req SWS_CanTrcv_00183 */
                        EcuM_SetWakeupEvent(CanTrcv_ConfigPtr->CanTrcvChannel[trcv].CanTrcvPorWakeupSourceRef);
                    } else if ((CANTRCV_WU_BY_SYSERR == wakeUpReason) && (CanTrcv_ConfigPtr->CanTrcvChannel[trcv].CanTrcvSyserrWakeupSourceRef !=CANTRCV_INVALID_WAKEUP_SOURCE))
                    {
                        /* @req SWS_CanTrcv_00184 */
                        EcuM_SetWakeupEvent(CanTrcv_ConfigPtr->CanTrcvChannel[trcv].CanTrcvSyserrWakeupSourceRef);
                    } else {
                        /* Do nothing */
                    }
                }
#endif
            }

        }

        /* Initialization done,Set the flag */
        CanTrcv_Internal.initRun = TRUE;
    }

} /* End of CanTrcv_Init function */


/* @req SWS_CanTrcv_00002 **/
/** Sets the mode of the Transceiver to the value OpMode. */
Std_ReturnType CanTrcv_SetOpMode(uint8 Transceiver, CanTrcv_TrcvModeType OpMode)
{
    const CanTrcv_SpiSequenceConfigType *spiSeq;
    CanTrcv_TrcvModeType currMode;
    CanTrcv_TrcvWakeupReasonType wakeUpReason;
    Std_ReturnType ret;
    boolean validSts;

    ret = E_OK;
    wakeUpReason = CANTRCV_WU_NOT_SUPPORTED;
    /* @req SWS_CanTrcv_00122 */
    VALIDATE((CanTrcv_Internal.initRun != FALSE), CANTRCV_SET_OPMODE_ID, CANTRCV_E_UNINIT, E_NOT_OK);

    /* @req SWS_CanTrcv_00123 */
    VALIDATE((Transceiver < CANTRCV_CHANNEL_COUNT), CANTRCV_SET_OPMODE_ID, CANTRCV_E_INVALID_TRANSCEIVER, E_NOT_OK);

    currMode = CanTrcv_Internal.CanTrcv_RuntimeData[Transceiver].CanTrcv_TrcvCurMode;
    /* @req SWS_CanTrcv_00102 */
    /* @req SWS_CanTrcv_00087 */
    /* @req SWS_CanTrcv_00105 */
    validSts = ((CANTRCV_TRCVMODE_NORMAL == OpMode) || (CANTRCV_TRCVMODE_STANDBY == OpMode) || (CANTRCV_TRCVMODE_SLEEP == OpMode));
    VALIDATE(validSts, CANTRCV_SET_OPMODE_ID, CANTRCV_E_PARAM_TRCV_OPMODE, E_NOT_OK);

    /* @req SWS_CanTrcv_00120 */
    validSts = !((currMode == CANTRCV_TRCVMODE_SLEEP) && (CANTRCV_TRCVMODE_STANDBY == OpMode));
    VALIDATE(validSts, CANTRCV_SET_OPMODE_ID, CANTRCV_E_TRCV_NOT_NORMAL, E_NOT_OK);

    /*@req SWS_CanTrcv_00121 */
    validSts = !((CANTRCV_TRCVMODE_SLEEP == OpMode) && (currMode == CANTRCV_TRCVMODE_NORMAL));
    VALIDATE(validSts, CANTRCV_SET_OPMODE_ID, CANTRCV_E_TRCV_NOT_STANDBY, E_NOT_OK);

    if (FALSE == CanTrcv_ConfigPtr->CanTrcvChannel[Transceiver].CanTrcvEnableStatus) {
        ret = E_NOT_OK;
    }
    else {
        spiSeq = CanTrcv_ConfigPtr->CanTrcvChannel[Transceiver].CanTrcvSpiSequenceConfig;
#if (CANTRCV_WAKE_UP_SUPPORTED == STD_ON)
        if (TRUE == CanTrcv_ConfigPtr->CanTrcvChannel[Transceiver].CanTrcvWakeupByBus)
        {
            /* @req SWS_CanTrcv_00186 */
            /* @req SWS_CanTrcv_00187 */
            /* Errors in configuration or bus errors lead to initialization */
            ret = initializeSelectiveWakeup(Transceiver,&wakeUpReason,CANTRCV_SET_OPMODE_ID);
        }
#else
        (void)wakeUpReason;
#endif
        if (E_OK == ret){
            if (E_OK == canTrcvSetMode(Transceiver,currMode,spiSeq,OpMode))
            {
                CanTrcv_Internal.CanTrcv_RuntimeData[Transceiver].CanTrcv_TrcvCurMode = OpMode;
#if (CANTRCV_WAKE_UP_SUPPORTED == STD_ON)
                /* We check if wake up & partial networking is supported before we indicate PN availability */
                if ((CanTrcv_Internal.CanTrcv_RuntimeData[Transceiver].CanTrcv_TrcvCurMode == CANTRCV_TRCVMODE_NORMAL)
                        || (TRUE == CanTrcv_ConfigPtr->CanTrcvChannel[Transceiver].CanTrcvPnSupport)
                        || (TRUE == CanTrcv_ConfigPtr->CanTrcvChannel[Transceiver].CanTrcvWakeupByBus))
                {
                    /* @req SWS_CanTrcv_00188 */
                    CanIf_ConfirmPnAvailability(Transceiver);
                }
#endif
            }
            else{
                ret = E_NOT_OK;
            }
        }
    }
    (void)wakeUpReason;
    return ret;
}


/** Gets the mode of the Transceiver and returns it in OpMode. */
/* @req SWS_CanTrcv_00005 */
Std_ReturnType CanTrcv_GetOpMode(uint8 Transceiver, CanTrcv_TrcvModeType* OpMode)
{
    const CanTrcv_SpiSequenceConfigType *spiSeq;
    Std_ReturnType ret;

    ret = E_OK;
    /* @req SWS_CanTrcv_00124 */
    VALIDATE((CanTrcv_Internal.initRun != FALSE), CANTRCV_GET_OPMODE_ID, CANTRCV_E_UNINIT, E_NOT_OK);

    /* @req SWS_CanTrcv_00129 */
    VALIDATE((Transceiver < CANTRCV_CHANNEL_COUNT), CANTRCV_GET_OPMODE_ID, CANTRCV_E_INVALID_TRANSCEIVER, E_NOT_OK);

    /* @req SWS_CanTrcv_00132 */
    VALIDATE((OpMode != NULL), CANTRCV_GET_OPMODE_ID, CANTRCV_E_PARAM_POINTER, E_NOT_OK);

    if (FALSE == CanTrcv_ConfigPtr->CanTrcvChannel[Transceiver].CanTrcvEnableStatus) {
        //QQ *OpMode= CANTRCV_TRCVMODE_UNITIALIZED;
        ret = E_NOT_OK;
    } else{
        spiSeq = CanTrcv_ConfigPtr->CanTrcvChannel[Transceiver].CanTrcvSpiSequenceConfig;
        /* @req SWS_CanTrcv_00115 */
        /* @req SWS_CanTrcv_00106 */
        VALIDATE((E_OK == CanTrcv_Hw_ReadCurrMode(OpMode,spiSeq)), CANTRCV_GET_OPMODE_ID, CANTRCV_E_NO_TRCV_CONTROL, E_NOT_OK);
    }
    return ret;
}


/** Gets the wakeup reason for the Transceiver and returns it in parameter Reason. */
/* @req SWS_CanTrcv_00007 **/
Std_ReturnType CanTrcv_GetBusWuReason(uint8 Transceiver, CanTrcv_TrcvWakeupReasonType* reason)
{
    Std_ReturnType ret;

    ret = E_OK;

    /* @req SWS_CanTrcv_00125 */
    VALIDATE((CanTrcv_Internal.initRun != FALSE), CANTRCV_GET_BUSWUREASON_ID, CANTRCV_E_UNINIT, E_NOT_OK);

    /* @req SWS_CanTrcv_00130 */
    VALIDATE((Transceiver < CANTRCV_CHANNEL_COUNT), CANTRCV_GET_BUSWUREASON_ID, CANTRCV_E_INVALID_TRANSCEIVER, E_NOT_OK);

    /* @req SWS_CanTrcv_00133 */
    VALIDATE((reason != NULL), CANTRCV_GET_BUSWUREASON_ID, CANTRCV_E_PARAM_POINTER, E_NOT_OK);

#if (CANTRCV_WAKE_UP_SUPPORTED == STD_ON)
    if ((FALSE == CanTrcv_ConfigPtr->CanTrcvChannel[Transceiver].CanTrcvEnableStatus)
            || ((FALSE == CanTrcv_Internal.CanTrcv_RuntimeData[Transceiver].CanTrcv_CurWakeupFlag))){
        ret =  E_NOT_OK; /* If there is no wake up detected return E_NOT_OK */
    }  else {
        *reason = CanTrcv_Internal.CanTrcv_RuntimeData[Transceiver].CanTrcv_TrcvWakeupReason ;
    }
#else
    ret = E_NOT_OK;
#endif
    return ret;
}


/** Gets the version of the module and returns it in VersionInfo. */
/* @req SWS_CanTrcv_00008 **/
#if ( CANTRCV_GET_VERSION_INFO == STD_ON )
#define CanTrcv_GetVersionInfo(_vi) STD_GET_VERSION_INFO(_vi,CANTRCV)
#endif

/** Enables, disables or clears wake-up events of the Transceiver according to TrcvWakeupMode. */
/* @req SWS_CanTrcv_00009 **/
Std_ReturnType CanTrcv_SetWakeupMode(uint8 Transceiver, CanTrcv_TrcvWakeupModeType TrcvWakeupMode)
{
    Std_ReturnType ret;
    boolean validSts;

    ret = E_OK;

    /* @req SWS_CanTrcv_00127 */
    VALIDATE((CanTrcv_Internal.initRun != FALSE), CANTRCV_SET_WAKEUPMODE_ID, CANTRCV_E_UNINIT, E_NOT_OK);

    /* @req SWS_CanTrcv_00131 */
    VALIDATE((Transceiver < CANTRCV_CHANNEL_COUNT), CANTRCV_SET_WAKEUPMODE_ID, CANTRCV_E_INVALID_TRANSCEIVER, E_NOT_OK);

    /* @req SWS_CanTrcv_00089 */
    validSts = ((TrcvWakeupMode == CANTRCV_WUMODE_ENABLE) || (TrcvWakeupMode == CANTRCV_WUMODE_CLEAR) || (TrcvWakeupMode == CANTRCV_WUMODE_DISABLE));
    VALIDATE(validSts, CANTRCV_SET_WAKEUPMODE_ID, CANTRCV_E_PARAM_TRCV_WAKEUP_MODE, E_NOT_OK);

#if (CANTRCV_WAKE_UP_SUPPORTED == STD_ON)
    const CanTrcv_SpiSequenceConfigType *spiSeq;

    if (FALSE == CanTrcv_ConfigPtr->CanTrcvChannel[Transceiver].CanTrcvEnableStatus) {
        ret = E_NOT_OK;

    } else if (TrcvWakeupMode != CanTrcv_Internal.CanTrcv_RuntimeData[Transceiver].CanTrcv_CurWakeupMode) {
        /* New mode is requested */
        spiSeq = CanTrcv_ConfigPtr->CanTrcvChannel[Transceiver].CanTrcvSpiSequenceConfig;
        /* @req SWS_CanTrcv_00111 */
       if (TrcvWakeupMode == CANTRCV_WUMODE_ENABLE)
       {
           /* @req SWS_CanTrcv_00117 */
           VALIDATE((E_OK == CanTrcv_Hw_EnableWakeUpEvent(spiSeq)), CANTRCV_SET_WAKEUPMODE_ID, CANTRCV_E_NO_TRCV_CONTROL, E_NOT_OK);
           CanTrcv_Internal.CanTrcv_RuntimeData[Transceiver].CanTrcv_CurWakeupMode = CANTRCV_WUMODE_ENABLE;
       }
       /* @req SWS_CanTrcv_00093 */
       else if (TrcvWakeupMode == CANTRCV_WUMODE_DISABLE)
       {
           /* @req SWS_CanTrcv_00095 */
           /* @req SWS_CanTrcv_00117 */
           /* Wake up is detected by the device but notification is disabled */
           CanTrcv_Internal.CanTrcv_RuntimeData[Transceiver].CanTrcv_CurWakeupMode = CANTRCV_WUMODE_DISABLE;
           /* @req SWS_CanTrcv_00150 */
           /* @req SWS_CanTrcv_00117 */
           VALIDATE((E_OK == CanTrcv_Hw_ClearWakeUpEventStatus(spiSeq)), CANTRCV_SET_WAKEUPMODE_ID, CANTRCV_E_NO_TRCV_CONTROL, E_NOT_OK);
           CanTrcv_Internal.CanTrcv_RuntimeData[Transceiver].CanTrcv_CurWakeupFlag = FALSE;
           CanTrcv_Internal.CanTrcv_RuntimeData[Transceiver].CanTrcv_CurWakeupMode = CANTRCV_WUMODE_CLEAR;
       }
       /* @req SWS_CanTrcv_00094 */
       else if (TrcvWakeupMode == CANTRCV_WUMODE_CLEAR)
       {
           /* @req SWS_CanTrcv_00117 */
           VALIDATE((E_OK == CanTrcv_Hw_ClearWakeUpEventStatus(spiSeq)), CANTRCV_SET_WAKEUPMODE_ID, CANTRCV_E_NO_TRCV_CONTROL, E_NOT_OK);
           CanTrcv_Internal.CanTrcv_RuntimeData[Transceiver].CanTrcv_CurWakeupFlag = FALSE;
           CanTrcv_Internal.CanTrcv_RuntimeData[Transceiver].CanTrcv_CurWakeupMode = CANTRCV_WUMODE_CLEAR;
       }  else {
           ret = E_NOT_OK;
       }
    }   else {
        /* Do nothing */
    }
#else
    ret = E_NOT_OK;
#endif
    return ret;
}


/** Reads the transceiver configuration/status data and returns it through para */
/* !req SWS_CanTrcv_00213 **/
Std_ReturnType CanTrcv_GetTrcvSystemData(uint8 Transceiver,uint32* TrcvSysData)
{
    Std_ReturnType ret = E_NOT_OK;

    ret = E_NOT_OK;

    /* SPI related local declaration */


    /* @req SWS_CanTrcv_00191 */
    VALIDATE((CanTrcv_Internal.initRun != FALSE), CANTRCV_GET_TRCVSYSTEMDATA_ID, CANTRCV_E_UNINIT, E_NOT_OK);

    /* @req SWS_CanTrcv_00192 */
    VALIDATE((Transceiver < CANTRCV_CHANNEL_COUNT), CANTRCV_GET_TRCVSYSTEMDATA_ID, CANTRCV_E_INVALID_TRANSCEIVER, E_NOT_OK);

    /* @req SWS_CanTrcv_00193 */
    VALIDATE((TrcvSysData != NULL), CANTRCV_GET_TRCVSYSTEMDATA_ID, CANTRCV_E_PARAM_POINTER, E_NOT_OK);

    if (FALSE == CanTrcv_ConfigPtr->CanTrcvChannel[Transceiver].CanTrcvEnableStatus) {
        ret = E_NOT_OK;
    }  else {
        /* Do nothing */
    }

    /* !req SWS_CanTrcv_00190 */
    return ret;
}


/** Clears the WUF flag in the transceiver hardware. This API shall exist only if CanTrcvHwPnSupport = TRUE. */
/* @req SWS_CanTrcv_00214 **/
Std_ReturnType CanTrcv_ClearTrcvWufFlag(uint8 Transceiver)
{
    Std_ReturnType ret;

    ret = E_OK;

    /* @req SWS_CanTrcv_00197 */
    VALIDATE((CanTrcv_Internal.initRun != FALSE), CANTRCV_CLEAR_TRCVWUFFLAG_ID, CANTRCV_E_UNINIT, E_NOT_OK);

    /* @req SWS_CanTrcv_00198 */
    VALIDATE((Transceiver < CANTRCV_CHANNEL_COUNT), CANTRCV_CLEAR_TRCVWUFFLAG_ID, CANTRCV_E_INVALID_TRANSCEIVER, E_NOT_OK);

#if (CANTRCV_WAKE_UP_SUPPORTED == STD_ON)
    const CanTrcv_SpiSequenceConfigType *spiSeq;

    if ((FALSE == CanTrcv_ConfigPtr->CanTrcvChannel[Transceiver].CanTrcvEnableStatus)|| (FALSE == CanTrcv_ConfigPtr->CanTrcvChannel[Transceiver].CanTrcvPnSupport)) {
        ret = E_NOT_OK;
    } else {

        spiSeq = CanTrcv_ConfigPtr->CanTrcvChannel[Transceiver].CanTrcvSpiSequenceConfig;
        /* @req SWS_CanTrcv_00196 */
        /* @req SWS_CanTrcv_00194 */
        VALIDATE((E_OK == CanTrcv_Hw_ClearWakeUpEventStatus(spiSeq)), CANTRCV_CLEAR_TRCVWUFFLAG_ID, CANTRCV_E_NO_TRCV_CONTROL, E_NOT_OK);
        /* @req SWS_CanTrcv_00196 */
        /* @req SWS_CanTrcv_00195 */
        CanTrcv_Internal.CanTrcv_RuntimeData[Transceiver].CanTrcv_CurWakeupFlag = FALSE;
        CanIf_ClearTrcvWufFlagIndication(Transceiver);
        /* Transition to standby mode happens later by CanIf_SetTrcvMode(). Ref flow diagram Deinitialization (SPI synchronous) */
    }
#else
    ret = E_NOT_OK;
#endif
    return ret;

}



/** Reads the status of the timeout flag from the transceiver hardware.
  * This API shall exist only if CanTrcvHwPnSupport = TRUE. */
/* !req SWS_CanTrcv_00215 **/
Std_ReturnType CanTrcv_ReadTrcvTimeoutFlag(uint8 Transceiver, CanTrcv_TrcvFlagStateType* FlagState)
{
    Std_ReturnType ret;

    ret = E_OK;

    VALIDATE((CanTrcv_Internal.initRun != FALSE), CANTRCV_READ_TRCVTIMEOUTFLAG_ID, CANTRCV_E_UNINIT, E_NOT_OK);

    /* @req SWS_CanTrcv_00199 */
    VALIDATE((Transceiver < CANTRCV_CHANNEL_COUNT), CANTRCV_READ_TRCVTIMEOUTFLAG_ID, CANTRCV_E_INVALID_TRANSCEIVER, E_NOT_OK);

    /* @req SWS_CanTrcv_00200 */
    VALIDATE((FlagState != NULL), CANTRCV_READ_TRCVTIMEOUTFLAG_ID, CANTRCV_E_PARAM_POINTER, E_NOT_OK);

    if ((FALSE == CanTrcv_ConfigPtr->CanTrcvChannel[Transceiver].CanTrcvEnableStatus)|| (FALSE == CanTrcv_ConfigPtr->CanTrcvChannel[Transceiver].CanTrcvPnSupport)) {
        ret = E_NOT_OK;
    } else {
        /* Do nothing */
    }
    return ret;
}


/** Clears the status of the timeout flag in the transceiver hardware.
  * This API shall exist only if CanTrcvHwPnSupport = TRUE. */
/* !req SWS_CanTrcv_00216 **/
Std_ReturnType CanTrcv_ClearTrcvTimeoutFlag(uint8 Transceiver)
{
    Std_ReturnType ret;

    ret = E_OK;

    VALIDATE((CanTrcv_Internal.initRun != FALSE), CANTRCV_CLEAR_TRCVTIMEOUTFLAG_ID, CANTRCV_E_UNINIT, E_NOT_OK);

    /* @req SWS_CanTrcv_00201 */
    VALIDATE((Transceiver < CANTRCV_CHANNEL_COUNT), CANTRCV_CLEAR_TRCVTIMEOUTFLAG_ID, CANTRCV_E_INVALID_TRANSCEIVER, E_NOT_OK);

    if ((FALSE == CanTrcv_ConfigPtr->CanTrcvChannel[Transceiver].CanTrcvEnableStatus)|| (FALSE == CanTrcv_ConfigPtr->CanTrcvChannel[Transceiver].CanTrcvPnSupport)) {
        ret = E_NOT_OK;
    } else {
        /* Do nothing */
    }

    return ret;
}


/** Reads the status of the silence flag from the transceiver hardware.
  * This API shall exist only if CanTrcvHwPnSupport = TRUE. */
/* @req SWS_CanTrcv_00217 **/
Std_ReturnType CanTrcv_ReadTrcvSilenceFlag(uint8 Transceiver, CanTrcv_TrcvFlagStateType* FlagState)
{
    const CanTrcv_SpiSequenceConfigType *spiSeq;
    Std_ReturnType ret;

    ret = E_OK;

    /* Without initialization it is not possible to read the status of silence flag */
    VALIDATE((CanTrcv_Internal.initRun != FALSE), CANTRCV_READ_TRCVSILENCEFLAG_ID, CANTRCV_E_UNINIT, E_NOT_OK);

    /* @req SWS_CanTrcv_00202 */
    VALIDATE((Transceiver < CANTRCV_CHANNEL_COUNT), CANTRCV_READ_TRCVSILENCEFLAG_ID, CANTRCV_E_INVALID_TRANSCEIVER, E_NOT_OK);

    /* @req SWS_CanTrcv_00203 */
    VALIDATE((FlagState != NULL), CANTRCV_READ_TRCVSILENCEFLAG_ID, CANTRCV_E_PARAM_POINTER, E_NOT_OK);

    if ((FALSE == CanTrcv_ConfigPtr->CanTrcvChannel[Transceiver].CanTrcvEnableStatus)|| (FALSE == CanTrcv_ConfigPtr->CanTrcvChannel[Transceiver].CanTrcvPnSupport)) {
        ret = E_NOT_OK;
    } else {
        spiSeq = CanTrcv_ConfigPtr->CanTrcvChannel[Transceiver].CanTrcvSpiSequenceConfig;
        VALIDATE((E_OK == CanTrcv_Hw_ReadSilenceFlag(FlagState,spiSeq)), CANTRCV_READ_TRCVSILENCEFLAG_ID, CANTRCV_E_NO_TRCV_CONTROL, E_NOT_OK);
    }

    return ret;
}


/** Service is called by underlying CANIF in case a wake up interrupt is detected. */
/* @req SWS_CanTrcv_00143 **/
Std_ReturnType CanTrcv_CheckWakeup(uint8 Transceiver)
{
    CanTrcv_TrcvWakeupReasonType reason;
    Std_ReturnType ret;
    boolean wakeupFlag;

    ret = E_OK;
    wakeupFlag = FALSE;
    reason = CANTRCV_WU_NOT_SUPPORTED;

    /* @req SWS_CanTrcv_00144 */
    VALIDATE((CanTrcv_Internal.initRun != FALSE), CANTRCV_CHECK_WAKEUP_ID, CANTRCV_E_UNINIT, E_NOT_OK);

    /* @req SWS_CanTrcv_00145 */
    VALIDATE((Transceiver < CANTRCV_CHANNEL_COUNT), CANTRCV_CHECK_WAKEUP_ID, CANTRCV_E_INVALID_TRANSCEIVER, E_NOT_OK);

#if (CANTRCV_WAKE_UP_SUPPORTED == STD_ON)
    const CanTrcv_SpiSequenceConfigType *spiSeq;

    if (FALSE == CanTrcv_ConfigPtr->CanTrcvChannel[Transceiver].CanTrcvEnableStatus) {
        ret = E_NOT_OK;
    } else {
        /* @req SWS_CanTrcv_00091 */

        spiSeq = CanTrcv_ConfigPtr->CanTrcvChannel[Transceiver].CanTrcvSpiSequenceConfig;

        /* @req SWS_CanTrcv_00146 */
        VALIDATE((E_OK == CanTrcv_Hw_ReadWakeupEventStatus(&reason,&wakeupFlag,spiSeq)), CANTRCV_CHECK_WAKEUP_ID, CANTRCV_E_NO_TRCV_CONTROL, E_NOT_OK);

        if (wakeupFlag){

            /* Transceiver should be brought to normal mode after sleep (wake up leads to transition to Standby in TJA1145) */
            VALIDATE((E_OK == canTrcvSetMode(Transceiver,CANTRCV_TRCVMODE_SLEEP,spiSeq, CANTRCV_TRCVMODE_NORMAL)), CANTRCV_CHECK_WAKEUP_ID, CANTRCV_E_NO_TRCV_CONTROL, E_NOT_OK);
            CanTrcv_Internal.CanTrcv_RuntimeData[Transceiver].CanTrcv_TrcvCurMode = CANTRCV_TRCVMODE_NORMAL;

            CanTrcv_Internal.CanTrcv_RuntimeData[Transceiver].CanTrcv_CurWakeupFlag = TRUE;
            CanTrcv_Internal.CanTrcv_RuntimeData[Transceiver].CanTrcv_TrcvWakeupReason = reason;

            /* The following requirements are derived from EcuM fixed sequence dig 9.2.3 in ASR 4.0.3 */
            if (CANTRCV_WUMODE_ENABLE == CanTrcv_Internal.CanTrcv_RuntimeData[Transceiver].CanTrcv_CurWakeupMode)
            {
                if ((CANTRCV_WU_BY_BUS == reason) && (CanTrcv_ConfigPtr->CanTrcvChannel[Transceiver].CanTrcvWakeupSourceRef != CANTRCV_INVALID_WAKEUP_SOURCE))
                {
                    EcuM_SetWakeupEvent(CanTrcv_ConfigPtr->CanTrcvChannel[Transceiver].CanTrcvWakeupSourceRef);
                }
                else if ((CANTRCV_WU_BY_SYSERR == reason) && (CanTrcv_ConfigPtr->CanTrcvChannel[Transceiver].CanTrcvSyserrWakeupSourceRef != CANTRCV_INVALID_WAKEUP_SOURCE))
                {
                    EcuM_SetWakeupEvent(CanTrcv_ConfigPtr->CanTrcvChannel[Transceiver].CanTrcvSyserrWakeupSourceRef);
                }
                else if ((CANTRCV_WU_BY_PIN == reason)|| (CANTRCV_WU_POWER_ON == reason))
                {
                    EcuM_SetWakeupEvent(CanTrcv_ConfigPtr->CanTrcvChannel[Transceiver].CanTrcvPorWakeupSourceRef);
                }  else {
                    /* Do nothing */
                }
            }

        } else {
            ret = E_NOT_OK;
        }
    }
#else
    (void)wakeupFlag;
    (void)reason;
    ret = E_NOT_OK;
#endif

    return ret;
}

/** The API configures the wake-up of the transceiver for Standby and Sleep Mode:
  * Either the CAN transceiver is woken up by a remote wake-up pattern
  * (standard CAN wake-up) or by the configured remote wake-up frame. */
/* @req SWS_CanTrcv_00219 **/
Std_ReturnType CanTrcv_SetPNActivationState(CanTrcv_PNActivationType ActivationState)
{
    Std_ReturnType ret;

    ret = E_OK;

    /* @req SWS_CanTrcv_00220 */
    VALIDATE((CanTrcv_Internal.initRun != FALSE), CANTRCV_SET_PNACTIVATIONSTATE_ID, CANTRCV_E_UNINIT, E_NOT_OK);

#if (CANTRCV_WAKE_UP_SUPPORTED == STD_ON)
    const CanTrcv_SpiSequenceConfigType *spiSeq;
    uint8 trcv;/*Transceiver count */
    boolean stsValid;

    /* @req SWS_CanTrcv_00222 */
    if (PN_DISABLED == ActivationState) {
        for(trcv=0;trcv<CANTRCV_CHANNEL_COUNT ;trcv++)
        {
            stsValid = (CanTrcv_ConfigPtr->CanTrcvChannel[trcv].CanTrcvEnableStatus) && (CanTrcv_ConfigPtr->CanTrcvChannel[trcv].CanTrcvWakeupByBus)
                            && (CanTrcv_ConfigPtr->CanTrcvChannel[trcv].CanTrcvPnSupport)
                            && (CanTrcv_ConfigPtr->CanTrcvChannel[trcv].CanTrcvPartialNetworkConfig->CanTrcvPnEnabled) ;
            if (TRUE == stsValid)
            {
                spiSeq = CanTrcv_ConfigPtr->CanTrcvChannel[trcv].CanTrcvSpiSequenceConfig;
                VALIDATE((E_OK == CanTrcv_Hw_DisablePN(spiSeq)), CANTRCV_SET_PNACTIVATIONSTATE_ID, CANTRCV_E_NO_TRCV_CONTROL, E_NOT_OK);
            } else {
                ret = E_NOT_OK;
            }
        }
    }
    /* @req SWS_CanTrcv_00221 */
    else if (PN_ENABLED == ActivationState)
    {
        for(uint8 trcv_index=0;trcv_index<CANTRCV_CHANNEL_COUNT ;trcv_index++)
        {
            stsValid = (CanTrcv_ConfigPtr->CanTrcvChannel[trcv_index].CanTrcvEnableStatus) && (CanTrcv_ConfigPtr->CanTrcvChannel[trcv_index].CanTrcvWakeupByBus)
                            && (CanTrcv_ConfigPtr->CanTrcvChannel[trcv_index].CanTrcvPnSupport)
                            && (CanTrcv_ConfigPtr->CanTrcvChannel[trcv_index].CanTrcvPartialNetworkConfig->CanTrcvPnEnabled) ;
            if (TRUE == stsValid)
            {
                spiSeq = CanTrcv_ConfigPtr->CanTrcvChannel[trcv_index].CanTrcvSpiSequenceConfig;
                VALIDATE((E_OK == CanTrcv_Hw_EnablePN(spiSeq)), CANTRCV_SET_PNACTIVATIONSTATE_ID, CANTRCV_E_NO_TRCV_CONTROL, E_NOT_OK);
            } else {
                ret = E_NOT_OK;
            }
        }
    }
#else
    ret = E_NOT_OK;
#endif

    return ret;
}


/** Requests to check the status of the wakeup flag from the transceiver hardware. */
/* @req SWS_CanTrcv_00223 **/
Std_ReturnType CanTrcv_CheckWakeFlag(uint8 Transceiver)
{
    CanTrcv_TrcvWakeupReasonType reason;
    Std_ReturnType ret;
    boolean wakeupFlag;

    ret = E_OK;
    reason = CANTRCV_WU_NOT_SUPPORTED;
    wakeupFlag = FALSE;

    /* @req SWS_CanTrcv_00225 */
    VALIDATE((Transceiver < CANTRCV_CHANNEL_COUNT), CANTRCV_CHECK_WAKEFLAG_ID, CANTRCV_E_INVALID_TRANSCEIVER, E_NOT_OK);

#if (CANTRCV_WAKE_UP_SUPPORTED == STD_ON)
    const CanTrcv_SpiSequenceConfigType *spiSeq;

    if (FALSE == CanTrcv_ConfigPtr->CanTrcvChannel[Transceiver].CanTrcvEnableStatus) {
        ret = E_NOT_OK;
    } else {
        spiSeq = CanTrcv_ConfigPtr->CanTrcvChannel[Transceiver].CanTrcvSpiSequenceConfig;
        VALIDATE((E_OK == CanTrcv_Hw_ReadWakeupEventStatus(&reason,&wakeupFlag,spiSeq)), CANTRCV_CHECK_WAKEFLAG_ID, CANTRCV_E_NO_TRCV_CONTROL, E_NOT_OK);

        /* @req SWS_CanTrcv_00224 */
        if (TRUE == wakeupFlag)
        {
            CanTrcv_Internal.CanTrcv_RuntimeData[Transceiver].CanTrcv_CurWakeupFlag = TRUE;
            CanTrcv_Internal.CanTrcv_RuntimeData[Transceiver].CanTrcv_TrcvWakeupReason = reason;
            CanIf_CheckTrcvWakeFlagIndication(Transceiver);
        }
    }
#else
    (void)reason;
    (void)wakeupFlag;
    ret = E_NOT_OK;
#endif
    return ret;
}


/** Service to scan all busses for wake up events and perform these event. */
/* Note: If Can Transceiver controls the power supply stage then there is no need
 * of scheduling CanTrcv_MainFunction as the ECU enters low power mode when transceiver
 * is switched to sleep.
 */
/* @req SWS_CanTrcv_00013 **/
void CanTrcv_MainFunction(void)
{

    /* @req SWS_CanTrcv_00128 */
    VALIDATE((CanTrcv_Internal.initRun != FALSE), CANTRCV_MAINFUNCTION_ID, CANTRCV_E_UNINIT);

#if (CANTRCV_WAKE_UP_SUPPORTED == STD_ON)
    /* @req SWS_CanTrcv_00091 */
    const CanTrcv_SpiSequenceConfigType *spiSeq;
    boolean wakeupFlag;
    CanTrcv_TrcvWakeupReasonType reason;
    CanTrcv_TrcvModeType mode;

    /* @req SWS_CanTrcv_00112 */
    for(uint8 trcv = 0; trcv < CANTRCV_CHANNEL_COUNT; trcv++)
    {
        if ((CanTrcv_ConfigPtr->CanTrcvChannel[trcv].CanTrcvWakeupByBus)
                && (CanTrcv_ConfigPtr->CanTrcvChannel[trcv].CanTrcvEnableStatus))
        {
            mode = CanTrcv_Internal.CanTrcv_RuntimeData[trcv].CanTrcv_TrcvCurMode;

            if ((CANTRCV_TRCVMODE_STANDBY == mode) || (CANTRCV_TRCVMODE_SLEEP == mode) ||
                    (CANTRCV_WUMODE_ENABLE == CanTrcv_Internal.CanTrcv_RuntimeData[trcv].CanTrcv_CurWakeupMode)) {
                wakeupFlag = FALSE;
                reason = CANTRCV_WU_NOT_SUPPORTED;
                spiSeq = CanTrcv_ConfigPtr->CanTrcvChannel[trcv].CanTrcvSpiSequenceConfig;

                /* Check for wake up and report EcuM */
                /* The following requirements are derived from EcuM fixed sequence dig 9.2.3 in ASR 4.0.3 */
                (void)CanTrcv_Hw_ReadWakeupEventStatus(&reason,&wakeupFlag,spiSeq);

                if (wakeupFlag){
                    if ((CANTRCV_WU_BY_BUS == reason) &&
                            (CanTrcv_ConfigPtr->CanTrcvChannel[trcv].CanTrcvWakeupSourceRef != CANTRCV_INVALID_WAKEUP_SOURCE)) {
                        EcuM_CheckWakeup(CanTrcv_ConfigPtr->CanTrcvChannel[trcv].CanTrcvWakeupSourceRef);
                    }
                    else if ((CANTRCV_WU_BY_SYSERR == reason) && (CanTrcv_ConfigPtr->CanTrcvChannel[trcv].CanTrcvSyserrWakeupSourceRef != CANTRCV_INVALID_WAKEUP_SOURCE)) {
                       EcuM_CheckWakeup(CanTrcv_ConfigPtr->CanTrcvChannel[trcv].CanTrcvSyserrWakeupSourceRef);
                    }
                    else if ((CANTRCV_WU_BY_PIN == reason)|| (CANTRCV_WU_POWER_ON == reason)) {
                           EcuM_CheckWakeup(CanTrcv_ConfigPtr->CanTrcvChannel[trcv].CanTrcvPorWakeupSourceRef);
                    } else {
                       /* Do nothing */
                    }
                }
            }
        }
    } /* end of for loop */
#endif
}

/** Reads the transceiver diagnostic status periodically and sets product/development accordingly. */
/* @req SWS_CanTrcv_00218 */
void CanTrcv_MainFunctionDiagnostics(void)
{
      /* !req SWS_CanTrcv_00204 */
      /* !req SWS_CanTrcv_00205 */
      /* !req SWS_CanTrcv_00206 */
      /* !req SWS_CanTrcv_00207 */
}

