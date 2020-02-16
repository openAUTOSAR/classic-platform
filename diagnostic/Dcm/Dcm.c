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


// 904 PC-Lint: OK. Allow VALIDATE, VALIDATE_RV and VALIDATE_NO_RV to return value.
//lint -emacro(904,VALIDATE_RV,VALIDATE_NO_RV,VALIDATE)


/*
 *  General requirements
 */
/** @req DCM054.Partially */ /** @req DCM055.Partially */ /** @req DCM110 */ /** @req DCM107 */ /** @req DCM332 */
/** @req DCM012 */ /** @req DCM044 */ /** @req DCM364 */ /** @req DCM041 */
/** @req DCM042 */ /** @req DCM049 */
/** @req DCM033 */
/** @req DCM171 */
/** @req DCM333 */ /** @req DCM334 */
/** @req DCM018 */
/** @req DCM048 */
 /** @req DCM040.Uninit */
 /** @req DCM040.Param */
 /** @req DCM043 */
 /** @req DCM048 */

#include <string.h>
#include "Dcm.h"
#include "Dcm_Internal.h"
#if defined(USE_DEM)
#if defined(DCM_USE_SERVICE_CLEARDIAGNOSTICINFORMATION) || defined(DCM_USE_SERVICE_READDTCINFORMATION) || defined(DCM_USE_SERVICE_CONTROLDTCSETTING)
#include "Dem.h"
#endif
#endif

#include "MemMap.h"
//#include "SchM_Dcm.h"
//#include "ComM_Dcm.h"
#include "PduR_Dcm.h"
#include "ComStack_Types.h"


// State variable
typedef enum
{
  DCM_UNINITIALIZED = 0,
  DCM_INITIALIZED
} Dcm_StateType;

//lint -esym(551,dcmState)	PC-Lint - Turn of warning of dcmState not accessed when having DCM_DEV_ERROR_DETECT to STD_OFF
static Dcm_StateType dcmState = DCM_UNINITIALIZED;

/* Global configuration */
const Dcm_ConfigType *Dcm_ConfigPtr;
/*********************************************
 * Interface for upper layer modules (8.3.1) *
 *********************************************/

/*
 * Procedure:	Dcm_GetVersionInfo
 * Reentrant:	Yes
 */
// Defined in Dcm.h


/*
 * Procedure:	Dcm_Init
 * Reentrant:	No
 */
void Dcm_Init(const Dcm_ConfigType *ConfigPtr) /** @req DCM037 */
{
	VALIDATE_NO_RV(((NULL != ConfigPtr) && (NULL != ConfigPtr->Dsl) && (NULL != ConfigPtr->Dsd) && (NULL != ConfigPtr->Dsp)), DCM_INIT_ID, DCM_E_CONFIG_INVALID);

	Dcm_ConfigPtr = ConfigPtr;
	DslInit();
	DsdInit();
	DspInit(TRUE);

	dcmState = DCM_INITIALIZED;

	return;
}


/*
 * Interface for basic software scheduler
 */
void Dcm_MainFunction(void) /** @req DCM362 */
{
	VALIDATE_NO_RV(dcmState == DCM_INITIALIZED, DCM_MAIN_ID, DCM_E_UNINIT);

	DsdMain();
	DspMain();
	DslMain();
}


/***********************************************
 * Interface for BSW modules and SW-Cs (8.3.2) *
 ***********************************************/
BufReq_ReturnType Dcm_StartOfReception(PduIdType dcmRxPduId, PduLengthType tpSduLength, PduLengthType *rxBufferSizePtr)
{
	BufReq_ReturnType returnCode;

	VALIDATE_RV(dcmState == DCM_INITIALIZED, DCM_START_OF_RECEPTION_ID, DCM_E_UNINIT, BUFREQ_NOT_OK);
	VALIDATE_RV(dcmRxPduId < DCM_DSL_RX_PDU_ID_LIST_LENGTH, DCM_START_OF_RECEPTION_ID, DCM_E_PARAM, BUFREQ_NOT_OK);

	returnCode = DslStartOfReception(dcmRxPduId, tpSduLength, rxBufferSizePtr);

	return returnCode;
}

