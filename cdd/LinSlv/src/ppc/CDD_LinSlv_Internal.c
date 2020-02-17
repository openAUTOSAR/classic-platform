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

/** @tagSettings DEFAULT_ARCHITECTURE=MPC5645S */
/** @reqSettings DEFAULT_SPECIFICATION_REVISION=4.1.2 */

/* ----------------------------[includes]------------------------------------*/
#include "CDD_LinSlv.h"
#include "CDD_LinSlv_Internal.h"
#include "LinFlex_Common.h"
#include "isr.h"
#include "mpc55xx.h"
#if defined(USE_DET)
#include "Det.h"
#endif
#if defined(USE_DIO)
#include "Dio.h"
#endif
#if defined(USE_PORT)
#include "Port.h"
#endif

/* ----------------------------[private define]------------------------------*/
#define UNASSING_PID    (uint8)0x00u
#define MASK_IFCR_PID   (uint32)0x3FuL
#define MASK_ID         (uint8)0x3Fu

#define LINS_HEADER_COMP (uint32)0x7u

#define HEADER_TIMEOUT      0x2CuL
/* ----------------------------[private macro]-------------------------------*/
/* Development error macros. */
/* @req CDD_LinSlv_00009 */
#if ( LINSLV_DEV_ERROR_DETECT == STD_ON )
#define VALIDATE(_exp,_api,_err ) \
        if( !(_exp) ) { \
          (void)Det_ReportError(CDD_LINSLV_MODULE_ID,0,_api,_err); \
          return; \
        }

#define VALIDATE_W_RV(_exp,_api,_err,_rv ) \
        if( !(_exp) ) { \
          (void)Det_ReportError(CDD_LINSLV_MODULE_ID,0,_api,_err); \
          return (_rv); \
        }
#else
#define VALIDATE(_exp,_api,_err )
#define VALIDATE_W_RV(_exp,_api,_err,_rv )
#endif

#define LINFLEX(exp) (volatile struct LINFLEX_tag *)(0xFFE40000uL + (0x4000uL * exp))
#define LINFLEXSLV(exp) (volatile struct LINFLEX_MS_tag *)(0xFFE40000uL + (0x4000uL * exp))
/* --------------------------------------------------------------------------*/
static uint32 chnIsrSetup[LINSLV_CHANNEL_CNT];


/**
 *
 * @param LinSlv_Cfg
 * @param Channel
 * @param frameCfgPtr
 * @param buffer
 * @return
 */
Std_ReturnType LinSlv_Hw_Rx(const LinSlv_ContainerType* LinSlv_Cfg, uint8 Channel, const LinIf_FrameType **frameCfgPtr, uint8 *buffer) {
    volatile struct LINFLEX_tag * LINFLEXHw;
    Std_ReturnType retval;
    uint32 filterMatchIdx;
    uint32 chnId;

    VALIDATE_W_RV( (NULL != LinSlv_Cfg)  , LINSLV_INTERNAL_ISR_ID, LINSLV_E_INVALID_POINTER, E_NOT_OK);
    VALIDATE_W_RV( (NULL != frameCfgPtr) , LINSLV_INTERNAL_ISR_ID, LINSLV_E_INVALID_POINTER, E_NOT_OK);
    VALIDATE_W_RV( (NULL != buffer)      , LINSLV_INTERNAL_ISR_ID, LINSLV_E_INVALID_POINTER, E_NOT_OK);

    chnId = LinSlv_Cfg->LinSlvCfgPtr->LinSlvChannelConfig[Channel].LinSlvChnConfig.LinChannelId;

    /*lint -e{923,9033} Hardware access */
    LINFLEXHw = LINFLEX(chnId);

    filterMatchIdx = LINFLEXHw->IFMI.R;

    if (filterMatchIdx > 0) {
        retval = LinFlex_Common_WriteHwRegToDataBuf(buffer, LINFLEXHw);
        VALIDATE_W_RV( (retval != E_NOT_OK) , LINSLV_INTERNAL_ISR_ID, LINSLV_E_INVALID_POINTER, E_NOT_OK);

        *frameCfgPtr = LinSlv_Cfg->LinSlvCfgPtr->LinSlvChannelFrameConfig[Channel].LinSlvChnFrameCfg[filterMatchIdx - 1uL];

        retval = E_OK;
    } else {
        LINFLEXHw->LINCR2.B.DDRQ =1; /* Discard any data that caused the interrupt */
        retval = E_NOT_OK;
    }

    /* Clear flags */
    LINFLEXHw->LINSR.B.HRF = 1;
    LINFLEXHw->LINSR.B.DRF = 1;

    return retval;
}

/**
 *
 * @param LinSlv_Cfg
 * @param Channel
 * @param frameCfgPtr
 * @return
 */
