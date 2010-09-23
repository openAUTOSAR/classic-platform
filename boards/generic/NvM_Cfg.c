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








#warning "This default file may only be used as an example!"

#include "NvM.h"

/*
 * Dummy declarations for callbacks
 */
Std_ReturnType SingleBlockFunctionCallbackBlock3(uint8 ServiceId, NvM_RequestResultType JobResult)
{
	return E_OK;
}

uint8 RamBlock2[2];
uint8 RamBlock3[4];
uint8 RamBlock5[4];


/*********************
 * NvM Configuration *
 *********************/

const NvM_BlockDescriptorType BlockDescriptorList[] = {
	{
		.NvramBlockIdentifier = 2,
		.BlockManagementType = NVM_BLOCK_NATIVE,
#if (NVM_JOB_PRIORIZATION == STD_ON)
		.BlockJobPriority = 0,
#endif
		.BlockWriteProt = STD_OFF,
		.WriteBlockOnce = STD_OFF,
		.SelectBlockForReadall = STD_OFF,
		.ResistantToChangesSw = STD_OFF,
		.SingleBlockCallback = NULL,
		.NvBlockLength = 2,

		.BlockUseCrc = STD_OFF,
		.BlockCRCType = NVM_CRC16,	// Default

		.RamBlockDataAddress = RamBlock2,
		.CalcRamBlockCrc = STD_OFF,

		.NvBlockNum = 1,	// Always 1 for native block
		.NvramDeviceId = 0,	// Ignored when only one device
		.NvBlockBaseNumber = 0,

		.RomBlockNum = 0,	// No ROM block for this NVRAM block
		.RomBlockDataAdress = NULL,
		.InitBlockCallback = NULL,

#if 0
		NvM_TargetBlockReferenceType		TargetBlockReference;	/** @req NVM486 */
#endif
		},
		{
			.NvramBlockIdentifier = 3,
			.BlockManagementType = NVM_BLOCK_NATIVE,
	#if (NVM_JOB_PRIORIZATION == STD_ON)
			.BlockJobPriority = 0,
	#endif
			.BlockWriteProt = STD_OFF,
			.WriteBlockOnce = STD_OFF,
			.SelectBlockForReadall = STD_ON,
			.ResistantToChangesSw = STD_OFF,
			.SingleBlockCallback = SingleBlockFunctionCallbackBlock3,
			.NvBlockLength = 4,

			.BlockUseCrc = STD_OFF,
			.BlockCRCType = NVM_CRC16,	// Default

			.RamBlockDataAddress = RamBlock3,
			.CalcRamBlockCrc = STD_OFF,

			.NvBlockNum = 1,	// Always 1 for native block
			.NvramDeviceId = 0,	// Ignored when only one device
			.NvBlockBaseNumber = 0,

			.RomBlockNum = 0,	// No ROM block for this NVRAM block
			.RomBlockDataAdress = NULL,
			.InitBlockCallback = NULL,

	#if 0
			NvM_TargetBlockReferenceType		TargetBlockReference;	/** @req NVM486 */
	#endif
			},
			{
				.NvramBlockIdentifier = 4,
				.BlockManagementType = NVM_BLOCK_NATIVE,
		#if (NVM_JOB_PRIORIZATION == STD_ON)
				.BlockJobPriority = 0,
		#endif
				.BlockWriteProt = STD_OFF,
				.WriteBlockOnce = STD_OFF,
				.SelectBlockForReadall = STD_OFF,
				.ResistantToChangesSw = STD_OFF,
				.SingleBlockCallback = NULL,
				.NvBlockLength = 2,

				.BlockUseCrc = STD_OFF,
				.BlockCRCType = NVM_CRC16,	// Default

				.RamBlockDataAddress =NULL,
				.CalcRamBlockCrc = STD_OFF,

				.NvBlockNum = 1,	// Always 1 for native block
				.NvramDeviceId = 0,	// Ignored when only one device
				.NvBlockBaseNumber = 0,

				.RomBlockNum = 0,	// No ROM block for this NVRAM block
				.RomBlockDataAdress = NULL,
				.InitBlockCallback = NULL,

		#if 0
				NvM_TargetBlockReferenceType		TargetBlockReference;	/** @req NVM486 */
		#endif
				},
				{
					.NvramBlockIdentifier = 5,
					.BlockManagementType = NVM_BLOCK_NATIVE,
			#if (NVM_JOB_PRIORIZATION == STD_ON)
					.BlockJobPriority = 0,
			#endif
					.BlockWriteProt = STD_OFF,
					.WriteBlockOnce = STD_OFF,
					.SelectBlockForReadall = STD_ON,
					.ResistantToChangesSw = STD_OFF,
					.SingleBlockCallback = NULL,
					.NvBlockLength = 4,

					.BlockUseCrc = STD_OFF,
					.BlockCRCType = NVM_CRC16,	// Default

					.RamBlockDataAddress = RamBlock5,
					.CalcRamBlockCrc = STD_OFF,

					.NvBlockNum = 1,	// Always 1 for native block
					.NvramDeviceId = 0,	// Ignored when only one device
					.NvBlockBaseNumber = 0,

					.RomBlockNum = 0,	// No ROM block for this NVRAM block
					.RomBlockDataAdress = NULL,
					.InitBlockCallback = NULL,

			#if 0
					NvM_TargetBlockReferenceType		TargetBlockReference;	/** @req NVM486 */
			#endif
					},
};


/*
 * NvM's config
 */
const NvM_ConfigType NvM_Config = {
		.Common = {
				.MultiBlockCallback = NULL,
		},
		.BlockDescriptor = BlockDescriptorList,
};

