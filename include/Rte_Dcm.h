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

#ifndef RTE_DCM_H_
#define RTE_DCM_H_

typedef uint8 Dcm_ConfirmationStatusType;
#define DCM_RES_POS_OK                          ((Dcm_ConfirmationStatusType)0x00)
#define DCM_RES_POS_NOT_OK                      ((Dcm_ConfirmationStatusType)0x01)
#define DCM_RES_NEG_OK                          ((Dcm_ConfirmationStatusType)0x02)
#define DCM_RES_NEG_NOT_OK                      ((Dcm_ConfirmationStatusType)0x03)

/*
 * Dcm_NegativeResponseCodeType
 */
#if !defined(_DEFINED_TYPEDEF_FOR_NegativeResponseCodeType_)
typedef uint8 Dcm_NegativeResponseCodeType;
#define DCM_E_GENERALREJECT                             ((Dcm_NegativeResponseCodeType)0x10)
#define DCM_E_BUSYREPEATREQUEST                         ((Dcm_NegativeResponseCodeType)0x21)
#define DCM_E_CONDITIONSNOTCORRECT                      ((Dcm_NegativeResponseCodeType)0x22)
#define DCM_E_REQUESTSEQUENCEERROR                      ((Dcm_NegativeResponseCodeType)0x24)
#define DCM_E_REQUESTOUTOFRANGE                         ((Dcm_NegativeResponseCodeType)0x31)
#define DCM_E_SECURITYACCESSDENIED                      ((Dcm_NegativeResponseCodeType)0x33)
#define DCM_E_INVALIDKEY                                ((Dcm_NegativeResponseCodeType)0x35)
#define DCM_E_EXCEEDNUMBEROFATTEMPTS                    ((Dcm_NegativeResponseCodeType)0x36)
#define DCM_E_REQUIREDTIMEDELAYNOTEXPIRED               ((Dcm_NegativeResponseCodeType)0x37)
#define DCM_E_GENERALPROGRAMMINGFAILURE                 ((Dcm_NegativeResponseCodeType)0x72)
#define DCM_E_SUBFUNCTIONNOTSUPPORTEDINACTIVESESSION    ((Dcm_NegativeResponseCodeType)0x7E)
#define DCM_E_RPMTOOHIGH                                ((Dcm_NegativeResponseCodeType)0x81)
#define DCM_E_RPMTOLOW                                  ((Dcm_NegativeResponseCodeType)0x82)
#define DCM_E_ENGINEISRUNNING                           ((Dcm_NegativeResponseCodeType)0x83)
#define DCM_E_ENGINEISNOTRUNNING                        ((Dcm_NegativeResponseCodeType)0x84)
#define DCM_E_ENGINERUNTIMETOOLOW                       ((Dcm_NegativeResponseCodeType)0x85)
#define DCM_E_TEMPERATURETOOHIGH                        ((Dcm_NegativeResponseCodeType)0x86)
#define DCM_E_TEMPERATURETOOLOW                         ((Dcm_NegativeResponseCodeType)0x87)
#define DCM_E_VEHICLESPEEDTOOHIGH                       ((Dcm_NegativeResponseCodeType)0x88)
#define DCM_E_VEHICLESPEEDTOOLOW                        ((Dcm_NegativeResponseCodeType)0x89)
#define DCM_E_THROTTLE_PEDALTOOHIGH                     ((Dcm_NegativeResponseCodeType)0x8A)
#define DCM_E_THROTTLE_PEDALTOOLOW                      ((Dcm_NegativeResponseCodeType)0x8B)
#define DCM_E_TRANSMISSIONRANGENOTINNEUTRAL             ((Dcm_NegativeResponseCodeType)0x8C)
#define DCM_E_TRANSMISSIONRANGENOTINGEAR                ((Dcm_NegativeResponseCodeType)0x8D)
#define DCM_E_BRAKESWITCH_NOTCLOSED                     ((Dcm_NegativeResponseCodeType)0x8F)
#define DCM_E_SHIFTERLEVERNOTINPARK                     ((Dcm_NegativeResponseCodeType)0x90)
#define DCM_E_TORQUECONVERTERCLUTCHLOCKED               ((Dcm_NegativeResponseCodeType)0x91)
#define DCM_E_VOLTAGETOOHIGH                            ((Dcm_NegativeResponseCodeType)0x92)
#define DCM_E_VOLTAGETOOLOW                             ((Dcm_NegativeResponseCodeType)0x93)

#define DCM_E_POSITIVERESPONSE                          ((Dcm_NegativeResponseCodeType)0x00)
#define DCM_E_SERVICENOTSUPPORTED                       ((Dcm_NegativeResponseCodeType)0x11)
#define DCM_E_SUBFUNCTIONNOTSUPPORTED                   ((Dcm_NegativeResponseCodeType)0x12)
#define DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT     ((Dcm_NegativeResponseCodeType)0x13)
#define DCM_E_SERVICENOTSUPPORTEDINACTIVESESSION        ((Dcm_NegativeResponseCodeType)0x7F)
#endif

/*
 * Dcm_OpStatusType
 */
typedef uint8 Dcm_OpStatusType;
#define DCM_INITIAL                     ((Dcm_OpStatusType)0x00)
#define DCM_PENDING                     ((Dcm_OpStatusType)0X01)
#define DCM_CANCEL                      ((Dcm_OpStatusType)0x02)
#define DCM_FORCE_RCRRP_OK              ((Dcm_OpStatusType)0X03)

/*
 * Dcm_ProtocolType
 */
typedef uint8 Dcm_ProtocolType;
#define DCM_OBD_ON_CAN                  ((Dcm_ProtocolType)0x00)
#define DCM_OBD_ON_FLEXRAY              ((Dcm_ProtocolType)0x01)
#define DCM_OBD_ON_IP                   ((Dcm_ProtocolType)0x02)
#define DCM_UDS_ON_CAN                  ((Dcm_ProtocolType)0x03)
#define DCM_UDS_ON_FLEXRAY              ((Dcm_ProtocolType)0x04)
#define DCM_UDS_ON_IP                   ((Dcm_ProtocolType)0x05)
#define DCM_ROE_ON_CAN                  ((Dcm_ProtocolType)0x06)
#define DCM_ROE_ON_FLEXRAY              ((Dcm_ProtocolType)0x07)
#define DCM_ROE_ON_IP                   ((Dcm_ProtocolType)0x08)
#define DCM_PERIODICTRANS_ON_CAN        ((Dcm_ProtocolType)0x09)
#define DCM_PERIODICTRANS_ON_FLEXRAY    ((Dcm_ProtocolType)0x0A)
#define DCM_PERIODICTRANS_ON_IP         ((Dcm_ProtocolType)0x0B)

#define DCM_UDS_ON_IP               ((Dcm_ProtocolType)0x05)


#endif /*RTE_DCM_H_*/
