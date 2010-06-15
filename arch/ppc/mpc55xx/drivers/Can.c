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
#include "mpc55xx.h"
#include "Cpu.h"
#include "Mcu.h"
#include "CanIf_Cbk.h"
#include "Det.h"
#if defined(USE_DEM)
#include "Dem.h"
#endif
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#if defined(USE_KERNEL)
#include "Os.h"
#include "irq.h"
#endif


/* CONFIGURATION NOTES
 * ------------------------------------------------------------------
 * - CanHandleType must be CAN_ARC_HANDLE_TYPE_BASIC
 *   i.e. CanHandleType=CAN_ARC_HANDLE_TYPE_FULL NOT supported
 *   i.e CanIdValue is NOT supported
 * - All CanXXXProcessing must be CAN_ARC_PROCESS_TYPE_INTERRUPT
 *   ie CAN_ARC_PROCESS_TYPE_POLLED not supported
 * - To select the Mailboxes to use in the CAN controller use Can_Arc_MbMask
 * - HOH's for Tx are global and Rx are for each controller
 * - CanControllerTimeQuanta is NOT used. The other CanControllerXXX selects
 *   the proper time-quanta
 * - Can_Arc_MbMask for Tx HOH must NOT overlap Can_Arc_MbMask for Rx.
 * - ONLY global mask is supported( NOT 14,15 and individual )
 * - Numbering the CanObjectId for Tx:
 *     To do this correctly there are a number of things that are good to know
 *     1. HTH's have unique numbers.
 *     2. One HTH/HRH is maped to one HOH
 *     3. The extension Can_Arc_MbMask binds FULL CAN boxes together.
 *
 *     Example:
 *
 *     HTH   B/F C  HOH
 *     ---------------------
 *      0    F   0  0
 *      1    F   0  1
 *      ..
 *      16   B   0  16  |
 *                  17  |    The use of Can_Arc_MbMask=0x000f0000 binds these to HTH 16
 *                  18  |    ( bits 16 to 19 set here )
 *                  19  |
 *           ...
 *      32   ... 1  32
 *
 *
 *        B - BASIC CAN
 *        F - FULL CAN
 *        C - Controller number
 *
 */

/* IMPLEMENTATION NOTES
 * -----------------------------------------------
 * - A HOH us unique for a controller( not a config-set )
 * - Hrh's are numbered for each controller from 0
 * - HOH is numbered for each controller in sequences of 0-31
 *   ( since we have 6 controllers and Hth is only uint8( See Can_Write() proto )
 * - loopback in HW NOT supported
 * - 32 of 64 boxes supported ( limited by Hth type )
  * - Fifo in HW NOT supported
 */

/* ABBREVATIONS
 *  -----------------------------------------------
 * - Can Hardware unit - One or multiple Can controllers of the same type.
 * - Hrh - HOH with receive definitions
 * - Hth - HOH with transmit definitions
 *
 */

/* HW INFO
 * ------------------------------------------------------------------
 *   This controller should really be called FlexCan+ or something because
 *   it's enhanced with:
 *   - A RX Fifo !!!!! ( yep, it's fantastic ;) )
 *   - A better matching process. From 25.4.4
 *   "By programming	more than one MB with the same ID, received messages will
 *    be queued into the MBs. The CPU can examine the time stamp field of the
 *    MBs to determine the order in which the messages arrived."
 *
 *   Soo, now it seems that Freescale have finally done something right.
 */

//-------------------------------------------------------------------

// Number of mailboxes used for each controller ( power of 2 only )
// ( It's NOT supported to set this to 64 )
#define MAX_NUM_OF_MAILBOXES 32

#if defined(CFG_MPC5567)
#define GET_CONTROLLER(_controller) 	\
        					((struct FLEXCAN2_tag *)(0xFFFC0000 + 0x4000*(_controller)))
#else
#define GET_CONTROLLER(_controller) 	\
        					((struct FLEXCAN_tag *)(0xFFFC0000 + 0x4000*(_controller)))
#endif

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

// Message box status defines
#define MB_TX_ONCE		0xc
#define MB_INACTIVE	0x8
#define MB_RX				0x4
#define MB_ABORT			0x9

//-------------------------------------------------------------------
typedef enum
{
  CAN_UNINIT = 0,
  CAN_READY
} Can_DriverStateType;

typedef union {
    vuint32_t R;
    struct {
        vuint32_t:14;
        vuint32_t TWRNINT:1;
        vuint32_t RWRNINT:1;
        vuint32_t BIT1ERR:1;
        vuint32_t BIT0ERR:1;
        vuint32_t ACKERR:1;
        vuint32_t CRCERR:1;
        vuint32_t FRMERR:1;
        vuint32_t STFERR:1;
        vuint32_t TXWRN:1;
        vuint32_t RXWRN:1;
        vuint32_t IDLE:1;
        vuint32_t TXRX:1;
        vuint32_t FLTCONF:2;
          vuint32_t:1;
        vuint32_t BOFFINT:1;
        vuint32_t ERRINT:1;
        vuint32_t WAKINT:1;
    } B;
} ESRType;                  /* Error and Status Register */

