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

#ifndef RTE_DLT_TYPE_H_
#define RTE_DLT_TYPE_H_

#define DLT_NOT_SERVICE_COMPONENT

#ifdef USE_RTE
#warning This file should only be used when not using an RTE with Dlt Service Component.
#include "Rte_Type.h"

/* @req SWS_Dlt_00230 */
/* @req SWS_Dlt_00010 */
/** Enum literals for Dlt_MessageLogLevelType */
#ifndef DLT_LOG_OFF
#define DLT_LOG_OFF 0U
#endif /* DLT_LOG_OFF */

#ifndef DLT_LOG_FATAL
#define DLT_LOG_FATAL 1U
#endif /* DLT_LOG_FATAL */

#ifndef DLT_LOG_ERROR
#define DLT_LOG_ERROR 2U
#endif /* DLT_LOG_ERROR */

#ifndef DLT_LOG_WARN
#define DLT_LOG_WARN 3U
#endif /* DLT_LOG_WARN */

#ifndef DLT_LOG_INFO
#define DLT_LOG_INFO 4U
#endif /* DLT_LOG_INFO */

#ifndef DLT_LOG_DEBUG
#define DLT_LOG_DEBUG 5U
#endif /* DLT_LOG_DEBUG */

#ifndef DLT_LOG_VERBOSE
#define DLT_LOG_VERBOSE 6U
#endif /* DLT_LOG_VERBOSE */

/** Enum literals for Dlt_MessageTraceType */
#ifndef DLT_TRACE_VARIABLE
#define DLT_TRACE_VARIABLE 1U
#endif /* DLT_TRACE_VARIABLE */

#ifndef DLT_TRACE_FUNCTION_IN
#define DLT_TRACE_FUNCTION_IN 2U
#endif /* DLT_TRACE_FUNCTION_IN */

#ifndef DLT_TRACE_FUNCTION_OUT
#define DLT_TRACE_FUNCTION_OUT 3U
#endif /* DLT_TRACE_FUNCTION_OUT */

#ifndef DLT_TRACE_STATE
#define DLT_TRACE_STATE 4U
#endif /* DLT_TRACE_STATE */

#ifndef DLT_TRACE_VFB
#define DLT_TRACE_VFB 5U
#endif /* DLT_TRACE_VFB */

#else
/* @req SWS_Dlt_00225 */
typedef uint32 Dlt_SessionIDType;

/* @req SWS_Dlt_00226 *//* @req SWS_Dlt_00127 *//* @req SWS_Dlt_00312 */
typedef uint8 Dlt_ApplicationIDType[4];

/* @req SWS_Dlt_00227 *//* @req SWS_Dlt_00128 *//* @req SWS_Dlt_00313 */
typedef uint8 Dlt_ContextIDType[4];

/* @req SWS_Dlt_00228 */
typedef uint32 Dlt_MessageIDType;

/* @req SWS_Dlt_00229 */
typedef uint8 Dlt_MessageOptionsType;

/* @req SWS_Dlt_00235 */
typedef uint16 Dlt_MessageArgumentCount;

typedef uint8 Dlt_MessageLogLevelType;

/* @req SWS_Dlt_00236 */
typedef struct {
    Dlt_MessageArgumentCount arg_count;
    Dlt_MessageLogLevelType log_level;
    Dlt_MessageOptionsType options;
    Dlt_ContextIDType  context_id;
    Dlt_ApplicationIDType  app_id;
} Dlt_MessageLogInfoType;

/* @req SWS_Dlt_00230 */
/* @req SWS_Dlt_00010 */
/** Enum literals for Dlt_MessageLogLevelType */
#ifndef DLT_LOG_OFF
#define DLT_LOG_OFF 0U
#endif /* DLT_LOG_OFF */

#ifndef DLT_LOG_FATAL
#define DLT_LOG_FATAL 1U
#endif /* DLT_LOG_FATAL */

#ifndef DLT_LOG_ERROR
#define DLT_LOG_ERROR 2U
#endif /* DLT_LOG_ERROR */

#ifndef DLT_LOG_WARN
#define DLT_LOG_WARN 3U
#endif /* DLT_LOG_WARN */

#ifndef DLT_LOG_INFO
#define DLT_LOG_INFO 4U
#endif /* DLT_LOG_INFO */

#ifndef DLT_LOG_DEBUG
#define DLT_LOG_DEBUG 5U
#endif /* DLT_LOG_DEBUG */

#ifndef DLT_LOG_VERBOSE
#define DLT_LOG_VERBOSE 6U
#endif /* DLT_LOG_VERBOSE */

typedef uint8 Dlt_MessageTraceType;

/* @req SWS_Dlt_00237 */
typedef struct {
    Dlt_MessageTraceType trace_info;
    Dlt_MessageOptionsType options;
    Dlt_ContextIDType  context;
    Dlt_ApplicationIDType app_id;
} Dlt_MessageTraceInfoType;

/** Enum literals for Dlt_MessageTraceType */
#ifndef DLT_TRACE_VARIABLE
#define DLT_TRACE_VARIABLE 1U
#endif /* DLT_TRACE_VARIABLE */

#ifndef DLT_TRACE_FUNCTION_IN
#define DLT_TRACE_FUNCTION_IN 2U
#endif /* DLT_TRACE_FUNCTION_IN */

#ifndef DLT_TRACE_FUNCTION_OUT
#define DLT_TRACE_FUNCTION_OUT 3U
#endif /* DLT_TRACE_FUNCTION_OUT */

#ifndef DLT_TRACE_STATE
#define DLT_TRACE_STATE 4U
#endif /* DLT_TRACE_STATE */

#ifndef DLT_TRACE_VFB
#define DLT_TRACE_VFB 5U
#endif /* DLT_TRACE_VFB */

typedef uint8 Dlt_ReturnType;


#endif /* USE_RTE */

#endif /* RTE_DLT_TYPE_H_ */
