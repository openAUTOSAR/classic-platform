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

#ifndef ETHIF_CBK_H_
#define ETHIF_CBK_H_

#include "ComStack_Types.h"
#include "EthIf_Types.h"

/* @req 4.2.2/SWS_EthIf_00085 */
void EthIf_RxIndication( uint8 CtrlIdx, Eth_FrameType FrameType, boolean IsBroadcast, const uint8* PhysAddrPtr, Eth_DataType* DataPtr, uint16 LenByte );
/* @req 4.2.2/SWS_EthIf_00091 */
void EthIf_TxConfirmation( uint8 CtrlIdx, Eth_BufIdxType BufIdx );
/* @req 4.2.2/SWS_EthIf_00231 */
void EthIf_CtrlModeIndication( uint8 CtrlIdx, Eth_ModeType CtrlMode );
/* @req 4.2.2/SWS_EthIf_00232 */
void EthIf_TrcvModeIndication( uint8 CtrlIdx, EthTrcv_ModeType TrcvMode );

#endif /* ETHIF_CBK_H_ */
