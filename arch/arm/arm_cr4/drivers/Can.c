

/*----------------------------------------------------------------------------*/
/* Include Files                                                              */



#include "Can.h"
#include "core_cr4.h"
#if defined(USE_DEM)
#include "Dem.h"
#endif
#include "Det.h"
//#include "Spi.h"
//#include "EcuM_Cbk.h"
#include "CanIf_Cbk.h"
//#include "MemMap.h"
#include "Os.h"
#include "irq.h"



#if !defined(USE_DEM)
#define Dem_ReportErrorStatus(...)
#endif


/*----------------------------------------------------------------------------*/
/* Variable Definition                                                        */

static Can_RegisterType* CanBase[]=
{
    Can0_Base,
#ifdef Can1_Base
    Can1_Base
#endif
};

typedef enum
{
    CAN_UNINIT,
    CAN_READY
} Can_StateType;

typedef enum
{
    INTERRUPT = 0x00000400,
    POLLING   = 0x00000000
} OpMode;

typedef struct
{
    OpMode       BusOff;
    OpMode       RxProc;
    OpMode       TxProc;
    OpMode       WakeupProc;
    uint16       MessageBoxCount;
    uint16       MaxBoxes;
    uint8        FirstHandle;
    uint32       WakeupSrc;
    uint32      *MaskRefPtr;
    Can_PduType *PduPtr;
    uint8       *CancelPtr;
    uint8       *TxPtr;    
} Controller_PreConfigType;


#if(CAN_DEV_ERROR_DETECT == STD_ON)
/* Module is in uninitialized state */
static Can_StateType            ModuleState = CAN_UNINIT;
#endif
static CanIf_ControllerModeType ControllerMode[CAN_CONTROLLER_COUNT];

/* Used to switch between IF1 and IF2 of DCAN */
static uint8 IfRegId = 0;

/* Used to order Data Bytes according to hardware registers in DCAN */
static const uint8 ElementIndex[] = {3, 2, 1, 0, 7, 6, 5, 4};

/* To save pointer to the configuration set */
static const Can_ConfigType *CurConfig;

/* To save the PduData of transmission objects */
static Can_PduType PduInfoArray_0[MAX_MESSAGEBOXES_0];
/* To save pending Cancel Requests of transmission objects */
static uint8 CancelRqstArray_0[MAX_MESSAGEBOXES_0];
/* To save pending Transmit Requests of transmission objects */
static uint8 TxRqstArray_0[MAX_MESSAGEBOXES_0];
/* Array to save Filtermask references of MessageObjects */
static uint32 FilterMaskRef_0[MESSAGEBOX_COUNT_0];

#ifdef CanController_1
static Can_PduType PduInfoArray_1[MAX_MESSAGEBOXES_1];
static uint8 CancelRqstArray_1[MAX_MESSAGEBOXES_1];
static uint8 TxRqstArray_1[MAX_MESSAGEBOXES_1];
static uint32 FilterMaskRef_1[MESSAGEBOX_COUNT_1];
#endif

/* Holds the Controller specific configuration */ 
static Controller_PreConfigType ControllerConfig[] =
{
    {
        BUSOFF_PROCESSING_0,
        RX_PROCESSING_0,
        TX_PROCESSING_0,
        WAKEUP_PROCESSING_0,
        MESSAGEBOX_COUNT_0,
        MAX_MESSAGEBOXES_0,
        FIRST_HANDLE_0,
        WAKEUP_SRC_REF_0,
        FilterMaskRef_0,
        PduInfoArray_0,
        CancelRqstArray_0,
        TxRqstArray_0
    },
#ifdef CanController_1
    {
        BUSOFF_PROCESSING_1,
        RX_PROCESSING_1,
        TX_PROCESSING_1,
        WAKEUP_PROCESSING_1,
        MESSAGEBOX_COUNT_1,
        MAX_MESSAGEBOXES_1,
        FIRST_HANDLE_1,
        WAKEUP_SRC_REF_1,
        FilterMaskRef_1,
        PduInfoArray_1,
        CancelRqstArray_1,
        TxRqstArray_1
    }
#endif
};

/* Shadow Buffer is used for buffering of received data */ 
static uint8 RxShadowBuf[8];

/* Driver must know how often Can_DisableControllerInterrupts() has been called */
static uint32 IntDisableCount[CAN_CONTROLLER_COUNT];


/*----------------------------------------------------------------------------*/
/* Function Name: Can1_InterruptHandler_0                                     */
/* Description:   CAN Node 1 Level 0 Interrupt Service Routine                */
/* Return Value:  None                                                        */
/* Remarks:       -/-                                                         */
/*                                                                            */

//#pragma INTERRUPT(CAN1_INTERRUPTHANDLER_0,IRQ)

void CAN1_INTERRUPTHANDLER_0()
{
    uint32  ErrCounter;
    uint32  MsgNr;
    uint32  MsgId;
    uint8   MsgDlc;
    uint8   DataByteIndex;
    uint8  *SduPtr;

    //Can_DisableControllerInterrupts(0);

    ErrCounter = CAN_TIMEOUT_DURATION;

    uint32 ir = CanBase[0]->IR;

    if(ir == 0x8000)
    {
    	uint32 sr = CanBase[0]->SR;
        /* WakeUp Pending */
        if(sr & 0x00000200) {
            /* Set Init Bit, so that Controller is in Stop state */
            CanBase[0]->CTL |= 0x1;
           // EcuM_CheckWakeUp(ControllerConfig[0].WakeupSrc);

        }
        /* Bus Off */
        if(sr & 0x00000080) {
        	Can_SetControllerMode(0, CAN_T_STOP); // CANIF272
            //CanIf_ControllerBusOff(0); // Not implemented in Arctic Core

        }
    }
    else
    {
        MsgNr = ir;

        /* Read Arbitration, Control and Data Bits and clear IntPnd and NewDat*/
        CanBase[0]->IFx[IfRegId].COM = 0x003F0000 | MsgNr;

        /* Wait until Busy Flag is 0 */
        while(CanBase[0]->IFx[IfRegId].COM & 0x8000)
        {
            ErrCounter--;
            if(ErrCounter == 0)
            {
                Dem_ReportErrorStatus(CAN_E_TIMEOUT, DEM_EVENT_STATUS_FAILED);
                ErrCounter = CAN_TIMEOUT_DURATION;
                return;
            }
        }

        /* Transmit Object */
        if(CanBase[0]->IFx[IfRegId].ARB & 0x20000000)
        {
            /* Reset TxRqst-Array Element */
            TxRqstArray_0[MsgNr - 1] = 0;
            /* A Message was successfully transmitted */
            CanIf_TxConfirmation(PduInfoArray_0[MsgNr - 1].swPduHandle);
        }
        /* Receive Object */
        else
        {
            /* Extended Id */
            if(CanBase[0]->IFx[IfRegId].ARB & 0x40000000)
            {
                /* Bring Id to standardized format (MSB marks extended Id) */
                MsgId = (CanBase[0]->IFx[IfRegId].ARB & 0x1FFFFFFF) | 0x80000000;
            }
            /* Standard Id */
            else
            {
                /* Bring Id to standardized format (MSB marks extended Id) */
                MsgId = (CanBase[0]->IFx[IfRegId].ARB & 0x1FFC0000) >> 18;
            }
            /* DLC (Max 8) */
            MsgDlc = CanBase[0]->IFx[IfRegId].MC & 0x000F;
            if(MsgDlc > 8)
            {
                MsgDlc = 8;
            }
            /* Let SduPtr point to Shadow Buffer */
            SduPtr = RxShadowBuf;

            /* Copy Message Data to Shadow Buffer */
            for(DataByteIndex = 0; DataByteIndex < MsgDlc; DataByteIndex++)
            {
                RxShadowBuf[DataByteIndex] = CanBase[0]->IFx[IfRegId].DATx[ElementIndex[DataByteIndex]];
            }
            /* Indicate successful Reception */
            CanIf_RxIndication(CurConfig->MessageBoxPtr[MsgNr - 1].ObjectId, MsgId, MsgDlc, SduPtr);
        }
    }
    //Can_EnableControllerInterrupts(0);
}



