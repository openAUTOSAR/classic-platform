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








#ifndef DCM_TYPES_H_
#define DCM_TYPES_H_
#include "Std_Types.h"
#include <Rte_Dcm.h>

/*
 * Dcm_SecLevelType
 */
typedef uint8 Dcm_SecLevelType;
#define DCM_SEC_LEV_LOCKED		((Dcm_SecLevelType)0x00)
#define DCM_SEC_LEV_L1			((Dcm_SecLevelType)0x01)
#define DCM_SEC_LEV_ALL			((Dcm_SecLevelType)0xFF)

/*
 * Dcm_SesCtrlType
 */
typedef uint8 Dcm_EcuResetType;
#define DCM_HARD_RESET							((Dcm_EcuResetType)0x01)
#define DCM_KEY_OFF_ON_RESET					((Dcm_EcuResetType)0x02)
#define DCM_SOFT_RESET							((Dcm_EcuResetType)0x03)

/*
 * Dcm_SesCtrlType
 */
typedef uint8 Dcm_SesCtrlType;
#define DCM_DEFAULT_SESSION						((Dcm_SesCtrlType)0x01)
#define DCM_PROGRAMMING_SESSION					((Dcm_SesCtrlType)0x02)
#define DCM_EXTENDED_DIAGNOSTIC_SESSION			((Dcm_SesCtrlType)0x03)
#define DCM_SAFTEY_SYSTEM_DIAGNOSTIC_SESSION	((Dcm_SesCtrlType)0x04)
#define DCM_OBD_SESSION							((Dcm_SesCtrlType)0x05)//only used for OBD diagnostic
#define DCM_ALL_SESSION_LEVEL					((Dcm_SesCtrlType)0xFF)

typedef uint8 Dcm_PidServiceType;
#define DCM_SERVICE_01							((Dcm_PidServiceType)0x01)
#define DCM_SERVICE_02							((Dcm_PidServiceType)0x02)
#define DCM_SERVICE_01_02						((Dcm_PidServiceType)0x03)

/*
 * Dcm_ReturnReadMemoryType
 */
typedef uint8 Dcm_ReturnReadMemoryType;
#define DCM_READ_OK						((Dcm_ReturnReadMemoryType)0x00)
#define DCM_READ_PENDING				((Dcm_ReturnReadMemoryType)0x01)
#define	DCM_READ_FAILED					((Dcm_ReturnReadMemoryType)0x02)

/*
 * Dcm_ReturnWriteMemoryType
 */
typedef uint8 Dcm_ReturnWriteMemoryType;
#define DCM_WRITE_OK					((Dcm_ReturnWriteMemoryType)0x00)
#define DCM_WRITE_PENDING				((Dcm_ReturnWriteMemoryType)0x01)
#define DCM_WRITE_FAILED				((Dcm_ReturnWriteMemoryType)0x02)

typedef uint8 Dcm_StatusType;
#define DCM_E_OK  										(Dcm_StatusType)0x00u
#define DCM_E_COMPARE_KEY_FAILED 						(Dcm_StatusType)0x01u
#define DCM_E_TI_PREPARE_LIMITS 						(Dcm_StatusType)0x02u
#define DCM_E_TI_PREPARE_INCONSTENT 					(Dcm_StatusType)0x03u
#define DCM_E_SESSION_NOT_ALLOWED 						(Dcm_StatusType)0x04u
#define DCM_E_PROTOCOL_NOT_ALLOWED 						(Dcm_StatusType)0x05u
#define DCM_E_ROE_NOT_ACCEPTED 							(Dcm_StatusType)0x06u
#define DCM_E_PERIODICID_NOT_ACCEPTED 					(Dcm_StatusType)0x07u
#define DCM_E_REQUEST_NOT_ACCEPTED 						(Dcm_StatusType)0x08u
#define DCM_E_REQUEST_ENV_NOK 							(Dcm_StatusType)0x09u

typedef uint8 Dcm_PeriodicTransmitModeType;
#define DCM_PERIODICTRANSMIT_DEFAULT_MODE      (Dcm_PeriodicTransmitModeType)0x00u 
#define DCM_PERIODICTRANSMIT_SLOWRATE_MODE 				(Dcm_PeriodicTransmitModeType)0x01u
#define DCM_PERIODICTRANSMIT_MEDIUM_MODE 					(Dcm_PeriodicTransmitModeType)0x02u
#define DCM_PERIODICTRANSMIT_FAST_MODE 					(Dcm_PeriodicTransmitModeType)0x03u
#define DCM_PERIODICTRANSMIT_STOPSENDING_MODE				(Dcm_PeriodicTransmitModeType)0x04u

typedef uint8 Dcm_DDDSubfunctionType;
#define DCM_DDD_SUBFUNCTION_DEFINEBYDID 				(Dcm_DDDSubfunctionType)0x01u
#define DCM_DDD_SUBFUNCTION_DEFINEBYADDRESS 			(Dcm_DDDSubfunctionType)0x02u
#define DCM_DDD_SUBFUNCTION_CLEAR 						(Dcm_DDDSubfunctionType)0x03u

typedef uint8 Dcm_PeriodicDidStartType;
#define	DCM_PERIODIC_TRANISMIT_STOP 					(Dcm_PeriodicDidStartType)0
#define	DCM_PERIODIC_TRANISMIT_START 					(Dcm_PeriodicDidStartType)0x01u

typedef uint8 Dcm_DDDSourceStateType;
#define DCM_DDDSOURCE_BY_DID 							(Dcm_DDDSourceStateType)0x00u
#define DCM_DDDSOURCE_BY_ADDRESS 						(Dcm_DDDSourceStateType)0x01u

typedef uint8 Dcm_IOControlParameterType;
#define DCM_RETURN_CONTROL_TO_ECU 					(Dcm_IOControlParameterType)0x0
#define DCM_RESET_TO_DEFAULT 							(Dcm_IOControlParameterType)0x01
#define DCM_FREEZE_CURRENT_STATE						(Dcm_IOControlParameterType)0x02
#define DCM_SHORT_TERM_ADJUSTMENT						(Dcm_IOControlParameterType)0x03

typedef struct {
	uint8  ProtocolId;
	uint8  TesterSourceAdd;
	uint8  Sid;
	uint8  SubFncId;
	boolean  ReprogramingRequest;
	boolean  ApplUpdated;
	boolean  ResponseRequired;
} Dcm_ProgConditionsType;

#endif /*DCM_TYPES_H_*/
