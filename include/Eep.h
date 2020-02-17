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

/** @tagSettings DEFAULT_ARCHITECTURE=PPC|MPC5645S|MPC5607B  */
/** @reqSettings DEFAULT_SPECIFICATION_REVISION=4.1.2 */


#include "Std_Types.h"
#include "MemIf_Types.h"

/* Standard info */
#define EEP_VENDOR_ID                   60u
#define EEP_MODULE_ID                   90u
#define EEP_SW_MAJOR_VERSION            2u
#define EEP_SW_MINOR_VERSION            0u
#define EEP_SW_PATCH_VERSION            0u
#define EEP_AR_RELASE_MAJOR_VERSION     4u
#define EEP_AR_RELASE_MINOR_VERSION     1u
#define EEP_AR_RELASE_PATCH_VERSION     2u

typedef uint32 Eep_AddressType;
typedef Eep_AddressType Eep_LengthType;

/* Development errors */
// API parameter checking
#define EEP_E_PARAM_CONFIG          0x10u
#define EEP_E_PARAM_ADDRESS         0x11u
#define EEP_E_PARAM_DATA            0x12u
#define EEP_E_PARAM_LENGTH          0x13u
#define EEP_E_PARAM_POINTER         0x23u

// EEPROM state checking
#define EEP_E_UNINIT                0x20u
#define EEP_E_BUSY                  0x21u
#define EEP_E_TIMEOUT               0x22u


/* Production errors */
// #define EEP_E_COM_FAILURE       0x30 /* Shall be located in DemIntErrId.h when its available */

/* Service id's for fls functions */
#define EEP_INIT_ID             0x00u
#define EEP_SETMODE_ID          0x01u
#define EEP_READ_ID             0x02u
#define EEP_WRITE_ID            0x03u
#define EEP_ERASE_ID            0x04u
#define EEP_COMPARE_ID          0x05u
#define EEP_CANCEL_ID           0x06u
#define EEP_GETSTATUS_ID        0x07u
#define EEP_GETJOBSTATUS_ID     0x08u
#define EEP_GETVERSIONINFO_ID   0x0Au
#define EEP_GLOBAL_SERVICE_ID   0x10u

#include "Eep_Cfg.h"

/** @req SWS_Eep_00143 */
void Eep_Init( const Eep_ConfigType *ConfigPtr );

/** @req SWS_Eep_00147 */
Std_ReturnType Eep_Erase(	Eep_AddressType   EepromAddress,
                            Eep_LengthType    Length );


/** @req SWS_Eep_00146 */
Std_ReturnType Eep_Write (    Eep_AddressType   EepromAddress,
                              const uint8         *SourceAddressPtr,
                              Eep_LengthType    Length );

/** @req SWS_Eep_00149 */
void Eep_Cancel( void );
/** @req SWS_Eep_00150 */
MemIf_StatusType 	Eep_GetStatus(	void );
/** @req SWS_Eep_00151 */
MemIf_JobResultType Eep_GetJobResult( void );

/** @req SWS_Eep_00153 */
void Eep_MainFunction( void );

/** @req SWS_Eep_00145 */
Std_ReturnType Eep_Read (	Eep_AddressType EepromAddress,
                            uint8 *TargetAddressPtr,
                            Eep_LengthType Length );

/** @req SWS_Eep_00148 */
Std_ReturnType Eep_Compare( Eep_AddressType EepromAddress,
                            const uint8 *TargetAddressPtr,
                            Eep_LengthType Length );

/** @req SWS_Eep_00144 */
void Eep_SetMode( MemIf_ModeType Mode );

#if ( EEP_VERSION_INFO_API == STD_ON )
/** @req SWS_Eep_00152 */
void Eep_GetVersionInfo( Std_VersionInfoType *versionInfo );
#endif

#endif /*EEP_H_*/
