/* -------------------------------- Arctic Core ------------------------------
 * Arctic Core - the open source AUTOSAR platform http://arccore.com
 *
 * Copyright (C) 2009  ArcCore AB <contact@arccore.com>
 *
 * This source code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by the
 * Free Software Foundation; See <http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 * -------------------------------- Arctic Core ------------------------------*/


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