Std_ReturnType LinSlv_Hw_Tx_GetFramePtr(const LinSlv_ContainerType* LinSlv_Cfg, uint8 Channel, const LinIf_FrameType **frameCfgPtr) {
    volatile struct LINFLEX_tag * LINFLEXHw;
    Std_ReturnType retval;
    uint32 filterMatchIdx;
    uint32 chnId;

    VALIDATE_W_RV( (NULL != LinSlv_Cfg)  , LINSLV_INTERNAL_ISR_ID, LINSLV_E_INVALID_POINTER, E_NOT_OK);
    VALIDATE_W_RV( (NULL != frameCfgPtr) , LINSLV_INTERNAL_ISR_ID, LINSLV_E_INVALID_POINTER, E_NOT_OK);

    chnId = LinSlv_Cfg->LinSlvCfgPtr->LinSlvChannelConfig[Channel].LinSlvChnConfig.LinChannelId;

    /*lint -e{923,9033} Hardware access */
    LINFLEXHw = LINFLEX(chnId);

    filterMatchIdx = LINFLEXHw->IFMI.R;

    if (filterMatchIdx > 0) {
        *frameCfgPtr = LinSlv_Cfg->LinSlvCfgPtr->LinSlvChannelFrameConfig[Channel].LinSlvChnFrameCfg[filterMatchIdx - 1uL];

        retval = E_OK;
    } else {
        LINFLEXHw->LINCR2.B.DTRQ =1; /* Discard the transmission request */
        retval = E_NOT_OK;
    }

    return retval;
}

/**
 * Handles the error interrupts and returns E_NOT_OK if the Response error bit is to be set.
 * @param LinSlv_Cfg
 * @param Channel
 * @return
 */
Std_ReturnType LinSlv_Hw_Err(const LinSlv_ContainerType* LinSlv_Cfg, uint8 Channel) {
    volatile struct LINFLEX_tag * LINFLEXHw;
    Std_ReturnType retval = E_OK;
    boolean headerError;
    boolean headerReceived = FALSE;
    uint32 linsrLins;
    uint32 cntOC1;
    uint32 cntOC2;
    uint32 csrCnt;
    boolean hrf;
    uint32 chnId;

    VALIDATE_W_RV( (NULL != LinSlv_Cfg)  , LINSLV_INTERNAL_ISR_ID, LINSLV_E_INVALID_POINTER, E_NOT_OK);

    chnId = LinSlv_Cfg->LinSlvCfgPtr->LinSlvChannelConfig[Channel].LinSlvChnConfig.LinChannelId;

    /*lint -e{923,9033} Hardware access */
    LINFLEXHw = LINFLEX(chnId);

    cntOC1    = (uint32)LINFLEXHw->LINOCR.B.OC1;
    cntOC2    = (uint32)LINFLEXHw->LINOCR.B.OC2;
    csrCnt    = (uint32)LINFLEXHw->LINTCSR.B.CNT;
    hrf       = (1uL == LINFLEXHw->LINSR.B.HRF) ? TRUE : FALSE;
    linsrLins = (uint32)LINFLEXHw->LINSR.B.LINS;

    /*lint -e{9007} No side effects, must write 1 to clear any bit in this register*/
    headerError = ((LINFLEXHw->LINESR.B.SFEF > 0uL) || (LINFLEXHw->LINESR.B.BDEF  > 0uL) || \
                    (LINFLEXHw->LINESR.B.IDPEF> 0uL) ) ? TRUE : FALSE;

    /*
     * Method used to detect if the header has been received or not;
     * The LINS register have automatic state transitions, which becomes an issue when the
     * stop bit is corrupted (it automatically goes to "break" state no matter what the previous
     * state was. Therefore we also look at the OC1 time register.
     * Although not written in any documentation, it appears that the base CNT starts at 0 at
     * new reception, therefore we can compare the OC1 value against the header timeout value.
     * If it is equal it means that we are in the state of receiving a header.
     */
    if (((csrCnt < cntOC2) && (cntOC1 != HEADER_TIMEOUT) &&
            (linsrLins > LINS_HEADER_COMP) && !headerError) ||
            hrf)
    {
        headerReceived = TRUE;
    }

    /* Time out after the header was received */
    if ((LINFLEXHw->LINESR.B.OCF == 1uL)) {
        /* Must be set to init mode to clear this flag */
        LINFLEXHw->LINCR1.B.INIT = 1;
        LINFLEXHw->LINCR1.B.INIT = 0;
        if (headerReceived) {
            retval = E_NOT_OK;
        }
    }
    /* Bit error occurred during response field transmission */
    else if (LINFLEXHw->LINESR.B.BEF == 1uL){
        retval = E_NOT_OK;
    }
    /* Checksum error */
    else if (LINFLEXHw->LINESR.B.CEF == 1uL) {
        retval = E_NOT_OK;
    }
    /* Invalid stop bit occurred during reception of response */
    /* Extra check with OC1 counter to check if it is receiving header or not */
    else if ((1uL == LINFLEXHw->LINESR.B.FEF) && (cntOC1 != HEADER_TIMEOUT)) {
        retval = E_NOT_OK;
    }
    /* No error occurred that shall set the Response_error signal. */
    else {
        retval = E_OK;
    }

    /* Clear flags  */
    LINFLEXHw->LINESR.R=0xffffffffuL;
    LINFLEXHw->LINSR.R = 0xffffffffuL;

    LINFLEXHw->LINCR2.B.ABRQ =1;
    LINFLEXHw->LINCR2.B.DDRQ = 1;

    return retval;
}


