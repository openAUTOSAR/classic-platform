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
#include "Det.h"
#include "Mcu.h"
#include <stdlib.h>
#include <string.h>
#include "LinIf_Cbk.h"
#if defined(USE_KERNEL)
#include "Os.h"
#include "isr.h"
#endif

#define ESCI(exp) (volatile struct ESCI_tag *)(0xFFFA0000 + (0x4000 * exp))

// LIN145: Reset -> LIN_UNINIT: After reset, the Lin module shall set its state to LIN_UNINIT.
static Lin_DriverStatusType LinDriverStatus = LIN_UNINIT;

static Lin_StatusType LinChannelStatus[LIN_CONTROLLER_CNT];
static Lin_StatusType LinChannelOrderedStatus[LIN_CONTROLLER_CNT];

static uint8* TxPtr[LIN_CONTROLLER_CNT];
static uint8* TxCurrPtr[LIN_CONTROLLER_CNT];
static uint8 TxSize[LIN_CONTROLLER_CNT];
static uint8* RxPtr[LIN_CONTROLLER_CNT];
static uint8* RxCurrPtr[LIN_CONTROLLER_CNT];
static uint8 RxSize[LIN_CONTROLLER_CNT];

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
	  uint32_t TN:1;
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


void LinInterrupt(uint8 Channel)
{
	volatile struct ESCI_tag * esciHw = ESCI(Channel);
	LinSRtype sr, tmp;
	LinLTRType tmpLtr;

	sr.R = esciHw->SR.R;

	// Clear flags
	tmp.R = 0;
	if(sr.B.TXRDY == 1)
	{
	  tmp.B.TXRDY = 1;
	}
	if(sr.B.RXRDY == 1)
	{
	  tmp.B.RXRDY = 1;
	}
	if(sr.B.TC == 1)
	{
	  tmp.B.TC = 1;
	}
	if(sr.B.RDRF == 1)
	{
	  tmp.B.RDRF = 1;
	}
	if(sr.B.IDLE == 1)
	{
	  tmp.B.IDLE = 1;
	}
	if(sr.B.OVFL == 1)
	{
	  tmp.B.OVFL = 1;
	}
	if(sr.B.FRC == 1)
	{
	  tmp.B.FRC = 1;
	}
	esciHw->SR.R = tmp.R;
	esciHw->LCR.B.TXIE = 0; // Always disable Tx Interrupt

	// TX
	if (LinChannelStatus[Channel]==LIN_TX_BUSY) {
		// Maybe transmit next byte
		if (TxSize[Channel] > 0 && sr.B.TXRDY) {
			tmpLtr.R = 0; // Clear
			tmpLtr.B4.T = *TxCurrPtr[Channel];
			TxCurrPtr[Channel]++; TxSize[Channel]--;
			esciHw->LCR.B.TXIE = 1; // Enable tx irq
			esciHw->LTR.R=tmpLtr.R; // write to transmit reg
 		}
		else {
			if(sr.B.FRC == 1){
				//Transmission complete
				if (LinChannelOrderedStatus[Channel]==LIN_CH_SLEEP){
					LinChannelStatus[Channel] = LIN_CH_SLEEP;
					LinChannelOrderedStatus[Channel]=LIN_CH_OPERATIONAL;
				}else {
					LinChannelStatus[Channel] = LIN_TX_OK;
				}
				esciHw->LCR.B.TXIE = 0; // Disable tx irq
			}
			else{
				if (LinChannelOrderedStatus[Channel]==LIN_CH_SLEEP){
					LinChannelStatus[Channel] = LIN_CH_SLEEP;
					LinChannelOrderedStatus[Channel]=LIN_CH_OPERATIONAL;
				}else {
					LinChannelStatus[Channel] = LIN_TX_ERROR;
				}
			}
		}
	}	//RX
	else if (LinChannelStatus[Channel]==LIN_RX_BUSY) {
	  // Maybe receive next byte
		if (RxSize[Channel] > 0 && sr.B.RXRDY) {
			tmpLtr.R = esciHw->LRR.R;
			*RxCurrPtr[Channel] = tmpLtr.B5.D;
			RxCurrPtr[Channel]++; RxSize[Channel]--;
 		} else if (RxSize[Channel] == 0 && sr.B.FRC == 1){
			//receive complete
			LinChannelStatus[Channel] = LIN_RX_OK;
			esciHw->LCR.B.RXIE = 0; // Disable Rx Interrupt
			esciHw->LCR.B.FCIE = 0; // Disable Rx Interrupt
 		}
	}

	// Error handling
	if(sr.B.OR || sr.B.NF || sr.B.FE || sr.B.PF || sr.B.BERR || sr.B.STO || sr.B.PBERR ||
	   sr.B.CERR || sr.B.CKERR || sr.B.OVFL) {
		static uint32 errCnt=0;
		errCnt++;
		if(LinChannelStatus[Channel]==LIN_RX_BUSY){
			LinChannelStatus[Channel] = LIN_RX_ERROR;
		}else if (LinChannelStatus[Channel]==LIN_TX_BUSY){
			LinChannelStatus[Channel] = LIN_TX_ERROR;
		}

		// Resynchronize driver and clear all errors
		esciHw->LCR.B.LRES = 1; // LIN Resynchronize. First set then cleared
		esciHw->LCR.B.LRES = 0; // LIN Resynchronize. First set then cleared
		// Clear flags
		esciHw->SR.R=0xffffffff;
	}
}

