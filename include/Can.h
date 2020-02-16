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


#ifndef CAN_H_
#define CAN_H_

#include "Modules.h"

#define CAN_VENDOR_ID			    VENDOR_ID_ARCCORE
#define CAN_MODULE_ID			    MODULE_ID_CAN

#define CAN_AR_MAJOR_VERSION    4
#define CAN_AR_MINOR_VERSION    0
#define CAN_AR_PATCH_VERSION    3

#define CAN_SW_MAJOR_VERSION    2
#define CAN_SW_MINOR_VERSION    0
#define CAN_SW_PATCH_VERSION    0

#define CAN_E_PARAM_POINTER     0x01
#define CAN_E_PARAM_HANDLE     0x02
#define CAN_E_PARAM_DLC     	0x03
#define CAN_E_PARAM_CONTROLLER 0x04
// API service used without initialization
#define CAN_E_UNINIT           0x05
// Init transition for current mode
#define CAN_E_TRANSITION       0x06
#define CAN_E_DATALOST         0x07     /** @req 4.0.3/CAN395 */
#define CAN_E_PARAM_BAUDRATE   0x08


/** @name Service id's */
//@{
#define CAN_INIT_SERVICE_ID                         0x00
#define CAN_MAINFUNCTION_WRITE_SERVICE_ID           0x01
#define CAN_INITCONTROLLER_SERVICE_ID               0x02
#define CAN_SETCONTROLLERMODE_SERVICE_ID            0x03
#define CAN_DISABLECONTROLLERINTERRUPTS_SERVICE_ID  0x04
#define CAN_ENABLECONTROLLERINTERRUPTS_SERVICE_ID   0x05
#define CAN_WRITE_SERVICE_ID                        0x06
#define CAN_GETVERSIONINFO_SERVICE_ID               0x07
#define CAN_MAINFUNCTION_READ_SERVICE_ID            0x08
#define CAN_MAINFUNCTION_BUSOFF_SERVICE_ID          0x09
#define CAN_MAINFUNCTION_WAKEUP_SERVICE_ID          0x0a
#define CAN_CBK_CHECKWAKEUP_SERVICE_ID              0x0b
#define CAN_MAIN_FUNCTION_MODE_SERVICE_ID           0x0c
#define CAN_CHANGE_BAUD_RATE_SERVICE_ID             0x0d
#define CAN_CHECK_BAUD_RATE_SERVICE_ID              0x0e
//@}

#if defined(CFG_PPC)

/* HOH flags */
#define CAN_HOH_FIFO_MASK           (1UL<<0)
//#define CAN_HOH_EOL_MASK            (1<<9)

/* Controller flags */
#define CAN_CTRL_RX_PROCESSING_INTERRUPT        (1UL<<0)
#define CAN_CTRL_RX_PROCESSING_POLLING          0
#define CAN_CTRL_TX_PROCESSING_INTERRUPT        (1UL<<1)
#define CAN_CTRL_TX_PROCESSING_POLLING          0
#define CAN_CTRL_WAKEUP_PROCESSING_INTERRUPT    (1UL<<2)
#define CAN_CTRL_WAKEUP_PROCESSING_POLLING      0
#define CAN_CTRL_BUSOFF_PROCESSING_INTERRUPT    (1UL<<3)
#define CAN_CTRL_BUSOFF_PROCESSING_POLLING      0
#define CAN_CTRL_ACTIVATION                     (1UL<<4)

#define CAN_CTRL_LOOPBACK                       (1UL<<5)
#define CAN_CTRL_FIFO                           (1UL<<6)

#define CAN_CTRL_ERROR_PROCESSING_INTERRUPT    (1UL<<7)
#define CAN_CTRL_ERROR_PROCESSING_POLLING      0

#endif

#include "Std_Types.h"
#include "CanIf_Types.h"
#include "ComStack_Types.h"
#include "Mcu.h"


typedef struct {
	uint32 txSuccessCnt;
	uint32 rxSuccessCnt;
	uint32 txErrorCnt;
	uint32 rxErrorCnt;
	uint32 boffCnt;
	uint32 fifoOverflow;
	uint32 fifoWarning;
} Can_Arc_StatisticsType;


