
#ifndef CAN_H
#define CAN_H

/*
// Include Files
#include "ComStack_Types.h"
#include "Can_Cfg.h"
*/

/*
// Std_Types Version Check
#if (STD_TYPES_AR_MAJOR_VERSION != 2)
    #error Can.h: STD_TYPES_AR_MAJOR_VERSION of Std_Types.h is incompatible.
#endif
#if (STD_TYPES_AR_MINOR_VERSION != 1)
    #error Can.h: STD_TYPES_AR_MINOR_VERSION of Std_Types.h is incompatible.
#endif
*/

/*
// CAN Published Information

#define CAN_VENDOR_ID        44
#define CAN_MODULE_ID       111
#define CAN_AR_MAJOR_VERSION  3
#define CAN_AR_MINOR_VERSION  1
#define CAN_AR_PATCH_VERSION  0
#define CAN_SW_MAJOR_VERSION  1
#define CAN_SW_MINOR_VERSION  0
#define CAN_SW_PATCH_VERSION  0



// CAN Development Error Codes

#define CAN_E_PARAM_POINTER              0x01
#define CAN_E_PARAM_HANDLE               0x02
#define CAN_E_PARAM_DLC                  0x03
#define CAN_E_PARAM_CONTROLLER           0x04
#define CAN_E_UNINIT                     0x05
#define CAN_E_TRANSITION                 0x06
*/


/*
// CAN register definition

typedef volatile struct
{
    uint32   CTL;
    uint32   SR;
    unsigned     : 16;
    unsigned REC :  8;
    unsigned TEC :  8;
    uint32   BTR;
    uint32   IR;
    uint32   TR;
    unsigned : 32;
    uint32   PEC;
    unsigned : 32;
    unsigned : 32;
    unsigned : 32;
    unsigned : 32;
    unsigned : 32;
    unsigned : 32;
    unsigned : 32;
    unsigned : 32;
    unsigned : 32;
    unsigned : 32;
    unsigned : 32;
    unsigned : 32;
    unsigned : 32;
    unsigned : 32;
    unsigned : 32;
    unsigned : 32;
    unsigned : 32;
    unsigned : 32;
    unsigned : 32;
    unsigned : 32;
    unsigned : 32;
    unsigned : 32;
	unsigned : 32;
	unsigned : 32;
    uint32   ABOT;
    uint32   TRX;
    uint32   TRx[4];
    uint32   NDX;
    uint32   NDx[4];
    uint32   IPX;
    uint32   IPx[4];
    uint32   MVX;
    uint32   MVx[4];
    unsigned : 32;
    uint32   IPMx[4];
    unsigned : 32;
    unsigned : 32;
    unsigned : 32;
    unsigned : 32;
    unsigned : 32;
	unsigned : 32;
    struct
    {
        uint32   COM;
        uint32   MASK;
        uint32   ARB;
        uint32   MC;
        uint8    DATx[8];
        unsigned : 32;
        unsigned : 32;
    } IFx[3];
    uint32   IF3UEy[4];
    unsigned : 32;
    unsigned : 32;
    unsigned : 32;
    unsigned : 32;
    unsigned : 32;
    unsigned : 32;
    unsigned : 32;
    unsigned : 32;
    unsigned : 32;
    unsigned : 32;
    unsigned : 32;
    unsigned : 32;
    unsigned : 32;
    unsigned : 32;
    unsigned : 32;
    unsigned : 32;
    unsigned : 32;
    unsigned : 32;
    unsigned : 32;
    unsigned : 32;
    unsigned : 32;
    unsigned : 32;
    unsigned : 32;
    unsigned : 32;
    unsigned : 32;
	unsigned : 32;
	unsigned : 32;
	unsigned : 32;
    uint32   IOTX;
    uint32   IORX;
} Can_RegisterType;

#ifdef CAN_BASE_ADDRESS_0
#define Can0_Base ((Can_RegisterType *)CAN_BASE_ADDRESS_0)
#endif
#ifdef CAN_BASE_ADDRESS_1
#define Can1_Base ((Can_RegisterType *)CAN_BASE_ADDRESS_1)
#endif
*/

/*----------------------------------------------------------------------------*/
/* CAN Type Definitions                                                       */
/*                                                                            */
/* CanTimeRegister: - Bit0...Bit5: BRP (Baud Rate Prescaler)                  */
/*                  - Bit6...Bit7: SJW (Synchronization Jump Width)           */
/*                  - Bit8...Bit11: TSeg1                                     */
/*                  - Bit12...Bit14: TSeg2                                    */
/*                  - Bit16...Bit19: BRPE (Baud Rate Prescaler Extension)     */
/*                                                                            */
/*----------------------------------------------------------------------------*/
//typedef uint32 Can_IdType;

