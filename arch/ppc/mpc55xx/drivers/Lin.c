/* -------------------------------- Arctic Core ------------------------------
 * Arctic Core - the open source AUTOSAR platform http://arccore.com
 *
 * Copyright (C) 2009  ArcCore AB <contact@arccore.com>
 *
 * This source code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by the
 * Free Software Foundation; See <http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 * -------------------------------- Arctic Core ------------------------------*/

#include "Lin.h"
#include "LinIf_Cbk.h"
#include "mpc55xx.h"
#if defined(USE_DET)
#include "Det.h"
#endif
#include "Mcu.h"
#include <stdlib.h>
#include <string.h>
#include "LinIf_Cbk.h"
#include "Os.h"
#include "isr.h"
#include "irq.h"
#include "arc.h"

/* ERRATA for REV_A of 551x chip. Will use a GPT timer for timeout handling */

#ifdef MPC551X_ERRATA_REV_A
#include "Gpt.h"

static Gpt_ChannelType LinChannelTimeOutGptChannelId[ LIN_CONTROLLER_CNT ];
static uint32          LinChannelBitTimeInTicks[ LIN_CONTROLLER_CNT ];
#endif

#define LIN_MAX_MSG_LENGTH 8

#if defined(CFG_MPC5567) || defined(CFG_MPC563XM)
#define ESCI(exp) (volatile struct ESCI_tag *)(0xFFFB0000 + (0x4000 * exp))
#else
#define ESCI(exp) (volatile struct ESCI_tag *)(0xFFFA0000 + (0x4000 * exp))
#endif

/* LIN145: Reset -> LIN_UNINIT: After reset, the Lin module shall set its state to LIN_UNINIT. */
static Lin_DriverStatusType LinDriverStatus = LIN_UNINIT;

static Lin_StatusType LinChannelStatus[LIN_CONTROLLER_CNT];
static Lin_StatusType LinChannelOrderedStatus[LIN_CONTROLLER_CNT];

/* static buffers, holds one frame at a time */
static uint8 LinBufTx[LIN_CONTROLLER_CNT][LIN_MAX_MSG_LENGTH];
static uint8 LinBufRx[LIN_CONTROLLER_CNT][LIN_MAX_MSG_LENGTH];

static uint8 *TxCurrPtr[LIN_CONTROLLER_CNT];
static uint8  TxSize[LIN_CONTROLLER_CNT];
static uint8 *RxCurrPtr[LIN_CONTROLLER_CNT];
static uint8  RxSize[LIN_CONTROLLER_CNT];

/* Development error macros. */
#if ( LIN_DEV_ERROR_DETECT == STD_ON )
#define VALIDATE(_exp,_api,_err ) \
        if( !(_exp) ) { \
          Det_ReportError(MODULE_ID_LIN,0,_api,_err); \
          return; \
        }

#define VALIDATE_W_RV(_exp,_api,_err,_rv ) \
        if( !(_exp) ) { \
          Det_ReportError(MODULE_ID_LIN,0,_api,_err); \
          return (_rv); \
        }
#else
#define VALIDATE(_exp,_api,_err )
#define VALIDATE_W_RV(_exp,_api,_err,_rv )
#endif

typedef volatile union
{
  uint32_t R;
  struct {
	  uint32_t PID:8;
	  uint32_t :24;
  } B1;
  struct {
	  uint32_t L:8;
	  uint32_t :24;
  } B2;
  struct {
	  uint32_t HDCHK:1;
	  uint32_t CSUM:1;
	  uint32_t CRC:1;
	  uint32_t TX:1;
	  uint32_t TN:4;
	  uint32_t :24;
  } B3;
  struct {
	  uint32_t T:8;
	  uint32_t :24;
  } B4;
  struct {
	  uint32_t D:8;
	  uint32_t :24;
  } B5;
}LinLTRType;

typedef volatile union {
    vuint32_t R;
    struct {
        vuint32_t TDRE:1;
        vuint32_t TC:1;
        vuint32_t RDRF:1;
        vuint32_t IDLE:1;
        vuint32_t OR:1;
        vuint32_t NF:1;
        vuint32_t FE:1;
        vuint32_t PF:1;
          vuint32_t:3;
        vuint32_t BERR:1;
          vuint32_t:3;
        vuint32_t RAF:1;
        vuint32_t RXRDY:1;
        vuint32_t TXRDY:1;
        vuint32_t LWAKE:1;
        vuint32_t STO:1;
        vuint32_t PBERR:1;
        vuint32_t CERR:1;
        vuint32_t CKERR:1;
        vuint32_t FRC:1;
          vuint32_t:7;
        vuint32_t OVFL:1;
    } B;
} LinSRtype;   /* Status Register */

