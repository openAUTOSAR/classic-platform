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

#ifndef DEM_EXTENSION_H_
#define DEM_EXTENSION_H_

#include "Dem.h"
#include "Dem_Internal.h"

void Dem_Extension_MainFunction(void);
void Dem_Extension_ClearEvent(const Dem_EventParameterType *eventParam);
void Dem_Extension_UpdateEventstatus(EventStatusRecType *eventStatusRecPtr, uint8 eventStatusExtendedBeforeUpdate, Dem_EventStatusType eventStatus);
void Dem_Extension_OperationCycleStart(Dem_OperationCycleIdType operationCycleId, EventStatusRecType *eventStatusRecPtr);
void Dem_Extension_OperationCycleEnd(Dem_OperationCycleIdType operationCycleId, EventStatusRecType *eventStatusRecPtr);
void Dem_Extension_PreInit(const Dem_ConfigType *ConfigPtr);
void Dem_Extension_Init_PostEventMerge(Dem_DTCOriginType origin);
void Dem_Extension_Init_Complete(void);
void Dem_Extension_Shutdown(void);
void Dem_Extension_GetExtendedDataInternalElement(Dem_EventIdType eventId, Dem_InternalDataElementType internalElement, uint8 *dataBuf);
void Dem_Extension_PostPreDebounceCounterBased(Dem_EventStatusType reportedStatus, EventStatusRecType* statusRecord);
void Dem_Extension_HealedEvent(Dem_EventIdType eventId);
#if defined(DEM_EVENT_DISPLACEMENT_STRATEGY_EXTENSION)
void Dem_Extension_GetExtDataEventForDisplacement(const Dem_EventParameterType *eventParam, const ExtDataRecType *extDataBuffer, uint16 bufferSize, Dem_EventIdType *eventToRemove);
void Dem_Extension_GetEventForDisplacement(const Dem_EventParameterType *eventParam, const EventRecType *eventRecordBuffer, uint16 bufferSize, Dem_EventIdType *eventToRemove);
void Dem_Extension_GetFFEventForDisplacement(const Dem_EventParameterType *eventParam, const FreezeFrameRecType *ffBuffer, uint16 bufferSize, Dem_EventIdType *eventToRemove);
#endif

void Dem_Extension_EventDataDisplaced(Dem_EventIdType eventId);
void Dem_Extension_EventExtendedDataDisplaced(Dem_EventIdType eventId);
void Dem_Extension_EventFreezeFrameDataDisplaced(Dem_EventIdType eventId);
void Dem_Extension_PreMergeExtendedData(Dem_EventIdType eventId, boolean *UpdateAllData);
void Dem_Extension_PreTransferPreInitFreezeFrames(Dem_EventIdType eventId, boolean *removeOldRecords, Dem_DTCOriginType origin);

#endif /* DEM_EXTENSION_H_ */