#ifdef LINSLV_CHANNEL_0_USED
ISR(LinSlvInterruptRxA); /* Prototype definition */
ISR(LinSlvInterruptRxA){LinSlv_Internal_Rx_Isr(0);}
ISR(LinSlvInterruptTxA); /* Prototype definition */
ISR(LinSlvInterruptTxA){LinSlv_Internal_Tx_Isr(0);}
ISR(LinSlvInterruptErrA); /* Prototype definition */
ISR(LinSlvInterruptErrA){LinSlv_Internal_Err_Isr(0);}
#endif

#ifdef LINSLV_CHANNEL_1_USED
ISR(LinSlvInterruptRxB); /* Prototype definition */
ISR(LinSlvInterruptRxB){LinSlv_Internal_Rx_Isr(1);}
ISR(LinSlvInterruptTxB); /* Prototype definition */
ISR(LinSlvInterruptTxB){LinSlv_Internal_Tx_Isr(1);}
ISR(LinSlvInterruptErrB); /* Prototype definition */
ISR(LinSlvInterruptErrB){LinSlv_Internal_Err_Isr(1);}
#endif

#ifdef LINSLV_CHANNEL_2_USED
ISR(LinSlvInterruptRxC); /* Prototype definition */
ISR(LinSlvInterruptRxC){LinSlv_Internal_Rx_Isr(2);}
ISR(LinSlvInterruptTxC); /* Prototype definition */
ISR(LinSlvInterruptTxC){LinSlv_Internal_Tx_Isr(2);}
ISR(LinSlvInterruptErrC); /* Prototype definition */
ISR(LinSlvInterruptErrC){LinSlv_Internal_Err_Isr(2);}
#endif

#ifdef LINSLV_CHANNEL_3_USED
ISR(LinSlvInterruptRxD); /* Prototype definition */
ISR(LinSlvInterruptRxD){LinSlv_Internal_Rx_Isr(3);}
ISR(LinSlvInterruptTxD); /* Prototype definition */
ISR(LinSlvInterruptTxD){LinSlv_Internal_Tx_Isr(3);}
ISR(LinSlvInterruptErrD); /* Prototype definition */
ISR(LinSlvInterruptErrD){LinSlv_Internal_Err_Isr(3);}
#endif

#ifdef LINSLV_CHANNEL_4_USED
ISR(LinSlvInterruptRxE); /* Prototype definition */
ISR(LinSlvInterruptRxE){LinSlv_Internal_Rx_Isr(4);}
ISR(LinSlvInterruptTxE); /* Prototype definition */
ISR(LinSlvInterruptTxE){LinSlv_Internal_Tx_Isr(4);}
ISR(LinSlvInterruptErrE); /* Prototype definition */
ISR(LinSlvInterruptErrE){LinSlv_Internal_Err_Isr(4);}
#endif

#ifdef LINSLV_CHANNEL_5_USED
ISR(LinSlvInterruptRxF); /* Prototype definition */
ISR(LinSlvInterruptRxF){LinSlv_Internal_Rx_Isr(5);}
ISR(LinSlvInterruptTxF); /* Prototype definition */
ISR(LinSlvInterruptTxF){LinSlv_Internal_Tx_Isr(5);}
ISR(LinSlvInterruptErrF); /* Prototype definition */
ISR(LinSlvInterruptErrF){LinSlv_Internal_Err_Isr(5);}
#endif

#ifdef LINSLV_CHANNEL_6_USED
ISR(LinSlvInterruptRxG); /* Prototype definition */
ISR(LinSlvInterruptRxG){LinSlv_Internal_Rx_Isr(6);}
ISR(LinSlvInterruptTxG); /* Prototype definition */
ISR(LinSlvInterruptTxG){LinSlv_Internal_Tx_Isr(6);}
ISR(LinSlvInterruptErrG); /* Prototype definition */
ISR(LinSlvInterruptErrG){LinSlv_Internal_Err_Isr(6);}
#endif

#ifdef LINSLV_CHANNEL_7_USED
ISR(LinSlvInterruptRxH); /* Prototype definition */
ISR(LinSlvInterruptRxH){LinSlv_Internal_Rx_Isr(7);}
ISR(LinSlvInterruptTxH); /* Prototype definition */
ISR(LinSlvInterruptTxH){LinSlv_Internal_Tx_Isr(7);}
ISR(LinSlvInterruptErrH); /* Prototype definition */
ISR(LinSlvInterruptErrH){LinSlv_Internal_Err_Isr(7);}
#endif

#ifdef LINSLV_CHANNEL_14_USED
ISR(LinSlvInterruptRx14); /* Prototype definition */
ISR(LinSlvInterruptRx14){LinSlv_Internal_Rx_Isr(14);}
ISR(LinSlvInterruptTx14); /* Prototype definition */
ISR(LinSlvInterruptTx14){LinSlv_Internal_Tx_Isr(14);}
ISR(LinSlvInterruptErr14); /* Prototype definition */
ISR(LinSlvInterruptErr14){LinSlv_Internal_Err_Isr(14);}
#endif