/*----------------------------------------------------------------------------*/
/* Function Name: Can_Init                                                    */
/* Service Id:    0x00                                                        */
/* Execution:     Synchronous                                                 */
/* Re-entrant:    No                                                          */
/* Description:   Initialize Can Driver                                       */
/* ConfigPtr:     Pointer to initialization data                              */
/* Return Value:  None                                                        */
/* Remarks:                                                                   */
/*                                                                            */
void CAN_INIT(const Can_ConfigType *Config)
{
    uint32                    Controller;
    uint8                     MsgNr;
    uint32                    ErrCounter;
    Can_ControllerConfigType *CurControllerPtr;
    Can_MessageObjectType    *MsgBoxPtr;
    uint32                   *CurFilterMaskPtr;
    uint32                    Eob;

/* DET Error Check */
#if(CAN_DEV_ERROR_DETECT == STD_ON)
    if(ModuleState != CAN_UNINIT)
    {
        Det_ReportError(CAN_MODULE_ID, CAN_INSTANCE, 0, CAN_E_TRANSITION);
        return;
    }
    if(Config == NULL)
    {
        Det_ReportError(CAN_MODULE_ID, CAN_INSTANCE, 0, CAN_E_PARAM_POINTER);
        return;
    }
#endif 
     
    CurConfig        = Config;
    /* Set Pointer to ControllerConfig */
    CurControllerPtr = Config->ControllerConfigPtr;    
    /* Set Pointer to MessageObjects */
    MsgBoxPtr        = CurConfig->MessageBoxPtr;

    for(Controller = 0; Controller < CAN_CONTROLLER_COUNT; Controller++)
    {
        ErrCounter = CAN_TIMEOUT_DURATION;

        /* Init, IE, AutomaticRetransmission, ConfChangeEnable, ABO Off,Parity On, SIE and EIE depending on ControllerConfig */
#if(CAN_WAKEUP_SUPPORT == STD_ON)
        CanBase[Controller]->CTL = 0x02001643 | (ControllerConfig[Controller].WakeupProc >> 8) | (ControllerConfig[Controller].BusOff >> 7);
#else
        CanBase[Controller]->CTL = 0x00001643 | (ControllerConfig[Controller].WakeupProc >> 8) | (ControllerConfig[Controller].BusOff >> 7);
#endif        
        /* LEC 7, TxOk, RxOk, PER */
        CanBase[Controller]->SR  = 0x0000011F;

        /* Test Mode only for Development time: Silent Loopback */
        //CanBase[Controller]->CTL |= 0x00000080;
        //CanBase[Controller]->TR   = 0x00000018;
            
        for(MsgNr = 0; MsgNr < ControllerConfig[Controller].MaxBoxes; MsgNr++)
        {
            /* Initialize the Arrays for Transmit and Cancellation handling */
            *(ControllerConfig[Controller].CancelPtr + MsgNr) = 0;
            *(ControllerConfig[Controller].TxPtr     + MsgNr) = 0;
            
            /* Set the current FilterMaskPointer */
            CurFilterMaskPtr = CurControllerPtr->FilterMaskPtr;
            /* Wait until Busy Flag is 0 */
            while(CanBase[Controller]->IFx[IfRegId].COM & 0x00008000)
            {
                ErrCounter--;
                if(ErrCounter == 0)
                {
                    Dem_ReportErrorStatus(CAN_E_TIMEOUT, DEM_EVENT_STATUS_FAILED);
                    ErrCounter = CAN_TIMEOUT_DURATION;
                    return;
                }
            }
            /* Configure the post-build defined MessageObjects */
            if(MsgNr < ControllerConfig[Controller].MessageBoxCount)
            {   
                /* Check, if the next message object has the same values as the current, and if it is the last one */             
                if((MsgNr     < (ControllerConfig[Controller].MessageBoxCount - 1)) &&
                   (MsgBoxPtr->Direction       == CAN_RECEIVE                     ) &&
                   (MsgBoxPtr->Direction       == (MsgBoxPtr + 1)->Direction      ) &&
                   (MsgBoxPtr->IdType          == (MsgBoxPtr + 1)->IdType         ) &&
                   (MsgBoxPtr->IdValue         == (MsgBoxPtr + 1)->IdValue        ) &&
                   (MsgBoxPtr->FilterMaskIndex == (MsgBoxPtr + 1)->FilterMaskIndex) &&
                   (MsgBoxPtr->HandleType      == (MsgBoxPtr + 1)->HandleType     )
                  )
                {
                    /* EndOfBlock Bit will not be set */
                    Eob = 0x00000000;
                }
                else
                {
                    /* EndOfBlock Bit will be set */
                    Eob = 0x00000080;
                }
                /* DLC=8, Use Mask only for receive, Set RxIE/TxIE depending on pre-config settings, Eob */
                CanBase[Controller]->IFx[IfRegId].MC   = 0x00001008 | ControllerConfig[Controller].RxProc | (ControllerConfig[Controller].TxProc << 1) | Eob & ~(MsgBoxPtr->Direction >> 17);
                
                if(MsgBoxPtr->IdType == CAN_STANDARD)      /* Standard Identifiers */
                {
                    /* Only Standard-Ids are accepted, Set Mask */
                    CanBase[Controller]->IFx[IfRegId].MASK = 0x80000000 | ((*(CurFilterMaskPtr + MsgBoxPtr->FilterMaskIndex)) & 0x1FFFFFFF);
                    /* Message valid, Id, Direction */
                    CanBase[Controller]->IFx[IfRegId].ARB  = 0x80000000 | ((MsgBoxPtr->IdValue & 0x7FF) << 18) | MsgBoxPtr->Direction;
                }
                else if(MsgBoxPtr->IdType == CAN_EXTENDED) /* Extended Identifiers */
                {
                    /* Only Extended-Ids are accepted, Set Mask */
                    CanBase[Controller]->IFx[IfRegId].MASK = 0x80000000 | ((*(CurFilterMaskPtr + MsgBoxPtr->FilterMaskIndex)) & 0x1FFFFFFF);
                    /* Message valid, Id, Direction */
                    CanBase[Controller]->IFx[IfRegId].ARB  = 0xC0000000 | (MsgBoxPtr->IdValue & 0x1FFFFFFF) | MsgBoxPtr->Direction;    
                }
                else /* Mixed Identifiers */
                {
                    /* Standard- and Mixed-Ids are accepted, Set Mask */
                    CanBase[Controller]->IFx[IfRegId].MASK = 0x00000000 | ((*(CurFilterMaskPtr + MsgBoxPtr->FilterMaskIndex)) & 0x1FFFFFF);
                    /* Message valid, Id, Direction */
                    CanBase[Controller]->IFx[IfRegId].ARB  = 0xC0000000 | (MsgBoxPtr->IdValue & 0x1FFFFFF) | MsgBoxPtr->Direction;
                }
                /* Start writing Mask, Arb, Control and Id bits */
                CanBase[Controller]->IFx[IfRegId].COM  = 0x00F80000 | (MsgNr + 1);
                
                /* Save FilterMask reference of actual MessageObject */
                *(ControllerConfig[Controller].MaskRefPtr + MsgNr) = MsgBoxPtr->FilterMaskIndex;

                /* Increment Pointer to next MessageObject */
                MsgBoxPtr++;
            }
            else /* Configure all other MessageObjects to not valid */
            {
                /* Valid = 0 */
                CanBase[Controller]->IFx[IfRegId].ARB = 0x00000000;
                /* Start writing Arbitration Bits */
                CanBase[Controller]->IFx[IfRegId].COM = 0x00A80000 | (MsgNr + 1);
            }
            /* Use IFx[0] and IFx[1] alternating */
            IfRegId ^= 1;
        }
        /* Set Bit Timing Register */
        CanBase[Controller]->BTR  = CurControllerPtr->CanTimeRegister;
        /* Reset CCE Bit */
        CanBase[Controller]->CTL &= ~0x00000040;

        /* Switch Controller pointer to next ConfigSet */
        CurControllerPtr++;

#if(CAN_DEV_ERROR_DETECT == STD_ON)
        /* Switch Controller State to CANIF_CS_STOPPED */
        ControllerMode[Controller] = CANIF_CS_STOPPED;
#endif
    }
#if(CAN_DEV_ERROR_DETECT == STD_ON)
    /* Switch Module State to CAN_READY */
    ModuleState = CAN_READY;
#endif


    // ARC INSTALL HANDLERS
    TaskType tid;
    tid = Os_Arc_CreateIsr(CAN1_INTERRUPTHANDLER_0, 2 ,"Can0Level0");
    Irq_AttachIsr2(tid, NULL, 16);

    tid = Os_Arc_CreateIsr(CAN1_INTERRUPTHANDLER_0, 2, "Can0Level1");
    Irq_AttachIsr2(tid, NULL, 29);

}