static void ResyncDriver(uint8 Channel)
{
	volatile struct ESCI_tag * esciHw = ESCI(Channel);

	esciHw->LCR.B.TXIE = 0; /* Disable tx irq */
	esciHw->LCR.B.RXIE = 0; /* Disable Rx Interrupt */
	esciHw->LCR.B.FCIE = 0; /* Disable Rx Interrupt */

	/* Disable transmitter and receiver. */
	esciHw->CR1.B.TE = 0;
	esciHw->CR1.B.RE = 0;

  	/* Clear flags  */
	esciHw->SR.R = esciHw->SR.R;

	/* Prepare module for resynchronization. */
	esciHw->LCR.B.LRES = 1; /* LIN Resynchronize. First set then cleared. */
	/* Resynchronize module. */
	esciHw->LCR.B.LRES = 0; /* LIN Resynchronize. First set then cleared. */

	/* Enable transmitter and receiver. */
	esciHw->CR1.B.TE = 1;
	esciHw->CR1.B.RE = 1;

#if 0 /* This needs if we Disable Fast Bit Error Detection otherwise SR will not be cleared properly....strange */
	volatile uint32 i;
	for(i=0; i < 10000; ){
	  i++;
	}
#endif

	/* Clear set flags again */
	esciHw->SR.R = esciHw->SR.R;
}


void LinInterrupt(uint8 Channel)
{
	volatile struct ESCI_tag * esciHw = ESCI(Channel);
	LinSRtype sr, tmp;
	LinLTRType tmpLtr;
#ifdef MPC551X_ERRATA_REV_A
	uint8 stopTimeOutTimer = FALSE;
#endif 

	sr.R = esciHw->SR.R;

	/* Clear flags */
	tmp.R = 0;
	if(sr.B.TXRDY == 1){tmp.B.TXRDY = 1;}
	if(sr.B.RXRDY == 1){tmp.B.RXRDY = 1;}
	if(sr.B.TC == 1){tmp.B.TC = 1;}
	if(sr.B.RDRF == 1){tmp.B.RDRF = 1;}
	if(sr.B.IDLE == 1){tmp.B.IDLE = 1;}
	if(sr.B.OVFL == 1){tmp.B.OVFL = 1;}
	if(sr.B.FRC == 1){tmp.B.FRC = 1;}

	esciHw->SR.R = tmp.R;
	esciHw->LCR.B.TXIE = 0; /* Always disable Tx Interrupt */

	/* TX */
	if (LinChannelStatus[Channel]==LIN_TX_BUSY) {
		/* Maybe transmit next byte */
		if (TxSize[Channel] > 0 && sr.B.TXRDY) {
			tmpLtr.R = 0; /* Clear */
			tmpLtr.B4.T = *TxCurrPtr[Channel];
			TxCurrPtr[Channel]++; TxSize[Channel]--;
			esciHw->LCR.B.TXIE = 1; /* Enable tx irq */
			esciHw->LTR.R=tmpLtr.R; /* write to transmit reg */
 		}
		else {
			if(sr.B.FRC == 1){
				/* Transmission complete */
				if (LinChannelOrderedStatus[Channel]==LIN_CH_SLEEP){
					LinChannelStatus[Channel] = LIN_CH_SLEEP;
					LinChannelOrderedStatus[Channel]=LIN_CH_OPERATIONAL;
				}else {
					LinChannelStatus[Channel] = LIN_TX_OK;
				}
				esciHw->LCR.B.TXIE = 0; /* Disable tx irq */
			}
			else{
				if (LinChannelOrderedStatus[Channel]==LIN_CH_SLEEP){
					LinChannelStatus[Channel] = LIN_CH_SLEEP;
					LinChannelOrderedStatus[Channel]=LIN_CH_OPERATIONAL;
				}else {
					LinChannelStatus[Channel] = LIN_TX_ERROR;
				}
			}
#ifdef MPC551X_ERRATA_REV_A
			/* Transmission is over. */
			stopTimeOutTimer = TRUE;
#endif 
		}
	}	/* RX */
	else if (LinChannelStatus[Channel]==LIN_RX_BUSY) {
	  /* Maybe receive next byte */
		if (RxSize[Channel] > 0 && sr.B.RXRDY) {
			tmpLtr.R = esciHw->LRR.R;
			*RxCurrPtr[Channel] = tmpLtr.B5.D;
			RxCurrPtr[Channel]++; RxSize[Channel]--;
 		} else if (RxSize[Channel] == 0 && sr.B.FRC == 1){
			/* receive complete */
			LinChannelStatus[Channel] = LIN_RX_OK;
			esciHw->LCR.B.RXIE = 0; /* Disable Rx Interrupt */
			esciHw->LCR.B.FCIE = 0; /* Disable Rx Interrupt */
#ifdef MPC551X_ERRATA_REV_A
			/* Transmission is over. */
			stopTimeOutTimer = TRUE;
#endif 
 		}
	}

	/* Error handling
	 * OR    - Overrun
	 * NF    – Noise fault
	 * FE    – Framing Error
	 * PF    – Parity fault
	 * BERR  – Bit error (read back)
	 * STO   – Slave Timeout
	 * PBERR – Physical bit error
	 * CERR  – CRC error
	 * CKERR – Checksum error
	 * OVFL  – Overflow flag
	 */
	if(sr.B.OR || sr.B.NF || sr.B.FE || sr.B.PF || sr.B.BERR || sr.B.STO || sr.B.PBERR ||
	   sr.B.CERR || sr.B.CKERR || sr.B.OVFL) {
		static uint32 errCnt=0;
		errCnt++;
		if(LinChannelStatus[Channel]==LIN_RX_BUSY){
			LinChannelStatus[Channel] = LIN_RX_ERROR;
		}else if (LinChannelStatus[Channel]==LIN_TX_BUSY){
			LinChannelStatus[Channel] = LIN_TX_ERROR;
		}

	  	/* Clear flags  */
		esciHw->SR.R=0xffffffff;

		/* Resynchronize driver in GetStatus call */

#ifndef MPC551X_ERRATA_REV_A
	}
#else
		/* Transmission is over. */
		stopTimeOutTimer = TRUE;
	}

	/* Handle LIN Wake Up reset manually. See [Freescale Device Errata MPC5510ACE, Rev. 10 APR 2009, errata ID: 8632]. */
	if( sr.B.LWAKE )
	{
		/* Resynchronize driver. */
		esciHw->LCR.B.LRES = 1; /* LIN Resynchronize. First set then cleared */
		esciHw->LCR.B.LRES = 0; /* LIN Resynchronize. First set then cleared */

		/* Transmission is over. */
		stopTimeOutTimer = TRUE;
	}

	if( TRUE == stopTimeOutTimer )
	{
		Gpt_StopTimer( LinChannelTimeOutGptChannelId[ Channel ] );
	}
