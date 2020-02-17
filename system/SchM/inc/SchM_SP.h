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

/* @req ARC_SWS_SchM_00002 The Basic Software Scheduler shall provide APIs for other BSW modules to enter exclusive areas */
/* @req ARC_SWS_SchM_00009 The Basic Software Scheduler shall provide complementary APIs for other BSW modules to exit exclusive areas */

#ifndef SCHM_SP_H_
#define SCHM_SP_H_

#include "Std_Types.h"

#define SCHM_MODULE_ID            130u
#define SCHM_VENDOR_ID            60u

/* Implementation version */
#define SCHM_SW_MAJOR_VERSION     1u
#define SCHM_SW_MINOR_VERSION     0u
#define SCHM_SW_PATCH_VERSION     0u

/* Error IDs */
#define SCHM_E_SYNCH    1u

/* Service IDs */
#define SCHM_SID_A0     0x01
#define SCHM_SID_QM     0x02

void SchM_GetVersionInfo( Std_VersionInfoType *versionInfo );   /* @req ARC_SWS_SchM_00007 */

#ifndef SCHM_TASK_EXTENDED_CONDITION
#define SCHM_TASK_EXTENDED_CONDITION 1
#endif

typedef struct  {
    uint32 timer;
} SchM_InfoType;

#define SCHM_DECLARE(_mod)  \
        SchM_InfoType SchM_Info_ ## _mod \

/* @req ARC_SWS_SchM_00008 */
#define SCHM_MAINFUNCTION(_mod,_func) \
        if( (++SchM_Info_ ## _mod.timer % SCHM_MAINFUNCTION_CYCLE_ ## _mod )== 0 ) { \
            _func; \
            SchM_Info_ ## _mod.timer = 0; \
        }

#define SP_SCHM_MAINFUNCTION(_rv,_mod,_func) \
        if( (++SchM_Info_ ## _mod.timer % SCHM_MAINFUNCTION_CYCLE_ ## _mod )== 0 ) { \
            _rv = _func; \
            SchM_Info_ ## _mod.timer = 0; \
        }


#endif /* SCHM_SP_H_*/
