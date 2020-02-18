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

/* !req DCM683 */

#include "Std_Types.h"
#include "Rte_Dcm_Type.h"
#include "ComStack_Types.h"

/** Enum literals for Dcm_ConfirmationStatusType */
#ifndef DCM_RES_POS_OK
#define DCM_RES_POS_OK 0U
#endif /* DCM_RES_POS_OK */

#ifndef DCM_RES_POS_NOT_OK
#define DCM_RES_POS_NOT_OK 1U
#endif /* DCM_RES_POS_NOT_OK */

#ifndef DCM_RES_NEG_OK
#define DCM_RES_NEG_OK 2U
#endif /* DCM_RES_NEG_OK */

#ifndef DCM_RES_NEG_NOT_OK
#define DCM_RES_NEG_NOT_OK 3U
#endif /* DCM_RES_NEG_NOT_OK */

/** Enum literals for Dcm_NegativeResponseCodeType */
#ifndef DCM_E_POSITIVERESPONSE
#define DCM_E_POSITIVERESPONSE 0U
#endif /* DCM_E_POSITIVERESPONSE */

#ifndef DCM_E_GENERALREJECT
#define DCM_E_GENERALREJECT 16U
#endif /* DCM_E_GENERALREJECT */

#ifndef DCM_E_SERVICENOTSUPPORTED
#define DCM_E_SERVICENOTSUPPORTED 17U
#endif /* DCM_E_SERVICENOTSUPPORTED */

#ifndef DCM_E_SUBFUNCTIONNOTSUPPORTED
#define DCM_E_SUBFUNCTIONNOTSUPPORTED 18U
#endif /* DCM_E_SUBFUNCTIONNOTSUPPORTED */

#ifndef DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT
#define DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT 19U
#endif /* DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT */

#ifndef DCM_E_RESPONSETOOLONG
#define DCM_E_RESPONSETOOLONG 20U
#endif /* DCM_E_RESPONSETOOLONG */

#ifndef DCM_E_BUSYREPEATREQUEST
#define DCM_E_BUSYREPEATREQUEST 33U
#endif /* DCM_E_BUSYREPEATREQUEST */

#ifndef DCM_E_CONDITIONSNOTCORRECT
#define DCM_E_CONDITIONSNOTCORRECT 34U
#endif /* DCM_E_CONDITIONSNOTCORRECT */

#ifndef DCM_E_REQUESTSEQUENCEERROR
#define DCM_E_REQUESTSEQUENCEERROR 36U
#endif /* DCM_E_REQUESTSEQUENCEERROR */

#ifndef DCM_E_NORESPONSEFROMSUBNETCOMPONENT
#define DCM_E_NORESPONSEFROMSUBNETCOMPONENT 37U
#endif /* DCM_E_NORESPONSEFROMSUBNETCOMPONENT */

#ifndef DCM_E_FAILUREPREVENTSEXECUTIONOFREQUESTEDACTION
#define DCM_E_FAILUREPREVENTSEXECUTIONOFREQUESTEDACTION 38U
#endif /* DCM_E_FAILUREPREVENTSEXECUTIONOFREQUESTEDACTION */

#ifndef DCM_E_REQUESTOUTOFRANGE
#define DCM_E_REQUESTOUTOFRANGE 49U
#endif /* DCM_E_REQUESTOUTOFRANGE */

#ifndef DCM_E_SECURITYACCESSDENIED
#define DCM_E_SECURITYACCESSDENIED 51U
#endif /* DCM_E_SECURITYACCESSDENIED */

#ifndef DCM_E_INVALIDKEY
#define DCM_E_INVALIDKEY 53U
#endif /* DCM_E_INVALIDKEY */

#ifndef DCM_E_EXCEEDNUMBEROFATTEMPTS
#define DCM_E_EXCEEDNUMBEROFATTEMPTS 54U
#endif /* DCM_E_EXCEEDNUMBEROFATTEMPTS */

#ifndef DCM_E_REQUIREDTIMEDELAYNOTEXPIRED
#define DCM_E_REQUIREDTIMEDELAYNOTEXPIRED 55U
#endif /* DCM_E_REQUIREDTIMEDELAYNOTEXPIRED */

#ifndef DCM_E_UPLOADDOWNLOADNOTACCEPTED
#define DCM_E_UPLOADDOWNLOADNOTACCEPTED 112U
#endif /* DCM_E_UPLOADDOWNLOADNOTACCEPTED */