#endif
}

static void LinInterruptA()
{
	LinInterrupt(LIN_CTRL_A);
}
static void LinInterruptB()
{
	LinInterrupt(LIN_CTRL_B);
}
#if !defined(CFG_MPC5567) && !defined(CFG_MPC563XM)
static void LinInterruptC()
{
	LinInterrupt(LIN_CTRL_C);
}
static void LinInterruptD()
{
	LinInterrupt(LIN_CTRL_D);
}
static void LinInterruptE()
{
	LinInterrupt(LIN_CTRL_E);
}
static void LinInterruptF()
{
	LinInterrupt(LIN_CTRL_F);
}
static void LinInterruptG()
{
	LinInterrupt(LIN_CTRL_G);
}
static void LinInterruptH()
{
	LinInterrupt(LIN_CTRL_H);
}
#endif

#ifdef MPC551X_ERRATA_REV_A
void LinTimeOutInterrupt( uint8 channel )
{
	/* We ended up here because of that the eSCI module for the given channel has stopped internally.
	* Now we have to reset the LIN module state machine ourself.
	* See [Freescale Device Errata MPC5510ACE, Rev. 10 APR 2009, errata ID: 8173 and 4968]. */

	volatile struct ESCI_tag *esciHw = ESCI( channel );

	/* Make sure that the timer it stopped (e.g. if by mistake setup as continuously). */
	Gpt_StopTimer( LinChannelTimeOutGptChannelId[ channel ] );

	/* Disable transmitter and receiver. */
	esciHw->CR1.B.TE = 0;
	esciHw->CR1.B.RE = 0;

	/* Prepare module for resynchronization. */
	esciHw->LCR.B.LRES = 1; /* LIN Resynchronize. First set then cleared. */

	/* Enable transmitter and receiver. */
	esciHw->CR1.B.TE = 1;
	esciHw->CR1.B.RE = 1;

	/* Resynchronize module. */
	esciHw->LCR.B.LRES = 0; /* LIN Resynchronize. First set then cleared. */
}

void LinTimeOutInterruptA()
{
  LinTimeOutInterrupt( LIN_CTRL_A );
}
void LinTimeOutInterruptB()
{
  LinTimeOutInterrupt( LIN_CTRL_B );
}
void LinTimeOutInterruptC()
{
  LinTimeOutInterrupt( LIN_CTRL_C );
}
void LinTimeOutInterruptD()
{
  LinTimeOutInterrupt( LIN_CTRL_D );
}
void LinTimeOutInterruptE()
{
  LinTimeOutInterrupt( LIN_CTRL_E );
}
void LinTimeOutInterruptF()
{
  LinTimeOutInterrupt( LIN_CTRL_F );
}
void LinTimeOutInterruptG()
{
  LinTimeOutInterrupt( LIN_CTRL_G );
}
void LinTimeOutInterruptH()
{
  LinTimeOutInterrupt( LIN_CTRL_H );
}
#endif

