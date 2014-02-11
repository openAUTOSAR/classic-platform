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
#include "stm32f10x.h"
#include "stm32f10x_can.h"
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
 * - loopback in HW NOT supported
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

typedef CAN_TypeDef CAN_HW_t;
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
  uint32		lock_cnt;

  // Statistics
  Can_Arc_StatisticsType stats;

  // Data stored for Txconfirmation callbacks to CanIf
  PduIdType swPduHandle; //
} Can_UnitType;

Can_UnitType CanUnit[CAN_CONTROLLER_CNT] =
{
  {
    .state = CANIF_CS_UNINIT,
  },
  {
    .state = CANIF_CS_UNINIT,
  },
};

//-------------------------------------------------------------------
static CAN_HW_t * GetController(int unit)
{
	return ((CAN_HW_t *)(CAN1_BASE + unit*0x400));
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

void Can_1_RxIsr( void  ) {	Can_RxIsr(CAN_CTRL_1); }
void Can_2_RxIsr( void  ) {	Can_RxIsr(CAN_CTRL_2); }

void Can_1_TxIsr( void  ) {	Can_TxIsr(CAN_CTRL_1); }
void Can_2_TxIsr( void  ) {	Can_TxIsr(CAN_CTRL_2); }

void Can_1_ErrIsr( void  ) {	Can_ErrIsr(CAN_CTRL_1); }
void Can_2_ErrIsr( void  ) {	Can_ErrIsr(CAN_CTRL_2); }


//-------------------------------------------------------------------

// Uses 25.4.5.1 Transmission Abort Mechanism
static void Can_AbortTx( CAN_HW_t *canHw, Can_UnitType *canUnit ) {
	// Disable Transmit irq

	// check if mb's empty

	// Abort all pending mb's

	// Wait for mb's being emptied
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

  // Check wake up
  if(SET == CAN_GetITStatus(canHw, CAN_IT_WKU)){
	  Can_WakeIsr(unit);
	  CAN_ClearITPendingBit(canHw, CAN_IT_WKU);
  }

  if(SET == CAN_GetITStatus(canHw, CAN_IT_BOF)){
	canUnit->stats.boffCnt++;
	if (GET_CALLBACKS()->ControllerBusOff != NULL)
	{
	  GET_CALLBACKS()->ControllerBusOff(unit);
	}
	Can_SetControllerMode(unit, CAN_T_STOP); // CANIF272

	Can_AbortTx( canHw, canUnit ); // CANIF273

	// Clear int
	CAN_ClearITPendingBit(canHw, CAN_IT_BOF);
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
  Can_UnitType *canUnit = GET_PRIVATE_DATA(unit);
  const Can_HardwareObjectType *hohObj;

  CanRxMsg RxMessage;

  RxMessage.StdId=0x00;
  RxMessage.ExtId=0x00;
  RxMessage.IDE=0;
  RxMessage.DLC=0;
  RxMessage.FMI=0;
  RxMessage.Data[0]=0x00;
  RxMessage.Data[1]=0x00;
  CAN_Receive(canHw,CAN_FIFO0, &RxMessage);

  // Loop over all the Hoh's
  hohObj= canHwConfig->Can_Arc_Hoh;
  --hohObj;
  do {
	++hohObj;

	if (hohObj->CanObjectType == CAN_OBJECT_TYPE_RECEIVE)
	{
	    Can_IdType id=0;

	    // According to autosar MSB shuould be set if extended
		if (RxMessage.IDE != CAN_ID_STD) {
		  id = RxMessage.ExtId;
		  id |= 0x80000000;
		} else {
		  id = RxMessage.StdId;
		}

		if (GET_CALLBACKS()->RxIndication != NULL)
		{
		  GET_CALLBACKS()->RxIndication(hohObj->CanObjectId,
										id,
										RxMessage.DLC,
										(uint8 *)&RxMessage.Data[0] ); // Next layer will copy
		}
		// Increment statistics
		canUnit->stats.rxSuccessCnt++;
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

		// Clear Tx interrupt
        CAN_ClearITPendingBit(canHw,CAN_IT_RQCP0);
        CAN_ClearITPendingBit(canHw,CAN_IT_RQCP1);
        CAN_ClearITPendingBit(canHw,CAN_IT_RQCP2);
	}
  } while ( !hohObj->Can_Arc_EOL);
}

//-------------------------------------------------------------------

#define INSTALL_HANDLERS(_can_name,_sce,_rx,_tx) \
  do { \
    ISR_INSTALL_ISR2( "Can_Err", _can_name ## _ErrIsr, _sce, 2, 0 ); \
	ISR_INSTALL_ISR2( "Can_Rx",  _can_name ## _RxIsr,  _rx,  2, 0 ); \
	ISR_INSTALL_ISR2( "Can_Tx",  _can_name ## _TxIsr,  _tx,  2, 0 ); \
  } while(0);

// This initiates ALL can controllers
void Can_Init( const Can_ConfigType *config ) {
  Can_UnitType *canUnit;
  const Can_ControllerConfigType *canHwConfig;
  uint32 ctlrId;

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
    memset(&canUnit->stats, 0, sizeof(Can_Arc_StatisticsType));

    // Note!
    // Could install handlers depending on HW objects to trap more errors
    // in configuration
    switch( canHwConfig->CanControllerId ) {
#ifndef STM32F10X_CL
    case CAN_CTRL_1:
        INSTALL_HANDLERS(Can_1, CAN1_SCE_IRQn, USB_LP_CAN1_RX0_IRQn, USB_HP_CAN1_TX_IRQn);	break;
#else
    case CAN_CTRL_1:
        INSTALL_HANDLERS(Can_1, CAN1_SCE_IRQn, CAN1_RX0_IRQn, CAN1_TX_IRQn);	break;
    case CAN_CTRL_2:
        INSTALL_HANDLERS(Can_2, CAN2_SCE_IRQn, CAN2_RX0_IRQn, CAN2_TX_IRQn);	break;
#endif
        default:
        assert(0);
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
    memset(&canUnit->stats, 0, sizeof(Can_Arc_StatisticsType));
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
  CAN_DeInit(canHw);

  /* CAN filter init. We set up two filters - one for the master (CAN1) and
   * one for the slave (CAN2)
   *
   * CAN_SlaveStartBank(n) denotes which filter is the first of the slave.
   *
   * The filter registers reside in CAN1 and is shared to CAN2, so we only need
   * to set up this once.
   */

  // We let all frames in and do the filtering in software.
  CAN_FilterInitTypeDef  CAN_FilterInitStructure;
  CAN_FilterInitStructure.CAN_FilterMode=CAN_FilterMode_IdMask;
  CAN_FilterInitStructure.CAN_FilterScale=CAN_FilterScale_32bit;
  CAN_FilterInitStructure.CAN_FilterIdHigh=0x0000;
  CAN_FilterInitStructure.CAN_FilterIdLow=0x0000;
  CAN_FilterInitStructure.CAN_FilterMaskIdHigh=0x0000;
  CAN_FilterInitStructure.CAN_FilterMaskIdLow=0x0000;
  CAN_FilterInitStructure.CAN_FilterFIFOAssignment=CAN_FIFO0;
  CAN_FilterInitStructure.CAN_FilterActivation=ENABLE;

  // Init filter 0 (CAN1/master)
  CAN_FilterInitStructure.CAN_FilterNumber=0;
  CAN_FilterInit(&CAN_FilterInitStructure);

  // Init filter 1 (CAN2/slave)
  CAN_FilterInitStructure.CAN_FilterNumber=1;
  CAN_FilterInit(&CAN_FilterInitStructure);

  // Set which filter to use for CAN2.
  CAN_SlaveStartBank(1);

  // acceptance filters
   hohObj = canHwConfig->Can_Arc_Hoh;
   --hohObj;
   do {
	 ++hohObj;
     if (hohObj->CanObjectType == CAN_OBJECT_TYPE_RECEIVE)
     {
    	 // TODO Hw filtering
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

  CAN_InitTypeDef        CAN_InitStructure;
  CAN_StructInit(&CAN_InitStructure);

  /* CAN cell init */
  CAN_InitStructure.CAN_TTCM=DISABLE;
  CAN_InitStructure.CAN_ABOM=ENABLE;
  CAN_InitStructure.CAN_AWUM=ENABLE;
  CAN_InitStructure.CAN_NART=DISABLE;
  CAN_InitStructure.CAN_RFLM=DISABLE;
  CAN_InitStructure.CAN_TXFP=DISABLE;
  if(config->Can_Arc_Loopback){
	  CAN_InitStructure.CAN_Mode=CAN_Mode_LoopBack;
  }else{
	  CAN_InitStructure.CAN_Mode=CAN_Mode_Normal;
  }

  CAN_InitStructure.CAN_SJW=1; //hard coded to value 1 since no configuration is available
  CAN_InitStructure.CAN_BS1=config->CanControllerPropSeg + config->CanControllerSeg1;
  CAN_InitStructure.CAN_BS2=config->CanControllerSeg2;
  CAN_InitStructure.CAN_Prescaler= clock/(config->CanControllerBaudRate*1000*tq);

  if(CANINITOK != CAN_Init(canHw,&CAN_InitStructure))
  {
	return;
  }

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
	CAN_WakeUp(canHw);
	canUnit->state = CANIF_CS_STOPPED;
	break;
  case CAN_T_SLEEP:  //CAN258, CAN290
    // Should be reported to DEM but DET is the next best
    VALIDATE(canUnit->state == CANIF_CS_STOPPED, 0x3, CAN_E_TRANSITION);
    CAN_Sleep(canHw);
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
  CAN_ITConfig(canHw, CAN_IT_TME, DISABLE);

  /* Turn off the bus off/tx warning/rx warning and error and rx  */
  CAN_ITConfig(canHw, CAN_IT_FMP0 | CAN_IT_BOF | CAN_IT_ERR | CAN_IT_WKU, DISABLE);
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
	CAN_ITConfig(canHw, CAN_IT_FMP0, ENABLE);
  }
  if( canHwConfig->CanTxProcessing == CAN_ARC_PROCESS_TYPE_INTERRUPT ) {
	/* Turn on the tx interrupt mailboxes */
  	CAN_ITConfig(canHw, CAN_IT_TME, ENABLE);
  }

  // BusOff here represents all errors and warnings
  if( canHwConfig->CanBusOffProcessing == CAN_ARC_PROCESS_TYPE_INTERRUPT ) {
	/* Turn on the bus off/tx warning/rx warning and error and rx  */
	CAN_ITConfig(canHw, CAN_IT_BOF | CAN_IT_ERR | CAN_IT_WKU, ENABLE);
  }

  return;
}

Can_ReturnType Can_Write( Can_Arc_HTHType hth, Can_PduType *pduInfo ) {
  Can_ReturnType rv = CAN_OK;
  CAN_HW_t *canHw;
  const Can_HardwareObjectType *hohObj;
  const Can_ControllerConfigType *canHwConfig;
  uint32 controller;
  imask_t state;

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

  CanTxMsg TxMessage;

  TxMessage.RTR=CAN_RTR_DATA;
  TxMessage.DLC=pduInfo->length;

  memcpy(TxMessage.Data, pduInfo->sdu, pduInfo->length);

  if (hohObj->CanIdType == CAN_ID_TYPE_EXTENDED) {
	TxMessage.IDE=CAN_ID_EXT;
	TxMessage.ExtId=pduInfo->id;
  } else {
	TxMessage.IDE=CAN_ID_STD;
	TxMessage.StdId=pduInfo->id;
  }

  // check for any free box
  if(CAN_Transmit(canHw,&TxMessage) != CAN_NO_MB) {
    canHwConfig = GET_CONTROLLER_CONFIG(Can_Global.channelMap[controller]);

    if( canHwConfig->CanTxProcessing == CAN_ARC_PROCESS_TYPE_INTERRUPT ) {
  	  /* Turn on the tx interrupt mailboxes */
    	CAN_ITConfig(canHw,CAN_IT_TME, ENABLE);
    }

	// Increment statistics
	canUnit->stats.txSuccessCnt++;

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


/**
 * Get send/receive/error statistics for a controller
 *
 * @param controller The controller
 * @param stats Pointer to data to copy statistics to
 */

void Can_Arc_GetStatistics( uint8 controller, Can_Arc_StatisticsType *stats)
{
  Can_UnitType *canUnit = GET_PRIVATE_DATA(controller);
  *stats = canUnit->stats;
}

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
void Can_MainFunction_BusOff( void ){}
void Can_MainFunction_Wakeup( void ){}

void Can_Arc_GetStatistics( uint8 controller, Can_Arc_StatisticsType * stat){}

#endif