static void LinInterruptA()
{
	LinInterrupt(LIN_CTRL_A);
}
static void LinInterruptB()
{
	LinInterrupt(LIN_CTRL_B);
}
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


void Lin_Init( const Lin_ConfigType* Config )
{
	uint8 i;

	VALIDATE( (LinDriverStatus == LIN_UNINIT), LIN_INIT_SERVICE_ID, LIN_E_STATE_TRANSITION );
	//VALIDATE( (Config!=0), LIN_INIT_SERVICE_ID, LIN_E_INVALID_POINTER );

	for (i=0;i<LIN_CONTROLLER_CNT;i++)
	{
		// LIN171: On entering the state LIN_INIT, the Lin module shall set each channel into
		// state LIN_CH_UNINIT.
		LinChannelStatus[i] = LIN_CH_UNINIT;
		LinChannelOrderedStatus[i]=LIN_CH_OPERATIONAL;
		TxPtr[i] = 0;
		TxCurrPtr[i] = 0;
		TxSize[i] = 0;
		RxPtr[i] = 0;
		RxCurrPtr[i] = 0;
		RxSize[i] = 0;
	}

	//LIN146: LIN_UNINIT -> LIN_INIT: The Lin module shall transition from LIN_UNINIT
	// to LIN_INIT when the function Lin_Init is called.
	LinDriverStatus = LIN_INIT;
}
void Lin_DeInit()
{
	uint8 i;

	// Make sure all allocated buffers are freed
	for (i=0;i<LIN_CONTROLLER_CNT;i++)
	{
		if (RxPtr[i] != 0)	{
			free(RxPtr[i]);
		}
		if (TxPtr[i] != 0)	{
			free(TxPtr[i]);
		}
	}
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


	// Install the interrupt
	switch (Channel) {
		case 0: ISR_INSTALL_ISR2( "LinA", LinInterruptA, SCI_A_COMB, LIN_PRIO, 0 ); break;
		case 1: ISR_INSTALL_ISR2( "LinB", LinInterruptB, SCI_B_COMB, LIN_PRIO, 0 ); break;
		case 2: ISR_INSTALL_ISR2( "LinC", LinInterruptC, SCI_C_COMB, LIN_PRIO, 0 ); break;
		case 3: ISR_INSTALL_ISR2( "LinD", LinInterruptD, SCI_D_COMB, LIN_PRIO, 0 ); break;
		case 4: ISR_INSTALL_ISR2( "LinE", LinInterruptE, SCI_E_COMB, LIN_PRIO, 0 ); break;
		case 5: ISR_INSTALL_ISR2( "LinF", LinInterruptF, SCI_F_COMB, LIN_PRIO, 0 ); break;
		case 6: ISR_INSTALL_ISR2( "LinG", LinInterruptG, SCI_G_COMB, LIN_PRIO, 0 ); break;
		case 7: ISR_INSTALL_ISR2( "LinH", LinInterruptH, SCI_H_COMB, LIN_PRIO, 0 ); break;
		default: assert(0); break;
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

	esciHw->LCR.B.LDBG = 0;/*Normally, bit errors should cause the LIN FSM to reset, stop driving the bus immediately, and stop
	further DMA requests until the BERR flag has been cleared. Set ESCIx_LCR[LDBG] = 0,*/
	esciHw->LCR.B.STIE = 1; // Enable some fault irq's
	esciHw->LCR.B.PBIE = 1;
	esciHw->LCR.B.CKIE = 1;
	esciHw->LCR.B.OFIE = 1;

	esciHw->CR2.B.SBSTP = 1;/*ESCIx_CR2[SBSTP] = 1, and ESCIx_CR2[BSTP] = 1 to accomplish these functions.*/
	esciHw->CR2.B.BSTP = 1;

	esciHw->CR2.B.FBR = 1;    // Fast bit error detection provides superior error checking, so ESCIx_CR2[FBR] should be set;
	esciHw->CR2.B.BESM13 = 1; // normally it will be used with ESCIx_CR2[BESM13] = 1.*/

	/* The error indicators NF, FE, BERR, STO, PBERR, CERR, CKERR, and OVFL should be enabled. */
	/* TODO Should we have these interrupts or check the status register? */
	/*Initially a wakeup character may need to be transmitted on the LIN bus, so that the LIN slaves
	activate.*/

	esciHw->CR1.B.TE = 1; /* Both transmitter and receiver are enabled (ESCIx_CR1[TE] = 1, ESCIx_CR1[RE] = 1). */

	LinChannelStatus[Channel]=LIN_CH_OPERATIONAL;
}

void Lin_DeInitChannel( uint8 Channel )
{
	volatile struct ESCI_tag * esciHw = ESCI(Channel);
	VALIDATE( (Channel < LIN_CONTROLLER_CNT), LIN_DEINIT_CHANNEL_SERVICE_ID, LIN_E_INVALID_CHANNEL );

	//LIN178: The function Lin_DeInitChannel shall only be executable when the LIN
	//channel state-machine is in state LIN_CH_OPERATIONAL.
	if(LinChannelStatus[Channel] != LIN_CH_UNINIT){
		esciHw->CR2.B.MDIS = 1;/* The module is disabled by writing the ESCIx_CR2[MDIS] bit to 1. */

		if (RxPtr[Channel] != 0)	{
			free(RxPtr[Channel]);
		}
		if (TxPtr[Channel] != 0)	{
			free(TxPtr[Channel]);
		}

		LinChannelStatus[Channel]=LIN_CH_UNINIT;
	}
}

Std_ReturnType Lin_SendHeader(  uint8 Channel,  Lin_PduType* PduInfoPtr )
{
    LinSRtype tmp;
    LinLTRType tmpLtr;
	volatile struct ESCI_tag * esciHw = ESCI(Channel);
	imask_t state;

	// LIN021
    Irq_Save(state);
	if(LinChannelStatus[Channel] == LIN_TX_BUSY || LinChannelStatus[Channel] == LIN_TX_ERROR ||
	   LinChannelStatus[Channel] == LIN_RX_BUSY || LinChannelStatus[Channel] == LIN_RX_ERROR){
		LinChannelStatus[Channel]=LIN_CH_OPERATIONAL;
		esciHw->LCR.B.LRES = 1; // LIN Resynchronize. First set then cleared
		esciHw->LCR.B.LRES = 0; // LIN Resynchronize. First set then cleared
		esciHw->LCR.B.TXIE = 0; // Disable tx irq
		esciHw->LCR.B.RXIE = 0; // Disable Rx Interrupt
		esciHw->LCR.B.FCIE = 0; // Disable Rx Interrupt
		// Clear flags
		esciHw->SR.R=0xffffffff;
	}
    Irq_Restore(state);


	VALIDATE_W_RV( (LinDriverStatus != LIN_UNINIT), LIN_SEND_HEADER_SERVICE_ID, LIN_E_UNINIT, E_NOT_OK);
	VALIDATE_W_RV( (LinChannelStatus[Channel] != LIN_CH_UNINIT), LIN_SEND_HEADER_SERVICE_ID, LIN_E_CHANNEL_UNINIT, E_NOT_OK);
	VALIDATE_W_RV( (Channel < LIN_CONTROLLER_CNT), LIN_SEND_HEADER_SERVICE_ID, LIN_E_INVALID_CHANNEL, E_NOT_OK);
	//Send header is used to wake the net in this implementation(no actual header is sent
	// VALIDATE_W_RV( (LinChannelStatus[Channel] != LIN_CH_SLEEP), LIN_SEND_HEADER_SERVICE_ID, LIN_E_STATE_TRANSITION, E_NOT_OK);
	VALIDATE_W_RV( (PduInfoPtr != NULL), LIN_SEND_HEADER_SERVICE_ID, LIN_E_INVALID_POINTER, E_NOT_OK);

	// Byte 1
	tmpLtr.R = 0; // Clear
	tmpLtr.B1.PID = PduInfoPtr->Pid;
	tmp.R = 0; // Clear ready flag before send
	tmp.B.TXRDY = 1;
	esciHw->SR.R = tmp.R;
	esciHw->LTR.R=tmpLtr.R; // write to transmit reg

	// Byte 2
	tmpLtr.R = 0; // Clear
	tmpLtr.B2.L = PduInfoPtr->DI;
	tmp.R = 0; // Clear ready flag before send
	tmp.B.TXRDY = 1;
	esciHw->SR.R = tmp.R;
	esciHw->LTR.R=tmpLtr.R; // write to transmit reg

	// Byte 3
	tmpLtr.R = 0; // Clear
	if (PduInfoPtr->Cs == LIN_ENHANCED_CS){ //Frame identifiers 60 (0x3C) to 61 (0x3D) shall always use classic checksum
		tmpLtr.B3.HDCHK = 1;
	}
	tmpLtr.B3.CSUM = 1; // Append checksum to TX frame or verify for a RX
	tmpLtr.B3.CRC = 0; // Append two CRC bytes(Not LIN standard)
	if (PduInfoPtr->Drc == LIN_MASTER_RESPONSE)
	{
		LinChannelStatus[Channel]=LIN_TX_BUSY;
		tmpLtr.B3.TX = 1; // TX frame
		tmpLtr.B3.TN = 0; // Timeout not valid for TX
		tmp.R = 0; // Clear ready flag before send
		tmp.B.TXRDY = 1;
		esciHw->SR.R = tmp.R;
		esciHw->LCR.B.FCIE = 1; // // Enable frame complete
		esciHw->LCR.B.TXIE = 1; // Enable tx irq
		if (PduInfoPtr->DI > 0){
			if (TxPtr[Channel] != 0)  {
				free(TxPtr[Channel]);
			}
			TxCurrPtr[Channel] = TxPtr[Channel] = (uint8 *)malloc(PduInfoPtr->DI);
			TxSize[Channel] = PduInfoPtr->DI;
			memcpy(TxPtr[Channel],PduInfoPtr->SduPtr,PduInfoPtr->DI);
		}
		esciHw->LTR.R=tmpLtr.R; // write to transmit reg
	}


	else
	{
		LinChannelStatus[Channel]=LIN_RX_BUSY;
		if (RxPtr[Channel] != 0)	{
			free(RxPtr[Channel]);
		}
		RxCurrPtr[Channel] = RxPtr[Channel] = (uint8 *)malloc(PduInfoPtr->DI);
		RxSize[Channel] = PduInfoPtr->DI;

		tmpLtr.B3.TX = 0; // RX frame
		tmpLtr.B3.TN = (10*PduInfoPtr->DI + 45)*1.4; // Timeout. (10 × NDATA + 45) × 1.4 according to LIN1.3
		esciHw->LTR.R=tmpLtr.R; // write to transmit reg
		// Byte 4 for RX
		tmpLtr.R = 0; // Clear
		tmpLtr.B4.T = (10*PduInfoPtr->DI + 45)*1.4; // Timeout. (10 × NDATA + 45) × 1.4 according to LIN1.3
		tmp.R = 0; // Clear ready flag before send
		tmp.B.TXRDY = 1;
		esciHw->SR.R = tmp.R;
		esciHw->LTR.R=tmpLtr.R; // write to transmit reg
		esciHw->LCR.B.FCIE = 1; // Enable frame complete
		esciHw->LCR.B.RXIE = 1; // Enable rx irq
	}

	return E_OK;
}

Std_ReturnType Lin_SendResponse(  uint8 Channel,   Lin_PduType* PduInfoPtr )
{
	VALIDATE_W_RV( (LinDriverStatus != LIN_UNINIT), LIN_SEND_RESPONSE_SERVICE_ID, LIN_E_UNINIT, E_NOT_OK);
	VALIDATE_W_RV( (LinChannelStatus[Channel] != LIN_CH_UNINIT), LIN_SEND_RESPONSE_SERVICE_ID, LIN_E_CHANNEL_UNINIT, E_NOT_OK);
	VALIDATE_W_RV( (Channel < LIN_CONTROLLER_CNT), LIN_SEND_RESPONSE_SERVICE_ID, LIN_E_INVALID_CHANNEL, E_NOT_OK);
	VALIDATE_W_RV( (LinChannelStatus[Channel] != LIN_CH_SLEEP), LIN_SEND_RESPONSE_SERVICE_ID, LIN_E_STATE_TRANSITION, E_NOT_OK);
	VALIDATE_W_RV( (PduInfoPtr != NULL), LIN_SEND_RESPONSE_SERVICE_ID, LIN_E_INVALID_POINTER, E_NOT_OK);

	// The response is sent from within the header in this implementation since this is a master only implementation
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

		esciHw->LCR.B.WUIE = 1; // enable wake-up irq
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

	esciHw->LCR.B.WUIE = 0; // disable wake-up irq
	esciHw->LCR.B.WU = 1; // send wake up
	esciHw->LCR.B.WUD0 = 1; // delimiter time
	esciHw->LCR.B.WUD1 = 0; // delimiter time

	// Just send any header to trigger the wakeup signal
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
	imask_t state;
	VALIDATE_W_RV( (LinDriverStatus != LIN_UNINIT), LIN_GETSTATUS_SERVICE_ID, LIN_E_UNINIT, E_NOT_OK);
	VALIDATE_W_RV( (LinChannelStatus[Channel] != LIN_CH_UNINIT), LIN_GETSTATUS_SERVICE_ID, LIN_E_CHANNEL_UNINIT, E_NOT_OK);
	VALIDATE_W_RV( (Channel < LIN_CONTROLLER_CNT), LIN_GETSTATUS_SERVICE_ID, LIN_E_INVALID_CHANNEL, E_NOT_OK);
	VALIDATE_W_RV( (Lin_SduPtr!=NULL), LIN_GETSTATUS_SERVICE_ID, LIN_E_INVALID_POINTER, E_NOT_OK);

    Irq_Save(state);
	Lin_StatusType res = LinChannelStatus[Channel];
	// We can only check for valid sdu ptr when LIN_RX_OK
	if(LinChannelStatus[Channel] == LIN_RX_OK || LinChannelStatus[Channel] == LIN_RX_ERROR){
		*Lin_SduPtr = RxPtr[Channel];
		LinChannelStatus[Channel]=LIN_CH_OPERATIONAL;
	} else if(LinChannelStatus[Channel] == LIN_TX_OK || LinChannelStatus[Channel] == LIN_TX_ERROR){
		LinChannelStatus[Channel]=LIN_CH_OPERATIONAL;
	}
    Irq_Restore(state);
	return res;
}