void Lin_Init( const Lin_ConfigType* Config )
{
	uint8 i;

	VALIDATE( (LinDriverStatus == LIN_UNINIT), LIN_INIT_SERVICE_ID, LIN_E_STATE_TRANSITION );
	/* VALIDATE( (Config!=0), LIN_INIT_SERVICE_ID, LIN_E_INVALID_POINTER ); */

	for (i=0;i<LIN_CONTROLLER_CNT;i++)
	{
		/* LIN171: On entering the state LIN_INIT, the Lin module shall set each channel into
		 * state LIN_CH_UNINIT. */
		LinChannelStatus[i] = LIN_CH_UNINIT;
		LinChannelOrderedStatus[i]=LIN_CH_OPERATIONAL;
		TxCurrPtr[i] = 0;
		TxSize[i] = 0;
		RxCurrPtr[i] = 0;
		RxSize[i] = 0;
#ifdef MPC551X_ERRATA_REV_A
		LinChannelTimeOutGptChannelId[ i ] = 0;
		LinChannelBitTimeInTicks[ i ] = 0;
#endif
	}

	/* LIN146: LIN_UNINIT -> LIN_INIT: The Lin module shall transition from LIN_UNINIT
	 * to LIN_INIT when the function Lin_Init is called. */
	LinDriverStatus = LIN_INIT;
}
void Lin_DeInit()
{
  LinDriverStatus = LIN_UNINIT;
}

void Lin_WakeupValidation( void )
{

}

