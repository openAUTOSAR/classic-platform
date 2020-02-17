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


#if !(((ECUM_SW_MAJOR_VERSION == 2) && (ECUM_SW_MINOR_VERSION == 3)) )
#error EcuM: Configuration file expected BSW module version to be 2.3.\*
#endif


#ifndef _ECUM_GENERATED_TYPES_H_
#define _ECUM_GENERATED_TYPES_H_


#include "PreCompiledDataHash.h"

/* @req SWS_EcuM_02992 */ /* @req SWS_EcuM_02992 */

#include "EcuM_Types.h"

#if defined(USE_MCU)
#include "Mcu.h"
#endif
#if defined(USE_PORT)
#include "Port.h"
#endif
#if defined(USE_CAN)
#include "Can.h"
#endif
#if defined(USE_DIO)
#include "Dio.h"
#endif
#if defined(USE_LIN)
#include "Lin.h"
#endif
#if defined(USE_CANIF)
#include "CanIf.h"
#endif
#if defined(USE_LINIF)
#include "LinIf.h"
#endif
#if defined(USE_PWM)
#include "Pwm.h"
#endif
#if defined (USE_OCU)
#include "Ocu.h"
#endif
#if defined (USE_ICU)
#include "Icu.h"
#endif
#if defined(USE_COM)
#include "Com.h"
#endif
#if defined(USE_PDUR)
#include "PduR.h"
#endif
#if defined(USE_DMA)
#include "Dma.h"
#endif
#if defined(USE_ADC)
#include "Adc.h"
#endif
#if defined(USE_GPT)
#include "Gpt.h"
#endif
#if defined(USE_COMM)
#include "ComM.h"
#endif
#if defined(USE_NM)
#include "Nm.h"
#endif
#if defined(USE_CANNM)
#include "CanNm.h"
#endif
#if defined(USE_CANSM)
#include "CanSM.h"
#endif

#if defined(USE_LINSM)
#include "LinSM.h"
#endif
#if defined(USE_J1939TP)
#include "J1939Tp.h"
#endif
#if defined(USE_UDPNM)
#include "UdpNm.h"
#endif
#if defined(USE_FLS)
#include "Fls.h"
#endif
#if defined(USE_EEP)
#include "Eep.h"
#endif
#if defined(USE_SPI)
#include "Spi.h"
#endif
#if defined(USE_WDG)
#include "Wdg.h"
#endif
#if defined(USE_WDGM)
#include "WdgM.h"
#endif
#if defined(USE_WDGIF)
#include "WdgIf.h"
#endif
#if defined(USE_ETH)
#include "Eth.h"
#endif
#if defined(USE_FR)
#include "Fr.h"
#endif

#if defined(USE_BSWM)
#include "BswM.h"
#endif

#if defined(USE_PDUR) || defined(USE_COM) || defined(USE_CANIF) || defined(USE_CANTP) || defined(USE_FRTP)
#include "EcuM_PBTypes.h"
#endif

#if defined(USE_DCM)
#include "Dcm.h"
#endif
#if defined(USE_DEM)
#include "Dem.h"
#endif
#if defined(USE_XCP)
#include "Xcp.h"
#endif
#if defined(USE_IPDUM)
#include "IpduM.h"
#endif
#if defined(USE_DLT)
#include "Dlt.h"
#endif

#if defined(USE_FR)
#include "Fr.h"
#endif

#if defined(USE_FRIF)
#include "FrIf.h"
#endif

#if defined(USE_FRNM)
#include "FrNm.h"
#endif

#if defined(USE_FRSM)
#include "FrSM.h"
#endif

#if defined(USE_FRTP)
#include "FrTp.h"
#endif


