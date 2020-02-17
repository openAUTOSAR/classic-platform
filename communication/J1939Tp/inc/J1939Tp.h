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
/** @reqSettings DEFAULT_SPECIFICATION_REVISION=4.1.2 */

/** Global requirements */

#ifndef J1939TP_H_
#define J1939TP_H_

/** @req SWS_J1939Tp_00230 *//** @req SWS_J1939Tp_00085 */
#include "Std_Types.h"
/** @req J1939TP0016 */
#include "ComStack_Types.h"
#include "J1939Tp_ConfigTypes.h"
#include "CanIf.h"


#define J1939TP_VENDOR_ID			60u
#define J1939TP_MODULE_ID			37u

#define J1939TP_AR_MAJOR_VERSION	4u
#define J1939TP_AR_MINOR_VERSION	1u
#define J1939TP_AR_PATCH_VERSION	2u

#define J1939TP_SW_MAJOR_VERSION	1u
#define J1939TP_SW_MINOR_VERSION	0u
#define J1939TP_SW_PATCH_VERSION	0u

#include "J1939Tp_Cfg.h"


#if (J1939TP_VERSION_INFO_API == STD_ON)
/** Returns the version info of J1939Tp */
/** @req SWS_J1939Tp_00089 */
#define J1939Tp_GetVersionInfo(_vi) STD_GET_VERSION_INFO(_vi,J1939TP)
#endif

/** Initializes the module */
void J1939Tp_Init(const J1939Tp_ConfigType* ConfigPtr);

/** Close down communication, usually called by EcuM */
void J1939Tp_Shutdown(void);

/** Used to request the transfer of a J1939Tp N-SDU*/
Std_ReturnType J1939Tp_Transmit(PduIdType TxSduId, const PduInfoType* TxInfoPtr);

/** Used to cancel the ongoing transmission of a J1939Tp N-SDU */
Std_ReturnType J1939Tp_CancelTransmit(PduIdType TxSduId);

/** Used to cancel the ongoing reception of a J1939Tp N-SDU */
Std_ReturnType J1939Tp_CancelReceive(PduIdType RxSduId);

/** Used to change reception or transmission parameters of J1939Tp for a specific N-SDU */
Std_ReturnType J1939Tp_ChangeParameter(PduIdType SduId, TPParameterType Parameter, uint16 Value);

/** Main function, used for scheduling purpose and timeout supervision */
void J1939Tp_MainFunction(void);

#endif