void Lin_InitChannel(  uint8 Channel,   const Lin_ChannelConfigType* Config )
{
	volatile struct ESCI_tag * esciHw = ESCI(Channel);
	enum
	{
	  LIN_PRIO = 3
	};

	VALIDATE( (Config!=0), LIN_INIT_CHANNEL_SERVICE_ID, LIN_E_INVALID_POINTER );
	VALIDATE( (LinDriverStatus != LIN_UNINIT), LIN_INIT_CHANNEL_SERVICE_ID, LIN_E_UNINIT );
	VALIDATE( (Channel < LIN_CONTROLLER_CNT), LIN_INIT_CHANNEL_SERVICE_ID, LIN_E_INVALID_CHANNEL );

	/* Install the interrupt */
	switch(Channel){
	case 0:
#if defined(CFG_MPC5567) || defined(CFG_MPC563XM)
		ISR_INSTALL_ISR2("LinIsr", LinInterruptA, (IrqType)(ESCI_A_COMB0),LIN_PRIO, 0);
#else
		ISR_INSTALL_ISR2("LinIsr", LinInterruptA, (IrqType)(SCI_A_COMB),LIN_PRIO, 0);
#endif
		break;
	case 1:
#if defined(CFG_MPC5567) || defined(CFG_MPC563XM)
		ISR_INSTALL_ISR2("LinIsr", LinInterruptB, (IrqType)(ESCI_A_COMB1),LIN_PRIO, 0);
#else
		ISR_INSTALL_ISR2("LinIsr", LinInterruptB, (IrqType)(SCI_B_COMB),LIN_PRIO, 0);
#endif
		break;
	case 2:
#if !defined(CFG_MPC5567) && !defined(CFG_MPC563XM)
		ISR_INSTALL_ISR2("LinIsr", LinInterruptC, (IrqType)(SCI_C_COMB),LIN_PRIO, 0);
#endif
		break;
	case 3:
#if !defined(CFG_MPC5567) && !defined(CFG_MPC563XM)
		ISR_INSTALL_ISR2("LinIsr", LinInterruptD, (IrqType)(SCI_D_COMB),LIN_PRIO, 0);
#endif
		break;
	case 4:
#if !defined(CFG_MPC5567) && !defined(CFG_MPC563XM)
		ISR_INSTALL_ISR2("LinIsr", LinInterruptE, (IrqType)(SCI_E_COMB),LIN_PRIO, 0);
#endif
		break;
	case 5:
#if !defined(CFG_MPC5567) && !defined(CFG_MPC563XM)
		ISR_INSTALL_ISR2("LinIsr", LinInterruptF, (IrqType)(SCI_F_COMB),LIN_PRIO, 0);
#endif
		break;
	case 6:
#if !defined(CFG_MPC5567) && !defined(CFG_MPC563XM)
		ISR_INSTALL_ISR2("LinIsr", LinInterruptG, (IrqType)(SCI_G_COMB+2),LIN_PRIO, 0);
#endif
		break;
	case 7:
#if !defined(CFG_MPC5567) && !defined(CFG_MPC563XM)
		ISR_INSTALL_ISR2("LinIsr", LinInterruptH, (IrqType)(SCI_H_COMB+3),LIN_PRIO, 0);
#endif
		break;
	default:
		break;
	}

	esciHw->CR2.B.MDIS = 0;/* The module is enabled by writing the ESCIx_CR2[MDIS] bit to 0. */

	esciHw->CR1.B.RE = 1;
	esciHw->CR1.B.M = 0; /* The data format bit ESCIx_CR1[M], is set to 0 (8 data bits), and the parity is disabled (PE = 0).*/
	esciHw->CR1.B.PE = 0;
	esciHw->CR1.B.TIE = 0; /*ESCIx_CR1[TIE], ESCIx_CR1[TCIE], ESCIx_CR1[RIE] interrupt enable bits should be inactive.*/
	esciHw->CR1.B.TCIE = 0;
	esciHw->CR1.B.RIE = 0;
	  /* Set up ESCIx_CR1 for LIN */
	  /*
	   * SCI Baud Rate. Used by the counter to determine the baud rate of the eSCI.
	   * The formula for calculating the baud rate is:
	   *
	   *                      eSCI system clock
	   * SCI baud rate =   -----------------------
	   *                          16 × SBR
	   *
	   * where SBR can contain a value from 1 to 8191. After reset, the baud generator
	   * is disabled until the TE bit or the RE bit is set for the first time. The baud
	   * rate generator is disabled when SBR = 0x0.
	   */
	esciHw->CR1.B.SBR  = McuE_GetPeripheralClock(Config->LinClockRef)/(16*Config->LinChannelBaudRate);
	esciHw->LCR.B.LIN = 1;  /* Instead, the LIN interrupts should be used.Switch eSCI to LIN mode (ESCIx_LCR[LIN] = 1).*/

	esciHw->CR2.B.BRK13 = 1;/* The LIN standard requires that the break character always be 13 bits long
	(ESCIx_CR2[BRK13] = 1). The eSCI will work with BRK13=0, but it will violate LIN 2.0. */

#ifdef MPC551X_ERRATA_REV_A
	esciHw->LCR.B.LDBG = 1; /* Enable LIN debug => Disable automatic reset of the LIN FSM. See [Freescale Device Errata MPC5510ACE, Rev. 10 APR 2009, errata ID: 8632] */
	esciHw->CR2.B.FBR = 1;  /* Disable Fast Bit Error Detection. See [Freescale Device Errata MPC5510ACE, Rev. 10 APR 2009, errata ID: 8635] */
#else
#if !defined(CFG_MPC563XM)
	esciHw->LCR.B.LDBG = 0; /* Normally, bit errors should cause the LIN FSM to reset, stop driving the bus immediately, and stop
	                         * further DMA requests until the BERR flag has been cleared. Set ESCIx_LCR[LDBG] = 0,*/
#endif
	esciHw->CR2.B.FBR = 1;    /* Fast bit error detection provides superior error checking, so ESCIx_CR2[FBR] should be set; */
#endif
	esciHw->LCR.B.STIE = 1; /* Enable some fault irq's */
	esciHw->LCR.B.PBIE = 1;
	esciHw->LCR.B.CKIE = 1;
	esciHw->LCR.B.OFIE = 1;

	esciHw->CR2.B.SBSTP = 1;/*ESCIx_CR2[SBSTP] = 1, and ESCIx_CR2[BSTP] = 1 to accomplish these functions.*/
	esciHw->CR2.B.BSTP = 1;

	esciHw->CR2.B.BESM13 = 1; /* normally it will be used with ESCIx_CR2[BESM13] = 1.*/

	/* The error indicators NF, FE, BERR, STO, PBERR, CERR, CKERR, and OVFL should be enabled. */
	/* Should we have these interrupts or check the status register? */
	/*Initially a wakeup character may need to be transmitted on the LIN bus, so that the LIN slaves
	activate.*/

	esciHw->CR1.B.TE = 1; /* Both transmitter and receiver are enabled (ESCIx_CR1[TE] = 1, ESCIx_CR1[RE] = 1). */

	LinChannelStatus[Channel]=LIN_CH_OPERATIONAL;
#ifdef MPC551X_ERRATA_REV_A
	LinChannelTimeOutGptChannelId[ Channel ] = Config->LinTimeOutGptChannelId;
	LinChannelBitTimeInTicks[ Channel ] = McuE_GetPeripheralClock( Config->LinClockRef / Config->LinChannelBaudRate ) / Config->LinChannelBaudRate;
#endif
}

void Lin_DeInitChannel( uint8 Channel )
{
	volatile struct ESCI_tag * esciHw = ESCI(Channel);
	VALIDATE( (Channel < LIN_CONTROLLER_CNT), LIN_DEINIT_CHANNEL_SERVICE_ID, LIN_E_INVALID_CHANNEL );

#ifdef MPC551X_ERRATA_REV_A
	/* Make sure that the timer is stopped. */
	Gpt_StopTimer( LinChannelTimeOutGptChannelId[ Channel ] );
#endif

	/* LIN178: The function Lin_DeInitChannel shall only be executable when the LIN
	 * channel state-machine is in state LIN_CH_OPERATIONAL. */
	if(LinChannelStatus[Channel] != LIN_CH_UNINIT){
		esciHw->CR2.B.MDIS = 1;/* The module is disabled by writing the ESCIx_CR2[MDIS] bit to 1. */

		LinChannelStatus[Channel]=LIN_CH_UNINIT;
	}
}

