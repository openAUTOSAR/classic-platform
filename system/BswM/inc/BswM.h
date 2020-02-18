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

#ifndef BSWM_H_
#define BSWM_H_

/* @req BswM0025 */
#include "Std_Types.h"

#if defined(USE_RTE)
#include "Rte_BswM_Type.h"
#endif

#define BSWM_VENDOR_ID  60u
#define BSWM_MODULE_ID  42u

#define BSWM_AR_RELEASE_MAJOR_VERSION   4
#define BSWM_AR_RELEASE_MINOR_VERSION   0
#define BSWM_AR_RELEASE_PATCH_VERSION   3

#define BSWM_AR_MAJOR_VERSION BSWM_AR_RELEASE_MAJOR_VERSION
#define BSWM_AR_MINOR_VERSION BSWM_AR_RELEASE_MINOR_VERSION
#define BSWM_AR_PATCH_VERSION BSWM_AR_RELEASE_PATCH_VERSION

#define BSWM_SW_MAJOR_VERSION   4u
#define BSWM_SW_MINOR_VERSION   0u
#define BSWM_SW_PATCH_VERSION   1u

/* @req BswM0029 */
#define BSWM_E_NO_INIT                  0x01u
#define BSWM_E_NULL_POINTER             0x02u
#define BSWM_E_PARAM_INVALID            0x03u
#define BSWM_E_REQ_USER_OUT_OF_RANGE    0x04u
#define BSWM_E_REQ_MODE_OUT_OF_RANGE    0x05u
#define BSWM_E_PARAM_CONFIG             0x06u
#define BSWM_E_PARAM_POINTER            0x07u

#define BSWM_SERVICEID_INIT                  0x00u
#define BSWM_SERVICEID_GETVERSIONINFO        0x01u
#define BSWM_SERVICEID_REQUESTMODE           0x02u
#define BSWM_SERVICEID_MAINFUNCTION          0x03u
#define BSWM_SERVICEID_DEINIT                0x04u
#define BSWM_SERVICEID_CANSMCURRENTSTATE     0x05u
#define BSWM_SERVICEID_DCMCOMMUNICATIONMODE  0x06u
#define BSWM_SERVICEID_LINSMCURRENTSTATE     0x09u
#define BSWM_SERVICEID_LINSMCURRENTSCHEDULE  0x0Au
#define BSWM_SERVICEID_LINTPREQUESTMODE      0x0Bu
#define BSWM_SERVICEID_FRSMCURRENTSTATE      0x0Cu
#define BSWM_SERVICEID_ETHSMCURRENTSTATE     0x0Du
#define BSWM_SERVICEID_COMMCURRENTMODE       0x0Eu
#define BSWM_SERVICEID_ECUMCURRENTSTATE      0x0Fu
#define BSWM_SERVICEID_ECUMCURRENTWAKEUP     0x10u
#define BSWM_SERVICEID_COMMCURRENTPNCMODE    0x15u

#define BSWM_SERVICEID_SDCLIENTSERVICECURRENTSTATE          0x1Fu
#define BSWM_SERVICEID_SDEVENTHANDLERCURRENTSTATE           0x20u
#define BSWM_SERVICEID_SDCONSUMEDEVENTGROUPCURRENTSTATE     0x21u

#include "BswM_Cfg.h"

#if defined(BSWM_COMM_ENABLED)
#define IS_VALID_COMM_MODE(_x)  ((COMM_FULL_COMMUNICATION == (_x)) || (COMM_SILENT_COMMUNICATION == (_x)) || (COMM_NO_COMMUNICATION == (_x)))
#endif

/* @req BswM0007 */
#if defined(USE_DEM)
#define BSWM_REPORT_ERROR_STATUS(eventID, error) Dem_ReportErrorStatus((Dem_EventIdType) eventID, error);
#else
#define BSWM_REPORT_ERROR_STATUS(eventID, error)
#endif

#define BSWM_UNDEFINED_REQUEST_VALUE 0xFF


/* @req BswM0041 */
typedef uint8 BswM_UserType;
typedef uint8 BswM_ModeType;


typedef enum
{
    BSWM_FALSE = 0,
    BSWM_TRUE,
    BSWM_UNDEFINED,
} BswM_RuleStateType;


typedef enum {
    BSWM_CONDITION = 0,
    BSWM_TRIGGER
} BswM_ExecutionType;


typedef enum {
    BSWM_ATOMIC = 0,
    BSWM_LIST,
    BSWM_RULE
} BswM_ActionType;


typedef enum {
    BSWM_PDUR_DISABLE = 0,
    BSWM_PDUR_ENABLE
} BswM_PduR_ActionType;


typedef struct {
    const uint32               AtomicActionIndex;
    const sint32               *PduRoutingPathGroupRefId;
    const sint32               ReferredListIndex;
    const sint32               ReportFailToDEMRefId;
    const sint32               *DisabledPduGroupRefId;
    const sint32               *EnabledPduGroupRefId;
    const sint16               ReferredRuleIndex;
    const BswM_ActionType      ActionType;
    const BswM_PduR_ActionType PduRouterAction;
    const uint8                LinScheduleRef;
    const boolean              AbortOnFail;
    const boolean              PduGroupSwitchReinit;
} BswM_ActionListItemType;


typedef struct {
    const BswM_ActionListItemType  * const * Items;
    const uint32                   NumberOfItems;
    const BswM_ExecutionType       Execution;
} BswM_ActionListType;


typedef struct {
    const uint32                  LogicalExpressionIndex;
    const sint32                  ActionListTrueIndex;
    const sint32                  ActionListFalseIndex;
    const BswM_RuleStateType      RuleStateInit;
} BswM_RuleType;


typedef struct {
    const sint16           *DeferredRules;
    const sint16           *const*ImmediateRules;
    const BswM_RuleType    *RuleData;
    const uint32           NumberOfRules;
} BswM_RuleConfigType;


typedef struct {
    const BswM_RuleConfigType *RuleCfg;
    const BswM_ActionListType *ActionListData;
    const uint32              *ModeRequestData;
} BswM_ConfigType;


/*================================ BswM API =================================*/

void BswM_Init( const BswM_ConfigType* ConfigPtr );
void BswM_Deinit( void );
void BswM_MainFunction( void );


#if ( BSWM_GENERIC_REQUEST_ENABLED == STD_ON )
/* @req BswM0046 */
void BswM_RequestMode( BswM_UserType requesting_user, BswM_ModeType requested_mode );
#endif

#if ( BSWM_VERSION_INFO_API == STD_ON )
/* @req BswM0003 */
void BswM_GetVersionInfo( Std_VersionInfoType* VersionInfo );
#endif

extern const BswM_ConfigType  BswM_Config;

#endif  /* BSWM_H_ */