#ifdef LINSLV_CHANNEL_15_USED
ISR(LinSlvInterruptRx15); /* Prototype definition */
ISR(LinSlvInterruptRx15){LinSlv_Internal_Rx_Isr(15);}
ISR(LinSlvInterruptTx15); /* Prototype definition */
ISR(LinSlvInterruptTx15){LinSlv_Internal_Tx_Isr(15);}
ISR(LinSlvInterruptErr15); /* Prototype definition */
ISR(LinSlvInterruptErr15){LinSlv_Internal_Err_Isr(15);}
#endif

#ifdef LINSLV_CHANNEL_16_USED
ISR(LinSlvInterruptRx16); /* Prototype definition */
ISR(LinSlvInterruptRx16){LinSlv_Internal_Rx_Isr(16);}
ISR(LinSlvInterruptTx16); /* Prototype definition */
ISR(LinSlvInterruptTx16){LinSlv_Internal_Tx_Isr(16);}
ISR(LinSlvInterruptErr16); /* Prototype definition */
ISR(LinSlvInterruptErr16){LinSlv_Internal_Err_Isr(16);}
#endif

/**
 * DeInitialize the peripheral and sets channel status
 * @param LinSlv_Cfg
 */
void LinSlv_Internal_Deinit(LinSlv_ContainerType* LinSlv_Cfg) {
    uint8 channel;

    VALIDATE( (NULL != LinSlv_Cfg), LINSLV_DEINIT_SERVICE_ID, LINSLV_E_PARAM_POINTER );

    for (channel = 0; channel < LINSLV_CHANNEL_CNT; channel++) {
        LinFlex_Common_DeInit(&LinSlv_Cfg->LinSlvCfgPtr->LinSlvChannelConfig[channel].LinSlvChnConfig);

        LinSlv_Cfg->LinSlvChannelStatus[channel]=LIN_CH_UNINIT;
        LinSlv_Cfg->LinSlvChannelOrderedStatus[channel]=LIN_CH_UNINIT;
    }
}

/**
 * Initializes the module
 * @param LinSlv_Cfg
 * @return
 */