Std_ReturnType Lin_SendHeader(  uint8 Channel,  Lin_PduType* PduInfoPtr )
{
    LinSRtype tmp;
    LinLTRType tmpLtr;
	volatile struct ESCI_tag * esciHw = ESCI(Channel);
	imask_t state;

	/* LIN021 */
    Irq_Save(state);
	if(LinChannelStatus[Channel] == LIN_TX_BUSY || LinChannelStatus[Channel] == LIN_TX_ERROR ||
	   LinChannelStatus[Channel] == LIN_RX_BUSY || LinChannelStatus[Channel] == LIN_RX_ERROR)
	{
		ResyncDriver(Channel);
		LinChannelStatus[Channel]=LIN_CH_OPERATIONAL;
	}
    Irq_Restore(state);


	VALIDATE_W_RV( (LinDriverStatus != LIN_UNINIT), LIN_SEND_HEADER_SERVICE_ID, LIN_E_UNINIT, E_NOT_OK);
	VALIDATE_W_RV( (LinChannelStatus[Channel] != LIN_CH_UNINIT), LIN_SEND_HEADER_SERVICE_ID, LIN_E_CHANNEL_UNINIT, E_NOT_OK);
	VALIDATE_W_RV( (Channel < LIN_CONTROLLER_CNT), LIN_SEND_HEADER_SERVICE_ID, LIN_E_INVALID_CHANNEL, E_NOT_OK);
	/* Send header is used to wake the net in this implementation(no actual header is sent */
	/* VALIDATE_W_RV( (LinChannelStatus[Channel] != LIN_CH_SLEEP), LIN_SEND_HEADER_SERVICE_ID, LIN_E_STATE_TRANSITION, E_NOT_OK); */
	VALIDATE_W_RV( (PduInfoPtr != NULL), LIN_SEND_HEADER_SERVICE_ID, LIN_E_INVALID_POINTER, E_NOT_OK);

	/* Byte 1 */
	tmpLtr.R = 0; /* Clear */
	tmpLtr.B1.PID = PduInfoPtr->Pid;
	tmp.R = 0; /* Clear ready flag before send */
	tmp.B.TXRDY = 1;
	esciHw->SR.R = tmp.R;
	esciHw->LTR.R=tmpLtr.R; /* write to transmit reg */

	/* Byte 2 */
	tmpLtr.R = 0; /* Clear */
	tmpLtr.B2.L = PduInfoPtr->DI;
	tmp.R = 0; /* Clear ready flag before send */
	tmp.B.TXRDY = 1;
	esciHw->SR.R = tmp.R;
	esciHw->LTR.R=tmpLtr.R; /* write to transmit reg */

	/* Byte 3 */
	tmpLtr.R = 0; /* Clear */
	if (PduInfoPtr->Cs == LIN_ENHANCED_CS){ /*Frame identifiers 60 (0x3C) to 61 (0x3D) shall always use classic checksum */
		tmpLtr.B3.HDCHK = 1;
	}
	tmpLtr.B3.CSUM = 1; /* Append checksum to TX frame or verify for a RX */
	tmpLtr.B3.CRC = 0; /* Append two CRC bytes(Not LIN standard) */

	/* Calculate the time out value for the frame.(10 × NDATA + 45) × 1.4 according to LIN1.3 */
	uint16 timeOutValue = (uint16)( ( ( 10 * PduInfoPtr->DI + 45 ) * 14 ) / 10);

	if (PduInfoPtr->Drc == LIN_MASTER_RESPONSE)
	{
		LinChannelStatus[Channel]=LIN_TX_BUSY;
		tmpLtr.B3.TX = 1; /* TX frame */
		tmpLtr.B3.TN = 0; /* Timeout not valid for TX */
		tmp.R = 0; /* Clear ready flag before send */
		tmp.B.TXRDY = 1;
		esciHw->SR.R = tmp.R;
		esciHw->LCR.B.FCIE = 1; /* Enable frame complete */
		esciHw->LCR.B.TXIE = 1; /* Enable tx irq */
		if (PduInfoPtr->DI > 0){
			TxCurrPtr[Channel] = LinBufTx[Channel];
			TxSize[Channel] = PduInfoPtr->DI;
			memcpy(TxCurrPtr[Channel],PduInfoPtr->SduPtr,PduInfoPtr->DI);
		}
		esciHw->LTR.R=tmpLtr.R; /* write to transmit reg */
	}


	else
	{
		LinChannelStatus[Channel]=LIN_RX_BUSY;
		RxCurrPtr[Channel] = LinBufRx[Channel];
		RxSize[Channel] = PduInfoPtr->DI;

		tmpLtr.B3.TX = 0; /* RX frame */
		tmpLtr.B3.TN = timeOutValue >> 8;     /* Most significant bits to be set here. */
		esciHw->LTR.R=tmpLtr.R; /* write to transmit reg */
		/* Byte 4 for RX */
		tmpLtr.R = 0; /* Clear */
        tmpLtr.B4.T = timeOutValue & 0xFF;   /* Least significant bits to be set here. */
		tmp.R = 0; /* Clear ready flag before send */
		tmp.B.TXRDY = 1;
		esciHw->SR.R = tmp.R;
		esciHw->LTR.R=tmpLtr.R; /* write to transmit reg */
		esciHw->LCR.B.FCIE = 1; /* Enable frame complete */
		esciHw->LCR.B.RXIE = 1; /* Enable rx irq */
	}

#ifdef MPC551X_ERRATA_REV_A
	/* Set up the GPT to twice as long time. Normally this time out shall not be needed but there are two
	* Erratas telling us that the LIN message timeout (written above to byte 3 and 4) is sometimes not used
	* by the LIN module and the LIN module state machine will then wait in infinity.
	* See [Freescale Device Errata MPC5510ACE, Rev. 10 APR 2009, errata ID: 8173 and 4968]. */
	Gpt_StopTimer( LinChannelTimeOutGptChannelId[ Channel ] );
	Gpt_StartTimer( LinChannelTimeOutGptChannelId[ Channel ], LinChannelBitTimeInTicks[ Channel ] * timeOutValue * 2 );
#endif

	return E_OK;
}

