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








/** @req NVM077 */


#ifndef NVM_H_
#define NVM_H_

#define NVM_MODULE_ID			MODULE_ID_NVM
#define NVM_VENDOR_ID			1

#define NVM_SW_MAJOR_VERSION	1
#define NVM_SW_MINOR_VERSION	0
#define NVM_SW_PATCH_VERSION	0
#define NVM_AR_MAJOR_VERSION	3
#define NVM_AR_MINOR_VERSION	0
#define NVM_AR_PATCH_VERSION	1

#include "NvM_Cfg.h"
/** @req NVM028 */ // NvmCommon configuration block is implemented in the NvM_Cfg.h file
/** @req NVM491 *//** @req NVM492 *//** @req NVM493 *//** @req NVM494 *//** @req NVM495 */
/** @req NVM496 *//** @req NVM497 *//** @req NVM498 *//** @req NVM499 */
/** @req NVM501 *//** @req NVM502 *//** @req NVM503 *//** @req NVM504 *//** @req NVM505 */


#if (NVM_DEV_ERROR_DETECT == STD_ON)
// Error codes reported by this module defined by AUTOSAR
#define NVM_E_PARAM_BLOCK_ID				0x0A
#define NVM_E_PARAM_BLOCK_TYPE				0x0B
#define NVM_E_PARAM_BLOCK_DATA_IDX			0x0C
#define NVM_E_PARAM_ADDRESS					0x0D
#define NVM_E_PARAM_DATA					0x0E
#define NVM_E_NOT_INITIALIZED				0x14
#define NVM_E_BLOCK_PENDING					0x15
#define NVM_E_LIST_OVERFLOW					0x16
#define NVM_E_NV_WRITE_PROTECTED			0x17
#define NVM_E_BLOCK_CONFIG					0x18


// Other error codes reported by this module
#define NVM_PARAM_OUT_OF_RANGE				0x40
#define NVM_UNEXPECTED_STATE				0x41
#define NVM_E_WRONG_CONFIG					0xfd
#define NVM_E_UNEXPECTED_EXECUTION			0xfe
#define NVM_E_NOT_IMPLEMENTED_YET			0xff

#endif
// Service ID in this module
#define NVM_INIT_ID								0x00
#define NVM_SET_DATA_INDEX_ID					0x01
#define NVM_GET_DATA_INDEX_ID					0x02
#define NVM_SET_BLOCK_PROTECTION_ID				0x03
#define NVM_GET_ERROR_STATUS_ID					0x04
#define NVM_SET_RAM_BLOCK_STATUS_ID				0x05
#define NVM_READ_BLOCK_ID						0x06
#define NVM_WRITE_BLOCK_ID						0x07
#define NVM_RESTORE_BLOCK_DEFAULTS_ID			0x08
#define NVM_ERASE_NV_BLOCK_ID					0x09
#define NVM_CANCEL_WRITE_ALL_ID					0x0a
#define NVM_INVALIDATENV_BLOCK_ID 				0x0b
#define NVM_READ_ALL_ID							0x0c
#define NVM_WRITE_ALL_ID						0x0d
#define NVM_MAIN_FUNCTION_ID					0x0e
#define NVM_GET_VERSION_INFO_ID					0x0f

#define NVM_LOC_READ_BLOCK_ID					0x40
#define NVM_LOC_WRITE_BLOCK_ID					0x41
#define NVM_GLOBAL_ID							0xff



#if ( NVM_VERSION_INFO_API == STD_ON )	/** @req NVM452 */
#define NvM_GetVersionInfo(_vi) STD_GET_VERSION_INFO(_vi, NVM)
#endif /* NVM_VERSION_INFO_API */

void NvM_MainFunction(void);	/** @req NVM464 */

void NvM_Init( void );	/** @req NVM447 */
void NvM_ReadAll( void );	/** @req NVM460 */
void NvM_WriteAll( void );	/** @req NVM461 */
void NvM_CancelWriteAll( void );	/** @req NVM458 */
Std_ReturnType NvM_GetErrorStatus( NvM_BlockIdType blockId, NvM_RequestResultType *requestResultPtr );	/** @req NVM451 */
void NvM_SetBlockLockStatus( NvM_BlockIdType blockId, boolean blockLocked );

#if (NVM_SET_RAM_BLOCK_STATUS_API == STD_ON)
Std_ReturnType NvM_SetRamBlockStatus( NvM_BlockIdType blockId, boolean blockChanged );	/** @req NVM453 */
#endif

#if (NVM_API_CONFIG_CLASS > NVM_API_CONFIG_CLASS_1)
Std_ReturnType NvM_SetDataIndex( NvM_BlockIdType blockId, uint8 dataIndex );	/** @req NVM448 */
Std_ReturnType NvM_GetDataIndex( NvM_BlockIdType blockId, uint8 *dataIndexPtr );	/** @req NVM449 */
Std_ReturnType NvM_ReadBlock( NvM_BlockIdType blockId, uint8 *dstPtr );	/** @req NVM454 */
Std_ReturnType NvM_WriteBlock( NvM_BlockIdType blockId, const uint8 *srcPtr );	/** @req NVM455 */
Std_ReturnType NvM_RestoreBlockDefaults( NvM_BlockIdType blockId, uint8 *dstPtr );	/** @req NVM456 */
#endif

#if (NVM_API_CONFIG_CLASS > NVM_API_CONFIG_CLASS_2)
Std_ReturnType NvM_SetBlockProtection( NvM_BlockIdType blockId, boolean protectionEnabled );	/** @req NVM450 */
Std_ReturnType NvM_EraseNvBlock( NvM_BlockIdType blockId );	/** @req NVM457 */
Std_ReturnType NvM_InvalidateNvBlock( NvM_BlockIdType blockId );	/** @req NVM459 */
#endif




#endif /*NVM_H_*/