typedef enum
{
    CAN_RECEIVE  = 0x00000000,
    CAN_TRANSMIT = 0x20000000
} Can_ObjectType;

typedef enum
{
    BASIC,
	FULL
} Can_HandleType;

typedef enum
{
	CAN_EXTENDED,
    CAN_MIXED,
    CAN_STANDARD
} Can_IdModeType;




typedef struct
{
    Can_HandleType  HandleType;
    Can_IdModeType  IdType;
    Can_IdType      IdValue;
	uint8           ObjectId;
    Can_ObjectType  Direction;
    uint8           Controller;
    uint32          FilterMaskIndex;
} Can_MessageObjectType;

typedef struct 
{
    uint32 CanTimeRegister;
	uint32 *FilterMaskPtr;
	uint8 CanControllerId;
} Can_ControllerConfigType;

typedef struct
{
    Can_ControllerConfigType *ControllerConfigPtr;
    Can_MessageObjectType       *MessageBoxPtr;
} Can_ConfigType;

/** COPIED FROM ARC **/
typedef enum {
  CAN_ARC_HANDLE_TYPE_BASIC,
  CAN_ARC_HANDLE_TYPE_FULL
} Can_Arc_HohType;

typedef enum {
  CAN_HTH_A_1 = 0,
  CAN_HTH_C_1,
  NUM_OF_HTHS
} Can_Arc_HTHType;

typedef enum {
  CAN_HRH_A_1 = 0,
  CAN_HRH_C_1,
  NUM_OF_HRHS
} Can_Arc_HRHType;

/** Container for callback configuration. */
typedef struct {
  void (*CancelTxConfirmation)( const Can_PduType *);               /**< Not supported. */
  void (*RxIndication)( uint8 ,Can_IdType ,uint8 , const uint8 * ); /**< Called on successful reception of a PDU. */
  void (*ControllerBusOff)(uint8);                                  /**< Called on BusOff. */
  void (*TxConfirmation)(PduIdType);                                /**< Called on successful transmission of a PDU. */
  void (*ControllerWakeup)(uint8);                                  /**< Not supported. */
  void (*Arc_Error)(uint8,Can_Arc_ErrorType);                       /**< Called on HW error. */
} Can_CallbackType;

/** Container for controller parameters. */
typedef struct {
  const Can_ControllerConfigType *CanController;

  // Callbacks( Extension )
  const Can_CallbackType *CanCallbacks;
} Can_ConfigSetType;

/** Available HW controllers. */
typedef enum {
  CAN_CTRL_A = 0,
  CAN_CTRL_B,
  CAN_CTRL_C,
  CAN_CTRL_D,
  CAN_CTRL_E,
  CAN_CTRL_F,
  CAN_CONTROLLER_CNT
}CanControllerIdType;

/** Top level container for parameters. */
extern const Can_ConfigType CanConfigData;
/** For direct access to controller list */
extern const Can_ControllerConfigType CanControllerConfigData[];
/** Container for controller parameters. */
extern const Can_ConfigSetType Can_ConfigSet;

/** END COPIED FROM ARC **/


/*
// CAN Driver Function Prototypes

extern void Can_Init(const Can_ConfigType *Config);
extern void Can_InitController(uint8 Controller, const Can_ControllerConfigType *Config);
extern Can_ReturnType Can_SetControllerMode(uint8 Controller, Can_StateTransitionType Transition);
extern void Can_DisableControllerInterrupts(uint8 Controller);
extern void Can_EnableControllerInterrupts(uint8 Controller);
extern Std_ReturnType Can_Cbk_CheckWakeup(uint8 Controller);
extern Can_ReturnType Can_Write(uint8 Hth, const Can_PduType *PduInfo);

#if(CAN_VERSION_INFO_API == STD_ON)
extern void Can_GetVersionInfo(Std_VersionInfoType *versioninfo);
#endif

extern void Can_MainFunction_Write();
extern void Can_MainFunction_Read();
extern void Can_MainFunction_BusOff();
extern void Can_MainFunction_Wakeup();

extern void Can1_InterruptHandler_0();
extern void Can2_InterruptHandler_0();
*/
#endif /* CAN_H */

/*----------------------------------------------------------------------------*/