Std_ReturnType Lin_SendResponse(  uint8 Channel,   Lin_PduType* PduInfoPtr )
{
	VALIDATE_W_RV( (LinDriverStatus != LIN_UNINIT), LIN_SEND_RESPONSE_SERVICE_ID, LIN_E_UNINIT, E_NOT_OK);
	VALIDATE_W_RV( (LinChannelStatus[Channel] != LIN_CH_UNINIT), LIN_SEND_RESPONSE_SERVICE_ID, LIN_E_CHANNEL_UNINIT, E_NOT_OK);
	VALIDATE_W_RV( (Channel < LIN_CONTROLLER_CNT), LIN_SEND_RESPONSE_SERVICE_ID, LIN_E_INVALID_CHANNEL, E_NOT_OK);
	VALIDATE_W_RV( (LinChannelStatus[Channel] != LIN_CH_SLEEP), LIN_SEND_RESPONSE_SERVICE_ID, LIN_E_STATE_TRANSITION, E_NOT_OK);
	VALIDATE_W_RV( (PduInfoPtr != NULL), LIN_SEND_RESPONSE_SERVICE_ID, LIN_E_INVALID_POINTER, E_NOT_OK);

	/* The response is sent from within the header in this implementation since this is a master only implementation */
	return E_OK;
}

