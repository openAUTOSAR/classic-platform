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

/* @req SWS_EcuM_02990*/
/* @req SWS_EcuMf_00028 Integration functions should be placed in callout functions. */


/* ----------------------------[includes]------------------------------------*/
//lint -emacro(9036,VALIDATE_STATE)

#include "EcuM.h"
#include "EcuM_Generated_Types.h"
#include "isr.h"


/* @req SWS_BSW_00152 */

/* ----------------------------[private define]------------------------------*/
/* Does not run functions that are not drivers */
#if defined(USE_ECUM_FIXED)
#define VALIDATE_STATE(_state) \
    do { \
        EcuM_StateType ecuMState;  \
        if (E_OK != EcuM_GetState(&ecuMState)) { \
            ASSERT(0); \
        } \
        ASSERT(ecuMState == (_state) ); \
    } while(0)
#else
#define VALIDATE_STATE(_state)
#endif

/* ----------------------------[private macro]-------------------------------*/

/* ----------------------------[private typedef]-----------------------------*/
/* ----------------------------[private function prototypes]-----------------*/
/* ----------------------------[private variables]---------------------------*/
/* ----------------------------[private functions]---------------------------*/
/* ----------------------------[public functions]----------------------------*/


#if defined(USE_ECUM_FIXED)
#if (ECUM_ARC_SAFETY_PLATFORM == STD_ON)

/**
 * At this point OS and essential drivers have started.
 * Start the rest of the drivers
 *
 *
 * Part of STARTUP II
 *
 * @param ConfigPtr
 */
void EcuM_AL_DriverInitTwo_Partition_QM(const EcuM_ConfigType* ConfigPtr)
{
    (void)ConfigPtr; /*lint !e920 MISRA:FALSE_POSITIVE:Allowed to cast pointer to void here:[MISRA 2012 Rule 1.3, required]*/
#if defined(USE_SPI)
    Spi_Init(ConfigPtr->SpiConfigPtr);  // Setup SPI
#endif
#if defined(USE_EEP)
    Eep_Init(ConfigPtr->EepConfigPtr);   // Setup EEP
#endif
#if defined(USE_FLS)
    Fls_Init(ConfigPtr->FlsConfigPtr);   // Setup Flash
#endif
#if defined(USE_FEE)
    Fee_Init();  // Setup FEE
#endif
#if defined(USE_EA)
    Ea_Init();   // Setup EA
#endif
#if defined(USE_NVM)
    NvM_Init();  // Setup NVRAM Manager and start the read all job
    NvM_ReadAll();
#endif
#if defined(USE_LIN)
    Lin_Init(ConfigPtr->LinConfigPtr);    // Setup Lin driver
#endif
#if defined(USE_LINIF)
    LinIf_Init(ConfigPtr->LinIfConfigPtr);    // Setup LinIf
#endif
#if defined(USE_LINSM)
    LinSM_Init(ConfigPtr->LinSMConfigPtr);    // Setup LinSM
#endif

#if defined(USE_CANTRCV)
    // Setup Can transceiver driver
    CanTrcv_Init(ConfigPtr->PostBuildConfig->CanTrcvConfigPtr);
#endif

#if defined(USE_CAN)
    Can_Init(ConfigPtr->PostBuildConfig->CanConfigPtr);  // Setup Can driver
#endif
#if defined(USE_FR)
    Fr_Init(ConfigPtr->FrConfigPtr);    // Setup Flexray CC driver
#endif
#if defined(USE_CANIF)
    CanIf_Init(ConfigPtr->PostBuildConfig->CanIfConfigPtr);  // Setup CanIf
#endif
#if defined(USE_FRIF)
    FrIf_Init(ConfigPtr->FrIfConfigPtr);    // Setup Flexray Interface
#endif
#if defined(USE_CANTP)
    CanTp_Init(ConfigPtr->PostBuildConfig->CanTpConfigPtr);  // Setup CAN TP
#endif
#if defined(USE_FRTP)
    FrTp_Init(ConfigPtr->FrTpConfigPtr);    // Setup Flexray TP
#endif
#if defined(USE_CANSM)
    CanSM_Init(ConfigPtr->CanSMConfigPtr);
#endif
#if defined(USE_FRSM)
    FrSM_Init(ConfigPtr->FrSMConfigPtr);    // Setup Flexray SM
#endif
#if defined(USE_ETHIF)
    // Setup EthIf before Eth & EthTrcv
    EthIf_Init(ConfigPtr->EthIfConfigPtr);
#endif
#if defined(USE_ETH)
    Eth_Init(ConfigPtr->EthConfigPtr);  // Setup Eth
#endif
#if defined(USE_ETHIF)
    EthIf_Init(ConfigPtr->EthIfConfigPtr);   //  Setup Eth If
#endif
#if defined(USE_ETHSM)
    EthSM_Init();    //  Setup Eth SM
#endif
#if defined(USE_TCPIP)
    TcpIp_Init(ConfigPtr->TcpIpConfigPtr);   // Setup Tcp Ip
#endif
#if defined(USE_J1939TP)
    J1939Tp_Init(ConfigPtr->J1939TpConfigPtr);          // Setup J1939Tp
#endif
#if defined(USE_SOAD)
    SoAd_Init(ConfigPtr->SoAdConfigPtr);     // Setup Socket Adaptor
#endif
#if defined(USE_SD)
    Sd_Init(ConfigPtr->SdConfigPtr); // Setup Service Discovery
#endif
#if defined(USE_LDCOM)
    LdCom_Init(ConfigPtr->LdComConfigPtr);   // Setup Large Data Com
#endif
#if defined(USE_PDUR)
    PduR_Init(ConfigPtr->PostBuildConfig->PduRConfigPtr);    // Setup PDU Router
#endif
#if defined(USE_OSEKNM)
    OsekNm_Init(ConfigPtr->OsekNmConfigPtr);    // Setup Osek Network Manager
#endif
#if defined(USE_CANNM)
    CanNm_Init(ConfigPtr->PostBuildConfig->CanNmConfigPtr);    // Setup Can Network Manager
#endif
#if defined(USE_FRNM)
    FrNm_Init(ConfigPtr->FrNmConfigPtr);    // Setup Flexray Network Manager
#endif
#if defined(USE_UDPNM)
    UdpNm_Init(ConfigPtr->UdpNmConfigPtr);        // Setup Udp Network Manager
#endif
#if defined(USE_NM)
    Nm_Init();        // Setup Network Management Interface
#endif
#if defined(USE_COM)
    Com_Init(ConfigPtr->PostBuildConfig->ComConfigPtr);  // Setup COM layer
#endif
#if defined(USE_DCM)
    Dcm_Init(ConfigPtr->DcmConfigPtr);   // Setup DCM
#endif
#if defined(USE_IOHWAB)
    IoHwAb_Init();  // Setup IO hardware abstraction layer
#endif
#if defined(USE_XCP)
    Xcp_Init(ConfigPtr->XcpConfigPtr);   // Setup XCP
#endif
#if defined(USE_ETHTSYN)
    // Setup EthTSyn
    EthTSyn_Init(ConfigPtr->EthTSynConfigPtr);
#endif
#if defined(USE_STBM)
    // Setup StbM
    StbM_Init(ConfigPtr->StbMConfigPtr);
#endif
}


