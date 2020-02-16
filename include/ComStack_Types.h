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








#ifndef COMSTACK_TYPES_H_
#define COMSTACK_TYPES_H_

#define ECUC_SW_MAJOR_VERSION   1
#define ECUC_SW_MINOR_VERSION   0
#define ECUC_SW_PATCH_VERSION   0

#include "Std_Types.h" /** @req COMM820.partially */


// Zero-based integer number
// The size of this global type depends on the maximum
// number of PDUs used within one software module.
// Example :
// If  no software module deals with more PDUs that
// 256, this type can be set to uint8.
// If at least one software module handles more than
// 256 PDUs, this type must globally be set to uint16.

// In order to be able to perform table-indexing within a software
// module, variables of this type shall be zero-based and consecutive.
// There might be several ranges of PduIds in a module, one for each type of
// operation performed within that module (e.g. sending and receiving).

typedef uint16 PduIdType;
typedef uint16 PduLengthType;
typedef struct {
	uint8 *SduDataPtr;			// payload
	PduLengthType SduLength;	// length of SDU
} PduInfoType;

typedef enum {
    TP_STMIN=0,
    TP_BS,
    TP_BC,
} TPParameterType;

typedef enum {
	TP_DATACONF,
	TP_DATARETRY,
	TP_CONFPENDING,
	TP_NORETRY,
} TpDataStateType;

typedef struct {
	TpDataStateType TpDataState;
	PduLengthType TxTpDataCnt;
} RetryInfoType;

/*
typedef struct {
   P2VAR(uint8,AUTOMATIC,AUTOSAR_COMSTACKDATA) SduDataPtr
   PduLengthType   SduLength;
} PduInfoType;
*/

// TODO: remove all non-E_prefixed error enum values
typedef enum {
	BUFREQ_OK=0,
    BUFREQ_NOT_OK=1,
    BUFREQ_E_NOT_OK=1,
    BUFREQ_BUSY=2,
    BUFREQ_E_BUSY=2,
    BUFREQ_OVFL=3,
    BUFREQ_E_OVFL=3,
} BufReq_ReturnType;

// 0x00--0x1e General return types
// 0x1f--0x3c Error notif, CAN
// 0x3d--0x5a Error notif, LIN
// more
typedef uint8 NotifResultType;

#define NTFRSLT_OK				       0x00
#define NTFRSLT_E_NOT_OK		       0x01
#define NTFRSLT_E_TIMEOUT_A		       0x02
#define NTFRSLT_E_TIMEOUT_BS 	       0x03
#define NTFRSLT_E_TIMEOUT_CR	       0x04
#define NTFRSLT_E_WRONG_SN 		       0x05
#define NTFRSLT_E_INVALID_FS	       0x06
#define NTFRSLT_E_UNEXP_PDU		       0x07
#define NTFRSLT_E_WFT_OVRN		       0x08
#define NTFRSLT_E_ABORT			       0x09
#define NTFRSLT_E_NO_BUFFER 	       0x0A
#define NTFRSLT_E_CANCELATION_OK       0x0B
#define NTFRSLT_E_CANCELATION_NOT_OK   0x0C
#define NTFRSLT_PARAMETER_OK           0x0D
#define NTFRSLT_E_PARAMETER_NOT_OK     0x0E
#define NTFRSLT_E_RX_ON                0x0F
#define NTFRSLT_E_VALUE_NOT_OK         0x10


// TODO, more

typedef uint8 BusTrcvErrorType;


#define BUSTRCV_NO_ERROR	0x00
#define BUSBUSTRCV_E_ERROR	0x01


#define COMSTACKTYPE_AR_MINOR_VERSION		1
#define COMSTACKTYPE_AR_MAJOR_VERSION		0
#define COMSTACKTYPE_AR_PATCH_VERSION		0

typedef uint8 NetworkHandleType;

#endif /*COMSTACK_TYPES_H_*/