Std_ReturnType LinSlv_Internal_Init(LinSlv_ContainerType* LinSlv_Cfg) {
    const Lin_ChannelConfigType *chnCfg;
    Std_ReturnType retval;
    uint8 channel;

    VALIDATE_W_RV( ( NULL !=  LinSlv_Cfg), LINSLV_INIT_SERVICE_ID, LINSLV_E_INVALID_POINTER, E_NOT_OK);

    #ifdef LINSLV_CHANNEL_0_USED
    ISR_INSTALL_ISR2("LinSlvIsrRxA", LinSlvInterruptRxA, (int16_t)(LINFLEX_0_RXI), 3, 0);
    ISR_INSTALL_ISR2("LinSlvIsrTxA", LinSlvInterruptTxA, (int16_t)(LINFLEX_0_TXI), 3, 0);
    ISR_INSTALL_ISR2("LinSlvIsrErrA", LinSlvInterruptErrA, (int16_t)(LINFLEX_0_ERR), 3, 0);
    #endif
    #ifdef LINSLV_CHANNEL_1_USED
    ISR_INSTALL_ISR2("LinSlvIsrRxB", LinSlvInterruptRxB, (int16_t)(LINFLEX_1_RXI), 3, 0);
    ISR_INSTALL_ISR2("LinSlvIsrTxB", LinSlvInterruptTxB, (int16_t)(LINFLEX_1_TXI), 3, 0);
    ISR_INSTALL_ISR2("LinSlvIsrErrB", LinSlvInterruptErrB, (int16_t)(LINFLEX_1_ERR), 3, 0);
    #endif
    #ifdef LINSLV_CHANNEL_2_USED
    ISR_INSTALL_ISR2("LinSlvIsrRxC", LinSlvInterruptRxC, (int16_t)(LINFLEX_2_RXI), 3, 0);
    ISR_INSTALL_ISR2("LinSlvIsrTxC", LinSlvInterruptTxC, (int16_t)(LINFLEX_2_TXI), 3, 0);
    ISR_INSTALL_ISR2("LinSlvIsrErrC", LinSlvInterruptErrC, (int16_t)(LINFLEX_2_ERR), 3, 0);
    #endif
    #ifdef LINSLV_CHANNEL_3_USED
    ISR_INSTALL_ISR2("LinSlvIsrRxD", LinSlvInterruptRxD, (int16_t)(LINFLEX_3_RXI), 3, 0);
    ISR_INSTALL_ISR2("LinSlvIsrTxD", LinSlvInterruptTxD, (int16_t)(LINFLEX_3_TXI), 3, 0);
    ISR_INSTALL_ISR2("LinSlvIsrErrD", LinSlvInterruptErrD, (int16_t)(LINFLEX_3_ERR), 3, 0);
    #endif
    #ifdef LINSLV_CHANNEL_4_USED
    ISR_INSTALL_ISR2("LinSlvIsrRxE", LinSlvInterruptRxE, (int16_t)(LINFLEX_4_RXI), 3, 0);
    ISR_INSTALL_ISR2("LinSlvIsrTxE", LinSlvInterruptTxE, (int16_t)(LINFLEX_4_TXI), 3, 0);
    ISR_INSTALL_ISR2("LinSlvIsrErrE", LinSlvInterruptErrE, (int16_t)(LINFLEX_4_ERR), 3, 0);
    #endif
    #ifdef LINSLV_CHANNEL_5_USED
    ISR_INSTALL_ISR2("LinSlvIsrRxF", LinSlvInterruptRxF, (int16_t)(LINFLEX_5_RXI), 3, 0);
    ISR_INSTALL_ISR2("LinSlvIsrTxF", LinSlvInterruptTxF, (int16_t)(LINFLEX_5_TXI), 3, 0);
    ISR_INSTALL_ISR2("LinSlvIsrErrF", LinSlvInterruptErrF, (int16_t)(LINFLEX_5_ERR), 3, 0);
    #endif
    #ifdef LINSLV_CHANNEL_6_USED
    ISR_INSTALL_ISR2("LinSlvIsrRxG", LinSlvInterruptRxG, (int16_t)(LINFLEX_6_RXI), 3, 0);
    ISR_INSTALL_ISR2("LinSlvIsrTxG", LinSlvInterruptTxG, (int16_t)(LINFLEX_6_TXI), 3, 0);
    ISR_INSTALL_ISR2("LinSlvIsrErrG", LinSlvInterruptErrG, (int16_t)(LINFLEX_6_ERR), 3, 0);
    #endif
    #ifdef LINSLV_CHANNEL_7_USED
    ISR_INSTALL_ISR2("LinSlvIsrRxH", LinSlvInterruptRxH, (int16_t)(LINFLEX_7_RXI), 3, 0);
    ISR_INSTALL_ISR2("LinSlvIsrTxH", LinSlvInterruptTxH, (int16_t)(LINFLEX_7_TXI), 3, 0);
    ISR_INSTALL_ISR2("LinSlvIsrErrH", LinSlvInterruptErrH, (int16_t)(LINFLEX_7_ERR), 3, 0);
    #endif
    #ifdef LINSLV_CHANNEL_14_USED
    ISR_INSTALL_ISR2("LinSlvIsrRx14", LinSlvInterruptRx14, (int16_t)(LINFLEX_14_RXI), 3, 0);
    ISR_INSTALL_ISR2("LinSlvIsrTx14", LinSlvInterruptTx14, (int16_t)(LINFLEX_14_TXI), 3, 0);
    ISR_INSTALL_ISR2("LinSlvIsrErr14", LinSlvInterruptErr14, (int16_t)(LINFLEX_14_ERR), 3, 0);
    #endif
    #ifdef LINSLV_CHANNEL_15_USED
    ISR_INSTALL_ISR2("LinSlvIsrRx15", LinSlvInterruptRx15, (int16_t)(LINFLEX_15_RXI), 3, 0);
    ISR_INSTALL_ISR2("LinSlvIsrTx15", LinSlvInterruptTx15, (int16_t)(LINFLEX_15_TXI), 3, 0);
    ISR_INSTALL_ISR2("LinSlvIsrErr15", LinSlvInterruptErr15, (int16_t)(LINFLEX_15_ERR), 3, 0);
    #endif
    #ifdef LINSLV_CHANNEL_16_USED
    ISR_INSTALL_ISR2("LinSlvIsrRx16", LinSlvInterruptRx16, (int16_t)(LINFLEX_16_RXI), 3, 0);
    ISR_INSTALL_ISR2("LinSlvIsrTx16", LinSlvInterruptTx16, (int16_t)(LINFLEX_16_TXI), 3, 0);
    ISR_INSTALL_ISR2("LinSlvIsrErr16", LinSlvInterruptErr16, (int16_t)(LINFLEX_16_ERR), 3, 0);
    #endif

    for (channel = 0; channel < LINSLV_CHANNEL_CNT; channel++) {
        chnCfg = &LinSlv_Cfg->LinSlvCfgPtr->LinSlvChannelConfig[channel].LinSlvChnConfig;

        retval = LinFlex_Common_Init(chnCfg, LIN_SLAVE);
        if (E_NOT_OK == retval) {
            return E_NOT_OK;
        }

        retval = LinFlex_Common_SetupSlvFilter(chnCfg, &LinSlv_Cfg->LinSlvCfgPtr->LinSlvChannelFrameConfig[channel]);
        if (E_NOT_OK == retval) {
            return E_NOT_OK;
        }

        if (LINIF_CHANNEL_SLEEP == LinSlv_Cfg->LinSlvCfgPtr->LinSlvChannelConfig[channel].LinSlvChannelInitState) {
            LinFlex_Common_GoToSleepInternal(chnCfg);

            LinSlv_Cfg->LinSlvChannelStatus[channel]=LIN_CH_SLEEP;
            LinSlv_Cfg->LinSlvChannelOrderedStatus[channel]=LIN_CH_SLEEP;
        } else {
            LinSlv_Cfg->LinSlvChannelStatus[channel]=LIN_OPERATIONAL;
            LinSlv_Cfg->LinSlvChannelOrderedStatus[channel]=LIN_OPERATIONAL;
        }
    }

    return E_OK;
}