/**
 * Part of STARTUP II
 *
 * @param ConfigPtr
 */
void EcuM_AL_DriverInitThree_Partition_QM(const EcuM_ConfigType* ConfigPtr)
{
    (void)ConfigPtr; /*lint !e920 MISRA:FALSE_POSITIVE:Allowed to cast pointer to void here:[MISRA 2012 Rule 1.3, required]*/

#if defined(USE_FIM)
    // Setup Function Inhibition Manager
#if (FIM_POSTBUILD_MODE == STD_ON)
    FiM_Init(ConfigPtr->PostBuildConfig->FiMConfigPtr);
#else
    FiM_Init(ConfigPtr->FiMConfigPtr);
#endif
#endif

#if defined(USE_DEM)
    // Setup DEM
    Dem_Init();
#endif

#if defined(USE_DLT)
    Dlt_Init(ConfigPtr->DltConfigPtr); /* Needs to be done after nvram has been initialised */
#endif

#if defined(USE_COMM)
    // Setup Communication Manager
    ComM_Init(ConfigPtr->PostBuildConfig->ComMConfigPtr);
#endif

#if defined(USE_SOMEIPXF)
    // Setup SomeIp Transformer
    SomeIpXf_Init(NULL);
#endif

#if defined(USE_E2EXF)
    // Setup E2E Transformer
    E2EXf_Init(NULL);
#endif

#if defined (USE_SECOC)
    SecOC_Init(ConfigPtr->SecOCConfigPtr);
#endif

}

void EcuM_OnEnterRun_Partition_QM(void)
{

}
void EcuM_OnExitRun_Partition_QM(void)
{

}
void EcuM_OnExitPostRun_Partition_QM(void)
{

}
void EcuM_OnPrepShutdown_Partition_QM(void)
{

}

void EcuM_OnGoOffOne_Partition_QM(void)
{

}
void EcuM_OnGoOffTwo_Partition_QM(void)
{

}

/**
 * At this point OS and essential drivers have started.
 * Start the rest of the drivers
 *
 *
 * Part of STARTUP II
 *
 * @param ConfigPtr
 */
/*lint -save -e715 MISRA:OTHER:Symbol ConfigPtr not referenced.since this file is only an example which shall be extended by implementer if needed:[MISRA 2012 Rule 2.7, advisory] */
void EcuM_AL_DriverInitTwo_Partition_A0(const EcuM_ConfigType* ConfigPtr)
{

}
/**
 * Part of STARTUP II
 *
 * @param ConfigPtr
 */
void EcuM_AL_DriverInitThree_Partition_A0(const EcuM_ConfigType* ConfigPtr)
{

}
/*lint -restore -e715 */

void EcuM_OnEnterRun_Partition_A0(void)
{

}

void EcuM_OnExitRun_Partition_A0(void)
{

}

void EcuM_OnExitPostRun_Partition_A0(void)
{

}

void EcuM_OnPrepShutdown_Partition_A0(void)
{

}

void EcuM_OnGoOffOne_Partition_A0(void)
{

}

void EcuM_OnGoOffTwo_Partition_A0(void)
{

}

#endif // ECUM_ARC_SAFETY_PLATFORM
#endif // USE_ECUM_FIXED
