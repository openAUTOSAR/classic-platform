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

/* Generator version: 3.0.1
 * AUTOSAR version:   4.0.3
 */

#ifndef PDUR_CFG_H_
#define PDUR_CFG_H_

#if !(((PDUR_SW_MAJOR_VERSION == 3) && (PDUR_SW_MINOR_VERSION == 0)) )
#error PduR: Configuration file expected BSW module version to be 3.0.*
#endif

#if !(((PDUR_AR_RELEASE_MAJOR_VERSION == 4) && (PDUR_AR_RELEASE_MINOR_VERSION == 0)) )
#error PduR: Configuration file expected AUTOSAR version to be 4.0.*
#endif

#include "PduR_Types.h"


#define PDUR_CANIF_SUPPORT 					STD_ON
#define PDUR_CANTP_SUPPORT					STD_OFF
#define PDUR_CANNM_SUPPORT					STD_OFF
#define PDUR_FRIF_SUPPORT					STD_OFF
#define PDUR_FRTP_SUPPORT					STD_OFF
#define PDUR_FRNM_SUPPORT					STD_OFF
#define PDUR_LINIF_SUPPORT					STD_OFF
#define PDUR_LINTP_SUPPORT					STD_OFF
#define PDUR_COM_SUPPORT					STD_ON
#define PDUR_DCM_SUPPORT					STD_OFF
#define PDUR_IPDUM_SUPPORT					STD_OFF
#define PDUR_DOIP_SUPPORT 					STD_OFF
#define PDUR_SOAD_SUPPORT 					STD_OFF
#define PDUR_UDPNM_SUPPORT 					STD_OFF
#define PDUR_J1939TP_SUPPORT 				STD_OFF
#define PDUR_CDD_LINSLV_SUPPORT				STD_OFF
#define PDUR_CDDPDUR_SUPPORT				STD_OFF
#define PDUR_LDCOM_SUPPORT					STD_OFF
#define PDUR_SECOC_SUPPORT					STD_OFF

#define PDUR_DEV_ERROR_DETECT				STD_ON
#define PDUR_VERSION_INFO_API				STD_OFF
#define PDUR_ZERO_COST_OPERATION			STD_ON
#define PDUR_MAX_ROUTE_COUNT				30

#define PDUR_GATEWAY_OPERATION				STD_ON
#define PDUR_SB_TX_BUFFER_SUPPORT			STD_ON
#define PDUR_FIFO_TX_BUFFER_SUPPORT			STD_OFF

#define PDUR_MULTICAST_TOIF_SUPPORT			STD_ON
#define PDUR_MULTICAST_FROMIF_SUPPORT		STD_ON
#define PDUR_MULTICAST_TOTP_SUPPORT			STD_ON
#define PDUR_MULTICAST_FROMTP_SUPPORT		STD_ON


/* Minimum routing not supported.
#define PDUR_MINIMUM_ROUTING_UP_MODULE		COM
#define PDUR_MINIMUM_ROUTING_LO_MODULE		CAN_IF
#define PDUR_MINIMUM_ROUTING_UP_RXPDUID		((PduIdType)100)
#define PDUR_MINIMUM_ROUTING_LO_RXPDUID 	((PduIdType)255)
#define PDUR_MINIMUM_ROUTING_UP_TXPDUID 	((PduIdType)255)
#define PDUR_MINIMUM_ROUTING_LO_TXPDUID 	((PduIdType)255)
*/

#define PDUR_MAX_NOF_ROUTING_PATH_GROUPS 	0

#define PDUR_NO_BUFFER 0xFFFF


// Tx buffer IDs (sorted in the same order as PduRTxBuffers array)

/* Maximum number of routing paths:
 * N/A (ZERO_COST)
 *  */

/* Zero cost definitions */
#define PDUR_PDU_ID_PDU_1  CANIF_PDU_ID_PDU_1
#define PDUR_REVERSE_PDU_ID_PDU_1	ComConf_ComIPdu_PDU_1

#define PDUR_PDU_ID_PDU_2  CANIF_PDU_ID_PDU_2
#define PDUR_REVERSE_PDU_ID_PDU_2	CANIF_PDU_ID_PDU_2







// Alphabeticaly output Symbolic names id's for non-PB modules.



// Alphabeticaly output Polite Define id's for non-PB modules.
// Offset at the back id's for which Tp is overriden as source module.


#if PDUR_ZERO_COST_OPERATION == STD_ON
// Zero cost operation support active.
/* @req PDUR287 */
#if PDUR_CANIF_SUPPORT == STD_ON
#include "Com.h"
#define PduR_CanIfRxIndication Com_RxIndication
#define PduR_CanIfTxConfirmation Com_TxConfirmation
#else
#define PduR_CanIfRxIndication(... )
#define PduR_CanIfTxConfirmation(...)
#endif

#if PDUR_CANNM_SUPPORT == STD_ON
#include "Com.h"
#define PduR_CanNmRxIndication Com_RxIndication
#define PduR_CanNmTxConfirmation Com_TxConfirmation
#else
#define PduR_CanNmRxIndication(... )
#define PduR_CanNmTxConfirmation(...)
#endif

#if PDUR_UDPNM_SUPPORT == STD_ON
#include "Com.h"
#define PduR_UdpNmRxIndication Com_RxIndication
#define PduR_UdpNmTxConfirmation Com_TxConfirmation
#else
#define PduR_UdpNmRxIndication(... )
#define PduR_UdpNmTxConfirmation(...)
#endif