#if defined(CFG_MPC5567)
typedef struct FLEXCAN2_tag flexcan_t;
#else
typedef struct FLEXCAN_tag flexcan_t;
#endif

// Mapping between HRH and Controller//HOH
typedef struct Can_Arc_ObjectHOHMapStruct
{
  uint32 HxHRef;    // Reference to HRH or HTH
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
  // Interrupt masks that is for all Mb's in this controller
  uint32 		Can_Arc_RxMbMask;
  uint32 		Can_Arc_TxMbMask;

  // Used at IFLG in controller at startup
  uint32 		iflagStart;

  // Statistics
  Can_Arc_StatisticsType stats;

  // Data stored for Txconfirmation callbacks to CanIf
  PduIdType swPduHandles[MAX_NUM_OF_MAILBOXES];

} Can_UnitType;

#if defined(CFG_MPC5567)
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
#else
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
  },{
    .state = CANIF_CS_UNINIT,
  }
};
#endif

//-------------------------------------------------------------------

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
  if (map->HxHRef != hth)
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

static void Can_Isr( int unit );
static void Can_Err( int unit );
static void Can_BusOff( int unit );

void Can_A_Isr( void  ) {	Can_Isr(CAN_CTRL_A); }
void Can_B_Isr( void  ) {	Can_Isr(CAN_CTRL_B); }
void Can_C_Isr( void  ) {	Can_Isr(CAN_CTRL_C); }
void Can_D_Isr( void  ) {	Can_Isr(CAN_CTRL_D); }
void Can_E_Isr( void  ) {	Can_Isr(CAN_CTRL_E); }
#if defined(CFG_MPC5567)
#else
void Can_F_Isr( void  ) {	Can_Isr(CAN_CTRL_F); }
#endif

void Can_A_Err( void  ) {	Can_Err(CAN_CTRL_A); }
void Can_B_Err( void  ) {	Can_Err(CAN_CTRL_B); }
void Can_C_Err( void  ) {	Can_Err(CAN_CTRL_C); }
void Can_D_Err( void  ) {	Can_Err(CAN_CTRL_D); }
void Can_E_Err( void  ) {	Can_Err(CAN_CTRL_E); }
#if defined(CFG_MPC5567)
#else
void Can_F_Err( void  ) {	Can_Err(CAN_CTRL_F); }
#endif

void Can_A_BusOff( void  ) {	Can_BusOff(CAN_CTRL_A); }
void Can_B_BusOff( void  ) {	Can_BusOff(CAN_CTRL_B); }
void Can_C_BusOff( void  ) {	Can_BusOff(CAN_CTRL_C); }
void Can_D_BusOff( void  ) {	Can_BusOff(CAN_CTRL_D); }
void Can_E_BusOff( void  ) {	Can_BusOff(CAN_CTRL_E); }
#if defined(CFG_MPC5567)
#else
void Can_F_BusOff( void  ) {	Can_BusOff(CAN_CTRL_F); }
#endif
//-------------------------------------------------------------------


/**
 * Hardware error ISR for CAN
 *
 * @param unit CAN controller number( from 0 )
 */

static void Can_Err( int unit ) {
  flexcan_t *canHw = GET_CONTROLLER(unit);
  Can_Arc_ErrorType err;
  ESRType esr;
  err.R = 0;

  esr.R = canHw->ESR.R;

  err.B.ACKERR = esr.B.ACKERR;
  err.B.BIT0ERR = esr.B.BIT0ERR;
  err.B.BIT1ERR = esr.B.BIT1ERR;
  err.B.CRCERR = esr.B.CRCERR;
  err.B.FRMERR = esr.B.FRMERR;
  err.B.STFERR = esr.B.STFERR;
  err.B.RXWRN = esr.B.RXWRN;
  err.B.TXWRN = esr.B.TXWRN;

  if (GET_CALLBACKS()->Arc_Error != NULL)
  {
    GET_CALLBACKS()->Arc_Error(unit, err );
  }
  // Clear ERRINT
  canHw->ESR.B.ERRINT = 1;
}


// Uses 25.4.5.1 Transmission Abort Mechanism
static void Can_AbortTx( flexcan_t *canHw, Can_UnitType *canUnit ) {
  uint32 mbMask;
  uint8 mbNr;

	// Find our Tx boxes.
  mbMask = canUnit->Can_Arc_TxMbMask;

  // Loop over the Mb's set to abort
  for (; mbMask; mbMask&=~(1<<mbNr)) {
    mbNr = ilog2(mbMask);

    canHw->BUF[mbNr].CS.B.CODE = MB_ABORT;

    // Did it take
    if( canHw->BUF[mbNr].CS.B.CODE != MB_ABORT ) {
			// nope..

    	// it's not sent... or being sent.
    	// Just wait for it
      int i = 0;
    	while( canHw->IFRL.R == (1<<mbNr) )
    	{
    	  i++;
    	  if (i > 1000)
    	    break;
    	}
    }
  }

  // Ack tx interrupts
  canHw->IFRL.R = canUnit->Can_Arc_TxMbMask;
  canUnit->iflagStart = canUnit->Can_Arc_TxMbMask;
}

