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








#ifndef FLS_H_
#define FLS_H_

// Only if external flash device
//#include "Spi.h"
#include "Std_Types.h"
#if defined(USE_DET)
#include "Det.h"
#endif
#include "MemIf_Types.h"
#if defined(USE_DEM)
#include "Dem.h"
#endif
// SW ans Autosar spec versions
#define FLS_SW_MAJOR_VERSION    	1
#define FLS_SW_MINOR_VERSION   		0
#define FLS_SW_PATCH_VERSION    	0
#define FLS_AR_MAJOR_VERSION		3
#define FLS_AR_MINOR_VERSION		0
#define FLS_AR_PATCH_VERSION		2

// API service called with wrong parameter
#define FLS_E_PARAM_CONFIG			0x01
#define FLS_E_PARAM_ADDRESS 		0x02
#define FLS_E_PARAM_LENGTH 			0x03
#define FLS_E_PARAM_DATA 			0x04
// API service called without module initialization
#define FLS_E_UNINIT				0x05
// API service called while driver still busy
#define FLS_E_BUSY 					0x06
// Erase verification (blank check) failed
#define FLS_E_VERIFY_ERASE_FAILED	0x07
// Write verification (compare) failed
#define FLS_E_VERIFY_WRITE_FAILED	0x08

// Service id's for fls functions
#define FLS_INIT_ID					0x00
#define FLS_ERASE_ID				0x01
#define FLS_WRITE_ID				0x02
#define FLS_CANCEL_ID				0x03
#define FLS_GET_STATUS_ID			0x04
#define FLS_GET_JOB_RESULT_ID		0x05
#define FLS_MAIN_FUNCTION_ID		0x06
#define FLS_READ_ID					0x07
#define FLS_COMPARE_ID				0x08
#define FLS_SET_MODE_ID				0x09
#define FLS_GET_VERSION_INFO_ID		0x10

// Used as address offset from the configured flash base address to access a certain
// flash memory area.
typedef uint32 Fls_AddressType;

// Specifies the number of bytes to read/write/erase/compare
//
// Note!
// Shall be the same type as Fls_AddressType because of
// arithmetic operations. Size depends on target platform and
// flash device.
typedef uint32 Fls_LengthType;

#if !defined(FLS_INCLUDE_FILE)
#include "Fls_Cfg.h"

/**
 * Initializes the Flash Driver.
 *
 * @param ConfigPtr Pointer to flash driver configuration set.
 */
void Fls_Init( const Fls_ConfigType *ConfigPtr );


/**
 * Erases flash sector(s).
 *
 * @param TargetAddress Target address in flash memory.
 *                      This address offset will be
 *						added to the flash memory base address.
 *						Min.: 0
 *                      Max.: FLS_SIZE - 1
 *
 * @param Length 		Number of bytes to erase
 *                      Min.: 1
 *                      Max.: FLS_SIZE - TargetAddress
 *
 * @return E_OK: erase command has been accepted
 *         E_NOT_OK: erase command has not been accepted
 */

Std_ReturnType Fls_Erase(	Fls_AddressType   TargetAddress,
                        Fls_LengthType    Length );


Std_ReturnType Fls_Write (    Fls_AddressType   TargetAddress,
                        const uint8         *SourceAddressPtr,
                        Fls_LengthType    Length );

#if ( FLS_CANCEL_API == STD_ON )
void 				Fls_Cancel( void );
#endif

#if ( FLS_GET_STATUS_API == STD_ON )
MemIf_StatusType 	Fls_GetStatus(	void );
#endif

#if ( 1 )
MemIf_JobResultType Fls_GetJobResult( void );
#endif

void Fls_MainFunction( void );

Std_ReturnType Fls_Read (	Fls_AddressType SourceAddress,
              uint8 *TargetAddressPtr,
              Fls_LengthType Length );

#if ( FLS_COMPARE_API == STD_ON )
Std_ReturnType Fls_Compare( Fls_AddressType SourceAddress,
              uint8 *TargetAddressPtr,
              Fls_LengthType Length );
#endif

#if ( FLS_SET_MODE_API == STD_ON )
void Fls_SetMode( MemIf_ModeType Mode );
#endif

void Fls_GetVersionInfo( Std_VersionInfoType *VersioninfoPtr );

void Fls_Check( uint32 flsBaseAddress, uint32 flsTotalSize );

#else
#include FLS_INCLUDE_FILE
#endif

#endif /*FLS_H_*/