#if defined(CFG_CAN_TEST)
typedef struct {
	uint64_t mbMaskTx;
	uint64_t mbMaskRx;
} Can_TestType;
#endif

/** @req 4.0.3/CAN416 */
// uint16: if only Standard IDs are used
// uint32: if also Extended IDs are used
typedef uint32 Can_IdType;

/* Good things to know
 * L-PDU  - Link PDU
 *
 *
 */

/** @req 4.0.3/CAN415 */
typedef struct Can_PduType_s {
	// private data for CanIf,just save and use for callback
	PduIdType   swPduHandle;
	// the CAN ID, 29 or 11-bit
	Can_IdType 	id;
	// Length, max 8 bytes
	uint8		length;
	// data ptr
	uint8 		*sdu;
} Can_PduType;


/** @req 4.0.3/CAN417 */
typedef enum {
	CAN_T_START,
	CAN_T_STOP,
	CAN_T_SLEEP,
	CAN_T_WAKEUP
} Can_StateTransitionType;

/** @req 4.0.3/CAN039 */
typedef enum {
	CAN_OK,
	CAN_NOT_OK,
	CAN_BUSY
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

 /* @req CAN429 */
typedef uint8 Can_HwHandleType;

#if defined(CFG_ARM_V6)
#include "Os.h"

typedef struct {
	Can_IdType 		msgId;
	TaskType		taskId;
	EventMaskType 	eventMask;
} Can_IdTableType;

#endif

#if defined(CFG_PPC)


 typedef enum {
     CAN_ID_TYPE_EXTENDED,
     CAN_ID_TYPE_MIXED,
     CAN_ID_TYPE_STANDARD
 } Can_IdTypeType;

 typedef enum {
      CAN_ARC_HANDLE_TYPE_BASIC,
      CAN_ARC_HANDLE_TYPE_FULL
  } Can_Arc_HohType;


 typedef struct Can_Callback {
     void (*CancelTxConfirmation)( PduIdType, const PduInfoType *);
     void (*RxIndication)( uint8 ,Can_IdType ,uint8 , const uint8 * );
     void (*ControllerBusOff)(uint8);
     void (*TxConfirmation)(PduIdType);
     void (*ControllerWakeup)(uint8);
     void (*Arc_Error)(uint8,Can_Arc_ErrorType);
     void (*ControllerModeIndication)(uint8, CanIf_ControllerModeType);
 } Can_CallbackType;


#include "Can_Cfg.h"


 typedef struct Can_HardwareObjectStruct {
     /* PC/PB, Specifies the type (Full-CAN or Basic-CAN) of a hardware object. */
     Can_Arc_HohType CanHandleType;

     /* PC/PB, Specifies whether the IdValue is of type - standard identifier - extended
      * identifier - mixed mode ImplementationType: Can_IdType */
     Can_IdTypeType CanIdType;

     /* PC/PB Specifies (together with the filter mask) the identifiers range that passes
      *  the hardware filter. */
     uint32 CanIdValue;

     /* PC/PB Holds the handle ID of HRH or HTH. The value of this parameter is unique
      *  in a given CAN Driver, and it should start with 0 and continue without any
      *  gaps. The HRH and HTH Ids are defined under two different name-spaces.
      *  Example: HRH0-0, HRH1-1, HTH0-2, HTH1-3 */
     uint32 CanObjectId;

     /* PC/PB, Specifies if the HardwareObject is used as Transmit or as Receive object */
     Can_ObjectTypeType CanObjectType;

     /* Reference to CAN Controller to which the HOH is associated to.  */
     uint8 Can_ControllerRef;

     /* PC/PB Reference to the filter mask that is used for hardware filtering togerther
      * with the CAN_ID_VALUE */
     Can_FilterMaskType *CanFilterMaskRef;

     /* PC,  See CAN_HOH_XX macros */
     uint32 Can_Arc_Flags;

     /* PC, Number of mailboxes that is owned by this HOH */
//     uint8  ArcCanNumMailboxes;
     uint64  ArcMailboxMask;
 } Can_HardwareObjectType;

 typedef struct {
     // Specifies the buadrate of the controller in kbps.
     uint32 CanControllerBaudRate;

     // Specifies propagation delay in time quantas.
     uint8 CanControllerPropSeg;

     // Specifies phase segment 1 in time quantas.
     uint8 CanControllerSeg1;

     // Specifies phase segment 2 in time quantas.
     uint8 CanControllerSeg2;

     // Specifies the resynchronization jump width in time quantas.
     uint8 CanControllerSyncJumpWidth;
 } CanBaudrateConfType;


typedef struct Can_ControllerConfig {

     bool CanControllerActivation;

     //  This parameter provides the controller ID which is unique in a given CAN
     //  Driver. The value for this parameter starts with 0 and continue without any
     //  gaps.

     CanControllerIdType CanControllerId;

     // Specifies the default baud rate in kbps (from the list CanControllerSupportedBaudrates)
     uint32 CanControllerDefaultBaudrate;

     // Specifies the supported baud rates
     const CanBaudrateConfType * const CanControllerSupportedBaudrates;

     // Specifices the length of the CanControllerSupportedBaudrates list
     uint32 CanControllerSupportedBaudratesCount;

     //  Reference to the CPU clock configuration, which is set in the MCU driver
     //  configuration
     uint32 CanCpuClockRef;

     //  This parameter contains a reference to the Wakeup Source for this
     //  controller as defined in the ECU State Manager. Implementation Type:
     //  reference to EcuM_WakeupSourceType
     uint32 CanWakeupSourceRef;

     /* Flags, See CAN_CTRL_XX macros */
     uint32 Can_Arc_Flags;

     /* Number of FIFO MB in the HOH list */
     uint8 Can_Arc_HohFifoCnt;

     /* Total number of HOHs in Can_Arc_Hoh */
     uint8 Can_Arc_HohCnt;

     // List of Hoh id's that belong to this controller
     const Can_HardwareObjectType  * const Can_Arc_Hoh;

     uint64 Can_Arc_RxMailBoxMask;
     uint64 Can_Arc_TxMailBoxMask;

     const uint8 * const Can_Arc_MailBoxToHrh;

     PduIdType * const Can_Arc_TxPduHandles;

     uint8 Can_Arc_TxMailboxStart;

     uint8 Can_Arc_MailboxMax;

#if 1
    //uint32 flags;
#else
     boolean Can_Arc_Loopback;

     // Set this to use the fifo
     boolean Can_Arc_Fifo;
#endif

 } Can_ControllerConfigType;


#else
#include "Can_Cfg.h"

#endif /* defined(PPC) */

// Each controller has 32 hth's, so the division of 32 will give the
// controller.
#define GET_CANCONTROLLER(a) (a / HTH_DIVIDER)


void Can_Init( const Can_ConfigType *Config );
void Can_DeInit(void);

#if ( CAN_VERSION_INFO_API == STD_ON )
#define Can_GetVersionInfo(_vi) STD_GET_VERSION_INFO(_vi,CAN)
#endif

void Can_InitController( uint8 controller, const Can_ControllerConfigType *config);
Std_ReturnType Can_ChangeBaudrate(uint8 controller, const uint16 baudrate);
Std_ReturnType Can_CheckBaudrate(uint8 controller, const uint16 baudrate);
Can_ReturnType Can_SetControllerMode( uint8 Controller, Can_StateTransitionType transition );
void Can_DisableControllerInterrupts( uint8 controller );
void Can_EnableControllerInterrupts( uint8 controller );
// Hth - for Flexcan, the hardware message box number... .We don't care


Can_ReturnType Can_Write( Can_HwHandleType hth, Can_PduType *pduInfo );

Can_ReturnType Can_CheckWakeup( uint8 controller );
void Can_MainFunction_Write( void );
void Can_MainFunction_Read( void );
void Can_MainFunction_BusOff( void );
void Can_MainFunction_Error( void );
void Can_MainFunction_Wakeup( void );
void Can_MainFunction_Mode( void );

Std_ReturnType Can_SetBaudrate(uint8 Controller, const uint16 BaudRateConfigID);

void Can_Arc_GetStatistics( uint8 controller, Can_Arc_StatisticsType * stat);
#if defined(CFG_CAN_TEST)
Can_TestType *Can_Arc_GetTestInfo( void  );
#endif


#endif /*CAN_H_*/