BufReq_ReturnType Dcm_CopyRxData(PduIdType dcmRxPduId, PduInfoType *pduInfoPtr, PduLengthType *rxBufferSizePtr)
{
	VALIDATE_RV(dcmState == DCM_INITIALIZED, DCM_COPY_RX_DATA_ID, DCM_E_UNINIT, BUFREQ_NOT_OK);
	VALIDATE_RV(dcmRxPduId < DCM_DSL_RX_PDU_ID_LIST_LENGTH, DCM_COPY_RX_DATA_ID, DCM_E_PARAM, BUFREQ_NOT_OK);

	return DslCopyDataToRxBuffer(dcmRxPduId, pduInfoPtr, rxBufferSizePtr);
}

void Dcm_TpRxIndication(PduIdType dcmRxPduId, NotifResultType result)
{
	VALIDATE_NO_RV(dcmState == DCM_INITIALIZED, DCM_TP_RX_INDICATION_ID, DCM_E_UNINIT);
	VALIDATE_NO_RV(dcmRxPduId < DCM_DSL_RX_PDU_ID_LIST_LENGTH, DCM_TP_RX_INDICATION_ID, DCM_E_PARAM);

	DslTpRxIndicationFromPduR(dcmRxPduId, result);
}


Std_ReturnType Dcm_GetActiveProtocol(Dcm_ProtocolType *activeProtocol)
{
	Std_ReturnType returnCode;

	VALIDATE_RV(dcmState == DCM_INITIALIZED, DCM_GET_ACTIVE_PROTOCOL_ID, DCM_E_UNINIT, E_NOT_OK);

	/* According to 4.0.3 spec. E_OK should always be returned.
	 * But if there is no active protocol? */
	returnCode = DslGetActiveProtocol(activeProtocol);

	return returnCode;
}


Std_ReturnType Dcm_GetSecurityLevel(Dcm_SecLevelType *secLevel)
{
	VALIDATE_RV(dcmState == DCM_INITIALIZED, DCM_GET_SECURITY_LEVEL_ID, DCM_E_UNINIT, E_NOT_OK);
	/* According to 4.0.3 spec. E_OK should always be returned.
	 * So if we cannot get the current security level using DslGetSecurityLevel,
	 * and this probably due to that there is no active protocol,
	 * we report the default security level according to DCM033 */
	if( E_OK != DslGetSecurityLevel(secLevel) ) {
		*secLevel = DCM_SEC_LEV_LOCKED;
	}
	return E_OK;
}


Std_ReturnType Dcm_GetSesCtrlType(Dcm_SesCtrlType *sesCtrlType)
{
	VALIDATE_RV(dcmState == DCM_INITIALIZED, DCM_GET_SES_CTRL_TYPE_ID, DCM_E_UNINIT, E_NOT_OK);
	/* According to 4.0.3 spec. E_OK should always be returned.
	 * So if we cannot get the current session using DslGetSesCtrlType,
	 * and this probably due to that there is no active protocol,
	 * we report the default session according to  DCM034 */
	if( E_OK !=  DslGetSesCtrlType(sesCtrlType) ) {
		*sesCtrlType = DCM_DEFAULT_SESSION;
	}
	return E_OK;
}

void Dcm_TpTxConfirmation(PduIdType dcmTxPduId, NotifResultType result)
{
	VALIDATE_NO_RV(dcmState == DCM_INITIALIZED, DCM_TP_TX_CONFIRMATION_ID, DCM_E_UNINIT);
	VALIDATE_NO_RV(dcmTxPduId < DCM_DSL_TX_PDU_ID_LIST_LENGTH, DCM_TP_TX_CONFIRMATION_ID, DCM_E_PARAM);

	DslTpTxConfirmation(dcmTxPduId, result);
}

BufReq_ReturnType Dcm_CopyTxData(PduIdType dcmTxPduId, PduInfoType *pduInfoPtr, RetryInfoType *periodData, PduLengthType *txDataCntPtr)
{
	VALIDATE_RV(dcmState == DCM_INITIALIZED, DCM_COPY_TX_DATA_ID, DCM_E_UNINIT, BUFREQ_NOT_OK);
	VALIDATE_RV(dcmTxPduId < DCM_DSL_TX_PDU_ID_LIST_LENGTH, DCM_COPY_TX_DATA_ID, DCM_E_PARAM, BUFREQ_NOT_OK);

	return DslCopyTxData(dcmTxPduId, pduInfoPtr, periodData, txDataCntPtr);

}
