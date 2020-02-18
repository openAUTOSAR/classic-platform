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

#ifndef DEM_NVM_H_
#define DEM_NVM_H_

#include "Dem.h"

void Dem_NvM_SetEventBlockChanged(Dem_DTCOriginType Origin, boolean ImmediateStorage);
void Dem_NvM_SetFreezeFrameBlockChanged(Dem_DTCOriginType Origin, boolean ImmediateStorage);
void Dem_NvM_SetExtendedDataBlockChanged(Dem_DTCOriginType Origin, boolean ImmediateStorage);
void Dem_NvM_SetIndicatorBlockChanged(boolean ImmediateStorage);
void Dem_NvM_SetStatusBitSubsetBlockChanged(boolean ImmediateStorage);
void Dem_NvM_SetPermanentBlockChanged(boolean ImmediateStorage);
void Dem_NvM_SetPreStoreFreezeFrameBlockChanged(boolean ImmediateStorage);
void Dem_Nvm_SetIumprBlockChanged(boolean ImmediateStorage);

void Dem_NvM_Init(void);
void Dem_NvM_MainFunction(void);

#endif /* DEM_NVM_H_ */
