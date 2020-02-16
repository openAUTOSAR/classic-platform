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








#warning "This default file may only be used as an example!"

#ifndef DEM_CFG_H_
#define DEM_CFG_H_
/*
 * DEM General
 */
#define DEM_VERSION_INFO_API    			STD_ON		// Activate/Deactivate ver info API.
#define DEM_DEV_ERROR_DETECT				STD_ON		// Activate/Deactivate Dev Error Detection and Notification.
#define DEM_OBD_SUPPORT						STD_OFF
#define DEM_PTO_SUPPORT						STD_OFF
#define DEM_TYPE_OF_DTC_SUPPORTED			0x01		// ISO14229-1
#define DEM_DTC_STATUS_AVAILABILITY_MASK	0xFF
#define DEM_CLEAR_ALL_EVENTS				STD_OFF		// All event or only events with DTC is cleared with Dem_ClearDTC


#define DEM_BSW_ERROR_BUFFER_SIZE			20	// Max nr of elements in BSW error buffer (0..255)
#define DEM_FF_DID_LENGTH					TBD	// Length of DID & PID of FreezeFrames in Bytes.
#define DEM_MAX_NUMBER_EVENT_ENTRY_MIR		0	// Max nr of events stored in mirror memory.
#define DEM_MAX_NUMBER_EVENT_ENTRY_PER		0	// Max nr of events stored in permanent memory.
#define DEM_MAX_NUMBER_EVENT_ENTRY_PRI		10	// Max nr of events stored in primary memory.
#define DEM_MAX_NUMBER_EVENT_ENTRY_SEC		0	// Max nr of events stored in secondary memory.
#define DEM_MAX_NUMBER_PRESTORED_FF			0	// Max nr of prestored FreezeFrames. 0=Not supported.

/*
 * Size limitations of the types derived from DemGeneral
 */
#define DEM_MAX_NR_OF_RECORDS_IN_EXTENDED_DATA	10	// 0..253 according to Autosar
#define DEM_MAX_NR_OF_EVENT_DESTINATION			 4	// 0..4 according to Autosar

/*
 * Size limitations of storage area
 */
#define DEM_MAX_SIZE_FF_DATA					 10	// Max number of bytes in one freeze frame
#define DEM_MAX_SIZE_EXT_DATA					 10	// Max number of bytes in one extended data record
#define DEM_MAX_NUMBER_EVENT					100	// Max number of events to keep status on

#define DEM_MAX_NUMBER_EVENT_PRE_INIT			 20	// Max number of events status to keep before init
#define DEM_MAX_NUMBER_FF_DATA_PRE_INIT			 20	// Max number of freeze frames to store before init
#define DEM_MAX_NUMBER_EXT_DATA_PRE_INIT		 20	// Max number of extended data to store before init

#define DEM_MAX_NUMBER_EVENT_PRI_MEM			(DEM_MAX_NUMBER_EVENT_ENTRY_PRI)	// Max number of events status to store in primary memory
#define DEM_MAX_NUMBER_FF_DATA_PRI_MEM			5									// Max number of freeze frames to store in primary memory
#define DEM_MAX_NUMBER_EXT_DATA_PRI_MEM			5									// Max number of extended data to store in primary memory

#endif /*DEM_CFG_H_*/
