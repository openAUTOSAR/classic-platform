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


/** @reqSettings DEFAULT_SPECIFICATION_REVISION=4.3.0 */
/** @fileSafetyClassification QM **/ /* SchM_Partition_QM contains SchM task for SP QM partition. */

#include "SchM_SP.h"
#include "SchM_cfg.h"

#if defined(HOST_TEST)
#include "schm_devtest_stubs.h"
#endif
/*lint -save -e451  MISRA:STANDARDIZED_INTERFACE:AUTOSAR need Inclusion:[MISRA 2012 Directive 4.10, required]*/
/*lint -save -e553  MISRA:STANDARDIZED_INTERFACE:AUTOSAR need Inclusion: [MISRA 2012 Rule 20.9, required]*/
/*lint -save -e9019 MISRA:STANDARDIZED_INTERFACE:AUTOSAR need Inclusion:[MISRA 2012 Rule 20.1, advisory] */
#if defined(USE_KERNEL)
#include "Os.h"
#endif

#if defined(USE_MCU)
#include "Mcu.h"
#endif

#if defined(USE_GPT)
#include "Gpt.h"
#endif

#if defined(USE_CAN)
#include "Can.h"
#include "SchM_Can.h"
/*lint -restore */
#define SCHM_START_SEC_VAR_CLEARED_QM_UNSPECIFIED
#include "SchM_MemMap.h"
SCHM_DECLARE(CAN_MODE);
#define SCHM_STOP_SEC_VAR_CLEARED_QM_UNSPECIFIED
#include "SchM_MemMap.h"

#else
#define SCHM_MAINFUNCTION_CAN_WRITE()
#define SCHM_MAINFUNCTION_CAN_READ()
#define SCHM_MAINFUNCTION_CAN_BUSOFF()
#define SCHM_MAINFUNCTION_CAN_ERROR()
#define SCHM_MAINFUNCTION_CAN_WAKEUP()
#define SCHM_MAINFUNCTION_CAN_MODE()
#endif

#if defined(USE_CANIF)
#include "CanIf.h"
#include "SchM_CanIf.h"
#endif

#if defined(USE_XCP)
#include "Xcp.h"
#include "SchM_Xcp.h"

#define SCHM_START_SEC_VAR_CLEARED_QM_UNSPECIFIED
#include "SchM_MemMap.h"
SCHM_DECLARE(XCP);
#define SCHM_STOP_SEC_VAR_CLEARED_QM_UNSPECIFIED
#include "SchM_MemMap.h"

#else
#define SCHM_MAINFUNCTION_XCP()
#endif

#if defined(USE_PDUR)
#include "PduR.h"
#include "SchM_PduR.h"
#endif

#if defined(USE_COM)
#include "Com.h"
#include "SchM_Com.h"

#define SCHM_START_SEC_VAR_CLEARED_QM_UNSPECIFIED
#include "SchM_MemMap.h"
SCHM_DECLARE(COMRX);
#define SCHM_STOP_SEC_VAR_CLEARED_QM_UNSPECIFIED
#include "SchM_MemMap.h"

#define SCHM_START_SEC_VAR_CLEARED_QM_UNSPECIFIED
#include "SchM_MemMap.h"
SCHM_DECLARE(COMTX);
#define SCHM_STOP_SEC_VAR_CLEARED_QM_UNSPECIFIED
#include "SchM_MemMap.h"

#else
#define SCHM_MAINFUNCTION_COMRX()
#define SCHM_MAINFUNCTION_COMTX()
#endif

#if defined(USE_CANTP)
#include "CanTp.h"
#include "SchM_CanTp.h"

#define SCHM_START_SEC_VAR_CLEARED_QM_UNSPECIFIED
#include "SchM_MemMap.h"
SCHM_DECLARE(CANTP);
#define SCHM_STOP_SEC_VAR_CLEARED_QM_UNSPECIFIED
#include "SchM_MemMap.h"

#else
#define SCHM_MAINFUNCTION_CANTP()
#endif


#if defined(USE_BSWM)
#include "BswM.h"
#include "SchM_BswM.h"

#define SCHM_START_SEC_VAR_CLEARED_QM_UNSPECIFIED
#include "SchM_MemMap.h"
SCHM_DECLARE(BSWM);
#define SCHM_STOP_SEC_VAR_CLEARED_QM_UNSPECIFIED
#include "SchM_MemMap.h"

