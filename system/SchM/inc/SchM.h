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
/** @fileSafetyClassification ASIL **/ /* Same SchM.h always used  */

/* @req ARC_SWS_SchM_00002 The Basic Software Scheduler shall provide APIs for other BSW modules to enter exclusive areas */
/* @req ARC_SWS_SchM_00009 The Basic Software Scheduler shall provide complementary APIs for other BSW modules to exit exclusive areas */

/* @req SWS_BSW_00020 */
/* @req ARC_SWS_SchM_00010 The SchM module shall be designed and implemented as a compile and hardware independent module. */
#ifndef SCHM_H_
#define SCHM_H_

/* @req SWS_BSW_00024 Include AUTOSAR Standard Types Header in implementation header */
#include "Std_Types.h"

#define SCHM_MODULE_ID            130u
#define SCHM_VENDOR_ID            60u

/* @req SWS_BSW_00059 Published information */
#define SCHM_SW_MAJOR_VERSION     1u
#define SCHM_SW_MINOR_VERSION     0u
#define SCHM_SW_PATCH_VERSION     0u

#define SCHM_AR_RELEASE_MAJOR_VERSION        4u
#define SCHM_AR_RELEASE_MINOR_VERSION        0u

void SchM_GetVersionInfo( Std_VersionInfoType *versionInfo );   /* @req ARC_SWS_SchM_00007 */

void SchM_Init( void );
void SchM_Deinit( void );

#define SchM_Enter( _module, _exc_area ) \
    SchM_Enter_EcuM ## _module ##  _exc_area

#define SchM_Exit( _module, _exc_area ) \
    SchM_Enter_EcuM ## _module ##  _exc_area


#define CONCAT_(_x,_y)	_x##_y


typedef struct  {
    uint32 timer;
} SchM_InfoType;

#define SCHM_DECLARE(_mod)	\
        SchM_InfoType SchM_Info_ ## _mod

/* @req ARC_SWS_SchM_00008 */
#define SCHM_MAINFUNCTION(_mod,_func) \
        if( (++SchM_Info_ ## _mod.timer % SCHM_MAINFUNCTION_CYCLE_ ## _mod )== 0 ) { \
            _func; \
            SchM_Info_ ## _mod.timer = 0; \
        }

#endif /*SCHM_H_*/