/*----------------------------------------------------------------------------*/
/* Function Name: Can_InitController                                          */
/* Service Id:    0x02                                                        */
/* Execution:     Synchronous                                                 */
/* Re-entrant:    No                                                          */
/* Description:   Initializes only CAN cantroller specific settings           */
/* Controller:    CAN controller to be initialized                            */
/* Config:        Pointer to controller configuration                         */
/* Return Value:  None                                                        */
/* Remarks:       -/-                                                         */
/*                                                                            */

void CAN_INITCONTROLLER(uint8 Controller, const Can_ControllerConfigType* Config)
{
    uint32 *CurMaskRef;
    uint8   MsgNr;
    uint32  ErrCounter;

#if(CAN_DEV_ERROR_DETECT == STD_ON)
    if(Config == NULL)
    {
        Det_ReportError(CAN_MODULE_ID, CAN_INSTANCE, 2, CAN_E_PARAM_POINTER);
        return;
    }
    if(ModuleState == CAN_UNINIT)
    {
        Det_ReportError(CAN_MODULE_ID, CAN_INSTANCE, 2, CAN_E_UNINIT);
        return;
    }
    if(Controller >= CAN_CONTROLLER_COUNT)
    {
        Det_ReportError(CAN_MODULE_ID, CAN_INSTANCE, 2, CAN_E_PARAM_CONTROLLER);
        return;
    }
    if(ControllerMode[Controller] != CANIF_CS_STOPPED)
    {
        Det_ReportError(CAN_MODULE_ID, CAN_INSTANCE, 2, CAN_E_TRANSITION);
        return;
    }
#endif 

    CurMaskRef = ControllerConfig[Controller].MaskRefPtr;
    ErrCounter = CAN_TIMEOUT_DURATION;
    
    for(MsgNr = 0; MsgNr < ControllerConfig[Controller].MessageBoxCount; MsgNr++)
    {
        /* Wait until Busy Flag is 0 */
        while(CanBase[Controller]->IFx[IfRegId].COM & 0x00008000)
        {
            ErrCounter--;
            if(ErrCounter == 0)
            {
                Dem_ReportErrorStatus(CAN_E_TIMEOUT, DEM_EVENT_STATUS_FAILED);
                ErrCounter = CAN_TIMEOUT_DURATION;
                return;
            }
        }
        /* Read actual MaskRegister value of MessageObject */
        CanBase[Controller]->IFx[IfRegId].COM = 0x004C0000 | (MsgNr + 1);

        /* Wait until Busy Flag is 0 */
        while(CanBase[Controller]->IFx[IfRegId].COM & 0x00008000)
        {
            ErrCounter--;
            if(ErrCounter == 0)
            {
                Dem_ReportErrorStatus(CAN_E_TIMEOUT, DEM_EVENT_STATUS_FAILED);
                ErrCounter = CAN_TIMEOUT_DURATION;
                return;
            }
        }
        CanBase[Controller]->IFx[IfRegId].MASK &= 0xD0000000;
        /* Set new Mask */
        CanBase[Controller]->IFx[IfRegId].MASK |= *(Config->FilterMaskPtr + *CurMaskRef) & 0x1FFFFFFF;
        /* Write new Mask to MaskRegister */
        CanBase[Controller]->IFx[IfRegId].COM   = 0x00C80000 | (MsgNr + 1);       
        
        CurMaskRef++;

        IfRegId ^= 1;
    }
    /* Wait until Busy Flag is 0 */
    while(CanBase[Controller]->IFx[IfRegId].COM & 0x00008000)
    {
        ErrCounter--;
        if(ErrCounter == 0)
        {
            Dem_ReportErrorStatus(CAN_E_TIMEOUT, DEM_EVENT_STATUS_FAILED);
            ErrCounter = CAN_TIMEOUT_DURATION;
            return;
        }
    }   
    /* Set CCE Bit to allow access to BitTiming Register (Init already set, in mode "stopped") */
    CanBase[Controller]->CTL |= 0x00000040;
    /* Set Bit Timing Register */
    CanBase[Controller]->BTR  = Config->CanTimeRegister;
    /* Clear CCE Bit */
    CanBase[Controller]->CTL &= ~0x00000040;

}


