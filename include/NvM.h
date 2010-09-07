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








#ifndef NVM_H_
#define NVM_H_

#include "NvM_Cfg.h"
/** @req NVM028 */ // NvmCommon configuration block is implemented in the NvM_Cfg.h file
/** @req NVM491 *//** @req NVM492 *//** @req NVM493 *//** @req NVM494 *//** @req NVM495 */
/** @req NVM496 *//** @req NVM497 *//** @req NVM498 *//** @req NVM499 */
/** @req NVM501 *//** @req NVM502 *//** @req NVM503 *//** @req NVM504 *//** @req NVM505 */


#define NVM_MODULE_ID			MODULE_ID_NVM
#define NVM_VENDOR_ID			1

#define NVM_SW_MAJOR_VERSION	1
#define NVM_SW_MINOR_VERSION	0
#define NVM_SW_PATCH_VERSION	0
#define NVM_AR_MAJOR_VERSION	3
#define NVM_AR_MINOR_VERSION	0
#define NVM_AR_PATCH_VERSION	1

#if ( NVM_VERSION_INFO_API == STD_ON )	/** @req NVM452 */
#define NvM_GetVersionInfo(_vi) STD_GET_VERSION_INFO(_vi, NVM)
#endif /* NVM_VERSION_INFO_API */

void NvM_MainFunction(void);	/** @req NVM464 */

void NvM_Init( void );	/** @req NVM447 */
void NvM_ReadAll( void );	/** @req NVM460 */
void NvM_WriteAll( void );	/** @req NVM461 */
void NvM_CancelWriteAll( void );	/** @req NVM458 */
void NvM_GetErrorStatus( NvM_BlockIdType BlockId, uint8 *RequestResultPtr );	/** @req NVM451 */

#if (NVM_SET_RAM_BLOCK_STATUS_API == STD_ON)
void Nvm_SetRamBlockStatus( NvM_BlockIdType BlockId, boolean BlockChanged );	/** @req NVM453 */
#endif

#if (NVM_API_CONFIG_CLASS > NVM_API_CONFIG_CLASS_1)
void NvM_SetDataIndex( NvM_BlockIdType BlockId, uint8 DataIndex );	/** @req NVM448 */
void NvM_GetDataIndex( NvM_BlockIdType BlockId, uint8 *DataIndexPtr );	/** @req NVM449 */
void Nvm_ReadBlock( NvM_BlockIdType BlockId, uint8 *NvM_DstPtr );	/** @req NVM454 */
void NvM_WriteBlock( NvM_BlockIdType BlockId, const uint8 *NvM_SrcPtr );	/** @req NVM455 */
void Nvm_RestoreBlockDefaults( NvM_BlockIdType BlockId, uint8 *NvM_DstPtr );	/** @req NVM456 */
#endif

#if (NVM_API_CONFIG_CLASS > NVM_API_CONFIG_CLASS_2)
void NvM_SetBlockProtection( NvM_BlockIdType BlockId, boolean ProtectionEnabled );	/** @req NVM450 */
void NvM_EraseNvBlock( NvM_BlockIdType BlockId );	/** @req NVM457 */
void NvM_InvalidateNvBlock( NvM_BlockIdType BlockId );	/** @req NVM459 */
#endif

#endif /*NVM_H_*/
