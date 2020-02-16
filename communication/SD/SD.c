/* -------------------------------- Arctic Core ------------------------------
 * Arctic Core - the open source AUTOSAR platform http://arccore.com
 *
 * Copyright (C) 2011  ArcCore AB <contact@arccore.com>
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

#include "SD.h"
#include "Soad.h"
#include "BswM.h"
#include "Det.h"
#include "Dem.h"
#include "MemMap.h"
#include "SchM_SD.h"

void Sd_Init( const Sd_ConfigType* ConfigPtr );
void Sd_GetVersionInfo( Std_VersionInfoType* versioninfo );
Std_ReturnType Sd_ServerServiceSetState( uint16 SdServerServiceHandleId, Sd_ServerServiceSetStateType ServerServiceState);
Std_ReturnType Sd_ClientServiceSetState( uint16 ClientServiceInstanceID, Sd_ClientServiceSetStateType ClientServiceState );
Std_ReturnType Sd_ConsumedEventGroupSetState( uint16 SdConsumedEventGroupHandleId, Sd_ConsumedEventGroupSetStateType ConsumedEventGroupState );
void Sd_LocalIpAddrAssignmentChg( SoAd_SoConIdType SoConId, TcpIp_IpAddrStateType State );

void Sd_RxIndication( PduIdType RxPduId, PduInfoType* PduInfoPtr);

void Sd_MainFunction( void );