/*----------------------------------------------------------------------------*/
/* Function Name: Can_SetControllerMode                                       */
/* Service Id:    0x03                                                        */
/* Execution:     Asynchronous                                                */
/* Re-entrant:    No                                                          */
/* Descrpition:   Performs software triggered state transitions               */
/* Controller:    CAN controller for which the status shall be changed        */
/* Transition:    Transition to be done                                        */
/* Return Value:  - CAN_OK: transition initiated                              */
/*                - CAN_NOT_OK: error or wakeup during transition to 'sleep'  */
/* Remarks:       -/-                                                         */
/*                                                                            */

Can_ReturnType Can_SetControllerMode(uint8 Controller, Can_StateTransitionType Transition)
{
    Can_ReturnType Status     = CAN_OK;
    uint32         ErrCounter = CAN_TIMEOUT_DURATION;
    uint32         RegBuf;
    
#if(CAN_DEV_ERROR_DETECT == STD_ON)    
    if(ModuleState == CAN_UNINIT)
    {
        Det_ReportError(CAN_MODULE_ID, CAN_INSTANCE, 3, CAN_E_UNINIT);
        return CAN_NOT_OK;
    }
    if(Controller >= CAN_CONTROLLER_COUNT)
    {
        Det_ReportError(CAN_MODULE_ID, CAN_INSTANCE, 3, CAN_E_PARAM_CONTROLLER);
        return CAN_NOT_OK;
    }
    if(((Transition == CAN_T_START ) && (ControllerMode[Controller] != CANIF_CS_STOPPED)) ||
       ((Transition == CAN_T_STOP  ) && (ControllerMode[Controller] != CANIF_CS_STARTED)) ||
       ((Transition == CAN_T_SLEEP ) && (ControllerMode[Controller] != CANIF_CS_STOPPED)) ||
       ((Transition == CAN_T_WAKEUP) && (ControllerMode[Controller] != CANIF_CS_SLEEP  )))
    {
        Det_ReportError(CAN_MODULE_ID, CAN_INSTANCE, 3, CAN_E_TRANSITION);
        return CAN_NOT_OK;
    }
#endif 

    switch(Transition)
    {
    case CAN_T_START:
        /* Clear Init Bit */
        CanBase[Controller]->CTL  &= ~0x00000001;
        /* Clear Status Register */
        CanBase[Controller]->SR    = 0x0000011F;

        ControllerMode[Controller] = CANIF_CS_STARTED;
        Can_EnableControllerInterrupts(Controller);
        break;

    case CAN_T_STOP:
        /* Set Init Bit */
        CanBase[Controller]->CTL  |=  0x00000001;
        ControllerMode[Controller] = CANIF_CS_STOPPED;
        Can_DisableControllerInterrupts(Controller);
        break;

    case CAN_T_SLEEP:
        /* Set PDR  Bit */
        CanBase[Controller]->CTL |=  0x01000000;
        /* Save actual Register status */
        RegBuf = CanBase[Controller]->CTL;
        /* Disable Status Interrupts and WUBA */
        CanBase[Controller]->CTL &= ~0x02000004;
        /* Wait until Local Power Down Mode acknowledged */
        while(!(CanBase[Controller]->SR & 0x00000400))
        {
            /* Check if a WakeUp occurs */
            if(CanBase[Controller]->SR & 0x00000200)
            {
                Status = CAN_NOT_OK;
                break;
            }
            ErrCounter--;
            if(ErrCounter == 0)
            {
                Dem_ReportErrorStatus(CAN_E_TIMEOUT, DEM_EVENT_STATUS_FAILED);
                ErrCounter = CAN_TIMEOUT_DURATION;
                Status     = CAN_NOT_OK;
                break;
            }
        }
        /* Reset Control Register */
        CanBase[Controller]->CTL   = RegBuf;
        ControllerMode[Controller] = CANIF_CS_SLEEP;
        break;

    case CAN_T_WAKEUP:
        /* Clear PDR Bit */
        CanBase[Controller]->CTL  &= ~0x01000000;
        ControllerMode[Controller] = CANIF_CS_STOPPED;
        break;

    default:
#if(CAN_DEV_ERROR_DETECT == STD_ON)
        /* Invalid Transition */
        Det_ReportError(CAN_MODULE_ID, CAN_INSTANCE, 3, CAN_E_TRANSITION);
        return CAN_NOT_OK;
#endif
        break;
    }

    return Status;
}


/*----------------------------------------------------------------------------*/
/* Function Name: Can_DisableControllerInterrupts                             */
/* Service Id:    0x04                                                        */
/* Execution:     Synchronous                                                 */
/* Re-entrant:    Yes                                                         */
/* Description:   Disables all interrupts for this controller                 */
/* Controller:    CAN controller for which interrupts shall be disabled       */
/* Return Value:  None                                                        */
/* Remarks:       -/-                                                         */
/*                                                                            */

void Can_DisableControllerInterrupts(uint8 Controller)
{
#if(CAN_DEV_ERROR_DETECT == STD_ON)
    if(ModuleState == CAN_UNINIT)
    {
        Det_ReportError(CAN_MODULE_ID, CAN_INSTANCE, 4, CAN_E_UNINIT);
        return;
    }
    if(Controller >= CAN_CONTROLLER_COUNT)
    {
        Det_ReportError(CAN_MODULE_ID, CAN_INSTANCE, 4, CAN_E_PARAM_CONTROLLER);
        return;
    }
#endif 
    /* Clear IE */
    CanBase[Controller]->CTL &= ~0x00000002;
    /* Increment Disable Counter */
    IntDisableCount[Controller]++;
}


