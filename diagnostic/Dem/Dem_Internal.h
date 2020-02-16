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

#ifndef DEM_INTERNAL_H_
#define DEM_INTERNAL_H_

#define DEM_EXT_STORE_EVENT_BIT                 (uint8)(1<<0)
#define DEM_EXT_STORE_FF_BIT                    (uint8)(1<<1)
#define DEM_EXT_STORE_EXT_DATA_BIT              (uint8)(1<<2)
#define DEM_EXT_CLEAR_BEFORE_STORE_FF_BIT       (uint8)(1<<3)
#define DEM_EXT_CLEAR_BEFORE_STORE_EXT_DATA_BIT (uint8)(1<<4)


// For keeping track of the events status
typedef struct {
    const Dem_EventParameterType *eventParamRef;
    Dem_EventIdType             eventId;
    uint16                      occurrence;             /** @req DEM011 */
#if (DEM_EVENT_DISPLACEMENT_SUPPORT == STD_ON) && !defined(DEM_EVENT_DISPLACEMENT_STRATEGY_EXTENSION)
    uint32                      timeStamp;
#endif
    sint8                       UDSFdc;
    sint8                       maxUDSFdc;
#if !defined(CFG_DEM_CONFIRMATION_PROCESSING_EXTENSION)
    uint8                       confirmationCounter;
#endif
    Dem_EventStatusExtendedType eventStatusExtended;    /** @req DEM006 */
    uint8                       extensionDataStoreBitfield;
#if !defined(CFG_DEM_CONFIRMATION_PROCESSING_EXTENSION)
    boolean                     confCntrUpdatedThisConfCycle:1;
#endif
    boolean                     errorStatusChanged:1;
    boolean                     extensionDataChanged:1;
    sint16                      fdcInternal;
} EventStatusRecType;

// Types for storing different event data on event memory
typedef struct {
    Dem_EventIdType             eventId;
    uint16                      occurrence;
#if (DEM_EVENT_DISPLACEMENT_SUPPORT == STD_ON) && !defined(DEM_EVENT_DISPLACEMENT_STRATEGY_EXTENSION)
    uint32                      timeStamp;
#endif
    Dem_EventStatusExtendedType eventStatusExtended;
#if !defined(CFG_DEM_CONFIRMATION_PROCESSING_EXTENSION)
    uint8                       confirmationCounter;
#endif
} EventRecType;

typedef struct {
    Dem_EventIdType     eventId;
    uint8               data[DEM_MAX_SIZE_EXT_DATA];
#if (DEM_EVENT_DISPLACEMENT_SUPPORT == STD_ON) && !defined(DEM_EVENT_DISPLACEMENT_STRATEGY_EXTENSION)
    uint32              timeStamp;
#endif
} ExtDataRecType;

void lookupEventStatusRec(Dem_EventIdType eventId, EventStatusRecType **const eventStatusRec);
boolean isOperationCycleStarted(Dem_OperationCycleIdType opCycleId);

#endif /* DEM_INTERNAL_H_ */