typedef struct EcuM_ConfigS
{
    uint32 EcuMPostBuildVariant;            /* @req SWS_EcuM_02794 */ /* @req SWS_EcuM_02794 */
    uint64 EcuMConfigConsistencyHashLow;    /* @req SWS_EcuM_02795 Hash set when compiling the whole software for the */ /* @req SWS_EcuM_02795 */
    uint64 EcuMConfigConsistencyHashHigh;   /* PB module. It is compared to the PB hash at startup. */
    EcuM_StateType EcuMDefaultShutdownTarget;
    uint8 EcuMDefaultSleepMode;
    AppModeType EcuMDefaultAppMode;
    uint32 EcuMRunMinimumDuration;
    uint32 EcuMNvramReadAllTimeout;
    uint32 EcuMNvramWriteAllTimeout;
    Mcu_ModeType EcuMNormalMcuMode;
#if defined(USE_DEM)
    Dem_EventIdType EcuMDemInconsistencyEventId;
    Dem_EventIdType EcuMDemRamCheckFailedEventId;
    Dem_EventIdType EcuMDemAllRunRequestsKilledEventId;
#endif
    const EcuM_SleepModeType *EcuMSleepModeConfig;
    const EcuM_WakeupSourceConfigType *EcuMWakeupSourceConfig;
#if defined(USE_ECUM_FLEXIBLE)
    const EcuM_UserType *EcuMGoDownAllowedUsersConfig;
#endif
#if defined (USE_COMM)
    const EcuM_ComMConfigType *EcuMComMConfig;
#endif   
#if defined(USE_MCU)
    const Mcu_ConfigType* McuConfigPtr;
#endif
#if defined(USE_PORT)
    const Port_ConfigType* PortConfigPtr;
#endif
#if defined(USE_CAN)
    const Can_ConfigType* CanConfigPtr;
#endif
#if defined(USE_DIO)
    const Dio_ConfigType* DioConfigPtr;
#endif
#if defined(USE_CANSM)
    const CanSM_ConfigType* CanSMConfigPtr;
#endif
#if defined(USE_LIN)
    const Lin_ConfigType* LinConfigPtr;
#endif
#if defined(USE_LINIF)
    const LinIf_ConfigType* LinIfConfigPtr;
#endif
#if defined(USE_LINSM)
    const LinSM_ConfigType* LinSMConfigPtr;
#endif
#if defined(USE_ETH)
    const Eth_ConfigType* EthConfigPtr;
#endif
#if defined(USE_NM)
    const Nm_ConfigType* NmConfigPtr;
#endif
#if defined(USE_UDPNM)
    const UdpNm_ConfigType* UdpNmConfigPtr;
#endif
#if defined(USE_COMM)
    const ComM_ConfigType* ComMConfigPtr;
#endif
#if defined(USE_BSWM)
    const BswM_ConfigType* BswMConfigPtr;
#endif
#if defined(USE_J1939TP)
    const J1939Tp_ConfigType* J1939TpConfigPtr;
#endif
#if defined(USE_PWM)
    const Pwm_ConfigType* PwmConfigPtr;
#endif
#if defined(USE_OCU)
    const Ocu_ConfigType* OcuConfigPtr;
#endif
#if defined(USE_ICU)
    const Icu_ConfigType* IcuConfigPtr;
#endif
#if defined(USE_DMA)
    const Dma_ConfigType* DmaConfigPtr;
#endif
#if defined(USE_ADC)
    const Adc_ConfigType* AdcConfigPtr;
#endif
#if defined(USE_GPT)
    const Gpt_ConfigType* GptConfigPtr;
#endif
#if defined(USE_FLS)
    const Fls_ConfigType* FlsConfigPtr;
#endif
#if defined(USE_EEP)
    const Eep_ConfigType* EepConfigPtr;
#endif
#if defined(USE_SPI)
    const Spi_ConfigType* SpiConfigPtr;
#endif
#if defined(USE_WDG)
    const Wdg_ConfigType* WdgConfigPtr;
#endif
#if defined(USE_WDGIF)
    const WdgIf_ConfigType* WdgIfConfigPtr;
#endif
#if defined(USE_WDGM)
    const WdgM_ConfigType* WdgMConfigPtr;
#endif
#if defined(USE_DCM)
    const Dcm_ConfigType* DcmConfigPtr;
#endif
#if defined(USE_DEM)
    const Dem_ConfigType* DemConfigPtr;
#endif
#if defined(USE_XCP)
    const Xcp_ConfigType* XcpConfigPtr;
#endif
#if defined(USE_IPDUM)
    const IpduM_ConfigType* IpduMConfigPtr;
#endif
#if defined(USE_PDUR) || defined(USE_COM) || defined(USE_CANIF) || defined(USE_CANTP) || defined(USE_CANNM)
    const PostbuildConfigType* PostBuildConfig;
#endif
#if defined(USE_DLT)
    const Dlt_ConfigType* DltConfigPtr;
#endif
#if defined(USE_FR)
    const Fr_ConfigType * FrConfigPtr;
#endif

#if defined(USE_FRIF)
    const FrIf_ConfigType * FrIfConfigPtr;
#endif

#if defined(USE_FRNM)
    const FrNm_ConfigType * FrNmConfigPtr;
#endif

#if defined(USE_FRSM)
    const FrSM_ConfigType * FrSMConfigPtr;
#endif

#if defined(USE_FRTP)
    const FrTp_ConfigType * FrTpConfigPtr;
#endif
} EcuM_ConfigType;

#endif /*_ECUM_GENERATED_TYPES_H_*/