/**
 * Internal functionality to enter power save mode.
 * @param LinSlv_Cfg
 * @param Channel
 */
void LinSlv_Internal_GoToSleepInternal(const LinSlv_ContainerType* LinSlv_Cfg, uint8 Channel) {
    LinFlex_Common_GoToSleepInternal(&LinSlv_Cfg->LinSlvCfgPtr->LinSlvChannelConfig[Channel].LinSlvChnConfig);
}

/**
 * Set the control to operational mode
 * @param LinSlv_Cfg
 * @param Channel
 */
void LinSlv_Internal_WakeupInternal(const LinSlv_ContainerType* LinSlv_Cfg, uint8 Channel) {
    LinFlex_Common_WakeupInternal(&LinSlv_Cfg->LinSlvCfgPtr->LinSlvChannelConfig[Channel].LinSlvChnConfig);
}

/**
 * Send wake-up on the bus
 * @param LinSlv_Cfg
 * @param Channel
 */
Std_ReturnType LinSlv_Internal_Wakeup(const LinSlv_ContainerType* LinSlv_Cfg, uint8 Channel) {
    return LinFlex_Common_Wakeup(&LinSlv_Cfg->LinSlvCfgPtr->LinSlvChannelConfig[Channel].LinSlvChnConfig);
}

/**
 * Check if the wake-up is set.
 * @param LinSlv_Cfg
 * @param Channel
 * @return
 */
boolean LinSlv_Internal_IsWakeUpSet(const LinSlv_ContainerType* LinSlv_Cfg, uint8 Channel) {
    return LinFlex_Common_IsWakeUpSet(&LinSlv_Cfg->LinSlvCfgPtr->LinSlvChannelConfig[Channel].LinSlvChnConfig);
}

/**
 * Read out the peripheral status of the unit
 * @param LinSlv_Cfg
 * @param Channel
 * @return
 */
Lin_StatusType LinSlv_Internal_GetStatus(const LinSlv_ContainerType* LinSlv_Cfg, uint8 Channel) {
    return LinFlex_Common_GetPeripheralStatus(&LinSlv_Cfg->LinSlvCfgPtr->LinSlvChannelConfig[Channel].LinSlvChnConfig);
}

/**
 * Set response data and request transmission of the response.
 * @param LinSlv_Cfg
 * @param Channel
 * @param Length
 * @param Buffer
 */
void LinSlv_Internal_SetResponse(const LinSlv_ContainerType* LinSlv_Cfg, uint8 Channel, uint8 Length, const uint8 *Buffer) {
    volatile struct LINFLEX_tag * LINFLEXHw;
    Std_ReturnType retval;
    uint32 chnId;

    VALIDATE( (NULL != LinSlv_Cfg), LINSLV_INTERNAL_ISR_ID, LINSLV_E_INVALID_POINTER);
    VALIDATE( (NULL != Buffer)    , LINSLV_INTERNAL_ISR_ID, LINSLV_E_INVALID_POINTER);

    chnId = LinSlv_Cfg->LinSlvCfgPtr->LinSlvChannelConfig[Channel].LinSlvChnConfig.LinChannelId;
    /*lint -e{923,9033} Hardware access */
    LINFLEXHw = LINFLEX(chnId);

    retval = LinFlex_Common_WriteDataBufToHwReg(Length, Buffer, LINFLEXHw);
    VALIDATE( (E_OK == retval)    , LINSLV_INTERNAL_ISR_ID, LINSLV_E_INVALID_CONFIG);

    // Trigger the data transmission
    LINFLEXHw->LINCR2.B.DTRQ = 1;
}

/**
 *
 * @param LinSlv_Cfg
 * @param Channel
 * @return
 */
LinSlv_TxIsrSourceType LinSlv_Hw_Tx_GetIsrSource(const LinSlv_ContainerType* LinSlv_Cfg, uint8 Channel) {
    const volatile struct LINFLEX_tag * LINFLEXHw;
    LinSlv_TxIsrSourceType retval = LINSLV_OTHER;
    uint32 chnId;

    VALIDATE_W_RV( ( NULL !=  LinSlv_Cfg), LINSLV_INTERNAL_ISR_ID, LINSLV_E_INVALID_POINTER, LINSLV_OTHER);

    chnId = LinSlv_Cfg->LinSlvCfgPtr->LinSlvChannelConfig[Channel].LinSlvChnConfig.LinChannelId;
    /*lint -e{923,9033} Hardware access */
    LINFLEXHw = LINFLEX(chnId);

    if (1uL == LINFLEXHw->LINSR.B.HRF) {
        retval = LINSLV_HEADER;
    } else if (1uL == LINFLEXHw->LINSR.B.DTF) {
        retval = LINSLV_TRANSMIT_DONE;
    } else {
        retval = LINSLV_OTHER;
    }

    return retval;
}

