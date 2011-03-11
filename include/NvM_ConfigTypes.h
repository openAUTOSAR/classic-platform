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



#ifndef NVM_CONFIG_TYPES_H_
#define NVM_CONFIG_TYPES_H_

#include "NvM_Types.h"

/* NvM_ApiConfigClassType */	/** @req NVM491 */
#define NVM_API_CONFIG_CLASS_1	0
#define NVM_API_CONFIG_CLASS_2	1
#define NVM_API_CONFIG_CLASS_3	2

typedef enum {
	NVM_CRC16,
	NVM_CRC32
} Nvm_BlockCRCTypeType;

typedef enum {
	NVM_BLOCK_NATIVE,
	NVM_BLOCK_REDUNDANT,
	NVM_BLOCK_DATASET
} NvM_BlockManagementTypeType;

/*
 * Callback function prototypes
 */

typedef Std_ReturnType (*NvM_SingleBlockCallbackFunctionType)(uint8 ServiceId, NvM_RequestResultType JobResult);	/** @req NVM467 */
typedef void (*NvM_MultiBlockCallbackFunctionType)(uint8 ServiceId, NvM_RequestResultType JobResult);	/** @req NVM468 */
typedef Std_ReturnType (*NvM_InitBlockCallbackFunctionType)(void);	/** @req NVM469 */

/*
 * Containers and configuration parameters
 */

typedef struct {
	NvM_MultiBlockCallbackFunctionType		MultiBlockCallback;		/** @req NVM500 */
	// The rest of the parameters is realized in NvM_Cfg.h
} NvM_CommonType;

#if 0	// Currently not used
typedef struct {
//	???			EaRef;	// TODO: Check this
//	???			FeeRef;	// TODO: Check this
} NvM_TargetBlockReferenceType;
#endif

typedef struct {
	// NVRAM block global settings
	uint16								NvramBlockIdentifier;	/** @req NVM481 */	// TODO: Remove? Not needed if block nr correspond to array index.
	NvM_BlockManagementTypeType			BlockManagementType;	/** @req NVM062 */
	uint8								BlockJobPriority;		/** @req NVM477 */
	boolean								BlockWriteProt;			/** @req NVM033 */
	boolean								WriteBlockOnce;			/** @req NVM072 */
	boolean								SelectBlockForReadall;	/** @req NVM117 *//** @req NVM245 */
	boolean								ResistantToChangesSw;	/** @req NVM483 */
	NvM_SingleBlockCallbackFunctionType	SingleBlockCallback;
	uint16								NvBlockLength;			/** @req NVM479 */	// TODO: Check this

	// CRC usage of RAM and NV blocks
	boolean								BlockUseCrc;			/** @req NVM036 */
	Nvm_BlockCRCTypeType				BlockCRCType;			/** @req NVM476 */

	// RAM block, RamBlockDataAddress == NULL means temporary block otherwise permanent block
	uint8								*RamBlockDataAddress;	/** @req NVM482 */
	boolean								CalcRamBlockCrc;		/** @req NVM119 */

	// NV block, FEE/EA references
	uint8								NvBlockNum;				/** @req NVM480 */
	uint32								NvramDeviceId;			/** @req NVM035 */
	uint16								NvBlockBaseNumber;		/** @req NVM478 */

	// ROM block, reference, if RomBlockDataAdress == NULL no ROM data is available
	uint16								RomBlockNum;			/** @req NVM485 */
	uint8								*RomBlockDataAdress;	/** @req NVM484 */
	NvM_InitBlockCallbackFunctionType	InitBlockCallback;		/** @req NVM116 */

	// Containers
#if 0	// Currently not used
	NvM_TargetBlockReferenceType		TargetBlockReference;	/** @req NVM486 */
#endif
} NvM_BlockDescriptorType;	/** @req NVM061 */

typedef struct {
	// Containers
	NvM_CommonType					Common;				// 1
	const NvM_BlockDescriptorType	*BlockDescriptor;	// 1..65536
} NvM_ConfigType;

/*
 * Make the NvM_Config visible for others.
 */
extern const NvM_ConfigType NvM_Config;




#endif /*NVM_CONFIG_TYPES_H_*/