#else
#define SCHM_MAINFUNCTION_BSWM()
#endif

#if defined(USE_FRTP)
#include "FrTp.h"
#include "SchM_FrTp.h"
#else
#define SCHM_MAINFUNCTION_FRTP()
#endif

#if defined(USE_J1939TP)
#include "J1939Tp.h"
#include "SchM_J1939Tp.h"

#define SCHM_START_SEC_VAR_CLEARED_QM_UNSPECIFIED
#include "SchM_MemMap.h"
SCHM_DECLARE(J1939TP);
#define SCHM_STOP_SEC_VAR_CLEARED_QM_UNSPECIFIED
#include "SchM_MemMap.h"

#else
#define SCHM_MAINFUNCTION_J1939TP()
#endif


#if defined(USE_DCM)
#include "Dcm.h"
#include "SchM_Dcm.h"

#define SCHM_START_SEC_VAR_CLEARED_QM_UNSPECIFIED
#include "SchM_MemMap.h"
SCHM_DECLARE(DCM);
#define SCHM_STOP_SEC_VAR_CLEARED_QM_UNSPECIFIED
#include "SchM_MemMap.h"

#else
#define SCHM_MAINFUNCTION_DCM()
#endif

#if defined(USE_DEM)
#include "Dem.h"
#include "SchM_Dem.h"

#define SCHM_START_SEC_VAR_CLEARED_QM_UNSPECIFIED
#include "SchM_MemMap.h"
SCHM_DECLARE(DEM);
#define SCHM_STOP_SEC_VAR_CLEARED_QM_UNSPECIFIED
#include "SchM_MemMap.h"

#else
#define SCHM_MAINFUNCTION_DEM()
#endif

#if defined(USE_PWM)
#include "Pwm.h"
#include "SchM_Pwm.h"
#endif


#if defined(USE_IOHWAB)
#include "IoHwAb.h"
#include "SchM_IoHwAb.h"

#define SCHM_START_SEC_VAR_CLEARED_QM_UNSPECIFIED
#include "SchM_MemMap.h"
SCHM_DECLARE(IOHWAB);
#define SCHM_STOP_SEC_VAR_CLEARED_QM_UNSPECIFIED
#include "SchM_MemMap.h"

#else
#define SCHM_MAINFUNCTION_IOHWAB()
#endif

#if defined(USE_FLS)
#include "Fls.h"
#include "SchM_Fls.h"

#define SCHM_START_SEC_VAR_CLEARED_QM_UNSPECIFIED
#include "SchM_MemMap.h"
SCHM_DECLARE(FLS);
#define SCHM_STOP_SEC_VAR_CLEARED_QM_UNSPECIFIED
#include "SchM_MemMap.h"

#else
#define SCHM_MAINFUNCTION_FLS()
#endif

#if defined(USE_ECUM_FIXED)
#include "EcuM.h"
#include "SchM_EcuM.h"

#define SCHM_START_SEC_VAR_CLEARED_QM_UNSPECIFIED
#include "SchM_MemMap.h"
SCHM_DECLARE(ECUM_QM);
#define SCHM_STOP_SEC_VAR_CLEARED_QM_UNSPECIFIED
#include "SchM_MemMap.h"

#else
#define SCHM_MAINFUNCTION_ECUM_QM()
#endif

#if defined(USE_EEP)
#include "Eep.h"
#include "SchM_Eep.h"

#define SCHM_START_SEC_VAR_CLEARED_QM_UNSPECIFIED
#include "SchM_MemMap.h"
SCHM_DECLARE(EEP);
#define SCHM_STOP_SEC_VAR_CLEARED_QM_UNSPECIFIED
#include "SchM_MemMap.h"

#else
#define SCHM_MAINFUNCTION_EEP()
#endif

#if defined(USE_FEE)
#include "Fee.h"
#include "SchM_Fee.h"

#define SCHM_START_SEC_VAR_CLEARED_QM_UNSPECIFIED
#include "SchM_MemMap.h"
SCHM_DECLARE(FEE);
#define SCHM_STOP_SEC_VAR_CLEARED_QM_UNSPECIFIED
#include "SchM_MemMap.h"

