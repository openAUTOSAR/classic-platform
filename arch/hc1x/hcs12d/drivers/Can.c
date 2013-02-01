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








#include "Can.h"

#ifndef USE_CAN_STUB
#include "regs.h"
#include "Cpu.h"
#include "Mcu.h"
#include "CanIf_Cbk.h"
#if defined(USE_DET)
#include "Det.h"
#endif
#if defined(USE_DEM)
#include "Dem.h"
#endif
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "Os.h"
#include "isr.h"
#include "arc.h"

#define USE_CAN_STATISTICS      STD_OFF

// bits in CANxCTL0:
#define BM_INITRQ	0x01
#define BM_SLPRQ	0x02
#define BM_WUPE 	0x04

// bits in CANxCTL1:
#define BM_INITAK	0x01
#define BM_SLPAK	0x02
#define BM_WUPM 	0x04
#define BM_LISTEN	0x10
#define BM_LOOPB	0x20
#define BM_CLKSRC	0x40
#define BM_CANE		0x80

// bits in CANxRFLG:
// bits in CANxRIER:
#define BM_WUPI		0x80
#define BM_CSCI		0x40
#define BM_RSTAT1	0x20
#define BM_RSTAT0	0x10
#define BM_TSTAT1	0x08
#define BM_TSTAT0	0x04
#define BM_OVRI 	0x02
#define BM_RXF		0x01

// bits in CANxTFLG:
// bits in CANxTBSEL:
#define BM_TX2		0x04
#define BM_TX1		0x02
#define BM_TX0		0x01

typedef struct{
	volatile uint8_t   idr0; // Identifier Register 0
	volatile uint8_t   idr1;
	volatile uint8_t   idr2;
	volatile uint8_t   idr3;
	volatile uint8_t   ds0; // Data Segment Register 0
	volatile uint8_t   ds1;
	volatile uint8_t   ds2;
	volatile uint8_t   ds3;
	volatile uint8_t   ds4;
	volatile uint8_t   ds5;
	volatile uint8_t   ds6;
	volatile uint8_t   ds7;
	volatile uint8_t   dlr;  // Data Length Register
	volatile uint8_t   tbpr; // Transmit Buffer Priority Register
	volatile uint8_t   tsr_hb; // Time Stamp Register (High Byte)
	volatile uint8_t   tsr_lb; // Time Stamp Register (Low Byte)
} RxTxBuf_t;

/*
IDR0 : ID28 ID27 ID26 ID25 ID24 ID23 ID22 ID21
IDR1 : ID20 ID19 ID18 SRR  IDE  ID17 ID16 ID15
IDR2 : ID14 ID13 ID12 ID11 ID10 ID9  ID8  ID7
IDR3 : ID6  ID5  ID4  ID3  ID2  ID1  ID0  RTR
*/
typedef union {
	volatile uint8_t R[4];
    struct {
		volatile uint8_t id28to21;
		volatile uint8_t id20to18:3;
		volatile uint8_t SRR:1;
		volatile uint8_t IDE:1;
		volatile uint8_t id17to15:3;
		volatile uint8_t id14to7;
		volatile uint8_t id6to0:7;
		volatile uint8_t RTR:1;
    } Bit;
} IdrType;


typedef struct{
	volatile uint8_t   CTL0; /*   control register 0 */
	volatile uint8_t   CTL1; /*   control register 1 */
	volatile uint8_t   BTR0; /*   bus timing register 0 */
	volatile uint8_t   BTR1; /*   bus timing register 1 */
	volatile uint8_t   RFLG; /*   receiver flag register */
	volatile uint8_t   RIER; /*   receiver interrupt reg */
	volatile uint8_t   TFLG; /*   transmitter flag reg */
	volatile uint8_t   TIER; /*   transmitter control reg */
	volatile uint8_t   TARQ; /*   transmitter abort request */
	volatile uint8_t   TAAK; /*   transmitter abort acknowledge */
	volatile uint8_t   TBSEL; /*   transmit buffer selection */
	volatile uint8_t   IDAC; /*   identifier acceptance */
	volatile uint8_t   NOTUSED1;
	volatile uint8_t   NOTUSED2;
	volatile uint8_t   RXERR; /*   receive error counter */
	volatile uint8_t   TXERR; /*   transmit error counter */
	volatile uint8_t   IDAR0; /*   id acceptance reg 0 */
	volatile uint8_t   IDAR1; /*   id acceptance reg 1 */
	volatile uint8_t   IDAR2; /*   id acceptance reg 2 */
	volatile uint8_t   IDAR3; /*   id acceptance reg 3 */
	volatile uint8_t   IDMR0; /*   id mask register 0 */
	volatile uint8_t   IDMR1; /*   id mask register 1 */
	volatile uint8_t   IDMR2; /*   id mask register 2 */
	volatile uint8_t   IDMR3; /*   id mask register 3 */
	volatile uint8_t   IDAR4; /*   id acceptance reg 4 */
	volatile uint8_t   IDAR5; /*   id acceptance reg 5 */
	volatile uint8_t   IDAR6; /*   id acceptance reg 6 */
	volatile uint8_t   IDAR7; /*   id acceptance reg 7 */
	volatile uint8_t   IDMR4; /*   id mask register 4 */
	volatile uint8_t   IDMR5; /*   id mask register 5 */
	volatile uint8_t   IDMR6; /*   id mask register 6 */
	volatile uint8_t   IDMR7; /*   id mask register 7 */
	volatile RxTxBuf_t   RXFG; /*   receive buffer */
	volatile RxTxBuf_t   TXFG; /*   transmit buffer */
} CAN_HW_t;