/*----------------------------------------------------------------------------*/
/* Function Name: Can_EnableControllerInterrupts                              */
/* Service Id:    0x05                                                        */
/* Execution:     Synchronous                                                 */
/* Re-entrant:    Yes                                                         */
/* Description:   Enables all allowed interrupts for this controller          */
/* Controller:    CAN controller for which interrupts shall be re-enabled     */
/* Return Value:  None                                                        */
/* Remarks:       -/-                                                         */
/*                                                                            */

void Can_EnableControllerInterrupts(uint8 Controller)
{
#if(CAN_DEV_ERROR_DETECT == STD_ON)
    if(ModuleState == CAN_UNINIT)
    {
        Det_ReportError(CAN_MODULE_ID, CAN_INSTANCE, 5, CAN_E_UNINIT);
        return;
    }
    if(Controller >= CAN_CONTROLLER_COUNT)
    {
        Det_ReportError(CAN_MODULE_ID, CAN_INSTANCE, 5, CAN_E_PARAM_CONTROLLER);
        return;
    }
#endif    
    if(IntDisableCount[Controller] > 0)
    {
        if(IntDisableCount[Controller] == 1)
        {
            /* Set IE */
            CanBase[Controller]->CTL |= 0x00000002;
        }
        IntDisableCount[Controller]--;
    }
}


/*----------------------------------------------------------------------------*/
/* Function Name: Can_Cbk_CheckWakeup                                         */
/* Service Id:    0x0b                                                        */
/* Execution:     Synchronous                                                 */
/* Re-entrant:    No                                                          */
/* Description:   Checks if a wakeup has occurred for the given controller    */
/* Controller:    CAN controller to be checked for wakeup                     */
/* Return Value:  - E_OK: Wakeup was detected for given controller            */
/*                - E_NOT_OK: No Wakeup was detected                          */
/* Remarks:       -/-                                                         */
/*                                                                            */

void Can_Cbk_CheckWakeup(uint8 Controller)
{
	/*
#if(CAN_DEV_ERROR_DETECT == STD_ON)
    if(Controller >= CAN_CONTROLLER_COUNT)
    {
        Det_ReportError(CAN_MODULE_ID, CAN_INSTANCE, 0x0B, CAN_E_PARAM_CONTROLLER);
        return CAN_NOT_OK;
    }
#endif
    // Check WakeUpPending
    if(CanBase[Controller]->SR & 0x00000200)
    {
        return E_OK;
    }
    else
    {
        return E_NOT_OK;
    }
    */
}


/*----------------------------------------------------------------------------*/
/* Function Name: Can_Write                                                   */
/* Service Id:    0x06                                                        */
/* Execution:     Synchronous                                                 */
/* Re-entrant:    Yes (thread-safe)                                           */
/* Description:   --                                                          */
/* Hth:           HW-transmit handle to be used for transmit                  */
/* PduInfo:       Pointer to SDU user memory, DLC and Identifier              */
/* Return Value:  - CAN_OK: Write command has been accepted                   */
/*                - CAN_NOT_OK: Development error occured                     */
/*                - CAN_BUSY: No TX hw buffer available or preemptive call    */ 
/* Remarks:       -/-                                                         */
/*                                                                            */