#else
#define SCHM_MAINFUNCTION_FEE()
#endif

#if defined(USE_EA)
#include "Ea.h"
#include "SchM_Ea.h"

#define SCHM_START_SEC_VAR_CLEARED_QM_UNSPECIFIED
#include "SchM_MemMap.h"
SCHM_DECLARE(EA);
#define SCHM_STOP_SEC_VAR_CLEARED_QM_UNSPECIFIED
#include "SchM_MemMap.h"

#else
#define SCHM_MAINFUNCTION_EA()
#endif

#if defined(USE_NVM)
#include "NvM.h"
#include "SchM_NvM.h"

#define SCHM_START_SEC_VAR_CLEARED_QM_UNSPECIFIED
#include "SchM_MemMap.h"
SCHM_DECLARE(NVM);
#define SCHM_STOP_SEC_VAR_CLEARED_QM_UNSPECIFIED
#include "SchM_MemMap.h"

#else
#define SCHM_MAINFUNCTION_NVM()
#endif

#if defined(USE_COMM)
#include "ComM.h"
#include "SchM_ComM.h"

#define SCHM_START_SEC_VAR_CLEARED_QM_UNSPECIFIED
#include "SchM_MemMap.h"
SCHM_DECLARE(COMM);
#define SCHM_STOP_SEC_VAR_CLEARED_QM_UNSPECIFIED
#include "SchM_MemMap.h"

#else
#define SCHM_MAINFUNCTION_COMM()
#endif

#if defined(USE_NM)
#include "Nm.h"
#include "SchM_Nm.h"

#define SCHM_START_SEC_VAR_CLEARED_QM_UNSPECIFIED
#include "SchM_MemMap.h"
SCHM_DECLARE(NM);
#define SCHM_STOP_SEC_VAR_CLEARED_QM_UNSPECIFIED
#include "SchM_MemMap.h"

#else
#define SCHM_MAINFUNCTION_NM()
#endif

#if defined(USE_CANNM)
#include "CanNm.h"
#include "SchM_CanNm.h"

#define SCHM_START_SEC_VAR_CLEARED_QM_UNSPECIFIED
#include "SchM_MemMap.h"
SCHM_DECLARE(CANNM);
#define SCHM_STOP_SEC_VAR_CLEARED_QM_UNSPECIFIED
#include "SchM_MemMap.h"

#else
#define SCHM_MAINFUNCTION_CANNM()
#endif

#if defined(USE_CANSM)
#include "CanSM.h"
#include "SchM_CanSM.h"

#define SCHM_START_SEC_VAR_CLEARED_QM_UNSPECIFIED
#include "SchM_MemMap.h"
SCHM_DECLARE(CANSM);
#define SCHM_STOP_SEC_VAR_CLEARED_QM_UNSPECIFIED
#include "SchM_MemMap.h"

#else
#define SCHM_MAINFUNCTION_CANSM()
#endif

#if defined(USE_CANTRCV)
#include "CanTrcv.h"
#include "SchM_CanTrcv.h"

#define SCHM_START_SEC_VAR_CLEARED_QM_UNSPECIFIED
#include "SchM_MemMap.h"
SCHM_DECLARE(CANTRCV);
#define SCHM_STOP_SEC_VAR_CLEARED_QM_UNSPECIFIED
#include "SchM_MemMap.h"

#else
#define SCHM_MAINFUNCTION_CANTRCV()
#endif

#if defined(USE_UDPNM)
#include "UdpNm.h"
#endif

#if defined(USE_LINIF)
#include "LinIf.h"
#include "SchM_LinIf.h"

#define SCHM_START_SEC_VAR_CLEARED_QM_UNSPECIFIED
#include "SchM_MemMap.h"
SCHM_DECLARE(LINIF);
#define SCHM_STOP_SEC_VAR_CLEARED_QM_UNSPECIFIED
#include "SchM_MemMap.h"

#else
#define SCHM_MAINFUNCTION_LINIF()
#endif

#if defined(USE_LINSM)
#include "LinSM.h"
#include "SchM_LinSM.h"