/* CONFIGURATION NOTES
 * ------------------------------------------------------------------
 * - CanHandleType must be CAN_ARC_HANDLE_TYPE_BASIC
 *   i.e. CanHandleType=CAN_ARC_HANDLE_TYPE_FULL NOT supported
 *   i.e CanIdValue is NOT supported
 * - All CanXXXProcessing must be CAN_ARC_PROCESS_TYPE_INTERRUPT
 *   ie CAN_ARC_PROCESS_TYPE_POLLED not supported
 * - HOH's for Tx are global and Rx are for each controller
 */

/* IMPLEMENTATION NOTES
 * -----------------------------------------------
 * - A HOH us unique for a controller( not a config-set )
 * - Hrh's are numbered for each controller from 0
 * - Only one transmit mailbox is used because otherwise
 *   we cannot use tx_confirmation since there is no way to know
 *   which mailbox caused the tx interrupt. TP will need this feature.
 * - Sleep,wakeup not fully implemented since other modules lack functionality
 */

/* ABBREVATIONS
 *  -----------------------------------------------
 * - Can Hardware unit - One or multiple Can controllers of the same type.
 * - Hrh - HOH with receive definitions
 * - Hth - HOH with transmit definitions
 *
 */

//-------------------------------------------------------------------

#define GET_CONTROLLER_CONFIG(_controller)	\
        					&Can_Global.config->CanConfigSet->CanController[(_controller)]

#define GET_CALLBACKS() \
							(Can_Global.config->CanConfigSet->CanCallbacks)

#define GET_PRIVATE_DATA(_controller) \
									&CanUnit[_controller]

#define GET_CONTROLLER_CNT() (CAN_CONTROLLER_CNT)

//-------------------------------------------------------------------

#if ( CAN_DEV_ERROR_DETECT == STD_ON )
#define VALIDATE(_exp,_api,_err ) \
        if( !(_exp) ) { \
          Det_ReportError(MODULE_ID_CAN,0,_api,_err); \
          return CAN_NOT_OK; \
        }

#define VALIDATE_NO_RV(_exp,_api,_err ) \
        if( !(_exp) ) { \
          Det_ReportError(MODULE_ID_CAN,0,_api,_err); \
          return; \
        }

#define DET_REPORTERROR(_x,_y,_z,_q) Det_ReportError(_x, _y, _z, _q)
#else
#define VALIDATE(_exp,_api,_err )
#define VALIDATE_NO_RV(_exp,_api,_err )
#define DET_REPORTERROR(_x,_y,_z,_q)
#endif

#if defined(USE_DEM)
#define VALIDATE_DEM_NO_RV(_exp,_err ) \
        if( !(_exp) ) { \
          Dem_ReportErrorStatus(_err, DEM_EVENT_STATUS_FAILED); \
          return; \
        }
#else
#define VALIDATE_DEM_NO_RV(_exp,_err )
#endif

//-------------------------------------------------------------------

typedef enum
{
  CAN_UNINIT = 0,
  CAN_READY
} Can_DriverStateType;

// Mapping between HRH and Controller//HOH
typedef struct Can_Arc_ObjectHOHMapStruct
{
  CanControllerIdType CanControllerRef;    // Reference to controller
  const Can_HardwareObjectType* CanHOHRef;       // Reference to HOH.
} Can_Arc_ObjectHOHMapType;

/* Type for holding global information used by the driver */
typedef struct {
  Can_DriverStateType initRun;

  // Our config
  const Can_ConfigType *config;

  // One bit for each channel that is configured.
  // Used to determine if validity of a channel
  // 1 - configured
  // 0 - NOT configured
  uint32  configured;
  // Maps the a channel id to a configured channel id
  uint8   channelMap[CAN_CONTROLLER_CNT];

  // This is a map that maps the HTH:s with the controller and Hoh. It is built
  // during Can_Init and is used to make things faster during a transmit.
  Can_Arc_ObjectHOHMapType CanHTHMap[NUM_OF_HTHS];
} Can_GlobalType;

// Global config
Can_GlobalType Can_Global =
{
  .initRun = CAN_UNINIT,
};

/* Type for holding information about each controller */
typedef struct {
  CanIf_ControllerModeType state;
  uint8		lock_cnt;

  // Statistics
#if (USE_CAN_STATISTICS == STD_ON)
    Can_Arc_StatisticsType stats;
#endif

  // Data stored for Txconfirmation callbacks to CanIf
  PduIdType swPduHandle; //
} Can_UnitType;

Can_UnitType CanUnit[CAN_CONTROLLER_CNT] =
{
  {
    .state = CANIF_CS_UNINIT,
  },{
    .state = CANIF_CS_UNINIT,
  },{
    .state = CANIF_CS_UNINIT,
  },{
    .state = CANIF_CS_UNINIT,
  },{
    .state = CANIF_CS_UNINIT,
  },
};

//-------------------------------------------------------------------