#ifndef DCM_E_TRANSFERDATASUSPENDED
#define DCM_E_TRANSFERDATASUSPENDED 113U
#endif /* DCM_E_TRANSFERDATASUSPENDED */

#ifndef DCM_E_GENERALPROGRAMMINGFAILURE
#define DCM_E_GENERALPROGRAMMINGFAILURE 114U
#endif /* DCM_E_GENERALPROGRAMMINGFAILURE */

#ifndef DCM_E_WRONGBLOCKSEQUENCECOUNTER
#define DCM_E_WRONGBLOCKSEQUENCECOUNTER 115U
#endif /* DCM_E_WRONGBLOCKSEQUENCECOUNTER */

#ifndef DCM_E_SUBFUNCTIONNOTSUPPORTEDINACTIVESESSION
#define DCM_E_SUBFUNCTIONNOTSUPPORTEDINACTIVESESSION 126U
#endif /* DCM_E_SUBFUNCTIONNOTSUPPORTEDINACTIVESESSION */

#ifndef DCM_E_SERVICENOTSUPPORTEDINACTIVESESSION
#define DCM_E_SERVICENOTSUPPORTEDINACTIVESESSION 127U
#endif /* DCM_E_SERVICENOTSUPPORTEDINACTIVESESSION */

#ifndef DCM_E_RPMTOOHIGH
#define DCM_E_RPMTOOHIGH 129U
#endif /* DCM_E_RPMTOOHIGH */

#ifndef DCM_E_RPMTOOLOW
#define DCM_E_RPMTOOLOW 130U
#endif /* DCM_E_RPMTOOLOW */

#ifndef DCM_E_ENGINEISRUNNING
#define DCM_E_ENGINEISRUNNING 131U
#endif /* DCM_E_ENGINEISRUNNING */

#ifndef DCM_E_ENGINEISNOTRUNNING
#define DCM_E_ENGINEISNOTRUNNING 132U
#endif /* DCM_E_ENGINEISNOTRUNNING */

#ifndef DCM_E_ENGINERUNTIMETOOLOW
#define DCM_E_ENGINERUNTIMETOOLOW 133U
#endif /* DCM_E_ENGINERUNTIMETOOLOW */

#ifndef DCM_E_TEMPERATURETOOHIGH
#define DCM_E_TEMPERATURETOOHIGH 134U
#endif /* DCM_E_TEMPERATURETOOHIGH */

#ifndef DCM_E_TEMPERATURETOOLOW
#define DCM_E_TEMPERATURETOOLOW 135U
#endif /* DCM_E_TEMPERATURETOOLOW */

#ifndef DCM_E_VEHICLESPEEDTOOHIGH
#define DCM_E_VEHICLESPEEDTOOHIGH 136U
#endif /* DCM_E_VEHICLESPEEDTOOHIGH */

#ifndef DCM_E_VEHICLESPEEDTOOLOW
#define DCM_E_VEHICLESPEEDTOOLOW 137U
#endif /* DCM_E_VEHICLESPEEDTOOLOW */

#ifndef DCM_E_THROTTLE_PEDALTOOHIGH
#define DCM_E_THROTTLE_PEDALTOOHIGH 138U
#endif /* DCM_E_THROTTLE_PEDALTOOHIGH */

#ifndef DCM_E_THROTTLE_PEDALTOOLOW
#define DCM_E_THROTTLE_PEDALTOOLOW 139U
#endif /* DCM_E_THROTTLE_PEDALTOOLOW */

#ifndef DCM_E_TRANSMISSIONRANGENOTINNEUTRAL
#define DCM_E_TRANSMISSIONRANGENOTINNEUTRAL 140U
#endif /* DCM_E_TRANSMISSIONRANGENOTINNEUTRAL */

#ifndef DCM_E_TRANSMISSIONRANGENOTINGEAR
#define DCM_E_TRANSMISSIONRANGENOTINGEAR 141U
#endif /* DCM_E_TRANSMISSIONRANGENOTINGEAR */

#ifndef DCM_E_BRAKESWITCH_NOTCLOSED
#define DCM_E_BRAKESWITCH_NOTCLOSED 143U
#endif /* DCM_E_BRAKESWITCH_NOTCLOSED */