#if PDUR_CANTP_SUPPORT == STD_ON
#include "Dcm.h"
#define PduR_CanTpProvideRxBuffer Dcm_ProvideRxBuffer
#define PduR_CanTpRxIndication Dcm_RxIndication
#define PduR_CanTpProvideTxBuffer Dcm_ProvideTxBuffer
#define PduR_CanTpTxConfirmation Dcm_TxConfirmation
#else
#define PduR_CanTpProvideRxBuffer(...)
#define PduR_CanTpRxIndication(...)
#define PduR_CanTpProvideTxBuffer(...)
#define PduR_CanTpTxConfirmation(...)
#endif

#if PDUR_LINIF_SUPPORT == STD_ON
#include "Com.h"
#define PduR_LinIfRxIndication Com_RxIndication
#define PduR_LinIfTxConfirmation Com_TxConfirmation
#define PduR_LinIfTriggerTransmit Com_TriggerTransmit
#else
#define PduR_LinIfRxIndication(...)
#define PduR_LinIfTxConfirmation(...)
#define PduR_LinIfTriggerTransmit(...)
#endif

#if PDUR_CDD_LINSLV_SUPPORT == STD_ON
#include "Com.h"
#define PduR_LinSlvIfRxIndication Com_RxIndication
#define PduR_LinSlvIfTxConfirmation Com_TxConfirmation
#define PduR_LinSlvIfTriggerTransmit Com_TriggerTransmit
#else
#define PduR_LinSlvIfRxIndication(...)
#define PduR_LinSlvIfTxConfirmation(...)
#define PduR_LinSlvIfTriggerTransmit(...)
#endif

#if PDUR_CDDPDUR_SUPPORT == STD_ON
#error NOT SUPPORTED
#endif

#if PDUR_SOAD_SUPPORT == STD_ON
#include "Dcm.h"
#define PduR_SoAdTpProvideRxBuffer Dcm_ProvideRxBuffer
#define PduR_SoAdTpRxIndication Dcm_RxIndication
#define PduR_SoAdTpProvideTxBuffer Dcm_ProvideTxBuffer
#define PduR_SoAdTpTxConfirmation Dcm_TxConfirmation
#else
#define PduR_SoAdProvideRxBuffer(...)
#define PduR_SoAdRxIndication(...)
#define PduR_SoAdProvideTxBuffer(...)
#define PduR_SoAdTxConfirmation(...)
#endif

#if PDUR_J1939TP_SUPPORT == STD_ON
#include "Dcm.h"
#define PduR_J1939TpProvideRxBuffer Dcm_ProvideRxBuffer
#define PduR_J1939TpRxIndication Dcm_RxIndication
#define PduR_J1939TpProvideTxBuffer Dcm_ProvideTxBuffer
#define PduR_J1939TpTxConfirmation Dcm_TxConfirmation
#else
#define PduR_J1939TpProvideRxBuffer(...)
#define PduR_J1939TpRxIndication(...)
#define PduR_J1939TpProvideTxBuffer(...)
#define PduR_J1939TpTxConfirmation(...)
#endif

#if PDUR_FRIF_SUPPORT == STD_ON
#include "Com.h"
#define PduR_FrIfRxIndication Com_RxIndication
#define PduR_FrIfTxConfirmation Com_TxConfirmation
#define PduR_FrIfTriggerTransmit Com_TriggerTransmit
#else
#define PduR_FrIfRxIndication(... )
#define PduR_FrIfTxConfirmation(...)
#define PduR_FrIfTriggerTransmit(...)
#endif

#if PDUR_FRNM_SUPPORT == STD_ON
#include "Com.h"
#define PduR_FrNmRxIndication Com_RxIndication
#define PduR_FrNmTxConfirmation Com_TxConfirmation
#define PduR_FrNmTriggerTransmit Com_TriggerTransmit
#else
#define PduR_FrNmRxIndication(... )
#define PduR_FrNmTxConfirmation(...)
#define PduR_FrNmTriggerTransmit(...)
#endif

#if PDUR_FRTP_SUPPORT == STD_ON
#include "Dcm.h"
#define PduR_FrTpProvideRxBuffer Dcm_ProvideRxBuffer
#define PduR_FrTpRxIndication Dcm_RxIndication
#define PduR_FrTpProvideTxBuffer Dcm_ProvideTxBuffer
#define PduR_FrTpTxConfirmation Dcm_TxConfirmation
#else
#define PduR_FrTpProvideRxBuffer(...)
#define PduR_FrTpRxIndication(...)
#define PduR_FrTpProvideTxBuffer(...)
#define PduR_FrTpTxConfirmation(...)
#endif

#if PDUR_COM_SUPPORT == STD_ON
#include "CanIf.h"
#define PduR_ComTransmit CanIf_Transmit
#else
#define PduR_ComTransmit(... )	(E_OK)
#endif

#if PDUR_DCM_SUPPORT == STD_ON
#include "CanIf.h"
#define PduR_DcmTransmit CanIf_Transmit
#else
#define PduR_DcmTransmit(... )	(E_OK)
#endif

#if PDUR_LDCOM_SUPPORT == STD_ON
#include "CanIf.h"
#define PduR_LdComTransmit CanIf_Transmit
#else
#define PduR_LdComTransmit(...) (E_OK)
#endif


#endif // endif PDUR_ZERO_COST_OPERATION

#endif
