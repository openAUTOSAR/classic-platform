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








#ifndef DCM_H_
#define DCM_H_

#define DCM_AR_RELEASE_MAJOR_VERSION        4u
#define DCM_AR_RELEASE_MINOR_VERSION        0u
#define DCM_AR_RELEASE_REVISION_VERSION     3u


#define DCM_MODULE_ID                       53u /* @req DCM052 */
#define DCM_VENDOR_ID                       60u

#define DCM_SW_MAJOR_VERSION    8u
#define DCM_SW_MINOR_VERSION   	7u
#define DCM_SW_PATCH_VERSION    0u
#define DCM_AR_MAJOR_VERSION    DCM_AR_RELEASE_MAJOR_VERSION
#define DCM_AR_MINOR_VERSION    DCM_AR_RELEASE_MINOR_VERSION
#define DCM_AR_PATCH_VERSION    DCM_AR_RELEASE_REVISION_VERSION

#include "Dcm_Types.h"/* @!req DCM750 Dcm_Types.h includes Rte_Dcm_Type.h */
#include "Dcm_Cfg.h"
#include "Dcm_Lcfg.h"/* !req DCM549 */
#include "ComStack_Types.h"
#include "Dcm_Cbk.h"

#if (DCM_DEV_ERROR_DETECT == STD_ON)
// Error codes produced by this module defined by Autosar
#define DCM_E_INTERFACE_TIMEOUT             0x01u
#define DCM_E_INTERFACE_VALUE_OUT_OF_RANGE  0x02u
#define DCM_E_INTERFACE_BUFFER_OVERFLOW     0x03u
#define DCM_E_INTERFACE_PROTOCOL_MISMATCH   0x04u
#define DCM_E_UNINIT                        0x05u
#define DCM_E_PARAM                         0x06u

// Other error codes reported by this module
#define DCM_E_CONFIG_INVALID                0x40u
#define DCM_E_TP_LENGTH_MISMATCH            0x50u
#define DCM_E_UNEXPECTED_RESPONSE           0x60u
#define DCM_E_UNEXPECTED_EXECUTION          0x61u
#define DCM_E_INTEGRATION_ERROR             0x62u
#define DCM_E_WRONG_BUFFER                  0x63u
#define DCM_E_NOT_SUPPORTED                 0xfeu
#define DCM_E_NOT_IMPLEMENTED_YET           0xffu

// Service IDs in this module defined by Autosar
#define DCM_START_OF_RECEPTION_ID			0x00u
#define DCM_INIT_ID							0x01u
#define DCM_COPY_RX_DATA_ID					0x02u
#define DCM_TP_RX_INDICATION_ID				0x03u
#define DCM_COPY_TX_DATA_ID					0x04u
#define DCM_TP_TX_CONFIRMATION_ID			0x05u
#define DCM_GET_SES_CTRL_TYPE_ID			0x06u
#define DCM_GET_SECURITY_LEVEL_ID			0x0du
#define DCM_GET_ACTIVE_PROTOCOL_ID			0x0fu
#define DCM_COMM_NO_COM_MODE_ENTERED_ID		0x21u
#define DCM_COMM_SILENT_COM_MODE_ENTERED_ID	0x22u
#define DCM_COMM_FULL_COM_MODE_ENTERED_ID	0x23u
#define DCM_MAIN_ID							0x25u
#define DCM_RESETTODEFAULTSESSION_ID        0x2au
#define DCM_EXTERNALSETNEGRESPONSE_ID       0x30u
#define DCM_EXTERNALPROCESSINGDONE_ID       0x31u

// Other service IDs reported by this module
#define DCM_HANDLE_RESPONSE_TRANSMISSION_ID 0x80u
#define DCM_UDS_READ_DTC_INFO_ID            0x81u
#define DCM_UDS_RESET_ID                    0x82u
#define DCM_UDS_COMMUNICATION_CONTROL_ID    0x83u
#define DCM_CHANGE_DIAGNOSTIC_SESSION_ID    0x88u
#define DCM_GLOBAL_ID                       0xffu

#endif

/*
 * Interfaces for BSW components (8.3.1)
 */
#if ( DCM_VERSION_INFO_API == STD_ON ) /* @req DCM337 */
/* @req DCM065 */
/* @req DCM335 */
/* @req DCM336 */
#define Dcm_GetVersionInfo(_vi) STD_GET_VERSION_INFO(_vi,DCM)
#endif /* DCM_VERSION_INFO_API */