#ifndef DCM_E_SHIFTERLEVERNOTINPARK
#define DCM_E_SHIFTERLEVERNOTINPARK 144U
#endif /* DCM_E_SHIFTERLEVERNOTINPARK */

#ifndef DCM_E_TORQUECONVERTERCLUTCHLOCKED
#define DCM_E_TORQUECONVERTERCLUTCHLOCKED 145U
#endif /* DCM_E_TORQUECONVERTERCLUTCHLOCKED */

#ifndef DCM_E_VOLTAGETOOHIGH
#define DCM_E_VOLTAGETOOHIGH 146U
#endif /* DCM_E_VOLTAGETOOHIGH */

#ifndef DCM_E_VOLTAGETOOLOW
#define DCM_E_VOLTAGETOOLOW 147U
#endif /* DCM_E_VOLTAGETOOLOW */

/** Enum literals for Dcm_OpStatusType */
#ifndef DCM_INITIAL
#define DCM_INITIAL 0U
#endif /* DCM_INITIAL */

#ifndef DCM_PENDING
#define DCM_PENDING 1U
#endif /* DCM_PENDING */

#ifndef DCM_CANCEL
#define DCM_CANCEL 2U
#endif /* DCM_CANCEL */

#ifndef DCM_FORCE_RCRRP_OK
#define DCM_FORCE_RCRRP_OK 3U
#endif /* DCM_FORCE_RCRRP_OK */

/** Enum literals for Dcm_ProtocolType */
#ifndef DCM_OBD_ON_CAN
#define DCM_OBD_ON_CAN 0U
#endif /* DCM_OBD_ON_CAN */

#ifndef DCM_OBD_ON_FLEXRAY
#define DCM_OBD_ON_FLEXRAY 1U
#endif /* DCM_OBD_ON_FLEXRAY */

#ifndef DCM_OBD_ON_IP
#define DCM_OBD_ON_IP 2U
#endif /* DCM_OBD_ON_IP */

#ifndef DCM_UDS_ON_CAN
#define DCM_UDS_ON_CAN 3U
#endif /* DCM_UDS_ON_CAN */

#ifndef DCM_UDS_ON_FLEXRAY
#define DCM_UDS_ON_FLEXRAY 4U
#endif /* DCM_UDS_ON_FLEXRAY */

#ifndef DCM_UDS_ON_IP
#define DCM_UDS_ON_IP 5U
#endif /* DCM_UDS_ON_IP */

#ifndef DCM_ROE_ON_CAN
#define DCM_ROE_ON_CAN 6U
#endif /* DCM_ROE_ON_CAN */

#ifndef DCM_ROE_ON_FLEXRAY
#define DCM_ROE_ON_FLEXRAY 7U
#endif /* DCM_ROE_ON_FLEXRAY */

#ifndef DCM_ROE_ON_IP
#define DCM_ROE_ON_IP 8U
#endif /* DCM_ROE_ON_IP */

#ifndef DCM_PERIODICTRANS_ON_CAN
#define DCM_PERIODICTRANS_ON_CAN 9U
#endif /* DCM_PERIODICTRANS_ON_CAN */

#ifndef DCM_PERIODICTRANS_ON_FLEXRAY
#define DCM_PERIODICTRANS_ON_FLEXRAY 10U
#endif /* DCM_PERIODICTRANS_ON_FLEXRAY */

#ifndef DCM_PERIODICTRANS_ON_IP
#define DCM_PERIODICTRANS_ON_IP 11U
#endif /* DCM_PERIODICTRANS_ON_IP */

#ifndef DCM_NO_ACTIVE_PROTOCOL
#define DCM_NO_ACTIVE_PROTOCOL 12U
#endif /* DCM_NO_ACTIVE_PROTOCOL */

#ifndef DCM_SUPPLIER_1
#define DCM_SUPPLIER_1 240U
#endif /* DCM_SUPPLIER_1 */

#ifndef DCM_SUPPLIER_2
#define DCM_SUPPLIER_2 241U
#endif /* DCM_SUPPLIER_2 */

#ifndef DCM_SUPPLIER_3
#define DCM_SUPPLIER_3 242U
#endif /* DCM_SUPPLIER_3 */

#ifndef DCM_SUPPLIER_4
#define DCM_SUPPLIER_4 243U
#endif /* DCM_SUPPLIER_4 */

