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
#ifndef IPDUM_H_
#define IPDUM_H_
/* @req 4.0.3/IPDUM076 The IpduM module can use macros instead of functions where source code is used and runtime is critical.*/
/* @req 4.0.3/IPDUM105 optional interfaces*/
/* @req 4.0.3/IPDUM011 Values in selector field does not have to be the whole range of possible values*/
/* @req 4.0.3/IPDUM004 A multiplexed I-PDU can contain a static part and a dynamic part.*/
/* @req 4.0.3/IPDUM005 The position of the static and the dynamic part of the multiplexer is arbitrary*/
/* @req 4.0.3/IPDUM006 The static and the dynamic part consist of more than one element.*/
/* @req 4.0.3/IPDUM007 There shall be only one selector field within one multiplexed I-PDU*/
/* @req 4.0.3/IPDUM009 The selector field of one I-PDU shall have a configurable size between one and eight contiguous bits*/
/* @req 4.0.3/IPDUM010 The position of the selector field within the I-PDU shall be defined by configuration*/
/* @req 4.0.3/IPDUM097 The IpduM shall be implemented so that no other modules depend on it*/
/* @req 4.0.3/IPDUM098 The IpduM module shall not set the selector field*/
/* @req 4.0.3/IPDUM084 The behavior of the IpduM is unspecified until a correct call to IpduM_Init is called*/
/* @req 4.0.3/IPDUM074 The code of the IpduM module shall avoid direct use of compiler and platform specific keywords*/
/* @req 4.0.3/IPDUM075 The code of the IpduM module shall indicate all global data with read-only purposes by explicitly assigning the const keyword*/
/* @req 4.0.3/IPDUM077 The IpduM module shall not define global data in the header files.*/
/* @req 4.0.3/IPDUM073 HIS compliance */
/* @req 4.0.3/IPDUM096 Satisfied by generator */
/* @req 4.0.3/IPDUM150 Satisfied by generator */

/* !req 4.0.3/IPDUM140 Optimized COM access via macros controlled by IpduMRxDirectComInvocation */
/* !req 4.0.3/IPDUM144 AUTOSAR debugging */
/* !req 4.0.3/IPDUM145 AUTOSAR debugging */
/* !req 4.0.3/IPDUM146 AUTOSAR debugging */
/* !req 4.0.3/IPDUM147 AUTOSAR debugging */
/* !req 4.0.3/IPDUM155 Satisfied by validator */

/* @req 4.0.3/IPDUM151 */
/* @req 4.0.3/IPDUM102 */
#include "ComStack_Types.h"
#include "MemMap.h"
#include "SchM_IpduM.h"

#define IPDUM_SW_MAJOR_VERSION	1u
#define IPDUM_SW_MINOR_VERSION	0u
#define IPDUM_SW_PATCH_VERSION	0u

#define IPDUM_VENDOR_ID                     60u
#define IPDUM_AR_RELEASE_MAJOR_VERSION      4u
#define IPDUM_AR_RELEASE_MINOR_VERSION      0u
#define IPDUM_AR_RELEASE_REVISION_VERSION   3u

#define IPDUM_MODULE_ID                     52u
#define IPDUM_AR_MAJOR_VERSION              IPDUM_AR_RELEASE_MAJOR_VERSION
#define IPDUM_AR_MINOR_VERSION              IPDUM_AR_RELEASE_MINOR_VERSION
#define IPDUM_AR_PATCH_VERSION              IPDUM_AR_RELEASE_REVISION_VERSION

#include "IpduM_PbCfg.h"
#include "IpduM_Cfg.h"
                                     /* @req 4.0.3/IPDUM106 */
#define IPDUM_E_PARAM			0x10u /* @req 4.0.3/IPDUM026 */
#define IPDUM_E_PARAM_POINTER	0x11u /* @req 4.0.3/IPDUM162 */
#define IPDUM_E_UNINIT			0x20u /* @req 4.0.3/IPDUM153 */

