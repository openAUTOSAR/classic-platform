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





#ifndef COMM_H_
#define COMM_H_

#include "ComM_Types.h"

typedef enum {
	TBD
} ComM_UserHandleType;

typedef struct {
	void *canIf;
	void *canTp;
	void *Frlf;
	void *LinIf;
	void *LinTp;
	void *PduR;
	void *IPDUM;
	void *Nm;
	void *Com;
	void *Dcm;	
} ComM_ConfigType;


void ComM_Init( ComM_ConfigType *);
void ComM_DeInit();
void ComM_GetVersionInfo( Std_VersionInfoType versioninfo );

Std_ReturnType ComM_GetStatus( ComM_InitStatusType* Status );
Std_ReturnType ComM_GetInhibitionStatus( NetworkHandleType Channel, ComM_InhibitionStatusType* Status );

Std_ReturnType ComM_RequestComMode( ComM_UserHandleType User, ComM_ModeType ComMode );
Std_ReturnType ComM_GetMaxComMode( ComM_UserHandleType User, ComM_ModeType* ComMode );
Std_ReturnType ComM_GetRequestedComMode( ComM_UserHandleType User, ComM_ModeType* ComMode );
Std_ReturnType ComM_GetCurrentComMode( ComM_UserHandleType User, ComM_ModeType* ComMode );

Std_ReturnType ComM_PreventWakeUp( NetworkHandleType Channel, boolean Status );
Std_ReturnType ComM_LimitChannelToNoComMode( NetworkHandleType Channel, boolean Status );
Std_ReturnType ComM_LimitECUToNoComMode( boolean Status );
Std_ReturnType ComM_ReadInhibitCounter( uint16* CounterValue );
Std_ReturnType ComM_ResetInhibitCounter();
Std_ReturnType ComM_SetECUGroupClassification( ComM_InhibitionStatusType Status );


#endif /*COMM_H_*/