Can_ReturnType Can_Write(Can_Arc_HTHType Hth, Can_PduType *PduInfo)
{
    uint32                 ErrCounter;
    uint8                  ControllerId;
    uint8                  MsgNr;
    uint32                 CancelId;
    uint8                  CancelNr;  
    uint32                 ArbRegValue;
    uint8                  DataByteIndex;
    uint8                 *CurSduPtr;
    Can_PduType           *CurPduArrayPtr;
    uint8                 *CurCancelRqstPtr;
    uint8                 *CurTxRqstPtr;
    Can_MessageObjectType *CurMsgBoxPtr;

    CurSduPtr       = PduInfo->sdu;
    CurMsgBoxPtr    = CurConfig->MessageBoxPtr + Hth;
    ErrCounter      = CAN_TIMEOUT_DURATION;
    
/* DET Error Check */
#if(CAN_DEV_ERROR_DETECT == STD_ON)
    if(PduInfo == NULL || PduInfo->sdu == NULL)
    {
        Det_ReportError(CAN_MODULE_ID, CAN_INSTANCE, 6, CAN_E_PARAM_POINTER);
        return CAN_NOT_OK;
    }
    if(ModuleState == CAN_UNINIT)
    {
        Det_ReportError(CAN_MODULE_ID, CAN_INSTANCE, 6, CAN_E_UNINIT);
        return CAN_NOT_OK;
    }
    if(PduInfo->length > 8)
    {
        Det_ReportError(CAN_MODULE_ID, CAN_INSTANCE, 6, CAN_E_PARAM_DLC);
        return CAN_NOT_OK;        
    }
    if(CurMsgBoxPtr->Direction != CAN_TRANSMIT)
    {
        Det_ReportError(CAN_MODULE_ID, CAN_INSTANCE, 6, CAN_E_PARAM_HANDLE);
        return CAN_NOT_OK;
    }      
#endif

    ControllerId     = CurMsgBoxPtr->Controller;
    
    MsgNr            = Hth - ControllerConfig[ControllerId].FirstHandle;

    CurPduArrayPtr   = ControllerConfig[ControllerId].PduPtr    + MsgNr;
    CurCancelRqstPtr = ControllerConfig[ControllerId].CancelPtr + MsgNr;
    CurTxRqstPtr     = ControllerConfig[ControllerId].TxPtr     + MsgNr;
    
    /* Bring Id Value to appropriate format and set ArbRegValue */
    if(PduInfo->id & 0x80000000)
    {
        /* MsgVal, Ext, Transmit, Extended Id */ 
        ArbRegValue = 0xD0000000 | (PduInfo->id & 0x1FFFFFFF);
    }
    else
    {
        /* MsgVal, Std, Transmit, Standard Id */ 
        ArbRegValue = 0xA0000000 | ((PduInfo->id & 0x7FF) << 18);
    }

    /* Check if TxRqst Bit of MsgObject is set */
    if(CanBase[ControllerId]->TRx[MsgNr >> 5] & (1 << (MsgNr & 0x1F)))
    {

#if(CAN_MULTIPLEXED_TRANSMISSION == STD_ON)

        CancelId = ArbRegValue & 0x1FFFFFFF;
        CancelNr = MsgNr;

        for(MsgNr = ControllerConfig[ControllerId].MessageBoxCount; MsgNr < ControllerConfig[ControllerId].MaxBoxes; MsgNr++)
        {
            /* Read actual MessageObject status: Data, Arbitration, Control */
            CanBase[ControllerId]->IFx[IfRegId].COM = 0x00330000 | (MsgNr + 1);       
            /* Wait until Busy Flag is 0 */
            while(CanBase[ControllerId]->IFx[IfRegId].COM & 0x00008000)
            {
                ErrCounter--;
                if(ErrCounter == 0)
                {
                    Dem_ReportErrorStatus(CAN_E_TIMEOUT, DEM_EVENT_STATUS_FAILED);
                    ErrCounter = CAN_TIMEOUT_DURATION;
                    return CAN_NOT_OK;
                }
            }

            /* If MessageObject is free, use it */
            if(!(CanBase[ControllerId]->IFx[IfRegId].MC & 0x00000100))
            {
                CurPduArrayPtr   = ControllerConfig[ControllerId].PduPtr + MsgNr;
                break;
            }
            /* Check if Id of MessageObject is higher */
            if((CanBase[ControllerId]->IFx[IfRegId].ARB & 0x1FFFFFFF) > CancelId )
            {
                /* Save the highest ID (low priority) */
                CancelId = CanBase[ControllerId]->IFx[IfRegId].ARB & 0x1FFFFFFF;
                /* and corresponding message number */
                CancelNr = MsgNr;
            }
        }
        if(MsgNr == ControllerConfig[ControllerId].MaxBoxes)
        {
#if(CAN_HW_TRANSMIT_CANCELLATION == STD_ON)
             /* Clear TxRqst */
            CanBase[ControllerId]->IFx[IfRegId].MC  &= ~0x00000100;
            /* Transfer Control bits to MessageObject */
            CanBase[ControllerId]->IFx[IfRegId].COM  =  0x90000000 | (CancelNr + 1);
                        
            if(ControllerConfig[ControllerId].TxProc == INTERRUPT)
            {
                /* Set CurPduPtr to Pdu of object to cancel */             
                CurPduArrayPtr  = ControllerConfig[ControllerId].PduPtr + CancelNr;
                /* Wait until Busy Flag is 0 */
                while(CanBase[ControllerId]->IFx[IfRegId].COM & 0x00008000)
                {
                    ErrCounter--;
                    if(ErrCounter == 0)
                    {
                        Dem_ReportErrorStatus(CAN_E_TIMEOUT, DEM_EVENT_STATUS_FAILED);
                        ErrCounter = CAN_TIMEOUT_DURATION;
                        return CAN_NOT_OK;
                    }
                }
                /* Notification about cancellation of object */
                CanIf_CancelTxConfirmation((const Can_PduType *)CurPduArrayPtr);
            }
            else
            {
                /* Set CancelPointer to Position of cancelled Object */
                CurCancelRqstPtr = ControllerConfig[ControllerId].CancelPtr + CancelNr;
                /* Set Arrayelement to 1 to indicate cancellation request */
                *CurCancelRqstPtr = 1;
            }
#endif
        }
#elif(CAN_HW_TRANSMIT_CANCELLATION == STD_ON)
    
        /* Wait until Busy Flag is 0 */
        while(CanBase[ControllerId]->IFx[IfRegId].COM & 0x00008000)
        {
            ErrCounter--;
            if(ErrCounter == 0)
            {
                Dem_ReportErrorStatus(CAN_E_TIMEOUT, DEM_EVENT_STATUS_FAILED);
                ErrCounter = CAN_TIMEOUT_DURATION;
                return CAN_NOT_OK;
            }
        }

        /* Read actual MessageObject status: Data, Arbitration, Control */
        CanBase[ControllerId]->IFx[IfRegId].COM = 0x00330000 | (MsgNr + 1);

        /* Wait until Busy Flag is 0 */
        while(CanBase[ControllerId]->IFx[IfRegId].COM & 0x00008000)
        {
            ErrCounter--;
            if(ErrCounter == 0)
            {
                Dem_ReportErrorStatus(CAN_E_TIMEOUT, DEM_EVENT_STATUS_FAILED);
                ErrCounter = CAN_TIMEOUT_DURATION;
                return CAN_NOT_OK;
            }
        }  
          
        /* Check if actual message has higher priority */
        if((ArbRegValue & 0x1FFFFFFF) < (CanBase[ControllerId]->IFx.ARB & 0x1FFFFFFF))
        {
            /* Clear TxRqst */
            CanBase[ControllerId]->IFx[IfRegId].MC  &= ~0x00000100;
            /* Transfer Control bits to MessageObject */
            CanBase[ControllerId]->IFx[IfRegId].COM  =  0x90000000 | (MsgNr + 1);
            
            if(ControllerConfig[ControllerId].TxProc == INTERRUPT)
            {
                /* Wait until Busy Flag is 0 */
                while(CanBase[ControllerId]->IFx[IfRegId].COM & 0x00008000)
                {
                    ErrCounter--;
                    if(ErrCounter == 0)
                    {
                        Dem_ReportErrorStatus(CAN_E_TIMEOUT, DEM_EVENT_STATUS_FAILED);
                        ErrCounter = CAN_TIMEOUT_DURATION;
                        return CAN_NOT_OK;
                    }
                } 
                /* Notification about cancellation of object */
                CanIf_CancelTxConfirmation((const Can_PduType *)CurPduArrayPtr);
            }
            else
            {
                /* Set Arrayelement to 1 to indicate cancellation request */
                *CurCancelRqstPtr = 1;
            }
        }
#endif
        return CAN_BUSY;
    }

    /* Wait until Busy Flag is 0 */
    while(CanBase[ControllerId]->IFx[IfRegId].COM & 0x00008000)
    {
        ErrCounter--;
        if(ErrCounter == 0)
        {
            Dem_ReportErrorStatus(CAN_E_TIMEOUT, DEM_EVENT_STATUS_FAILED);
            ErrCounter = CAN_TIMEOUT_DURATION;
            return CAN_NOT_OK;
        }
    }

    /* Set NewDat, TxIE (dep on ControllerConfig), TxRqst, EoB and DLC */
    CanBase[ControllerId]->IFx[IfRegId].MC = 0x00000180 | (0x000F & PduInfo->length) | (ControllerConfig[ControllerId].TxProc << 1);

    /* Set ArbitrationRegister */
    CanBase[ControllerId]->IFx[IfRegId].ARB = ArbRegValue;

    /* Set Databytes */
    for(DataByteIndex = 0; DataByteIndex < PduInfo->length; DataByteIndex++)
    {
        CanBase[ControllerId]->IFx[IfRegId].DATx[ElementIndex[DataByteIndex]] = *CurSduPtr++;
    }

    /* Start transmission to MessageRAM */
    CanBase[ControllerId]->IFx[IfRegId].COM = 0x00BF0000 | (MsgNr + 1);
    
    /* Save the PduInfo in PduArray, so that messages can be identified later */
    *CurPduArrayPtr = *PduInfo;
    /* TxRqstArray-Elements are used to identifiy transmitted objects in polling mode */
    *CurTxRqstPtr   = 1;
    
    IfRegId ^= 1;
       
    return CAN_OK;
}


