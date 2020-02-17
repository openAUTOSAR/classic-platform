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


#ifndef BSWM_INTERNAL_H_
#define BSWM_INTERNAL_H_

/* @req BswM0037 */
#define BSWM_HIERARCHIC_LIST_LEVELS_MAX (7)

typedef enum {
    BSWM_INIT,
    BSWM_UNINIT
} BswM_InitStatusType;

typedef struct {
    BswM_InitStatusType InitStatus;
} BswM_InternalType;

typedef struct {
    uint32 itemIndexBuffer[BSWM_HIERARCHIC_LIST_LEVELS_MAX];
    sint32 listIndexBuffer[BSWM_HIERARCHIC_LIST_LEVELS_MAX];
    uint8 levelCnt;
} BswM_ListStateType;

/* Development Error Tracer */
/* @req BswM0031 */
/* @req BswM0033 */
/* @req BswM0030 */
#if (BSWM_DEV_ERROR_DETECT == STD_ON)

extern BswM_InternalType BswM_Internal;

#define BSWM_DET_REPORTERROR(serviceId, errorId)            \
    (void)Det_ReportError(BSWM_MODULE_ID, 0, serviceId, errorId)

#define BSWM_VALIDATE(expression, serviceId, errorId, ret)  \
    if (!(expression)) {                                    \
        BSWM_DET_REPORTERROR(serviceId, errorId);           \
        return ret;                                 \
    }

#define BSWM_VALIDATE_NORV(expression, serviceId, errorId)  \
    if (!(expression)) {                                    \
        BSWM_DET_REPORTERROR(serviceId, errorId);           \
        return;                                 \
    }

#else
#define BSWM_DET_REPORTERROR(...)
#define BSWM_VALIDATE(...)
#define BSWM_VALIDATE_NORV(...)
#endif

/*lint -emacro(904,BSWM_VALIDATE_INIT,BSWM_VALIDATE_INIT_NORV,BSWM_VALIDATE_POINTER_NORV,BSWM_VALIDATE_REQUESTMODE_NORV,BSWM_VALIDATE_REQUESTMODE,BSWM_VALIDATE_CFGPOINTER_NORV,BSWM_VALIDATE_CFGPARAMRANGE_NORV)*/ /*904 PC-Lint exception to MISRA 14.7 (validate DET macros)*/
#define BSWM_VALIDATE_INIT(serviceID)                   \
        BSWM_VALIDATE((BswM_Internal.InitStatus == BSWM_INIT), serviceID, BSWM_E_NO_INIT, 0, E_NOT_OK)

#define BSWM_VALIDATE_INIT_NORV(serviceID)                  \
        BSWM_VALIDATE_NORV((BswM_Internal.InitStatus == BSWM_INIT), serviceID, BSWM_E_NO_INIT)

#define BSWM_VALIDATE_REQUESTMODE(expression, serviceID)                    \
        BSWM_VALIDATE(expression, serviceID, BSWM_E_REQ_MODE_OUT_OF_RANGE, E_NOT_OK)

#define BSWM_VALIDATE_REQUESTMODE_NORV(expression, serviceID)                   \
        BSWM_VALIDATE_NORV(expression, serviceID, BSWM_E_REQ_MODE_OUT_OF_RANGE)

#define BSWM_VALIDATE_POINTER_NORV(pointer, serviceID)                  \
        BSWM_VALIDATE_NORV((pointer != NULL), serviceID, BSWM_E_PARAM_POINTER)

#define BSWM_VALIDATE_CFGPOINTER_NORV(pointer, serviceID)                  \
        BSWM_VALIDATE_NORV((pointer != NULL), serviceID, BSWM_E_NULL_POINTER)

#define BSWM_VALIDATE_CFGPARAMRANGE_NORV(expression, serviceID)                  \
        BSWM_VALIDATE_NORV(expression, serviceID, BSWM_E_PARAM_CONFIG)

#define BSWM_VALIDATE_LINTPMODEREQUEST_NORV(modeReq) \
        BSWM_VALIDATE_NORV(((LINTP_APPLICATIVE_SCHEDULE == modeReq) || (LINTP_DIAG_REQUEST == modeReq) || (LINTP_DIAG_RESPONSE == modeReq)), BSWM_SERVICEID_LINTPREQUESTMODE, BSWM_E_REQ_MODE_OUT_OF_RANGE)


void BswM_Internal_PduGroupSwitchAction( const BswM_ActionListItemType *item );
void BswM_Internal_DMControlAction( const BswM_ActionListItemType *item );
void BswM_Internal_PduRouterControlAction( const BswM_ActionListItemType *item );
void BswM_Internal_ProcessImmediateRequest( const uint32 modeReqIndex );

#endif /* BSWM_INTERNAL_H_ */
