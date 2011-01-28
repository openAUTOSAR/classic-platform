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








#ifndef CAN_H_
#define CAN_H_

#define CAN_VENDOR_ID			    1
#define CAN_MODULE_ID			    MODULE_ID_CAN
#define CAN_AR_MAJOR_VERSION  2
#define CAN_AR_MINOR_VERSION 	2
#define CAN_AR_PATCH_VERSION	2

#define CAN_SW_MAJOR_VERSION	1
#define CAN_SW_MINOR_VERSION 	0
#define CAN_SW_PATCH_VERSION	0

#define CAN_E_PARAM_POINTER     0x01
#define CAN_E_PARAM_HANDLE     0x02
#define CAN_E_PARAM_DLC     	0x03
#define CAN_E_PARAM_CONTROLLER 0x04
// API service used without initialization
#define CAN_E_UNINIT           0x05
// Init transition for current mode
#define CAN_E_TRANSITION       0x06


#include "Std_Types.h"
#include "CanIf_Types.h"
#include "ComStack_Types.h"
#if defined(USE_MCU)
#include "Mcu.h"
#endif


typedef struct {
	uint32 txSuccessCnt;
	uint32 rxSuccessCnt;
	uint32 txErrorCnt;
	uint32 rxErrorCnt;
	uint32 boffCnt;
	uint32 fifoOverflow;
	uint32 fifoWarning;
} Can_Arc_StatisticsType;


// uint16: if only Standard IDs are used
// uint32: if also Extended IDs are used
typedef uint32 Can_IdType;

/* Good things to know
 * L-PDU  - Link PDU
 *
 *
 */

typedef struct Can_PduType_s {
	// the CAN ID, 29 or 11-bit
	Can_IdType 	id;
	// Length, max 8 bytes
	uint8		length;
	// data ptr
	uint8 		*sdu;
	// private data for CanIf,just save and use for callback
	PduIdType   swPduHandle;
} Can_PduType;


typedef enum {
	CAN_T_START,
	CAN_T_STOP,
	CAN_T_SLEEP,
	CAN_T_WAKEUP
} Can_StateTransitionType;

typedef enum {
	CAN_OK,
	CAN_NOT_OK,
	CAN_BUSY
// 	CAN_WAKEUP,		// Removed in 3.0
} Can_ReturnType;

/* Error from  CAN controller */
typedef union {
     volatile uint32_t R;
     struct {
    	 volatile uint32_t:24;
         volatile uint32_t BIT1ERR:1;
         volatile uint32_t BIT0ERR:1;
         volatile uint32_t ACKERR:1;
         volatile uint32_t CRCERR:1;
         volatile uint32_t FRMERR:1;
         volatile uint32_t STFERR:1;
         volatile uint32_t TXWRN:1;
         volatile uint32_t RXWRN:1;
     } B;
 } Can_Arc_ErrorType;

// Each controller has 32 hth's, so the division of 32 will give the
// controller.
#define GET_CANCONTROLLER(a) (a / HTH_DIVIDER)

#include "Can_Cfg.h"

void Can_Init( const Can_ConfigType *Config );
void Can_DeInit(void);

#if ( CAN_VERSION_INFO_API == STD_ON )
#define Can_GetVersionInfo(_vi) STD_GET_VERSION_INFO(_vi,CAN)
#endif

void Can_InitController( uint8 controller, const Can_ControllerConfigType *config);
Can_ReturnType Can_SetControllerMode( uint8 Controller, Can_StateTransitionType transition );
void Can_DisableControllerInterrupts( uint8 controller );
void Can_EnableControllerInterrupts( uint8 controller );
// Hth - for Flexcan, the hardware message box number... .We don't care


Can_ReturnType Can_Write( Can_Arc_HTHType hth, Can_PduType *pduInfo );

void Can_Cbk_CheckWakeup( uint8 controller );
void Can_MainFunction_Write( void );
void Can_MainFunction_Read( void );
void Can_MainFunction_BusOff( void );
void Can_MainFunction_Wakeup( void );

void Can_Arc_GetStatistics( uint8 controller, Can_Arc_StatisticsType * stat);

#endif /*CAN_H_*/
