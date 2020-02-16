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

#ifndef EEP_H_
#define EEP_H_

#include "Std_Types.h"
#include "MemIf_Types.h"
#include "Modules.h"

/* Standard info */
#define EEP_VENDOR_ID             VENDOR_ID_ARCCORE
#define EEP_MODULE_ID						 MODULE_ID_EEP
#define EEP_SW_MAJOR_VERSION      1
#define EEP_SW_MINOR_VERSION      0
#define EEP_SW_PATCH_VERSION      0
#define EEP_AR_MAJOR_VERSION      3
#define EEP_AR_MINOR_VERSION      1
#define EEP_AR_PATCH_VERSION      5

typedef uint32 Eep_AddressType;
typedef Eep_AddressType Eep_LengthType;

/* Development errors */
// API parameter checking
#define EEP_E_PARAM_CONFIG			0x10
#define EEP_E_PARAM_ADDRESS			0x11
#define EEP_E_PARAM_DATA 			  0x12
#define EEP_E_PARAM_LENGTH			0x13
#define EEP_E_PARAM_POINTER			0x23

// EEPROM state checking
#define EEP_E_UNINIT				    0x20
#define EEP_E_BUSY					    0x21
#define EEP_E_TIMEOUT					0x22


/* Production errors */
// #define EEP_E_COM_FAILURE       0x30 /* Shall be located in DemIntErrId.h when its available */

/* Service id's for fls functions */
#define EEP_INIT_ID					    0x00
#define EEP_SETMODE_ID          0x01
#define EEP_READ_ID             0x02
#define EEP_WRITE_ID            0x03
#define EEP_ERASE_ID            0x04
#define EEP_COMPARE_ID          0x05
#define EEP_CANCEL_ID           0x06
#define EEP_GETSTATUS_ID        0x07
#define EEP_GETJOBSTATUS_ID     0x08
#define EEP_GETVERSIONINFO_ID   0x0A

#include "Eep_Cfg.h"

void Eep_Init( const Eep_ConfigType *ConfigPtr );
Std_ReturnType Eep_Erase(	Eep_AddressType   EepromAddress,
                  			Eep_LengthType    Length );


Std_ReturnType Eep_Write (    Eep_AddressType   EepromAddress,
			                  const uint8         *SourceAddressPtr,
			                  Eep_LengthType    Length );

void Eep_Cancel( void );
MemIf_StatusType 	Eep_GetStatus(	void );
MemIf_JobResultType Eep_GetJobResult( void );

void Eep_MainFunction( void );

Std_ReturnType Eep_Read (	Eep_AddressType EepromAddress,
							uint8 *TargetAddressPtr,
							Eep_LengthType Length );

Std_ReturnType Eep_Compare( Eep_AddressType EepromAddress,
							uint8 *TargetAddressPtr,
							Eep_LengthType Length );

void Eep_SetMode( MemIf_ModeType Mode );

#if ( EEP_VERSION_INFO_API == STD_ON )
#define Eep_GetVersionInfo(_vi) STD_GET_VERSION_INFO(_vi,EEP)
#endif

#endif /*EEP_H_*/
