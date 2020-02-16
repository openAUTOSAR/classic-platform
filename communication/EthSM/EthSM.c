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

//lint -emacro(904,ETHSM_VALIDATE_INIT,ETHSM_VALIDATE_NETWORK,ETHSM_VALIDATE_POINTER,ETHSM_VALIDATE_MODE) //904 PC-Lint exception to MISRA 14.7 (validate macros).

/* Globally fulfilled requirements */
/** @req  ETHSM0082 */
/** @req  ETHSM0107 */
/** @req  ETHSM0014 */
/** @req  ETHSM0015 */
/** @req  ETHSM0016 */
/** @req  ETHSM0017 */
/** @req  ETHSM0018 */
/** @req  ETHSM0019 */
/** @req  ETHSM0020 */
/** @req  ETHSM0021 */
/** @req  ETHSM0022 */
/** @req  ETHSM0023 */
/** @req   */
/** @req   */
/** @req   */
/** @req   */
/** @req   */
/** @req   */
#ifdef USE_ETHSM
#include "ComStack_Types.h"
#include "EthSM.h"              /**< @req  */
#include "Com.h"                /**< @req  */
#include "ComM.h"               /**< @req  */
#include "ComM_BusSm.h"         /**< @req ETHSM0013 */
#include "Det.h"                /**< @req ETHSM0008 */
#if defined(USE_DEM)
#include "Dem.h"                /**< @req ETHSM0011 */
#endif
#include "EthIf.h"              /**< @req ETHSM0010 */
#include "EthSM_Internal.h"
#include "SoAd_EthSM.h" /**< @req ETHSM0106 */
#include "MemMap.h" /**< @req ETHSM0009 */

static EthSM_Internal_NetworkType EthSM_InternalNetworks[ETHSM_NETWORK_COUNT];

static EthSM_InternalType EthSM_Internal = {
		.InitStatus = ETHSM_STATUS_UNINIT,
		.Networks = EthSM_InternalNetworks,
};

static const EthSM_ConfigType* EthSM_Config;

/** @req ETHSM217.exceptTranceiver */
void EthSM_Init( const EthSM_ConfigType* ConfigPtr ) {
	ETHSM_VALIDATE_POINTER(ConfigPtr, ETHSM_SERVICEID_INIT);  /**< @req  */

	EthSM_Config = ConfigPtr;
	Std_ReturnType status = E_OK;
	Std_ReturnType totalStatus = E_OK;

	for (uint8 i = 0; i < ETHSM_NETWORK_COUNT; ++i) {
		status = EthSM_Internal_RequestComMode(i, COMM_NO_COMMUNICATION);  /**< @req  */
		if (status > totalStatus) {
			totalStatus = status;
		}
		EthSM_Internal.Networks[i].requestedMode = COMM_NO_COMMUNICATION;
	}

	if (totalStatus == E_OK) {
		EthSM_Internal.InitStatus = ETHSM_STATUS_INIT;
	} else {
		// IMPROVEMENT report error?
	}
}

/** @req   @req   @req .partially  @req  */
Std_ReturnType EthSM_RequestComMode( NetworkHandleType NetworkHandle, ComM_ModeType ComM_Mode ) {
	ETHSM_VALIDATE_INIT(ETHSM_SERVICEID_REQUESTCOMMODE, E_NOT_OK);
	ETHSM_VALIDATE_NETWORK(NetworkHandle, ETHSM_SERVICEID_REQUESTCOMMODE, E_NOT_OK);
	ETHSM_VALIDATE_MODE(ComM_Mode, ETHSM_SERVICEID_REQUESTCOMMODE, E_NOT_OK);

	return EthSM_Internal_RequestComMode(NetworkHandle, ComM_Mode);
}

/** @req .partially  @req   @req .exceptTranceiver  @req .exceptTranceiver
 *  @req   @req  */
Std_ReturnType EthSM_Internal_RequestComMode( NetworkHandleType NetworkHandle, ComM_ModeType ComM_Mode ) {
	Std_ReturnType overallStatus = E_OK;
	Std_ReturnType status;
	status = EthSM_Internal_RequestEthIfMode(NetworkHandle, ComM_Mode);      /**< @req  */
	if (status > overallStatus){
		overallStatus = status;
	}
	status = EthSM_Internal_RequestComGroupMode(NetworkHandle, ComM_Mode);   /**< @req  */
	if (status > overallStatus) {
		overallStatus = status;
	}

	if (overallStatus == E_OK) {
		ComM_BusSM_ModeIndication(NetworkHandle, ComM_Mode);                 /**< @req  */
		EthSM_Internal.Networks[NetworkHandle].requestedMode = ComM_Mode;
	}

	return overallStatus;
}