#define SCHM_START_SEC_VAR_CLEARED_QM_UNSPECIFIED
#include "SchM_MemMap.h"
SCHM_DECLARE(LINSM);
#define SCHM_STOP_SEC_VAR_CLEARED_QM_UNSPECIFIED
#include "SchM_MemMap.h"

#else
#define SCHM_MAINFUNCTION_LINSM()
#endif


#if defined(USE_CDD_LINSLV)
#include "CDD_LinSlv.h"
#include "SchM_LinCdd.h"
#define SCHM_START_SEC_VAR_CLEARED_QM_UNSPECIFIED
#include "SchM_MemMap.h"
SCHM_DECLARE(LINCDD);
#define SCHM_STOP_SEC_VAR_CLEARED_QM_UNSPECIFIED
#include "SchM_MemMap.h"
#else
#define SCHM_MAINFUNCTION_LINCDD()
#endif

#if defined(USE_SPI) && !defined(CFG_SPI_ASIL)
#include "Spi.h"
#include "SchM_Spi.h"
#define SCHM_START_SEC_VAR_CLEARED_QM_UNSPECIFIED
#include "SchM_MemMap.h"
SCHM_DECLARE(SPI);
#define SCHM_STOP_SEC_VAR_CLEARED_QM_UNSPECIFIED
#include "SchM_MemMap.h"
#else
#define SCHM_MAINFUNCTION_SPI()
#endif

#if defined(USE_WDG)
#include "Wdg.h"
#endif

#if (CAN_USE_WRITE_POLLING == STD_ON)
#define SCHM_START_SEC_VAR_CLEARED_QM_UNSPECIFIED
#include "SchM_MemMap.h"
SCHM_DECLARE(CAN_WRITE);
#define SCHM_STOP_SEC_VAR_CLEARED_QM_UNSPECIFIED
#include "SchM_MemMap.h"
#endif

#if (CAN_USE_READ_POLLING == STD_ON)
#define SCHM_START_SEC_VAR_CLEARED_QM_UNSPECIFIED
#include "SchM_MemMap.h"
SCHM_DECLARE(CAN_READ);
#define SCHM_STOP_SEC_VAR_CLEARED_QM_UNSPECIFIED
#include "SchM_MemMap.h"
#endif

#if (CAN_USE_BUSOFF_POLLING == STD_ON)
#define SCHM_START_SEC_VAR_CLEARED_QM_UNSPECIFIED
#include "SchM_MemMap.h"
SCHM_DECLARE(CAN_BUSOFF);
#define SCHM_STOP_SEC_VAR_CLEARED_QM_UNSPECIFIED
#include "SchM_MemMap.h"
#endif

#if (CAN_USE_WAKEUP_POLLING == STD_ON)
#define SCHM_START_SEC_VAR_CLEARED_QM_UNSPECIFIED
#include "SchM_MemMap.h"
SCHM_DECLARE(CAN_WAKEUP);
#define SCHM_STOP_SEC_VAR_CLEARED_QM_UNSPECIFIED
#include "SchM_MemMap.h"
#endif

#if (ARC_CAN_ERROR_POLLING == STD_ON)
#define SCHM_START_SEC_VAR_CLEARED_QM_UNSPECIFIED
#include "SchM_MemMap.h"
SCHM_DECLARE(CAN_ERROR);
#define SCHM_STOP_SEC_VAR_CLEARED_QM_UNSPECIFIED
#include "SchM_MemMap.h"
#endif

#if defined(USE_FIM)
#include "FiM.h"
#include "SchM_FiM.h"
#else
#define SCHM_MAINFUNCTION_FIM()
#endif
/*lint -restore */



static void runMemory( void ) {
    SCHM_MAINFUNCTION_NVM();
    SCHM_MAINFUNCTION_EA();
    SCHM_MAINFUNCTION_FEE();
    SCHM_MAINFUNCTION_EEP();
    SCHM_MAINFUNCTION_FLS();
    SCHM_MAINFUNCTION_SPI();
}

