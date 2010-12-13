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
