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

#include "Dem_Extension.h"
#include "Dem.h"
#include "Std_Types.h"

void Dem_Extension_MainFunction(void)
{

}

void Dem_Extension_ClearEvent(const Dem_EventParameterType *eventParam)
{
    (void)eventParam;
}

void Dem_Extension_UpdateEventstatus(EventStatusRecType *eventStatusRecPtr, uint8 eventStatusExtendedBeforeUpdate, Dem_EventStatusType eventStatus)
{
    (void)eventStatusRecPtr;
    (void)eventStatusExtendedBeforeUpdate;
    (void)eventStatus;
}



void Dem_Extension_OperationCycleStart(Dem_OperationCycleIdType operationCycleId, EventStatusRecType *eventStatusRecPtr)
{
    (void)operationCycleId;
    (void)eventStatusRecPtr;
}

void Dem_Extension_OperationCycleEnd(Dem_OperationCycleIdType operationCycleId, EventStatusRecType *eventStatusRecPtr)
{
    (void)operationCycleId;
    (void)eventStatusRecPtr;
}

void Dem_Extension_PreInit(const Dem_ConfigType *ConfigPtr)
{
    (void)ConfigPtr;
}

void Dem_Extension_Init_PostEventMerge(Dem_DTCOriginType origin)
{

}

void Dem_Extension_Init_Complete(void)
{

}

void Dem_Extension_Shutdown(void)
{

}

void Dem_Extension_GetExtendedDataInternalElement(Dem_EventIdType eventId, Dem_InternalDataElementType internalElement, uint8 *dataBuf)
{
    (void)eventId;
    (void)internalElement;
    (void)dataBuf;
}

void Dem_Extension_PostPreDebounceCounterBased(Dem_EventStatusType reportedStatus, EventStatusRecType* statusRecord)
{
    (void)reportedStatus;
    (void)statusRecord;
}


void Dem_Extension_HealedEvent(Dem_EventIdType eventId)
{
    (void)eventId;
}

#if defined(DEM_EVENT_DISPLACEMENT_STRATEGY_EXTENSION)
void Dem_Extension_GetExtDataEventForDisplacement(const Dem_EventParameterType *eventParam, const ExtDataRecType *extDataBuffer, uint16 bufferSize, Dem_EventIdType *eventToRemove)
{
    (void)eventParam;
    (void)extDataBuffer;
    (void)bufferSize;
    (void)eventParam;
}

void Dem_Extension_GetEventForDisplacement(const Dem_EventParameterType *eventParam, const EventRecType *eventRecordBuffer, uint16 bufferSize, Dem_EventIdType *eventToRemove)
{
    (void)eventParam;
    (void)eventRecordBuffer;
    (void)bufferSize;
    (void)eventParam;
}

void Dem_Extension_GetFFEventForDisplacement(const Dem_EventParameterType *eventParam, const FreezeFrameRecType *ffBuffer, uint16 bufferSize, Dem_EventIdType *eventToRemove)
{
    (void)eventParam;
    (void)ffBuffer;
    (void)bufferSize;
    (void)eventParam;
}
#endif

void Dem_Extension_EventDataDisplaced(Dem_EventIdType eventId)
{
    (void)eventId;
}

void Dem_Extension_EventExtendedDataDisplaced(Dem_EventIdType eventId)
{
    (void)eventId;
}

void Dem_Extension_EventFreezeFrameDataDisplaced(Dem_EventIdType eventId)
{
    (void)eventId;
}

void Dem_Extension_PreMergeExtendedData(Dem_EventIdType eventId, boolean *UpdateAllData)
{
    (void)eventId;
    (void)UpdateAllData;
}
void Dem_Extension_PreTransferPreInitFreezeFrames(Dem_EventIdType eventId, boolean *removeOldRecords, Dem_DTCOriginType origin)
{
    (void)eventId;
    (void)removeOldRecords;
    (void)origin;
}