#define IPDUM_API_INIT						0x00u
#define IPDUM_API_MAIN						0x10u
#define IPDUM_API_GET_VERSION_INFO			0x01u
#define IPDUM_API_TRANSMIT					0x03u
#define IPDUM_API_RX_INDICATION				0x42u
#define IPDUM_API_TX_CONFIRMATION			0x40u
#define IPDUM_API_TRIGGER_TRANSMIT			0x41u

typedef enum {
    IPDUM_STATIC,
    IPDUM_DYNAMIC
} IpduM_PartType;

typedef enum {
    IPDUM_STATIC_PART_TRIGGER,
    IPDUM_DYNAMIC_PART_TRIGGER,
    IPDUM_STATIC_OR_DYNAMIC_PART_TRIGGER,
    IPDUM_NONE
} IpduM_TriggerType;

typedef enum {
    IPDUM_BIG_ENDIAN,
    IPDUM_LITTLE_ENDIAN
} IpduM_SignalEndianess_type;

typedef struct {
    PduLengthType	startByte;
    const uint8	*	mask;
    uint8			maskLength;
} IpduM_Segment;

typedef struct {
    IpduM_PartType			type;
    PduIdType				IpduMOutgoingPduId;
    uint8					IpduMRxSelectorValue;
    uint8       			nSegments;
    const IpduM_Segment *	segments;
} IpduM_RxPart;

typedef struct IpduM_TxPart IpduM_TxPart;
typedef struct IpduM_TxPathway IpduM_TxPathway;

struct IpduM_TxPart {
    IpduM_PartType			type;
    PduIdType 				pduId;
    boolean					IpduMJitUpdate;
    boolean					useConfirmation;
    const IpduM_TxPathway *	txPathway;
    uint8					nSegments;
    const IpduM_Segment *	segments;
};

struct IpduM_TxPathway {
    PduIdType IpduMOutgoingPduId;
    uint8 IpduMIPduUnusedAreasDefault;
    IpduM_TriggerType trigger;
    uint32 IpduMTxConfirmationTimeout;
    uint32 *timeoutCounter;
    uint8 nParts;
    const IpduM_TxPart * const *parts;
    const IpduM_TxPart * staticPart;
    const IpduM_TxPart **activeDynamicPart;
    PduLengthType pduSize;
    uint8 *buffer;
};

typedef struct {
    PduIdType IpduMSelectorFieldLength;
    PduIdType IpduMSelectorFieldPosition;
    const uint8* IpduMSelectorMask;
    PduIdType IpduMSelectorMasklength;
    const IpduM_RxPart *staticPart;
    uint8 nDynamicParts;
    const IpduM_RxPart * const *dynamicParts;
    uint8 pduSize;
    uint8 *buffer;
    IpduM_SignalEndianess_type IpduMRxIndicationEndianess;
} IpduM_RxPathway;

typedef struct { /* @req 4.0.3/IPDUM159 */
    const IpduM_TxPathway *txPathways;
    const IpduM_RxPathway *rxPathways;
    uint8 				  *txBuffer;
    const IpduM_TxPart	  *txParts;
} IpduM_ConfigType;

typedef enum {
    IPDUM_UNINIT,
    IPDUM_INIT
} IpduM_InitState;

extern const IpduM_ConfigType IpduMConfigData;

void IpduM_Init(const IpduM_ConfigType *config); /* @req 4.0.3/IPDUM032 */
Std_ReturnType IpduM_Transmit(PduIdType IpduMTxPduId, const PduInfoType *PduInfoPtr); /* @req 4.0.3/IPDUM043 */

Std_ReturnType IpduM_TriggerTransmit(PduIdType TxPduId, PduInfoType *PduInfoPtr); /* @req 4.0.3/IPDUM060 */
void IpduM_MainFunction(void); /* @req 4.0.3/IPDUM103 */

void IpduM_GetVersionInfo( Std_VersionInfoType* versioninfo );

#endif /*IPDUM_H_*/