static CAN_HW_t * GetController(int unit)
{
	CAN_HW_t *res = 0;

	if(unit == CAN_CTRL_0)
	{
		res = (CAN_HW_t *)CAN0_BASE;
	}
	else if(unit == CAN_CTRL_1)
	{
		res = (CAN_HW_t *)CAN1_BASE;
	}
	else if(unit == CAN_CTRL_2)
	{
		res = (CAN_HW_t *)CAN2_BASE;
	}
	else if(unit == CAN_CTRL_3)
	{
		res = (CAN_HW_t *)CAN3_BASE;
	}
	else if(unit == CAN_CTRL_4)
	{
		res = (CAN_HW_t *)CAN4_BASE;
	}

	return res;
}

//-------------------------------------------------------------------
/**
 * Function that finds the Hoh( HardwareObjectHandle ) from a Hth
 * A HTH may connect to one or several HOH's. Just find the first one.
 *
 * @param hth The transmit handle
 * @returns Ptr to the Hoh
 */
static const Can_HardwareObjectType * Can_FindHoh( Can_Arc_HTHType hth , uint32* controller)
{
  const Can_HardwareObjectType *hohObj;
  const Can_Arc_ObjectHOHMapType *map;
  const Can_ControllerConfigType *canHwConfig;

  map = &Can_Global.CanHTHMap[hth];

  // Verify that this is the correct map
  if (map->CanHOHRef->CanObjectId != hth)
  {
    DET_REPORTERROR(MODULE_ID_CAN, 0, 0x6, CAN_E_PARAM_HANDLE);
  }

  canHwConfig= GET_CONTROLLER_CONFIG(Can_Global.channelMap[map->CanControllerRef]);

  hohObj = map->CanHOHRef;

  // Verify that this is the correct Hoh type
  if ( hohObj->CanObjectType == CAN_OBJECT_TYPE_TRANSMIT)
  {
    *controller = map->CanControllerRef;
    return hohObj;
  }

  DET_REPORTERROR(MODULE_ID_CAN, 0, 0x6, CAN_E_PARAM_HANDLE);

  return NULL;
}

//-------------------------------------------------------------------

static void Can_RxIsr( int unit );
static void Can_TxIsr( int unit );
static void Can_ErrIsr( int unit );
static void Can_WakeIsr( int unit );

void Can_0_RxIsr( void  ) {	Can_RxIsr(CAN_CTRL_0); }
void Can_1_RxIsr( void  ) {	Can_RxIsr(CAN_CTRL_1); }
void Can_2_RxIsr( void  ) {	Can_RxIsr(CAN_CTRL_2); }
void Can_3_RxIsr( void  ) {	Can_RxIsr(CAN_CTRL_3); }
void Can_4_RxIsr( void  ) {	Can_RxIsr(CAN_CTRL_4); }

void Can_0_TxIsr( void  ) {	Can_TxIsr(CAN_CTRL_0); }
void Can_1_TxIsr( void  ) {	Can_TxIsr(CAN_CTRL_1); }
void Can_2_TxIsr( void  ) {	Can_TxIsr(CAN_CTRL_2); }
void Can_3_TxIsr( void  ) {	Can_TxIsr(CAN_CTRL_3); }
void Can_4_TxIsr( void  ) {	Can_TxIsr(CAN_CTRL_4); }

void Can_0_ErrIsr( void  ) {	Can_ErrIsr(CAN_CTRL_0); }
void Can_1_ErrIsr( void  ) {	Can_ErrIsr(CAN_CTRL_1); }
void Can_2_ErrIsr( void  ) {	Can_ErrIsr(CAN_CTRL_2); }
void Can_3_ErrIsr( void  ) {	Can_ErrIsr(CAN_CTRL_3); }
void Can_4_ErrIsr( void  ) {	Can_ErrIsr(CAN_CTRL_4); }

void Can_0_WakeIsr( void  ) {	Can_WakeIsr(CAN_CTRL_0); }
void Can_1_WakeIsr( void  ) {	Can_WakeIsr(CAN_CTRL_1); }
void Can_2_WakeIsr( void  ) {	Can_WakeIsr(CAN_CTRL_2); }
void Can_3_WakeIsr( void  ) {	Can_WakeIsr(CAN_CTRL_3); }
void Can_4_WakeIsr( void  ) {	Can_WakeIsr(CAN_CTRL_4); }

//-------------------------------------------------------------------

// Uses 25.4.5.1 Transmission Abort Mechanism
static void Can_AbortTx( CAN_HW_t *canHw, Can_UnitType *canUnit ) {
	uint8_t mask = 0;

	// Disable Transmit irq
	canHw->TIER = 0;

	// check if mb's empty
	if((canHw->TFLG & BM_TX0) == 0){mask |= BM_TX0;}
	if((canHw->TFLG & BM_TX1) == 0){mask |= BM_TX1;}
	if((canHw->TFLG & BM_TX2) == 0){mask |= BM_TX2;}

	canHw->TARQ = mask; // Abort all pending mb's

	// Wait for mb's being emptied
	int i=0;
	while(canHw->TFLG != (BM_TX0 | BM_TX1 | BM_TX2)){
		i++;
		if(i > 100)
		{
			break;
		}
	}
}

/**
 * Hardware wake ISR for CAN
 *
 * @param unit CAN controller number( from 0 )
 */
static void Can_WakeIsr( int unit ) {
	if (GET_CALLBACKS()->ControllerWakeup != NULL)
	{
		GET_CALLBACKS()->ControllerWakeup(unit);
	}
	// 269,270,271
	Can_SetControllerMode(unit, CAN_T_STOP);

	// TODO EcuM_CheckWakeup();
}

