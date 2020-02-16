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


#ifndef SCHM_H_
#define SCHM_H_

#include "Std_Types.h"

void SchM_Init( void );
void SchM_Deinit( void );
void SchM_GetVersionInfo( Std_VersionInfoType *versionInfo );

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

#define SCHM_MAINFUNCTION(_mod,_func) \
		if( (++SchM_Info_ ## _mod.timer % SCHM_MAINFUNCTION_CYCLE_ ## _mod )== 0 ) { \
			_func; \
			SchM_Info_ ## _mod.timer = 0; \
		}



#endif /*SCHM_H_*/