/*----------------------------------------------------------------------------*/
/* Function Name: Can_MainFunction_Write                                      */
/* Service Id:    0x01                                                        */
/* Timing:        FIXED_CYCLIC                                                */
/* Description:   Polling of TX confirmation and TX cancellation confirmation */
/* Return Value:  None                                                        */
/* Remarks:       -/-                                                         */
/*                                                                            */

void CAN_MAINFUNCTION_WRITE()
{
#if 0
    uint8        Controller;
    uint8        MsgNr;
    Can_PduType *CurPduPtr;

#if(CAN_DEV_ERROR_DETECT == STD_ON)
    if(ModuleState == CAN_UNINIT)
    {
        Det_ReportError(CAN_MODULE_ID, CAN_INSTANCE, 1, CAN_E_UNINIT);
        return;
    }
#endif       
    /* Check all controllers */
    for(Controller = 0; Controller < CAN_CONTROLLER_COUNT; Controller++)
    {
        /* Check all MessageObjects */
        for(MsgNr = 0; MsgNr < ControllerConfig[Controller].MaxBoxes; MsgNr++)
        {
            /* Check if a transmission was initiated for this MessageObject */
            if(*(ControllerConfig[Controller].TxPtr + MsgNr) == 1)
            {
                /* Check if TxRqst Bit has already been reset */
                if(!(CanBase[Controller]->TRx[MsgNr >> 5] & (1 << (MsgNr & 0x1F))))
                {
                    /* Reset swTxRqst */
                    *(ControllerConfig[Controller].TxPtr + MsgNr) = 0;
                    
                    CurPduPtr = ControllerConfig[Controller].PduPtr + MsgNr;
                    /* A Message was successfully transmitted */
                    CanIf_TxConfirmation(CurPduPtr->swPduHandle);
                }
            }
            /* Check if a cancellation was initiated for this MessageObject */
            if(*(ControllerConfig[Controller].CancelPtr + MsgNr) == 1)
            {
                /* Check if TxRqst Bit has already been reset */
                if(!(CanBase[Controller]->TRx[MsgNr >> 5] & (1 << (MsgNr & 0x1F))))
                {
                    /* Reset swCancelRqst */
                    *(ControllerConfig[Controller].CancelPtr + MsgNr) = 0;
                    
                    CurPduPtr = ControllerConfig[Controller].PduPtr + MsgNr;
                    /* A Message was successfully transmitted */
                     CanIf_CancelTxConfirmation(CurPduPtr);     
                }
            }
        }
    }
#endif
}


/*----------------------------------------------------------------------------*/
/* Function Name: Can_MainFunction_Read                                       */
/* Service Id:    0x08                                                        */
/* Timing:        FIXED_CYCLIC                                                */
/* Description:   Polling of RX indications                                   */
/* Return Value:  None                                                        */
/* Remarks:       -/-                                                         */
/*                                                                            */

void CAN_MAINFUNCTION_READ()
{
#if 0
    uint8                  Controller;
    uint8                  MsgNr;
    Can_MessageObjectType *CurMsgBoxPtr;
    uint8                  MsgDlc;
    uint32                 Identifier;
    uint8                  DataByteIndex;
    uint8                 *SduPtr;
    uint32                 ErrCounter;    
   

#if(CAN_DEV_ERROR_DETECT == STD_ON)
    if(ModuleState == CAN_UNINIT)
    {
        Det_ReportError(CAN_MODULE_ID, CAN_INSTANCE, 8, CAN_E_UNINIT);
        return;
    }
#endif

    CurMsgBoxPtr = CurConfig->MessageBoxPtr;
    
    for(Controller = 0; Controller < CAN_CONTROLLER_COUNT; Controller++)
    {
        for(MsgNr = 0; MsgNr < ControllerConfig[Controller].MessageBoxCount; MsgNr++)
        {
            /* Check if NewDat Bit is set and if MessageObject is Receive Object */
            if((CanBase[Controller]->NDx[MsgNr >> 5] & (1 << (MsgNr & 0x1F))) && (CurMsgBoxPtr->Direction == RECEIVE))
            {
            
                /* Read Arbitration, Control and Data Bits and clear IntPnd and NewDat*/
                CanBase[Controller]->IFx[IfRegId].COM = 0x003F0000 | (MsgNr + 1);
                
                ErrCounter = CAN_TIMEOUT_DURATION;
                /* Wait until Busy Flag is 0 */
                while(CanBase[Controller]->IFx[IfRegId].COM & 0x8000)
                {
                    ErrCounter--;
                    if(ErrCounter == 0)
                    {
                        Dem_ReportErrorStatus(CAN_E_TIMEOUT, DEM_EVENT_STATUS_FAILED);
                        return;
                    }
                }
                /* Extended Id */
                if(CanBase[0]->IFx[IfRegId].ARB & 0x40000000)
                {
                    /* Bring Id to standardized format (MSB marks extended Id) */
                    Identifier = (CanBase[0]->IFx[IfRegId].ARB & 0x1FFFFFFF) | 0x80000000;
                }
                /* Standard Id */
                else
                {
                    /* Bring Id to standardized format (MSB marks extended Id) */
                    Identifier = (CanBase[0]->IFx[IfRegId].ARB & 0x1FFC0000) >> 18;
                }
                /* DLC (Max 8) */
                MsgDlc = CanBase[0]->IFx[IfRegId].MC & 0x000F;
                if(MsgDlc > 8)
                {
                    MsgDlc = 8;
                }
                /* Let SduPtr point to Shadow Buffer */
                SduPtr = RxShadowBuf;

                /* Copy Message Data to Shadow Buffer */
                for(DataByteIndex = 0; DataByteIndex < MsgDlc; DataByteIndex++)
                {
                    RxShadowBuf[DataByteIndex] = CanBase[0]->IFx[IfRegId].DATx[ElementIndex[DataByteIndex]];
                }
                /* Indicate successful Reception */
                CanIf_RxIndication(MsgNr, Identifier, MsgDlc, SduPtr);
            }     
            CurMsgBoxPtr++;
        }
    }
#endif
}