/** @req   @req  */
Std_ReturnType EthSM_Internal_RequestEthIfMode( NetworkHandleType NetworkHandle, ComM_ModeType ComM_Mode ) {
	const EthSM_NetworkType* Network = &EthSM_Config->Networks[NetworkHandle];
	EthIf_ControllerModeType EthIf_Mode = ETHIF_CS_STARTED;
	Std_ReturnType totalStatus = E_OK;

	switch (ComM_Mode) {
		case COMM_NO_COMMUNICATION:
/* ETH wake-up capability is not implemented so we cannot set
 * controllers into sleep because then we would be unable to
 * wake except on internal requests.
 */
#if 0
			EthIf_Mode = ETHIF_CS_SLEEP;
#endif
			EthIf_Mode = ETHIF_CS_STARTED;
			break;
		case COMM_FULL_COMMUNICATION:
			EthIf_Mode = ETHIF_CS_STARTED;
			break;
		default:
			totalStatus = E_NOT_OK;
			break;
	}

	if (totalStatus == E_OK) {
		for (uint8 i = 0; i < Network->ControllerCount; ++i) {
			const EthSM_ControllerType* Controller = &Network->Controllers[i];
			Std_ReturnType status =
					EthIf_SetControllerMode(Controller->EthIfControllerId, EthIf_Mode);
			if (status > totalStatus) {
				totalStatus = status;
			}
		}
	}
	return totalStatus;
}

/** @req  */
Std_ReturnType EthSM_Internal_RequestComGroupMode( NetworkHandleType NetworkHandle, ComM_ModeType ComM_Mode ) {
	const EthSM_NetworkType* Network = &EthSM_Config->Networks[NetworkHandle];
	Std_ReturnType status = E_OK;

	switch (ComM_Mode) {
		case COMM_NO_COMMUNICATION:
			Com_IpduGroupStop(Network->ComRxPduGroupId);
			Com_IpduGroupStop(Network->ComTxPduGroupId);
			break;
		case COMM_SILENT_COMMUNICATION:
			Com_IpduGroupStart(Network->ComRxPduGroupId, FALSE);
			Com_IpduGroupStop(Network->ComTxPduGroupId);
			break;
		case COMM_FULL_COMMUNICATION:
			Com_IpduGroupStart(Network->ComRxPduGroupId, FALSE);
			Com_IpduGroupStart(Network->ComTxPduGroupId, FALSE);
			break;
		default:
			status = E_NOT_OK;
			break;
	}
	return status;
}

/** @req   @req   @req   @req   @req  */
Std_ReturnType EthSM_GetCurrentComMode( NetworkHandleType NetworkHandle, ComM_ModeType* ComM_ModePtr ) {
	ETHSM_VALIDATE_INIT(ETHSM_SERVICEID_GETCURRENTCOMMODE, E_NOT_OK);
	ETHSM_VALIDATE_NETWORK(NetworkHandle, ETHSM_SERVICEID_GETCURRENTCOMMODE, E_NOT_OK);

	ComM_ModeType ComM_Mode = COMM_FULL_COMMUNICATION;

	const EthSM_NetworkType* Network = &EthSM_Config->Networks[NetworkHandle];
	Std_ReturnType totalStatus = E_OK;
	for (uint8 i = 0; i < Network->ControllerCount; ++i) {
		const EthSM_ControllerType* Controller = &Network->Controllers[i];
		EthIf_ControllerModeType EthIf_Mode;
		Std_ReturnType status =
				EthIf_GetControllerMode(Controller->EthIfControllerId, &EthIf_Mode);
		if (status > totalStatus) {
			totalStatus = status;
		}

		if (EthIf_Mode != ETHIF_CS_STARTED) {
			ComM_Mode = COMM_NO_COMMUNICATION;
		}
	}

	*ComM_ModePtr = ComM_Mode;
	return totalStatus;
}

void EthSM_MainFunction() {
	ETHSM_VALIDATE_INIT(ETHSM_SERVICEID_MAINFUNCTION);
}

Std_ReturnType EthSM_TrcvLinkStateChg( NetworkHandleType NetworkHandle, EthTrcv_LinkStateType TransceiverLinkState );
Std_ReturnType EthSm_TcpIpModeIndication( NetworkHandleType NetworkHandle, TcpIp_StateType TcpIpState );
Std_ReturnType EthSM_GetCurrentInternalMode( NetworkHandleType NetworkHandle, EthSM_NetworkModeStateType* EthSM_InternalMode );

void EthSM_CtrlModeIndication ( uint8 CtrlIdx, Eth_ModeType CtrlMode );
void EthSM_TrcvModeIndication ( uint8 TrcvIdx, EthTrcv_ModeType TrcvMode );
#endif /* USE_ETHSM */