void Dcm_Init( const Dcm_ConfigType *ConfigPtr ); /* @req DCM037 */


/*
 * Interfaces for BSW modules and to SW-Cs (8.3.2)
 */
Std_ReturnType Dcm_GetSecurityLevel(Dcm_SecLevelType *secLevel); /* @req DCM338 */
Std_ReturnType Dcm_GetSesCtrlType(Dcm_SesCtrlType *sesCtrlType); /* @req DCM339 */
Std_ReturnType Dcm_GetActiveProtocol(Dcm_ProtocolType *activeProtocol); /* @req DCM340 */
Std_ReturnType Dcm_ResetToDefaultSession( void ); /* @req DCM520 */


#ifdef DCM_USE_SERVICE_RESPONSEONEVENT
/* Response on Event related functions */
Std_ReturnType Dcm_TriggerOnEvent(uint8 RoeEventId); /* @req DCM521 */
Std_ReturnType Dcm_StopROE(void); /* @req DCM730 May return E_NOT_OK when no RxPduId has been saved */
Std_ReturnType Dcm_RestartROE(void); /* @req DCM731 */

Std_ReturnType Dcm_Arc_AddDataIdentifierEvent(uint16 eventTypeRecord, const uint8* serviceToRespondTo, uint8 serviceToRespondToLength);
void Dcm_Arc_InitROEBlock(void);
Std_ReturnType Dcm_Arc_GetROEPreConfig(const Dcm_ArcROEDidPreconfigType **ROEPreConfigPtr);

#endif

/*
 * Interface for basic software scheduler (8.5)
 */
void Dcm_MainFunction( void ); /* @req DCM053 */

/*
 * Dcm callouts.
 */
Dcm_ReturnWriteMemoryType Dcm_WriteMemory(Dcm_OpStatusType OpStatus, uint8 MemoryIdentifier, uint32 MemoryAddress, uint32 MemorySize, uint8* MemoryData);
Dcm_ReturnReadMemoryType Dcm_ReadMemory(Dcm_OpStatusType OpStatus, uint8 MemoryIdentifier, uint32 MemoryAddress, uint32 MemorySize, uint8* MemoryData);
void Dcm_Arc_CommunicationControl(uint8 subFunction, uint8 communicationType, Dcm_NegativeResponseCodeType *responseCode );
Std_ReturnType Dcm_ProcessRequestDownload(Dcm_OpStatusType OpStatus, uint8 DataFormatIdentifier, uint32 MemoryAddress, uint32 MemorySize, uint32 *BlockLength, Dcm_NegativeResponseCodeType* ErrorCode);
Std_ReturnType Dcm_ProcessRequestUpload(Dcm_OpStatusType OpStatus, uint8 DataFormatIdentifier, uint32 MemoryAddress, uint32 MemorySize, Dcm_NegativeResponseCodeType* ErrorCode);
Std_ReturnType Dcm_ProcessRequestTransferExit(Dcm_OpStatusType OpStatus, uint8 *ParameterRecord, uint32 ParameterRecordSize, Dcm_NegativeResponseCodeType* ErrorCode);
void Dcm_Arc_GetDownloadResponseParameterRecord(uint8 *Data, uint16 *ParameterRecordLength);
void Dcm_Arc_GetTransferExitResponseParameterRecord(uint8 *Data, uint16 *ParameterRecordLength);
Std_ReturnType Dcm_SetProgConditions(Dcm_ProgConditionsType *ProgConditions);
Dcm_EcuStartModeType Dcm_GetProgConditions(Dcm_ProgConditionsType * ProgConditions);
void Dcm_Confirmation(Dcm_IdContextType idContext,PduIdType dcmRxPduId,Dcm_ConfirmationStatusType status);

/* External diagnostic service processing */
void Dcm_ExternalSetNegResponse(Dcm_MsgContextType* pMsgContext, Dcm_NegativeResponseCodeType ErrorCode);
void Dcm_ExternalProcessingDone(Dcm_MsgContextType* pMsgContext);

#ifdef DCM_NOT_SERVICE_COMPONENT
Std_ReturnType Rte_Switch_DcmEcuReset_DcmEcuReset(Dcm_EcuResetType resetType);
Std_ReturnType Rte_Switch_DcmDiagnosticSessionControl_DcmDiagnosticSessionControl(Dcm_SesCtrlType session);
Std_ReturnType Rte_Switch_DcmControlDTCSetting_DcmControlDTCSetting(uint8 mode);
#endif
#endif /*DCM_H_*/