//-------------------------------------------------------------------

/**
 * BussOff ISR for CAN
 *
 * @param unit CAN controller number( from 0 )
 */
static void Can_BusOff( int unit ) {
  flexcan_t *canHw = GET_CONTROLLER(unit);
  Can_UnitType *canUnit = GET_PRIVATE_DATA(unit);
  Can_Arc_ErrorType err;
  err.R = 0;

  if ( canHw->ESR.B.TWRNINT )
  {
    err.B.TXWRN = canHw->ESR.B.TXWRN;
    canUnit->stats.txErrorCnt++;
    canHw->ESR.B.TWRNINT = 1;
  }

  if ( canHw->ESR.B.RWRNINT )
  {
    err.B.RXWRN = canHw->ESR.B.RXWRN;
    canUnit->stats.rxErrorCnt++;
    canHw->ESR.B.RWRNINT = 1;
  }

  if (err.R != 0)
  {
    if (GET_CALLBACKS()->Arc_Error != NULL)
    {
      GET_CALLBACKS()->Arc_Error( unit, err );
    }
  }

  if( canHw->ESR.B.BOFFINT ) {

    canUnit->stats.boffCnt++;
    if (GET_CALLBACKS()->ControllerBusOff != NULL)
    {
      GET_CALLBACKS()->ControllerBusOff(unit);
    }
    Can_SetControllerMode(unit, CAN_T_STOP); // CANIF272

    canHw->ESR.B.BOFFINT = 1;

    Can_AbortTx( canHw, canUnit ); // CANIF273
  }
}

//-------------------------------------------------------------------

/**
 * ISR for CAN. Normal Rx/Tx operation
 *
 * @param unit CAN controller number( from 0 )
 */
static void Can_Isr(int unit) {

  flexcan_t *canHw= GET_CONTROLLER(unit);
  const Can_ControllerConfigType *canHwConfig= GET_CONTROLLER_CONFIG(Can_Global.channelMap[unit]);
  uint32 iFlagLow = canHw->IFRL.R;
  Can_UnitType *canUnit = GET_PRIVATE_DATA(unit);

  // Read interrupt flags to seeTxConfirmation what interrupt triggered the interrupt
  if (iFlagLow & canHw->IMRL.R) {
    // Check FIFO

#if defined(CFG_MPC5516) || defined(CFG_MPC5517)
  	// Note!
  	//   FIFO code NOT tested
    if (canHw->MCR.B.FEN) {

      // Check overflow
      if (iFlagLow & (1<<7)) {
        canUnit->stats.fifoOverflow++;
        canHw->IFRL.B.BUF07I = 1;
      }

      // Check warning
      if (iFlagLow & (1<<6)) {
        canUnit->stats.fifoWarning++;
        canHw->IFRL.B.BUF06I = 1;
      }

      // Pop fifo "realtime"
      while (canHw->IFRL.B.BUF05I) {
        // At
        // TODO MAHI: Must read the entire data-buffer to unlock??
      	if (GET_CALLBACKS()->RxIndication != NULL)
        {
          GET_CALLBACKS()->RxIndication((-1), canHw->BUF[0].ID.B.EXT_ID,
            canHw->BUF[0].CS.B.LENGTH, (uint8 *)&canHw->BUF[0].DATA.W[0] );
        }
        // Clear the interrupt
        canHw->IFRL.B.BUF05I = 1;
      }
    } else {
#endif
      // No FIFO used
      const Can_HardwareObjectType *hohObj;
      uint32 mbMask;
      uint8 mbNr = 0;
      uint32 data;
      Can_IdType id;

      //
      // Loop over all the Hoh's
      //

      // Rx
      hohObj= canHwConfig->Can_Arc_Hoh;
      --hohObj;
      do {
        ++hohObj;

        mbMask = hohObj->Can_Arc_MbMask & iFlagLow;

        if (hohObj->CanObjectType == CAN_OBJECT_TYPE_RECEIVE)
        {
          // Loop over the Mb's for this Hoh
          for (; mbMask; mbMask&=~(1<<mbNr)) {
            mbNr = ilog2(mbMask);

            // Do the necessary dummy reads to keep controller happy
            data = canHw->BUF[mbNr].CS.R;
            data = canHw->BUF[mbNr].DATA.W[0];

            // According to autosar MSB shuould be set if extended
            if (hohObj->CanIdType == CAN_ID_TYPE_EXTENDED) {
              id = canHw->BUF[mbNr].ID.R;
              id |= 0x80000000;
            } else {
              id = canHw->BUF[mbNr].ID.B.STD_ID;
            }

            if (GET_CALLBACKS()->RxIndication != NULL)
            {
              GET_CALLBACKS()->RxIndication(hohObj->CanObjectId,
                                            id,
                                            canHw->BUF[mbNr].CS.B.LENGTH,
                                            (uint8 *)&canHw->BUF[mbNr].DATA.W[0] );
            }
            // Increment statistics
            canUnit->stats.rxSuccessCnt++;

            // unlock MB (dummy read timer)
            canHw->TIMER.R;

            // Clear interrupt
            canHw->IFRL.R = (1<<mbNr);
          }
        }
      } while ( !hohObj->Can_Arc_EOL);

      // Tx
      hohObj= canHwConfig->Can_Arc_Hoh;
      --hohObj;
      do {
        ++hohObj;

        if (hohObj->CanObjectType == CAN_OBJECT_TYPE_TRANSMIT)
        {
          mbMask = hohObj->Can_Arc_MbMask & iFlagLow;

          // Loop over the Mb's for this Hoh
          for (; mbMask; mbMask&=~(1<<mbNr)) {
            mbNr = ilog2(mbMask);

            if (GET_CALLBACKS()->TxConfirmation != NULL)
            {
              GET_CALLBACKS()->TxConfirmation(canUnit->swPduHandles[mbNr]);
            }
            canUnit->swPduHandles[mbNr] = 0;  // Is this really necessary ??

            // Clear interrupt
            canUnit->iflagStart |= (1<<mbNr);
            canHw->IFRL.R = (1<<mbNr);
          }
        }
      } while ( !hohObj->Can_Arc_EOL);
#if defined(CFG_MPC5516) || defined(CFG_MPC5517)
    } // FIFO code
#endif
  } else {
    // Note! Over 32 boxes is not implemented
    // Other reasons that we end up here
    // - Interupt on a masked box
  }

  if (canHwConfig->Can_Arc_Fifo) {
  	/* Note
  	 * NOT tested at all
  	 */
    while (canHw->IFRL.B.BUF05I) {
      // At
      // TODO MAHI: Must read the entire data-buffer to unlock??
      if (GET_CALLBACKS()->RxIndication != NULL)
      {
        GET_CALLBACKS()->RxIndication((-1), canHw->BUF[0].ID.B.EXT_ID,
                   canHw->BUF[0].CS.B.LENGTH, (uint8 *)&canHw->BUF[0].DATA.W[0] );
      }
      // Increment statistics
      canUnit->stats.rxSuccessCnt++;

      // Clear the interrupt
      canHw->IFRL.B.BUF05I = 1;
    }
  }
}