#ifndef DCM_SUPPLIER_5
#define DCM_SUPPLIER_5 244U
#endif /* DCM_SUPPLIER_5 */

#ifndef DCM_SUPPLIER_6
#define DCM_SUPPLIER_6 245U
#endif /* DCM_SUPPLIER_6 */

#ifndef DCM_SUPPLIER_7
#define DCM_SUPPLIER_7 246U
#endif /* DCM_SUPPLIER_7 */

#ifndef DCM_SUPPLIER_8
#define DCM_SUPPLIER_8 247U
#endif /* DCM_SUPPLIER_8 */

#ifndef DCM_SUPPLIER_9
#define DCM_SUPPLIER_9 248U
#endif /* DCM_SUPPLIER_9 */

#ifndef DCM_SUPPLIER_10
#define DCM_SUPPLIER_10 249U
#endif /* DCM_SUPPLIER_10 */

#ifndef DCM_SUPPLIER_11
#define DCM_SUPPLIER_11 250U
#endif /* DCM_SUPPLIER_11 */

#ifndef DCM_SUPPLIER_12
#define DCM_SUPPLIER_12 251U
#endif /* DCM_SUPPLIER_12 */

#ifndef DCM_SUPPLIER_13
#define DCM_SUPPLIER_13 252U
#endif /* DCM_SUPPLIER_13 */

#ifndef DCM_SUPPLIER_14
#define DCM_SUPPLIER_14 253U
#endif /* DCM_SUPPLIER_14 */

#ifndef DCM_SUPPLIER_15
#define DCM_SUPPLIER_15 254U
#endif /* DCM_SUPPLIER_15 */

/*
 * Dcm_SecLevelType
 */
#ifndef DCM_SEC_LEV_LOCKED
#define DCM_SEC_LEV_LOCKED      ((Dcm_SecLevelType)0x00)
#endif

#ifndef DCM_SEC_LEV_L1
#define DCM_SEC_LEV_L1          ((Dcm_SecLevelType)0x01)
#endif

#ifndef DCM_SEC_LEV_ALL
#define DCM_SEC_LEV_ALL         ((Dcm_SecLevelType)0xFF)
#endif

/*
 * Dcm_SesCtrlType
 */
typedef uint8 Dcm_EcuResetType;
#define DCM_HARD_RESET							((Dcm_EcuResetType)0x01)
#define DCM_KEY_OFF_ON_RESET					((Dcm_EcuResetType)0x02)
#define DCM_SOFT_RESET							((Dcm_EcuResetType)0x03)
#define DCM_ENABLE_RAPID_POWER_SHUTDOWN         ((Dcm_EcuResetType)0x04)
#define DCM_DISABLE_RAPID_POWER_SHUTDOWN        ((Dcm_EcuResetType)0x05)

#ifndef RTE_MODE_DcmEcuReset_EXECUTE
#define RTE_MODE_DcmEcuReset_EXECUTE 0U
#endif

#ifndef RTE_MODE_DcmEcuReset_HARD
#define RTE_MODE_DcmEcuReset_HARD 1U
#endif

#ifndef RTE_MODE_DcmEcuReset_JUMPTOBOOTLOADER
#define RTE_MODE_DcmEcuReset_JUMPTOBOOTLOADER 2U
#endif

#ifndef RTE_MODE_DcmEcuReset_JUMPTOSYSSUPPLIERBOOTLOADER
#define RTE_MODE_DcmEcuReset_JUMPTOSYSSUPPLIERBOOTLOADER 3U
#endif

#ifndef RTE_MODE_DcmEcuReset_KEYONOFF
#define RTE_MODE_DcmEcuReset_KEYONOFF 4U
#endif

#ifndef RTE_MODE_DcmEcuReset_NONE
#define RTE_MODE_DcmEcuReset_NONE 5U
#endif

#ifndef RTE_MODE_DcmEcuReset_SOFT
#define RTE_MODE_DcmEcuReset_SOFT 6U
#endif


#ifndef RTE_MODE_DcmCommunicationControl_DCM_ENABLE_RX_TX_NORM
#define RTE_MODE_DcmCommunicationControl_DCM_ENABLE_RX_TX_NORM 0U
#endif

#ifndef RTE_MODE_DcmCommunicationControl_DCM_ENABLE_RX_DISABLE_TX_NORM
#define RTE_MODE_DcmCommunicationControl_DCM_ENABLE_RX_DISABLE_TX_NORM 1U
#endif

