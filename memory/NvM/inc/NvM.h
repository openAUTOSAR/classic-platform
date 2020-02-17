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


/** @reqSettings DEFAULT_SPECIFICATION_REVISION=4.0.3 */


/** @req NVM077 */ /* Function prototypes */


#ifndef NVM_H_
#define NVM_H_

/* @req NVM743 */
#define NVM_MODULE_ID			20u
#define NVM_VENDOR_ID			60u

#define NVM_SW_MAJOR_VERSION	1u
#define NVM_SW_MINOR_VERSION	2u
#define NVM_SW_PATCH_VERSION	1u
#define NVM_AR_RELEASE_MAJOR_VERSION	4u
#define NVM_AR_RELEASE_MINOR_VERSION	0u
#define NVM_AR_RELEASE_REVISION_VERSION	3u

#define NVM_AR_MAJOR_VERSION    NVM_AR_RELEASE_MAJOR_VERSION
#define NVM_AR_MINOR_VERSION    NVM_AR_RELEASE_MINOR_VERSION
#define NVM_AR_PATCH_VERSION    NVM_AR_RELEASE_REVISION_VERSION

/* @req NVM553 */
#include "NvM_Cfg.h"
/** @req NVM028 */ // NvmCommon configuration block is implemented in the NvM_Cfg.h file



// Error codes reported by this module defined by AUTOSAR
/* @req NVM187 */
#define NVM_E_PARAM_BLOCK_ID				0x0Au
#define NVM_E_PARAM_BLOCK_TYPE				0x0Bu
#define NVM_E_PARAM_BLOCK_DATA_IDX			0x0Cu
#define NVM_E_PARAM_ADDRESS					0x0Du
#define NVM_E_PARAM_DATA					0x0Eu
/* !req NVM023 NVM_E_PARAM_POINTER not detected (NvM_GetVersionInfo) */
//#define NVM_E_PARAM_POINTER					0x0Fu
/* @req NVM586 */
#define NVM_E_NOT_INITIALIZED				0x14u
/* @req NVM587 */
#define NVM_E_BLOCK_PENDING					0x15u
/* !req NVM590 */
//#define NVM_E_BLOCK_CONFIG					0x18u

/* Arccore extra errors */
/* @req NVM024 */
#define NVM_E_LIST_OVERFLOW                 0x50u
#define NVM_E_NV_WRITE_PROTECTED            0x51u

// Other error codes reported by this module
#define NVM_PARAM_OUT_OF_RANGE				0x40u
#define NVM_UNEXPECTED_STATE				0x41u
#define NVM_E_WRONG_CONFIG					0xfdu
#define NVM_E_UNEXPECTED_EXECUTION			0xfeu
#define NVM_E_NOT_IMPLEMENTED_YET			0xffu


// Service ID in this module
#define NVM_INIT_ID								0x00u
#define NVM_SET_DATA_INDEX_ID					0x01u
#define NVM_GET_DATA_INDEX_ID					0x02u
#define NVM_SET_BLOCK_PROTECTION_ID				0x03u
#define NVM_GET_ERROR_STATUS_ID					0x04u
#define NVM_SET_RAM_BLOCK_STATUS_ID				0x05u
#define NVM_READ_BLOCK_ID						0x06u
#define NVM_WRITE_BLOCK_ID						0x07u
#define NVM_RESTORE_BLOCK_DEFAULTS_ID			0x08u
#define NVM_ERASE_NV_BLOCK_ID					0x09u
#define NVM_CANCEL_WRITE_ALL_ID					0x0au
#define NVM_INVALIDATENV_BLOCK_ID 				0x0bu
#define NVM_READ_ALL_ID							0x0cu
#define NVM_WRITE_ALL_ID						0x0du
#define NVM_MAIN_FUNCTION_ID					0x0eu
#define NVM_GET_VERSION_INFO_ID					0x0fu

#define NVM_LOC_READ_BLOCK_ID					0x40u
#define NVM_LOC_WRITE_BLOCK_ID					0x41u
#define NVM_GLOBAL_ID							0xffu

/* APIs */
/* @req NVM149 *//* API configuration classes */
/* !req NVM560 API support missing */
/* !req NVM561 API support missing */
/* !req NVM562 API support missing */

/** @req NVM286 */
/** @req NVM650 */
#if ( NVM_VERSION_INFO_API == STD_ON )
/** @req NVM452 */
/** @req NVM285 */
/** !req NVM613 */
#define NvM_GetVersionInfo(_vi) STD_GET_VERSION_INFO(_vi, NVM)
#endif /* NVM_VERSION_INFO_API */

void NvM_MainFunction(void);	/** @req NVM464 */

void NvM_Init( void );	/** @req NVM447 */
void NvM_ReadAll( void );	/** @req NVM460 */
void NvM_WriteAll( void );	/** @req NVM461 */
void NvM_CancelWriteAll( void );
Std_ReturnType NvM_GetErrorStatus( NvM_BlockIdType blockId, NvM_RequestResultType *requestResultPtr );	/** @req NVM451 */
void NvM_SetBlockLockStatus( NvM_BlockIdType blockId, boolean blockLocked );

#if (NVM_SET_RAM_BLOCK_STATUS_API == STD_ON)
Std_ReturnType NvM_SetRamBlockStatus( NvM_BlockIdType blockId, boolean blockChanged );	/** @req NVM453 */
#endif

#if (NVM_API_CONFIG_CLASS > NVM_API_CONFIG_CLASS_1)
Std_ReturnType NvM_SetDataIndex( NvM_BlockIdType blockId, uint8 dataIndex );	/** @req NVM448 */
Std_ReturnType NvM_GetDataIndex( NvM_BlockIdType blockId, uint8 *dataIndexPtr );	/** @req NVM449 */
Std_ReturnType NvM_ReadBlock( NvM_BlockIdType blockId, void *NvM_DstPtr );	/** @req NVM454 */
Std_ReturnType NvM_WriteBlock( NvM_BlockIdType blockId, const void *NvM_SrcPtr );	/** @req NVM455 */
Std_ReturnType NvM_RestoreBlockDefaults( NvM_BlockIdType blockId, void *NvM_DstPtr );	/** @req NVM456 */
#endif

#if (NVM_API_CONFIG_CLASS > NVM_API_CONFIG_CLASS_2)
Std_ReturnType NvM_SetBlockProtection( NvM_BlockIdType blockId, boolean protectionEnabled );
Std_ReturnType NvM_EraseNvBlock( NvM_BlockIdType blockId );
Std_ReturnType NvM_InvalidateNvBlock( NvM_BlockIdType blockId );	/** @req NVM459 */
#endif




#endif /*NVM_H_*/