/* @req ARC_SWS_SchM_00003 */
/* @req ARC_SWS_SchM_00011 The service EcuM mainfunctions shall not be called from tasks which may invoke runnable entities. */
TASK(SchM_Partition_QM) {
    EcuM_StateType state;
    EcuM_SP_RetStatus retStatus = ECUM_SP_OK;

    do {
        /*lint -save -e534 MISRA:HARDWARE_ACCESS::[MISRA 2012 Rule 17.7, required] */
        SYS_CALL_WaitEvent(EVENT_MASK_Alarm_BswServices_Partition_QM);
        SYS_CALL_ClearEvent(EVENT_MASK_Alarm_BswServices_Partition_QM);

        EcuM_GetState(&state);
        /*lint -restore */

        /* @req ARC_SWS_SchM_00006 The BSW scheduler shall schedule BSW modules by calling their MainFunctions */
        switch( state ) {
            case ECUM_STATE_STARTUP_ONE:
            case ECUM_STATE_STARTUP_TWO:

                SCHM_MAINFUNCTION_ECUM_QM(state, retStatus);

                /* Synchronize with the ASIL partition */
                if (retStatus == ECUM_SP_RELEASE_SYNC) {
                    retStatus = ECUM_SP_OK;
                    /* Report to ASIL SchM, QM is ready */ /* ARC_SWS_SchM_00004 */
                    SYS_CALL_SetEvent(TASK_ID_SchM_Partition_A0, EVENT_MASK_SynchPartition_A0); /*lint !e534 MISRA:HARDWARE_ACCESS::[MISRA 2012 Rule 17.7, required] */
                }
                break;
            default: /* Schedule BSW on QM partition */

               runMemory(); /*lint !e522 MISRA:FALSE_POSITIVE:Schedule BSW on QM partition:[MISRA 2012 Rule 2.2, required]*/

               SCHM_MAINFUNCTION_BSWM();

               SCHM_MAINFUNCTION_ECUM_QM(state, retStatus);

               SCHM_MAINFUNCTION_CANTRCV();
               SCHM_MAINFUNCTION_CAN_MODE();
                  /*lint -save -e553 MISRA:CONFIGURATION:CAN polling:[MISRA 2012 Rule 20.9, required]*/
#if (CAN_USE_WRITE_POLLING == STD_ON)
               SCHM_MAINFUNCTION_CAN_WRITE();
#endif
#if (CAN_USE_READ_POLLING == STD_ON)
               SCHM_MAINFUNCTION_CAN_READ();
#endif
#if (CAN_USE_BUSOFF_POLLING == STD_ON)
               SCHM_MAINFUNCTION_CAN_BUSOFF();
#endif
#if (ARC_CAN_ERROR_POLLING == STD_ON)
               SCHM_MAINFUNCTION_CAN_ERROR();
#endif
#if (CAN_USE_WAKEUP_POLLING == STD_ON)
               SCHM_MAINFUNCTION_CAN_WAKEUP();
#endif
               /*lint -restore */
               SCHM_MAINFUNCTION_COMRX();
               SCHM_MAINFUNCTION_COMTX();

               SCHM_MAINFUNCTION_XCP();

               SCHM_MAINFUNCTION_CANTP();
               SCHM_MAINFUNCTION_J1939TP();
               SCHM_MAINFUNCTION_DCM();
               SCHM_MAINFUNCTION_DEM();
               SCHM_MAINFUNCTION_FIM();

               SCHM_MAINFUNCTION_IOHWAB();
               SCHM_MAINFUNCTION_COMM();
               SCHM_MAINFUNCTION_NM();
               SCHM_MAINFUNCTION_CANNM();
               SCHM_MAINFUNCTION_CANSM();
               SCHM_MAINFUNCTION_LINIF();
               SCHM_MAINFUNCTION_LINSM();
               SCHM_MAINFUNCTION_LINCDD();

                /* Check if QM EcuM is ready with synching */
               if (retStatus == ECUM_SP_RELEASE_SYNC) {
                   retStatus = ECUM_SP_OK;
                  /* Report to ASIL SchM, QM is ready */
                  SYS_CALL_SetEvent(TASK_ID_SchM_Partition_A0, EVENT_MASK_SynchPartition_A0); /*lint !e534 MISRA:HARDWARE_ACCESS::[MISRA 2012 Rule 17.7, required] */
               }
               break;
        }
    } while (SCHM_TASK_EXTENDED_CONDITION != 0);  /*lint !e506 MISRA:PERFORMANCE:Required to loop task in order to synchronize partitions:[MISRA 2012 Rule 2.1, required]*/
}