#ifndef RTE_MODE_DcmCommunicationControl_DCM_DISABLE_RX_ENABLE_TX_NORM
#define RTE_MODE_DcmCommunicationControl_DCM_DISABLE_RX_ENABLE_TX_NORM 2U
#endif

#ifndef RTE_MODE_DcmCommunicationControl_DCM_DISABLE_RX_TX_NORMAL
#define RTE_MODE_DcmCommunicationControl_DCM_DISABLE_RX_TX_NORMAL 3U
#endif

#ifndef RTE_MODE_DcmCommunicationControl_DCM_ENABLE_RX_TX_NM
#define RTE_MODE_DcmCommunicationControl_DCM_ENABLE_RX_TX_NM 4U
#endif

#ifndef RTE_MODE_DcmCommunicationControl_DCM_ENABLE_RX_DISABLE_TX_NM
#define RTE_MODE_DcmCommunicationControl_DCM_ENABLE_RX_DISABLE_TX_NM 5U
#endif

#ifndef RTE_MODE_DcmCommunicationControl_DCM_DISABLE_RX_ENABLE_TX_NM
#define RTE_MODE_DcmCommunicationControl_DCM_DISABLE_RX_ENABLE_TX_NM 6U
#endif

#ifndef RTE_MODE_DcmCommunicationControl_DCM_DISABLE_RX_TX_NM
#define RTE_MODE_DcmCommunicationControl_DCM_DISABLE_RX_TX_NM 7U
#endif

#ifndef RTE_MODE_DcmCommunicationControl_DCM_ENABLE_RX_TX_NORM_NM
#define RTE_MODE_DcmCommunicationControl_DCM_ENABLE_RX_TX_NORM_NM 8U
#endif

#ifndef RTE_MODE_DcmCommunicationControl_DCM_ENABLE_RX_DISABLE_TX_NORM_NM
#define RTE_MODE_DcmCommunicationControl_DCM_ENABLE_RX_DISABLE_TX_NORM_NM 9U
#endif

#ifndef RTE_MODE_DcmCommunicationControl_DCM_DISABLE_RX_ENABLE_TX_NORM_NM
#define RTE_MODE_DcmCommunicationControl_DCM_DISABLE_RX_ENABLE_TX_NORM_NM 10U
#endif

#ifndef RTE_MODE_DcmCommunicationControl_DCM_DISABLE_RX_TX_NORM_NM
#define RTE_MODE_DcmCommunicationControl_DCM_DISABLE_RX_TX_NORM_NM 11U
#endif


/*
 * Dcm_SesCtrlType
 */
#ifndef DCM_DEFAULT_SESSION
#define DCM_DEFAULT_SESSION                     ((Dcm_SesCtrlType)0x01)
#endif
#ifndef DCM_PROGRAMMING_SESSION
#define DCM_PROGRAMMING_SESSION                 ((Dcm_SesCtrlType)0x02)
#endif
#ifndef DCM_EXTENDED_DIAGNOSTIC_SESSION
#define DCM_EXTENDED_DIAGNOSTIC_SESSION         ((Dcm_SesCtrlType)0x03)
#endif
#ifndef DCM_SAFTEY_SYSTEM_DIAGNOSTIC_SESSION
#define DCM_SAFTEY_SYSTEM_DIAGNOSTIC_SESSION    ((Dcm_SesCtrlType)0x04)
#endif
#ifndef DCM_OBD_SESSION
#define DCM_OBD_SESSION                         ((Dcm_SesCtrlType)0x05)//only used for OBD diagnostic
#endif
#ifndef DCM_ALL_SESSION_LEVEL
#define DCM_ALL_SESSION_LEVEL                   ((Dcm_SesCtrlType)0xFF)
#endif


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
#define DCM_WRITE_FORCE_RCRRP           ((Dcm_ReturnWriteMemoryType)0x03)

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
#define DCM_E_PENDING 							        (Dcm_StatusType)0x0au

