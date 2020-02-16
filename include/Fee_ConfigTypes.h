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



#ifndef FEE_CONFIG_TYPES_H_
#define FEE_CONFIG_TYPES_H_

#include "Std_Types.h"
//#include "Fee_Types.h"


/*
 * Callback function prototypes
 */

typedef void (*Fee_JobEndCallbackFunctionType)(void);	/** @req FEE098 */
typedef void (*Fee_JobErrorCallbackFunctionType)(void);	/** @req FEE099 */

/*
 * Containers and configuration parameters
 */

typedef struct {
	Fee_JobEndCallbackFunctionType		NvmJobEndCallbackNotificationCallback;
	Fee_JobErrorCallbackFunctionType	NvmJobErrorCallbackNotificationCallback;
	// The rest of the parameters is realized in Fee_Cfg.h
} Fee_GeneralType;	/** @req FEE039 */

typedef struct {
	uint8	DeviceIndex;			/** @req FEE106 */	// Needed by NvM and MemIf
	uint16	BlockNumber;			/** @req FEE107 */
	uint16	BlockSize;				/** @req FEE108 */
	boolean	ImmediateData;			/** @req FEE109 */
	uint32	NumberOfWriteCycles;	/** @req FEE110 */
} Fee_BlockConfigType;				/** @req FEE040 */

typedef struct {
	// Containers
	Fee_GeneralType					General;			// 1
	const Fee_BlockConfigType		*BlockConfig;		// 1..*
} Fee_ConfigType;

/*
 * Make the Fee_Config visible for others.
 */
extern const Fee_ConfigType Fee_Config;




#endif /*FEE_CONFIG_TYPES_H_*/