//-------------------------------------------------------------------

#if defined(USE_KERNEL)
#define INSTALL_HANDLERS( _can_name,_boff,_err,_start,_stop) \
  do { \
    TaskType tid; \
    tid = Os_Arc_CreateIsr(_can_name ## _BusOff,2/*prio*/,"Can"); \
    Irq_AttachIsr2(tid,NULL,_boff); \
    tid = Os_Arc_CreateIsr(_can_name ## _Err,2/*prio*/,"Can"); \
    Irq_AttachIsr2(tid,NULL,_err); \
    for(i=_start;i<=_stop;i++) {  \
      tid = Os_Arc_CreateIsr(_can_name ## _Isr,2/*prio*/,"Can"); \
			Irq_AttachIsr2(tid,NULL,i); \
    } \
  } while(0);
#else
#define INSTALL_HANDLERS( _can_name,_boff,_err,_start,_stop) \
  Irq_InstallVector(_can_name ## _BusOff, _boff, 1, CPU_Z1); \
  Irq_InstallVector(_can_name ## _Err, _err, 1, CPU_Z1);    \
  for(i=_start;i<=_stop;i++) {																\
    Irq_InstallVector(_can_name ## _Isr, i, 1, CPU_Z1); \
  }
#endif

// This initiates ALL can controllers
void Can_Init( const Can_ConfigType *config ) {
  Can_UnitType *canUnit;
  const Can_ControllerConfigType *canHwConfig;
  int i;
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
        Can_Global.CanHTHMap[hoh->CanObjectId].HxHRef = hoh->CanObjectId;
      }
    } while (!hoh->Can_Arc_EOL);

    // Note!
    // Could install handlers depending on HW objects to trap more errors
    // in configuration
#if defined(CFG_MPC5567)
    switch( canHwConfig->CanControllerId ) {
    case CAN_CTRL_A:
        INSTALL_HANDLERS(Can_A, FLEXCAN_A_ESR_BOFF_INT, FLEXCAN_A_ESR_ERR_INT, FLEXCAN_A_IFLAG1_BUF0I, FLEXCAN_A_IFLAG1_BUF31_16I);	break;
    case CAN_CTRL_B:
        INSTALL_HANDLERS(Can_B, FLEXCAN_B_ESR_BOFF_INT, FLEXCAN_B_ESR_ERR_INT, FLEXCAN_B_IFLAG1_BUF0I, FLEXCAN_B_IFLAG1_BUF31_16I);	break;
    case CAN_CTRL_C:
        INSTALL_HANDLERS(Can_C, FLEXCAN_C_ESR_BOFF_INT, FLEXCAN_C_ESR_ERR_INT, FLEXCAN_C_IFLAG1_BUF0I, FLEXCAN_C_IFLAG1_BUF31_16I);	break;
    case CAN_CTRL_D:
        INSTALL_HANDLERS(Can_D, FLEXCAN_D_ESR_BOFF_INT, FLEXCAN_D_ESR_ERR_INT, FLEXCAN_D_IFLAG1_BUF0I, FLEXCAN_D_IFLAG1_BUF31_16I);	break;
    case CAN_CTRL_E:
        INSTALL_HANDLERS(Can_E, FLEXCAN_E_ESR_BOFF_INT, FLEXCAN_E_ESR_ERR_INT, FLEXCAN_E_IFLAG1_BUF0I, FLEXCAN_E_IFLAG1_BUF31_16I);	break;
    default:
        assert(0);
    }
#else
    switch( canHwConfig->CanControllerId ) {
    case CAN_CTRL_A:
        INSTALL_HANDLERS(Can_A, FLEXCAN_A_ESR_BOFF_INT, FLEXCAN_A_ESR_ERR_INT, FLEXCAN_A_IFLAG1_BUF0I, FLEXCAN_A_IFLAG1_BUF31_16I);	break;
    case CAN_CTRL_B:
        INSTALL_HANDLERS(Can_B, FLEXCAN_B_ESR_BOFF_INT, FLEXCAN_B_ESR_ERR_INT, FLEXCAN_B_IFLAG1_BUF0I, FLEXCAN_B_IFLAG1_BUF31_16I);	break;
    case CAN_CTRL_C:
        INSTALL_HANDLERS(Can_C, FLEXCAN_C_ESR_BOFF_INT, FLEXCAN_C_ESR_ERR_INT, FLEXCAN_C_IFLAG1_BUF0I, FLEXCAN_C_IFLAG1_BUF31_16I);	break;
    case CAN_CTRL_D:
        INSTALL_HANDLERS(Can_D, FLEXCAN_D_ESR_BOFF_INT, FLEXCAN_D_ESR_ERR_INT, FLEXCAN_D_IFLAG1_BUF0I, FLEXCAN_D_IFLAG1_BUF31_16I);	break;
    case CAN_CTRL_E:
        INSTALL_HANDLERS(Can_E, FLEXCAN_E_ESR_BOFF_INT, FLEXCAN_E_ESR_ERR_INT, FLEXCAN_E_IFLAG1_BUF0I, FLEXCAN_E_IFLAG1_BUF31_16I);	break;
    case CAN_CTRL_F:
        INSTALL_HANDLERS(Can_F, FLEXCAN_F_ESR_BOFF_INT, FLEXCAN_F_ESR_ERR_INT, FLEXCAN_F_IFLAG1_BUF0I, FLEXCAN_F_IFLAG1_BUF31_16I);	break;
    default:
        assert(0);
    }
#endif
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

void Can_InitController( uint8 controller, const Can_ControllerConfigType *config) {

  flexcan_t *canHw;
  uint8_t tq;
  uint8_t tq1;
  uint8_t tq2;
  uint32_t clock;
  int i;
  Can_UnitType *canUnit;
  uint8 cId = controller;
  const Can_ControllerConfigType *canHwConfig;
  const Can_HardwareObjectType *hohObj;

  VALIDATE_NO_RV( (Can_Global.initRun == CAN_READY), 0x2, CAN_E_UNINIT );
  VALIDATE_NO_RV( (config != NULL ), 0x2,CAN_E_PARAM_POINTER);
  VALIDATE_NO_RV( (controller < GET_CONTROLLER_CNT()), 0x2, CAN_E_PARAM_CONTROLLER );

  canUnit = GET_PRIVATE_DATA(controller);

  VALIDATE_NO_RV( (canUnit->state==CANIF_CS_STOPPED), 0x2, CAN_E_TRANSITION );

  canHw = GET_CONTROLLER(cId);
  canHwConfig = GET_CONTROLLER_CONFIG(Can_Global.channelMap[cId]);

  // Start this baby up
  canHw->MCR.B.MDIS = 0;

  // Wait for it to reset
  if( !SIMULATOR() ) {
    // Make a reset so we have a known state
    canHw->MCR.B.SOFTRST = 1;
    while( canHw->MCR.B.SOFTRST == 1);
    // Freeze to write all mem mapped registers ( see 25.4.8.1 )
    canHw->MCR.B.FRZ = 1;
    while( canHw->MCR.B.FRZACK == 0);
  }

#if defined(CFG_MPC5516) || defined(CFG_MPC5517)
  // Note!
  // FIFO implemenation not tested
  if( config->Can_Arc_Fifo ) {
    canHw->MCR.B.FEN = 1;	// Enable FIFO
    canHw->MCR.B.IDAM = 0; 	// We want extended id's to match with
  }
  canHw->MCR.B.BCC = 1; // Enable all nice features
#endif
  /* Use Fsys derivate */
  canHw->CR.B.CLKSRC = 1;
  canHw->MCR.B.MAXMB = MAX_NUM_OF_MAILBOXES - 1;

  /* Disable selfreception */
  canHw->MCR.B.SRXDIS = !config->Can_Arc_Loopback;

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

  // Calculate the number of timequanta's
  // From "Protocol Timing"( chap. 25.4.7.4 )
  tq1 = ( config->CanControllerPropSeg + config->CanControllerSeg1 + 2);
  tq2 = (config->CanControllerSeg2 + 1);
  tq = 1 + tq1 + tq2;

  // Check TQ limitations..
  VALIDATE_DEM_NO_RV(( (tq1>=4) && (tq1<=16)), CAN_E_TIMEOUT );
  VALIDATE_DEM_NO_RV(( (tq2>=2) && (tq2<=8)), CAN_E_TIMEOUT );
  VALIDATE_DEM_NO_RV(( (tq>8) && (tq<25 )), CAN_E_TIMEOUT );

  // Assume we're using the peripheral clock instead of the crystal.
  clock = McuE_GetPeripheralClock(config->CanCpuClockRef);

  canHw->CR.B.PRESDIV = clock/(config->CanControllerBaudRate*1000*tq) - 1;
  canHw->CR.B.PROPSEG = config->CanControllerPropSeg;
  canHw->CR.B.PSEG1 = config->CanControllerSeg1;
  canHw->CR.B.PSEG2 = config->CanControllerSeg2;
  canHw->CR.B.SMP = 	1;	// 3 samples better than 1 ??
  canHw->CR.B.LPB =	config->Can_Arc_Loopback;
  canHw->CR.B.BOFFREC = 1;  // Disable bus off recovery

#if defined(CFG_MPC5516) || defined(CFG_MPC5517)
  // Check if we use individual masks. If so accept anything(=0) for now
  if( canHw->MCR.B.BCC ) {
    i = (config->Can_Arc_Fifo ? 8 : 0 );
    for(;i<63;i++) {
      canHw->RXIMR[i].R = 0;
    }
  }
#endif
#if defined(CFG_MPC5567)
  // Enable individual Rx ID masking and the reception queue features.
  canHw->MCR.B.MBFEN = 1;
#endif
  // Set the id's
  if( config->Can_Arc_Fifo ) {
    // Clear ID's in FIFO also, MUST set extended bit here
    uint32_t *fifoId = (uint32_t*)(((uint8_t *)canHw)+0xe0);
    for(int k=0;k<8;k++) {
      fifoId[k] = 0x40000000; 	// accept extended frames
    }
  }

  // Mark all slots as inactive( depending on fifo )
  i = (config->Can_Arc_Fifo ? 8 : 0 );
  for(; i < 63; i++) {
    //canHw->BUF[i].CS.B.CODE = 0;
    canHw->BUF[i].CS.R = 0;
    canHw->BUF[i].ID.R = 0;
  }

  {
    /* Build a global interrupt/mb mask for all Hoh's */
    uint32 mbMask;
    uint32 mbNr = 0;
    Can_FilterMaskType mask = 0xffffffff;

    // Rx
    hohObj = canHwConfig->Can_Arc_Hoh;
    --hohObj;
    do {
      ++hohObj;

      mbMask = hohObj->Can_Arc_MbMask;
      mbNr = 0;

      if (hohObj->CanObjectType == CAN_OBJECT_TYPE_RECEIVE)
      {
        for(;mbMask;mbMask&=~(1<<mbNr)) {
          mbNr = ilog2(mbMask);
          canHw->BUF[mbNr].CS.B.CODE = MB_RX;
          if ( hohObj->CanIdType == CAN_ID_TYPE_EXTENDED )
          {
            canHw->BUF[mbNr].CS.B.IDE = 1;
#if defined(CFG_MPC5567)
            canHw->RXIMR[mbNr].B.MI = *hohObj->CanFilterMaskRef;
#else
            canHw->BUF[mbNr].ID.R = *hohObj->CanFilterMaskRef; // Write 29-bit MB IDs
#endif
          }
          else
          {
            canHw->BUF[mbNr].CS.B.IDE = 0;
#if defined(CFG_MPC5567)
            canHw->RXIMR[mbNr].B.MI = *hohObj->CanFilterMaskRef;
#else
            canHw->BUF[mbNr].ID.B.STD_ID = *hohObj->CanFilterMaskRef;
#endif
         }
        }

        // Add to global mask
        canUnit->Can_Arc_RxMbMask |= hohObj->Can_Arc_MbMask;
        if( hohObj->CanFilterMaskRef != NULL  ) {
          mask &= *hohObj->CanFilterMaskRef;
        }
      }
      else
      {
        canUnit->Can_Arc_TxMbMask |= hohObj->Can_Arc_MbMask;
      }
    } while( !hohObj->Can_Arc_EOL );

#if defined(CFM_MPC5567)
#else
    // Set global mask
    canHw->RXGMASK.R = mask;
    // Don't use them
    canHw->RX14MASK.R = 0;
    canHw->RX15MASK.R = 0;
#endif
  }

  canUnit->iflagStart = canUnit->Can_Arc_TxMbMask;

  canUnit->state = CANIF_CS_STOPPED;
  Can_EnableControllerInterrupts(cId);

  return;
}


Can_ReturnType Can_SetControllerMode( uint8 controller, Can_StateTransitionType transition ) {
  flexcan_t *canHw;
  Can_ReturnType rv = CAN_OK;
  VALIDATE( (controller < GET_CONTROLLER_CNT()), 0x3, CAN_E_PARAM_CONTROLLER );

  Can_UnitType *canUnit = GET_PRIVATE_DATA(controller);

  VALIDATE( (canUnit->state!=CANIF_CS_UNINIT), 0x3, CAN_E_UNINIT );
  canHw = GET_CONTROLLER(controller);

  switch(transition )
  {
  case CAN_T_START:
    canHw->MCR.B.FRZ = 0;
    canHw->MCR.B.HALT = 0;
    canUnit->state = CANIF_CS_STARTED;
    imask_t state = McuE_EnterCriticalSection();
    if (canUnit->lock_cnt == 0)   // REQ CAN196
      Can_EnableControllerInterrupts(controller);
    McuE_ExitCriticalSection(state);
    break;
  case CAN_T_WAKEUP:  //CAN267
  case CAN_T_SLEEP:  //CAN258, CAN290
    // Should be reported to DEM but DET is the next best
    VALIDATE(canUnit->state == CANIF_CS_STOPPED, 0x3, CAN_E_TRANSITION);
  case CAN_T_STOP:
    // Stop
    canHw->MCR.B.FRZ = 1;
    canHw->MCR.B.HALT = 1;
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
  Can_UnitType *canUnit;
  flexcan_t *canHw;

  VALIDATE_NO_RV( (controller < GET_CONTROLLER_CNT()), 0x4, CAN_E_PARAM_CONTROLLER );

  canUnit = GET_PRIVATE_DATA(controller);

  VALIDATE_NO_RV( (canUnit->state!=CANIF_CS_UNINIT), 0x4, CAN_E_UNINIT );

  imask_t state = McuE_EnterCriticalSection();
  if(canUnit->lock_cnt > 0 )
  {
    // Interrupts already disabled
    canUnit->lock_cnt++;
    McuE_ExitCriticalSection(state);
    return;
  }
  canUnit->lock_cnt++;
  McuE_ExitCriticalSection(state);

  /* Don't try to be intelligent, turn everything off */
  canHw = GET_CONTROLLER(controller);

  /* Turn off the interrupt mailboxes */
  canHw->IMRH.R = 0;
  canHw->IMRL.R = 0;

  /* Turn off the bus off/tx warning/rx warning and error */
  canHw->MCR.B.WRNEN = 0;  	/* Disable warning int */
  canHw->CR.B.ERRMSK = 0;  	/* Disable error interrupt */
  canHw->CR.B.BOFFMSK = 0; 	/* Disable bus-off interrupt */
  canHw->CR.B.TWRNMSK = 0;	/* Disable Tx warning */
  canHw->CR.B.RWRNMSK = 0;	/* Disable Rx warning */
}

void Can_EnableControllerInterrupts( uint8 controller ) {
  Can_UnitType *canUnit;
  flexcan_t *canHw;
  const Can_ControllerConfigType *canHwConfig;
  VALIDATE_NO_RV( (controller < GET_CONTROLLER_CNT()), 0x5, CAN_E_PARAM_CONTROLLER );

  canUnit = GET_PRIVATE_DATA(controller);

  VALIDATE_NO_RV( (canUnit->state!=CANIF_CS_UNINIT), 0x5, CAN_E_UNINIT );

  imask_t state = McuE_EnterCriticalSection();
  if( canUnit->lock_cnt > 1 )
  {
    // IRQ should still be disabled so just decrement counter
    canUnit->lock_cnt--;
    McuE_ExitCriticalSection(state);
    return;
  } else if (canUnit->lock_cnt == 1)
  {
    canUnit->lock_cnt = 0;
  }
  McuE_ExitCriticalSection(state);

  canHw = GET_CONTROLLER(controller);

  canHwConfig = GET_CONTROLLER_CONFIG(Can_Global.channelMap[controller]);

  canHw->IMRH.R = 0;
  canHw->IMRL.R = 0;

  if( canHwConfig->CanRxProcessing == CAN_ARC_PROCESS_TYPE_INTERRUPT ) {
    /* Turn on the interrupt mailboxes */
    canHw->IMRL.R = canUnit->Can_Arc_RxMbMask;
  }

  if( canHwConfig->CanTxProcessing == CAN_ARC_PROCESS_TYPE_INTERRUPT ) {
    /* Turn on the interrupt mailboxes */
    canHw->IMRL.R |= canUnit->Can_Arc_TxMbMask;
  }

  // BusOff here represents all errors and warnings
  if( canHwConfig->CanBusOffProcessing == CAN_ARC_PROCESS_TYPE_INTERRUPT ) {
    canHw->MCR.B.WRNEN = 1;  	/* Turn On warning int */

    canHw->CR.B.ERRMSK = 1;  	/* Enable error interrupt */
    canHw->CR.B.BOFFMSK = 1; 	/* Enable bus-off interrupt */
    canHw->CR.B.TWRNMSK = 1;	/* Enable Tx warning */
    canHw->CR.B.RWRNMSK = 1;	/* Enable Rx warning */
  }

  return;
}

Can_ReturnType Can_Write( Can_Arc_HTHType hth, Can_PduType *pduInfo ) {
  uint16_t timer;
  uint32_t iflag;
  Can_ReturnType rv = CAN_OK;
  uint32_t mbNr;
  flexcan_t *canHw;
  const Can_HardwareObjectType *hohObj;
  uint32 controller;
  uint32 oldMsr;

  VALIDATE( (Can_Global.initRun == CAN_READY), 0x6, CAN_E_UNINIT );
  VALIDATE( (pduInfo != NULL), 0x6, CAN_E_PARAM_POINTER );
  VALIDATE( (pduInfo->length <= 8), 0x6, CAN_E_PARAM_DLC );
  VALIDATE( (hth < NUM_OF_HTHS ), 0x6, CAN_E_PARAM_HANDLE );

  hohObj = Can_FindHoh(hth, &controller);
  if (hohObj == NULL)
    return CAN_NOT_OK;

  Can_UnitType *canUnit = GET_PRIVATE_DATA(controller);

  canHw = GET_CONTROLLER(controller);
  oldMsr = McuE_EnterCriticalSection();
  iflag = canHw->IFRL.R & canUnit->Can_Arc_TxMbMask;

  // check for any free box
  // Normally we would just use the iflag to get the free box
  // but that does not work the first time( iflag == 0 ) so we
  // create one( iflagStart )
  if( iflag | canUnit->iflagStart ) {
    mbNr =  ilog2((iflag | canUnit->iflagStart));	// find mb number
    // clear flag
    canHw->IFRL.R = (1<<mbNr);
    canUnit->iflagStart &= ~(1<<mbNr);

    // Setup message box type
    if( hohObj->CanIdType == CAN_ID_TYPE_EXTENDED ) {
      canHw->BUF[mbNr].CS.B.IDE = 1;
    } else if ( hohObj->CanIdType == CAN_ID_TYPE_STANDARD ) {
      canHw->BUF[mbNr].CS.B.IDE = 0;
    } else {
      // No support for mixed in this processor
      assert(0);
    }

    // Send on buf
    canHw->BUF[mbNr].CS.B.CODE = MB_INACTIVE;	// Hold the transmit buffer inactive
    if( hohObj->CanIdType == CAN_ID_TYPE_EXTENDED ) {
      canHw->BUF[mbNr].ID.R = pduInfo->id; // Write 29-bit MB IDs
    } else {
      assert( !(pduInfo->id & 0xfffff800) );
      canHw->BUF[mbNr].ID.B.STD_ID = pduInfo->id;
    }

#if defined(CFG_MPC5516) || defined(CFG_MPC5517)
    canHw->BUF[mbNr].ID.B.PRIO = 1; 			// Set Local Priority
#endif

    memset(&canHw->BUF[mbNr].DATA, 0, 8);
    memcpy(&canHw->BUF[mbNr].DATA, pduInfo->sdu, pduInfo->length);

    canHw->BUF[mbNr].CS.B.SRR = 1;
    canHw->BUF[mbNr].CS.B.RTR = 0;

    canHw->BUF[mbNr].CS.B.LENGTH = pduInfo->length;
    canHw->BUF[mbNr].CS.B.CODE = MB_TX_ONCE;			// Write tx once code
    timer = canHw->TIMER.R; 						// Unlock Message buffers

    canUnit->stats.txSuccessCnt++;

    // Store pdu handle in unit to be used by TxConfirmation
    canUnit->swPduHandles[mbNr] = pduInfo->swPduHandle;

  } else {
    rv = CAN_BUSY;
  }
  McuE_ExitCriticalSection(oldMsr);

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
	CanIf_RxIndication(CAN_HRH_A_1, 3, 8, CanSduData);

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