/**
 * Clear the flags that caused the tx isr
 * @param LinSlv_Cfg
 * @param Channel
 */
void LinSlv_Hw_ClearTxIsrFlags(const LinSlv_ContainerType* LinSlv_Cfg, uint8 Channel) {
    volatile struct LINFLEX_tag * LINFLEXHw;
    uint32 chnId;

    VALIDATE( ( NULL !=  LinSlv_Cfg), LINSLV_INTERNAL_ISR_ID, LINSLV_E_INVALID_POINTER);

    chnId = LinSlv_Cfg->LinSlvCfgPtr->LinSlvChannelConfig[Channel].LinSlvChnConfig.LinChannelId;

    /*lint -e{923,9033} Hardware access */
    LINFLEXHw = LINFLEX(chnId);

    LINFLEXHw->LINSR.B.HRF = 1;
    LINFLEXHw->LINSR.B.DTF = 1;
}




/**
 * Store the ISR setup of the channel and disables them.
 * @param LinSlv_Cfg
 * @param Channel
 */
void LinSlv_Internal_StoreAndDisableIsr(const LinSlv_ContainerType* LinSlv_Cfg, uint8 Channel) {
    volatile struct LINFLEX_tag * LINFLEXHw;
    uint32 chnId;

    VALIDATE( ( NULL !=  LinSlv_Cfg), LINSLV_INTERNAL_ISR_ID, LINSLV_E_INVALID_POINTER);

    chnId = LinSlv_Cfg->LinSlvCfgPtr->LinSlvChannelConfig[Channel].LinSlvChnConfig.LinChannelId;

    /*lint -e{923,9033} Hardware access */
    LINFLEXHw = LINFLEX(chnId);

    chnIsrSetup[Channel] = LINFLEXHw->LINIER.R;
    LINFLEXHw->LINIER.R = 0;
}

/**
 * Restores the ISR setup of the channel and enables them.
 * @param LinSlv_Cfg
 * @param Channel
 */
void LinSlv_Internal_RestoreAndEnableIsr(const LinSlv_ContainerType* LinSlv_Cfg, uint8 Channel) {
    volatile struct LINFLEX_tag * LINFLEXHw;
    uint32 chnId;

    VALIDATE( ( NULL !=  LinSlv_Cfg), LINSLV_INTERNAL_ISR_ID, LINSLV_E_INVALID_POINTER);

    chnId = LinSlv_Cfg->LinSlvCfgPtr->LinSlvChannelConfig[Channel].LinSlvChnConfig.LinChannelId;

    /*lint -e{923,9033} Hardware access */
    LINFLEXHw = LINFLEX(chnId);

    LINFLEXHw->LINIER.R = chnIsrSetup[Channel];
}

/**
 * Update the filter rules for a frame.
 * @param LinSlv_Cfg
 * @param Channel
 * @param frameIndex
 * @param newPid
 */
void LinSlv_Internal_UpdatePid(const LinSlv_ContainerType* LinSlv_Cfg, uint8 Channel, uint8 frameIndex, uint8 newPid) {
    volatile struct LINFLEX_MS_tag * LINFLEXHw;
    uint32 chnId;
    uint32 ifcr;
    uint8 id;

    VALIDATE( ( NULL !=  LinSlv_Cfg), LINSLV_INTERNAL_ISR_ID, LINSLV_E_INVALID_POINTER);

    chnId = LinSlv_Cfg->LinSlvCfgPtr->LinSlvChannelConfig[Channel].LinSlvChnConfig.LinChannelId;

    /*lint -e{923,9033} Hardware access */
    LINFLEXHw = LINFLEXSLV(chnId);

    LINFLEXHw->LINCR1.B.INIT = 1; /* Go to init mode */

    //Mask out parity bits
    id = newPid & MASK_ID;

    //Store current filter rules
    ifcr = LINFLEXHw->IFCR[frameIndex].R;
    ifcr &= ~(MASK_IFCR_PID);

    ifcr |= (uint32)id;

    LINFLEXHw->IFCR[frameIndex].R = ifcr;

    if (UNASSING_PID == newPid) {
        // Disable the filter
        LINFLEXHw->IFER.R &= ~(1uL << frameIndex);
    } else {
        LINFLEXHw->IFER.R |= (1uL << frameIndex);
    }

    LINFLEXHw->LINCR1.B.INIT = 0;
}

/**
 *
 * @param LinSlv_Cfg
 * @param Channel
 * @return
 */
boolean LinSlv_Internal_IsWakeUpDoneTransmitting(const LinSlv_ContainerType* LinSlv_Cfg, uint8 Channel) {
    const volatile struct LINFLEX_MS_tag * LINFLEXHw;
    boolean retval;
    uint32 chnId;

    VALIDATE_W_RV( ( NULL !=  LinSlv_Cfg), LINSLV_MAINFUNCTION_SERVICE_ID, LINSLV_E_INVALID_POINTER, FALSE);

    chnId = LinSlv_Cfg->LinSlvCfgPtr->LinSlvChannelConfig[Channel].LinSlvChnConfig.LinChannelId;

    /*lint -e{923,9033} Hardware access */
    LINFLEXHw = LINFLEXSLV(chnId);

    retval = (0uL == LINFLEXHw->LINCR2.B.WURQ) ? TRUE : FALSE;

    return retval;
}