/**
 * Hardware error ISR for CAN
 *
 * @param unit CAN controller number( from 0 )
 */
static void Can_ErrIsr( int unit ) {
  CAN_HW_t *canHw = GetController(unit);
  Can_UnitType *canUnit = GET_PRIVATE_DATA(unit);
  Can_Arc_ErrorType err;
  err.R = 0;
  uint8_t rflg = canHw->RFLG;

  if((rflg & BM_OVRI) == BM_OVRI){
    err.B.FRMERR = 1;

    // Clear ERRINT
    canHw->RFLG = BM_OVRI;
  }

  if((rflg & BM_CSCI) == BM_CSCI){
    if( ((rflg & (BM_RSTAT0 | BM_RSTAT0)) == (BM_RSTAT0 | BM_RSTAT1)) ||
        ((rflg & (BM_TSTAT0 | BM_TSTAT0)) == (BM_TSTAT0 | BM_TSTAT1)) )
    {
#if (USE_CAN_STATISTICS == STD_ON)
        canUnit->stats.boffCnt++;
#endif
      if (GET_CALLBACKS()->ControllerBusOff != NULL)
      {
        GET_CALLBACKS()->ControllerBusOff(unit);
      }
        Can_SetControllerMode(unit, CAN_T_STOP); // CANIF272

        Can_AbortTx( canHw, canUnit ); // CANIF273
      }

      // Clear ERRINT
      canHw->RFLG = BM_CSCI;
  }

  if (err.R != 0)
  {
    if (GET_CALLBACKS()->Arc_Error != NULL)
    {
      GET_CALLBACKS()->Arc_Error( unit, err );
    }
  }
}

//-------------------------------------------------------------------

/**
 * ISR for CAN. Normal Rx/operation
 *
 * @param unit CAN controller number( from 0 )
 */
static void Can_RxIsr(int unit) {

  CAN_HW_t *canHw= GetController(unit);
  const Can_ControllerConfigType *canHwConfig= GET_CONTROLLER_CONFIG(Can_Global.channelMap[unit]);
#if (USE_CAN_STATISTICS == STD_ON)
  Can_UnitType *canUnit = GET_PRIVATE_DATA(unit);
#endif
  const Can_HardwareObjectType *hohObj;

  // Loop over all the Hoh's
  hohObj= canHwConfig->Can_Arc_Hoh;
  --hohObj;
  do {
    ++hohObj;

    if (hohObj->CanObjectType == CAN_OBJECT_TYPE_RECEIVE)
    {
        Can_IdType id=0;
        IdrType *idr;
        idr = (IdrType *)&canHw->RXFG.idr0;

        // According to autosar MSB shuould be set if extended
        if (idr->Bit.IDE == 1) {
          id = ((uint32)idr->Bit.id28to21 << 21) | ((uint32)idr->Bit.id20to18 << 18) | ((uint32)idr->Bit.id17to15 << 15) |
                ((uint32)idr->Bit.id14to7 << 7) | idr->Bit.id6to0;
          id |= 0x80000000;
        } else {
          id = ((uint32)idr->Bit.id28to21 << 3) | (uint32)idr->Bit.id20to18;
        }

        if (GET_CALLBACKS()->RxIndication != NULL)
        {
          GET_CALLBACKS()->RxIndication(hohObj->CanObjectId,
                                        id,
                                        canHw->RXFG.dlr & 0x0f,
                                        (uint8 *)&canHw->RXFG.ds0 ); // Next layer will copy
        }
#if (USE_CAN_STATISTICS == STD_ON)
        // Increment statistics
        canUnit->stats.rxSuccessCnt++;
#endif

        // Clear interrupt
        canHw->RFLG = BM_RXF;					// clear RX flag
    }
  } while ( !hohObj->Can_Arc_EOL);
}

/**
 * ISR for CAN. Normal Tx operation
 *
 * @param unit CAN controller number( from 0 )
 */
static void Can_TxIsr(int unit) {
  CAN_HW_t *canHw= GetController(unit);
  const Can_ControllerConfigType *canHwConfig= GET_CONTROLLER_CONFIG(Can_Global.channelMap[unit]);
  Can_UnitType *canUnit = GET_PRIVATE_DATA(unit);
  const Can_HardwareObjectType *hohObj;

  // Loop over all the Hoh's
  hohObj= canHwConfig->Can_Arc_Hoh;
  --hohObj;
  do {
    ++hohObj;

    if (hohObj->CanObjectType == CAN_OBJECT_TYPE_TRANSMIT)
    {
        if (GET_CALLBACKS()->TxConfirmation != NULL)
        {
          GET_CALLBACKS()->TxConfirmation(canUnit->swPduHandle);
        }
        canUnit->swPduHandle = 0;  // Is this really necessary ??

        // Disable Tx interrupt
        canHw->TIER = 0;
    }
  } while ( !hohObj->Can_Arc_EOL);
}

//-------------------------------------------------------------------