/*----------------------------------------------------------------------------*/
/* Function Name: Can_MainFunction_BusOff                                     */
/* Service Id:    0x09                                                        */
/* Timing:        FIXED_CYCLIC                                                */
/* Description:   Polling of bus-off events                                   */
/* Return Value:  None                                                        */
/* Remarks:       -/-                                                         */
/*                                                                            */

void CAN_MAINFUNCTION_BUSOFF()
{
    uint8  Controller;

/* DET Error Check */
#if(CAN_DEV_ERROR_DETECT == STD_ON)
    if(ModuleState == CAN_UNINIT)
    {
        Det_ReportError(CAN_MODULE_ID, CAN_INSTANCE, 9, CAN_E_UNINIT);
        return;
    }    
#endif
    
    for(Controller = 0 ; Controller < CAN_CONTROLLER_COUNT; Controller++)
    {
        /* Bus Off */
        if(CanBase[Controller]->SR & 0x00000080)
        {
#if(CAN_DEV_ERROR_DETECT == STD_ON)
            ControllerMode[Controller] = CANIF_CS_STOPPED;
#endif
            CanIf_ControllerBusOff(Controller);
        }
    }
}


/*----------------------------------------------------------------------------*/
/* Function Name: Can_MainFunction_Wakeup                                     */
/* Service Id:    0x0a                                                        */
/* Timing:        FIXED_CYCLIC                                                */
/* Description:   Polling of wake-up events                                   */
/* Return Value:  None                                                        */
/* Remarks:       -/-                                                         */
/*                                                                            */

void CAN_MAINFUNCTION_WAKEUP()
{
    uint8 Controller;

/* DET Error Check */
#if(CAN_DEV_ERROR_DETECT == STD_ON)
    if(ModuleState == CAN_UNINIT)
    {
        Det_ReportError(CAN_MODULE_ID, CAN_INSTANCE, 0xA, CAN_E_UNINIT);
        return;
    }    
#endif

    for(Controller = 0 ; Controller < CAN_CONTROLLER_COUNT; Controller++)
    {
        /* Check WakeUp Pending */
        if(CanBase[Controller]->SR & 0x200)
        {
            /* Set Init Bit, so that Controller is in Stop state */
            CanBase[Controller]->CTL |= 0x1;
#if(CAN_DEV_ERROR_DETECT == STD_ON)
            ControllerMode[Controller] = CANIF_CS_STOPPED;
#endif
            //EcuM_CheckWakeUp(ControllerConfig[Controller].WakeupSrc);
        }
    }
}




/*----------------------------------------------------------------------------*/
/* Function Name: Can2_InterruptHandler_0                                     */
/* Description:   CAN Node 2 Level 0 Interrupt Service Routine                */
/* Return Value:  None                                                        */
/* Remarks:       -/-                                                         */
/*                                                                            */
#ifdef CanController_1
//#pragma INTERRUPT(CAN2_INTERRUPTHANDLER_0,IRQ)

void CAN2_INTERRUPTHANDLER_0()
{

    uint32  ErrCounter;
    uint32  MsgNr;
    uint32  MsgId;
    uint8   MsgDlc;
    uint8   DataByteIndex;
    uint8  *SduPtr; 

    ErrCounter = CAN_TIMEOUT_DURATION;
    
    if(CanBase[1]->IR == 0x8000)
    {
        /* WakeUp Pending */
        if(CanBase[1]->SR & 0x200)
        {
#if(CAN_DEV_ERROR_DETECT == STD_ON)
            ControllerMode[1] = CANIF_CS_STOPPED;
#endif
            /* Set Init Bit, so that Controller is in Stop state */
            CanBase[1]->CTL |= 0x1;
          //  EcuM_CheckWakeUp(ControllerConfig[1].WakeupSrc);
        }
        /* Bus Off */
        if(CanBase[1]->SR & 0x080)
        {
#if(CAN_DEV_ERROR_DETECT == STD_ON)
            ControllerMode[1] = CANIF_CS_STOPPED;
#endif
            CanIf_ControllerBusOff(1);
        }
    }
    else
    {
        MsgNr = CanBase[1]->IR;
        
        /* Read Arbitration, Control and Data Bits and clear IntPnd */
        CanBase[1]->IFx[IfRegId].COM = 0x003F0000 | MsgNr;
        
        /* Wait until Busy Flag is 0 */
        while(CanBase[1]->IFx[IfRegId].COM & 0x8000)
        {
            ErrCounter--;
            if(ErrCounter == 0)
            {
                Dem_ReportErrorStatus(CAN_E_TIMEOUT, DEM_EVENT_STATUS_FAILED);
                ErrCounter = CAN_TIMEOUT_DURATION;
                return;
            }
        }
        
        /* Transmit Object */
        if(CanBase[1]->IFx[IfRegId].ARB & 0x20000000)
        {
            /* Reset TxRqst-Array Element */
            TxRqstArray_1[MsgNr - 1] = 0;
            /* A Message was successfully transmitted */
            CanIf_TxConfirmation(PduInfoArray_1[MsgNr - 1].swPduHandle);
        }        
        /* Receive Object */
        else
        {
            /* Extended Id */
            if(CanBase[1]->IFx[IfRegId].ARB & 0x40000000)
            {
                /* Bring Id to standardized format (MSB marks extended Id) */
                MsgId = (CanBase[1]->IFx[IfRegId].ARB & 0x1FFFFFFF) | 0x80000000;
            }
            /* Standard Id */
            else
            {
                /* Bring Id to standardized format (MSB marks extended Id) */
                MsgId = (CanBase[1]->IFx[IfRegId].ARB & 0x1FFC0000) >> 18;
            }
            /* DLC (Max 8) */
            MsgDlc = CanBase[1]->IFx[IfRegId].MC & 0x000F;
            if(MsgDlc > 8)
            {
                MsgDlc = 8;
            }
            /* Let SduPtr point to Shadow Buffer */
            SduPtr = RxShadowBuf;

            /* Copy Message Data to Shadow Buffer */
            for(DataByteIndex = 0; DataByteIndex < MsgDlc; DataByteIndex++)
            {
                RxShadowBuf[DataByteIndex] = CanBase[1]->IFx[IfRegId].DATx[ElementIndex[DataByteIndex]];
            }
            /* Indicate successful Reception */
            CanIf_RxIndication(MsgNr - 1, MsgId, MsgDlc, SduPtr);
        }
    }

}
#endif
