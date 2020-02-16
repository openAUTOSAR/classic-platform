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


#ifndef CANSM_CONFIGTYPES_H_
#define CANSM_CONFIGTYPES_H_

/** @req CANSM010.bswbuilder */
#if defined(USE_DEM)
#include "Dem.h"
#endif
typedef struct {
	const uint8						CanIfControllerId;
} CanSM_ControllerType;

typedef struct {
	const CanSM_ControllerType*		Controllers;
	const uint8						ControllerCount;
	const uint8						ComMNetworkHandle;
	const uint32                    CanSMBorTimeTxEnsured;
	const uint32                    CanSMBorTimeL1;
	const uint32                    CanSMBorTimeL2;
	const uint8                     CanSMBorCounterL1ToL2;
	const boolean                   CanSMBorTxConfirmationPolling;
#if (USE_DEM)
	Dem_EventIdType					CanSMDemEventId;
#endif
} CanSM_NetworkType;

typedef struct {
	const uint8						CanSMModeRequestRepetitionMax;
	const uint32					CanSMModeRequestRepetitionTime;
	const CanSM_NetworkType*		Networks;
} CanSM_ConfigType;

#endif /* CANSM_CONFIGTYPES_H_ */
