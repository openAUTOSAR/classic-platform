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

/* @req 4.0.3/IPDUM076 */
/* @req 4.0.3/IPDUM078 */
/* @req 4.0.3/IPDUM107 */
/* @req 4.0.3/IPDUM104 */
/* @req 4.0.3/IPDUM105 */
/* @req 4.0.3/IPDUM011 */
/* @req 4.0.3/IPDUM004 */
/* @req 4.0.3/IPDUM005 */
/* @req 4.0.3/IPDUM006 */
/* @req 4.0.3/IPDUM007 */
/* @req 4.0.3/IPDUM010 */
/* @req 4.0.3/IPDUM097 */
/* @req 4.0.3/IPDUM098 */
/* @req 4.0.3/IPDUM084 */
/* @req 4.0.3/IPDUM074 */
/* @req 4.0.3/IPDUM075 */
/* @req 4.0.3/IPDUM095 Satisfied by generator */
/* @req 4.0.3/IPDUM096 Satisfied by generator */
/* @req 4.0.3/IPDUM149 Satisfied by generator */
/* @req 4.0.3/IPDUM150 Satisfied by generator */

/* !req 4.0.3/IPDUM165 imported modules version checks */
/* !req 4.0.3/IPDUM073 Misra compatibility */
/* !req 4.0.3/IPDUM140 Optimized COM access via macros controlled by IpduMRxDirectComInvocation */
/* !req 4.0.3/IPDUM144 AUTOSAR debugging */
/* !req 4.0.3/IPDUM145 AUTOSAR debugging */
/* !req 4.0.3/IPDUM146 AUTOSAR debugging */
/* !req 4.0.3/IPDUM147 AUTOSAR debugging */
/* !req 4.0.3/IPDUM155 Satisfied by validator */
/* !req 4.0.3/IPDUM166 Satisfied by validator */
/* !req 4.0.3/IPDUM009 Satisfied by validator */

/* @req 4.0.3/IPDUM151 */
/* @req 4.0.3/IPDUM102 */
#include "ComStack_Types.h"
#include "MemMap.h"
//#include "SchM_IpduM.h"
#include "IpduM_PbCfg.h"

#define IPDUM_SW_MAJOR_VERSION	1
#define IPDUM_SW_MINOR_VERSION	0
#define IPDUM_SW_PATCH_VERSION	0

									 /* @req 4.0.3/IPDUM106 */
#define IPDUM_E_PARAM			0x10 /* @req 4.0.3/IPDUM026 */
#define IPDUM_E_PARAM_POINTER	0x11 /* @req 4.0.3/IPDUM162 */
#define IPDUM_E_UNINIT			0x20 /* @req 4.0.3/IPDUM153 */

#define IPDUM_API_INIT						0x00
#define IPDUM_API_MAIN						0x10
#define IPDUM_API_GET_VERSION_INFO			0x01
#define IPDUM_API_TRANSMIT					0x03
#define IPDUM_API_RX_INDICATION				0x42
#define IPDUM_API_TX_CONFIRMATION			0x40
#define IPDUM_API_TRIGGER_TRANSMIT			0x41

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

typedef struct {
	PduLengthType	startByte;
	const uint8	*	mask;
	uint8			maskLength;
} IpduM_Segment;

typedef struct {
	PduIdType	IpduMOutgoingDynamicPduId;
	uint8		IpduMRxSelectorValue;
} IpduM_RxDynamicPart;

typedef struct IpduM_TxPart IpduM_TxPart;
typedef struct IpduM_TxPathway IpduM_TxPathway;

struct IpduM_TxPart {
	IpduM_PartType			type;
	PduIdType 				pduId;
	boolean					IpduMJitUpdate;
	boolean					useConfirmation;
	const IpduM_TxPathway *	txPathway;
	uint16					nSegments;
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
	boolean hasStaticPart;
	PduIdType IpduMOutgoingStaticPduId;
	uint8 nDynamicParts;
	const IpduM_RxDynamicPart *dynamicParts;
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

/* @req 4.0.3/IPDUM037 */
/* @req 4.0.3/IPDUM038 */
/* @req 4.0.3/IPDUM085 */
#if IPDUM_VERSION_INFO_API == STD_ON /* @req 4.0.3/IPDUM039 */
#define IpduM_GetVersionInfo(_vi) STD_GET_VERSION_INFO(_vi,IPDUM)
#else
#define IpduM_GetVersionInfo(_vi)
#endif

void IpduM_Init(const IpduM_ConfigType *config); /* @req 4.0.3/IPDUM032 */
Std_ReturnType IpduM_Transmit(PduIdType IpduMTxPduId, const PduInfoType *PduInfoPtr); /* @req 4.0.3/IPDUM043 */
void IpduM_RxIndication(PduIdType RxPduId, PduInfoType *PduInfoPtr); /* @req 4.0.3/IPDUM040 */
void IpduM_TxConfirmation(PduIdType TxPduId); /* @req 4.0.3/IPDUM044 */
Std_ReturnType IpduM_TriggerTransmit(PduIdType TxPduId, PduInfoType *PduInfoPtr); /* @req 4.0.3/IPDUM060 */
void IpduM_MainFunction(void); /* @req 4.0.3/IPDUM103 */