typedef uint8 Dcm_PeriodicTransmitModeType;
#define DCM_PERIODICTRANSMIT_DEFAULT_MODE               (Dcm_PeriodicTransmitModeType)0x00u
#define DCM_PERIODICTRANSMIT_SLOW_MODE 				    (Dcm_PeriodicTransmitModeType)0x01u
#define DCM_PERIODICTRANSMIT_MEDIUM_MODE 				(Dcm_PeriodicTransmitModeType)0x02u
#define DCM_PERIODICTRANSMIT_FAST_MODE 					(Dcm_PeriodicTransmitModeType)0x03u
#define DCM_PERIODICTRANSMIT_STOPSENDING_MODE			(Dcm_PeriodicTransmitModeType)0x04u

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
#define DCM_RETURN_CONTROL_TO_ECU 					    (Dcm_IOControlParameterType)0x0
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

#define DCM_PERIODIC_ON_CAN             DCM_PERIODICTRANS_ON_CAN
#define DCM_PERIODIC_ON_FLEXRAY         DCM_PERIODICTRANS_ON_FLEXRAY
#define DCM_PERIODIC_ON_IP              DCM_PERIODICTRANS_ON_IP

typedef uint8 Dcm_MsgItemType;
typedef Dcm_MsgItemType* Dcm_MsgType;

typedef uint32 Dcm_MsgLenType;
//dont know what is bit type
typedef struct{
    boolean reqType:1;
    boolean suppressPosResponse:1;
}Dcm_MsgAddInfoType;
typedef uint8 Dcm_IdContextType;
typedef struct {
    Dcm_MsgType reqData;
    Dcm_MsgLenType reqDataLen;
    Dcm_MsgType resData;
    Dcm_MsgLenType resDataLen;
    Dcm_MsgAddInfoType msgAddInfo;
    Dcm_MsgLenType resMaxDataLen;
    Dcm_IdContextType idContext;
    PduIdType dcmRxPduId;
} Dcm_MsgContextType;

typedef uint8 Dcm_CommunicationModeType;
#define DCM_ENABLE_RX_TX_NORM 				(Dcm_CommunicationModeType)0x00
#define DCM_ENABLE_RX_DISABLE_TX_NORM 		(Dcm_CommunicationModeType)0x01
#define DCM_DISABLE_RX_ENABLE_TX_NORM 		(Dcm_CommunicationModeType)0x02
#define DCM_DISABLE_RX_TX_NORMAL			(Dcm_CommunicationModeType)0x03
#define DCM_ENABLE_RX_TX_NM					(Dcm_CommunicationModeType)0x04
#define DCM_ENABLE_RX_DISABLE_TX_NM			(Dcm_CommunicationModeType)0x05
#define DCM_DISABLE_RX_ENABLE_TX_NM			(Dcm_CommunicationModeType)0x06
#define DCM_DISABLE_RX_TX_NM				(Dcm_CommunicationModeType)0x07
#define DCM_ENABLE_RX_TX_NORM_NM			(Dcm_CommunicationModeType)0x08
#define DCM_ENABLE_RX_DISABLE_TX_NORM_NM	(Dcm_CommunicationModeType)0x09
#define DCM_DISABLE_RX_ENABLE_TX_NORM_NM	(Dcm_CommunicationModeType)0x0A
#define DCM_DISABLE_RX_TX_NORM_NM			(Dcm_CommunicationModeType)0x0B

typedef uint8 Dcm_EcuStartModeType;
#define DCM_COLD_START	(Dcm_EcuStartModeType)0x00
#define DCM_WARM_START	(Dcm_EcuStartModeType)0x01


/**  Dcm_RoeStateType */
#ifndef DCM_ROE_ACTIVE
#define DCM_ROE_ACTIVE 0U
#endif

#ifndef DCM_ROE_UNACTIVE
#define DCM_ROE_UNACTIVE 1U
#endif

/**  DTRStatusType  */
#ifndef DCM_DTRSTATUS_VISIBLE
#define DCM_DTRSTATUS_VISIBLE (DTRStatusType)0U
#endif

#ifndef DCM_DTRSTATUS_INVISIBLE
#define DCM_DTRSTATUS_INVISIBLE (DTRStatusType)1U
#endif

#ifndef RTE_MODE_DcmControlDTCSetting_DISABLEDTCSETTING
#define RTE_MODE_DcmControlDTCSetting_DISABLEDTCSETTING 0U
#endif

#ifndef RTE_MODE_DcmControlDTCSetting_ENABLEDTCSETTING
#define RTE_MODE_DcmControlDTCSetting_ENABLEDTCSETTING 1U
#endif

#endif /*DCM_TYPES_H_*/
