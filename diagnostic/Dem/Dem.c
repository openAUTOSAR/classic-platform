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









#include "Std_Types.h"
#include "Rte.h"
#include "Dem.h"
#include "Dem_Lcfg.h"
#include "Dem_PBcfg.h"
//#include "NvM.h"
// #include "Fim.h"
// #include "Rte_Dem.h"
#include "Dem_IntEvtId.h"
#include "Dem_IntErrId.h"
#include "Os.h"

void Dem_PreInit( void ) {
	
}
void Dem_Init( void ) {
	
}

Std_ReturnType Dem_ReportErrorStatus( Dem_EventIdType id ,Dem_EventStatusType type ) {
	(void )id;
	(void )type;
	return RTE_E_OK;
}