#if (LINSLV_TRCV_SUPPORT == STD_ON)
/**
 * Set the transceiver mode.
 * @param LinSlv_Cfg
 * @param Channel
 * @param TransceiverMode
 * @return
 */
Std_ReturnType LinSlv_Internal_SetTrcvMode(const LinSlv_ContainerType* LinSlv_Cfg,
        uint8 Channel,
        LinTrcv_TrcvModeType TransceiverMode)
{
    const LinSlv_TrcvType * trcvCfg;
    uint16 portTxSymbolicRef;
    uint32 dioTxSymbolicRef;
    uint32 dioEnableSymbolicRef;

    VALIDATE_W_RV( ( NULL !=  LinSlv_Cfg), LINSLV_SETTRCVMODE_SERVICE_ID, LINSLV_E_INVALID_POINTER, E_NOT_OK);

    trcvCfg = &LinSlv_Cfg->LinSlvCfgPtr->LinSlvChnTrcvConfig[Channel];

    portTxSymbolicRef = trcvCfg->LinSlvTrcvDioAccess.LinSlvTrcvTxPortSource;
    dioTxSymbolicRef = trcvCfg->LinSlvTrcvDioAccess.LinSlvTrcvTxDioSource;
    dioEnableSymbolicRef = trcvCfg->LinSlvTrcvDioAccess.LinSlvTrcvEnableSource;

    if (LINTRCV_TRCV_MODE_SLEEP == TransceiverMode) {
        // To set the transceiver into sleep mode the TX pin must be set low
        // for 20 us. The reliable method of doing this is to temporarily reconfigure
        // the pin to be a digital output set to low.

        uint16 txPcrReg = PortConfigData.padConfig[portTxSymbolicRef].config;
        uint16 txPcrForceDio = (~(PORT_BIT4 | PORT_BIT5)) & txPcrReg;
        uint32 j;

        Dio_WriteChannel(dioEnableSymbolicRef, STD_LOW);

        // Set tx pin to digital output, low.
        Port_SetPinMode(portTxSymbolicRef, txPcrForceDio);
        Dio_WriteChannel(dioTxSymbolicRef, STD_LOW);

        // Use the LowWaitCount parameter to achieve the 20us delay.
        for (j = 0; j < trcvCfg->LinSlvTrcvDioAccess.LinSlvTrcvTxLowWaitCount; j++) {
            ;
        }
        // Set tx port pin config back to its original state.
        Port_SetPinMode(portTxSymbolicRef, txPcrReg);

    } else if (LINTRCV_TRCV_MODE_STANDBY == TransceiverMode) {
        Dio_WriteChannel(dioEnableSymbolicRef, STD_LOW);
    } else { /* LINTRCV_TRCV_MODE_NORMAL */
        Dio_WriteChannel(dioEnableSymbolicRef, STD_HIGH);
    }

    return E_OK;
}

/**
 * Returns the transceiver mode.
 * @param LinSlv_Cfg
 * @param Channel
 * @param TransceiverMode
 * @return
 */
Std_ReturnType LinSlv_Internal_GetTrcvMode(const LinSlv_ContainerType* LinSlv_Cfg,
        uint8 Channel,
        LinTrcv_TrcvModeType *TransceiverModePtr)
{
    const LinSlv_TrcvType * trcvCfg;
    Std_ReturnType retval = E_OK;
    boolean dioEnablePin;
    boolean dioRstnPin;

    VALIDATE_W_RV( ( NULL !=  LinSlv_Cfg)        , LINSLV_GETTRCVMODE_SERVICE_ID, LINSLV_E_INVALID_POINTER, E_NOT_OK);
    VALIDATE_W_RV( ( NULL !=  TransceiverModePtr), LINSLV_GETTRCVMODE_SERVICE_ID, LINSLV_E_INVALID_POINTER, E_NOT_OK);

    trcvCfg = &LinSlv_Cfg->LinSlvCfgPtr->LinSlvChnTrcvConfig[Channel];

    dioEnablePin = (STD_HIGH == Dio_ReadChannel(trcvCfg->LinSlvTrcvDioAccess.LinSlvTrcvEnableSource)) ? TRUE : FALSE;
    dioRstnPin   = (STD_HIGH == Dio_ReadChannel(trcvCfg->LinSlvTrcvDioAccess.LinSlvTrcvRstnSource))   ? TRUE : FALSE;

    if (dioEnablePin && dioRstnPin) {
        *TransceiverModePtr = LINTRCV_TRCV_MODE_NORMAL;
    } else if (!dioEnablePin && dioRstnPin) {
        *TransceiverModePtr = LINTRCV_TRCV_MODE_STANDBY;
    } else if (!dioEnablePin && !dioRstnPin) {
        *TransceiverModePtr = LINTRCV_TRCV_MODE_SLEEP;
    } else {
        retval = E_NOT_OK;
    }

    return retval;
}
#endif
