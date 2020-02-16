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




/* @req COM005 */




#ifndef COM_H_
#define COM_H_

#include "Std_Types.h"
#include "ComStack_Types.h"

/* @req COM786 */
/* @req COM424 */
#define COM_AR_RELEASE_MAJOR_VERSION   		4
#define COM_AR_RELEASE_MINOR_VERSION   		0
#define COM_AR_RELEASE_REVISION_VERSION   	3

#define COM_AR_MAJOR_VERSION   COM_AR_RELEASE_MAJOR_VERSION
#define COM_AR_MINOR_VERSION   COM_AR_RELEASE_MINOR_VERSION
#define COM_AR_PATCH_VERSION   COM_AR_RELEASE_REVISION_VERSION

#define COM_VENDOR_ID          60
#define COM_MODULE_ID          MODULE_ID_COM
#define COM_SW_MAJOR_VERSION   1
#define COM_SW_MINOR_VERSION   0
#define COM_SW_PATCH_VERSION   0

#include "Com_Cfg.h"

// This is needed since the RTE is using signal names (needs attention when it comes to post build)
#include "Com_PbCfg.h"

#include "Com_Types.h"
#include "Com_Com.h"
#include "Com_Sched.h"

typedef uint8 Com_IpduGroupVector[((COM_N_SUPPORTED_IPDU_GROUPS - 1)/8) + 1];
//-------------------------------------------------------------------
// From OSEK_VDX spec...
//
//typedef uint32 MessageIdentifier;
// TODO: Have no idea here..
//typedef void * ApplicationDataRef;






//-------------------------------------------------------------------

// From OSEK_VDX

/* The service SendMessage updates the message object identified by
 * <Message> with the application message referenced by the
 * <DataRef> parameter.
 *
 * Internal communication:
 * The message <Message> is routed to the receiving part of the IL.
  */
// Update 2008-10-30, SendMessage and ReceiveMessage should not be required. ensured by RTE. COM013
//StatusType SendMessage(MessageIdentifier , ApplicationDataRef );

// The service  ReceiveMessage updates the application message
// referenced by <DataRef> with the data in the message object
// identified by <Message>. It resets all flags (Notification classes 1 and
// 3) associated with <Message>.
//StatusType ReceiveMessage ( MessageIdentifier , ApplicationDataRef );


// From Autosar
/* @req COM432 */
void Com_Init(const Com_ConfigType * config);
/* @req COM130 */
void Com_DeInit(void);
/* @req COM751 */
void Com_IpduGroupControl(Com_IpduGroupVector ipduGroupVector, boolean Initialize);
/* @req COM749 */
void Com_ClearIpduGroupVector(Com_IpduGroupVector ipduGroupVector);
/* @req COM750 */
void Com_SetIpduGroup(Com_IpduGroupVector ipduGroupVector, Com_IpduGroupIdType ipduGroupId, boolean bitval);
/* @req COM194 */
Com_StatusType Com_GetStatus(void);

#if ( COM_VERSION_INFO_API == STD_ON )
/* @req COM425 */
/* @req COM426 */
#define Com_GetVersionInfo(_vi) STD_GET_VERSION_INFO(_vi,COM)
#endif

/* Autosar 4 api */
/* @req COM693 */
BufReq_ReturnType Com_CopyTxData(PduIdType PduId, PduInfoType* PduInfoPtr, RetryInfoType* RetryInfoPtr, PduLengthType* TxDataCntPtr);
/* @req COM692 */
BufReq_ReturnType Com_CopyRxData(PduIdType PduId, const PduInfoType* PduInfoPtr, PduLengthType* RxBufferSizePtr);
/* @req COM691 */
BufReq_ReturnType Com_StartOfReception(PduIdType ComRxPduId, PduLengthType TpSduLength, PduLengthType* RxBufferSizePtr);
/* @req COM650 */
void Com_TpRxIndication(PduIdType PduId, NotifResultType Result);
/* @req COM725 */
void Com_TpTxConfirmation(PduIdType PduId, NotifResultType Result);


extern ComSignalEndianess_type Com_SystemEndianness;
/* @req COM459 */ /* Partly */
#define COM_BUSY 0x81
#define COM_SERVICE_NOT_AVAILABLE 0x80
#endif /*COM_H_*/