#define INSTALL_HANDLERS(_can_name,_err,_wake,_rx,_tx) \
  do { \
    ISR_INSTALL_ISR2("Can",_can_name ## _ErrIsr,_err,2,0); \
	ISR_INSTALL_ISR2("Can",_can_name ## _WakeIsr,_wake,2,0); \
	ISR_INSTALL_ISR2("Can",_can_name ## _RxIsr,_rx,2,0); \
	ISR_INSTALL_ISR2("Can",_can_name ## _TxIsr,_tx,2,0); \
  } while(0);

// This initiates ALL can controllers
void Can_Init( const Can_ConfigType *config ) {
  Can_UnitType *canUnit;
  const Can_ControllerConfigType *canHwConfig;
  uint8 ctlrId;

  VALIDATE_NO_RV( (Can_Global.initRun == CAN_UNINIT), 0x0, CAN_E_TRANSITION );
  VALIDATE_NO_RV( (config != NULL ), 0x0, CAN_E_PARAM_POINTER );

  // Save config
  Can_Global.config = config;
  Can_Global.initRun = CAN_READY;

  for (int configId=0; configId < CAN_ARC_CTRL_CONFIG_CNT; configId++) {
    canHwConfig = GET_CONTROLLER_CONFIG(configId);
    ctlrId = canHwConfig->CanControllerId;

    // Assign the configuration channel used later..
    Can_Global.channelMap[canHwConfig->CanControllerId] = configId;
    Can_Global.configured |= (1<<ctlrId);

    canUnit = GET_PRIVATE_DATA(ctlrId);
    canUnit->state = CANIF_CS_STOPPED;

    canUnit->lock_cnt = 0;

    // Clear stats
#if (USE_CAN_STATISTICS == STD_ON)
    memset(&canUnit->stats, 0, sizeof(Can_Arc_StatisticsType));
#endif

    // Note!
    // Could install handlers depending on HW objects to trap more errors
    // in configuration
    if( canHwConfig->CanControllerId  == CAN_CTRL_0) {
      INSTALL_HANDLERS(Can_0, IRQ_TYPE_CAN0_ERR, IRQ_TYPE_CAN0_WAKE, IRQ_TYPE_CAN0_RX, IRQ_TYPE_CAN0_TX);
    }
    else if( canHwConfig->CanControllerId  == CAN_CTRL_1) {
      INSTALL_HANDLERS(Can_1, IRQ_TYPE_CAN1_ERR, IRQ_TYPE_CAN1_WAKE, IRQ_TYPE_CAN1_RX, IRQ_TYPE_CAN1_TX);
    }
    else if( canHwConfig->CanControllerId  == CAN_CTRL_2) {
      INSTALL_HANDLERS(Can_2, IRQ_TYPE_CAN2_ERR, IRQ_TYPE_CAN2_WAKE, IRQ_TYPE_CAN2_RX, IRQ_TYPE_CAN2_TX);
    }
    else if( canHwConfig->CanControllerId  == CAN_CTRL_3) {
      INSTALL_HANDLERS(Can_3, IRQ_TYPE_CAN3_ERR, IRQ_TYPE_CAN3_WAKE, IRQ_TYPE_CAN3_RX, IRQ_TYPE_CAN3_TX);
    }
    else if( canHwConfig->CanControllerId  == CAN_CTRL_4) {
      INSTALL_HANDLERS(Can_4, IRQ_TYPE_CAN4_ERR, IRQ_TYPE_CAN4_WAKE, IRQ_TYPE_CAN4_RX, IRQ_TYPE_CAN4_TX);
    }

    Can_InitController(ctlrId, canHwConfig);

    // Loop through all Hoh:s and map them into the HTHMap
    const Can_HardwareObjectType* hoh;
    hoh = canHwConfig->Can_Arc_Hoh;
    hoh--;
    do
    {
      hoh++;

      if (hoh->CanObjectType == CAN_OBJECT_TYPE_TRANSMIT)
      {
        Can_Global.CanHTHMap[hoh->CanObjectId].CanControllerRef = canHwConfig->CanControllerId;
        Can_Global.CanHTHMap[hoh->CanObjectId].CanHOHRef = hoh;
      }
    } while (!hoh->Can_Arc_EOL);
  }
  return;
}

// Unitialize the module
void Can_DeInit()
{
  Can_UnitType *canUnit;
  const Can_ControllerConfigType *canHwConfig;
  uint32 ctlrId;

  for (int configId=0; configId < CAN_ARC_CTRL_CONFIG_CNT; configId++) {
    canHwConfig = GET_CONTROLLER_CONFIG(configId);
    ctlrId = canHwConfig->CanControllerId;

    canUnit = GET_PRIVATE_DATA(ctlrId);
    canUnit->state = CANIF_CS_UNINIT;

    Can_DisableControllerInterrupts(ctlrId);

    canUnit->lock_cnt = 0;

    // Clear stats
#if (USE_CAN_STATISTICS == STD_ON)
    memset(&canUnit->stats, 0, sizeof(Can_Arc_StatisticsType));
#endif
  }

  Can_Global.config = NULL;
  Can_Global.initRun = CAN_UNINIT;

  return;
}

void Can_InitController( uint8 controller, const Can_ControllerConfigType *config)
{
  CAN_HW_t *canHw;
  uint8_t tq;
  uint8_t tqSync;
  uint8_t tq1;
  uint8_t tq2;
  uint32_t clock;
  Can_UnitType *canUnit;
  uint8 cId = controller;
  const Can_ControllerConfigType *canHwConfig;
  const Can_HardwareObjectType *hohObj;

  VALIDATE_NO_RV( (Can_Global.initRun == CAN_READY), 0x2, CAN_E_UNINIT );
  VALIDATE_NO_RV( (config != NULL ), 0x2,CAN_E_PARAM_POINTER);
  VALIDATE_NO_RV( (controller < GET_CONTROLLER_CNT()), 0x2, CAN_E_PARAM_CONTROLLER );

  canUnit = GET_PRIVATE_DATA(controller);

  VALIDATE_NO_RV( (canUnit->state==CANIF_CS_STOPPED), 0x2, CAN_E_TRANSITION );

  canHw = GetController(cId);
  canHwConfig = GET_CONTROLLER_CONFIG(Can_Global.channelMap[cId]);

  // Start this baby up
  canHw->CTL0 = BM_INITRQ;				// request Init Mode
  while((canHw->CTL1 & BM_INITAK) == 0) ;   // wait until Init Mode is established

  // set CAN enable bit, deactivate listen-only mode,
  // use Bus Clock as clock source and select loop back mode on/off
  canHw->CTL1 = BM_CANE | BM_CLKSRC | (config->Can_Arc_Loopback ? BM_LOOPB : 0x00);

  // acceptance filters
   hohObj = canHwConfig->Can_Arc_Hoh;
   --hohObj;
   do {
     ++hohObj;
     if (hohObj->CanObjectType == CAN_OBJECT_TYPE_RECEIVE)
     {
        canHw->IDAC = hohObj->CanFilterMaskRef->idam;
        canHw->IDAR0 = hohObj->CanFilterMaskRef->idar[0];
        canHw->IDAR1 = hohObj->CanFilterMaskRef->idar[1];
        canHw->IDAR2 = hohObj->CanFilterMaskRef->idar[2];
        canHw->IDAR3 = hohObj->CanFilterMaskRef->idar[3];
        canHw->IDAR4 = hohObj->CanFilterMaskRef->idar[4];
        canHw->IDAR5 = hohObj->CanFilterMaskRef->idar[5];
        canHw->IDAR6 = hohObj->CanFilterMaskRef->idar[6];
        canHw->IDAR7 = hohObj->CanFilterMaskRef->idar[7];
        canHw->IDMR0 = hohObj->CanFilterMaskRef->idmr[0];
        canHw->IDMR1 = hohObj->CanFilterMaskRef->idmr[1];
        canHw->IDMR2 = hohObj->CanFilterMaskRef->idmr[2];
        canHw->IDMR3 = hohObj->CanFilterMaskRef->idmr[3];
        canHw->IDMR4 = hohObj->CanFilterMaskRef->idmr[4];
        canHw->IDMR5 = hohObj->CanFilterMaskRef->idmr[5];
        canHw->IDMR6 = hohObj->CanFilterMaskRef->idmr[6];
        canHw->IDMR7 = hohObj->CanFilterMaskRef->idmr[7];
     }
   }while( !hohObj->Can_Arc_EOL );

  // Clock calucation
  // -------------------------------------------------------------------
  //
  // * 1 TQ = Sclk period( also called SCK )
  // * Ftq = Fcanclk / ( PRESDIV + 1 ) = Sclk
  //   ( Fcanclk can come from crystal or from the peripheral dividers )
  //
  // -->
  // TQ = 1/Ftq = (PRESDIV+1)/Fcanclk --> PRESDIV = (TQ * Fcanclk - 1 )
  // TQ is between 8 and 25
  clock = McuE_GetSystemClock()/2;

  tqSync = config->CanControllerPropSeg + 1;
  tq1 = config->CanControllerSeg1 + 1;
  tq2 = config->CanControllerSeg2 + 1;
  tq = tqSync + tq1 + tq2;

  // Check TQ limitations..
  VALIDATE_DEM_NO_RV(( (tq1>=1) && (tq1<=16)), CAN_E_TIMEOUT );
  VALIDATE_DEM_NO_RV(( (tq2>=1) && (tq2<=8)), CAN_E_TIMEOUT );
  VALIDATE_DEM_NO_RV(( (tq>=3) && (tq<25 )), CAN_E_TIMEOUT );

  canHw->BTR0 = (config->CanControllerPropSeg << 6) | (uint8)(clock/(config->CanControllerBaudRate*1000*tq) - 1); // Prescaler
  canHw->BTR1 = (config->CanControllerSeg2 << 4) | config->CanControllerSeg1;

  canHw->CTL0 &= ~BM_INITRQ;				// exit Init Mode
  while((canHw->CTL1 & BM_INITAK) != 0) ;// wait until Normal Mode is established

  canHw->TBSEL = BM_TX0;					// use (only) TX buffer 0

  canUnit->state = CANIF_CS_STOPPED;
  Can_EnableControllerInterrupts(cId);

  return;
}


Can_ReturnType Can_SetControllerMode( uint8 controller, Can_StateTransitionType transition ) {
  imask_t state;
  CAN_HW_t *canHw;
  Can_ReturnType rv = CAN_OK;
  VALIDATE( (controller < GET_CONTROLLER_CNT()), 0x3, CAN_E_PARAM_CONTROLLER );

  Can_UnitType *canUnit = GET_PRIVATE_DATA(controller);

  VALIDATE( (canUnit->state!=CANIF_CS_UNINIT), 0x3, CAN_E_UNINIT );
  canHw = GetController(controller);

  switch(transition )
  {
  case CAN_T_START:
    canUnit->state = CANIF_CS_STARTED;
    Irq_Save(state);
    if (canUnit->lock_cnt == 0){   // REQ CAN196
      Can_EnableControllerInterrupts(controller);
    }
    Irq_Restore(state);
    break;
  case CAN_T_WAKEUP:
	VALIDATE(canUnit->state == CANIF_CS_SLEEP, 0x3, CAN_E_TRANSITION);
	canHw->CTL0 &= ~BM_SLPRQ; // Clear Sleep request
	canHw->CTL0 &= ~BM_WUPE; // Clear Wake up enable
	canUnit->state = CANIF_CS_STOPPED;
	break;
  case CAN_T_SLEEP:  //CAN258, CAN290
    // Should be reported to DEM but DET is the next best
    VALIDATE(canUnit->state == CANIF_CS_STOPPED, 0x3, CAN_E_TRANSITION);
	canHw->CTL0 |= BM_WUPE; // Set wake up enable
	canHw->CTL0 |= BM_SLPRQ; // Set sleep request
	canHw->RIER |= BM_WUPI; // Enable wake up irq
	canUnit->state = CANIF_CS_SLEEP;
	break;
  case CAN_T_STOP:
    // Stop
    canUnit->state = CANIF_CS_STOPPED;
    Can_AbortTx( canHw, canUnit ); // CANIF282
    break;
  default:
    // Should be reported to DEM but DET is the next best
    VALIDATE(canUnit->state == CANIF_CS_STOPPED, 0x3, CAN_E_TRANSITION);
    break;
  }

  return rv;
}

void Can_DisableControllerInterrupts( uint8 controller )
{
  imask_t state;
  Can_UnitType *canUnit;
  CAN_HW_t *canHw;

  VALIDATE_NO_RV( (controller < GET_CONTROLLER_CNT()), 0x4, CAN_E_PARAM_CONTROLLER );

  canUnit = GET_PRIVATE_DATA(controller);

  VALIDATE_NO_RV( (canUnit->state!=CANIF_CS_UNINIT), 0x4, CAN_E_UNINIT );

  Irq_Save(state);
  if(canUnit->lock_cnt > 0 )
  {
    // Interrupts already disabled
    canUnit->lock_cnt++;
    Irq_Restore(state);
    return;
  }
  canUnit->lock_cnt++;
  Irq_Restore(state);

  /* Don't try to be intelligent, turn everything off */
  canHw = GetController(controller);

  /* Turn off the tx interrupt mailboxes */
  canHw->TIER = 0;

  /* Turn off the bus off/tx warning/rx warning and error and rx  */
  canHw->RIER = 0;
 }

void Can_EnableControllerInterrupts( uint8 controller ) {
  imask_t state;
  Can_UnitType *canUnit;
  CAN_HW_t *canHw;
  const Can_ControllerConfigType *canHwConfig;
  VALIDATE_NO_RV( (controller < GET_CONTROLLER_CNT()), 0x5, CAN_E_PARAM_CONTROLLER );

  canUnit = GET_PRIVATE_DATA(controller);

  VALIDATE_NO_RV( (canUnit->state!=CANIF_CS_UNINIT), 0x5, CAN_E_UNINIT );

  Irq_Save(state);
  if( canUnit->lock_cnt > 1 )
  {
    // IRQ should still be disabled so just decrement counter
    canUnit->lock_cnt--;
    Irq_Restore(state);
    return;
  } else if (canUnit->lock_cnt == 1)
  {
    canUnit->lock_cnt = 0;
  }
  Irq_Restore(state);

  canHw = GetController(controller);

  canHwConfig = GET_CONTROLLER_CONFIG(Can_Global.channelMap[controller]);

  if( canHwConfig->CanRxProcessing == CAN_ARC_PROCESS_TYPE_INTERRUPT ) {
    /* Turn on the rx interrupt */
      canHw->RIER |= BM_RXF;
  }

  // BusOff here represents all errors and warnings
  if( canHwConfig->CanBusOffProcessing == CAN_ARC_PROCESS_TYPE_INTERRUPT ) {
    /* Turn off the bus off/tx warning/rx warning and error and rx  */
    canHw->RIER |= BM_WUPI | BM_CSCI | BM_OVRI | BM_RXF | BM_RSTAT0 | BM_TSTAT0;
  }

  return;
}

static IdrType ConstructIdBytes(Can_IdType id, Can_IdTypeType idType)
{
    IdrType idr;

    idr.R[3] = idr.R[2] = idr.R[1] = idr.R[0] = 0;

    if(idType == CAN_ID_TYPE_EXTENDED) {
      idr.Bit.SRR = 1;
      idr.Bit.RTR = 0;
      idr.Bit.IDE = 1;
      idr.Bit.id28to21 = id>>21;
      idr.Bit.id20to18 = id>>18 & 0x07;
      idr.Bit.id17to15 = id>>15 & 0x07;
      idr.Bit.id14to7  = id>>7;
      idr.Bit.id6to0   = id & 0x7F;
    } else if (idType == CAN_ID_TYPE_STANDARD) {
      idr.R[0] = id>>3;
      idr.R[1] = id<<5 & 0xE0;
    } else {
      // No support for mixed in this processor
      assert(0);
    }

    return idr;
}

Can_ReturnType Can_Write( Can_Arc_HTHType hth, Can_PduType *pduInfo ) {
  Can_ReturnType rv = CAN_OK;
  CAN_HW_t *canHw;
  const Can_HardwareObjectType *hohObj;
  const Can_ControllerConfigType *canHwConfig;
  uint32 controller;
  imask_t state;
  IdrType idr;

  VALIDATE( (Can_Global.initRun == CAN_READY), 0x6, CAN_E_UNINIT );
  VALIDATE( (pduInfo != NULL), 0x6, CAN_E_PARAM_POINTER );
  VALIDATE( (pduInfo->length <= 8), 0x6, CAN_E_PARAM_DLC );
  VALIDATE( (hth < NUM_OF_HTHS ), 0x6, CAN_E_PARAM_HANDLE );

  hohObj = Can_FindHoh(hth, &controller);
  if (hohObj == NULL)
    return CAN_NOT_OK;

  Can_UnitType *canUnit = GET_PRIVATE_DATA(controller);

  canHw = GetController(controller);
  Irq_Save(state);

  // check for any free box
  if((canHw->TFLG & BM_TX0) == BM_TX0) {
    canHw->TBSEL = BM_TX0; // Select mb0

    idr = ConstructIdBytes(pduInfo->id, hohObj->CanIdType);

    canHw->TXFG.idr0 = idr.R[0];
    canHw->TXFG.idr1 = idr.R[1];
    canHw->TXFG.idr2 = idr.R[2];
    canHw->TXFG.idr3 = idr.R[3];

    memcpy((uint8 *)&canHw->TXFG.ds0, pduInfo->sdu, pduInfo->length);
    canHw->TXFG.dlr = pduInfo->length;
    canHw->TXFG.tbpr = 0; // Highest prio

    // Send
    canHw->TFLG = BM_TX0;

    canHwConfig = GET_CONTROLLER_CONFIG(Can_Global.channelMap[controller]);

    if( (canHwConfig->CanTxProcessing == CAN_ARC_PROCESS_TYPE_INTERRUPT) &&
        (canUnit->lock_cnt == 0) ) {
  	  /* Turn on the tx interrupt mailboxes */
      canHw->TIER = BM_TX0; // We only use TX0
    }

    // Increment statistics
#if (USE_CAN_STATISTICS == STD_ON)
    canUnit->stats.txSuccessCnt++;
#endif

    // Store pdu handle in unit to be used by TxConfirmation
    canUnit->swPduHandle = pduInfo->swPduHandle;
  } else {
    rv = CAN_BUSY;
  }
  Irq_Restore(state);

  return rv;
}

void Can_MainFunction_Read( void ) {

	/* NOT SUPPORTED */
}

void Can_MainFunction_BusOff( void ) {
  /* Bus-off polling events */

	/* NOT SUPPORTED */
}

void Can_MainFunction_Wakeup( void ) {
  /* Wakeup polling events */

	/* NOT SUPPORTED */
}


void Can_MainFunction_Write( void ) {
    /* NOT SUPPORTED */
}

void Can_MainFunction_Error( void ) {
    /* NOT SUPPORTED */
}


/**
 * Get send/receive/error statistics for a controller
 *
 * @param controller The controller
 * @param stats Pointer to data to copy statistics to
 */

#if (USE_CAN_STATISTICS == STD_ON)
void Can_Arc_GetStatistics( uint8 controller, Can_Arc_StatisticsType *stats)
{
  Can_UnitType *canUnit = GET_PRIVATE_DATA(controller);
  *stats = canUnit->stats;
}
#endif

#else // Stub all functions for use in simulator environment

#include "debug.h"

void Can_Init( const Can_ConfigType *Config )
{
  // Do initial configuration of layer here
}

void Can_InitController( uint8 controller, const Can_ControllerConfigType *config)
{
	// Do initialisation of controller here.
}

Can_ReturnType Can_SetControllerMode( uint8 Controller, Can_StateTransitionType transition )
{
	// Turn on off controller here depending on transition
	return E_OK;
}

Can_ReturnType Can_Write( Can_Arc_HTHType hth, Can_PduType *pduInfo )
{
	// Write to mailbox on controller here.
	DEBUG(DEBUG_MEDIUM, "Can_Write(stub): Received data ");
	for (int i = 0; i < pduInfo->length; i++) {
		DEBUG(DEBUG_MEDIUM, "%d ", pduInfo->sdu[i]);
	}
	DEBUG(DEBUG_MEDIUM, "\n");

	return E_OK;
}

extern void CanIf_RxIndication(uint8 Hrh, Can_IdType CanId, uint8 CanDlc, const uint8 *CanSduPtr);
Can_ReturnType Can_ReceiveAFrame()
{
	// This function is not part of autosar but needed to feed the stack with data
	// from the mailboxes. Normally this is an interrup but probably not in the PCAN case.
	uint8 CanSduData[] = {1,2,1,0,0,0,0,0};
	CanIf_RxIndication(CAN_HRH_0_1, 3, 8, CanSduData);

	return E_OK;
}

void Can_DisableControllerInterrupts( uint8 controller )
{
}

void Can_EnableControllerInterrupts( uint8 controller )
{
}


// Hth - for Flexcan, the hardware message box number... .We don't care
void Can_Cbk_CheckWakeup( uint8 controller ){}

void Can_MainFunction_Write( void ){}
void Can_MainFunction_Read( void ){}
void Can_MainFunction_Error( void ){}
void Can_MainFunction_BusOff( void ){}
void Can_MainFunction_Wakeup( void ){}

void Can_Arc_GetStatistics( uint8 controller, Can_Arc_StatisticsType * stat){}

#endif


