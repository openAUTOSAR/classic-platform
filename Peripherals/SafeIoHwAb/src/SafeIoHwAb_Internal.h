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

/** @reqSettings DEFAULT_SPECIFICATION_REVISION=4.3.0*/

#ifndef SAFEIOHWAB_INTERNAL_H_
#define SAFEIOHWAB_INTERNAL_H_

/* @req SWS_BSW_00203 API parameter checking enablement. */
/* @req SWS_BSW_00042 Detection of DevErrors should only be performed if configuration parameter for Development errors is set. */
#if (SAFEIOHWAB_DEV_ERROR_DETECT == STD_ON)

#include "Det.h"

/******************************************* DET macros *******************************************/

 /*lint -e904 MISRA:OTHER:argument check:[MISRA 2012 Rule 15.5, advisory]*/
/* Validates parameters, and if it fails will immediately leave the function with the specified return code */
/* This is not inline with Table 8, ISO26262-6:2011, Req 1a and 1h */


/* @req SWS_BSW_00045 Development errors should be reported to DET module */
#define SAFEIOHWAB_DET_REPORT_ERROR(api, error)                   \
            (void)Det_ReportError(SAFEIOHWAB_MODULE_ID, SAFEIOHWAB_INSTANCE_ID, api, error); \

/* @req SWS_BSW_00049 API parameter checking */
#define SAFEIOHWAB_VALIDATE_RETURN(expression, api, error, rv)  \
            if ( !(expression) ) {                              \
                SAFEIOHWAB_DET_REPORT_ERROR(api, error);        \
                return rv;                                      \
            }                                                   \

/* @req SWS_BSW_00049 API parameter checking */			
#define SAFEIOHWAB_VALIDATE_NO_RV(expression, api, error)   \
            if ( !(expression) ) {                              \
                SAFEIOHWAB_DET_REPORT_ERROR(api, error);        \
                return;                                         \
            }                                                   \

#else  /* (SAFEIOHWAB_DEV_ERROR_DETECT == STD_OFF) */

#define SAFEIOHWAB_DET_REPORT_ERROR(api, error)
#define SAFEIOHWAB_VALIDATE_RETURN(expression, api, error, rv)  \
            if ( !(expression) ) {                              \
                return rv;                                      \
            }                                                   \


#define SAFEIOHWAB_VALIDATE_NO_RV(expression, api, error)   \
            if ( !(expression) ) {                              \
                return;                                         \
            }

#endif /* SAFEIOHWAB_DEV_ERROR_DETECT */

#endif /* SAFEIOHWAB_INTERNAL_H_ */

