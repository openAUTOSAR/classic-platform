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









#include "ComM.h"
#include "ComM_BusSm.h"

void ComM_Init( ComM_ConfigType *){

}

void ComM_DeInit(){

}

void ComM_GetVersionInfo( Std_VersionInfoType versioninfo ){

}


Std_ReturnType ComM_GetStatus( ComM_InitStatusType* Status ){
	return E_OK;
}

Std_ReturnType ComM_GetInhibitionStatus( NetworkHandleType Channel, ComM_InhibitionStatusType* Status ){
	return E_OK;
}


Std_ReturnType ComM_RequestComMode( ComM_UserHandleType User, ComM_ModeType ComMode ){
	return E_OK;
}

Std_ReturnType ComM_GetMaxComMode( ComM_UserHandleType User, ComM_ModeType* ComMode ){
	return E_OK;
}

Std_ReturnType ComM_GetRequestedComMode( ComM_UserHandleType User, ComM_ModeType* ComMode ){
	return E_OK;
}

Std_ReturnType ComM_GetCurrentComMode( ComM_UserHandleType User, ComM_ModeType* ComMode ){
	return E_OK;
}


Std_ReturnType ComM_PreventWakeUp( NetworkHandleType Channel, boolean Status ){
	return E_OK;
}

Std_ReturnType ComM_LimitChannelToNoComMode( NetworkHandleType Channel, boolean Status ){
	return E_OK;
}

Std_ReturnType ComM_LimitECUToNoComMode( boolean Status ){
	return E_OK;
}

Std_ReturnType ComM_ReadInhibitCounter( uint16* CounterValue ){
	return E_OK;
}

Std_ReturnType ComM_ResetInhibitCounter(){
	return E_OK;
}

Std_ReturnType ComM_SetECUGroupClassification( ComM_InhibitionStatusType Status ){
	return E_OK;
}


// Network Management Interface Callbacks
// --------------------------------------

void ComM_Nm_NetworkStartIndication( NetworkHandleType Channel ){

}

void ComM_Nm_NetworkMode( NetworkHandleType Channel ){

}

void ComM_Nm_PrepareBusSleepMode( NetworkHandleType Channel ){

}

void ComM_Nm_BusSleepMode( NetworkHandleType Channel ){

}

void ComM_Nm_RestartIndication( NetworkHandleType Channel ){

}


// ECU State Manager Callbacks
// ---------------------------

void ComM_EcuM_RunModeIndication( NetworkHandleType channel ){

}

void ComM_EcuM_WakeUpIndication( NetworkHandleType Channel ){

}


// Diagnostic Communication Manager Callbacks
// ------------------------------------------

void ComM_DCM_ActiveDiagnostic(){

}

void ComM_DCM_InactiveDiagnostic(){

}


// Bus State Manager Callbacks
// ---------------------------

void ComM_BusSM_ModeIndication( NetworkHandleType Channel,ComM_ModeType ComMode ){

}
