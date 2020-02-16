

#ifndef BSWM_H_
#define BSWM_H_

#include "Modules.h"
#include "Std_Types.h"

#ifdef EGSM_USE_HARDCODED_RTE_BSWM
#include "Rte_Type.h"
#endif

#define BSWM_MODULE_ID			MODULE_ID_BSWM
#define BSWM_VENDOR_ID			60

#define BSWM_SW_MAJOR_VERSION	1
#define BSWM_SW_MINOR_VERSION	0
#define BSWM_SW_PATCH_VERSION	0

#define BSWM_AR_MAJOR_VERSION	4
#define BSWM_AR_MINOR_VERSION	0
#define BSWM_AR_PATCH_VERSION	3

// TODO: move to config:
#define BSWM_DEV_ERROR_DETECT   STD_ON

// Dev errors
#define BSWM_E_NULL_POINTER     0x01


#include "ComStack_Types.h"

//#include "CanSM_Types.h" // Missing
//#include "Com_Types.h"
#include "ComM_Types.h"
#include "Dcm_Types.h"
#include "Dem_Types.h"
#include "EcuM_Cbk.h"
#include "EcuM_Types.h"
//#include "EthSm_Types.h" // Missing
//#include "FrSm_Types.h" // Missing
#include "LinIf_Types.h"
//#include "LinSM_Types.h" // Missing
//#include "McOs_Types.h" // Missing
#include "NvM_Types.h"
//#include "Os_Types.h" // Missing

#include "CanSM_BswM.h"

#if 0
NOT IMPLEMENTED YET

#include "BswM_Cfg.h"
#include "Rte_BswM_Type.h"
#endif

/* This is the BswM interface */

#define BSWM_VERSION_INFO_API STD_ON

/* @req BswM0003*/
#if ( BSWM_VERSION_INFO_API == STD_ON)
#define BswM_GetVersionInfo(_vi) STD_GET_VERSION_INFO(_vi,BSWM)
#endif



typedef uint8 BswM_ConfigType;

// Use RTE defined mode type (or below defined type)
#ifndef EGSM_USE_HARDCODED_RTE_BSWM
// Use this mode type

// TODO: move to config:
typedef enum
{
    BSWM_MODE_NONE,     /* Not a real mode */
    BSWM_MODE_STARTUP,  /* Starting up */
    BSWM_MODE_SILENT,   /* Sending None, Receiving All */
    BSWM_MODE_NORMAL,   /* Sending All, Receiving All */
    BSWM_MODE_SHUTDOWN, /* Shutdown */
    BSWM_MODE_FCC_FAIL,  /* Frame configuration check failed, formerly called EXPULSION */
} BswM_ModeType;

#else
// Use RTE defined type
typedef ModeType BswM_ModeType;

#endif

// TODO: move to config:
typedef enum
{
    BSWM_USER_0,
    BSWM_USER_1,
} BswM_UserType;


/* @req BswM0002 */
void BswM_Init(const BswM_ConfigType * ConfigPtr);

/* @req BswM0119 */
void BswM_Deinit(void);

/* @req BswM0046*/
void BswM_RequestMode(BswM_UserType requesting_user, BswM_ModeType requested_mode);

/* @req BswM0053 */
void BswM_MainFunction( void );


/* @req BswM0047 */
void BswM_ComM_CurrentMode(NetworkHandleType Network, ComM_ModeType RequestedMode);

/* @req BswM0056*/
void BswM_EcuM_CurrentState(EcuM_StateType CurrentState);

/* @req BswM0031*/
void BswM_EcuM_CurrentWakeup(EcuM_WakeupSourceType source, EcuM_WakeupStatusType state);

/* @req BswM0104 */
void BswM_NvM_CurrentBlockMode(NvM_BlockIdType Block, NvM_RequestResultType CurrentBlockMode);

/* @req BswM0152 */
void BswM_NvM_CurrentJobMode(uint8 ServiceId, NvM_RequestResultType CurrentJobMode);

/* @req BswM0058 */
void BswM_LinSM_CurrentSchedule(NetworkHandleType Network, LinIf_SchHandleType CurrentSchedule);

/* @req BswM0157 */
void BswM_WdgM_RequestPartitionReset(ApplicationType Application);

/* @req BswM0049 */
void BswM_CanSM_CurrentState(NetworkHandleType Network, CanSM_BswMCurrentStateType CurrentState);


#if 0

/* @req BswM0148 */
void BswM_ComM_CurrentPNCMode(PNCHandleType PNC, ComM_PncModeType RequestedMode);
// Missing PNCHandleType, ComM_PncModeType in ComStack_Types

/* @req BswM0048 */
void BswM_Dcm_CommunicationMode_CurrentState(NetworkHandleType Network, Dcm_CommunicationModeType RequestedMode);
// Missing Dcm_CommunicationModeType


/* @req BswM0050*/
void BswM_EthSM_CurrentState(NetworkHandleType Network, EthSM_NetworkModeStateType CurrentState);
// Missing EthSM_NetworkModeStateType

/* @req BswM0051*/
void BswM_FrSM_CurrentState(NetworkHandleType Network, FrSM_BswM_StateType CurrentState);
// Missing FrSM_BswM_StateType

/* @req BswM0052*/
void BswM_LinSM_CurrentState(NetworkHandleType Network, LinSM_ModeType CurrentState);
// Missing LinSM_ModeType

/* @req BswM0156 */
void BswM_LinTp_RequestMode(NetworkHandleType Network, LinTp_Mode LinTpRequestedMode);
// Missing LinTp_Mode

/* @req BswM0140 */
Std_ReturnType BswM_TriggerStartUpPhase2(CoreIdType CoreID );
// Missing CoreIdType

/* @req BswM0141 */
Std_ReturnType BswM_TriggerSlaveRTEStop(CoreIdType CoreID );
// Missing CoreIdType

#endif



#endif
