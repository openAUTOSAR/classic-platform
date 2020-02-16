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

/*
 * NB! This file is for COM internal use only and may only be included from COM C-files!
 */



#ifndef COM_INTERNAL_H_
#define COM_INTERNAL_H_

#include "Com_Arc_Types.h"

typedef struct {
	PduLengthType currentPosition;
	boolean locked;
} Com_BufferPduStateType;
extern Com_BufferPduStateType Com_BufferPduState[];

extern const Com_ConfigType *ComConfig;
extern Com_Arc_Config_type Com_Arc_Config;


#if COM_DEV_ERROR_DETECT == STD_ON
#include "Det.h"
/* @req COM442 */
/* @req COM028 */
#define DET_REPORTERROR(_api,_err) Det_ReportError(MODULE_ID_COM, 0, _api, _err);

#else

#define DET_REPORTERROR(_api,_err)

#endif

/* Error codes */
#define COM_E_PARAM				0x01
#define COM_E_UNINIT			0x02
#define COM_E_PARAM_POINTER		0x03

/* Service ids */
#define COM_INIT_ID							0x01
#define COM_DEINIT_ID						0x02
#define COM_IPDUGROUPCONTROL_ID				0x03
//#define COM_RECEPTIONDMCONTROL_ID			0x06
//#define COM_GETSTATUS_ID					0x07
//#define COM_GETCONFIGURATIONID_ID			0x08
//#define COM_GETVERSIONINFO_ID				0x09
#define COM_SENDSIGNAL_ID					0x0A
#define COM_RECEIVESIGNAL_ID				0x0B
#define COM_UPDATESHADOWSIGNAL_ID			0x0C
#define COM_SENDSIGNALGROUP_ID				0x0D
#define COM_RECEIVESIGNALGROUP_ID			0x0E
#define COM_RECEIVESHADOWSIGNAL_ID			0x0F
//#define COM_INVALIDATESIGNAL_ID			0x10
#define COM_INVALIDATESHADOWSIGNAL_ID		0x16
#define COM_TRIGGERIPDUSEND_ID				0x17
#define COM_MAINFUNCTIONRX_ID				0x18
#define COM_MAINFUNCTIONTX_ID				0x19
//#define COM_MAINFUNCTIONROUTESIGNALS_ID		0x1A
//#define COM_INVALIDATESIGNAL_GROUP_ID		0x1B
#define COM_CLEARIPDUGROUPVECTOR_ID			0x1C
#define COM_SETIPDUGROUP_ID					0x1D
#define COM_TPRXINDICATION_ID				0x1E
#define COM_SENDDYNSIGNAL_ID				0x21
#define COM_RECEIVEDYNSIGNAL_ID				0x22
#define COM_COPYRXDATA_ID					0x23
#define COM_COPYTXDATA_ID					0x24
#define COM_STARTOFRECEPTION_ID				0x25
#define COM_TPTXCONFIRMATION_ID				0x26
//#define COM_SWITCHIPDUTXMODE				0x27
#define COM_TXCONFIRMATION_ID				0x40
#define COM_TRIGGERTRANSMIT_ID				0x41
#define COM_RXINDICATION_ID					0x42

#define TESTBIT(source,bit)	( *( (uint8 *)source  + (bit / 8) ) &  (uint8)(1u << (bit % 8)) )
#define SETBIT(dest,bit)	( *( (uint8 *)dest    + (bit / 8) ) |= (uint8)(1u << (bit % 8)) )
#define CLEARBIT(dest,bit)	( *( (uint8 *)dest    + (bit / 8) ) &= (uint8)~(uint8)(1u << (bit % 8)) )


#define GET_Signal(SignalId) \
	(&ComConfig->ComSignal[SignalId])

#define GET_ArcSignal(SignalId) \
	(&Com_Arc_Config.ComSignal[SignalId])

#define GET_IPdu(IPduId) \
	(&ComConfig->ComIPdu[IPduId])

#define GET_ArcIPdu(IPduId) \
	(&Com_Arc_Config.ComIPdu[IPduId])

#define GET_GroupSignal(GroupSignalId) \
	(&ComConfig->ComGroupSignal[GroupSignalId])

#define GET_ArcGroupSignal(GroupSignalId) \
	(&Com_Arc_Config.ComGroupSignal[GroupSignalId])



#endif /* COM_INTERNAL_H_ */
