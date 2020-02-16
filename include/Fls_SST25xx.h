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









#ifndef FLS_SST25XX_H_
#define FLS_SST25XX_H_


// Only if external flash device
//#include "Spi.h"
#include "Std_Types.h"
#if defined(USE_DET)
#include "Det.h"
#endif
#include "Fls.h"

// SW ans Autosar spec versions
#define FLS_SST25XX_SW_MAJOR_VERSION    	1
#define FLS_SST25XX_SW_MINOR_VERSION   		0
#define FLS_SST25XX_SW_PATCH_VERSION    	0
#define FLS_SST25XX_AR_MAJOR_VERSION		3
#define FLS_SST25XX_AR_MINOR_VERSION		0
#define FLS_SST25XX_AR_PATCH_VERSION		2

#if 0
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
#endif

#include "Fls_SST25xx_Cfg.h"

/**
 * Initializes the Flash Driver.
 *
 * @param ConfigPtr Pointer to flash driver configuration set.
 */
void Fls_SST25xx_Init( const Fls_ConfigType *ConfigPtr );


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
#if 0
typedef uint32 Fls_AddressType;

// Specifies the number of bytes to read/write/erase/compare
//
// Note!
// Shall be the same type as Fls_AddressType because of
// arithmetic operations. Size depends on target platform and
// flash device.
typedef uint32 Fls_LengthType;
#endif


Std_ReturnType Fls_SST25xx_Erase(	Fls_AddressType   TargetAddress,
                        Fls_LengthType    Length );


Std_ReturnType Fls_SST25xx_Write (    Fls_AddressType   TargetAddress,
                        const uint8         *SourceAddressPtr,
                        Fls_LengthType    Length );

#if ( FLS_CANCEL_API == STD_ON )
void 				Fls_SST25xx_Cancel( void );
#endif

MemIf_StatusType 	Fls_SST25xx_GetStatus(	void );
MemIf_JobResultType Fls_SST25xx_GetJobResult( void );

void Fls_SST25xx_MainFunction( void );

Std_ReturnType Fls_SST25xx_Read (	Fls_AddressType SourceAddress,
              uint8 *TargetAddressPtr,
              Fls_LengthType Length );

#if ( FLS_COMPARE_API == STD_ON )
Std_ReturnType Fls_SST25xx_Compare( Fls_AddressType SourceAddress,
              uint8 *TargetAddressPtr,
              Fls_LengthType Length );
#endif

#if ( FLS_SET_MODE_API == STD_ON )
void Fls_SST25xx_SetMode( MemIf_ModeType Mode );
#endif

void Fls_SST25xx_GetVersionInfo( Std_VersionInfoType *VersioninfoPtr );


#endif /* FLS_SST25XX_H_ */