Std_ReturnType Lin_GoToSleep(  uint8 Channel )
{
	volatile struct ESCI_tag * esciHw = ESCI(Channel);
	Lin_PduType PduInfo;
	uint8 data[8] = {0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

	VALIDATE_W_RV( (LinDriverStatus != LIN_UNINIT), LIN_GO_TO_SLEEP_SERVICE_ID, LIN_E_UNINIT, E_NOT_OK);
	VALIDATE_W_RV( (LinChannelStatus[Channel] != LIN_CH_UNINIT), LIN_GO_TO_SLEEP_SERVICE_ID, LIN_E_CHANNEL_UNINIT, E_NOT_OK);
	VALIDATE_W_RV( (Channel < LIN_CONTROLLER_CNT), LIN_GO_TO_SLEEP_SERVICE_ID, LIN_E_INVALID_CHANNEL, E_NOT_OK);
	VALIDATE_W_RV( (LinChannelStatus[Channel] != LIN_CH_SLEEP), LIN_GO_TO_SLEEP_SERVICE_ID, LIN_E_STATE_TRANSITION, E_NOT_OK);

	if (LinChannelOrderedStatus[Channel]!=LIN_CH_SLEEP){
		LinChannelOrderedStatus[Channel]=LIN_CH_SLEEP;

		PduInfo.Cs = LIN_CLASSIC_CS;
		PduInfo.Pid = 0x3C;
		PduInfo.SduPtr = data;
		PduInfo.DI = 8;
		PduInfo.Drc = LIN_MASTER_RESPONSE;

		Lin_SendHeader(Channel,  &PduInfo);
		Lin_SendResponse(Channel,  &PduInfo);

		esciHw->LCR.B.WUIE = 1; /* enable wake-up irq */
	}
	return E_OK;
}

Std_ReturnType Lin_GoToSleepInternal(  uint8 Channel )
{
	VALIDATE_W_RV( (LinDriverStatus != LIN_UNINIT), LIN_GO_TO_SLEEP_INTERNAL_SERVICE_ID, LIN_E_UNINIT, E_NOT_OK);
	VALIDATE_W_RV( (LinChannelStatus[Channel] != LIN_CH_UNINIT), LIN_GO_TO_SLEEP_INTERNAL_SERVICE_ID, LIN_E_CHANNEL_UNINIT, E_NOT_OK);
	VALIDATE_W_RV( (Channel < LIN_CONTROLLER_CNT), LIN_GO_TO_SLEEP_INTERNAL_SERVICE_ID, LIN_E_INVALID_CHANNEL, E_NOT_OK);
	VALIDATE_W_RV( (LinChannelStatus[Channel] != LIN_CH_SLEEP), LIN_GO_TO_SLEEP_INTERNAL_SERVICE_ID, LIN_E_STATE_TRANSITION, E_NOT_OK);
	Lin_GoToSleep(Channel);
	return E_OK;
}

Std_ReturnType Lin_WakeUp( uint8 Channel )
{
	volatile struct ESCI_tag * esciHw = ESCI(Channel);
	Lin_PduType PduInfo;
	uint8 data[2] = {0xFF,0xFF};

	VALIDATE_W_RV( (LinDriverStatus != LIN_UNINIT), LIN_WAKE_UP_SERVICE_ID, LIN_E_UNINIT, E_NOT_OK);
	VALIDATE_W_RV( (LinChannelStatus[Channel] != LIN_CH_UNINIT), LIN_WAKE_UP_SERVICE_ID, LIN_E_CHANNEL_UNINIT, E_NOT_OK);
	VALIDATE_W_RV( (Channel < LIN_CONTROLLER_CNT), LIN_WAKE_UP_SERVICE_ID, LIN_E_INVALID_CHANNEL, E_NOT_OK);
	VALIDATE_W_RV( (LinChannelStatus[Channel] == LIN_CH_SLEEP), LIN_WAKE_UP_SERVICE_ID, LIN_E_STATE_TRANSITION, E_NOT_OK);

	esciHw->LCR.B.WUIE = 0; /* disable wake-up irq */
	esciHw->LCR.B.WU = 1; /* send wake up */
	esciHw->LCR.B.WUD0 = 1; /* delimiter time */
	esciHw->LCR.B.WUD1 = 0; /* delimiter time */

	/* Just send any header to trigger the wakeup signal */
	PduInfo.Cs = LIN_CLASSIC_CS;
	PduInfo.Pid = 0x00;
	PduInfo.SduPtr = data;
	PduInfo.DI = 2;
	PduInfo.Drc = LIN_SLAVE_RESPONSE;
	Lin_SendHeader(Channel,  &PduInfo);

	LinChannelStatus[Channel]=LIN_CH_OPERATIONAL;
	return E_OK;
}

Lin_StatusType Lin_GetStatus( uint8 Channel, uint8** Lin_SduPtr )
{
	VALIDATE_W_RV( (LinDriverStatus != LIN_UNINIT), LIN_GETSTATUS_SERVICE_ID, LIN_E_UNINIT, E_NOT_OK);
	VALIDATE_W_RV( (LinChannelStatus[Channel] != LIN_CH_UNINIT), LIN_GETSTATUS_SERVICE_ID, LIN_E_CHANNEL_UNINIT, E_NOT_OK);
	VALIDATE_W_RV( (Channel < LIN_CONTROLLER_CNT), LIN_GETSTATUS_SERVICE_ID, LIN_E_INVALID_CHANNEL, E_NOT_OK);
	VALIDATE_W_RV( (Lin_SduPtr!=NULL), LIN_GETSTATUS_SERVICE_ID, LIN_E_INVALID_POINTER, E_NOT_OK);

	imask_t state;
    Irq_Save(state);
	Lin_StatusType res = LinChannelStatus[Channel];
	/* We can only check for valid sdu ptr when LIN_RX_OK */
	if(LinChannelStatus[Channel] == LIN_RX_OK || LinChannelStatus[Channel] == LIN_RX_ERROR){
		*Lin_SduPtr = LinBufRx[Channel];
		if(LinChannelStatus[Channel] == LIN_RX_ERROR){
			ResyncDriver(Channel);
		}
		LinChannelStatus[Channel]=LIN_CH_OPERATIONAL;
	} else if(LinChannelStatus[Channel] == LIN_TX_OK || LinChannelStatus[Channel] == LIN_TX_ERROR){
		if(LinChannelStatus[Channel] == LIN_TX_ERROR){
			ResyncDriver(Channel);
		}
		LinChannelStatus[Channel]=LIN_CH_OPERATIONAL;
	}
    Irq_Restore(state);
	return res;
}





